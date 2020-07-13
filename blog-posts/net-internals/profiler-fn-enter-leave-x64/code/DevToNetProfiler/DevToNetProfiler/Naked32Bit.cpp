#include "pch.h"
#include<iostream>

extern "C" void _stdcall StackOverflowDetected(FunctionID funcId, int count) {
  std::cout << "stackoverflow: " << funcId << ", count: " << count;
}

extern "C" void _stdcall EnterCpp(
  FunctionID funcId,
  int identifier) {
  std::cout << "enter funcion id: " << funcId << ", Arguments in correct order: " << (identifier == 12345) << "\r\n";
}

#ifdef _WIN64

#else

bool* activateCallbacks;
int* pHashMap;
int mapSize;

// mögliche fehler: im linker hinzufügen?
// https://social.msdn.microsoft.com/Forums/vstudio/en-US/e6e95a9c-bba1-43d1-9ecc-1c6f3ac9cdc1/vc-linking-to-x64-assembly-file-does-not-work?forum=vcgeneral

void InitEnterLeaveCallbacks(bool* activate, int* hashMap, int size) {
  activateCallbacks = activate;
  pHashMap = hashMap;
  mapSize = size;
}

void __declspec(naked) FnEnterCallback(
  FunctionID funcId,
  UINT_PTR clientData,
  COR_PRF_FRAME_INFO func,
  COR_PRF_FUNCTION_ARGUMENT_INFO* argumentInfo) {
  __asm {
    push ebx
    mov ebx, [activateCallbacks]
    cmp byte ptr[ebx], 1
    JNE skipCallback

    ; check stackoverflow
    mov ebx, [pHashMap]
    mov eax, [ESP + 8]
    xor edx, edx
    div dword ptr [mapSize]
    add ebx, edx
    inc dword ptr [ebx]
    cmp dword ptr [ebx], 30
    jb skipStackOverflow

    push [ebx]
    push [ESP + 8]
    CALL StackOverflowDetected

    skipStackOverflow:

    ; push last parameter first!
    push 12345
    push [ESP+12]
    call EnterCpp

    skipCallback:    

    pop ebx
    ret 16
  }
}

void __declspec(naked) FnLeaveCallback(
  FunctionID funcId,
  UINT_PTR clientData,
  COR_PRF_FRAME_INFO func,
  COR_PRF_FUNCTION_ARGUMENT_INFO* argumentInfo) {
  __asm {
    push ebx
    mov ebx, [activateCallbacks]
    cmp byte ptr[ebx], 1
    JNE skipCallback

    mov ebx, [pHashMap]
    mov eax, [ESP + 8]
    xor edx, edx
    div dword ptr [mapSize]
    add ebx, edx
    dec dword ptr [ebx]

    skipCallback:

    pop ebx
    ret 16
  }
}

void __declspec(naked) FnTailcallCallback(FunctionID funcId,
  UINT_PTR clientData,
  COR_PRF_FRAME_INFO func) {
  __asm {
    ret 12
  }
}
#endif
