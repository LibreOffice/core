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
#include "precompiled_soltools.hxx"

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
int bUseDirectives = 0;
int bVerbose = 0;
int bExportByName = 0;

class ExportSet : public HashTable
{
public:
    ExportSet
    (
        unsigned long lSize,
        double  dMaxLoadFactor = HashTable::m_defMaxLoadFactor,
        double  dGrowFactor = HashTable::m_defDefGrowFactor
    )
        : HashTable(lSize,false,dMaxLoadFactor,dGrowFactor) {}

    virtual ~ExportSet() {}

    LibExport *  Find (char * const& Key) const
    { return (LibExport *) HashTable::Find((char *) Key); }

    bool Insert (char * const& Key, LibExport * Object)
    { return HashTable::Insert((char *) Key, (void*) Object); }

    LibExport *  Delete (char * const&Key)
    { return (LibExport *) HashTable::Delete ((char *) Key); }
};

LibDump::LibDump( char *cFileName, int bExportByName )
                : cBName( NULL ),
                cAPrefix( NULL ),
                cLibName( NULL ),
                cFilterName( NULL ),
                cModName( NULL )
{
    fprintf( stderr, "LIB-NT File Dumper v4.00 (C) 2000 Sun Microsystems, Inc.\n\n" );
    fprintf( stderr, "%s ", cFileName );

    bExportName = bExportByName;

    unsigned long nSlots =  0xfffff;
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
    if (!bExportName)
        CheckDataBase();
}

