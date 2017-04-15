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

#include <rscerror.h>
#include <rschash.hxx>
#include <rscdb.hxx>
#include <rsctop.hxx>
#include <rsckey.hxx>
#include <rscpar.hxx>
#include <rscdef.hxx>

#include <rsclex.hxx>
#include <rscyacc.hxx>

#include <rtl/character.hxx>
#include <rtl/textcvt.h>
#include <rtl/textenc.h>


const char* StringContainer::putString( const char* pString )
{
    OString aString( pString );
    std::pair<
        std::unordered_set< OString, OStringHash >::iterator,
        bool > aInsert =
        m_aStrings.insert( aString );

    return aInsert.first->getStr();
}

static char     c;
static bool     bLastInclude;//  true, if last symbol was INCLUDE
RscFileInst*    pFI;
RscTypCont*     pTC;
RscExpression * pExp;
struct KeyVal
{
    int     nKeyWord;
    YYSTYPE aYYSType;
};
static KeyVal aKeyVal[ 1 ];
static bool bTargetDefined;

StringContainer* pStringContainer = nullptr;

static RscDefine RSC_GLOBAL_DEFINE(RscFileTab::Index(0), OString("__RSC"), 1);

sal_uInt32 GetNumber()
{
    sal_uInt32  l = 0;
    sal_uInt32  nLog = 10;

    if( '0' == c )
    {
        c = pFI->GetFastChar();
        if( 'x' == c )
        {
            nLog = 16;
            c = pFI->GetFastChar();
        }
    }

    if( nLog == 16 )
    {
        while( rtl::isAsciiHexDigit( static_cast<unsigned char>(c) ) )
        {
            if( rtl::isAsciiDigit( static_cast<unsigned char>(c) ) )
                l = l * nLog + (c - '0');
            else
                l = l * nLog + (rtl::toAsciiUpperCase( static_cast<sal_uInt32>(c) ) - 'A' + 10 );

            c = pFI->GetFastChar();
        }
    }
    else
    {
        while( rtl::isAsciiDigit( static_cast<unsigned char>(c) ) || 'x' == c )
        {
            l = l * nLog + (c - '0');
            c = pFI->GetFastChar();
        }
    }

    while( c=='U' || c=='u' || c=='l' || c=='L' ) // because of unsigned longs
        c = pFI->GetFastChar();

    if( l > 0x7fffffff ) // drop the most significant bit if needed;
        l &= 0x7fffffff;

    return l;
}

