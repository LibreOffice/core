// so_activex.cpp : Implementation of DLL Exports.


// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL,
//      run nmake -f so_activexps.mk in the project directory.

#include "stdio.h"
#include "stdafx2.h"
#include "resource.h"
#include <initguid.h>
#include "so_activex.h"

#include "so_activex_i.c"
#include "SOActiveX.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_SOActiveX, CSOActiveX)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_SO_ACTIVEXLib);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;    // ok
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

#define SUPPORTED_EXT_NUM 10
const char* aFileExt[] = { ".sds", ".sda", ".sdd", ".sdc", ".sdw",
                           ".sxw", ".sxc", ".sxi", ".sxd", ".sxg" };
const char* aMimeType[] = { "application/vnd.stardivision.chart",
                          "application/vnd.stardivision.draw",
                          "application/vnd.stardivision.impress",
                          "application/vnd.stardivision.calc",
                          "application/vnd.staroffice.writer",

                          "application/vnd.sun.xml.writer",
                          "application/vnd.sun.xml.calc",
                          "application/vnd.sun.xml.impress",
                          "application/vnd.sun.xml.draw",
                          "application/vnd.sun.xml.writer.global" };


const char* aClassID = "{67F2A879-82D5-4A6D-8CC5-FFB3C114B69D}";
const char* aTypeLib = "{61FA3F13-8061-4796-B055-3697ED28CB38}";

// ISOComWindowPeer interface information
const char* aInterIDWinPeer = "{BF5D10F3-8A10-4A0B-B150-2B6AA2D7E118}";
const char* aProxyStubWinPeer = "{00020424-0000-0000-C000-000000000046}";

// ISODispatchInterceptor interface information
const char* aInterIDDispInt = "{9337694C-B27D-4384-95A4-9D8E0EABC9E5}";
const char* aProxyStubDispInt = "{00020424-0000-0000-C000-000000000046}";

const char* aLocalPrefix = "Software\\Classes\\";

BOOL createKey( HKEY hkey,
                const char* aKeyToCreate,
                const char* aValue = NULL,
                const char* aChildName = NULL,
                const char* aChildValue = NULL )
{
    HKEY hkey1;

    return ( ERROR_SUCCESS == RegCreateKey( hkey, aKeyToCreate, &hkey1 )
           && ( !aValue || ERROR_SUCCESS == RegSetValueEx( hkey1,
                                                              "",
                                                           0,
                                                           REG_SZ,
                                                           (const BYTE*)aValue,
                                                           strlen( aValue ) ) )
           && ( !aChildName || ERROR_SUCCESS == RegSetValueEx( hkey1,
                                                                  aChildName,
                                                               0,
                                                               REG_SZ,
                                                               (const BYTE*)aChildValue,
                                                               strlen( aChildValue ) ) )
           && ERROR_SUCCESS == RegCloseKey( hkey1 ) );

}

