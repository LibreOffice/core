/*************************************************************************
 *
 *  $RCSfile: parser.cxx,v $
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

#if SUPD >= 375
#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#else
#include <svmem.hxx>
#include <sbx.hxx>
#endif
#include "sbcomp.hxx"
#pragma hdrstop

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBCOMP, SBCOMP_CODE )

struct SbiParseStack {              // "Stack" fuer Statement-Blocks
    SbiParseStack* pNext;           // Chain
    SbiExprNode* pWithVar;          // Variable fuer WITH
    SbiToken eExitTok;              // Exit-Token
    USHORT  nChain;                 // JUMP-Chain
};

struct SbiStatement {
    SbiToken eTok;
    void( SbiParser::*Func )();     // Verarbeitungsroutine
    BOOL  bMain;                    // TRUE: ausserhalb SUBs OK
    BOOL  bSubr;                    // TRUE: in SUBs OK
    BOOL  bVBScript;                // TRUE: in  OK
};

#define Y   TRUE
#define N   FALSE

static SbiStatement StmntTable [] = {
{ CALL,     &SbiParser::Call,       N, Y, Y, }, // CALL
{ CLOSE,    &SbiParser::Close,      N, Y, Y, }, // CLOSE
{ _CONST_,  &SbiParser::Dim,        Y, Y, Y, }, // CONST
{ DECLARE,  &SbiParser::Declare,    Y, N, N, }, // DECLARE
{ DEFBOOL,  &SbiParser::DefXXX,     Y, N, N, }, // DEFBOOL
{ DEFCUR,   &SbiParser::DefXXX,     Y, N, N, }, // DEFCUR
{ DEFDATE,  &SbiParser::DefXXX,     Y, N, N, }, // DEFDATE
{ DEFDBL,   &SbiParser::DefXXX,     Y, N, N, }, // DEFDBL
{ DEFERR,   &SbiParser::DefXXX,     Y, N, N, }, // DEFERR
{ DEFINT,   &SbiParser::DefXXX,     Y, N, N, }, // DEFINT
{ DEFLNG,   &SbiParser::DefXXX,     Y, N, N, }, // DEFLNG
{ DEFOBJ,   &SbiParser::DefXXX,     Y, N, N, }, // DEFOBJ
{ DEFSNG,   &SbiParser::DefXXX,     Y, N, N, }, // DEFSNG
{ DEFSTR,   &SbiParser::DefXXX,     Y, N, N, }, // DEFSTR
{ DEFVAR,   &SbiParser::DefXXX,     Y, N, N, }, // DEFVAR
{ DIM,      &SbiParser::Dim,        Y, Y, Y, }, // DIM
{ DO,       &SbiParser::DoLoop,     N, Y, Y, }, // DO
{ ELSE,     &SbiParser::NoIf,       N, Y, Y, }, // ELSE
{ ELSEIF,   &SbiParser::NoIf,       N, Y, Y, }, // ELSEIF
{ ENDIF,    &SbiParser::NoIf,       N, Y, Y, }, // ENDIF
{ END,      &SbiParser::Stop,       N, Y, N, }, // END
{ ERASE,    &SbiParser::Erase,      N, Y, Y, }, // ERASE
{ _ERROR_,  &SbiParser::ErrorStmnt, N, Y, Y, }, // ERROR
{ EXIT,     &SbiParser::Exit,       N, Y, N, }, // EXIT
{ FOR,      &SbiParser::For,        N, Y, Y, }, // FOR
{ FUNCTION, &SbiParser::SubFunc,    Y, N, Y, }, // FUNCTION
{ GOSUB,    &SbiParser::Goto,       N, Y, N, }, // GOSUB
{ GLOBAL,   &SbiParser::Dim,        Y, N, Y, }, // GLOBAL
{ GOTO,     &SbiParser::Goto,       N, Y, N, }, // GOTO
{ IF,       &SbiParser::If,         N, Y, Y, }, // IF
{ INPUT,    &SbiParser::Input,      N, Y, Y, }, // INPUT
{ LET,      &SbiParser::Assign,     N, Y, Y, }, // LET
{ LINEINPUT,&SbiParser::LineInput,  N, Y, Y, }, // LINE INPUT
{ LOOP,     &SbiParser::BadBlock,   N, Y, Y, }, // LOOP
{ LSET,     &SbiParser::LSet,       N, Y, N, }, // LSET
{ NAME,     &SbiParser::Name,       N, Y, Y, }, // NAME
{ NEXT,     &SbiParser::BadBlock,   N, Y, Y, }, // NEXT
{ ON,       &SbiParser::On,         N, Y, N, }, // ON
{ OPEN,     &SbiParser::Open,       N, Y, Y, }, // OPEN
{ OPTION,   &SbiParser::Option,     Y, N, N, }, // OPTION
{ PRINT,    &SbiParser::Print,      N, Y, Y, }, // PRINT
{ PRIVATE,  &SbiParser::Dim,        Y, N, Y, }, // PRIVATE
{ PUBLIC,   &SbiParser::Dim,        Y, N, Y, }, // PUBLIC
{ REDIM,    &SbiParser::ReDim,      N, Y, Y, }, // DIM
{ RESUME,   &SbiParser::Resume,     N, Y, N, }, // RESUME
{ RETURN,   &SbiParser::Return,     N, Y, N, }, // RETURN
{ RSET,     &SbiParser::RSet,       N, Y, N, }, // RSET
{ SELECT,   &SbiParser::Select,     N, Y, N, }, // SELECT
{ SET,      &SbiParser::Set,        N, Y, Y, }, // SET
{ STATIC,   &SbiParser::Static,     Y, Y, Y, }, // STATIC
{ STOP,     &SbiParser::Stop,       N, Y, N, }, // STOP
{ SUB,      &SbiParser::SubFunc,    Y, N, Y, }, // SUB
{ TYPE,     &SbiParser::Type,       Y, N, Y, }, // TYPE
{ UNTIL,    &SbiParser::BadBlock,   N, Y, Y, }, // UNTIL
{ WHILE,    &SbiParser::While,      N, Y, Y, }, // WHILE
{ WEND,     &SbiParser::BadBlock,   N, Y, Y, }, // WEND
{ WITH,     &SbiParser::With,       N, Y, N, }, // WITH
{ WRITE,    &SbiParser::Write,      N, Y, Y, }, // WRITE

{ NIL }
};


#ifdef MSC
// 'this' : used in base member initializer list
#pragma warning( disable: 4355 )
#endif

SbiParser::SbiParser( StarBASIC* pb, SbModule* pm )
        : SbiTokenizer( pm->GetSource(), pb ),
          aGblStrings( this ),
          aLclStrings( this ),
          aPublics( aGblStrings, SbPUBLIC ),
          aGlobals( aGblStrings, SbGLOBAL ),
          aRtlSyms( aGblStrings, SbRTL ),
          aGen( *pm, this, 1024 )
{
    pBasic   = pb;
    eCurExpr = SbSYMBOL;
    eEndTok  = NIL;
    pProc    = NULL;
    pStack   = NULL;
    pWithVar = NULL;
    nBase    = 0;
    bText    =
    bGblDefs =
    bNewGblDefs =
    bSingleLineIf =
    bExplicit = FALSE;
    pPool    = &aPublics;
    for( short i = 0; i < 26; i++ )
        eDefTypes[ i ] = SbxVARIANT;    // Kein expliziter Defaulttyp

    aPublics.SetParent( &aGlobals );
    aGlobals.SetParent( &aRtlSyms );

    // Die globale Chainkette faengt bei Adresse 0 an:
    nGblChain = aGen.Gen( _JUMP, 0 );

    rTypeArray = new SbxArray; // Array fuer Benutzerdefinierte Typen
}


// Ist  Teil der Runtime-Library?
SbiSymDef* SbiParser::CheckRTLForSym( const String& rSym, SbxDataType eType )
{
    SbxVariable* pVar = GetBasic()->GetRtl()->Find( rSym, SbxCLASS_DONTCARE );
    SbiSymDef* pDef = NULL;
    if( pVar )
    {
        if( pVar->IsA( TYPE(SbxMethod) ) )
        {
            SbiProcDef* pProc = aRtlSyms.AddProc( rSym );
            pProc->SetType( pVar->GetType() );
            pDef = pProc;
        }
        else
        {
            pDef = aRtlSyms.AddSym( rSym );
            pDef->SetType( eType );
        }
    }
    return pDef;
}

// Globale Chainkette schliessen

BOOL SbiParser::HasGlobalCode()
{
    if( bGblDefs && nGblChain )
    {
        aGen.BackChain( nGblChain );
        aGen.Gen( _LEAVE );
        // aGen.Gen( _STOP );
        nGblChain = 0;
    }
    return bGblDefs;
}

void SbiParser::OpenBlock( SbiToken eTok, SbiExprNode* pVar )
{
    SbiParseStack* p = new SbiParseStack;
    p->eExitTok = eTok;
    p->nChain   = 0;
    p->pWithVar = pWithVar;
    p->pNext    = pStack;
    pStack      = p;
    pWithVar    = pVar;

    // #29955 for-Schleifen-Ebene pflegen
    if( eTok == FOR )
        aGen.IncForLevel();
}

void SbiParser::CloseBlock()
{
    if( pStack )
    {
        SbiParseStack* p = pStack;

        // #29955 for-Schleifen-Ebene pflegen
        if( p->eExitTok == FOR )
            aGen.DecForLevel();

        aGen.BackChain( p->nChain );
        pStack = p->pNext;
        pWithVar = p->pWithVar;
        delete p;
    }
}

// EXIT ...

void SbiParser::Exit()
{
    SbiToken eTok = Next();
    for( SbiParseStack* p = pStack; p; p = p->pNext )
    {
        if( eTok == p->eExitTok )
        {
            p->nChain = aGen.Gen( _JUMP, p->nChain );
            return;
        }
    }
    if( pStack )
        Error( SbERR_EXPECTED, pStack->eExitTok );
    else
        Error( SbERR_BAD_EXIT );
}

BOOL SbiParser::TestSymbol( BOOL bKwdOk )
{
    Peek();
    if( eCurTok == SYMBOL || ( bKwdOk && IsKwd( eCurTok ) ) )
    {
        Next(); return TRUE;
    }
    Error( SbERR_SYMBOL_EXPECTED );
    return FALSE;
}

// Testen auf ein bestimmtes Token

BOOL SbiParser::TestToken( SbiToken t )
{
    if( Peek() == t )
    {
        Next(); return TRUE;
    }
    else
    {
        Error( SbERR_EXPECTED, t );
        return FALSE;
    }
}

// Testen auf Komma oder EOLN

BOOL SbiParser::TestComma()
{
    SbiToken eTok = Peek();
    if( IsEoln( eTok ) )
    {
        Next();
        return FALSE;
    }
    else if( eTok != COMMA )
    {
        Error( SbERR_EXPECTED, COMMA );
        return FALSE;
    }
    Next();
    return TRUE;
}

// Testen, ob EOLN vorliegt

void SbiParser::TestEoln()
{
    if( !IsEoln( Next() ) )
    {
        Error( SbERR_EXPECTED, EOLN );
        while( !IsEoln( Next() ) ) {}
    }
}

// Parsing eines Statement-Blocks
// Das Parsing laeuft bis zum Ende-Token.

void SbiParser::StmntBlock( SbiToken eEnd )
{
    SbiToken xe = eEndTok;
    eEndTok = eEnd;
    while( !bAbort && Parse() ) {}
    eEndTok = xe;
    if( IsEof() )
    {
        Error( SbERR_BAD_BLOCK, eEnd );
        bAbort = TRUE;
    }
}

// Die Hauptroutine. Durch wiederholten Aufrufs dieser Routine wird
// die Quelle geparst. Returnwert FALSE bei Ende/Fehlern.

BOOL SbiParser::Parse()
{
    if( bAbort ) return FALSE;

    EnableErrors();

    Peek();
    // Dateiende?
    if( IsEof() )
    {
        // AB #33133: Falls keine Sub angelegt wurde, muss hier
        // der globale Chain abgeschlossen werden!
        // AB #40689: Durch die neue static-Behandlung kann noch
        // ein nGblChain vorhanden sein, daher vorher abfragen
        if( bNewGblDefs && nGblChain == 0 )
            nGblChain = aGen.Gen( _JUMP, 0 );
        return FALSE;
    }

    // Leerstatement?
    if( IsEoln( eCurTok ) )
    {
        Next(); return TRUE;
    }

    if( !bSingleLineIf && MayBeLabel( TRUE ) )
    {
        // Ist ein Label
        if( !pProc )
            Error( SbERR_NOT_IN_MAIN, aSym );
        else
            pProc->GetLabels().Define( aSym );
        Next(); Peek();
        // Leerstatement?
        if( IsEoln( eCurTok ) )
        {
            Next(); return TRUE;
        }
    }

    // Ende des Parsings?
    if( eCurTok == eEndTok )
    {
        Next();
        if( eCurTok != NIL )
            aGen.Statement();
        return FALSE;
    }

    // Kommentar?
    if( eCurTok == REM )
    {
        Next(); return TRUE;
    }

    // Kommt ein Symbol, ist es entweder eine Variable( LET )
    // oder eine SUB-Prozedur( CALL ohne Klammern )
    // DOT fuer Zuweisungen im WITH-Block: .A=5
    if( eCurTok == SYMBOL || eCurTok == DOT )
    {
#ifndef VBSCRIPT_TEST
        if( !pProc )
            Error( SbERR_EXPECTED, SUB );
        else
#endif
        {
            // Damit Zeile & Spalte stimmen...
            Next();
            Push( eCurTok );
            aGen.Statement();
            Symbol();
        }
    }
    else
    {
        Next();

        // Hier folgen nun die Statement-Parser.

        SbiStatement* p;
        for( p = StmntTable; p->eTok != NIL; p++ )
            if( p->eTok == eCurTok )
                break;
        if( p->eTok != NIL )
        {
#ifdef VBSCRIPT_TEST
            if( !p->bVBScript )
                Error( ERRCODE_BASIC_NOT_IN_VBSCRIPT );
            else
#endif
#ifndef VBSCRIPT_TEST
            if( !pProc && !p->bMain )
                Error( SbERR_NOT_IN_MAIN, eCurTok );
            else
#endif
            if( pProc && !p->bSubr )
                Error( SbERR_NOT_IN_SUBR, eCurTok );
            else
            {
                // globalen Chain pflegen
                // AB #41606/#40689: Durch die neue static-Behandlung kann noch
                // ein nGblChain vorhanden sein, daher vorher abfragen
                if( bNewGblDefs && ( eCurTok == SUB || eCurTok == FUNCTION ) && nGblChain == 0 )
                {
                    nGblChain = aGen.Gen( _JUMP, 0 );
                    bNewGblDefs = FALSE;
                }
                // Statement-Opcode bitte auch am Anfang einer Sub
                if( ( p->bSubr && (eCurTok != STATIC || Peek() == SUB || Peek() == FUNCTION ) ) ||
                        eCurTok == SUB || eCurTok == FUNCTION )
                    aGen.Statement();
                (this->*( p->Func ) )();
                SbxError nSbxErr = SbxBase::GetError();
                if( nSbxErr )
                    SbxBase::ResetError(), Error( (SbError)nSbxErr );
            }
        }
        else
            Error( SbERR_UNEXPECTED, eCurTok );
    }

    // Test auf Ende des Statements:
    // Kann auch ein ELSE sein, da vor dem ELSE kein : stehen muss!

    if( !IsEos() )
    {
        Peek();
        if( !IsEos() && eCurTok != ELSE )
        {
            // falls das Parsing abgebrochen wurde, bis zum ":" vorgehen:
            Error( SbERR_UNEXPECTED, eCurTok );
            while( !IsEos() ) Next();
        }
    }
    // Der Parser bricht am Ende ab, das naechste Token ist noch nicht
    // geholt!
    return TRUE;
}

// Innerste With-Variable liefern
SbiExprNode* SbiParser::GetWithVar()
{
    if( pWithVar )
        return pWithVar;

    // Sonst im Stack suchen
    SbiParseStack* p = pStack;
    while( p )
    {
        // LoopVar kann zur Zeit nur fuer with sein
        if( p->pWithVar )
            return p->pWithVar;
        p = p->pNext;
    }
    return NULL;
}


// Zuweisung oder Subroutine Call

void SbiParser::Symbol()
{
    SbiExpression aVar( this, SbSYMBOL );
    aVar.Gen();
    if( Peek() != EQ )
    {
        aGen.Gen( _GET );
    }
    else
    {
        // Dann muss es eine Zuweisung sein. Was anderes gibts nicht!
        if( !aVar.IsLvalue() )
            Error( SbERR_LVALUE_EXPECTED );
        TestToken( EQ );
        SbiExpression aExpr( this );
        aExpr.Gen();
        SbiOpcode eOp = _PUT;
        SbiSymDef* pDef = aVar.GetRealVar();
        if( pDef )
        {
            if( pDef->GetConstDef() )
                Error( SbERR_DUPLICATE_DEF, pDef->GetName() );
            if( pDef->GetType() == SbxOBJECT )
            {
                eOp = _SET;
                if( pDef->GetTypeId() )
//              if( pDef->GetTypeId() && !pDef->HabIchAlsTypeDefiniert() )
                    aGen.Gen( _CLASS, pDef->GetTypeId() );
                // x = Objektfunktion (ohne Set) is nich drin
//              Error( SbERR_SYNTAX );
            }
        }
        aGen.Gen( eOp );
    }
}

// Zuweisungen

void SbiParser::Assign()
{
    SbiExpression aLvalue( this, SbLVALUE );
    TestToken( EQ );
    SbiExpression aExpr( this );
    aLvalue.Gen();
    aExpr.Gen();
    USHORT nLen = 0;
    SbiSymDef* pDef = aLvalue.GetRealVar();
    {
        if( pDef->GetConstDef() )
            Error( SbERR_DUPLICATE_DEF, pDef->GetName() );
        nLen = aLvalue.GetRealVar()->GetLen();
    }
    if( nLen )
        aGen.Gen( _PAD, nLen );
    aGen.Gen( _PUT );
}

// Zuweisungen einer Objektvariablen

void SbiParser::Set()
{
    SbiExpression aLvalue( this, SbLVALUE );
    if( aLvalue.GetType() != SbxOBJECT )
        Error( SbERR_INVALID_OBJECT );
    TestToken( EQ );
    SbiSymDef* pDef = aLvalue.GetRealVar();
    if( pDef && pDef->GetConstDef() )
        Error( SbERR_DUPLICATE_DEF, pDef->GetName() );
    SbiExpression aExpr( this );
    aLvalue.Gen();
    aExpr.Gen();
    if( pDef->GetTypeId() )
//      if( pDef->GetTypeId() && !pDef->HabIchAlsTypeDefiniert() )
        aGen.Gen( _CLASS, pDef->GetTypeId() );
    aGen.Gen( _SET );
}

// JSM 07.10.95
void SbiParser::LSet()
{
    SbiExpression aLvalue( this, SbLVALUE );
    if( aLvalue.GetType() != SbxSTRING )
        Error( SbERR_INVALID_OBJECT );
    TestToken( EQ );
    SbiSymDef* pDef = aLvalue.GetRealVar();
    if( pDef && pDef->GetConstDef() )
        Error( SbERR_DUPLICATE_DEF, pDef->GetName() );
    SbiExpression aExpr( this );
    aLvalue.Gen();
    aExpr.Gen();
    aGen.Gen( _LSET );
}

// JSM 07.10.95
void SbiParser::RSet()
{
    SbiExpression aLvalue( this, SbLVALUE );
    if( aLvalue.GetType() != SbxSTRING )
        Error( SbERR_INVALID_OBJECT );
    TestToken( EQ );
    SbiSymDef* pDef = aLvalue.GetRealVar();
    if( pDef && pDef->GetConstDef() )
        Error( SbERR_DUPLICATE_DEF, pDef->GetName() );
    SbiExpression aExpr( this );
    aLvalue.Gen();
    aExpr.Gen();
    aGen.Gen( _RSET );
}

// DEFINT, DEFLNG, DEFSNG, DEFDBL, DEFSTR und so weiter

void SbiParser::DefXXX()
{
    sal_Unicode ch1, ch2;
    SbxDataType t = SbxDataType( eCurTok - DEFINT + SbxINTEGER );

    while( !bAbort )
    {
        if( Next() != SYMBOL ) break;
        ch1 = aSym.ToUpperAscii().GetBuffer()[0];
        ch2 = 0;
        if( Peek() == MINUS )
        {
            Next();
            if( Next() != SYMBOL ) Error( SbERR_SYMBOL_EXPECTED );
            else
            {
                ch2 = aSym.ToUpperAscii().GetBuffer()[0];
                //ch2 = aSym.Upper();
                if( ch2 < ch1 ) Error( SbERR_SYNTAX ), ch2 = 0;
            }
        }
        if (!ch2) ch2 = ch1;
        ch1 -= 'A'; ch2 -= 'A';
        for (; ch1 <= ch2; ch1++) eDefTypes[ ch1 ] = t;
        if( !TestComma() ) break;
    }
}

// STOP/SYSTEM

void SbiParser::Stop()
{
    aGen.Gen( _STOP );
    Peek();     // #35694: Nur Peek(), damit EOL in Single-Line-If erkannt wird
}

// OPTION

void SbiParser::Option()
{
    switch( Next() )
    {
        case EXPLICIT:
            bExplicit = TRUE; break;
        case BASE:
            if( Next() == NUMBER )
            {
                if( nVal == 0 || nVal == 1 )
                {
                    nBase = (short) nVal;
                    break;
                }
            }
            Error( SbERR_EXPECTED, "0/1" );
            break;
        case PRIVATE:
        {
            String aString = SbiTokenizer::Symbol(Next());
            if( !aString.EqualsIgnoreCaseAscii("Module") )
                Error( SbERR_EXPECTED, "Module" );
            break;
        }
        case COMPARE:
            switch( Next() )
            {
                case TEXT:      bText = TRUE; return;
                case BINARY:    bText = FALSE; return;
                default:;
            } // Fall thru!
        default:
            Error( SbERR_BAD_OPTION, eCurTok );
    }
}

// ERROR n

void SbiParser::ErrorStmnt()
{
    SbiExpression aPar( this );
    aPar.Gen();
    aGen.Gen( _ERROR );
}


// AB 22.5.1996
// JavaScript-Parsing zunaechst provisorisch hier implementiert
void SbiParser::OpenJavaBlock( SbiToken, SbiExprNode* )
{
}

void SbiParser::CloseJavaBlock()
{
}

void SbiParser::JavaStmntBlock( SbiToken )
{
}

void SbiParser::JavaBreak()
{
}

void SbiParser::JavaContinue()
{
}

void SbiParser::JavaFor()
{
}

void SbiParser::JavaFunction()
{
}

void SbiParser::JavaIf()
{
}

void SbiParser::JavaNew()
{
}

void SbiParser::JavaReturn()
{
}

void SbiParser::JavaThis()
{
}

void SbiParser::JavaVar()
{
}

void SbiParser::JavaWhile()
{
}

void SbiParser::JavaWith()
{
}