bool LibDump::Dump()
{
    FILE *pList;
    char  aBuf[MAX_MAN];
    int   nLen;
    char  aName[MAX_MAN];

    pList = fopen( cLibName, "rb");
    if (!pList)
        DumpError(10);

    // forget about offset when working on linker directives
    if ( !bUseDirectives )
    {
        // calculating offset for name section
        unsigned char TmpBuffer[4];
        fread( TmpBuffer, 1, 4, pList);
        // anzahl bigendian mal laenge + ueberspringen der naechsten laengenangabe
        unsigned long nOffSet = (unsigned long) ( TmpBuffer[2] * 256 + TmpBuffer[3] ) * 4 + 4;
        fseek( pList, (long) nOffSet, 0);
    }

    char aTmpBuf[4096];
    // reading file containing symbols
    while( !feof( pList ) )
    {
           int i = 0;
        if ( !bUseDirectives )
        {
            // symbol komplett einlesen
            for (;;)
            {
                int c = fgetc( pList );
                if ( c == '\0' )
                {
                    break;
                }
                if ( ((c >= 33) && (c <= 126)) && ( c!=40 && c!=41) )
                    aBuf[i] = static_cast< char >(c);
                else
                {
                    aBuf[0] = '\0';
                    break;
                }
                i++;
            }
            // Namen found
            aBuf[i] = '\0';
        }
        else
        {
            fgets( aTmpBuf, 4096, pList );
            char * pEnd = 0;
            char *pFound = 0;
            aBuf[0] = '\0';
            pFound = strchr( aTmpBuf, 'E' );
            while ( pFound )
            {
                if ( strncmp( "EXPORT:", pFound, 7) == 0 )
                {
                    pFound += 7;
                    pEnd = strchr( pFound, ',');
                    if ( pEnd )
                        *pEnd = '\0';
                    strncpy( aBuf, pFound, strlen( pFound));
                    aBuf[ strlen( pFound) ] = '\0';
//                    fprintf( stderr, "\n--- %s\n", aBuf);
                    break;
                }
                else
                {
                    pFound++;
                    pFound = strchr( pFound, 'E' );
                }
            }
        }

        if ((aBuf[0] =='?') || !strncmp(aBuf, "__CT",4))
        {
            nLen = (int) strlen(aBuf);
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
            PrintSym( aName, bExportByName );
        }
        else if ( bAll == true &&
                  strncmp(aBuf, "__real@", 7) != 0 &&
                  strncmp(aBuf, "__TI3?", 6) != 0 )
        {
            int nPreLen = (int) strlen( cAPrefix );

            nLen = (int) strlen(aBuf);
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
            nLen = (int) strlen(aName);
#ifndef _WIN64
            if (aName[0] == '_')
                strcpy( aBuf , &aName[1] );
#endif
            strncpy ( aTmpBuf, aBuf, (size_t) nPreLen );
            aTmpBuf[nPreLen] = '\0';
            if ( !strcmp( aTmpBuf, cAPrefix ))
            {
                if ( bLdump3 ) {
                    int nChar = '@';
                    char *pNeu = strchr( aBuf, nChar );
                    size_t nPos = pNeu - aBuf + 1;
                    if ( nPos > 0 )
                    {
                        char aOldBuf[MAX_MAN];
                        strcpy( aOldBuf, aBuf );
                        char pChar[MAX_MAN];
                        strncpy( pChar, aBuf, nPos - 1 );
                        pChar[nPos-1] = '\0';
                        strcpy( aBuf, pChar );
                        strcat( aBuf, "=" );
                        strcat( aBuf, aOldBuf );
                        strcpy( pChar, "" );
                    }
                }
                // und raus damit
                PrintSym( aBuf, true );
            }
        }
    }
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
    }

    while( fgets( aBuf, MAX_MAN, pfFilter ) != 0 )
    {
        nLen = (int) strlen(aBuf);
        pStr = new char[(unsigned int) nLen];
        if ( !pStr )
            DumpError( 98 );
        memcpy( pStr, aBuf, (unsigned int) nLen );
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
    LibExport *pData;


    // Filter auswerten
    if ( Filter( pName ) )
    {
        if ( strlen( pName ) > 3 )
        {
            if ( bDef )
            {
                if (!bBase)
                    if (bExportName) {
                        fprintf( stdout, "\t%s\n", pName );
                    } else {
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
                        if ( bVerbose )
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
                        if ( bVerbose )
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

bool LibDump::IsFromAnonymousNamespace (char *pExportName) {
    char* pattern1 = "@?A0x";

    if (strstr(pExportName, pattern1)) {
        return true;
    };
    return false;
};

bool LibDump::Filter(char *pExportName)
{
    unsigned long i;
    char pTest[256];

    // filter out symbols from anonymous namespaces
    if (IsFromAnonymousNamespace (pExportName))
        return false;

    // Kein Filter gesetzt
    if ( ::bFilter == 0 )
        return true;

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
    char  cBuffer[ 30 ];
    int   nLen;
    LibExport *pData;

    pfBase = fopen( cBName, "r" );

    if ( !pfBase )
    {
        bBase = 0;
        DumpError( 600 );
    }

    bool bRet = true;
    while( fgets( aBuf, MAX_MAN, pfBase ) != 0 )
    {
        nLen = (int) strlen(aBuf);
        pStr = new char[(unsigned int) nLen];
        if ( !pStr )
            DumpError( 98 );
        memcpy( pStr, aBuf, (size_t) nLen );
        if ( *(pStr+nLen-1) == '\n' )
            *(pStr+nLen-1) = '\0';
        pData = new LibExport;
        pData->cExportName = pStr;
        pData->nOrdinal = nBaseLines;
        pData->bExport=false;

        if (pBaseTab->Insert(pData->cExportName, pData ) == NULL)
            bRet = false;
        ltoa( (long) pData->nOrdinal, cBuffer, 10 );
        if (pIndexTab->Insert( cBuffer, pData ) == NULL)
            bRet = false;
        nBaseLines++;
        if ( nBaseLines >= MAXBASE )
            DumpError( 610 );
    }
    fclose( pfBase );
    return bRet;
}

class ExportSetIter : public HashTableIterator
{
public:
    ExportSetIter(HashTable const& aTable)
        : HashTableIterator(aTable) {}

    LibExport * GetFirst()
    { return (LibExport *)HashTableIterator::GetFirst(); }
    LibExport * GetNext()
    { return (LibExport *)HashTableIterator::GetNext();  }
    LibExport * GetLast()
    { return (LibExport *)HashTableIterator::GetLast();  }
    LibExport * GetPrev()
    { return (LibExport *)HashTableIterator::GetPrev();  }

private:
    void operator =(ExportSetIter &); // not defined
};

bool LibDump::PrintDataBase()
{
    if (bExportName)
        return true;
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
                fprintf(stdout,"\t%s\n",
                    pData->sExportName.GetBuffer());
            }
            else
            {
                fprintf(stdout,"\t%s\t\t@%d NONAME\n",
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
                        fprintf(stdout,"\t%s\n",
                            pData->cExportName);
                }
                else
                {
                    if ( strlen( pData->cExportName ))
                        fprintf(stdout,"\t%s\t\t@%d NONAME\n",
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
#ifdef OS2
        _mkdir ("defs", 0777);
#else
        _mkdir ("defs");
#endif
        strcpy(cBName,pTmp);
#ifdef OS2
        strcat(cBName,"gcc");
#else
        strcat(cBName,getenv ("COMP_ENV"));
#endif

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

void LibDump::DumpError( unsigned long n )
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
        case 99: p = "LDUMP [-LD3] [-D] [-N] [-A] [-E nn] [-F name] Filename[.LIB]\n"
                     "-LD3   : Supports feature set of ldump3 (default: ldump/ldump2)\n"
                     "-A     : all symbols (default: only C++)\n"
                     "-E nn  : gerenration of export table beginning with number nn\n"
                     "-F name: Filter file\n"
                     "-D     : file contains \"dumpbin\" directives\n"
                     "-N     : export by name\n"
                     "-V     : be verbose\n"; break;
        case 500: p = "Unable to open filter file\n"; break;
        case 510: p = "Overflow of filter table\n"; break;
        case 600: p = "Unable to open base database file\n"; break;
        case 610: p = "Overflow in base database table\n"; break;
        default: p = "Unspecified error";
    }
    fprintf( stdout, "%s\n", p );
    exit (1);
}

/*********************************************************************
        Test Funktionen
*********************************************************************/


void usage()
{
    LibDump::DumpError(99);
}

#define STATE_NON       0x0000
#define STATE_BEGIN     0x0001
#define STATE_FILTER    0x0002
#define STATE_CEXPORT   0x0003

int
#ifdef WNT
__cdecl
#endif
main( int argc, char **argv )
{
    char *pLibName = NULL, *pFilterName = NULL, *pCExport= NULL;
    unsigned short nBegin=1;

    unsigned short nState = STATE_NON;

    if ( argc == 1 ) {
        usage();
    }

    for ( int i = 1; i < argc; i++ ) {
        if (( !strcmp( argv[ i ], "-H" )) ||
            ( !strcmp( argv[ i ], "-h" )) ||
            ( !strcmp( argv[ i ], "-?" )))
        {
            usage();
        }
        else if (( !strcmp( argv[ i ], "-LD3" )) ||
            ( !strcmp( argv[ i ], "-Ld3" )) ||
            ( !strcmp( argv[ i ], "-ld3" )) ||
            ( !strcmp( argv[ i ], "-lD3" )))
        {
            if ( nState != STATE_NON ) {
                usage();
            }
            bLdump3 = 1;
        }
        else if (( !strcmp( argv[ i ], "-E" )) || ( !strcmp( argv[ i ], "-e" ))) {
            if ( nState != STATE_NON ) {
                usage();
            }
            nState = STATE_BEGIN;
        }
        else if (( !strcmp( argv[ i ], "-F" )) || ( !strcmp( argv[ i ], "-f" ))) {
            if ( nState != STATE_NON ) {
                usage();
            }
            nState = STATE_FILTER;
        }
        else if (( !strcmp( argv[ i ], "-A" )) || ( !strcmp( argv[ i ], "-a" ))) {
            if ( nState != STATE_NON ) {
                usage();
            }
            nState = STATE_CEXPORT;
            pCExport = new char[ 1 ];
            pCExport[ 0 ] = 0;
        }
        else if (( !strcmp( argv[ i ], "-D" )) || ( !strcmp( argv[ i ], "-d" ))) {
            if ( nState != STATE_NON ) {
                usage();
            }
            bUseDirectives = 1;
        }
        else if (( !strcmp( argv[ i ], "-N" )) || ( !strcmp( argv[ i ], "-n" ))) {
            if ( nState != STATE_NON ) {
                usage();
            }
            bExportByName = 1;
        }
        else if (( !strcmp( argv[ i ], "-V" )) || ( !strcmp( argv[ i ], "-v" ))) {
            if ( nState != STATE_NON ) {
                usage();
            }
            bVerbose = 1;
        }
        else {
            switch ( nState ) {
                case STATE_BEGIN:
                    nBegin = static_cast< unsigned short >(atoi( argv[ i ] ));
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
    }

    LibDump *pDump = new LibDump( pLibName, bExportByName );
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
    delete [] pLibName;
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
