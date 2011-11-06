/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include <sys/stat.h>
#include <tools/stream.hxx>
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
    delete pSources;
    delete pSearchPath;
    delete pFileList;
}

void CppDep::Execute()
{
    sal_uIntPtr nCount = pSources->Count();
    for ( sal_uIntPtr n=0; n<nCount;n++)
    {
        ByteString *pStr = pSources->GetObject(n);
        Search( *pStr );
    }
}

sal_Bool CppDep::AddSearchPath( const char* aPath )
{
    ByteString *pStr = new ByteString( aPath );
    pSearchPath->Insert( pStr, LIST_APPEND );
    return sal_False;
}

sal_Bool CppDep::AddSource( const char* aSource )
{
    ByteString *pStr = new ByteString( aSource );
    pSources->Insert( pStr, LIST_APPEND );
    return sal_False;
}

sal_Bool CppDep::Search( ByteString aFileName )
{
#ifdef DEBUG_VERBOSE
    fprintf( stderr, "SEARCH : %s\n", aFileName.GetBuffer());
#endif
    sal_Bool bRet = sal_False;

    SvFileStream aFile;
    ByteString aReadLine;

    UniString suFileName( aFileName, gsl_getSystemTextEncoding());

    aFile.Open( suFileName, STREAM_READ );
    while ( aFile.ReadLine( aReadLine ))
    {
        sal_uInt16 nPos = aReadLine.Search( "include" );
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
                sal_Bool bFound = sal_False;
                sal_uIntPtr nCount = pFileList->Count();
                for ( sal_uIntPtr i=0; i<nCount; i++ )
                {
                    ByteString *pStr = pFileList->GetObject(i);
                    if ( *pStr == aNewFile )
                        bFound = sal_True;
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

    sal_uIntPtr nCount = pSearchPath->Count();
    for ( sal_uIntPtr n=0; n<nCount; n++)
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
        sal_uInt16 nLen = aLine.Len();
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
