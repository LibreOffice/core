/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include "stdafx.h"

#include "XMergeFilter.h"
#include "XMergeFactory.h"


CXMergeSyncModule _Module;


//////////////////////////////////////////////////////////////////////
// DLL Functions
//////////////////////////////////////////////////////////////////////
BOOL WINAPI DllMain(HANDLE hInst, ULONG ulReason, LPVOID lpReserved)
{
    switch (ulReason)
    {
        case DLL_PROCESS_ATTACH:
            _Module.m_hInst = reinterpret_cast<HINSTANCE>(hInst);
            break;

        case DLL_PROCESS_DETACH:
            _Module.m_hInst = NULL;
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;
    }

    return TRUE;
}


STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
    // Create the factory object
    CXMergeFactory *pFactory = new CXMergeFactory();
    if (pFactory == NULL) 
    {
        *ppv = NULL;
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pFactory->QueryInterface(riid, ppv);
    pFactory->Release();

    return hr;
}


STDAPI DllCanUnloadNow() 
{
    if (_Module.GetLockCount() == 0)
        return S_OK;

    return S_FALSE;
}


// Utility function to close open keys during registration
static _signalRegError(long lRet, HKEY hKey, HKEY hDataKey)
{
    if (hKey)
        ::RegCloseKey(hKey);

    
    if (hDataKey)
        ::RegCloseKey(hDataKey);
    
    return HRESULT_FROM_WIN32(lRet);
}


