/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: checkversion.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2007-09-06 13:36:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2005 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
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


