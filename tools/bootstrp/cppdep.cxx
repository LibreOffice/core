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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include <sys/stat.h>
#include <tools/stream.hxx>
#include "cppdep.hxx"

CppDep::CppDep( ByteString aFileName )
{
    aSourceFile = aFileName;

    pSearchPath = new ByteStringList;
    pFileList = new ByteStringList;
}

CppDep::CppDep()
{
    pSources = new ByteStringList;
    pSearchPath = new ByteStringList;
    pFileList = new ByteStringList;
}

CppDep::~CppDep()
{
    for ( size_t i = 0, n = pSources->size(); i < n; ++i ) {
        delete (*pSources)[ i ];
    }
    delete pSources;

    for ( size_t i = 0, n = pSearchPath->size(); i < n; ++i ) {
        delete (*pSearchPath)[ i ];
    }
    delete pSearchPath;

    for ( size_t i = 0, n = pFileList->size(); i < n; ++i ) {
        delete (*pFileList)[ i ];
    }
    delete pFileList;
}

void CppDep::Execute()
{
    size_t nCount = pSources->size();
    for ( size_t n = 0; n < nCount; n++ )
    {
        ByteString *pStr = (*pSources)[ n ];
        Search( *pStr );
    }
}

BOOL CppDep::AddSearchPath( const char* aPath )
{
    ByteString *pStr = new ByteString( aPath );
    pSearchPath->push_back( pStr );
    return FALSE;
}

BOOL CppDep::AddSource( const char* aSource )
{
    ByteString *pStr = new ByteString( aSource );
    pSources->push_back( pStr );
    return FALSE;
}

BOOL CppDep::Search( ByteString aFileName )
{
#ifdef DEBUG_VERBOSE
    fprintf( stderr, "SEARCH : %s\n", aFileName.GetBuffer());
#endif
    BOOL bRet = FALSE;

    SvFileStream aFile;
    ByteString aReadLine;

    UniString suFileName( aFileName, gsl_getSystemTextEncoding());

    aFile.Open( suFileName, STREAM_READ );
    while ( aFile.ReadLine( aReadLine ))
    {
        USHORT nPos = aReadLine.Search( "include" );
        if ( nPos != STRING_NOTFOUND  )
        {
#ifdef DEBUG_VERBOSE
            fprintf( stderr, "found : %d %s\n", nPos, aReadLine.GetBuffer() );
#endif
            ByteString aResult = IsIncludeStatement( aReadLine );
#ifdef DEBUG_VERBOSE
            fprintf( stderr, "Result : %s\n", aResult.GetBuffer() );
#endif

            ByteString aNewFile;
            if ( aResult !="")
            if ( (aNewFile = Exists( aResult )) != "" )
            {
                BOOL bFound = FALSE;
                size_t nCount = pFileList->size();
                for ( size_t i = 0; i < nCount; i++ )
                {
                    ByteString *pStr = (*pFileList)[ i ];
                    if ( *pStr == aNewFile )
                        bFound = TRUE;
                }
#ifdef DEBUG_VERBOSE
                fprintf( stderr, "not in list : %d %s\n", nPos, aReadLine.GetBuffer() );
#endif
                if ( !bFound )
                {
                    pFileList->push_back( new ByteString( aNewFile ) );
#ifdef DEBUG_VERBOSE
                    fprintf( stderr, " CppDep %s\\\n", aNewFile.GetBuffer() );
#endif
                    Search(aNewFile);
                }
            }
        }
    }
    aFile.Close();

    return bRet;
}

ByteString CppDep::Exists( ByteString aFileName )
{
    char pFullName[1023];
    ByteString aString;

#ifdef DEBUG_VERBOSE
    fprintf( stderr, "Searching %s \n", aFileName.GetBuffer() );
#endif

    size_t nCount = pSearchPath->size();
    for ( size_t n = 0; n < nCount; n++ )
    {
        struct stat aBuf;
        ByteString *pPathName = (*pSearchPath)[ n ];

        strcpy( pFullName, pPathName->GetBuffer());
        strcat( pFullName, DIR_SEP );
        strcat( pFullName, aFileName.GetBuffer());

#ifdef DEBUG_VERBOSE
        fprintf( stderr, "looking for %s\t ", pFullName );
#endif
        if ( stat( pFullName, &aBuf ) == 0 )
        {
#ifdef DEBUG_VERBOSE
            fprintf( stderr, "Got Dependency ", pFullName );
#endif
#ifdef DEBUG_VERBOSE
            fprintf( stderr, "%s \\\n", pFullName );
#endif

            return ByteString(pFullName);
        }
    }
    return aString;
}

ByteString CppDep::IsIncludeStatement( ByteString aLine )
{
    ByteString aRetStr;
    if ( aLine.Search("/*",0) != STRING_NOTFOUND )
    {
#ifdef DEBUG_VERBOSE
        fprintf( stderr, "found starting C comment : %s\n", aLine.GetBuffer() );
#endif
        aLine.Erase(aLine.Search("/*",0), aLine.Len() - 1);
#ifdef DEBUG_VERBOSE
        fprintf( stderr, "cleaned string : %s\n", aLine.GetBuffer() );
#endif
    }
    if ( aLine.Search("//",0) != STRING_NOTFOUND )
    {
#ifdef DEBUG_VERBOSE
        fprintf( stderr, "found C++ comment : %s\n", aLine.GetBuffer() );
#endif
        aLine.Erase(aLine.Search("//",0), aLine.Len() - 1);
#ifdef DEBUG_VERBOSE
        fprintf( stderr, "cleaned string : %s\n", aLine.GetBuffer() );
#endif
    }
    // WhiteSpacesfressen
    aLine.EraseAllChars(' ');
    aLine.EraseAllChars('\t');
#ifdef DEBUG_VERBOSE
    fprintf( stderr, "now : %s\n", aLine.GetBuffer() );
#endif
    // ist der erste Teil ein #include ?
    ByteString aTmpStr;
    aTmpStr = aLine.Copy( 0, 8 );
#ifdef DEBUG_VERBOSE
    fprintf( stderr, "is include : %s\n", aTmpStr.GetBuffer() );
#endif
    if ( aTmpStr.Equals("#include") )
    {
        aTmpStr = aLine.Erase( 0, 8 );
        USHORT nLen = aLine.Len();
        aLine.Erase( nLen-1, 1 );
        aLine.Erase( 0, 1 );
#ifdef DEBUG_VERBOSE
        fprintf( stderr, "Gotcha : %s\n", aLine.GetBuffer() );
#endif
        aRetStr = aLine;
    }
    return aRetStr;
}

#ifdef TEST

int main( int argc, char **argv )
{
    CppDep *pDep = new CppDep( "cppdep.cxx" );
    pDep->AddSearchPath(".");
    pDep->AddSearchPath("/usr/include");
    pDep->AddSearchPath("/usr/local/include");
    pDep->AddSearchPath("/usr/include/sys");
    pDep->AddSearchPath("/usr/include/X11");
    pDep->Execute();
    delete pDep;
    return 0;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
