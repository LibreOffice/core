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


#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <tools/stream.hxx>
#include <rtl/strbuf.hxx>
#include "cppdep.hxx"

CppDep::CppDep()
{
}

CppDep::~CppDep()
{
}

void CppDep::Execute()
{
    size_t nCount = m_aSources.size();
    for ( size_t n = 0; n < nCount; ++n )
    {
        const rtl::OString &rStr = m_aSources[n];
        Search(rStr);
    }
}

sal_Bool CppDep::AddSearchPath( const char* pPath )
{
    m_aSearchPath.push_back( rtl::OString(pPath) );
    return sal_False;
}

sal_Bool CppDep::AddSource( const char* pSource )
{
    m_aSources.push_back( rtl::OString(pSource) );
    return sal_False;
}

sal_Bool CppDep::Search(const rtl::OString &rFileName)
{
#ifdef DEBUG_VERBOSE
    fprintf( stderr, "SEARCH : %s\n", rFileName.getStr());
#endif
    sal_Bool bRet = sal_False;

    SvFileStream aFile;
    rtl::OString aReadLine;

    rtl::OUString suFileName(rtl::OStringToOUString(rFileName, osl_getThreadTextEncoding()));

    aFile.Open( suFileName, STREAM_READ );
    while ( aFile.ReadLine( aReadLine ))
    {
        sal_Int32 nPos = aReadLine.indexOfL(
            RTL_CONSTASCII_STRINGPARAM("include"));
        if ( nPos != -1 )
        {
#ifdef DEBUG_VERBOSE
            fprintf( stderr, "found : %d %s\n", nPos, aReadLine.getStr() );
#endif
            rtl::OString aResult = IsIncludeStatement( aReadLine );
#ifdef DEBUG_VERBOSE
            fprintf( stderr, "Result : %s\n", aResult.getStr() );
#endif

            rtl::OString aNewFile;
            if (!aResult.isEmpty())
            if ( !(aNewFile = Exists( aResult )).isEmpty() )
            {
                sal_Bool bFound = sal_False;
                size_t nCount = m_aFileList.size();
                for ( size_t i = 0; i < nCount; ++i )
                {
                    const rtl::OString &rStr = m_aFileList[i];
                    if ( rStr == aNewFile )
                        bFound = sal_True;
                }
#ifdef DEBUG_VERBOSE
                fprintf( stderr, "not in list : %d %s\n", nPos, aReadLine.getStr() );
#endif
                if ( !bFound )
                {
                    m_aFileList.push_back(aNewFile);
#ifdef DEBUG_VERBOSE
                    fprintf( stderr, " CppDep %s\\\n", aNewFile.getStr() );
#endif
                    Search(aNewFile);
                }
            }
        }
    }
    aFile.Close();

    return bRet;
}

rtl::OString CppDep::Exists(const rtl::OString &rFileName)
{
    char pFullName[1023];

#ifdef DEBUG_VERBOSE
    fprintf( stderr, "Searching %s \n", rFileName.getStr() );
#endif

    size_t nCount = m_aSearchPath.size();
    for ( size_t n = 0; n < nCount; ++n )
    {
        struct stat aBuf;
        const rtl::OString &rPathName = m_aSearchPath[n];

        strcpy( pFullName, rPathName.getStr());
        strcat( pFullName, DIR_SEP );
        strcat( pFullName, rFileName.getStr());

#ifdef DEBUG_VERBOSE
        fprintf( stderr, "looking for %s\t ", pFullName );
#endif
        if ( stat( pFullName, &aBuf ) == 0 )
        {
#ifdef DEBUG_VERBOSE
            fprintf( stderr, "Got Dependency %s \\\n", pFullName );
#endif
            return rtl::OString(pFullName);
        }
    }
    return rtl::OString();
}

rtl::OString CppDep::IsIncludeStatement(rtl::OString aLine)
{
    sal_Int32 nIndex;

    nIndex = aLine.indexOf("/*");
    if ( nIndex != -1 )
    {
#ifdef DEBUG_VERBOSE
        fprintf( stderr, "found starting C comment : %s\n", aLine.getStr() );
#endif
        aLine = aLine.copy(0, nIndex);
#ifdef DEBUG_VERBOSE
        fprintf( stderr, "cleaned string : %s\n", aLine.getStr() );
#endif
    }

    nIndex = aLine.indexOf("//");
    if ( nIndex != -1 )
    {
#ifdef DEBUG_VERBOSE
        fprintf( stderr, "found C++ comment : %s\n", aLine.getStr() );
#endif
        aLine = aLine.copy(0, nIndex);
#ifdef DEBUG_VERBOSE
        fprintf( stderr, "cleaned string : %s\n", aLine.getStr() );
#endif
    }
    // WhiteSpacesfressen
    aLine = aLine.replaceAll(rtl::OString(' '), rtl::OString()).
        replaceAll(rtl::OString('\t'), rtl::OString());
#ifdef DEBUG_VERBOSE
    fprintf( stderr, "now : %s\n", aLine.getStr() );
#endif
    // ist der erste Teil ein #include ?
    rtl::OString aRetStr;
    if (
        aLine.getLength() >= 10 &&
        aLine.match(rtl::OString(RTL_CONSTASCII_STRINGPARAM("#include")))
       )
    {
        //#include<foo> or #include"foo"
        aLine = aLine.copy(9, aLine.getLength()-10);
#ifdef DEBUG_VERBOSE
        fprintf( stderr, "Gotcha : %s\n", aLine.getStr() );
#endif
        aRetStr = aLine;
    }
    return aRetStr;
}

#ifdef TEST

int main( int argc, char **argv )
{
    CppDep *pDep = new CppDep();
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
