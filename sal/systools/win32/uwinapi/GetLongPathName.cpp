
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

    if ( lstrlen( lpShortPath ) == 2 && lpShortPath[1] == ':' )
    {
        lstrcpy( lpLongPath, lpShortPath );
        dwResult = lstrlen( lpLongPath );
    }
    else
    {
        HANDLE          hFind;
        WIN32_FIND_DATA aFindFileData;

        if ( lpShortPath[lstrlen(lpShortPath)-1] == '\\' )
        {
            TCHAR   szFilePath[MAX_PATH];

            lstrcpy( szFilePath, lpShortPath );
            lstrcat( szFilePath, TEXT("*.*") );
            hFind = FindFirstFile( szFilePath, &aFindFileData );;
            aFindFileData.cFileName[0] = 0;
        }
        else
        {
            hFind = FindFirstFile( lpShortPath, &aFindFileData );
            if ( !IsValidHandle( hFind ) )
            {
                TCHAR   szFilePath[MAX_PATH];

                lstrcpy( szFilePath, lpShortPath );
                lstrcat( szFilePath, TEXT("\\*.*") );
                hFind = FindFirstFile( szFilePath, &aFindFileData );;
                aFindFileData.cFileName[0] = 0;
            }
        }

        if ( IsValidHandle( hFind ) )
        {
            FindClose( hFind );

            LPCTSTR lpLastSlash = lstrrchr( lpShortPath, '\\' );

            if ( lpLastSlash )
            {
                int nParentLen = lpLastSlash - lpShortPath;
                LPTSTR  lpParentPath = (LPTSTR)_alloca( (nParentLen + 1) * sizeof(TCHAR) );

                CopyMemory( lpParentPath, lpShortPath, nParentLen * sizeof(TCHAR) );
                lpParentPath[nParentLen] = 0;

                dwResult = GetLongPathName( lpParentPath, lpLongPath, cchBuffer );

                if ( !dwResult )
                    lstrcpy( lpLongPath, lpParentPath );
            }
            else
            {
                lstrcpy( lpLongPath, lpShortPath );
                dwResult = lstrlen( lpLongPath );
            }

            if ( dwResult < cchBuffer )
            {
                lstrcat( lpLongPath, TEXT("\\") );
                lstrcat( lpLongPath, aFindFileData.cFileName );
                dwResult = lstrlen( lpLongPath );
            }
            else
                dwResult += lstrlen( aFindFileData.cFileName ) + 1;
        }
    }

    return dwResult;
}

