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
#if defined(UNX) || defined(OS2)
#include <unistd.h>
#endif

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include "bootstrp/prj.hxx"
#include "sal/main.h"

#include <tools/string.hxx>
#include <tools/list.hxx>
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

//static String aDelim;

int main( int argc, char** argv )
{
    char aBuf[255];
    char pFileNamePrefix[255];
    char pOutputFileName[255];
    char pSrsFileName[255];
    String aSrsBaseName;
    sal_Bool bSource = sal_False;
    ByteString aRespArg;
//  who needs anything but '/' ?
//  String aDelim = String(DirEntry::GetAccessDelimiter());
    String aDelim = '/';
    RscHrcDep *pDep = new RscHrcDep;

    // When the options are processed, the non-option arguments are
    // collected at the head of the argv array.
    // nLastNonOption points to the last of them.
    int nLastNonOption (-1);

    pOutputFileName[0] = 0;
    pSrsFileName[0] = 0;

    for ( int i=1; i<argc; i++)
    {
        strcpy( aBuf, (const char *)argv[i] );
        const sal_Int32 nLength (strlen(aBuf));

        printf("option %d is [%s] and has length %d\n", i, aBuf, nLength);

        if (nLength == 0)
        {
            // Is this even possible?
            continue;
        }
        if (aBuf[0] == '-' && nLength > 0)
        {
            bool bIsKnownOption (true);
            // Make a switch on the first character after the - for a
            // preselection of the option.
            // This is faster then multiple ifs and improves readability.
            switch (aBuf[1])
            {
                case 'p':
                    if (nLength>1 && aBuf[2] == '=' )
                        strcpy(pFileNamePrefix, &aBuf[3]);
                    else
                        bIsKnownOption = false;
                    break;

                case 'f':
                    if (nLength>2 && aBuf[2] == 'o' && aBuf[3] == '=' )
                    {
                        strcpy(pOutputFileName, &aBuf[4]);
                    }
                    else if (nLength>2 && aBuf[2] == 'p' && aBuf[3] == '=' )
                    {
                        strcpy(pSrsFileName, &aBuf[4]);
                        String aName( pSrsFileName, gsl_getSystemTextEncoding());
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
#endif
        }
        else if (aBuf[0] == '@' )
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
                    //break;
                }
                if ( aBuf2[0] == '-' && aBuf2[1] == 'f' && aBuf2[2] == 'o' )
                {
                    strcpy(pOutputFileName, &aBuf2[3]);
                    //break;
                }
                if ( aBuf2[0] == '-' && aBuf2[1] == 'f' && aBuf2[2] == 'p' )
                {
                    strcpy(pSrsFileName, &aBuf2[3]);
                    String aName( pSrsFileName, gsl_getSystemTextEncoding());
                    DirEntry aDest( aName );
                    aSrsBaseName = aDest.GetBase();
                    //break;
                }
                if (aBuf2[0] == '-' &&  aBuf2[1] == 'i' )
                {
                    //printf("Include : %s\n", &aBuf[2] );
                    pDep->AddSearchPath( &aBuf2[2] );
                }
                if (aBuf2[0] == '-' &&  aBuf2[1] == 'I' )
                {
                    //printf("Include : %s\n", &aBuf[2] );
                    pDep->AddSearchPath( &aBuf2[2] );
                }
                if (( aBuf2[0] != '-' ) && ( aBuf2[0] != '@' ))
                {
                    pDep->AddSource( &aBuf2[0] );
                    aRespArg += " ";
                    aRespArg += &aBuf2[0];
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

    DirEntry aEntry(".");
    aEntry.ToAbs();
//  String aCwd = aEntry.GetName();
    String aCwd(pFileNamePrefix, gsl_getSystemTextEncoding());
/*  sal_uInt16 nPos;
#ifndef UNX
    while ( (nPos = aCwd.Search('\\') != STRING_NOTFOUND  ))
#else
    while ( (nPos = aCwd.Search('/') != STRING_NOTFOUND  ))
#endif
    {
        String attt = aCwd.Copy( 0, nPos );
        aCwd.Erase( 0, nPos );
    } */
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
    //fprintf( stderr, "OutFileName : %s \n",aFileName.GetStr());
    aOutStream.Open( aFileName, STREAM_WRITE );

    // Process the yet unhandled non-options.  These are supposed to
    // be names of files on which the target depends.
    ByteString aString;
    if (nLastNonOption >= 0)
    {
#ifdef DEBUG_VERBOSE
        printf("further arguments : ");
#endif
        aString = ByteString( pSrsFileName );
        aString.SearchAndReplaceAll('\\', ByteString( aDelim,  RTL_TEXTENCODING_ASCII_US ));
        aString += ByteString(" : " );

        for (sal_Int32 nIndex=0; nIndex<=nLastNonOption; ++nIndex)
        {
            printf("option at %d is [%s]\n", nIndex, argv[nIndex]);
            if (!bSource )
            {
                aString += ByteString(" " );
                aString += ByteString( argv[nIndex]);
                pDep->AddSource( argv[nIndex]);
            }
        }
    }
    aString += aRespArg;
    pDep->Execute();
    ByteStringList *pLst = pDep->GetDepList();
    sal_uIntPtr nCount = pLst->Count();
    if ( nCount == 0 )
    {
        aOutStream.WriteLine( aString );
    }
    else
    {
        aString += ByteString( "\\" );
        aOutStream.WriteLine( aString );
    }

    for ( sal_uIntPtr j=0; j<nCount; j++ )
    {
        ByteString *pStr = pLst->GetObject(j);
        pStr->SearchAndReplaceAll('\\', ByteString( aDelim,  RTL_TEXTENCODING_ASCII_US ));
        if ( j != (nCount-1) )
            *pStr += ByteString( "\\" );
        aOutStream.WriteLine( *pStr );
    }
    delete pDep;
    aOutStream.Close();

    return 0;
}

