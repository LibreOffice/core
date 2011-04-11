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
#include "precompiled_basic.hxx"

#include "sbcomp.hxx"

// Single-line IF und Multiline IF

void SbiParser::If()
{
    sal_uInt32 nEndLbl;
    SbiToken eTok = NIL;
    // Ende-Tokens ignorieren:
    SbiExpression aCond( this );
    aCond.Gen();
    TestToken( THEN );
    if( IsEoln( Next() ) )
    {
        // Am Ende jeden Blocks muss ein Jump zu ENDIF
        // eingefuegt werden, damit bei ELSEIF nicht erneut die Bedingung
        // ausgewertet wird. Die Tabelle nimmt alle Absprungstellen auf.
#define JMP_TABLE_SIZE 100
        sal_uInt32 pnJmpToEndLbl[JMP_TABLE_SIZE];   // 100 ELSEIFs zulaessig
        sal_uInt16 iJmp = 0;                        // aktueller Tabellen-Index

        // multiline IF
        nEndLbl = aGen.Gen( _JUMPF, 0 );
        eTok = Peek();
        while( !( eTok == ELSEIF || eTok == ELSE || eTok == ENDIF ) &&
                !bAbort && Parse() )
        {
            eTok = Peek();
            if( IsEof() )
            {
                Error( SbERR_BAD_BLOCK, IF ); bAbort = sal_True; return;
            }
        }
        while( eTok == ELSEIF )
        {
            // Bei erfolgreichem IF/ELSEIF auf ENDIF springen
            if( iJmp >= JMP_TABLE_SIZE )
            {
                Error( SbERR_PROG_TOO_LARGE );  bAbort = sal_True;  return;
            }
            pnJmpToEndLbl[iJmp++] = aGen.Gen( _JUMP, 0 );

            Next();
            aGen.BackChain( nEndLbl );

            aGen.Statement();
            SbiExpression* pCond = new SbiExpression( this );
            pCond->Gen();
            nEndLbl = aGen.Gen( _JUMPF, 0 );
            delete pCond;
            TestToken( THEN );
            eTok = Peek();
            while( !( eTok == ELSEIF || eTok == ELSE || eTok == ENDIF ) &&
                    !bAbort && Parse() )
            {
                eTok = Peek();
                if( IsEof() )
                {
                    Error( SbERR_BAD_BLOCK, ELSEIF );  bAbort = sal_True; return;
                }
            }
        }
        if( eTok == ELSE )
        {
            Next();
            sal_uInt32 nElseLbl = nEndLbl;
            nEndLbl = aGen.Gen( _JUMP, 0 );
            aGen.BackChain( nElseLbl );

            aGen.Statement();
            StmntBlock( ENDIF );
        }
        else if( eTok == ENDIF )
            Next();

        // Jmp-Tabelle abarbeiten
        while( iJmp > 0 )
        {
            iJmp--;
            aGen.BackChain( pnJmpToEndLbl[iJmp] );
        }
    }
    else
    {
        // single line IF
        bSingleLineIf = sal_True;
        nEndLbl = aGen.Gen( _JUMPF, 0 );
        Push( eCurTok );
        while( !bAbort )
        {
            if( !Parse() ) break;
            eTok = Peek();
            if( eTok == ELSE || eTok == EOLN || eTok == REM )
                break;
        }
        if( eTok == ELSE )
        {
            Next();
            sal_uInt32 nElseLbl = nEndLbl;
            nEndLbl = aGen.Gen( _JUMP, 0 );
            aGen.BackChain( nElseLbl );
            while( !bAbort )
            {
                if( !Parse() ) break;
                eTok = Peek();
                if( eTok == EOLN )
                    break;
            }
        }
        bSingleLineIf = sal_False;
    }
    aGen.BackChain( nEndLbl );
}

// ELSE/ELSEIF/ENDIF ohne IF

void SbiParser::NoIf()
{
    Error( SbERR_NO_IF );
    StmntBlock( ENDIF );
}

// DO WHILE...LOOP
// DO ... LOOP WHILE

