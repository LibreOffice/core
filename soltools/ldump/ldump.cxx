/*************************************************************************
 *
 *  $RCSfile: ldump.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-09 11:23:31 $
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

#include <string.h>
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>

#include "ldump.hxx"
#include "hashtbl.hxx"

#define MAXSYM     65536
#define MAXBASE    98304
#define MAX_MAN     4096

int bFilter = 0;
int bLdump3 = 0;

DECLARE_HASHTABLE( ExportSet, char *, LibExport*)

LibDump::LibDump( char *cFileName )
                : cBName( NULL ),
                cAPrefix( NULL ),
                cLibName( NULL ),
                cFilterName( NULL ),
                cModName( NULL )
{
    fprintf( stderr, "LIB-NT File Dumper v4.00 (C) 2000 Sun Microsystems, Inc.\n\n" );
    fprintf( stderr, "%s ", cFileName );

    int nSlots =  63997;
    pBaseTab = new ExportSet( nSlots );
    pIndexTab = new ExportSet( nSlots );
    pFilterLines = new char * [MAXFILT];
    CheckLibrary(cFileName);
    bBase = 0;
    bAll = false;
    nDefStart = 0;
    nBaseLines = 0;
    nFilterLines = 0;
    bDef = true;
    cAPrefix = new char[ 1 ];
    cAPrefix[ 0 ] = 0;
    CheckDataBase();
}

bool LibDump::Dump()
{
    FILE *pList;
    char  aBuf[MAX_MAN];
    int   nLen;
    char  aName[MAX_MAN];

#ifndef ICC
    pList = fopen( cLibName, "rb");
    if (!pList)
        DumpError(10);

    // berechnug des offsets fuer anfang der gemangelten namen
    unsigned char TmpBuffer[4];
    fread( TmpBuffer, 1, 4, pList);
    // anzahl bigendian mal laenge + ueberspringen der naechsten laengenangabe
    unsigned long nOffSet = ( TmpBuffer[2] * 256 + TmpBuffer[3] ) * 4 + 4;
    fseek( pList, nOffSet, 0);

    char c;
    char aTmpBuf[4096];
    // Einlesen der .exp-Datei
    while( !feof( pList ) )
    {
        int i = 0;
        // symbol komplett einlesen
        while ( (c = fgetc( pList )) != '\0' )
        {
            if ( ((c >= 33) && (c <= 126)) && ( c!=40 && c!=41) )
                aBuf[i] = c;
            else
            {
                aBuf[0] = '\0';
                break;
            }
            i++;
        }
        // Namen gefunden
        aBuf[i] = '\0';

        int n_is_ct;
        if ((aBuf[0] =='?') || ( n_is_ct = !strncmp(aBuf, "__CT",4)))
        {
            nLen = strlen(aBuf);
            memset( aName, 0, sizeof( aName ) );
            int nName = 0;
            for( i = 0; i < nLen; i++ )
            {
                if ( (aBuf[i] != '\n') && (aBuf[i] != '\r') )
                {
                    aName[nName] = aBuf[i];
                    nName++;
                }
            }
            // und raus damit
            PrintSym( aName );
        }
        else if ( bAll == true )
        {
            int nPreLen = strlen( cAPrefix );

            nLen = strlen(aBuf);
            memset( aName, 0, sizeof( aName ) );
            int nName = 0;

            for( i = 0; i < nLen; i++ )
            {
              if ( (aBuf[i] != '\n') && (aBuf[i] != '\r') )
              {
                  aName[nName] = aBuf[i];
                  nName++;
              }
            }
            //fprintf( stderr, "Gefundenen Prefix : %s %d \n", aTmpBuf, nPreLen );
            // den ersten _ raus
            nLen = strlen(aName);
            if (aName[0] == '_')
                strcpy( aBuf , &aName[1] );
            strncpy ( aTmpBuf, aBuf, nPreLen );
            aTmpBuf[nPreLen] = '\0';
            if ( !strcmp( aTmpBuf, cAPrefix ))
            {
                if ( bLdump3 ) {
                    int nChar = '@';
                    char *pNeu = strchr( aBuf, nChar );
                    int nPos = pNeu - aBuf + 1;
                    if ( nPos > 0 )
                    {
                        char aOldBuf[MAX_MAN];
                        strcpy( aOldBuf, aBuf );
                        char pChar[MAX_MAN];
                        strncpy( pChar, aBuf, nPos -1 );
                        pChar[nPos-1] = '\0';
                        strcpy( aBuf, pChar );
                        strcat( aBuf, "=" );
                        strcat( aBuf, aOldBuf );
                        strcpy( pChar, "" );
                    }
                }
                // und raus damit
                PrintSym( aBuf, false );
            }
        }
    }
#else
    pList = fopen( sLibName.GetBuffer(), "rb");
    if (!pList)
        DumpError(10);

    char c = fgetc( pList );

    while( !feof( pList ) )
    {
        int bBreakOut = 0;

        int i=0;
        bool bStop = false;

        while ( c != '\0' )
        {
            if(c == 0x0d || c == 0x20)
            {
                c = fgetc( pList );
                continue;
            }

            if(c == '@')
            {
                bStop = true;
                c = fgetc( pList );
                continue;
            }

            if(c != 0x0a)
            {
                if(!bStop)
                    aBuf[i++] = c;
            }
            else
            {
                aBuf[i] = '\0';
                c = fgetc( pList );
                break;
            }

            c = fgetc( pList );
        }


        //wenn erstes Zeichen kein Semikolon ist, wird die Zeile bernommen
        if(aBuf[0] != ';' && aBuf[0] != 0)
        {
            //cdecl Methoden werden ohne NONAME expotiert
            if(aBuf[0] == '_' && aBuf[1] != '_' && sAPrefix.Len())
                  PrintSym( aBuf, false );
            else if(ByteString(aBuf).Search(sAPrefix) == 0)
                  PrintSym( aBuf, false );
            else
                PrintSym( aBuf );
        }
    }
#endif
    fclose(pList);
    return true;
}

bool LibDump::ReadFilter( char * cFilterName )
{
    FILE* pfFilter = 0;
    char  aBuf[MAX_MAN];
    char* pStr;
    int   nLen;

    pfFilter = fopen( cFilterName, "r" );

    if ( !pfFilter )
    {
        ::bFilter = 0;
        DumpError( 500 );
        return true;
    }

    while( fgets( aBuf, MAX_MAN, pfFilter ) != 0 )
    {
        nLen = strlen(aBuf);
        pStr = new char[nLen];
        if ( !pStr )
            DumpError( 98 );
        memcpy( pStr, aBuf, nLen );
        if ( *(pStr+nLen-1) == '\n' )
            *(pStr+nLen-1) = '\0';
        pFilterLines[nFilterLines] = pStr;
        nFilterLines++;
        if ( nFilterLines >= MAXFILT )
                DumpError( 510 );
    }

    fclose( pfFilter );
    return true;
}

bool LibDump::PrintSym(char *pName, bool bName )
{
    int nTreffer = 0;
    int bTreffer = false;
    LibExport *pData;


    // Filter auswerten
    if ( Filter( pName ) )
    {
        if ( strlen( pName ) > 3 )
        {
            if ( bDef )
            {
                if (!bBase)
                {
                        fprintf( stdout, "\t%s\t\t@%lu\n", pName, nDefStart );
                }
                else
                {
                     pData = pBaseTab->Find( pName );
                     if ( pData )
                     {
                        pData->bExport = true;
                        if ( bName )
                            pData->bByName = true;
                        else
                            pData->bByName = false;
                        fprintf(stderr,".");
                     }
                     else
                     {
                         // neuen Export eintragen
                         pData = new LibExport;
                        pData->cExportName = new char[ strlen( pName ) + 1 ];
                        strcpy( pData->cExportName, pName );
                        pData->nOrdinal = nBaseLines++;
                        pData->bExport = true;
                        if ( bName )
                            pData->bByName = true;
                        else
                            pData->bByName = false;
                        pBaseTab->Insert( pData->cExportName, pData );
                        char *cBuffer = new char[ 30 ];
                        sprintf( cBuffer, "%lu", pData->nOrdinal );
                        pIndexTab->Insert( cBuffer, pData );
                        delete [] cBuffer;
                        fprintf(stderr,"n");
                     }
                }
            }
            else
                printf( "%s\n", pName );
            nDefStart++;
        }
    }
    return true;
}

bool LibDump::Filter(char *pExportName)
{
    unsigned long i;
    char pTest[256];

    // Kein Filter gesetzt
    if ( ::bFilter == 0 )
        return true;

    // die Imports bleiben drin !
    if ( strstr ( pExportName, "mport"))
        return true;

    if( strstr ( pExportName, "getImpl"))
        i= 5;

    for ( i=0; i<nFilterLines; i++ )
    {
        //Zum vergleichen muá das Plus abgeschnitteb werden
        if(pFilterLines[i][0] != '+')
        {
            if ( strstr( pExportName, pFilterLines[i]))
                return false;
        }
        else
        {
            strcpy(pTest,&pFilterLines[i][1]);
            if ( strstr( pExportName, pTest))
                return true;
        }
    }
    return true;
}

bool LibDump::SetFilter(char * cFilterName)
{
    ReadFilter( cFilterName );
    return true;
}

bool LibDump::CheckLibrary(char * cName)
{
    delete [] cLibName;
    cLibName = new char[ strlen( cName ) + 1 ];
    strcpy( cLibName, cName );
    return true;
}

bool LibDump::ReadDataBase()
{
    FILE* pfBase = 0;
    char  aBuf[MAX_MAN];
    char* pStr;
    int   nLen;
    LibExport *pData;

    pfBase = fopen( cBName, "r" );

    if ( !pfBase )
    {
        bBase = 0;
        DumpError( 600 );
        return false;
    }

    bool bRet = true;
    while( fgets( aBuf, MAX_MAN, pfBase ) != 0 )
    {
        nLen = strlen(aBuf);
        pStr = new char[nLen];
        if ( !pStr )
            DumpError( 98 );
        memcpy( pStr, aBuf, nLen );
        if ( *(pStr+nLen-1) == '\n' )
            *(pStr+nLen-1) = '\0';
        pData = new LibExport;
        pData->cExportName = new char[ strlen( pStr ) + 1 ];
        strcpy( pData->cExportName, pStr );
        pData->nOrdinal = nBaseLines;
        pData->bExport=false;

        if (pBaseTab->Insert(pData->cExportName, pData ) == NULL)
            bRet = false;
        char *cBuffer = new char[ 30 ];
        sprintf( cBuffer, "%lu", pData->nOrdinal );
        if (pIndexTab->Insert( cBuffer, pData ) == NULL)
            bRet = false;
        delete [] cBuffer;
        nBaseLines++;
        if ( nBaseLines >= MAXBASE )
            DumpError( 610 );
    }
    //nNumber = nBaseLines-1;
    fclose( pfBase );
    return bRet;
}

DECLARE_HASHTABLE_ITERATOR( ExportSetIter, LibExport* )

bool LibDump::PrintDataBase()
{
    FILE *pFp;
    pFp = fopen (cBName,"w+");
    if (!pFp)
        fprintf( stderr, "Error opening DataBase File\n" );

    LibExport *pData;
    for ( unsigned long i=0; i < nBaseLines+10; i++ )
    {
        char * cBuffer = new char[ 30 ];
        sprintf( cBuffer, "%lu", i );
        pData = pIndexTab->Find( cBuffer );
        delete [] cBuffer;
        if ( pData )
            fprintf(pFp,"%s\n",pData->cExportName);
    }
    fclose(pFp);
    return true;
}

bool LibDump::PrintDefFile()
{
#ifdef FAST
    ExportSetIter aIterator( *pBaseTab );
    for ( LibExport *pData = aIterator.GetFirst(); pData != NULL;
                                        pData = aIterator.GetNext() )
    {
        if ( pData->bExport )
        {
            if ( pData->bByName )
            {
                fprintf(stdout,"\t%s\t\t@%d NONAME\n",
                    pData->sExportName.GetBuffer(), pData->nOrdinal+nBegin);
            }
            else
            {
                fprintf(stdout,"\t%s\t\t@%d\n",
                    pData->sExportName.GetBuffer(), pData->nOrdinal+nBegin);
            }
        }
    }
#else
    // sortiert nach Ordinals;
    LibExport *pData;
    for ( unsigned long i=0; i<nBaseLines+1; i++)
    {
        char * cBuffer = new char[ 30 ];
        sprintf( cBuffer, "%lu", i );
        pData = pIndexTab->Find( cBuffer );
        delete [] cBuffer;
        if ( pData )
            if ( pData->bExport )
            {
                if ( pData->bByName )
                {
                    if ( strlen( pData->cExportName ))
                        fprintf(stdout,"\t%s\t\t@%d NONAME\n",
                            pData->cExportName, pData->nOrdinal+nBegin);
                }
                else
                {
                    if ( strlen( pData->cExportName ))
                        fprintf(stdout,"\t%s\t\t@%d\n",
                            pData->cExportName, pData->nOrdinal+nBegin);
                }
            }
    }
#endif
    return true;
}

bool LibDump::CheckDataBase()
{
    // existiert eine Datenbasis ?
    if (!bBase)
    {
        cBName = new char[ 2048 ];
        char *pTmp = "defs\\";

        FILE *fp;
        _mkdir ("defs");
        strcpy(cBName,pTmp);
        strcat(cBName,getenv ("COMP_ENV"));

        fp = fopen (cBName,"r");
        if (fp)
        {
            bBase = true;
        }
        else
        {
            fp = fopen (cBName,"w+");
            bBase = true;
        }
        fclose (fp);
    }
    // lese Datenbasis !
    if (bBase)
    {
        ReadDataBase();
    }
    return true;
}

LibDump::~LibDump()
{
    delete [] cBName;
    delete [] cAPrefix;
//  delete [] cLibName;
    delete [] cFilterName;
    delete [] cModName;
}

void LibDump::SetCExport( char* pName )
{
    delete [] cAPrefix;
    cAPrefix = new char[ strlen( pName ) + 1 ];
    strcpy( cAPrefix, pName );bAll = true;
}

//******************************************************************
//* Error() - Gibt Fehlermeldumg aus
//******************************************************************

bool LibDump::DumpError( unsigned long n )
{
    char *p;

    switch (n)
    {
        case 1:  p = "Input error in library file"; break;
        case 2:  p = "Position error in library file (no THEADR set)"; break;
        case 3:  p = "Overflow of symbol table"; break;
#ifdef WNT
        case 10: p = "EXP file not found"; break;
        case 11: p = "No valid EXP file"; break;
#else
        case 10: p = "Library file not found"; break;
        case 11: p = "No valid library file"; break;
#endif
        case 98: p = "Out of memory"; break;
        case 99: p = "LDUMP [-LD3] [-A] [-E nn] [-F name] Filename[.LIB]\n"
                     "-LD3   : Supports feature set of ldump3 (default: ldump/ldump2)\n"
                     "-A     : all symbols (default: only C++)\n"
                     "-E nn  : gerenration of export table beginning with number nn\n"
                     "-F name: Filter file\n"; break;
        case 500: p = "Unable to open filter file\n"; break;
        case 510: p = "Overflow of filter table\n"; break;
        case 600: p = "Unable to open base database file\n"; break;
        case 610: p = "Overflow in base database table\n"; break;
        default: p = "Unspecified error";
    }
    fprintf( stdout, "%s\n", p );
    exit (1);
    return false;
}

/*********************************************************************
        Test Funktionen
*********************************************************************/


