/*************************************************************************
 *
 *  $RCSfile: loops.cxx,v $
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

#include "sbcomp.hxx"
#pragma hdrstop

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBCOMP, SBCOMP_CODE )

// Single-line IF und Multiline IF

void SbiParser::If()
{
    USHORT nEndLbl;
    SbiToken eTok;
    // Ende-Tokens ignorieren:
    SbiExpression aCond( this );
    aCond.Gen();
    TestToken( THEN );
    if( IsEoln( Next() ) )
    {
        // AB 13.5.1996: #27720# Am Ende jeden Blocks muss ein Jump zu ENDIF
        // eingefuegt werden, damit bei ELSEIF nicht erneut die Bedingung
        // ausgewertet wird. Die Tabelle nimmt alle Absprungstellen auf.
#define JMP_TABLE_SIZE 100
        USHORT pnJmpToEndLbl[JMP_TABLE_SIZE];   // 100 ELSEIFs zulaessig
        USHORT iJmp = 0;                        // aktueller Tabellen-Index

        // multiline IF
        nEndLbl = aGen.Gen( _JUMPF, 0 );
        while( !bAbort && Parse() )
        {
            eTok = Peek();
            if( eTok == ELSEIF || eTok == ELSE || eTok == ENDIF )
                break;
            if( IsEof() )
            {
                Error( SbERR_BAD_BLOCK, IF ); bAbort = TRUE; return;
            }
        }
        // ELSEIF?
        while( eTok == ELSEIF )
        {
            // #27720# Bei erfolgreichem IF/ELSEIF auf ENDIF springen
            if( iJmp >= JMP_TABLE_SIZE )
            {
                Error( SbERR_PROG_TOO_LARGE );  bAbort = TRUE;  return;
            }
            pnJmpToEndLbl[iJmp++] = aGen.Gen( _JUMP, 0 );

            Next();
            aGen.BackChain( nEndLbl );
            SbiExpression* pCond = new SbiExpression( this );
            pCond->Gen();
            nEndLbl = aGen.Gen( _JUMPF, 0 );
            delete pCond;
            TestToken( THEN );
            while( !bAbort && Parse() )
            {
                eTok = Peek();
                if( eTok == ELSEIF || eTok == ELSE || eTok == ENDIF )
                    break;
                if( IsEof() )
                {
                    Error( SbERR_BAD_BLOCK, ELSEIF );  bAbort = TRUE; return;
                }
            }
        }
        if( eTok == ELSE )
        {
            Next();
            USHORT nElseLbl = nEndLbl;
            nEndLbl = aGen.Gen( _JUMP, 0 );
            aGen.BackChain( nElseLbl );
            StmntBlock( ENDIF );
        }
        else if( eTok == ENDIF )
            Next();

        // #27720# Jmp-Tabelle abarbeiten
        while( iJmp > 0 )
        {
            iJmp--;
            aGen.BackChain( pnJmpToEndLbl[iJmp] );
        }
    }
    else
    {
        // single line IF
        bSingleLineIf = TRUE;
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
            USHORT nElseLbl = nEndLbl;
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
        bSingleLineIf = FALSE;
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
    USHORT nStartLbl = aGen.GetPC();
    USHORT nEndChain = 0;
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
        USHORT nEndLbl = aGen.Gen( eTok == UNTIL ? _JUMPT : _JUMPF, 0 );
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
    USHORT nStartLbl = aGen.GetPC();
    aCond.Gen();
    USHORT nEndLbl = aGen.Gen( _JUMPF, 0 );
    StmntBlock( WEND );
    aGen.Gen( _JUMP, nStartLbl );
    aGen.BackChain( nEndLbl );
}

// FOR var = expr TO expr STEP

void SbiParser::For()
{
    SbiExpression aLvalue( this, SbOPERAND );
    aLvalue.Gen();      // Variable auf dem Stack
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
    USHORT nLoop = aGen.GetPC();
    // Test durchfuehren, evtl. Stack freigeben
    USHORT nEndTarget = aGen.Gen( _TESTFOR, 0 );
    OpenBlock( FOR );
    StmntBlock( NEXT );
    aGen.Gen( _NEXT );
    aGen.Gen( _JUMP, nLoop );
    // Kommen Variable nach NEXT?
    if( Peek() == SYMBOL )
    {
        SbiExpression aVar( this, SbOPERAND );
        if( aVar.GetRealVar() != aLvalue.GetRealVar() )
            Error( SbERR_EXPECTED, aVar.GetRealVar()->GetName() );
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
    USHORT nLabelsTarget = aGen.Gen( _ONJUMP, 0 );
    SbiToken eTok = Next();
    if( eTok != GOTO && eTok != GOSUB )
    {
        Error( SbERR_EXPECTED, "GoTo/GoSub" );
        eTok = GOTO;
    }
    // Label-Tabelle einlesen:
    short nLbl = 0;
    do
    {
        SbiToken eTok2 = Next();    // Label holen
        if( MayBeLabel() )
        {
            USHORT nOff = pProc->GetLabels().Reference( aSym );
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
        USHORT nOff = pProc->GetLabels().Reference( aSym );
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
        USHORT nOff = pProc->GetLabels().Reference( aSym );
        aGen.Gen( _RETURN, nOff );
    }
    else aGen.Gen( _RETURN, 0 );
}

// SELECT CASE

void SbiParser::Select()
{
    TestToken( CASE );
    SbiExpression aCase( this );
    SbiToken eTok;
    aCase.Gen();
    aGen.Gen( _CASE );
    TestEoln();
    USHORT nNextTarget = 0;
    USHORT nDoneTarget = 0;
    BOOL bElse = FALSE;
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
            BOOL bDone = FALSE;
            USHORT nTrueTarget = 0;
            if( Peek() == ELSE )
            {
                // CASE ELSE
                Next();
                bElse = TRUE;
            }
            else while( !bDone )
            {
                if( bElse )
                    Error( SbERR_SYNTAX );
                SbiToken eTok = Peek();
                if( eTok == IS || ( eTok >= EQ && eTok <= GE ) )
                {   // CASE [IS] operator expr
                    if( eTok == IS )
                        Next();
                    eTok = Peek();
                    if( eTok < EQ || eTok > GE )
                        Error( SbERR_SYNTAX );
                    else Next();
                    SbiExpression aCompare( this );
                    aCompare.Gen();
                    nTrueTarget = aGen.Gen( _CASEIS, nTrueTarget,
                                  SbxEQ + ( eTok - EQ ) );
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
                else TestEoln(), bDone = TRUE;
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
    //if (!aString.ICompare("ERROR"))
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
            if( MayBeLabel() )
            {
                if( eCurTok == NUMBER && !nVal )
                    aGen.Gen( _STDERROR );
                else
                {
                    USHORT nOff = pProc->GetLabels().Reference( aSym );
                    aGen.Gen( _ERRHDL, nOff );
                }
            }
            else Error( SbERR_LABEL_EXPECTED );
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
    USHORT nLbl;

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

