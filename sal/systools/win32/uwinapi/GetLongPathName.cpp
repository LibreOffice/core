/*************************************************************************
 *
 *  $RCSfile: GetLongPathName.cpp,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hro $ $Date: 2002-08-26 13:43:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
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
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

