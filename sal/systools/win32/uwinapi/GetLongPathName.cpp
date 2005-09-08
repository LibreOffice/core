/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GetLongPathName.cpp,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:17:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


{
    DWORD   dwResult = 0;   // Assume failure

    if ( IsBadStringPtr( lpShortPath, MAX_PATH ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return dwResult;
    }

    // Assume a not existing buffer means a bufsize of zero
    if ( !lpLongPath )
        cchBuffer = 0;

    if ( _tcslen( lpShortPath ) == 2 && lpShortPath[1] == ':' )
    {
        _tcscpy( lpLongPath, lpShortPath );
        dwResult = _tcslen( lpLongPath );
    }
    else
    {
        HANDLE          hFind;
        WIN32_FIND_DATA aFindFileData;

        if ( lpShortPath[_tcslen(lpShortPath)-1] == '\\' )
        {
            TCHAR   szFilePath[MAX_PATH];

            _tcscpy( szFilePath, lpShortPath );
            _tcscat( szFilePath, TEXT("*.*") );
            hFind = FindFirstFile( szFilePath, &aFindFileData );;
            aFindFileData.cFileName[0] = 0;
        }
        else
        {
            hFind = FindFirstFile( lpShortPath, &aFindFileData );
            if ( !IsValidHandle( hFind ) )
            {
                TCHAR   szFilePath[MAX_PATH];

                _tcscpy( szFilePath, lpShortPath );
                _tcscat( szFilePath, TEXT("\\*.*") );
                hFind = FindFirstFile( szFilePath, &aFindFileData );;
                aFindFileData.cFileName[0] = 0;
            }
        }

        if ( IsValidHandle( hFind ) )
        {
            FindClose( hFind );

            LPCTSTR lpLastSlash = _tcsrchr( lpShortPath, '\\' );

            if ( lpLastSlash )
            {
                int nParentLen = lpLastSlash - lpShortPath;
                LPTSTR  lpParentPath = (LPTSTR)_alloca( (nParentLen + 1) * sizeof(TCHAR) );

                CopyMemory( lpParentPath, lpShortPath, nParentLen * sizeof(TCHAR) );
                lpParentPath[nParentLen] = 0;

                dwResult = GetLongPathName( lpParentPath, lpLongPath, cchBuffer );

                if ( !dwResult )
                    _tcscpy( lpLongPath, lpParentPath );
            }
            else
            {
                _tcscpy( lpLongPath, lpShortPath );
                dwResult = _tcslen( lpLongPath );
            }

            if ( dwResult < cchBuffer )
            {
                _tcscat( lpLongPath, TEXT("\\") );
                _tcscat( lpLongPath, aFindFileData.cFileName );
                dwResult = _tcslen( lpLongPath );
            }
            else
                dwResult += _tcslen( aFindFileData.cFileName ) + 1;
        }
    }

    return dwResult;
}

