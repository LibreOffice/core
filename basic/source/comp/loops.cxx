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


#include "parser.hxx"
#include <memory>

// Single-line IF and Multiline IF

void SbiParser::If()
{
    sal_uInt32 nEndLbl;
    SbiToken eTok = NIL;
    // ignore end-tokens
    SbiExpression aCond( this );
    aCond.Gen();
    TestToken( THEN );
    if( IsEoln( Next() ) )
    {
        // At the end of each block a jump to ENDIF must be inserted,
        // so that the condition is not evaluated again at ELSEIF.
        // The table collects all jump points.
#define JMP_TABLE_SIZE 100
        sal_uInt32 pnJmpToEndLbl[JMP_TABLE_SIZE];   // 100 ELSEIFs allowed
        sal_uInt16 iJmp = 0;                        // current table index

        // multiline IF
        nEndLbl = aGen.Gen( _JUMPF, 0 );
        eTok = Peek();
        while( !( eTok == ELSEIF || eTok == ELSE || eTok == ENDIF ) &&
                !bAbort && Parse() )
        {
            eTok = Peek();
            if( IsEof() )
            {
                Error( ERRCODE_BASIC_BAD_BLOCK, IF ); bAbort = true; return;
            }
        }
        while( eTok == ELSEIF )
        {
            // jump to ENDIF in case of a successful IF/ELSEIF
            if( iJmp >= JMP_TABLE_SIZE )
            {
                Error( ERRCODE_BASIC_PROG_TOO_LARGE );  bAbort = true;  return;
            }
            pnJmpToEndLbl[iJmp++] = aGen.Gen( _JUMP, 0 );

            Next();
            aGen.BackChain( nEndLbl );

            aGen.Statement();
            std::unique_ptr<SbiExpression> pCond(new SbiExpression( this ));
            pCond->Gen();
            nEndLbl = aGen.Gen( _JUMPF, 0 );
            pCond.reset();
            TestToken( THEN );
            eTok = Peek();
            while( !( eTok == ELSEIF || eTok == ELSE || eTok == ENDIF ) &&
                    !bAbort && Parse() )
            {
                eTok = Peek();
                if( IsEof() )
                {
                    Error( ERRCODE_BASIC_BAD_BLOCK, ELSEIF );  bAbort = true; return;
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


        while( iJmp > 0 )
        {
            iJmp--;
            aGen.BackChain( pnJmpToEndLbl[iJmp] );
        }
    }
    else
    {
        // single line IF
        bSingleLineIf = true;
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
        bSingleLineIf = false;
    }
    aGen.BackChain( nEndLbl );
}

// ELSE/ELSEIF/ENDIF without IF

void SbiParser::NoIf()
{
    Error( ERRCODE_BASIC_NO_IF );
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
                Error( ERRCODE_BASIC_EXPECTED, WHILE );
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
    aLvalue.Gen();      // variable on the Stack

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
        aStartExpr.Gen();
        TestToken( TO );
        SbiExpression aStopExpr( this );
        aStopExpr.Gen();
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
        // The stack has all 4 elements now: variable, start, end, increment
        // bind start value
        aGen.Gen( _INITFOR );
    }

    sal_uInt32 nLoop = aGen.GetPC();
    // do tests, maybe free the stack
    sal_uInt32 nEndTarget = aGen.Gen( _TESTFOR, 0 );
    OpenBlock( FOR );
    StmntBlock( NEXT );
    aGen.Gen( _NEXT );
    aGen.Gen( _JUMP, nLoop );
    // are there variables after NEXT?
    if( Peek() == SYMBOL )
    {
        SbiExpression aVar( this, SbOPERAND );
        if( aVar.GetRealVar() != aLvalue.GetRealVar() )
            Error( ERRCODE_BASIC_EXPECTED, aLvalue.GetRealVar()->GetName() );
    }
    aGen.BackChain( nEndTarget );
    CloseBlock();
}

// WITH .. END WITH

void SbiParser::With()
{
    SbiExpression aVar( this, SbOPERAND );

    SbiExprNode *pNode = aVar.GetExprNode()->GetRealNode();
    if (!pNode)
        return;
    SbiSymDef* pDef = pNode->GetVar();
    // Variant, from 27.6.1997, #41090: empty -> must be Object
    if( pDef->GetType() == SbxVARIANT || pDef->GetType() == SbxEMPTY )
        pDef->SetType( SbxOBJECT );
    else if( pDef->GetType() != SbxOBJECT )
        Error( ERRCODE_BASIC_NEEDS_OBJECT );


    pNode->SetType( SbxOBJECT );

    OpenBlock( NIL, aVar.GetExprNode() );
    StmntBlock( ENDWITH );
    CloseBlock();
}

// LOOP/NEXT/WEND without construct

void SbiParser::BadBlock()
{
    if( eEndTok )
        Error( ERRCODE_BASIC_BAD_BLOCK, eEndTok );
    else
        Error( ERRCODE_BASIC_BAD_BLOCK, "Loop/Next/Wend" );
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
        Error( ERRCODE_BASIC_EXPECTED, "GoTo/GoSub" );
        eTok = GOTO;
    }

    sal_uInt32 nLbl = 0;
    do
    {
        Next(); // get label
        if( MayBeLabel() )
        {
            sal_uInt32 nOff = pProc->GetLabels().Reference( aSym );
            aGen.Gen( _JUMP, nOff );
            nLbl++;
        }
        else Error( ERRCODE_BASIC_LABEL_EXPECTED );
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
    else Error( ERRCODE_BASIC_LABEL_EXPECTED );
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
    bool bElse = false;

    while( !bAbort )
    {
        eTok = Next();
        if( eTok == CASE )
        {
            if( nNextTarget )
                aGen.BackChain( nNextTarget ), nNextTarget = 0;
            aGen.Statement();

            bool bDone = false;
            sal_uInt32 nTrueTarget = 0;
            if( Peek() == ELSE )
            {
                // CASE ELSE
                Next();
                bElse = true;
            }
            else while( !bDone )
            {
                if( bElse )
                    Error( ERRCODE_BASIC_SYNTAX );
                SbiToken eTok2 = Peek();
                if( eTok2 == IS || ( eTok2 >= EQ && eTok2 <= GE ) )
                {   // CASE [IS] operator expr
                    if( eTok2 == IS )
                        Next();
                    eTok2 = Peek();
                    if( eTok2 < EQ || eTok2 > GE )
                        Error( ERRCODE_BASIC_SYNTAX );
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
                else TestEoln(), bDone = true;
            }

            if( !bElse )
            {
                nNextTarget = aGen.Gen( _JUMP, nNextTarget );
                aGen.BackChain( nTrueTarget );
            }
            // build the statement body
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
        Error( ERRCODE_BASIC_EXPECTED, ENDSELECT );
    if( nNextTarget )
        aGen.BackChain( nNextTarget );
    aGen.BackChain( nDoneTarget );
    aGen.Gen( _ENDCASE );
}

// ON Error/Variable

void SbiParser::On()
{
    SbiToken eTok = Peek();
    OUString aString = SbiTokenizer::Symbol(eTok);
    if (aString.equalsIgnoreAsciiCase("ERROR"))
    {
        eTok = _ERROR_; // Error comes as SYMBOL
    }
    if( eTok != _ERROR_ && eTok != LOCAL )
    {
        OnGoto();
    }
    else
    {
        if( eTok == LOCAL )
        {
            Next();
        }
        Next (); // no more TestToken, as there'd be an error otherwise

        Next(); // get token after error
        if( eCurTok == GOTO )
        {
            // ON ERROR GOTO label|0
            Next();
            bool bError_ = false;
            if( MayBeLabel() )
            {
                if( eCurTok == NUMBER && !nVal )
                {
                    aGen.Gen( _STDERROR );
                }
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
                {
                    aGen.Gen( _STDERROR );
                }
                else
                {
                    bError_ = true;
                }
            }
            if( bError_ )
            {
                Error( ERRCODE_BASIC_LABEL_EXPECTED );
            }
        }
        else if( eCurTok == RESUME )
        {
            TestToken( NEXT );
            aGen.Gen( _NOERROR );
        }
        else Error( ERRCODE_BASIC_EXPECTED, "GoTo/Resume" );
    }
}

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
            }
            SAL_FALLTHROUGH;
        case SYMBOL:
            if( MayBeLabel() )
            {
                nLbl = pProc->GetLabels().Reference( aSym );
                aGen.Gen( _RESUME, nLbl );
                Next();
                break;
            }
            SAL_FALLTHROUGH;
        default:
            Error( ERRCODE_BASIC_LABEL_EXPECTED );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
