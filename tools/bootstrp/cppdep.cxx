/*************************************************************************
 *
 *  $RCSfile: cppdep.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2003-07-16 17:14:31 $
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

#include <stdio.h>
#include <string.h>

#ifdef WNT
#ifdef BOOTSTRAPPER
#include <glibc/posix/getopt.h>
#else
#include <glibc/getopt.h>
#endif
#endif

#ifdef UNX
#ifdef MACOSX
#if ( __GNUC__ < 3 )
#ifdef BOOTSTRAPPER
#include <glibc/posix/getopt.h>
#else
#include <glibc/getopt.h>
#endif
#endif
#endif
#include <unistd.h>
#endif

#include <sys/stat.h>
#include <stream.hxx>
#include "cppdep.hxx"

//#define TEST

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
    delete pSearchPath;
    delete pFileList;
}

void CppDep::Execute()
{
    ULONG nCount = pSources->Count();
    for ( ULONG n=0; n<nCount;n++)
    {
        ByteString *pStr = pSources->GetObject(n);
        Search( *pStr );
    }
}

BOOL CppDep::AddSearchPath( const char* aPath )
{
    ByteString *pStr = new ByteString( aPath );
    pSearchPath->Insert( pStr, LIST_APPEND );
    return FALSE;
}

BOOL CppDep::AddSource( const char* aSource )
{
    ByteString *pStr = new ByteString( aSource );
    pSources->Insert( pStr, LIST_APPEND );
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
        USHORT nPos;
        if ( nPos = aReadLine.Search( "include" ) != STRING_NOTFOUND  )
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
                ULONG nCount = pFileList->Count();
                for ( ULONG i=0; i<nCount; i++ )
                {
                    ByteString *pStr = pFileList->GetObject(i);
                    if ( *pStr == aNewFile )
                        bFound = TRUE;
                }
#ifdef DEBUG_VERBOSE
                fprintf( stderr, "not in list : %d %s\n", nPos, aReadLine.GetBuffer() );
#endif
                if ( !bFound )
                {
                    pFileList->Insert( new ByteString( aNewFile ), LIST_APPEND );
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

    ULONG nCount = pSearchPath->Count();
    for ( ULONG n=0; n<nCount; n++)
    {
        struct stat aBuf;
        ByteString *pPathName = pSearchPath->GetObject(n);

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
