/*************************************************************************
 *
 *  $RCSfile: rsclex.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:55 $
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
/************************************************************************

    Source Code Control System - Header

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/source/parser/rsclex.cxx,v 1.1.1.1 2000-09-18 16:42:55 hr Exp $

**************************************************************************/
#pragma hdrstop
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#ifdef _RSCERROR_H
#include <rscerror.h>
#endif
#ifndef _RSCHASH_HXX
#include <rschash.hxx>
#endif
#ifndef _RSCDB_HXX
#include <rscdb.hxx>
#endif
#ifndef _RSCTOP_HXX
#include <rsctop.hxx>
#endif
#ifndef _RSCKEY_HXX
#include <rsckey.hxx>
#endif
#ifndef _RSCPAR_HXX
#include <rscpar.hxx>
#endif
#ifndef _RSCDEF_HXX
#include <rscdef.hxx>
#endif

#include "rsclex.hxx"
#include <rscyacc.yxx.h>

#include <rtl/textcvt.h>
#include <rtl/textenc.h>

DECLARE_LIST( RscCharList, char * );
/*************************************************************************/
int             c;
BOOL            bLastInclude;// War letztes Symbol INCLUDE
RscFileInst*    pFI;
RscTypCont*     pTC;
RscCharStack *  pCS;
RscExpression * pExp;
struct {
    int     nKeyWord;
    YYSTYPE aYYSType;
} aKeyVal[ 1 ];
BOOL bTargetDefined;


/****************** C O D E **********************************************/
ULONG GetNumber(){
    ULONG   l = 0;
    short   nLog = 10;

    if( '0' == c ){
        c = pFI->GetFastChar();
        if( 'x' == c ){
            nLog = 16;
            c = pFI->GetFastChar();
        }
    };

    if( nLog == 16 ){
        while( isxdigit( c ) ){
            if( isdigit( c ) )
                l = l * nLog + (c - '0');
            else
                l = l * nLog + (toupper( c ) - 'A' + 10 );
            c = pFI->GetFastChar();
        }
    }
    else{
        while( isdigit( c ) || 'x' == c ){
            l = l * nLog + (c - '0');
            c = pFI->GetFastChar();
        }
    }

    while( c=='U' || c=='u' || c=='l' || c=='L' ) //Wg. Unsigned Longs
        c = pFI->GetFastChar();

    if( l > LONG_MAX ) //Oberstes bit gegebenenfalls abschneiden;
        l &= LONG_MAX;

    return( l );
}

char * MallocString(){
    char *  pRet;

    if( NULL == (pRet = pCS->Pop()) )
        pRet = (char *)RscMem::Malloc( MINBUF );

    *pRet = '\0';

    return( pRet );
}

void PutStringBack( char * pStr ){
    //pCS->Push( pStr );
}