void SbiParser::DoLoop()
{
    sal_uInt32 nStartLbl = aGen.GetPC();
    OpenBlock( DO );
    SbiToken eTok = Next();
    if( IsEoln( eTok ) )
    {
        // DO ... LOOP [WHILE|UNTIL expr]
        StmntBlock( LOOP );
        eTok = Next();
        if( eTok == UNTIL || eTok == WHILE )
        {
            SbiExpression aExpr( this );
            aExpr.Gen();
            aGen.Gen( eTok == UNTIL ? _JUMPF : _JUMPT, nStartLbl );
        } else
            if (eTok == EOLN || eTok == REM)
                aGen.Gen (_JUMP, nStartLbl);
            else
                Error( SbERR_EXPECTED, WHILE );
    }
    else
    {
        // DO [WHILE|UNTIL expr] ... LOOP
        if( eTok == UNTIL || eTok == WHILE )
        {
            SbiExpression aCond( this );
            aCond.Gen();
        }
        sal_uInt32 nEndLbl = aGen.Gen( eTok == UNTIL ? _JUMPT : _JUMPF, 0 );
        StmntBlock( LOOP );
        TestEoln();
        aGen.Gen( _JUMP, nStartLbl );
        aGen.BackChain( nEndLbl );
    }
    CloseBlock();
}

// WHILE ... WEND

void SbiParser::While()
{
    SbiExpression aCond( this );
    sal_uInt32 nStartLbl = aGen.GetPC();
    aCond.Gen();
    sal_uInt32 nEndLbl = aGen.Gen( _JUMPF, 0 );
    StmntBlock( WEND );
    aGen.Gen( _JUMP, nStartLbl );
    aGen.BackChain( nEndLbl );
}

// FOR var = expr TO expr STEP

void SbiParser::For()
{
    bool bForEach = ( Peek() == EACH );
    if( bForEach )
        Next();
    SbiExpression aLvalue( this, SbOPERAND );
    aLvalue.Gen();      // Variable auf dem Stack

    if( bForEach )
    {
        TestToken( _IN_ );
        SbiExpression aCollExpr( this, SbOPERAND );
        aCollExpr.Gen();    // Colletion var to for stack
        TestEoln();
        aGen.Gen( _INITFOREACH );
    }
    else
    {
        TestToken( EQ );
        SbiExpression aStartExpr( this );
        aStartExpr.Gen();   // Startausdruck auf dem Stack
        TestToken( TO );
        SbiExpression aStopExpr( this );
        aStopExpr.Gen();    // Endausdruck auf dem Stack
        if( Peek() == STEP )
        {
            Next();
            SbiExpression aStepExpr( this );
            aStepExpr.Gen();
        }
        else
        {
            SbiExpression aOne( this, 1, SbxINTEGER );
            aOne.Gen();
        }
        TestEoln();
        // Der Stack hat jetzt 4 Elemente: Variable, Start, Ende, Inkrement
        // Startwert binden
        aGen.Gen( _INITFOR );
    }

    sal_uInt32 nLoop = aGen.GetPC();
    // Test durchfuehren, evtl. Stack freigeben
    sal_uInt32 nEndTarget = aGen.Gen( _TESTFOR, 0 );
    OpenBlock( FOR );
    StmntBlock( NEXT );
    aGen.Gen( _NEXT );
    aGen.Gen( _JUMP, nLoop );
    // Kommen Variable nach NEXT?
    if( Peek() == SYMBOL )
    {
        SbiExpression aVar( this, SbOPERAND );
        if( aVar.GetRealVar() != aLvalue.GetRealVar() )
            Error( SbERR_EXPECTED, aLvalue.GetRealVar()->GetName() );
    }
    aGen.BackChain( nEndTarget );
    CloseBlock();
}

// WITH .. END WITH

