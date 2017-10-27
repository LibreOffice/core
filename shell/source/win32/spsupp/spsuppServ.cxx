/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

// Include MIDL-generated file
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextra-tokens"
    // "#endif !_MIDL_USE_GUIDDEF_" in midl-generated code
#endif
#include <spsupp_i.c>
#if defined __clang__
#pragma clang diagnostic pop
#endif

#include <memory>
#include <olectl.h>
#include <wchar.h>
#include <spsuppServ.hpp>
#include <spsuppClassFactory.hpp>
#include <COMOpenDocuments.hpp>
#include <registrar.hpp>

namespace {

HANDLE g_hModule;

}

ITypeLib* GetTypeLib()
{
    typedef std::unique_ptr<ITypeLib, void(*)(IUnknown* p)> ITypeLibGuard;
    static ITypeLibGuard aITypeLibGuard(nullptr, [](IUnknown* p) { if (p) p->Release(); });
    if (!aITypeLibGuard.get())
    {
        wchar_t szFile[MAX_PATH];
        if (GetModuleFileNameW(static_cast<HMODULE>(g_hModule), szFile, MAX_PATH) == 0)
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
        if (GetModuleFileNameW(static_cast<HMODULE>(g_hModule), sPath, MAX_PATH) == 0)
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
    if (GetModuleFileNameW(static_cast<HMODULE>(g_hModule), szFile, MAX_PATH) == 0)
        return HRESULT_FROM_WIN32(GetLastError());

    HRESULT hr = RegisterTypeLib(pTypeLib, szFile, nullptr);
    if (FAILED(hr))
        return hr;

    return Registrar(CLSID_spsupp).RegisterObject(LIBID_spsupp, L"LOSPSupport", L"OpenDocuments", 1, szFile, true);
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

    return Registrar(CLSID_spsupp).UnRegisterObject(L"LOSPSupport", L"OpenDocuments", 1);
}

// This is called when regsvr32.exe is called with "/i" flag
// pszCmdLine is the string passed to "/i:<string>"
// See https://msdn.microsoft.com/library/windows/desktop/bb759846
STDAPI DllInstall(BOOL bInstall, _In_opt_ PCWSTR pszCmdLine)
{
    if (wcscmp(pszCmdLine, L"Substitute_OWSSUPP") == 0)
    {
        HRESULT hr;
        Registrar registrar(CLSID_spsupp);
        if (bInstall)
        {
            hr = registrar.RegisterProgID(L"SharePoint", L"OpenDocuments", 3, true);
            if (SUCCEEDED(hr))
                hr = registrar.RegisterProgID(L"SharePoint", L"OpenDocuments", 2, false);
            if (SUCCEEDED(hr))
                hr = registrar.RegisterProgID(L"SharePoint", L"OpenDocuments", 1, false);
        }
        else
        {
            // Try all ProgIDs regardless of error, but make sure to return failure result if at least one failed
            hr = registrar.UnRegisterProgID(L"SharePoint", L"OpenDocuments", 1);
            HRESULT hrLast;
            hr = SUCCEEDED(hrLast = registrar.UnRegisterProgID(L"SharePoint", L"OpenDocuments", 2)) ? hr : hrLast;
            hr = SUCCEEDED(hrLast = registrar.UnRegisterProgID(L"SharePoint", L"OpenDocuments", 3)) ? hr : hrLast;
        }
        return hr;
    }
    return E_INVALIDARG;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