int MakeToken( YYSTYPE * pTokenVal ){
    int             c1;
    USHORT          i;
    char *          pStr;

    while( TRUE ){  // Kommentare und Leerzeichen ueberlesen
        while( isspace( c ) )
            c = pFI->GetFastChar();
        if( '/' == c ){
            c1 = c;
            c = pFI->GetFastChar();
            if( '/' == c ){
                while( '\n' != c && !pFI->IsEof() )
                    c = pFI->GetFastChar();
                c = pFI->GetFastChar();
            }
            else if( '*' == c ){
                c = pFI->GetFastChar();
                do {
                    while( '*' != c && !pFI->IsEof() )
                        c = pFI->GetFastChar();
                    c = pFI->GetFastChar();
                } while( '/' != c && !pFI->IsEof() );
                c = pFI->GetFastChar();
            }
            else
                return( c1 );
        }
        else
            break;
    };

    if( c == pFI->IsEof() ){
        return( 0 );
    }

    if( bLastInclude ){
        bLastInclude = FALSE; //Zuruecksetzten
        if( '<' == c ){
            USHORT          nBufLen = MINBUF;
            char *          pBuf = MallocString();

            i = 0;
            c = pFI->GetFastChar();
            while( '>' != c && !pFI->IsEof() )
            {
                if( nBufLen <= (USHORT)(i +1) )
                {
                    nBufLen += MINBUF;
                    pBuf = (char *)RscMem::Realloc( pBuf, nBufLen );
                }
                pBuf[i++] = c;
                c = pFI->GetFastChar();
            };
            c = pFI->GetFastChar();
            pBuf[i] = '\0';
            pTokenVal->string = pBuf;
            return( INCLUDE_STRING );
        };
    }

    if( c == '"' )
    {
        USHORT          nBufLen = MINBUF;
        char *          pBuf = MallocString();

        i = 0;
        BOOL bDone = FALSE;
        while( !bDone && !pFI->IsEof() && c )
        {
            if( nBufLen <= (USHORT)(i +2) )
            {
                nBufLen += MINBUF;
                pBuf = (char *)RscMem::Realloc( pBuf, nBufLen );
            }
            c = pFI->GetFastChar();
            if( c == '"' )
            {
                c = pFI->GetFastChar();
                if( c == '"' )
                {
                    pBuf[i++] = '"';
                    pBuf[i++] = '"';
                }
                else
                    bDone = TRUE;
            }
            else if( c == '\\' )
            {
                pBuf[i++] = '\\';
                c = pFI->GetFastChar();
                if( c )
                {
                    pBuf[i++] = c;
                }
            }
            else
                pBuf[i++]   = c;
        }
        pBuf[i++] = '\0';
        //pStr = pTokenVal->string = RscChar::MakeUTF8( pBuf, pFI->GetCharSet() );
        pStr = pTokenVal->string = pBuf;
        //PutStringBack( pBuf );
        return( STRING );
    }
    if (isdigit (c)){
        pTokenVal->value = GetNumber();
        return( NUMBER );
    }

    if( isalpha (c) || (c == '_') ){
        HASHID      nHashId;
        USHORT      nBufLen = MINBUF;
        char *      pBuf = MallocString();


        i = 0;
        while( isalnum (c) || (c == '_') )
        {
            if( nBufLen <= (USHORT)(i +1) )
            {
                nBufLen += MINBUF;
                pBuf = (char *)RscMem::Realloc( pBuf, nBufLen );
            }
            pBuf[i++] = c;
            c = pFI->GetFastChar();
        }
        /*
        if( pBuf[0] == 'L' && i == 1 && c == '"' )
        {
            // it is an L string

            i = 0;
            BOOL bDone = FALSE;
            while( !bDone && !pFI->IsEof() && c )
            {
                if( nBufLen <= (USHORT)(i +2) )
                {
                    nBufLen += MINBUF;
                    pBuf = (char *)RscMem::Realloc( pBuf, nBufLen );
                }
                c = pFI->GetFastChar();
                if( c == '"' )
                {
                    c = pFI->GetFastChar();
                    if( c == '"' )
                    {
                        pBuf[i++] = '"';
                        pBuf[i++] = '"';
                    }
                    else
                        bDone = TRUE;
                }
                else if( c == '\\' )
                {
                    pBuf[i++] = '\\';
                    c = pFI->GetFastChar();
                    if( c )
                    {
                        pBuf[i++] = c;
                    }
                }
                else
                    pBuf[i++]   = c;
            }
            pBuf[i++] = '\0';
            pStr = pTokenVal->string = RscChar::MakeUTF8FromL( pBuf );
            PutStringBack( pBuf );
            return( STRING );
        }
        else
        */
        {
            pBuf[i++] = '\0';

            nHashId = pHS->Test( pBuf );
            if( HASH_NONAME != nHashId )
            {
                KEY_STRUCT  aKey;

                // Suche nach dem Schluesselwort
                if( pTC->aNmTb.Get( nHashId, &aKey ) )
                {

                    // Schluesselwort gefunden
                    switch( aKey.nTyp )
                    {
                        case CLASSNAME:
                            pTokenVal->pClass = (RscTop *)aKey.yylval;
                            break;
                        case VARNAME:
                            pTokenVal->varid = aKey.nName;
                            break;
                        case CONSTNAME:
                            pTokenVal->constname.hashid = aKey.nName;
                            pTokenVal->constname.nValue = aKey.yylval;
                            break;
                        case BOOLEAN:
                            pTokenVal->svbool = (BOOL)aKey.yylval;
                            break;
                        case INCLUDE:
                            bLastInclude = TRUE;
                        default:
                            pTokenVal->value = aKey.yylval;
                    };

                    // String zurueckgeben
                    PutStringBack( pBuf );
                    return( aKey.nTyp );
                }
                else{
                    pTokenVal->string = pBuf;
                    return( SYMBOL );
                }
            }
            else{       // Symbol
                RscDefine  * pDef;

                pDef = pTC->aFileTab.FindDef( pBuf );
                if( pDef ){
                    pTokenVal->defineele = pDef;

                    // String zurueckgeben
                    PutStringBack( pBuf );
                    return( RSCDEFINE );
                }

                pTokenVal->string = pBuf;
                return( SYMBOL );
            }
        }
    }

    if( c=='<' )
    {
        c = pFI->GetFastChar();
        if( c=='<' )
        {
            c = pFI->GetFastChar();
            return LEFTSHIFT;
        }
        else
            return '<';
    }

    if( c=='>' )
    {
        c = pFI->GetFastChar();
        if( c=='>' )
        {
            c = pFI->GetFastChar();
            return RIGHTSHIFT;
        }
        else
            return '>';
    }

    c1 = c;
    c = pFI->GetFastChar();
    return( c1 );
}

