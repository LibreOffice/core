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


#include <parser.hxx>
#include <memory>

#include <basic/sberrors.hxx>

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
        nEndLbl = aGen.Gen( SbiOpcode::JUMPF_, 0 );
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
            pnJmpToEndLbl[iJmp++] = aGen.Gen( SbiOpcode::JUMP_, 0 );

            Next();
            aGen.BackChain( nEndLbl );

            aGen.Statement();
            auto pCond = std::make_unique<SbiExpression>( this );
            pCond->Gen();
            nEndLbl = aGen.Gen( SbiOpcode::JUMPF_, 0 );
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
            nEndLbl = aGen.Gen( SbiOpcode::JUMP_, 0 );
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
        nEndLbl = aGen.Gen( SbiOpcode::JUMPF_, 0 );
        Push( eCurTok );
        // tdf#128263: update push positions to correctly restore in Next()
        nPLine = nLine;
        nPCol1 = nCol1;
        nPCol2 = nCol2;

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
            nEndLbl = aGen.Gen( SbiOpcode::JUMP_, 0 );
            aGen.BackChain( nElseLbl );
            while( !bAbort )
            {
                if( !Parse() ) break;
                eTok = Peek();
                if( eTok == EOLN || eTok == REM )
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
            aGen.Gen( eTok == UNTIL ? SbiOpcode::JUMPF_ : SbiOpcode::JUMPT_, nStartLbl );
        } else
            if (eTok == EOLN || eTok == REM)
                aGen.Gen (SbiOpcode::JUMP_, nStartLbl);
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
        sal_uInt32 nEndLbl = aGen.Gen( eTok == UNTIL ? SbiOpcode::JUMPT_ : SbiOpcode::JUMPF_, 0 );
        StmntBlock( LOOP );
        TestEoln();
        aGen.Gen( SbiOpcode::JUMP_, nStartLbl );
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
    sal_uInt32 nEndLbl = aGen.Gen( SbiOpcode::JUMPF_, 0 );
    StmntBlock( WEND );
    aGen.Gen( SbiOpcode::JUMP_, nStartLbl );
    aGen.BackChain( nEndLbl );
}

// FOR var = expr TO expr STEP

void SbiParser::For()
{
    bool bForEach = ( Peek() == EACH );
    if( bForEach )
        Next();
    SbiExpression aLvalue( this, SbOPERAND );
    if (!aLvalue.IsVariable())
    {
        bAbort = true;
        return; // the error is already set in SbiExpression ctor
    }
    aLvalue.Gen();      // variable on the Stack

    if( bForEach )
    {
        TestToken( IN_ );
        SbiExpression aCollExpr( this, SbOPERAND );
        aCollExpr.Gen();    // Collection var to for stack
        TestEoln();
        aGen.Gen( SbiOpcode::INITFOREACH_ );
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
        aGen.Gen( SbiOpcode::INITFOR_ );
    }

    sal_uInt32 nLoop = aGen.GetPC();
    // do tests, maybe free the stack
    sal_uInt32 nEndTarget = aGen.Gen( SbiOpcode::TESTFOR_, 0 );
    OpenBlock( FOR );
    StmntBlock( NEXT );
    aGen.Gen( SbiOpcode::NEXT_ );
    aGen.Gen( SbiOpcode::JUMP_, nLoop );
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
        Error( ERRCODE_BASIC_BAD_BLOCK, u"Loop/Next/Wend"_ustr );
}

// On expr Goto/Gosub n,n,n...

void SbiParser::OnGoto()
{
    SbiExpression aCond( this );
    aCond.Gen();
    sal_uInt32 nLabelsTarget = aGen.Gen( SbiOpcode::ONJUMP_, 0 );
    SbiToken eTok = Next();
    if( eTok != GOTO && eTok != GOSUB )
    {
        Error( ERRCODE_BASIC_EXPECTED, u"GoTo/GoSub"_ustr );
        eTok = GOTO;
    }

    sal_uInt32 nLbl = 0;
    do
    {
        Next(); // get label
        if( MayBeLabel() )
        {
            sal_uInt32 nOff = pProc->GetLabels().Reference( aSym );
            aGen.Gen( SbiOpcode::JUMP_, nOff );
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
    SbiOpcode eOp = eCurTok == GOTO ? SbiOpcode::JUMP_ : SbiOpcode::GOSUB_;
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
        aGen.Gen( SbiOpcode::RETURN_, nOff );
    }
    else aGen.Gen( SbiOpcode::RETURN_, 0 );
}

// SELECT CASE

void SbiParser::Select()
{
    TestToken( CASE );
    SbiExpression aCase( this );
    SbiToken eTok = NIL;
    aCase.Gen();
    aGen.Gen( SbiOpcode::CASE_ );
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
            {
                aGen.BackChain( nNextTarget );
                nNextTarget = 0;
            }
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
                        SbiOpcode::CASEIS_, nTrueTarget,
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
                        nTrueTarget = aGen.Gen( SbiOpcode::CASETO_, nTrueTarget );
                    }
                    else
                        // CASE a
                        nTrueTarget = aGen.Gen( SbiOpcode::CASEIS_, nTrueTarget, SbxEQ );

                }
                if( Peek() == COMMA ) Next();
                else
                {
                    TestEoln();
                    bDone = true;
                }
            }

            if( !bElse )
            {
                nNextTarget = aGen.Gen( SbiOpcode::JUMP_, nNextTarget );
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
                nDoneTarget = aGen.Gen( SbiOpcode::JUMP_, nDoneTarget );
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
    aGen.Gen( SbiOpcode::ENDCASE_ );
}

// ON Error/Variable

void SbiParser::On()
{
    SbiToken eTok = Peek();
    OUString aString = SbiTokenizer::Symbol(eTok);
    if (aString.equalsIgnoreAsciiCase("ERROR"))
    {
        eTok = ERROR_; // Error comes as SYMBOL
    }
    if( eTok != ERROR_ && eTok != LOCAL )
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
                    aGen.Gen( SbiOpcode::STDERROR_ );
                }
                else
                {
                    sal_uInt32 nOff = pProc->GetLabels().Reference( aSym );
                    aGen.Gen( SbiOpcode::ERRHDL_, nOff );
                }
            }
            else if( eCurTok == MINUS )
            {
                Next();
                if( eCurTok == NUMBER && nVal == 1 )
                {
                    aGen.Gen( SbiOpcode::STDERROR_ );
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
            aGen.Gen( SbiOpcode::NOERROR_ );
        }
        else Error( ERRCODE_BASIC_EXPECTED, u"GoTo/Resume"_ustr );
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
            aGen.Gen( SbiOpcode::RESUME_, 0 );
            break;
        case NEXT:
            aGen.Gen( SbiOpcode::RESUME_, 1 );
            Next();
            break;
        case NUMBER:
            if( !nVal )
            {
                aGen.Gen( SbiOpcode::RESUME_, 0 );
                break;
            }
            [[fallthrough]];
        case SYMBOL:
            if( MayBeLabel() )
            {
                nLbl = pProc->GetLabels().Reference( aSym );
                aGen.Gen( SbiOpcode::RESUME_, nLbl );
                Next();
                break;
            }
            [[fallthrough]];
        default:
            Error( ERRCODE_BASIC_LABEL_EXPECTED );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
