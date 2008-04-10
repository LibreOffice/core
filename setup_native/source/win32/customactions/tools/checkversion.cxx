/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: checkversion.cxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#define UNICODE

#ifdef _MSC_VER
#pragma warning(push,1) // disable warnings within system headers
#endif
#include <windows.h>
#include <msiquery.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include "strsafe.h"

#include <seterror.hxx>

//----------------------------------------------------------
BOOL GetMsiProp( MSIHANDLE hMSI, const wchar_t* pPropName, wchar_t** ppValue )
{
    DWORD sz = 0;
       if ( MsiGetProperty( hMSI, pPropName, L"", &sz ) == ERROR_MORE_DATA )
       {
           sz++;
           DWORD nbytes = sz * sizeof( wchar_t );
           wchar_t* buff = reinterpret_cast<wchar_t*>( malloc( nbytes ) );
           ZeroMemory( buff, nbytes );
           MsiGetProperty( hMSI, pPropName, buff, &sz );
           *ppValue = buff;

        return TRUE;
    }

    return FALSE;
}

//----------------------------------------------------------
#ifdef DEBUG
inline void OutputDebugStringFormat( LPCTSTR pFormat, ... )
{
    TCHAR    buffer[1024];
    va_list  args;

    va_start( args, pFormat );
    StringCchVPrintf( buffer, sizeof(buffer), pFormat, args );
    OutputDebugString( buffer );
}
#else
static inline void OutputDebugStringFormat( LPCTSTR, ... )
{
}
#endif

//----------------------------------------------------------
extern "C" UINT __stdcall CheckVersions( MSIHANDLE hMSI )
{
    // MessageBox(NULL, L"CheckVersions", L"Information", MB_OK | MB_ICONINFORMATION);

    wchar_t* pVal = NULL;

    if ( GetMsiProp( hMSI, L"NEWPRODUCTS", &pVal ) && pVal )
    {
        OutputDebugStringFormat( TEXT("DEBUG: NEWPRODUCTS found [%s]"), pVal );
        if ( *pVal != 0 )
            SetMsiErrorCode( MSI_ERROR_NEW_VERSION_FOUND );
        free( pVal );
    }
    pVal = NULL;
    if ( GetMsiProp( hMSI, L"SAMEPRODUCTS", &pVal ) && pVal )
    {
        OutputDebugStringFormat( TEXT("DEBUG: SAMEPRODUCTS found [%s]"), pVal );
        if ( *pVal != 0 )
            SetMsiErrorCode( MSI_ERROR_SAME_VERSION_FOUND );
        free( pVal );
    }
    pVal = NULL;
    if ( GetMsiProp( hMSI, L"OLDPRODUCTS", &pVal ) && pVal )
    {
        OutputDebugStringFormat( TEXT("DEBUG: OLDPRODUCTS found [%s]"), pVal );
        if ( *pVal != 0 )
            SetMsiErrorCode( MSI_ERROR_OLD_VERSION_FOUND );
        free( pVal );
    }
    pVal = NULL;
    if ( GetMsiProp( hMSI, L"BETAPRODUCTS", &pVal ) && pVal )
    {
        OutputDebugStringFormat( TEXT("DEBUG: BETAPRODUCTS found [%s]"), pVal );
        if ( *pVal != 0 )
            SetMsiErrorCode( MSI_ERROR_OLD_VERSION_FOUND );
        free( pVal );
    }

    pVal = NULL;
    if ( GetMsiProp( hMSI, L"NEWPRODUCTSPATCH", &pVal ) && pVal )
    {
        OutputDebugStringFormat( TEXT("DEBUG: NEWPRODUCTSPATCH found [%s]"), pVal );
        if ( *pVal != 0 )
            SetMsiErrorCode( MSI_ERROR_NEW_PATCH_FOUND );
        free( pVal );
    }
    pVal = NULL;
    if ( GetMsiProp( hMSI, L"SAMEPRODUCTSPATCH", &pVal ) && pVal )
    {
        OutputDebugStringFormat( TEXT("DEBUG: SAMEPRODUCTSPATCH found [%s]"), pVal );
        if ( *pVal != 0 )
            SetMsiErrorCode( MSI_ERROR_SAME_PATCH_FOUND );
        free( pVal );
    }
    pVal = NULL;
    if ( GetMsiProp( hMSI, L"OLDPRODUCTSPATCH", &pVal ) && pVal )
    {
        OutputDebugStringFormat( TEXT("DEBUG: OLDPRODUCTSPATCH found [%s]"), pVal );
        if ( *pVal != 0 )
            SetMsiErrorCode( MSI_ERROR_OLD_PATCH_FOUND );
        free( pVal );
    }

    return ERROR_SUCCESS;
}