void SbiParser::With()
{
    SbiExpression aVar( this, SbOPERAND );

    // Letzten Knoten in der Objekt-Kette ueberpruefen
    SbiExprNode *pNode = aVar.GetExprNode()->GetRealNode();
    SbiSymDef* pDef = pNode->GetVar();
    // Variant, AB 27.6.1997, #41090: bzw. empty -> muß Object sein
    if( pDef->GetType() == SbxVARIANT || pDef->GetType() == SbxEMPTY )
        pDef->SetType( SbxOBJECT );
    else if( pDef->GetType() != SbxOBJECT )
        Error( SbERR_NEEDS_OBJECT );

    // Knoten auch auf SbxOBJECT setzen, damit spaeter Gen() klappt
    pNode->SetType( SbxOBJECT );

    OpenBlock( NIL, aVar.GetExprNode() );
    StmntBlock( ENDWITH );
    CloseBlock();
}

// LOOP/NEXT/WEND ohne Konstrukt

void SbiParser::BadBlock()
{
    if( eEndTok )
        Error( SbERR_BAD_BLOCK, eEndTok );
    else
        Error( SbERR_BAD_BLOCK, "Loop/Next/Wend" );
}

// On expr Goto/Gosub n,n,n...

void SbiParser::OnGoto()
{
    SbiExpression aCond( this );
    aCond.Gen();
    sal_uInt32 nLabelsTarget = aGen.Gen( _ONJUMP, 0 );
    SbiToken eTok = Next();
    if( eTok != GOTO && eTok != GOSUB )
    {
        Error( SbERR_EXPECTED, "GoTo/GoSub" );
        eTok = GOTO;
    }
    // Label-Tabelle einlesen:
    sal_uInt32 nLbl = 0;
    do
    {
        Next(); // Label holen
        if( MayBeLabel() )
        {
            sal_uInt32 nOff = pProc->GetLabels().Reference( aSym );
            aGen.Gen( _JUMP, nOff );
            nLbl++;
        }
        else Error( SbERR_LABEL_EXPECTED );
    }
    while( !bAbort && TestComma() );
    if( eTok == GOSUB )
        nLbl |= 0x8000;
    aGen.Patch( nLabelsTarget, nLbl );
}

// GOTO/GOSUB

void SbiParser::Goto()
{
    SbiOpcode eOp = eCurTok == GOTO ? _JUMP : _GOSUB;
    Next();
    if( MayBeLabel() )
    {
        sal_uInt32 nOff = pProc->GetLabels().Reference( aSym );
        aGen.Gen( eOp, nOff );
    }
    else Error( SbERR_LABEL_EXPECTED );
}

// RETURN [label]

void SbiParser::Return()
{
    Next();
    if( MayBeLabel() )
    {
        sal_uInt32 nOff = pProc->GetLabels().Reference( aSym );
        aGen.Gen( _RETURN, nOff );
    }
    else aGen.Gen( _RETURN, 0 );
}

// SELECT CASE

