/*************************************************************************
 *
 *  $RCSfile: rscdep.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: kz $ $Date: 2003-08-25 15:36:59 $
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
/***************************************************************
*
*   rscdep.cxx
*
*   (c) Martin Hollmichel 1998
*   added intern simple_getopt Vladimir Glazounov 2002
*
***************************************************************/

#ifdef UNX
#include <unistd.h>
#endif

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include "prj.hxx"

#if SUPD < 356
#include <tools.hxx>
#else
#include <string.hxx>
#include <list.hxx>
#include <fsys.hxx>
#include <stream.hxx>
#endif

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

/* poor man's getopt() */
int     simple_getopt(int argc, char *argv[], const char *optstring);
#ifdef WNT
static char *optarg = NULL;
static int  optind = 1;
static int  optopt = 0;
static int  opterr = 0;
#endif


int
#ifdef WNT
_cdecl
#endif
main( int argc, char **argv )
{
    int c;
    int digit_optind = 0;
    char aBuf[255];
    char pOutputFileName[255];
    String aSrsBaseName;
    BOOL bSource = FALSE;
    ByteString aRespArg;
    String aDelim = String(DirEntry::GetAccessDelimiter());

    RscHrcDep *pDep = new RscHrcDep;

    for ( int i=1; i<argc; i++)
    {
        strcpy( aBuf, (const char *)argv[i] );
        if ( aBuf[0] == '-' && aBuf[1] == 'f' && aBuf[2] == 'o' )
        {
            strcpy(pOutputFileName, &aBuf[3]);
            //break;
        }
        if ( aBuf[0] == '-' && aBuf[1] == 'f' && aBuf[2] == 'p' )
        {
            strcpy(pOutputFileName, &aBuf[3]);
            String aName( pOutputFileName, gsl_getSystemTextEncoding());
            USHORT nPos = 0;
            DirEntry aDest( aName );
            aSrsBaseName = DirEntry(aDest.GetBase()).GetExtension();
            //break;
        }
        if (aBuf[0] == '-' &&  aBuf[1] == 'i' )
        {
            //printf("Include : %s\n", &aBuf[2] );
            pDep->AddSearchPath( &aBuf[2] );
        }
        if (aBuf[0] == '-' &&  aBuf[1] == 'I' )
        {
            //printf("Include : %s\n", &aBuf[2] );
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
                if ( aBuf2[0] == '-' && aBuf2[1] == 'f' && aBuf2[2] == 'o' )
                {
                    strcpy(pOutputFileName, &aBuf2[3]);
                    //break;
                }
                if ( aBuf2[0] == '-' && aBuf2[1] == 'f' && aBuf2[2] == 'p' )
                {
                    strcpy(pOutputFileName, &aBuf2[3]);
                    String aName( pOutputFileName, gsl_getSystemTextEncoding());
                    USHORT nPos = 0;
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
                    bSource = TRUE;
                }
            }
        }
    }

    while( 1 )
    {
        int this_option_optind = optind ? optind : 1;
        c = simple_getopt( argc, argv,
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


    DirEntry aEntry(".");
    aEntry.ToAbs();
    String aCwd = aEntry.GetName();
/*  USHORT nPos;
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

    ByteString aString;
    if ( optind < argc )
    {
#ifdef DEBUG_VERBOSE
        printf("further arguments : ");
#endif
        aString = ByteString( pOutputFileName );
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
    ULONG nCount = pLst->Count();
    if ( nCount == 0 )
    {
        aOutStream.WriteLine( aString );
    }
    else
    {
        aString += ByteString( "\\" );
        aOutStream.WriteLine( aString );
    }

    for ( ULONG j=0; j<nCount; j++ )
    {
        ByteString *pStr = pLst->GetObject(j);
#ifdef UNX
        pStr->SearchAndReplaceAll('\\', ByteString( aDelim,  RTL_TEXTENCODING_ASCII_US ));
#endif
#ifdef WNT
        pStr->SearchAndReplaceAll('/', ByteString( aDelim,  RTL_TEXTENCODING_ASCII_US ));
#endif
        if ( j != (nCount-1) )
            *pStr += ByteString( "\\" );
        aOutStream.WriteLine( *pStr );
    }
    delete pDep;
    aOutStream.Close();

    return 0;
}

/* my very simple minded implementation of getopt()
 * it's too sad that getopt() is not available everywhere
 * note: this is not a full POSIX conforming getopt()
 */
int simple_getopt(int argc, char *argv[], const char *optstring)
{
    char *arg = argv[optind];

    /* skip all response file arguments */
    if ( arg ) {
        while ( *arg == '@' )
            arg = argv[++optind];

        if ( arg[0] == '-' && arg[1] != '\0' ) {
            const char *popt;
            int c = arg[1];
            if ( (popt = strchr(optstring, c)) == NULL ) {
                optopt = c;
                if ( opterr )
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                return '?';
            }
            if ( *(++popt) == ':') {
                 if ( arg[2] != '\0' ) {
                     optarg = ++arg;
                 } else {
                     optarg = argv[++optind];
                 }
             } else {
                 optarg = NULL;
             }
             ++optind;
             return c;
        }
    }
    return -1;
}