STDAPI DllRegisterServerNative( BOOL bForAllUsers )
{
    BOOL aResult = FALSE;

    HKEY        hkey = NULL;
    HKEY        hkey1 = NULL;
    HKEY        hkey2 = NULL;
    HKEY        hkey3 = NULL;
    HKEY        hkey4 = NULL;
    char        aSubKey[513];
    int         ind;
    const char* aPrefix = bForAllUsers ? "" : aLocalPrefix;

    // get iervp.dll path
    char aActiveXPath[1019];
    char aActiveXPath101[1024];
    char aPrCatalogPath[1019];

    HMODULE aCurModule = GetModuleHandleA( "so_activex.dll" );
    if( aCurModule && GetModuleFileNameA( aCurModule, aActiveXPath, 1019 ) )
    {
        sprintf( aActiveXPath101, "%s, 101", aActiveXPath );

        int nPrCatLength = strlen( aActiveXPath ) - 14;
        strncpy( aPrCatalogPath, aActiveXPath, nPrCatLength );
        aPrCatalogPath[ nPrCatLength ] = 0;

        if( aActiveXPath )
        {
               wsprintf( aSubKey, "%sCLSID\\%s", aPrefix, aClassID );
            aResult =
                ( ERROR_SUCCESS == RegCreateKey( bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aSubKey, &hkey )
                      && ERROR_SUCCESS == RegSetValueEx( hkey, "", 0, REG_SZ, (const BYTE*)"SOActiveX Class", 17 )
                    && createKey( hkey, "Control" )
                    && createKey( hkey, "EnableFullPage" )
                    && createKey( hkey, "InprocServer32", aActiveXPath, "ThreadingModel", "Apartment" )
                    && createKey( hkey, "MiscStatus", "0" )
                    && createKey( hkey, "MiscStatus\\1", "131473" )
                    && createKey( hkey, "ProgID", "so_activex.SOActiveX.1" )
                    && createKey( hkey, "Programmable" )
                    && createKey( hkey, "ToolboxBitmap32", aActiveXPath101 )
                    && createKey( hkey, "TypeLib", aTypeLib )
                    && createKey( hkey, "Version", "1.0" )
                    && createKey( hkey, "VersionIndependentProgID", "so_activex.SOActiveX" )
                && ERROR_SUCCESS == RegCloseKey( hkey )
                  && ERROR_SUCCESS == RegCreateKey( bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aPrefix, &hkey )
                    && createKey( hkey, "so_activex.SOActiveX", "SOActiveX Class" )
                      && ERROR_SUCCESS == RegCreateKey( hkey, "so_activex.SOActiveX", &hkey1 )
                        && createKey( hkey1, "CLSID", aClassID )
                        && createKey( hkey1, "CurVer", "so_activex.SOActiveX.1" )
                      && ERROR_SUCCESS == RegCloseKey( hkey1 )
                    && createKey( hkey, "so_activex.SOActiveX.1", "SOActiveX Class" )
                      && ERROR_SUCCESS == RegCreateKey( hkey, "so_activex.SOActiveX.1", &hkey1 )
                        && createKey( hkey1, "CLSID", aClassID )
                      && ERROR_SUCCESS == RegCloseKey( hkey1 )
                      && ERROR_SUCCESS == RegCreateKey( hkey, "TypeLib", &hkey1 )
                          && ERROR_SUCCESS == RegCreateKey( hkey1, aTypeLib, &hkey2 )
                            && createKey( hkey2, "1.0", "wrap_activex 1.0 Type Library" )
                              && ERROR_SUCCESS == RegCreateKey( hkey2, "1.0", &hkey3 )
                                  && ERROR_SUCCESS == RegCreateKey( hkey3, "0", &hkey4 )
                                    && createKey( hkey4, "win32", aActiveXPath )
                                  && ERROR_SUCCESS == RegCloseKey( hkey4 )
                                && createKey( hkey3, "FLAGS", "0" )
                                && createKey( hkey3, "HELPDIR", aPrCatalogPath )
                              && ERROR_SUCCESS == RegCloseKey( hkey3 )
                          && ERROR_SUCCESS == RegCloseKey( hkey2 )
                      && ERROR_SUCCESS == RegCloseKey( hkey1 )
                      && ERROR_SUCCESS == RegCreateKey( hkey, "Interface", &hkey1 )
                        && createKey( hkey1, aInterIDWinPeer, "ISOComWindowPeer" )
                          && ERROR_SUCCESS == RegCreateKey( hkey1, aInterIDWinPeer, &hkey2 )
                            && createKey( hkey2, "ProxyStubClsid", aProxyStubWinPeer )
                            && createKey( hkey2, "ProxyStubClsid32", aProxyStubWinPeer )
                            && createKey( hkey2, "TypeLib", aTypeLib, "Version", "1.0" )
                          && ERROR_SUCCESS == RegCloseKey( hkey2 )
                        && createKey( hkey1, aInterIDDispInt, "ISODispatchInterceptor" )
                          && ERROR_SUCCESS == RegCreateKey( hkey1, aInterIDDispInt, &hkey2 )
                            && createKey( hkey2, "ProxyStubClsid", aProxyStubDispInt )
                            && createKey( hkey2, "ProxyStubClsid32", aProxyStubDispInt )
                            && createKey( hkey2, "TypeLib", aTypeLib, "Version", "1.0" )
                          && ERROR_SUCCESS == RegCloseKey( hkey2 )
                      && ERROR_SUCCESS == RegCloseKey( hkey1 )
                  && ERROR_SUCCESS == RegCloseKey( hkey ) );
        }
    }

    for( ind = 0; ind < SUPPORTED_EXT_NUM && aResult; ind++ )
    {
        wsprintf( aSubKey, "%sMIME\\DataBase\\Content Type\\%s", aPrefix, aMimeType[ind] );
        if ( ERROR_SUCCESS != RegCreateKey(bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aSubKey, &hkey)
//          || ERROR_SUCCESS != RegSetValueEx(hkey, "Extension", 0, REG_SZ,
//             (const BYTE *)aFileExt[ind], strlen( aFileExt[ind] ) )
          || ERROR_SUCCESS != RegSetValueEx(hkey, "CLSID", 0, REG_SZ,
             (const BYTE *)aClassID, strlen(aClassID)) )
                aResult = FALSE;

        if( hkey )
            RegCloseKey(hkey);

/*
        wsprintf( aSubKey, "%s%s", aPrefix, aFileExt[ind] );
        if ( ERROR_SUCCESS != RegCreateKey(bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aSubKey, &hkey)
          || ERROR_SUCCESS != RegSetValueEx(hkey, "Content Type", 0, REG_SZ,
             (const BYTE *)aMimeType[ind], strlen( aMimeType[ind] ) ) )
                aResult = FALSE;
        if( hkey )
            RegCloseKey(hkey);
*/
    }


    wsprintf( aSubKey, "%sCLSID\\%s", aPrefix, aClassID );
    if ( aResult && ERROR_SUCCESS == RegOpenKey(bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aSubKey, &hkey) )
    {
        for( ind = 0; ind < SUPPORTED_EXT_NUM; ind++ )
        {
            wsprintf( aSubKey, "EnableFullPage\\%s", aFileExt[ind] );
            if ( ERROR_SUCCESS != RegCreateKey( hkey, aSubKey, &hkey1 ) )
                aResult = FALSE;

            if ( hkey1 )
                RegCloseKey(hkey1);
         }
    }
    else
        aResult = FALSE;

    if ( hkey )
        RegCloseKey(hkey);

    return aResult;
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServerNative( BOOL bForAllUsers )
{
    HKEY        hkey = NULL;
    BOOL        fErr = FALSE;
    char        aSubKey[513];
    HKEY        aIter[2] = { HKEY_CLASSES_ROOT, HKEY_CURRENT_USER };
    const char* aPrefix = bForAllUsers ? "" : aLocalPrefix;

      for( int ind = 0; ind < SUPPORTED_EXT_NUM; ind++ )
    {
        DWORD nSubKeys = 0, nValues = 0;
           wsprintf( aSubKey, "%sMIME\\DataBase\\Content Type\\%s", aPrefix, aMimeType[ind] );
           if ( ERROR_SUCCESS != RegCreateKey(bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aSubKey, &hkey) )
            fErr = TRUE;
        else
        {
               if ( ERROR_SUCCESS != RegDeleteValue( hkey, "CLSID" ) )
                fErr = TRUE;

               if ( ERROR_SUCCESS != RegQueryInfoKey(  hkey, NULL, NULL, NULL,
                                                &nSubKeys, NULL, NULL,
                                                &nValues, NULL, NULL, NULL, NULL ) )
            {
                RegCloseKey( hkey );
                fErr = TRUE;
            }
            else
            {
                RegCloseKey( hkey );
                if ( !nSubKeys && !nValues )
                    SHDeleteKey( bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aSubKey );
            }
        }

           wsprintf( aSubKey, "%s%s", aPrefix, aFileExt[ind] );
           if ( ERROR_SUCCESS != RegCreateKey(bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aSubKey, &hkey) )
            fErr = TRUE;
        else
        {
               if ( ERROR_SUCCESS != RegQueryInfoKey(  hkey, NULL, NULL, NULL,
                                                &nSubKeys, NULL, NULL,
                                                &nValues, NULL, NULL, NULL, NULL ) )
            {
                RegCloseKey( hkey );
                fErr = TRUE;
            }
            else
            {
                RegCloseKey( hkey );
                if ( !nSubKeys && !nValues )
                    SHDeleteKey( bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aSubKey );
            }
        }

    }

    wsprintf( aSubKey, "%sCLSID\\%s", aPrefix, aClassID );
    if( ERROR_SUCCESS != SHDeleteKey( bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aSubKey ) )
           fErr = TRUE;

    wsprintf( aSubKey, "%sso_activex.SOActiveX", aPrefix );
    if( ERROR_SUCCESS != SHDeleteKey( bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aSubKey ) )
           fErr = TRUE;

    wsprintf( aSubKey, "%sso_activex.SOActiveX.1", aPrefix );
    if( ERROR_SUCCESS != SHDeleteKey( bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aSubKey ) )
           fErr = TRUE;

    wsprintf( aSubKey, "%s\\TypeLib\\%s", aPrefix, aTypeLib );
    if( ERROR_SUCCESS != SHDeleteKey( bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aSubKey ) )
           fErr = TRUE;

    wsprintf( aSubKey, "%s\\Interface\\%s", aPrefix, aInterIDWinPeer );
    if( ERROR_SUCCESS != SHDeleteKey( bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aSubKey ) )
           fErr = TRUE;

    wsprintf( aSubKey, "%s\\Interface\\%s", aPrefix, aInterIDDispInt );
    if( ERROR_SUCCESS != SHDeleteKey( bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aSubKey ) )
           fErr = TRUE;

    return !fErr;
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServerDoc - Adds entries to the system registry

#define SUPPORTED_MSEXT_NUM 7
const char* aMSFileExt[] = { ".dot", ".doc", ".xlt", ".xls", ".pot", ".ppt", ".pps" };
const char* aMSMimeType[] = { "application/msword",
                          "application/msword",
                          "application/msexcell",
                          "application/msexcell",
                          "application/mspowerpoint",
                          "application/mspowerpoint",
                          "application/mspowerpoint" };
const int nForModes[] = { 1, 1, 2, 2, 4, 4, 4 };

STDAPI DllRegisterServerDoc( int nMode, BOOL bForAllUsers )
{
    BOOL aResult = TRUE;

    HKEY        hkey = NULL;
    HKEY        hkey1 = NULL;
    char        aSubKey[513];
    int         ind;
    const char* aPrefix = bForAllUsers ? "" : aLocalPrefix;

    for( ind = 0; ind < SUPPORTED_MSEXT_NUM && aResult; ind++ )
    {
        if( nForModes[ind] & nMode )
        {
               wsprintf( aSubKey, "%sMIME\\DataBase\\Content Type\\%s", aPrefix, aMSMimeType[ind] );
               if ( ERROR_SUCCESS != RegCreateKey(bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aSubKey, &hkey)
               || ERROR_SUCCESS != RegSetValueEx(hkey, "Extension", 0, REG_SZ,
                   (const BYTE *)aMSFileExt[ind], strlen( aMSFileExt[ind] ) )
               || ERROR_SUCCESS != RegSetValueEx(hkey, "CLSID", 0, REG_SZ,
                   (const BYTE *)aClassID, strlen(aClassID)) )
                       aResult = FALSE;

               if( hkey )
                   RegCloseKey(hkey);

               wsprintf( aSubKey, "%s%s", aPrefix, aMSFileExt[ind] );
               if ( ERROR_SUCCESS != RegCreateKey(bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aSubKey, &hkey)
               || ERROR_SUCCESS != RegSetValueEx(hkey, "Content Type", 0, REG_SZ,
                   (const BYTE *)aMSMimeType[ind], strlen( aMSMimeType[ind] ) ) )
                       aResult = FALSE;

               if( hkey )
                   RegCloseKey(hkey);
        }
    }

    wsprintf( aSubKey, "%sCLSID\\%s", aPrefix, aClassID );
    if ( aResult && ERROR_SUCCESS == RegCreateKey( bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aSubKey, &hkey )
      && createKey( hkey, "EnableFullPage" ) )
    {
           for( ind = 0; ind < SUPPORTED_MSEXT_NUM; ind++ )
           {
            if( nForModes[ind] & nMode )
            {
                   wsprintf( aSubKey, "EnableFullPage\\%s", aMSFileExt[ind] );
                   if ( ERROR_SUCCESS != RegCreateKey( hkey, aSubKey, &hkey1 ) )
                       aResult = FALSE;

                   if ( hkey1 )
                       RegCloseKey(hkey1);
            }
           }
    }
    else
        aResult = FALSE;

    if ( hkey )
        RegCloseKey(hkey);

    return aResult;
}



/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServerDoc - Removes entries from the system registry

STDAPI DllUnregisterServerDoc( int nMode, BOOL bForAllUsers )
{
    HKEY        hkey = NULL;
    BOOL        fErr = FALSE;
    char        aSubKey[513];
    HKEY        aIter[2] = { HKEY_CLASSES_ROOT, HKEY_CURRENT_USER };
    const char* aPrefix = bForAllUsers ? "" : aLocalPrefix;

      for( int ind = 0; ind < SUPPORTED_MSEXT_NUM; ind++ )
       {
        if( nForModes[ind] & nMode )
        {
            DWORD nSubKeys = 0, nValues = 0;

               wsprintf( aSubKey, "%sMIME\\DataBase\\Content Type\\%s", aPrefix, aMSMimeType[ind] );
               if ( ERROR_SUCCESS != RegCreateKey(bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aSubKey, &hkey) )
                fErr = TRUE;
            else
            {
                   if ( ERROR_SUCCESS != RegDeleteValue( hkey, "Extension" ) )
                    fErr = TRUE;

                   if ( ERROR_SUCCESS != RegDeleteValue( hkey, "CLSID" ) )
                    fErr = TRUE;

                if ( ERROR_SUCCESS != RegQueryInfoKey(  hkey, NULL, NULL, NULL,
                                                        &nSubKeys, NULL, NULL,
                                                        &nValues, NULL, NULL, NULL, NULL ) )
                {
                    RegCloseKey( hkey );
                    fErr = TRUE;
                }
                else
                {
                    RegCloseKey( hkey );
                    if ( !nSubKeys && !nValues )
                        SHDeleteKey( bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aSubKey );
                }
            }

               wsprintf( aSubKey, "%s%s", aPrefix, aMSFileExt[ind] );
               if ( ERROR_SUCCESS != RegCreateKey(bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aSubKey, &hkey) )
                fErr = TRUE;
            else
            {
                   if ( ERROR_SUCCESS != RegDeleteValue( hkey, "Content Type" ) )
                    fErr = TRUE;

                if ( ERROR_SUCCESS != RegQueryInfoKey(  hkey, NULL, NULL, NULL,
                                                        &nSubKeys, NULL, NULL,
                                                        &nValues, NULL, NULL, NULL, NULL ) )
                {
                    RegCloseKey( hkey );
                    fErr = TRUE;
                }
                else
                {
                    RegCloseKey( hkey );
                    if ( !nSubKeys && !nValues )
                        SHDeleteKey( bForAllUsers ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER, aSubKey );
                }
            }
        }
       }

    return !fErr;
}

STDAPI DllRegisterServer( void )
{
    HRESULT aResult = DllRegisterServerNative( TRUE );
    if( FAILED( aResult ) )
        aResult = DllRegisterServerNative( FALSE );

    return aResult;
}

STDAPI DllUnregisterServer( void )
{
    DllUnregisterServerNative( FALSE );
    return DllUnregisterServerNative( TRUE );
}

