/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

// Include MIDL-generated file
#include "spsupp_i.c"

#include <memory>
#include "olectl.h"
#include "spsuppServ.hpp"
#include "spsuppClassFactory.hpp"
#include "COMOpenDocuments.hpp"
#include "registrar.hpp"

HANDLE g_hModule;

ITypeLib* GetTypeLib()
{
    typedef std::unique_ptr<ITypeLib, void(*)(IUnknown* p)> ITypeLibGuard;
    static ITypeLibGuard aITypeLibGuard(nullptr, [](IUnknown* p) { if (p) p->Release(); });
    if (!aITypeLibGuard.get())
    {
        wchar_t szFile[MAX_PATH];
        if (GetModuleFileNameW((HMODULE)g_hModule, szFile, MAX_PATH) == 0)
            return nullptr;
        ITypeLib* pTypeLib;
        HRESULT hr = LoadTypeLib(szFile, &pTypeLib);
        if (FAILED(hr))
            return nullptr;
        aITypeLibGuard.reset(pTypeLib);
    }
    return aITypeLibGuard.get();
}

const wchar_t* GetLOPath()
{
    static wchar_t sPath[MAX_PATH] = { 0 };
    if (*sPath == 0)
    {
        // Initialization
        if (GetModuleFileNameW((HMODULE)g_hModule, sPath, MAX_PATH) == 0)
            return nullptr;
        wchar_t* pSlashPos = wcsrchr(sPath, L'\\');
        if (pSlashPos == nullptr)
            return nullptr;
        wcscpy(pSlashPos+1, L"soffice.exe");
    }
    return sPath;
}

BOOL APIENTRY DllMain( HANDLE hinstDLL,
                       DWORD  fdwReason,
                       LPVOID /*lpvReserved*/ )
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            g_hModule = hinstDLL;
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

namespace {
    // {F1924D0C-9B35-4A46-BCDE-CFEF2CE67A17}
    static const IID CLSID_spsupp =
    { 0xf1924d0c, 0x9b35, 0x4a46, { 0xbc, 0xde, 0xcf, 0xef, 0x2c, 0xe6, 0x7a, 0x17 } };
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvOut)
{
    *ppvOut = nullptr;
    if (IsEqualIID(rclsid, CLSID_spsupp))
    {
       ClassFactory *pCf = new ClassFactory;
       HRESULT hr = pCf->QueryInterface(riid, ppvOut);
       pCf->Release();
       return hr;
    }
    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllCanUnloadNow(void)
{
    if (ClassFactory::GetLockCount() == 0 &&
        ClassFactory::GetObjectCount() == 0 &&
        COMOpenDocuments::GetObjectCount() == 0)
        return S_OK;
    else
        return S_FALSE;
}

STDAPI DllRegisterServer(void)
{
    ITypeLib* pTypeLib = GetTypeLib();
    if (!pTypeLib)
        return ResultFromScode(SELFREG_E_TYPELIB);

    wchar_t szFile[MAX_PATH];
    if (GetModuleFileNameW((HMODULE)g_hModule, szFile, MAX_PATH) == 0)
        return HRESULT_FROM_WIN32(GetLastError());

    HRESULT hr = RegisterTypeLib(pTypeLib, szFile, nullptr);
    if (FAILED(hr))
        return hr;

    return Registrar::RegisterObject(CLSID_spsupp, LIBID_spsupp, L"LOSPSupport", L"OpenDocuments", szFile);
}

STDAPI DllUnregisterServer(void)
{
    ITypeLib* pTypeLib = GetTypeLib();
    if (!pTypeLib)
        return ResultFromScode(SELFREG_E_TYPELIB);
    TLIBATTR* pLibAttr;
    HRESULT hr = pTypeLib->GetLibAttr(&pLibAttr);
    if (FAILED(hr))
        return hr;
    auto ReleaseFunc = [pTypeLib](TLIBATTR* p) { if (p) pTypeLib->ReleaseTLibAttr(p); };
    typedef std::unique_ptr<TLIBATTR, decltype(ReleaseFunc)> TLIBATTRGuard;
    static TLIBATTRGuard aTLIBATTRGuard(nullptr, ReleaseFunc);

    hr = UnRegisterTypeLib(pLibAttr->guid, pLibAttr->wMajorVerNum, pLibAttr->wMinorVerNum, pLibAttr->lcid, pLibAttr->syskind);
    if (FAILED(hr))
        return hr;

    return Registrar::UnRegisterObject(CLSID_spsupp, L"LOSPSupport", L"OpenDocuments");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
