/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifdef UNX
#include <unistd.h>
#endif

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include "bootstrp/prj.hxx"

#include <rtl/strbuf.hxx>
#include <tools/string.hxx>
#include <tools/fsys.hxx>
#include <tools/stream.hxx>

#include "cppdep.hxx"

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

int main( int argc, char** argv )
{
    char aBuf[255];
    char pFileNamePrefix[255];
    char pOutputFileName[255];
    char pSrsFileName[255];
    String aSrsBaseName;
    sal_Bool bSource = sal_False;
    rtl::OStringBuffer aRespArg;
//  who needs anything but '/' ?
    sal_Char cDelim = '/';

    RscHrcDep *pDep = new RscHrcDep;

    // When the options are processed, the non-option arguments are
    // collected at the head of the argv array.
    // nLastNonOption points to the last of them.
    int nLastNonOption (-1);

    pOutputFileName[0] = 0;
    pSrsFileName[0] = 0;

    for ( int i=1; i<argc; i++)
    {
        strncpy( aBuf, (const char *)argv[i], sizeof(aBuf) );
        const sal_Int32 nLength (strlen(aBuf));

#ifdef DEBUG
        printf("option %d is [%s] and has length %d\n", i, aBuf, (int)nLength);
#endif

        if (nLength == 0) // Is this even possible?
            continue;

        if (aBuf[0] == '-' && nLength > 0)
        {
            bool bIsKnownOption = true;
            // Make a switch on the first character after the - for a
            // preselection of the option.
            // This is faster then multiple ifs and improves readability.
            switch (aBuf[1])
            {
                case 'p':
                    if (nLength > 1 && aBuf[2] == '=' )
                        strcpy(pFileNamePrefix, &aBuf[3]);
                    else
                        bIsKnownOption = false;
                    break;
                case 'f':
                    if (nLength > 2 && aBuf[2] == 'o' && aBuf[3] == '=' )
                        strcpy(pOutputFileName, &aBuf[4]);
                    else if (nLength>2 && aBuf[2] == 'p' && aBuf[3] == '=' )
                    {
                        strcpy(pSrsFileName, &aBuf[4]);
                        String aName( pSrsFileName, osl_getThreadTextEncoding());
                        DirEntry aDest( aName );
                        aSrsBaseName = aDest.GetBase();
                    }
                    else
                        bIsKnownOption = false;
                    break;
                case 'i':
                case 'I':
#ifdef DEBUG_VERBOSE
                    printf("Include : %s\n", &aBuf[2] );
#endif
                    pDep->AddSearchPath( &aBuf[2] );
                    break;
                case 'h' :
                case 'H' :
                case '?' :
                    printf("RscDep 1.0\n");
                    break;
                case 'a' :
#ifdef DEBUG_VERBOSE
                    printf("option a\n");
#endif
                    break;

                case 'l' :
#ifdef DEBUG_VERBOSE
                    printf("option l with Value %s\n", &aBuf[2] );
#endif
                    pDep->AddSource(&aBuf[2]);
                    break;

                default:
                    bIsKnownOption = false;
                    break;
            }
#ifdef DEBUG_VERBOSE
            if ( ! bIsKnownOption)
				printf("Unknown option error [%s]\n", aBuf);
#else
            (void)bIsKnownOption;
#endif
        }
        else if (aBuf[0] == '@' )
        {
            rtl::OString aToken;
            String aRespName( &aBuf[1], osl_getThreadTextEncoding());
            SimpleConfig aConfig( aRespName );
            while (!(aToken = aConfig.getNext()).isEmpty())
            {
                char aBuf2[255];
                strcpy( aBuf2, aToken.getStr());
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
                    String aName( pSrsFileName, osl_getThreadTextEncoding());
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
                    aRespArg.append(' ').append(&aBuf2[0]);
                    bSource = sal_True;
                }
            }
        }
        else
        {
            // Collect all non-options at the head of argv.
            if (++nLastNonOption < i)
                argv[nLastNonOption] = argv[i];
        }
    }

    String aCwd(pFileNamePrefix, osl_getThreadTextEncoding());
    SvFileStream aOutStream;
    String aOutputFileName( pOutputFileName, osl_getThreadTextEncoding());
    DirEntry aOutEntry( aOutputFileName );
    String aOutPath = aOutEntry.GetPath().GetFull();

    String aFileName( aOutPath );
    aFileName += cDelim;
    aFileName += aCwd;
    aFileName += String(".", osl_getThreadTextEncoding());
    aFileName += aSrsBaseName;
    aFileName += String(".dprr", osl_getThreadTextEncoding());
    aOutStream.Open( aFileName, STREAM_WRITE );

    // Process the yet unhandled non-options.  These are supposed to
    // be names of files on which the target depends.
    rtl::OStringBuffer aString;
    if ( nLastNonOption >= 0 )
    {
#ifdef DEBUG_VERBOSE
        printf("further arguments : ");
#endif
        aString.append(rtl::OString(pSrsFileName).replace('\\', cDelim));
        aString.append(RTL_CONSTASCII_STRINGPARAM(" : " ));

        for (sal_Int32 nIndex=0; nIndex<=nLastNonOption; ++nIndex)
        {
#ifdef DEBUG
            printf("option at %d is [%s]\n", (int)nIndex, argv[nIndex]);
#endif
            if (!bSource )
            {
                aString.append(' ');
                aString.append(argv[nIndex]);
                pDep->AddSource(argv[nIndex]);
            }
        }
    }
    aString.append(aRespArg.makeStringAndClear());
    pDep->Execute();
    std::vector<rtl::OString>& rLst = pDep->GetDepList();
    size_t nCount = rLst.size();
    if ( nCount != 0 )
        aString.append('\\');
    aOutStream.WriteLine( aString.makeStringAndClear() );

    for ( size_t j = 0; j < nCount; ++j )
    {
        rtl::OStringBuffer aStr(rLst[j].replace('\\', cDelim));
        if ( j != (nCount-1) )
            aStr.append('\\');
        aOutStream.WriteLine(aStr.makeStringAndClear());
    }
    delete pDep;
    aOutStream.Close();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
