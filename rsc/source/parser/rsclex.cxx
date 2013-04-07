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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#ifdef _RSCERROR_H
#include <rscerror.h>
#endif
#include <rschash.hxx>
#include <rscdb.hxx>
#include <rsctop.hxx>
#include <rsckey.hxx>
#include <rscpar.hxx>
#include <rscdef.hxx>

#include <rsclex.hxx>
#include <rscyacc.hxx>

#include <rtl/textcvt.h>
#include <rtl/textenc.h>


const char* StringContainer::putString( const char* pString )
{
    OString aString( static_cast<const sal_Char*>(pString) );
    std::pair<
        boost::unordered_set< OString, OStringHash >::iterator,
        bool > aInsert =
        m_aStrings.insert( aString );

    return aInsert.first->getStr();
}

int             c;
sal_Bool            bLastInclude;// War letztes Symbol INCLUDE
RscFileInst*    pFI;
RscTypCont*     pTC;
RscExpression * pExp;
struct KeyVal {
    int     nKeyWord;
    YYSTYPE aYYSType;
} aKeyVal[ 1 ];
sal_Bool bTargetDefined;

StringContainer* pStringContainer = NULL;


sal_uInt32 GetNumber(){
    sal_uInt32  l = 0;
    sal_uInt32  nLog = 10;

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

    if( l > 0x7fffffff ) //Oberstes bit gegebenenfalls abschneiden;
        l &= 0x7fffffff;

    return( l );
}

int MakeToken( YYSTYPE * pTokenVal ){
    int             c1;

    while( true ){  // Kommentare und Leerzeichen ueberlesen
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
        bLastInclude = sal_False; //Zuruecksetzten
        if( '<' == c ){
            OStringBuffer aBuf( 256 );
            c = pFI->GetFastChar();
            while( '>' != c && !pFI->IsEof() )
            {
                aBuf.append( sal_Char(c) );
                c = pFI->GetFastChar();
            };
            c = pFI->GetFastChar();
            pTokenVal->string = const_cast<char*>(pStringContainer->putString( aBuf.getStr() ));
            return( INCLUDE_STRING );
        };
    }

    if( c == '"' )
    {
        OStringBuffer aBuf( 256 );
        sal_Bool bDone = sal_False;
        while( !bDone && !pFI->IsEof() && c )
        {
            c = pFI->GetFastChar();
            if( c == '"' )
            {
                do
                {
                    c = pFI->GetFastChar();
                }
                while(  c == ' ' || c == '\t' );
                if( c == '"' )
                {
                    // this is a continued string
                    // note: multiline string continuations are handled by the parser
                    // see rscyacc.y
                }
                else
                    bDone = sal_True;
            }
            else if( c == '\\' )
            {
                aBuf.append( '\\' );
                c = pFI->GetFastChar();
                if( c )
                    aBuf.append( sal_Char(c) );
            }
            else
                aBuf.append( sal_Char(c) );
        }
        pTokenVal->string = const_cast<char*>(pStringContainer->putString( aBuf.getStr() ));
        return( STRING );
    }
    if (isdigit (c)){
        pTokenVal->value = GetNumber();
        return( NUMBER );
    }

    if( isalpha (c) || (c == '_') ){
        Atom        nHashId;
        OStringBuffer aBuf( 256 );

        while( isalnum (c) || (c == '_') || (c == '-') )
        {
            aBuf.append( sal_Char(c) );
            c = pFI->GetFastChar();
        }

        nHashId = pHS->getID( aBuf.getStr(), true );
        if( InvalidAtom != nHashId )
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
                        pTokenVal->svbool = (sal_Bool)aKey.yylval;
                        break;
                    case INCLUDE:
                        bLastInclude = sal_True;
                    default:
                        pTokenVal->value = aKey.yylval;
                };

                return( aKey.nTyp );
            }
            else
            {
                pTokenVal->string = const_cast<char*>(pStringContainer->putString( aBuf.getStr() ));
                return( SYMBOL );
            }
        }
        else{       // Symbol
            RscDefine  * pDef;

            pDef = pTC->aFileTab.FindDef( aBuf.getStr() );
            if( pDef ){
                pTokenVal->defineele = pDef;

                return( RSCDEFINE );
            }

            pTokenVal->string = const_cast<char*>(pStringContainer->putString( aBuf.getStr() ));
            return( SYMBOL );
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

#if defined( RS6000 )
extern "C" int yylex()
#else
int yylex()
#endif
{
    if( bTargetDefined )
        bTargetDefined = sal_False;
    else
        aKeyVal[ 0 ].nKeyWord =
                     MakeToken( &aKeyVal[ 0 ].aYYSType );

    yylval = aKeyVal[ 0 ].aYYSType;
    return( aKeyVal[ 0 ].nKeyWord );
}

#ifdef RS6000
extern "C" void yyerror( char* pMessage )
#elif defined SOLARIS
extern "C" void yyerror( const char* pMessage )
#else
void yyerror( char* pMessage )
#endif
{
    pTC->pEH->Error( ERR_YACC, NULL, RscId(), pMessage );
}

void InitParser( RscFileInst * pFileInst )
{
    pTC = pFileInst->pTypCont;          // Datenkontainer setzten
    pFI = pFileInst;
    pStringContainer = new StringContainer();
    pExp = NULL;                //fuer MacroParser
    bTargetDefined = sal_False;

    // Anfangszeichen initialisieren
    bLastInclude = sal_False;
    c = pFI->GetFastChar();
}

void EndParser(){
    // Stack abraeumen
    while( ! S.IsEmpty() )
        S.Pop();

    // free string container
    delete pStringContainer;
    pStringContainer = NULL;

    if( pExp )
        delete pExp;
    pTC      = NULL;
    pFI      = NULL;
    pExp     = NULL;

}

void IncludeParser( RscFileInst * pFileInst )
{
    int           nToken;   // Wert des Tokens
    YYSTYPE       aYYSType; // Daten des Tokens
    RscFile     * pFName;   // Filestruktur
    sal_uLong         lKey;     // Fileschluessel
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
                    pFName->InsertDependFile( lKey, ULONG_MAX );
                }
                else if( INCLUDE_STRING == nToken ){
                    lKey = pTypCon->aFileTab.NewIncFile( aYYSType.string,
                                                         OString() );
                    pFName->InsertDependFile( lKey, ULONG_MAX );
                };
            };
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
    if( 0 == aError )
        aError.Clear();
    if( pFileInst->pTypCont->pEH->nErrors )
        aError = ERR_ERROR;
    pFileInst->SetError( aError );
    return( aError );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