#if defined( RS6000 ) || defined( HP9000 ) || defined( SCO )
extern "C" int yylex()
#else
int yylex()
#endif
{
    if( bTargetDefined )
        bTargetDefined = FALSE;
    else
        aKeyVal[ 0 ].nKeyWord =
                     MakeToken( &aKeyVal[ 0 ].aYYSType );

    yylval = aKeyVal[ 0 ].aYYSType;
    return( aKeyVal[ 0 ].nKeyWord );
}

/****************** yyerror **********************************************/
#ifdef RS6000
extern "C" void yyerror( char* pMessage )
#elif defined HP9000 || defined SCO || defined IRIX || defined SOLARIS
extern "C" void yyerror( const char* pMessage )
#else
void yyerror( char* pMessage )
#endif
{
    pTC->pEH->Error( ERR_YACC, NULL, RscId(), pMessage );
}

/****************** parser start function ********************************/
void InitParser( RscFileInst * pFileInst )
{
    pTC = pFileInst->pTypCont;          // Datenkontainer setzten
    pFI = pFileInst;
    pCS = new RscCharStack();   //Puffer Vorrat
    pExp = NULL;                //fuer MacroParser
    bTargetDefined = FALSE;

    // Anfangszeichen initialisieren
    bLastInclude = FALSE;
    c = pFI->GetFastChar();
}

void EndParser(){
    char *  pStr;

    // Stack abraeumen
    while( ! S.IsEmpty() )
        S.Pop();

    // Speicher freigeben
    while( NULL != (pStr = pCS->Pop()) )
        RscMem::Free( pStr );
    delete pCS;
    if( pExp )
        delete pExp;
    pTC      = NULL;
    pFI      = NULL;
    pCS      = NULL;
    pExp     = NULL;

}

void IncludeParser( RscFileInst * pFileInst )
{
    int           nToken;   // Wert des Tokens
    YYSTYPE       aYYSType; // Daten des Tokens
    RscFile     * pFName;   // Filestruktur
    ULONG         lKey;     // Fileschluessel
    RscTypCont  * pTypCon  = pFileInst->pTypCont;

    pFName = pTypCon->aFileTab.Get( pFileInst->GetFileIndex() );
    InitParser( pFileInst );

    nToken = MakeToken( &aYYSType );
    while( 0 != nToken && CLASSNAME != nToken ){
        if( '#' == nToken ){
            if( INCLUDE == (nToken = MakeToken( &aYYSType )) ){
                if( STRING == (nToken = MakeToken( &aYYSType )) ){
                    lKey = pTypCon->aFileTab.NewIncFile( aYYSType.string,
                                                         aYYSType.string );
                    pFName->InsertDependFile( lKey, LIST_APPEND );
                }
                else if( INCLUDE_STRING == nToken ){
                    lKey = pTypCon->aFileTab.NewIncFile( aYYSType.string,
                                                         ByteString() );
                    pFName->InsertDependFile( lKey, LIST_APPEND );
                };
            };
        };
        switch( nToken ){
            case STRING:
            case INCLUDE_STRING:
            case SYMBOL:
                // String zurueckgeben
                pCS->Push( aYYSType.string );
                break;
        };
        nToken = MakeToken( &aYYSType );
    };

    EndParser();
}

ERRTYPE parser( RscFileInst * pFileInst )
{
    ERRTYPE aError;

    InitParser( pFileInst );

    aError = yyparse();

    EndParser();

    // yyparser gibt 0 zurueck, wenn erfolgreich
    if( 0 == (USHORT)aError )
        aError.Clear();
    if( pFileInst->pTypCont->pEH->nErrors )
        aError = ERR_ERROR;
    pFileInst->SetError( aError );
    return( aError );
}

RscExpression * MacroParser( RscFileInst & rFileInst )
{
    ERRTYPE       aError;
    RscExpression * pExpression;

    InitParser( &rFileInst );

    //Ziel auf macro_expression setzen
    aKeyVal[ 0 ].nKeyWord = MACROTARGET;
    bTargetDefined = TRUE;
    aError = yyparse();

    pExpression = pExp;
    //EndParser() wuerde pExp loeschen
    if( pExp )
        pExp = NULL;

    EndParser();

    // yyparser gibt 0 zurueck, wenn erfolgreich
    if( 0 == (USHORT)aError )
        aError.Clear();
    if( rFileInst.pTypCont->pEH->nErrors )
        aError = ERR_ERROR;
    rFileInst.SetError( aError );

    //im Fehlerfall pExpression loeschen
    if( aError.IsError() && pExpression ){
        delete pExpression;
        pExpression = NULL;
    };
    return( pExpression );
}