usage()
{
    LibDump::DumpError(99);
    exit(0);
    return 0;
}

#define STATE_NON       0x0000
#define STATE_BEGIN     0x0001
#define STATE_FILTER    0x0002
#define STATE_CEXPORT   0x0003

#ifdef WNT
int __cdecl
#endif
main( int argc, char **argv )
{
    char *pLibName = NULL, *pFilterName, *pCExport= NULL;
    int nBegin=1;

    unsigned short nState = STATE_NON;

    if ( argc == 1 ) {
        usage();
        return 0;
    }

    for ( int i = 1; i < argc; i++ ) {
        if (( !strcmp( argv[ i ], "-H" )) ||
            ( !strcmp( argv[ i ], "-h" )) ||
            ( !strcmp( argv[ i ], "-?" )))
        {
            usage();
            return 0;
        }
        else if (( !strcmp( argv[ i ], "-LD3" )) ||
            ( !strcmp( argv[ i ], "-Ld3" )) ||
            ( !strcmp( argv[ i ], "-ld3" )) ||
            ( !strcmp( argv[ i ], "-lD3" )))
        {
            if ( nState != STATE_NON ) {
                usage();
                return 0;
            }
            bLdump3 = 1;
        }
        else if (( !strcmp( argv[ i ], "-E" )) || ( !strcmp( argv[ i ], "-e" ))) {
            if ( nState != STATE_NON ) {
                usage();
                return 0;
            }
            nState = STATE_BEGIN;
        }
        else if (( !strcmp( argv[ i ], "-F" )) || ( !strcmp( argv[ i ], "-f" ))) {
            if ( nState != STATE_NON ) {
                usage();
                return 0;
            }
            nState = STATE_FILTER;
        }
        else if (( !strcmp( argv[ i ], "-A" )) || ( !strcmp( argv[ i ], "-a" ))) {
            if ( nState != STATE_NON ) {
                usage();
                return 0;
            }
            nState = STATE_CEXPORT;
            pCExport = new char[ 1 ];
            pCExport[ 0 ] = 0;
        }
        else {
            switch ( nState ) {
                case STATE_BEGIN:
                    nBegin = atoi( argv[ i ] );
                    nState = STATE_NON;
                break;
                case STATE_FILTER:
                    pFilterName = new char[ strlen( argv[ i ] ) + 1 ];
                    strcpy( pFilterName, argv[ i ] );
                    bFilter = 1;
                    nState = STATE_NON;
                break;
                case STATE_CEXPORT:
                    delete [] pCExport;
                    pCExport = new char[ strlen( argv[ i ] ) + 1 ];
                    strcpy( pCExport, argv[ i ] );
                    nState = STATE_NON;
                break;
                default:
                    pLibName = new char[ strlen( argv[ i ] ) + 1 ];
                    strcpy( pLibName, argv[ i ] );
                break;
            }
        }
    }

    if ( !pLibName ) {
        usage();
        return 0;
    }

    LibDump *pDump = new LibDump( pLibName );
    pDump->SetBeginExport(nBegin);
    if ( bFilter != 0 )
        pDump->SetFilter( pFilterName );
    if ( pCExport )
        pDump->SetCExport( pCExport );
    else {
        char *pEmpty = "";
        pDump->SetCExport( pEmpty );
    }
    pDump->Dump();
    pDump->PrintDefFile();
    pDump->PrintDataBase();
    delete pDump;
    return 0;
}

