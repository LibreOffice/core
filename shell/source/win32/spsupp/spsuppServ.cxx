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

#include <shlwapi.h> // declaration of DllInstall

namespace
{
HANDLE g_hModule;

HMODULE GetHModule() { return static_cast<HMODULE>(g_hModule); }
} // namespace

ITypeLib* GetTypeLib()
{
    typedef std::unique_ptr<ITypeLib, void(*)(IUnknown* p)> ITypeLibGuard;
    static ITypeLibGuard s_aITypeLibGuard = [] {
        ITypeLibGuard aITypeLibGuard(nullptr, [](IUnknown* p) { if (p) p->Release(); });
        wchar_t szFile[MAX_PATH];
        if (GetModuleFileNameW(GetHModule(), szFile, MAX_PATH) == 0)
            return aITypeLibGuard;
        ITypeLib* pTypeLib;
        if (FAILED(LoadTypeLib(szFile, &pTypeLib)))
            return aITypeLibGuard;
        aITypeLibGuard.reset(pTypeLib);
        return aITypeLibGuard;
    }();
    return s_aITypeLibGuard.get();
}

const wchar_t* GetHelperExe()
{
    static wchar_t* s_sPath = []() -> wchar_t* {
        static wchar_t sPath[MAX_PATH];
        if (GetModuleFileNameW(GetHModule(), sPath, MAX_PATH) == 0)
            return nullptr;
        wchar_t* pSlashPos = wcsrchr(sPath, L'\\');
        if (pSlashPos == nullptr)
            return nullptr;
        wcscpy(pSlashPos + 1, L"spsupp_helper.exe");
        return sPath;
    }();
    return s_sPath;
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
    if (GetModuleFileNameW(GetHModule(), szFile, MAX_PATH) == 0)
        return HRESULT_FROM_WIN32(GetLastError());

    HRESULT hr = RegisterTypeLib(pTypeLib, szFile, nullptr);
    if (FAILED(hr))
        return hr;

    // Default is v.5
    return Registrar(CLSID_spsupp)
        .RegisterObject(LIBID_spsupp, L"LOSPSupport", L"OpenDocuments", { 5, 1, 2, 3, 4 }, szFile);
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

    return Registrar(CLSID_spsupp)
        .UnRegisterObject(L"LOSPSupport", L"OpenDocuments", { 1, 2, 3, 4, 5 });
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
            // Default is v.5
            hr = registrar.RegisterProgIDs(L"SharePoint", L"OpenDocuments", { 5, 1, 2, 3, 4 });
        }
        else
        {
            hr = registrar.UnRegisterProgIDs(L"SharePoint", L"OpenDocuments", { 1, 2, 3, 4, 5 });
        }
        return hr;
    }
    return E_INVALIDARG;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
