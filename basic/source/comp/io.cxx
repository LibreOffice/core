/*************************************************************************
 *
 *  $RCSfile: io.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:10 $
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


#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#include "sbcomp.hxx"
#pragma hdrstop
#include "iosys.hxx"

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBCOMP, SBCOMP_CODE )

// Test, ob ein I/O-Channel angegeben wurde

BOOL SbiParser::Channel( BOOL bAlways )
{
    BOOL bRes = FALSE;
    Peek();
    if( IsHash() )
    {
        SbiExpression aExpr( this );
        if( Peek() == COMMA ) Next();
        aExpr.Gen();
        aGen.Gen( _CHANNEL );
        bRes = TRUE;
    }
    else if( bAlways )
        Error( SbERR_EXPECTED, "#" );
    return bRes;
}

// Fuer PRINT und WRITE wird bei Objektvariablen versucht,
// die Default-Property anzusprechen.

// PRINT

void SbiParser::Print()
{
    BOOL bChan = Channel();
    // Die Ausdruecke zum Drucken:
    while( !bAbort )
    {
        if( !IsEoln( Peek() ) )
        {
            SbiExpression* pExpr = new SbiExpression( this );
            pExpr->Gen();
            delete pExpr;
            Peek();
            aGen.Gen( eCurTok == COMMA ? _PRINTF : _BPRINT );
        }
        if( eCurTok == COMMA || eCurTok == SEMICOLON )
        {
            Next();
            if( IsEoln( Peek() ) ) break;
        }
        else
        {
            aGen.Gen( _PRCHAR, '\n' );
            break;
        }
    }
    if( bChan )
        aGen.Gen( _CHAN0 );
}

// WRITE #chan, expr, ...

void SbiParser::Write()
{
    BOOL bChan = Channel();
    // Die Ausdruecke zum Drucken:
    while( !bAbort )
    {
        SbiExpression* pExpr = new SbiExpression( this );
        pExpr->Gen();
        delete pExpr;
        aGen.Gen( _BWRITE );
        if( Peek() == COMMA )
        {
            aGen.Gen( _PRCHAR, ',' );
            Next();
            if( IsEoln( Peek() ) ) break;
        }
        else
        {
            aGen.Gen( _PRCHAR, '\n' );
            break;
        }
    }
    if( bChan )
        aGen.Gen( _CHAN0 );
}

// LINE INPUT [prompt], var$

void SbiParser::LineInput()
{
    Channel( TRUE );
    // BOOL bChan = Channel( TRUE );
    SbiExpression* pExpr = new SbiExpression( this, SbOPERAND );
    /* AB 15.1.96: Keinen allgemeinen Ausdruck mehr zulassen
    SbiExpression* pExpr = new SbiExpression( this );
    if( !pExpr->IsVariable() )
    {
        SbiToken eTok = Peek();
        if( eTok == COMMA || eTok == SEMICOLON ) Next();
        else Error( SbERR_EXPECTED, COMMA );
        // mit Prompt
        if( !bChan )
        {
            pExpr->Gen();
            aGen.Gen( _PROMPT );
        }
        else
            Error( SbERR_VAR_EXPECTED );
        delete pExpr;
        pExpr = new SbiExpression( this, SbOPERAND );
    }
    */
    if( !pExpr->IsVariable() )
        Error( SbERR_VAR_EXPECTED );
    if( pExpr->GetType() != SbxVARIANT && pExpr->GetType() != SbxSTRING )
        Error( SbERR_CONVERSION );
    pExpr->Gen();
    aGen.Gen( _LINPUT );
    delete pExpr;
    aGen.Gen( _CHAN0 );     // ResetChannel() nicht mehr in StepLINPUT()
}

// INPUT

void SbiParser::Input()
{
    aGen.Gen( _RESTART );
    Channel( TRUE );
    // BOOL bChan = Channel( TRUE );
    SbiExpression* pExpr = new SbiExpression( this, SbOPERAND );
    /* ALT: Jetzt keinen allgemeinen Ausdruck mehr zulassen
    SbiExpression* pExpr = new SbiExpression( this );
    ...
    siehe LineInput
    */
    while( !bAbort )
    {
        if( !pExpr->IsVariable() )
            Error( SbERR_VAR_EXPECTED );
        pExpr->Gen();
        aGen.Gen( _INPUT );
        if( Peek() == COMMA )
        {
            Next();
            delete pExpr;
            pExpr = new SbiExpression( this, SbOPERAND );
        }
        else break;
    }
    delete pExpr;
    aGen.Gen( _CHAN0 );     // ResetChannel() nicht mehr in StepINPUT()
}