void SbiParser::Select()
{
    TestToken( CASE );
    SbiExpression aCase( this );
    SbiToken eTok = NIL;
    aCase.Gen();
    aGen.Gen( _CASE );
    TestEoln();
    sal_uInt32 nNextTarget = 0;
    sal_uInt32 nDoneTarget = 0;
    sal_Bool bElse = sal_False;
    // Die Cases einlesen:
    while( !bAbort )
    {
        eTok = Next();
        if( eTok == CASE )
        {
            if( nNextTarget )
                aGen.BackChain( nNextTarget ), nNextTarget = 0;
            aGen.Statement();
            // Jeden Case einlesen
            sal_Bool bDone = sal_False;
            sal_uInt32 nTrueTarget = 0;
            if( Peek() == ELSE )
            {
                // CASE ELSE
                Next();
                bElse = sal_True;
            }
            else while( !bDone )
            {
                if( bElse )
                    Error( SbERR_SYNTAX );
                SbiToken eTok2 = Peek();
                if( eTok2 == IS || ( eTok2 >= EQ && eTok2 <= GE ) )
                {   // CASE [IS] operator expr
                    if( eTok2 == IS )
                        Next();
                    eTok2 = Peek();
                    if( eTok2 < EQ || eTok2 > GE )
                        Error( SbERR_SYNTAX );
                    else Next();
                    SbiExpression aCompare( this );
                    aCompare.Gen();
                    nTrueTarget = aGen.Gen(
                        _CASEIS, nTrueTarget,
                        sal::static_int_cast< sal_uInt16 >(
                            SbxEQ + ( eTok2 - EQ ) ) );
                }
                else
                {   // CASE expr | expr TO expr
                    SbiExpression aCase1( this );
                    aCase1.Gen();
                    if( Peek() == TO )
                    {
                        // CASE a TO b
                        Next();
                        SbiExpression aCase2( this );
                        aCase2.Gen();
                        nTrueTarget = aGen.Gen( _CASETO, nTrueTarget );
                    }
                    else
                        // CASE a
                        nTrueTarget = aGen.Gen( _CASEIS, nTrueTarget, SbxEQ );

                }
                if( Peek() == COMMA ) Next();
                else TestEoln(), bDone = sal_True;
            }
            // Alle Cases abgearbeitet
            if( !bElse )
            {
                nNextTarget = aGen.Gen( _JUMP, nNextTarget );
                aGen.BackChain( nTrueTarget );
            }
            // den Statement-Rumpf bauen
            while( !bAbort )
            {
                eTok = Peek();
                if( eTok == CASE || eTok == ENDSELECT )
                    break;
                if( !Parse() ) goto done;
                eTok = Peek();
                if( eTok == CASE || eTok == ENDSELECT )
                    break;
            }
            if( !bElse )
                nDoneTarget = aGen.Gen( _JUMP, nDoneTarget );
        }
        else if( !IsEoln( eTok ) )
            break;
    }
done:
    if( eTok != ENDSELECT )
        Error( SbERR_EXPECTED, ENDSELECT );
    if( nNextTarget )
        aGen.BackChain( nNextTarget );
    aGen.BackChain( nDoneTarget );
    aGen.Gen( _ENDCASE );
}

// ON Error/Variable

#ifdef _MSC_VER
#pragma optimize("",off)
#endif

void SbiParser::On()
{
    SbiToken eTok = Peek();
    String aString = SbiTokenizer::Symbol(eTok);
    if (aString.EqualsIgnoreCaseAscii("ERROR"))
        eTok = _ERROR_; // Error kommt als SYMBOL
    if( eTok != _ERROR_ && eTok != LOCAL ) OnGoto();
    else
    {
        if( eTok == LOCAL ) Next();
        Next (); // Kein TestToken mehr, da es sonst einen Fehler gibt

        Next(); // Token nach Error holen
        if( eCurTok == GOTO )
        {
            // ON ERROR GOTO label|0
            Next();
            bool bError_ = false;
            if( MayBeLabel() )
            {
                if( eCurTok == NUMBER && !nVal )
                    aGen.Gen( _STDERROR );
                else
                {
                    sal_uInt32 nOff = pProc->GetLabels().Reference( aSym );
                    aGen.Gen( _ERRHDL, nOff );
                }
            }
            else if( eCurTok == MINUS )
            {
                Next();
                if( eCurTok == NUMBER && nVal == 1 )
                    aGen.Gen( _STDERROR );
                else
                    bError_ = true;
            }
            if( bError_ )
                Error( SbERR_LABEL_EXPECTED );
        }
        else if( eCurTok == RESUME )
        {
            TestToken( NEXT );
            aGen.Gen( _NOERROR );
        }
        else Error( SbERR_EXPECTED, "GoTo/Resume" );
    }
}

#ifdef _MSC_VER
#pragma optimize("",off)
#endif

// RESUME [0]|NEXT|label

void SbiParser::Resume()
{
    sal_uInt32 nLbl;

    switch( Next() )
    {
        case EOS:
        case EOLN:
            aGen.Gen( _RESUME, 0 );
            break;
        case NEXT:
            aGen.Gen( _RESUME, 1 );
            Next();
            break;
        case NUMBER:
            if( !nVal )
            {
                aGen.Gen( _RESUME, 0 );
                break;
            } // fall thru
        case SYMBOL:
            if( MayBeLabel() )
            {
                nLbl = pProc->GetLabels().Reference( aSym );
                aGen.Gen( _RESUME, nLbl );
                Next();
                break;
            } // fall thru
        default:
            Error( SbERR_LABEL_EXPECTED );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