STDAPI DllRegisterServer()
{
    HKEY hKey = NULL;
    HKEY hDataKey = NULL;
    
    long lRet = 0;
    TCHAR sTemp[_MAX_PATH + 1] = "\0";


    /*
     * Following calls create the HKEY_CLASSES_ROOT\CLSID entry for the Writer export filter.
     *
     * Note that import are export are relative to the WinCE device, so files are 
     * exported to the desktop format.
     */

    // Get a handle to the CLSID key
    lRet = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("CLSID"), 0, KEY_ALL_ACCESS, &hKey);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    // Create the CLSID key for the XMergeFilter
    lRet = ::RegCreateKeyEx(hKey, CXMergeFilter::m_pszPSWExportCLSID, 0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hKey, NULL);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegSetValueEx(hKey, _T(""), 0, REG_SZ, (LPBYTE)CXMergeFilter::m_pszPSWExportShortDesc,
                (::_tcslen(CXMergeFilter::m_pszPSWExportShortDesc) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);


    // Create the DefaultIcon key.  For the moment, use one of the Async supplied ones
    lRet = ::RegCreateKeyEx(hKey, _T("DefaultIcon"), 0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hDataKey, NULL);
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegSetValueEx(hDataKey, NULL, 0, REG_SZ, (LPBYTE)_T("C:\\Program Files\\Microsoft ActiveSync\\pwdcnv.dll,0"),
                            (::_tcslen(_T("C:\\Program Files\\Microsoft ActiveSync\\pwdcnv.dll,0")) 
                               * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);
    ::RegCloseKey(hDataKey);  hDataKey = NULL;

    // Create the InprocServer32 key
    lRet = ::RegCreateKeyEx(hKey, _T("InProcServer32"), 0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hDataKey, NULL);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegSetValueEx(hDataKey, _T("ThreadingModel"), 0, REG_SZ, (LPBYTE)_T("Apartment"), 10);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);
    
    // Create the key for the DLL file.  First find the filename of the dll
    if (!::GetModuleFileName((HMODULE)_Module.m_hInst, sTemp, (_MAX_PATH + 1)))
    {
        lRet = ::GetLastError();
        if (lRet != ERROR_SUCCESS) 
            return _signalRegError(lRet, hKey, hDataKey);	
    }
    
    
    lRet = ::RegSetValueEx(hDataKey, NULL, 0, REG_SZ, (LPBYTE)sTemp, 
                (::_tcslen(sTemp) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);
    ::RegCloseKey(hDataKey);	hDataKey = NULL;


    // Setup the PegasusFilter key values
    lRet = ::RegCreateKeyEx(hKey, _T("PegasusFilter"), 0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hDataKey, NULL);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegSetValueEx(hDataKey, _T("Description"), 0, REG_SZ, (LPBYTE)CXMergeFilter::m_pszPSWExportDesc,
                (::_tcslen(CXMergeFilter::m_pszPSWExportDesc) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);


    lRet = ::RegSetValueEx(hDataKey, _T("Export"), 0, REG_SZ, (LPBYTE)_T(""), (1 * sizeof(TCHAR)));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);


    lRet = ::RegSetValueEx(hDataKey, _T("NewExtension"), 0, REG_SZ, (LPBYTE)CXMergeFilter::m_pszPSWExportExt,
                (::_tcslen(CXMergeFilter::m_pszPSWExportExt) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);


    ::RegCloseKey(hKey);		hKey = NULL;
    ::RegCloseKey(hDataKey);	hDataKey = NULL;




    /*
     * Following calls create the entries for the filter in 
     * HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows CE Services\Filters
     */

    lRet = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows CE Services\\Filters"), 
                0, KEY_ALL_ACCESS, &hKey);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    _snprintf(sTemp, _MAX_PATH + 1, "%c%s\\InstalledFilters\0", '.', CXMergeFilter::m_pszPSWImportExt);
    lRet = ::RegCreateKeyEx(hKey, _T(sTemp), 
                0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hDataKey, NULL);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegSetValueEx(hDataKey, CXMergeFilter::m_pszPSWExportCLSID, 0, REG_SZ, (LPBYTE)_T(""), (1 * sizeof(TCHAR)));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    ::RegCloseKey(hKey);		hKey = NULL;
    ::RegCloseKey(hDataKey);	hDataKey = NULL;



    /*
     * Following calls create the HKEY_CLASSES_ROOT\CLSID entry for the Writer import filter.
     *
     * Note that import are export are relative to the WinCE device, so files are 
     * exported to the desktop format.
     */
    // Get a handle to the CLSID key
    lRet = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("CLSID"), 0, KEY_ALL_ACCESS, &hKey);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    // Create the CLSID key for the XMergeFilter
    lRet = ::RegCreateKeyEx(hKey, CXMergeFilter::m_pszPSWImportCLSID, 0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hKey, NULL);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegSetValueEx(hKey, _T(""), 0, REG_SZ, (LPBYTE)CXMergeFilter::m_pszPSWImportShortDesc,
                (::_tcslen(CXMergeFilter::m_pszPSWImportShortDesc) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);


    // Create the DefaultIcon key.  For the moment, use one of the Async supplied ones
    lRet = ::RegCreateKeyEx(hKey, _T("DefaultIcon"), 0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hDataKey, NULL);
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegSetValueEx(hDataKey, NULL, 0, REG_SZ, (LPBYTE)_T("C:\\Program Files\\Microsoft ActiveSync\\pwdcnv.dll,0"),
                            (::_tcslen(_T("C:\\Program Files\\Microsoft ActiveSync\\pwdcnv.dll,0")) 
                               * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);
    ::RegCloseKey(hDataKey);  hDataKey = NULL;

    
    // Create the InprocServer32 key
    lRet = ::RegCreateKeyEx(hKey, _T("InProcServer32"), 0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hDataKey, NULL);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegSetValueEx(hDataKey, _T("ThreadingModel"), 0, REG_SZ, (LPBYTE)_T("Apartment"), 10);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);


    // Create the key for the DLL file.  First find the filename of the dll
    if (!::GetModuleFileName((HMODULE)_Module.m_hInst, sTemp, (_MAX_PATH + 1)))
    {
        lRet = ::GetLastError();
        if (lRet != ERROR_SUCCESS) 
            return _signalRegError(lRet, hKey, hDataKey);	
    }
    
    
    lRet = ::RegSetValueEx(hDataKey, NULL, 0, REG_SZ, (LPBYTE)sTemp, 
                (::_tcslen(sTemp) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);
    ::RegCloseKey(hDataKey);	hDataKey = NULL;


    // Setup the PegasusFilter key values
    lRet = ::RegCreateKeyEx(hKey, _T("PegasusFilter"), 0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hDataKey, NULL);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

        lRet = ::RegSetValueEx(hDataKey, _T("Description"), 0, REG_SZ, (LPBYTE)CXMergeFilter::m_pszPSWImportDesc,
                (::_tcslen(CXMergeFilter::m_pszPSWImportDesc) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);


    lRet = ::RegSetValueEx(hDataKey, _T("Import"), 0, REG_SZ, (LPBYTE)_T(""), (1 * sizeof(TCHAR)));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);


    lRet = ::RegSetValueEx(hDataKey, _T("NewExtension"), 0, REG_SZ, (LPBYTE)CXMergeFilter::m_pszPSWImportExt,
                (::_tcslen(CXMergeFilter::m_pszPSWImportExt) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);


    ::RegCloseKey(hKey);		hKey = NULL;
    ::RegCloseKey(hDataKey);	hDataKey = NULL;


    /*
     * Following calls create the entries for the filter in 
     * HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows CE Services\Filters
     */
    lRet = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows CE Services\\Filters"), 
                0, KEY_ALL_ACCESS, &hKey);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    // Add in defaults for import and export
    _snprintf(sTemp, _MAX_PATH +1, "%c%s\0", '.', CXMergeFilter::m_pszPSWExportExt);
    lRet = ::RegCreateKeyEx(hKey, _T(sTemp), 0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hDataKey, NULL);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegSetValueEx(hDataKey, _T("DefaultImport"), 0, REG_SZ, 
                            (LPBYTE)CXMergeFilter::m_pszPSWImportCLSID, 
                            (::_tcslen(CXMergeFilter::m_pszPSWImportDesc) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);


    lRet = ::RegSetValueEx(hDataKey, _T("DefaultExport"), 0, REG_SZ, (LPBYTE)_T("Binary Copy"),
                            (::_tcslen(_T("Binary Copy")) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    ::RegCloseKey(hDataKey);
                            
    // Update registered filters
    _snprintf(sTemp, _MAX_PATH + 1, "%c%s\\InstalledFilters\0", '.', CXMergeFilter::m_pszPSWExportExt);
    lRet = ::RegCreateKeyEx(hKey, _T(sTemp), 
                0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hDataKey, NULL);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    
    lRet = ::RegSetValueEx(hDataKey, CXMergeFilter::m_pszPSWImportCLSID, 0, REG_SZ, (LPBYTE)_T(""), (1 * sizeof(TCHAR)));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    ::RegCloseKey(hKey);		hKey = NULL;
    ::RegCloseKey(hDataKey);	hDataKey = NULL;



    /*
     * Following calls create the HKEY_CLASSES_ROOT\CLSID entry for the Calc export filter.
     *
     * Note that import are export are relative to the WinCE device, so files are 
     * exported to the desktop format.
     */

    // Get a handle to the CLSID key
    lRet = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("CLSID"), 0, KEY_ALL_ACCESS, &hKey);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    // Create the CLSID key for the XMerge Filter
    lRet = ::RegCreateKeyEx(hKey, CXMergeFilter::m_pszPXLExportCLSID, 0, _T(""), 
                                0, KEY_ALL_ACCESS, NULL, &hKey, NULL);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegSetValueEx(hKey, _T(""), 0, REG_SZ, (LPBYTE)CXMergeFilter::m_pszPXLExportShortDesc,
                (::_tcslen(CXMergeFilter::m_pszPXLExportShortDesc) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);


    // Create the DefaultIcon key.  For the moment, use one of the Async supplied ones
    lRet = ::RegCreateKeyEx(hKey, _T("DefaultIcon"), 0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hDataKey, NULL);
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegSetValueEx(hDataKey, NULL, 0, REG_SZ, (LPBYTE)_T("C:\\Program Files\\Microsoft ActiveSync\\pwdcnv.dll,0"),
                            (::_tcslen(_T("C:\\Program Files\\Microsoft ActiveSync\\pwdcnv.dll,0")) 
                               * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);
    ::RegCloseKey(hDataKey);  hDataKey = NULL;


    // Create the InprocServer32 key
    lRet = ::RegCreateKeyEx(hKey, _T("InProcServer32"), 0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hDataKey, NULL);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegSetValueEx(hDataKey, _T("ThreadingModel"), 0, REG_SZ, (LPBYTE)_T("Apartment"), 10);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    
    // Create the key for the DLL file.  First find the filename of the dll
    if (!::GetModuleFileName((HMODULE)_Module.m_hInst, sTemp, (_MAX_PATH + 1)))
    {
        lRet = ::GetLastError();
        if (lRet != ERROR_SUCCESS) 
            return _signalRegError(lRet, hKey, hDataKey);	
    }
    
    
    lRet = ::RegSetValueEx(hDataKey, NULL, 0, REG_SZ, (LPBYTE)sTemp, 
                (::_tcslen(sTemp) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);
    ::RegCloseKey(hDataKey);	hDataKey = NULL;


    // Setup the PegasusFilter key values
    lRet = ::RegCreateKeyEx(hKey, _T("PegasusFilter"), 0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hDataKey, NULL);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

        lRet = ::RegSetValueEx(hDataKey, _T("Description"), 0, REG_SZ, (LPBYTE)CXMergeFilter::m_pszPXLExportDesc,
                (::_tcslen(CXMergeFilter::m_pszPXLExportDesc) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);


    lRet = ::RegSetValueEx(hDataKey, _T("Export"), 0, REG_SZ, (LPBYTE)_T(""), (1 * sizeof(TCHAR)));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);


    lRet = ::RegSetValueEx(hDataKey, _T("NewExtension"), 0, REG_SZ, (LPBYTE)CXMergeFilter::m_pszPXLExportExt,
                (::_tcslen(CXMergeFilter::m_pszPXLExportExt) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);


    ::RegCloseKey(hKey);		hKey = NULL;
    ::RegCloseKey(hDataKey);	hDataKey = NULL;




    /*
     * Following calls create the entries for the filter in 
     * HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows CE Services\Filters
     */

    lRet = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows CE Services\\Filters"), 
                0, KEY_ALL_ACCESS, &hKey);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    _snprintf(sTemp, _MAX_PATH + 1, "%c%s\\InstalledFilters\0", '.', CXMergeFilter::m_pszPXLImportExt);
    lRet = ::RegCreateKeyEx(hKey, _T(sTemp), 
                0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hDataKey, NULL);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegSetValueEx(hDataKey, CXMergeFilter::m_pszPXLExportCLSID, 0, REG_SZ, (LPBYTE)_T(""), (1 * sizeof(TCHAR)));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    ::RegCloseKey(hKey);		hKey = NULL;
    ::RegCloseKey(hDataKey);	hDataKey = NULL;



    /*
     * Following calls create the HKEY_CLASSES_ROOT\CLSID entry for the Calc import filter.
     *
     * Note that import are export are relative to the WinCE device, so files are 
     * exported to the desktop format.
     */
    // Get a handle to the CLSID key
    lRet = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("CLSID"), 0, KEY_ALL_ACCESS, &hKey);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);


    // Create the CLSID key for the XMergeFilter
    lRet = ::RegCreateKeyEx(hKey, CXMergeFilter::m_pszPXLImportCLSID, 0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hKey, NULL);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegSetValueEx(hKey, _T(""), 0, REG_SZ, (LPBYTE)CXMergeFilter::m_pszPXLImportShortDesc,
                (::_tcslen(CXMergeFilter::m_pszPXLImportShortDesc) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    // Create the DefaultIcon key.  For the moment, use one of the Async supplied ones
    lRet = ::RegCreateKeyEx(hKey, _T("DefaultIcon"), 0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hDataKey, NULL);
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegSetValueEx(hDataKey, NULL, 0, REG_SZ, (LPBYTE)_T("C:\\Program Files\\Microsoft ActiveSync\\pwdcnv.dll,0"),
                            (::_tcslen(_T("C:\\Program Files\\Microsoft ActiveSync\\pwdcnv.dll,0")) 
                               * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);
    ::RegCloseKey(hDataKey);  hDataKey = NULL;

    
    // Create the InprocServer32 key
    lRet = ::RegCreateKeyEx(hKey, _T("InProcServer32"), 0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hDataKey, NULL);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegSetValueEx(hDataKey, _T("ThreadingModel"), 0, REG_SZ, (LPBYTE)_T("Apartment"), 10);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);


    // Create the key for the DLL file.  First find the filename of the dll
    if (!::GetModuleFileName((HMODULE)_Module.m_hInst, sTemp, (_MAX_PATH + 1)))
    {
        lRet = ::GetLastError();
        if (lRet != ERROR_SUCCESS) 
            return _signalRegError(lRet, hKey, hDataKey);	
    }
    
    
    lRet = ::RegSetValueEx(hDataKey, NULL, 0, REG_SZ, (LPBYTE)sTemp, 
                (::_tcslen(sTemp) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);
    ::RegCloseKey(hDataKey);	hDataKey = NULL;


    // Setup the PegasusFilter key values
    lRet = ::RegCreateKeyEx(hKey, _T("PegasusFilter"), 0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hDataKey, NULL);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

        lRet = ::RegSetValueEx(hDataKey, _T("Description"), 0, REG_SZ, (LPBYTE)CXMergeFilter::m_pszPXLImportDesc,
                (::_tcslen(CXMergeFilter::m_pszPXLImportDesc) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);


    lRet = ::RegSetValueEx(hDataKey, _T("Import"), 0, REG_SZ, (LPBYTE)_T(""), (1 * sizeof(TCHAR)));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);


    lRet = ::RegSetValueEx(hDataKey, _T("NewExtension"), 0, REG_SZ, (LPBYTE)CXMergeFilter::m_pszPXLImportExt,
                (::_tcslen(CXMergeFilter::m_pszPXLImportExt) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);


    ::RegCloseKey(hKey);		hKey = NULL;
    ::RegCloseKey(hDataKey);	hDataKey = NULL;



    /*
     * Following calls create the entries for the filter in 
     * HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows CE Services\Filters
     */
    lRet = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows CE Services\\Filters"), 
                0, KEY_ALL_ACCESS, &hKey);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    // Add in defaults for import and export
    _snprintf(sTemp, _MAX_PATH +1, "%c%s\0", '.', CXMergeFilter::m_pszPXLExportExt);
    lRet = ::RegCreateKeyEx(hKey, _T(sTemp), 0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hDataKey, NULL);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegSetValueEx(hDataKey, _T("DefaultImport"), 0, REG_SZ, 
                            (LPBYTE)CXMergeFilter::m_pszPXLImportCLSID, 
                            (::_tcslen(CXMergeFilter::m_pszPSWImportDesc) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);


    lRet = ::RegSetValueEx(hDataKey, _T("DefaultExport"), 0, REG_SZ, (LPBYTE)_T("Binary Copy"),
                            (::_tcslen(_T("Binary Copy")) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    ::RegCloseKey(hDataKey);
                            
    // Update registered filters


    _snprintf(sTemp, _MAX_PATH + 1, "%c%s\\InstalledFilters\0", '.', CXMergeFilter::m_pszPXLExportExt);
    lRet = ::RegCreateKeyEx(hKey, _T(sTemp), 
                0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hDataKey, NULL);
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegSetValueEx(hDataKey, CXMergeFilter::m_pszPXLImportCLSID, 0, REG_SZ, (LPBYTE)_T(""), (1 * sizeof(TCHAR)));
    if (lRet != ERROR_SUCCESS) 
        return _signalRegError(lRet, hKey, hDataKey);

    ::RegCloseKey(hKey);		hKey = NULL;
    ::RegCloseKey(hDataKey);	hDataKey = NULL;



    return HRESULT_FROM_WIN32(lRet);
}


STDAPI DllUnregisterServer()
{
    long lRet = 0;
    HKEY hKey = NULL;
    HKEY hDataKey = NULL;
    
    TCHAR szClassName[_MAX_PATH] = "\0";
    TCHAR szKeyName[_MAX_PATH]   = "\0";
    DWORD dwClassName            = _MAX_PATH;
    DWORD dwKeyName              = _MAX_PATH;

    /*
     * Remove HKEY_CLASS_ROOT\CLSID\{XXX} entry for the export and import filters
     *
     * Windows 95/98/Me allow one step deletion of a key and all subkeys.
     * Windows NT/2000/XP do not so the subkeys must be deleted individually.
     */
    lRet = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("CLSID"), 0, KEY_ALL_ACCESS, &hKey);
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);


    // First up, the Writer export filter
    lRet = ::RegOpenKeyEx(hKey, CXMergeFilter::m_pszPSWExportCLSID, 0, KEY_ALL_ACCESS, &hDataKey);
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);


    while ((lRet = ::RegEnumKeyEx(hDataKey, 0, szKeyName, &dwKeyName, 0, szClassName, &dwClassName, NULL))
                != ERROR_NO_MORE_ITEMS)
    {
        lRet = ::RegDeleteKey(hDataKey, szKeyName);

        ::lstrcpy(szKeyName, "\0");
        ::lstrcpy(szClassName, "\0");

        dwClassName = _MAX_PATH;
        dwKeyName   = _MAX_PATH;
    }

    ::RegCloseKey(hDataKey);  hDataKey = NULL;

    lRet = ::RegDeleteKey(hKey, CXMergeFilter::m_pszPSWExportCLSID);
    if (lRet != ERROR_SUCCESS && lRet != ERROR_FILE_NOT_FOUND)
        return _signalRegError(lRet, hKey, hDataKey);



    // Next, the Writer import filter
    lRet = ::RegOpenKeyEx(hKey, CXMergeFilter::m_pszPSWImportCLSID, 0, KEY_ALL_ACCESS, &hDataKey);
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);


    while ((lRet = ::RegEnumKeyEx(hDataKey, 0, szKeyName, &dwKeyName, 0, szClassName, &dwClassName, NULL))
                != ERROR_NO_MORE_ITEMS)
    {
        lRet = ::RegDeleteKey(hDataKey, szKeyName);

        ::lstrcpy(szKeyName, "\0");
        ::lstrcpy(szClassName, "\0");

        dwClassName = _MAX_PATH;
        dwKeyName   = _MAX_PATH;
    }

    ::RegCloseKey(hDataKey);  hDataKey = NULL;

    lRet = ::RegDeleteKey(hKey, CXMergeFilter::m_pszPSWImportCLSID);
    if (lRet != ERROR_SUCCESS && lRet != ERROR_FILE_NOT_FOUND)
        return _signalRegError(lRet, hKey, hDataKey);


    // Next up, the Calc export filter
    lRet = ::RegOpenKeyEx(hKey, CXMergeFilter::m_pszPXLExportCLSID, 0, KEY_ALL_ACCESS, &hDataKey);
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);


    while ((lRet = ::RegEnumKeyEx(hDataKey, 0, szKeyName, &dwKeyName, 0, szClassName, &dwClassName, NULL))
                != ERROR_NO_MORE_ITEMS)
    {
        lRet = ::RegDeleteKey(hDataKey, szKeyName);

        ::lstrcpy(szKeyName, "\0");
        ::lstrcpy(szClassName, "\0");

        dwClassName = _MAX_PATH;
        dwKeyName   = _MAX_PATH;
    }

    ::RegCloseKey(hDataKey);  hDataKey = NULL;

    lRet = ::RegDeleteKey(hKey, CXMergeFilter::m_pszPXLExportCLSID);
    if (lRet != ERROR_SUCCESS && lRet != ERROR_FILE_NOT_FOUND)
        return _signalRegError(lRet, hKey, hDataKey);


    // Next, the Calc import filter
    lRet = ::RegOpenKeyEx(hKey, CXMergeFilter::m_pszPXLImportCLSID, 0, KEY_ALL_ACCESS, &hDataKey);
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);


    while ((lRet = ::RegEnumKeyEx(hDataKey, 0, szKeyName, &dwKeyName, 0, szClassName, &dwClassName, NULL))
                != ERROR_NO_MORE_ITEMS)
    {
        lRet = ::RegDeleteKey(hDataKey, szKeyName);

        ::lstrcpy(szKeyName, "\0");
        ::lstrcpy(szClassName, "\0");

        dwClassName = _MAX_PATH;
        dwKeyName   = _MAX_PATH;
    }

    ::RegCloseKey(hDataKey);  hDataKey = NULL;

    lRet = ::RegDeleteKey(hKey, CXMergeFilter::m_pszPXLImportCLSID);
    if (lRet != ERROR_SUCCESS && lRet != ERROR_FILE_NOT_FOUND)
        return _signalRegError(lRet, hKey, hDataKey);

    ::RegCloseKey(hKey);  hKey = NULL;
        


    /*
     * Remove the HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows CE Services\Filters
     */
    lRet = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows CE Services\\Filters"), 
                            0, KEY_ALL_ACCESS, &hKey);
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);


    // Remove the Writer export filter from the Writer import file extension subkey.
    _snprintf(szKeyName, _MAX_PATH, ".%s\\InstalledFilters", CXMergeFilter::m_pszPSWImportExt);
    lRet = ::RegOpenKeyEx(hKey, _T(szKeyName), 0, KEY_ALL_ACCESS, &hDataKey);
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegDeleteValue(hDataKey, CXMergeFilter::m_pszPSWExportCLSID);
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);
    
    ::lstrcpyn(szKeyName, "\0", _MAX_PATH);
    ::RegCloseKey(hDataKey);	hDataKey = NULL;


    // Remove the Writer import filter from the Writer export file extension subkey.
    _snprintf(szKeyName, _MAX_PATH, ".%s\\InstalledFilters", CXMergeFilter::m_pszPSWExportExt);
    lRet = ::RegOpenKeyEx(hKey, _T(szKeyName), 0, KEY_ALL_ACCESS, &hDataKey);
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegDeleteValue(hDataKey, CXMergeFilter::m_pszPSWImportCLSID);
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);

    ::lstrcpyn(szKeyName, "\0", _MAX_PATH);
    ::RegCloseKey(hDataKey);	hDataKey = NULL;


    // Make Binary Copy the default for Writer export file extension subkey DefaultImport
    _snprintf(szKeyName, _MAX_PATH, ".%s\0", CXMergeFilter::m_pszPSWExportExt);
    lRet = ::RegOpenKeyEx(hKey, _T(szKeyName), 0, KEY_ALL_ACCESS, &hDataKey);
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegSetValueEx(hDataKey, _T("DefaultImport"), 0, REG_SZ, (LPBYTE)_T("Binary Copy"),
                            (::_tcslen(_T("Binary Copy")) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);

    ::lstrcpyn(szKeyName, "\0", _MAX_PATH);
    ::RegCloseKey(hDataKey);	hDataKey = NULL;


    // Remove the Calc export filter from the Calc import file extension subkey.
    _snprintf(szKeyName, _MAX_PATH, ".%s\\InstalledFilters", CXMergeFilter::m_pszPXLImportExt);
    lRet = ::RegOpenKeyEx(hKey, _T(szKeyName), 0, KEY_ALL_ACCESS, &hDataKey);
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegDeleteValue(hDataKey, CXMergeFilter::m_pszPXLExportCLSID);
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);
    
    ::lstrcpyn(szKeyName, "\0", _MAX_PATH);
    ::RegCloseKey(hDataKey);	hDataKey = NULL;

    // Remove the Calc import filter from the Calc export file extension subkey.
    _snprintf(szKeyName, _MAX_PATH, ".%s\\InstalledFilters", CXMergeFilter::m_pszPXLExportExt);
    lRet = ::RegOpenKeyEx(hKey, _T(szKeyName), 0, KEY_ALL_ACCESS, &hDataKey);
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegDeleteValue(hDataKey, CXMergeFilter::m_pszPXLImportCLSID);
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);
    
    ::lstrcpyn(szKeyName, "\0", _MAX_PATH);
    ::RegCloseKey(hDataKey);	hDataKey = NULL;


    // Make Binary Copy the default for Calc export file extension subkey DefaultImport
    _snprintf(szKeyName, _MAX_PATH, ".%s\0", CXMergeFilter::m_pszPXLExportExt);
    lRet = ::RegOpenKeyEx(hKey, _T(szKeyName), 0, KEY_ALL_ACCESS, &hDataKey);
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);

    lRet = ::RegSetValueEx(hDataKey, _T("DefaultImport"), 0, REG_SZ, (LPBYTE)_T("Binary Copy"),
                            (::_tcslen(_T("Binary Copy")) * sizeof(TCHAR) + (1 * sizeof(TCHAR))));
    if (lRet != ERROR_SUCCESS)
        return _signalRegError(lRet, hKey, hDataKey);

    ::lstrcpyn(szKeyName, "\0", _MAX_PATH);
    ::RegCloseKey(hDataKey);	hDataKey = NULL;

    

    ::RegCloseKey(hKey);		hKey	 = NULL;
    
    return HRESULT_FROM_WIN32(lRet);
} 


//////////////////////////////////////////////////////////////////////
// CXMergeSyncModule methods
//////////////////////////////////////////////////////////////////////
CXMergeSyncModule::CXMergeSyncModule () 
{
}

CXMergeSyncModule::~CXMergeSyncModule () 
{
}

long CXMergeSyncModule::LockServer(BOOL fLock)
{
    if(fLock)
        return ::InterlockedIncrement(&m_lLocks);
    else
        return ::InterlockedDecrement(&m_lLocks);
}

long CXMergeSyncModule::GetLockCount()
{
    return m_lLocks + m_lObjs;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