int MakeToken( YYSTYPE * pTokenVal )
{
    int             c1;

    while( true ) // ignore comments and space characters
    {
        while( rtl::isAsciiWhiteSpace( static_cast<unsigned char>(c) ) )
            c = pFI->GetFastChar();

        if( '/' == c )
        {
            c1 = c;
            c = pFI->GetFastChar();
            if( '/' == c )
            {
                while( '\n' != c && !pFI->IsEof() )
                    c = pFI->GetFastChar();

                c = pFI->GetFastChar();
            }
            else if( '*' == c )
            {
                c = pFI->GetFastChar();
                do
                {
                    while( '*' != c && !pFI->IsEof() )
                        c = pFI->GetFastChar();

                    c = pFI->GetFastChar();
                }
                while( '/' != c && !pFI->IsEof() );
                c = pFI->GetFastChar();
            }
            else
                return c1;
        }
        else
            break;
    }

    // FIXME: wtf is this supposed to do?
    if( (c != 0) == pFI->IsEof() )
    {
        return 0;
    }

    if( bLastInclude )
    {
        bLastInclude = false; // reset
        if( '<' == c )
        {
            OStringBuffer aBuf( 256 );
            c = pFI->GetFastChar();
            while( '>' != c && !pFI->IsEof() )
            {
                aBuf.append( c );
                c = pFI->GetFastChar();
            }
            c = pFI->GetFastChar();
            pTokenVal->string = const_cast<char*>(pStringContainer->putString( aBuf.getStr() ));
            return INCLUDE_STRING;
        }
    }

    if( c == '"' )
    {
        OStringBuffer aBuf( 256 );
        bool bDone = false;
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
                    bDone = true;
            }
            else if( c == '\\' )
            {
                aBuf.append( '\\' );
                c = pFI->GetFastChar();
                if( c )
                    aBuf.append( c );
            }
            else
                aBuf.append( c );
        }
        pTokenVal->string = const_cast<char*>(pStringContainer->putString( aBuf.getStr() ));
        return STRING;
    }
    if (rtl::isAsciiDigit (static_cast<unsigned char>(c)))
    {
        pTokenVal->value = GetNumber();
        return NUMBER;
    }

    if( rtl::isAsciiAlpha (static_cast<unsigned char>(c)) || (c == '_') )
    {
        Atom        nHashId;
        OStringBuffer aBuf( 256 );

        while( rtl::isAsciiAlphanumeric (static_cast<unsigned char>(c))
               || (c == '_') || (c == '-') || (c == ':'))
        {
            aBuf.append( c );
            c = pFI->GetFastChar();
        }

        nHashId = pHS->getID( aBuf.getStr(), true );
        if( InvalidAtom != nHashId )
        {
            KEY_STRUCT  aKey;

            // search for keyword
            if( pTC->aNmTb.Get( nHashId, &aKey ) )
            {

                // keyword found
                switch( aKey.nTyp )
                {
                case CLASSNAME:
                    pTokenVal->pClass = reinterpret_cast<RscTop *>(aKey.yylval);
                    break;
                case VARNAME:
                    pTokenVal->varid = aKey.nName;
                    break;
                case CONSTNAME:
                    pTokenVal->constname.hashid = aKey.nName;
                    pTokenVal->constname.nValue = aKey.yylval;
                    break;
                case BOOLEAN:
                    pTokenVal->svbool = (bool)aKey.yylval;
                    break;
                case INCLUDE:
                    bLastInclude = true;
                    SAL_FALLTHROUGH;
                default:
                    pTokenVal->value = aKey.yylval;
                }

                return aKey.nTyp;
            }
            else
            {
                pTokenVal->string = const_cast<char*>(pStringContainer->putString( aBuf.getStr() ));
                return SYMBOL;
            }
        }
        else
        {
            // Symbol
            RscDefine  * pDef;

            // this #define symbol is used to indicate to various code that it is being processed with the RSC compiler
            if (strcmp(aBuf.getStr(), "__RSC") == 0)
                pDef = &RSC_GLOBAL_DEFINE;
            else
                pDef = pTC->aFileTab.FindDef( aBuf.getStr() );
            if( pDef )
            {
                pTokenVal->defineele = pDef;

                return RSCDEFINE;
            }

            pTokenVal->string = const_cast<char*>(pStringContainer->putString( aBuf.getStr() ));
            return SYMBOL;
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
    return c1;
}

int yylex()
{
    if( bTargetDefined )
        bTargetDefined = false;
    else
        aKeyVal[ 0 ].nKeyWord = MakeToken( &aKeyVal[ 0 ].aYYSType );

    yylval = aKeyVal[ 0 ].aYYSType;
    return aKeyVal[ 0 ].nKeyWord;
}

#if defined __sun
extern "C" void yyerror( const char* pMessage )
#else
void yyerror( char* pMessage )
#endif
{
    pTC->pEH->Error( ERR_YACC, nullptr, RscId(), pMessage );
}

void InitParser( RscFileInst * pFileInst )
{
    pTC = pFileInst->pTypCont;          // set file container
    pFI = pFileInst;
    pStringContainer = new StringContainer();
    pExp = nullptr;                // for macro parser
    bTargetDefined = false;

    // initialize first character
    bLastInclude = false;
    c = pFI->GetFastChar();
}

void EndParser()
{
    // empty stack
    while( ! S.IsEmpty() )
        S.Pop();

    // free string container
    delete pStringContainer;
    pStringContainer = nullptr;

    delete pExp;
    pTC      = nullptr;
    pFI      = nullptr;
    pExp     = nullptr;

}

void IncludeParser( RscFileInst * pFileInst )
{
    int           nToken;   // token value
    YYSTYPE       aYYSType; // token data
    RscFile     * pFName;   // file structure
    RscFileTab::Index lKey;     // file key
    RscTypCont  * pTypCon  = pFileInst->pTypCont;

    pFName = pTypCon->aFileTab.Get( pFileInst->GetFileIndex() );
    InitParser( pFileInst );

    nToken = MakeToken( &aYYSType );
    while( 0 != nToken && CLASSNAME != nToken )
    {
        if( '#' == nToken )
        {
            if( INCLUDE == (nToken = MakeToken( &aYYSType )) )
            {
                if( STRING == (nToken = MakeToken( &aYYSType )) )
                {
                    lKey = pTypCon->aFileTab.NewIncFile( aYYSType.string,
                                                         aYYSType.string );
                    pFName->InsertDependFile( lKey );
                }
                else if( INCLUDE_STRING == nToken )
                {
                    lKey = pTypCon->aFileTab.NewIncFile( aYYSType.string,
                                                         OString() );
                    pFName->InsertDependFile( lKey );
                }
            }
        }
        nToken = MakeToken( &aYYSType );
    }

    EndParser();
}

ERRTYPE parser( RscFileInst * pFileInst )
{
    ERRTYPE aError;

    InitParser( pFileInst );

    aError = yyparse();

    EndParser();

    // yyparser returns 0 on success
    if( 0 == aError.GetError() )
        aError.Clear();
    if( pFileInst->pTypCont->pEH->nErrors )
        aError = ERR_ERROR;
    pFileInst->SetError( aError );
    return aError;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