// OPEN stringexpr FOR mode ACCCESS access mode AS Channel [Len=n]

void SbiParser::Open()
{
    SbiExpression aFileName( this );
    SbiToken eTok;
    TestToken( FOR );
    short nMode = 0;
    short nFlags = 0;
    switch( Next() )
    {
        case INPUT:
            nMode = STREAM_READ;  nFlags |= SBSTRM_INPUT; break;
        case OUTPUT:
            nMode = STREAM_WRITE | STREAM_TRUNC; nFlags |= SBSTRM_OUTPUT; break;
        case APPEND:
            nMode = STREAM_WRITE; nFlags |= SBSTRM_APPEND; break;
        case RANDOM:
            nMode = STREAM_READ | STREAM_WRITE; nFlags |= SBSTRM_RANDOM; break;
        case BINARY:
            nMode = STREAM_READ | STREAM_WRITE; nFlags |= SBSTRM_BINARY; break;
        default:
            Error( SbERR_SYNTAX );
    }
    if( Peek() == ACCESS )
    {
        Next();
        eTok = Next();
        // #27964# Nur STREAM_READ,STREAM_WRITE-Flags in nMode beeinflussen
        nMode &= ~(STREAM_READ | STREAM_WRITE);     // loeschen
        if( eTok == READ )
        {
            if( Peek() == WRITE )
            {
                Next();
                nMode |= (STREAM_READ | STREAM_WRITE);
            }
            else
                nMode |= STREAM_READ;
        }
        else if( eTok == WRITE )
            nMode |= STREAM_WRITE;
        else
            Error( SbERR_SYNTAX );
    }
    switch( Peek() )
    {
#ifdef SHARED
#undef SHARED
#define tmpSHARED
#endif
        case SHARED:
            Next(); nMode |= STREAM_SHARE_DENYNONE; break;
#ifdef tmpSHARED
#define SHARED
#undef tmpSHARED
#endif
        case LOCK:
            Next();
            eTok = Next();
            if( eTok == READ )
            {
                if( Peek() == WRITE ) Next(), nMode |= STREAM_SHARE_DENYALL;
                else nMode |= STREAM_SHARE_DENYREAD;
            }
            else if( eTok == WRITE )
                nMode |= STREAM_SHARE_DENYWRITE;
            else
                Error( SbERR_SYNTAX );
            break;
    }
    TestToken( AS );
    // Die Kanalnummer
    SbiExpression* pChan = new SbiExpression( this );
    if( !pChan )
        Error( SbERR_SYNTAX );
    SbiExpression* pLen = NULL;
    if( Peek() == SYMBOL )
    {
        Next();
        String aLen( aSym );
        if( aLen.EqualsIgnoreCaseAscii( "LEN" ) )
        {
            TestToken( EQ );
            pLen = new SbiExpression( this );
        }
    }
    if( !pLen ) pLen = new SbiExpression( this, 128, SbxINTEGER );
    // Der Stack fuer den OPEN-Befehl sieht wie folgt aus:
    // Blocklaenge
    // Kanalnummer
    // Dateiname
    pLen->Gen();
    if( pChan )
        pChan->Gen();
    aFileName.Gen();
    aGen.Gen( _OPEN, nMode, nFlags );
    delete pLen;
    delete pChan;
}

// NAME file AS file

void SbiParser::Name()
{
    SbiExpression aExpr1( this );
    TestToken( AS );
    SbiExpression aExpr2( this );
    aExpr1.Gen();
    aExpr2.Gen();
    aGen.Gen( _RENAME );
}

// CLOSE [n,...]

void SbiParser::Close()
{
    Peek();
    if( IsEoln( eCurTok ) )
        aGen.Gen( _CLOSE, 0 );
    else
    for( ;; )
    {
        if( Channel( TRUE ) )
            aGen.Gen( _CLOSE, 1 );
        else
            break;
        if( IsEoln( Peek() ) )
            break;
    }
}


