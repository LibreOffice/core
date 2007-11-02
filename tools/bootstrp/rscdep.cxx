/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rscdep.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 12:59:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

/* poor man's getopt() */
int     simple_getopt(char *argv[], const char *optstring);
#if defined(WNT) || defined(OS2)
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
    char aBuf[255];
    char pOutputFileName[255];
    char pSrsFileName[255];
    String aSrsBaseName;
    BOOL bSource = FALSE;
    ByteString aRespArg;
    String aDelim = String(DirEntry::GetAccessDelimiter());

    RscHrcDep *pDep = new RscHrcDep;

    pOutputFileName[0] = 0;
    pSrsFileName[0] = 0;

    for ( int i=1; i<argc; i++)
    {
        strcpy( aBuf, (const char *)argv[i] );
        if ( aBuf[0] == '-' && aBuf[1] == 'f' && aBuf[2] == 'o' && aBuf[3] == '=' )
        {
            strcpy(pOutputFileName, &aBuf[4]);
            //break;
        }
        if ( aBuf[0] == '-' && aBuf[1] == 'f' && aBuf[2] == 'p' && aBuf[3] == '=' )
        {
            strcpy(pSrsFileName, &aBuf[4]);
            String aName( pSrsFileName, gsl_getSystemTextEncoding());
            DirEntry aDest( aName );
            aSrsBaseName = aDest.GetBase();
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
                    bSource = TRUE;
                }
            }
        }
    }

    while( 1 )
    {
        c = simple_getopt( argv,
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
        aString = ByteString( pSrsFileName );
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
int simple_getopt(char *argv[], const char *optstring)
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

