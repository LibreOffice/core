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
#ifdef UNX
#include <unistd.h>
#endif

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include "bootstrp/prj.hxx"
#include "sal/main.h"

#include <tools/string.hxx>
#include <tools/fsys.hxx>
#include <tools/stream.hxx>

#include "cppdep.hxx"

#if defined WNT
#if !defined HAVE_GETOPT
#define __STDC__ 1
#define __GNU_LIBRARY__
#include <external/glibc/getopt.h>
#else
#include <getopt.h>
#endif
#endif

class RscHrcDep : public CppDep
{
public:
                    RscHrcDep();
    virtual         ~RscHrcDep();

    virtual void    Execute();
};

RscHrcDep::RscHrcDep()  :
    CppDep()
{
}

RscHrcDep::~RscHrcDep()
{
}

void RscHrcDep::Execute()
{
    CppDep::Execute();
}

SAL_IMPLEMENT_MAIN_WITH_ARGS( argc, argv )
{
    int c;
    char aBuf[255];
    char pFileNamePrefix[255];
    char pOutputFileName[255];
    char pSrsFileName[255];
    String aSrsBaseName;
    BOOL bSource = FALSE;
    ByteString aRespArg;
//  who needs anything but '/' ?
//  String aDelim = String(DirEntry::GetAccessDelimiter());
    String aDelim = '/';

    RscHrcDep *pDep = new RscHrcDep;

    pOutputFileName[0] = 0;
    pSrsFileName[0] = 0;

    for ( int i=1; i<argc; i++)
    {
        strcpy( aBuf, (const char *)argv[i] );
        if ( aBuf[0] == '-' && aBuf[1] == 'p' && aBuf[2] == '=' )
        {
            strcpy(pFileNamePrefix, &aBuf[3]);
        }
        if ( aBuf[0] == '-' && aBuf[1] == 'f' && aBuf[2] == 'o' && aBuf[3] == '=' )
        {
            strcpy(pOutputFileName, &aBuf[4]);
        }
        if ( aBuf[0] == '-' && aBuf[1] == 'f' && aBuf[2] == 'p' && aBuf[3] == '=' )
        {
            strcpy(pSrsFileName, &aBuf[4]);
            String aName( pSrsFileName, gsl_getSystemTextEncoding());
            DirEntry aDest( aName );
            aSrsBaseName = aDest.GetBase();
        }
        if (aBuf[0] == '-' &&  aBuf[1] == 'i' )
        {
            pDep->AddSearchPath( &aBuf[2] );
        }
        if (aBuf[0] == '-' &&  aBuf[1] == 'I' )
        {
            pDep->AddSearchPath( &aBuf[2] );
        }
        if (aBuf[0] == '@' )
        {
            ByteString aToken;
            String aRespName( &aBuf[1], gsl_getSystemTextEncoding());
            SimpleConfig aConfig( aRespName );
            while ( (aToken = aConfig.GetNext()) != "")
            {
                char aBuf2[255];
                (void) strcpy( aBuf2, aToken.GetBuffer());
                if ( aBuf[0] == '-' && aBuf[1] == 'p' && aBuf[2] == '=' )
                {
                    strcpy(pFileNamePrefix, &aBuf[3]);
                }
                if ( aBuf2[0] == '-' && aBuf2[1] == 'f' && aBuf2[2] == 'o' )
                {
                    strcpy(pOutputFileName, &aBuf2[3]);
                }
                if ( aBuf2[0] == '-' && aBuf2[1] == 'f' && aBuf2[2] == 'p' )
                {
                    strcpy(pSrsFileName, &aBuf2[3]);
                    String aName( pSrsFileName, gsl_getSystemTextEncoding());
                    DirEntry aDest( aName );
                    aSrsBaseName = aDest.GetBase();
                }
                if (aBuf2[0] == '-' &&  aBuf2[1] == 'i' )
                {
                    pDep->AddSearchPath( &aBuf2[2] );
                }
                if (aBuf2[0] == '-' &&  aBuf2[1] == 'I' )
                {
                    pDep->AddSearchPath( &aBuf2[2] );
                }
                if (( aBuf2[0] != '-' ) && ( aBuf2[0] != '@' ))
                {
                    pDep->AddSource( &aBuf2[0] );
                    aRespArg += " ";
                    aRespArg += &aBuf2[0];
                    bSource = TRUE;
                }
            }
        }
    }

    while( 1 )
    {
        c = getopt( argc, argv,
        "_abcdefghi:jklmnopqrstuvwxyzABCDEFGHI:JKLMNOPQRSTUVWXYZ1234567890/-+=.\\()\"");
        if ( c == -1 )
            break;

        switch( c )
        {
            case 0:
                break;
            case 'a' :
#ifdef DEBUG_VERBOSE
                printf("option a\n");
#endif
                break;

            case 'l' :
#ifdef DEBUG_VERBOSE
                printf("option l with Value %s\n", optarg );
#endif
                pDep->AddSource( optarg );
                break;

            case 'h' :
            case 'H' :
            case '?' :
                printf("RscDep 1.0 (c)2000 StarOffice\n");
                break;

            default:
#ifdef DEBUG_VERBOSE
                printf("Unknown getopt error\n");
#endif
                ;
        }
    }


    String aCwd(pFileNamePrefix, gsl_getSystemTextEncoding());
    SvFileStream aOutStream;
    String aOutputFileName( pOutputFileName, gsl_getSystemTextEncoding());
    DirEntry aOutEntry( aOutputFileName );
    String aOutPath = aOutEntry.GetPath().GetFull();

    String aFileName( aOutPath );
    aFileName += aDelim;
    aFileName += aCwd;
    aFileName += String(".", gsl_getSystemTextEncoding());
    aFileName += aSrsBaseName;
    aFileName += String(".dprr", gsl_getSystemTextEncoding());
    aOutStream.Open( aFileName, STREAM_WRITE );

    ByteString aString;
    if ( optind < argc )
    {
#ifdef DEBUG_VERBOSE
        printf("further arguments : ");
#endif
        aString = ByteString( pSrsFileName );
        aString.SearchAndReplaceAll('\\', ByteString( aDelim,  RTL_TEXTENCODING_ASCII_US ));
        aString += ByteString(" : " );

        while ( optind < argc )
        {
            if (!bSource )
            {
                aString += ByteString(" " );
                aString += ByteString( argv[optind]);
                pDep->AddSource( argv[optind++]);
            }
            else
            {
                optind++;
            }
        }
    }
    aString += aRespArg;
    pDep->Execute();
    ByteStringList *pLst = pDep->GetDepList();
    size_t nCount = pLst->size();
    if ( nCount == 0 )
    {
        aOutStream.WriteLine( aString );
    }
    else
    {
        aString += ByteString( "\\" );
        aOutStream.WriteLine( aString );
    }

    for ( size_t j = 0; j < nCount; j++ )
    {
        ByteString *pStr = (*pLst)[ j ];
        pStr->SearchAndReplaceAll('\\', ByteString( aDelim,  RTL_TEXTENCODING_ASCII_US ));
        if ( j != (nCount-1) )
            *pStr += ByteString( "\\" );
        aOutStream.WriteLine( *pStr );
    }
    delete pDep;
    aOutStream.Close();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
