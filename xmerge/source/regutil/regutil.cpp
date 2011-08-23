/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Simple Application which calls the DllRegisterServer or DllUnregisterServer functions
 * of the XMerge ActiveSync plugin.
 */


#include <stdio.h>
#include <string.h>
#include <windows.h>


typedef HRESULT (STDAPICALLTYPE *DLLREGISTERSERVER)(void);
typedef HRESULT (STDAPICALLTYPE *DLLUNREGISTERSERVER)(void);

int main(int argc, char* argv[])
{
    BOOL  bUninstall = FALSE;
    int   nPathIndex = 1;

    if (argc < 2 || argc > 3)
    {
        printf("\nUsage:  regutil [/u] <Full Path of XMergeSync.dll>\n\n");
        return -1;
    }

    
    if (argc == 3)
    {
        if (strcmp("/u", argv[1]))
        {
            printf("\nUnrecognised option: %s\n", argv[1]); 
            return -1;
        }

        bUninstall = TRUE;
        nPathIndex = 2;
    }

    
    // Dynamically load the library
    HMODULE hmXMDll = LoadLibrary(argv[nPathIndex]);

    if (hmXMDll == NULL) 
    {
        printf("\nUnable to load the library %s\n", argv[nPathIndex]);
        return -1;
    }

    
    // Get an offset to the either the DllRegisterServer or DllUnregisterServer functions
    if (!bUninstall) 
    {
        printf("\nRegistering %s ... ", argv[nPathIndex]);

        DLLREGISTERSERVER DllRegisterServer = (DLLREGISTERSERVER)GetProcAddress(hmXMDll, "DllRegisterServer");
        
        if (DllRegisterServer == NULL)
        {
            printf("failed.\n\nDllRegisterServer is not present in library.\n");
            return -1;
        }	

        // Now call the procedure ...
        HRESULT regResult = DllRegisterServer() ;

        if (regResult != S_OK) 
        {
            printf("failed.\n");
            return -1;
        }
    }
    else
    {
        printf("\nUnregistering %s ... ", argv[nPathIndex]);

        DLLUNREGISTERSERVER DllUnregisterServer = (DLLUNREGISTERSERVER)GetProcAddress(hmXMDll, "DllUnregisterServer");

        if (DllUnregisterServer == NULL)
        {
            printf("failed.\n\nDllUnregisterServer is not present in library.\n");
            return -1;
        }	

        // Now call the procedure ...
        HRESULT regResult = DllUnregisterServer() ;

        if (regResult != S_OK) 
        {
            printf("failed.\n");
            return -1;
        }

    }

    printf("done.\n");


    // Clean up
    FreeLibrary(hmXMDll);
    
    return 0;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
