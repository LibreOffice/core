/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
