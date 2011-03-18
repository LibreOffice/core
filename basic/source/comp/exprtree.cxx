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
#include <basic/sbx.hxx>        // w.g. ...IMPL_REF(...sbxvariable)
#include "expr.hxx"

/***************************************************************************
|*
|*      SbiExpression
|*
***************************************************************************/

SbiExpression::SbiExpression( SbiParser* p, SbiExprType t,
    SbiExprMode eMode, const KeywordSymbolInfo* pKeywordSymbolInfo )
{
    pParser = p;
    bError = bByVal = bBased = bBracket = sal_False;
    nParenLevel = 0;
    eCurExpr = t;
    m_eMode = eMode;
    pNext = NULL;
    pExpr = (t != SbSTDEXPR ) ? Term( pKeywordSymbolInfo ) : Boolean();
    if( t != SbSYMBOL )
        pExpr->Optimize();
    if( t == SbLVALUE && !pExpr->IsLvalue() )
        p->Error( SbERR_LVALUE_EXPECTED );
    if( t == SbOPERAND && !IsVariable() )
        p->Error( SbERR_VAR_EXPECTED );
}

SbiExpression::SbiExpression( SbiParser* p, double n, SbxDataType t )
{
    pParser = p;
    eCurExpr = SbOPERAND;
    pNext = NULL;
    bError = bByVal = bBased = bBracket = sal_False;
    pExpr = new SbiExprNode( pParser, n, t );
    pExpr->Optimize();
}

SbiExpression::SbiExpression( SbiParser* p, const String& r )
{
    pParser = p;
    pNext = NULL;
    bError = bByVal = bBased = bBracket = sal_False;
    eCurExpr = SbOPERAND;
    pExpr = new SbiExprNode( pParser, r );
}

SbiExpression::SbiExpression( SbiParser* p, const SbiSymDef& r, SbiExprList* pPar )
{
    pParser = p;
    pNext = NULL;
    bError = bByVal = bBased = bBracket = sal_False;
    eCurExpr = SbOPERAND;
    pExpr = new SbiExprNode( pParser, r, SbxVARIANT, pPar );
}

SbiExpression::SbiExpression( SbiParser* p, SbiToken t )
{
    pParser = p;
    pNext = NULL;
    bError = bByVal = bBased = bBracket = sal_False;
    eCurExpr = SbOPERAND;
    pExpr = new SbiExprNode( pParser, NULL, t, NULL );
}

SbiExpression::~SbiExpression()
{
    delete pExpr;
}

// Einlesen eines kompletten Bezeichners
// Ein Bezeichner hat folgende Form:
// name[(Parameter)][.Name[(parameter)]]...
// Strukturelemente werden ueber das Element pNext verkoppelt,
// damit sie nicht im Baum stehen.

// Folgen Parameter ohne Klammer? Dies kann eine Zahl, ein String,
// ein Symbol oder auch ein Komma sein (wenn der 1. Parameter fehlt)

static sal_Bool DoParametersFollow( SbiParser* p, SbiExprType eCurExpr, SbiToken eTok )
{
    if( eTok == LPAREN )
        return sal_True;
    // Aber nur, wenn CALL-aehnlich!
    if( !p->WhiteSpace() || eCurExpr != SbSYMBOL )
        return sal_False;
    if (   eTok == NUMBER || eTok == MINUS || eTok == FIXSTRING
        || eTok == SYMBOL || eTok == COMMA  || eTok == DOT || eTok == NOT || eTok == BYVAL )
    {
        return sal_True;
    }
    else // check for default params with reserved names ( e.g. names of tokens )
    {
        SbiTokenizer tokens( *(SbiTokenizer*)p );
        // Urk the Next() / Peek() symantics are... weird
        tokens.Next();
        if ( tokens.Peek() == ASSIGN )
            return sal_True;
    }
    return sal_False;
}

// Definition eines neuen Symbols

static SbiSymDef* AddSym
    ( SbiToken eTok, SbiSymPool& rPool, SbiExprType eCurExpr,
      const String& rName, SbxDataType eType, SbiParameters* pPar )
{
    SbiSymDef* pDef;
    // A= ist keine Prozedur
    sal_Bool bHasType = sal_Bool( eTok == EQ || eTok == DOT );
    if( ( !bHasType && eCurExpr == SbSYMBOL ) || pPar )
    {
        // Dies ist also eine Prozedur
        // da suche man doch den richtigen Pool raus, da Procs
        // immer in einem Public-Pool landen muessen
        SbiSymPool* pPool = &rPool;
        if( pPool->GetScope() != SbPUBLIC )
            pPool = &rPool.GetParser()->aPublics;
        SbiProcDef* pProc = pPool->AddProc( rName );

        // Sonderbehandlung fuer Colls wie Documents(1)
        if( eCurExpr == SbSTDEXPR )
            bHasType = sal_True;

        pDef = pProc;
        pDef->SetType( bHasType ? eType : SbxEMPTY );
        if( pPar )
        {
            // Dummy-Parameter generieren
            sal_uInt16 n = 1;
            for( short i = 0; i < pPar->GetSize(); i++ )
            {
                String aPar = String::CreateFromAscii( "PAR" );
                aPar += ++n;
                pProc->GetParams().AddSym( aPar );
            }
        }
    }
    else
    {
        // oder ein normales Symbol
        pDef = rPool.AddSym( rName );
        pDef->SetType( eType );
    }
    return pDef;
}

// Zur Zeit sind sogar Keywords zugelassen (wg. gleichnamiger Dflt-Properties)

SbiExprNode* SbiExpression::Term( const KeywordSymbolInfo* pKeywordSymbolInfo )
{
    if( pParser->Peek() == DOT )
    {
        // eine WITH-Variable
        SbiExprNode* pWithVar = pParser->GetWithVar();
        // #26608: Ans Ende der Node-Kette gehen, um richtiges Objekt zu uebergeben
        SbiSymDef* pDef = pWithVar ? pWithVar->GetRealVar() : NULL;
        SbiExprNode* pNd = NULL;
        if( !pDef )
        {
            pParser->Next();
        }
        else
        {
            pNd = ObjTerm( *pDef );
            if( pNd )
                pNd->SetWithParent( pWithVar );
        }
        if( !pNd )
        {
            pParser->Error( SbERR_UNEXPECTED, DOT );
            pNd = new SbiExprNode( pParser, 1.0, SbxDOUBLE );
        }
        return pNd;
    }

    SbiToken eTok = (pKeywordSymbolInfo == NULL) ? pParser->Next() : pKeywordSymbolInfo->m_eTok;
    // Anfang des Parsings merken
    pParser->LockColumn();
    String aSym( (pKeywordSymbolInfo == NULL) ? pParser->GetSym() : pKeywordSymbolInfo->m_aKeywordSymbol );
    SbxDataType eType = (pKeywordSymbolInfo == NULL) ? pParser->GetType() : pKeywordSymbolInfo->m_eSbxDataType;
    SbiParameters* pPar = NULL;
    SbiExprListVector* pvMoreParLcl = NULL;
    // Folgen Parameter?
    SbiToken eNextTok = pParser->Peek();
    // Ist es ein benannter Parameter?
    // Dann einfach eine Stringkonstante erzeugen. Diese wird
    // im SbiParameters-ctor erkannt und weiterverarbeitet
    if( eNextTok == ASSIGN )
    {
        pParser->UnlockColumn();
        return new SbiExprNode( pParser, aSym );
    }
    // ab hier sind keine Keywords zugelassen!
    if( pParser->IsKwd( eTok ) )
    {
        if( pParser->IsCompatible() && eTok == INPUT )
        {
            eTok = SYMBOL;
        }
        else
        {
            pParser->Error( SbERR_SYNTAX );
            bError = sal_True;
        }
    }

    if( DoParametersFollow( pParser, eCurExpr, eTok = eNextTok ) )
    {
        bool bStandaloneExpression = (m_eMode == EXPRMODE_STANDALONE);
        pPar = new SbiParameters( pParser, bStandaloneExpression );
        bError |= !pPar->IsValid();
        if( !bError )
            bBracket = pPar->IsBracket();
        eTok = pParser->Peek();

        // i75443 check for additional sets of parameters
        while( eTok == LPAREN )
        {
            if( pvMoreParLcl == NULL )
                pvMoreParLcl = new SbiExprListVector();
            SbiParameters* pAddPar = new SbiParameters( pParser );
            pvMoreParLcl->push_back( pAddPar );
            bError |= !pPar->IsValid();
            eTok = pParser->Peek();
        }
    }
    // Es koennte ein Objektteil sein, wenn . oder ! folgt
    // Bei . muss aber die Variable bereits definiert sein; wenn pDef
    // nach der Suche NULL ist, isses ein Objekt!
    sal_Bool bObj = sal_Bool( ( eTok == DOT || eTok == EXCLAM )
                    && !pParser->WhiteSpace() );
    if( bObj )
    {
        bBracket = sal_False;   // Now the bracket for the first term is obsolete
        if( eType == SbxVARIANT )
            eType = SbxOBJECT;
        else
        {
            // Name%. geht wirklich nicht!
            pParser->Error( SbERR_BAD_DECLARATION, aSym );
            bError = sal_True;
        }
    }
    // Suche:
    SbiSymDef* pDef = pParser->pPool->Find( aSym );
    if( !pDef )
    {
        // Teil der Runtime-Library?
        // AB 31.3.1996: In Parser-Methode ausgelagert
        // (wird auch in SbiParser::DefVar() in DIM.CXX benoetigt)
        pDef = pParser->CheckRTLForSym( aSym, eType );

        // #i109184: Check if symbol is or later will be defined inside module
        SbModule& rMod = pParser->aGen.GetModule();
        SbxArray* pModMethods = rMod.GetMethods();
        if( pModMethods->Find( aSym, SbxCLASS_DONTCARE ) )
            pDef = NULL;
    }
    if( !pDef )
    {
        // Falls ein Punkt angegeben war, isses Teil eines Objekts,
        // also muss der Returnwert ein Objekt sein
        if( bObj )
            eType = SbxOBJECT;
        pDef = AddSym( eTok, *pParser->pPool, eCurExpr, aSym, eType, pPar );
        // Looks like this is a local ( but undefined variable )
        // if it is in a static procedure then make this Symbol
        // static
        if ( !bObj && pParser->pProc && pParser->pProc->IsStatic() )
            pDef->SetStatic();
    }
    else
    {

        // Symbol ist bereits definiert.
        // Ist es eine Konstante?
        SbiConstDef* pConst = pDef->GetConstDef();
        if( pConst )
        {
            if( pConst->GetType() == SbxSTRING )
                return new SbiExprNode( pParser, pConst->GetString() );
            else
                return new SbiExprNode( pParser, pConst->GetValue(), pConst->GetType() );
        }
        // Hat es Dimensionen,
        // und sind auch Parameter angegeben?
        // (Wobei 0 Parameter () entsprechen)
        if( pDef->GetDims() )
        {
            if( pPar && pPar->GetSize() && pPar->GetSize() != pDef->GetDims() )
                pParser->Error( SbERR_WRONG_DIMS );
        }
        if( pDef->IsDefinedAs() )
        {
            SbxDataType eDefType = pDef->GetType();
            // #119187 Only error if types conflict
            if( eType >= SbxINTEGER && eType <= SbxSTRING && eType != eDefType )
            {
                // Wie? Erst mit AS definieren und dann einen Suffix nehmen?
                pParser->Error( SbERR_BAD_DECLARATION, aSym );
                bError = sal_True;
            }
            else if ( eType == SbxVARIANT )
                // Falls nix angegeben, den Typ des Eintrags nehmen
                // aber nur, wenn die Var nicht mit AS XXX definiert ist
                // damit erwischen wir n% = 5 : print n
                eType = eDefType;
        }
        // Typcheck bei Variablen:
        // ist explizit im Scanner etwas anderes angegeben?
        // Bei Methoden ist dies OK!
        if( eType != SbxVARIANT &&          // Variant nimmt alles
            eType != pDef->GetType() &&
            !pDef->GetProcDef() )
        {
            // Es kann sein, dass pDef ein Objekt beschreibt, das bisher
            // nur als SbxVARIANT erkannt wurde, dann Typ von pDef aendern
            // AB, 16.12.95 (Vielleicht noch aehnliche Faelle moeglich ?!?)
            if( eType == SbxOBJECT && pDef->GetType() == SbxVARIANT )
            {
                pDef->SetType( SbxOBJECT );
            }
            else
            {
                pParser->Error( SbERR_BAD_DECLARATION, aSym );
                bError = sal_True;
            }
        }
    }
    SbiExprNode* pNd = new SbiExprNode( pParser, *pDef, eType );
    if( !pPar )
        pPar = new SbiParameters( pParser,sal_False,sal_False );
    pNd->aVar.pPar = pPar;
    pNd->aVar.pvMorePar = pvMoreParLcl;
    if( bObj )
    {
        // AB, 8.1.95: Objekt kann auch vom Typ SbxVARIANT sein
        if( pDef->GetType() == SbxVARIANT )
            pDef->SetType( SbxOBJECT );
        // Falls wir etwas mit Punkt einscannen, muss der
        // Typ SbxOBJECT sein
        if( pDef->GetType() != SbxOBJECT && pDef->GetType() != SbxVARIANT )
        {
            // defer error until runtime if in vba mode
            if ( !pParser->IsVBASupportOn() )
            {
                pParser->Error( SbERR_BAD_DECLARATION, aSym );
                bError = sal_True;
            }
        }
        if( !bError )
            pNd->aVar.pNext = ObjTerm( *pDef );
    }
    // Merken der Spalte 1 wieder freigeben
    pParser->UnlockColumn();
    return pNd;
}

// Aufbau eines Objekt-Terms. Ein derartiger Term ist Teil
// eines Ausdrucks, der mit einer Objektvariablen beginnt.

SbiExprNode* SbiExpression::ObjTerm( SbiSymDef& rObj )
{
    pParser->Next();
    SbiToken eTok = pParser->Next();
    if( eTok != SYMBOL && !pParser->IsKwd( eTok ) && !pParser->IsExtra( eTok ) )
    {
        // #66745 Einige Operatoren koennen in diesem Kontext auch
        // als Identifier zugelassen werden, wichtig fuer StarOne
        if( eTok != MOD && eTok != NOT && eTok != AND && eTok != OR &&
            eTok != XOR && eTok != EQV && eTok != IMP && eTok != IS )
        {
            pParser->Error( SbERR_VAR_EXPECTED );
            bError = sal_True;
        }
    }
    /* #118410 Allow type for Class methods and RTL object, e.g. RTL.Chr$(97)
    else
    {
        if( pParser->GetType() != SbxVARIANT )
            pParser->Error( SbERR_SYNTAX ), bError = sal_True;
    }
    */
    if( bError )
        return NULL;

    String aSym( pParser->GetSym() );
    SbxDataType eType = pParser->GetType();
    SbiParameters* pPar = NULL;
    SbiExprListVector* pvMoreParLcl = NULL;
    eTok = pParser->Peek();
    // Parameter?
    if( DoParametersFollow( pParser, eCurExpr, eTok ) )
    {
        bool bStandaloneExpression = false;
        pPar = new SbiParameters( pParser, bStandaloneExpression );
        bError |= !pPar->IsValid();
        eTok = pParser->Peek();

        // i109624 check for additional sets of parameters
        while( eTok == LPAREN )
        {
            if( pvMoreParLcl == NULL )
                pvMoreParLcl = new SbiExprListVector();
            SbiParameters* pAddPar = new SbiParameters( pParser );
            pvMoreParLcl->push_back( pAddPar );
            bError |= !pPar->IsValid();
            eTok = pParser->Peek();
        }

    }
    sal_Bool bObj = sal_Bool( ( eTok == DOT || eTok == EXCLAM ) && !pParser->WhiteSpace() );
    if( bObj )
    {
        if( eType == SbxVARIANT )
            eType = SbxOBJECT;
        else
        {
            // Name%. geht wirklich nicht!
            pParser->Error( SbERR_BAD_DECLARATION, aSym );
            bError = sal_True;
        }
    }

    // Der Symbol-Pool eines Objekts ist immer PUBLIC
    SbiSymPool& rPool = rObj.GetPool();
    rPool.SetScope( SbPUBLIC );
    SbiSymDef* pDef = rPool.Find( aSym );
    if( !pDef )
    {
        pDef = AddSym( eTok, rPool, eCurExpr, aSym, eType, pPar );
        pDef->SetType( eType );
    }

    SbiExprNode* pNd = new SbiExprNode( pParser, *pDef, eType );
    pNd->aVar.pPar = pPar;
    pNd->aVar.pvMorePar = pvMoreParLcl;
    if( bObj )
    {
        // Falls wir etwas mit Punkt einscannen, muss der
        // Typ SbxOBJECT sein

        // AB, 3.1.96
        // Es kann sein, dass pDef ein Objekt beschreibt, das bisher
        // nur als SbxVARIANT erkannt wurde, dann Typ von pDef aendern
        if( pDef->GetType() == SbxVARIANT )
            pDef->SetType( SbxOBJECT );

        if( pDef->GetType() != SbxOBJECT )
        {
            pParser->Error( SbERR_BAD_DECLARATION, aSym );
            bError = sal_True;
        }
        if( !bError )
        {
            pNd->aVar.pNext = ObjTerm( *pDef );
            pNd->eType = eType;
        }
    }
    return pNd;
}

// Als Operanden kommen in Betracht:
//      Konstante
//      skalare Variable
//      Strukturelemente
//      Array-Elemente
//      Funktionen
//      geklammerte Ausdruecke

SbiExprNode* SbiExpression::Operand( bool bUsedForTypeOf )
{
    SbiExprNode *pRes;
    SbiToken eTok;

    // Operand testen:
    switch( eTok = pParser->Peek() )
    {
        case SYMBOL:
            pRes = Term();
            // process something like "IF Not r Is Nothing Then .."
            if( !bUsedForTypeOf && pParser->IsVBASupportOn() && pParser->Peek() == IS )
            {
                eTok = pParser->Next();
                pRes = new SbiExprNode( pParser, pRes, eTok, Like() );
            }
            break;
        case DOT:   // .with
            pRes = Term(); break;
        case NUMBER:
            pParser->Next();
            pRes = new SbiExprNode( pParser, pParser->GetDbl(), pParser->GetType() );
            break;
        case FIXSTRING:
            pParser->Next();
            pRes = new SbiExprNode( pParser, pParser->GetSym() ); break;
        case LPAREN:
            pParser->Next();
            if( nParenLevel == 0 && m_eMode == EXPRMODE_LPAREN_PENDING && pParser->Peek() == RPAREN )
            {
                m_eMode = EXPRMODE_EMPTY_PAREN;
                pRes = new SbiExprNode();   // Dummy node
                pParser->Next();
                break;
            }
            nParenLevel++;
            pRes = Boolean();
            if( pParser->Peek() != RPAREN )
            {
                // If there was a LPARAM, it does not belong to the expression
                if( nParenLevel == 1 && m_eMode == EXPRMODE_LPAREN_PENDING )
                    m_eMode = EXPRMODE_LPAREN_NOT_NEEDED;
                else
                    pParser->Error( SbERR_BAD_BRACKETS );
            }
            else
            {
                pParser->Next();
                if( nParenLevel == 1 && m_eMode == EXPRMODE_LPAREN_PENDING )
                {
                    SbiToken eTokAfterRParen = pParser->Peek();
                    if( eTokAfterRParen == EQ || eTokAfterRParen == LPAREN || eTokAfterRParen == DOT )
                        m_eMode = EXPRMODE_ARRAY_OR_OBJECT;
                    else
                        m_eMode = EXPRMODE_STANDARD;
                }
            }
            nParenLevel--;
            pRes->bComposite = sal_True;
            break;
        default:
            // Zur Zeit sind Keywords hier OK!
            if( pParser->IsKwd( eTok ) )
                pRes = Term();
            else
            {
                pParser->Next();
                pRes = new SbiExprNode( pParser, 1.0, SbxDOUBLE ); // bei Fehlern
                pParser->Error( SbERR_UNEXPECTED, eTok );
            }
    }
    return pRes;
}

SbiExprNode* SbiExpression::Unary()
{
    SbiExprNode* pNd;
    SbiToken eTok = pParser->Peek();
    switch( eTok )
    {
        case MINUS:
            eTok = NEG;
            pParser->Next();
            pNd = new SbiExprNode( pParser, Unary(), eTok, NULL );
            break;
        case NOT:
            if( pParser->IsVBASupportOn() )
            {
                pNd = Operand();
            }
            else
            {
                pParser->Next();
                pNd = new SbiExprNode( pParser, Unary(), eTok, NULL );
            }
            break;
        case PLUS:
            pParser->Next();
            pNd = Unary();
            break;
        case TYPEOF:
        {
            pParser->Next();
            bool bUsedForTypeOf = true;
            SbiExprNode* pObjNode = Operand( bUsedForTypeOf );
            pParser->TestToken( IS );
            String aDummy;
            SbiSymDef* pTypeDef = new SbiSymDef( aDummy );
            pParser->TypeDecl( *pTypeDef, sal_True );
            pNd = new SbiExprNode( pParser, pObjNode, pTypeDef->GetTypeId() );
            break;
        }
        case NEW:
        {
            pParser->Next();
            String aStr;
            SbiSymDef* pTypeDef = new SbiSymDef( aStr );
            pParser->TypeDecl( *pTypeDef, sal_True );
            pNd = new SbiExprNode( pParser, pTypeDef->GetTypeId() );
            break;
        }
        default:
            pNd = Operand();
    }
    return pNd;
}

SbiExprNode* SbiExpression::Exp()
{
    SbiExprNode* pNd = Unary();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        while( pParser->Peek() == EXPON ) {
            SbiToken eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, Unary() );
        }
    }
    return pNd;
}

SbiExprNode* SbiExpression::MulDiv()
{
    SbiExprNode* pNd = Exp();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        for( ;; )
        {
            SbiToken eTok = pParser->Peek();
            if( eTok != MUL && eTok != DIV )
                break;
            eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, Exp() );
        }
    }
    return pNd;
}

SbiExprNode* SbiExpression::IntDiv()
{
    SbiExprNode* pNd = MulDiv();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        while( pParser->Peek() == IDIV ) {
            SbiToken eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, MulDiv() );
        }
    }
    return pNd;
}

SbiExprNode* SbiExpression::Mod()
{
    SbiExprNode* pNd = IntDiv();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        while( pParser->Peek() == MOD ) {
            SbiToken eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, IntDiv() );
        }
    }
    return pNd;
}

SbiExprNode* SbiExpression::AddSub()
{
    SbiExprNode* pNd = Mod();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        for( ;; )
        {
            SbiToken eTok = pParser->Peek();
            if( eTok != PLUS && eTok != MINUS )
                break;
            eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, Mod() );
        }
    }
    return pNd;
}

SbiExprNode* SbiExpression::Cat()
{
    SbiExprNode* pNd = AddSub();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        for( ;; )
        {
            SbiToken eTok = pParser->Peek();
            if( eTok != CAT )
                break;
            eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, AddSub() );
        }
    }
    return pNd;
}

SbiExprNode* SbiExpression::Comp()
{
    SbiExprNode* pNd = Cat();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        short nCount = 0;
        for( ;; )
        {
            SbiToken eTok = pParser->Peek();
            if( m_eMode == EXPRMODE_ARRAY_OR_OBJECT )
                break;
            if( eTok != EQ && eTok != NE && eTok != LT
             && eTok != GT && eTok != LE && eTok != GE )
                break;
            eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, Cat() );
            nCount++;
        }
    }
    return pNd;
}


SbiExprNode* SbiExpression::VBA_Not()
{
    SbiExprNode* pNd = NULL;

    SbiToken eTok = pParser->Peek();
    if( eTok == NOT )
    {
        pParser->Next();
        pNd = new SbiExprNode( pParser, VBA_Not(), eTok, NULL );
    }
    else
    {
        pNd = Comp();
    }
    return pNd;
}

SbiExprNode* SbiExpression::VBA_And()
{
    SbiExprNode* pNd = VBA_Not();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        for( ;; )
        {
            SbiToken eTok = pParser->Peek();
            if( eTok != AND )
                break;
            eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, VBA_Not() );
        }
    }
    return pNd;
}

SbiExprNode* SbiExpression::VBA_Or()
{
    SbiExprNode* pNd = VBA_And();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        for( ;; )
        {
            SbiToken eTok = pParser->Peek();
            if( eTok != OR )
                break;
            eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, VBA_And() );
        }
    }
    return pNd;
}

SbiExprNode* SbiExpression::VBA_Xor()
{
    SbiExprNode* pNd = VBA_Or();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        for( ;; )
        {
            SbiToken eTok = pParser->Peek();
            if( eTok != XOR )
                break;
            eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, VBA_Or() );
        }
    }
    return pNd;

}

SbiExprNode* SbiExpression::VBA_Eqv()
{
    SbiExprNode* pNd = VBA_Xor();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        for( ;; )
        {
            SbiToken eTok = pParser->Peek();
            if( eTok != EQV )
                break;
            eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, VBA_Xor() );
        }
    }
    return pNd;
}

SbiExprNode* SbiExpression::VBA_Imp()
{
    SbiExprNode* pNd = VBA_Eqv();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        for( ;; )
        {
            SbiToken eTok = pParser->Peek();
            if( eTok != IMP )
                break;
            eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, VBA_Eqv() );
        }
    }
    return pNd;

}

SbiExprNode* SbiExpression::Like()
{
    SbiExprNode* pNd = pParser->IsVBASupportOn() ? VBA_Not() : Comp();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        short nCount = 0;
        while( pParser->Peek() == LIKE ) {
            SbiToken eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, Comp() ), nCount++;
        }
        // Mehrere Operatoren hintereinander gehen nicht
        if( nCount > 1 && !pParser->IsVBASupportOn() )
        {
            pParser->Error( SbERR_SYNTAX );
            bError = sal_True;
        }
    }
    return pNd;
}

SbiExprNode* SbiExpression::Boolean()
{
    SbiExprNode* pNd = Like();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        for( ;; )
        {
            SbiToken eTok = pParser->Peek();
            if( eTok != AND && eTok != OR && eTok != XOR
             && eTok != EQV && eTok != IMP && eTok != IS )
                break;
            eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, Like() );
        }
    }
    return pNd;
}

/***************************************************************************
|*
|*      SbiConstExpression
|*
***************************************************************************/

// Parsing einer Expression, die sich zu einer numerischen
// Konstanten verarbeiten laesst.

SbiConstExpression::SbiConstExpression( SbiParser* p ) : SbiExpression( p )
{
    if( pExpr->IsConstant() )
    {
        eType = pExpr->GetType();
        if( pExpr->IsNumber() )
        {
            nVal = pExpr->nVal;
        }
        else
        {
            nVal = 0;
            aVal = pExpr->aStrVal;
        }
    }
    else
    {
        // #40204 Spezialbehandlung fuer sal_Bool-Konstanten
        sal_Bool bIsBool = sal_False;
        if( pExpr->eNodeType == SbxVARVAL )
        {
            SbiSymDef* pVarDef = pExpr->GetVar();

            // Ist es eine sal_Bool-Konstante?
            sal_Bool bBoolVal = sal_False;
            if( pVarDef->GetName().EqualsIgnoreCaseAscii( "true" ) )
            //if( pVarDef->GetName().ICompare( "true" ) == COMPARE_EQUAL )
            {
                bIsBool = sal_True;
                bBoolVal = sal_True;
            }
            else if( pVarDef->GetName().EqualsIgnoreCaseAscii( "false" ) )
            //else if( pVarDef->GetName().ICompare( "false" ) == COMPARE_EQUAL )
            {
                bIsBool = sal_True;
                bBoolVal = sal_False;
            }

            // Wenn es ein sal_Bool ist, Node austauschen
            if( bIsBool )
            {
                delete pExpr;
                pExpr = new SbiExprNode( pParser, (bBoolVal ? SbxTRUE : SbxFALSE), SbxINTEGER );
                eType = pExpr->GetType();
                nVal = pExpr->nVal;
            }
        }

        if( !bIsBool )
        {
            pParser->Error( SbERR_SYNTAX );
            eType = SbxDOUBLE;
            nVal = 0;
        }
    }
}

short SbiConstExpression::GetShortValue()
{
    if( eType == SbxSTRING )
    {
        SbxVariableRef refConv = new SbxVariable;
        refConv->PutString( aVal );
        return refConv->GetInteger();
    }
    else
    {
        double n = nVal;
        if( n > 0 ) n += .5; else n -= .5;
        if( n > SbxMAXINT ) n = SbxMAXINT, pParser->Error( SbERR_OUT_OF_RANGE );
        else
        if( n < SbxMININT ) n = SbxMININT, pParser->Error( SbERR_OUT_OF_RANGE );
        return (short) n;
    }
}


/***************************************************************************
|*
|*      SbiExprList
|*
***************************************************************************/

SbiExprList::SbiExprList( SbiParser* p )
{
    pParser = p;
    pFirst = NULL;
    nExpr  =
    nDim   = 0;
    bError =
    bBracket = sal_False;
}

SbiExprList::~SbiExprList()
{
    SbiExpression* p = pFirst;
    while( p )
    {
        SbiExpression* q = p->pNext;
        delete p;
        p = q;
    }
}

// Parameter anfordern (ab 0)

SbiExpression* SbiExprList::Get( short n )
{
    SbiExpression* p = pFirst;
    while( n-- && p )
        p = p->pNext;
    return p;
}

void SbiExprList::addExpression( SbiExpression* pExpr )
{
    SbiExpression* p = pFirst;
    while( p && p->pNext )
        p = p->pNext;

    p->pNext = pExpr;
}


/***************************************************************************
|*
|*      SbiParameters
|*
***************************************************************************/

// Parsender Konstruktor:
// Die Parameterliste wird komplett geparst.
// "Prozedurname()" ist OK.
// Dann handelt es sich um eine Funktion ohne Parameter
// respektive um die Angabe eines Arrays als Prozedurparameter.

// #i79918/#i80532: bConst has never been set to true
// -> reused as bStandaloneExpression
//SbiParameters::SbiParameters( SbiParser* p, sal_Bool bConst, sal_Bool bPar) :
SbiParameters::SbiParameters( SbiParser* p, sal_Bool bStandaloneExpression, sal_Bool bPar) :
    SbiExprList( p )
{
    if( !bPar )
        return;

    SbiExpression *pExpr;
    SbiToken eTok = pParser->Peek();

    // evtl. Klammer auf weg:
    bool bAssumeExprLParenMode = false;
    bool bAssumeArrayMode = false;
    if( eTok == LPAREN )
    {
        if( bStandaloneExpression )
        {
            bAssumeExprLParenMode = true;
        }
        else
        {
            bBracket = sal_True;
            pParser->Next();
            eTok = pParser->Peek();
        }
    }

    // Ende-Test
    if( ( bBracket && eTok == RPAREN ) || pParser->IsEoln( eTok ) )
    {
        if( eTok == RPAREN )
            pParser->Next();
        return;
    }
    // Parametertabelle einlesen und in richtiger Folge ablegen!
    SbiExpression* pLast = NULL;
    String aName;
    while( !bError )
    {
        aName.Erase();
        // Fehlendes Argument
        if( eTok == COMMA )
        {
            pExpr = new SbiExpression( pParser, 0, SbxEMPTY );
            //if( bConst )
            //  pParser->Error( SbERR_SYNTAX ), bError = sal_True;
        }
        // Benannte Argumente: entweder .name= oder name:=
        else
        {
            bool bByVal = false;
            if( eTok == BYVAL )
            {
                bByVal = true;
                pParser->Next();
                eTok = pParser->Peek();
            }

            if( bAssumeExprLParenMode )
            {
                pExpr = new SbiExpression( pParser, SbSTDEXPR, EXPRMODE_LPAREN_PENDING );
                bAssumeExprLParenMode = sal_False;

                SbiExprMode eModeAfter = pExpr->m_eMode;
                if( eModeAfter == EXPRMODE_LPAREN_NOT_NEEDED )
                {
                    bBracket = sal_True;
                }
                else if( eModeAfter == EXPRMODE_ARRAY_OR_OBJECT )
                {
                    // Expression "looks" like an array assignment
                    // a(...)[(...)] = ? or a(...).b(...)
                    // RPAREN is already parsed
                    bBracket = sal_True;
                    bAssumeArrayMode = true;
                    eTok = NIL;
                }
                else if( eModeAfter == EXPRMODE_EMPTY_PAREN )
                {
                    bBracket = sal_True;
                    delete pExpr;
                    if( bByVal )
                        pParser->Error( SbERR_LVALUE_EXPECTED );
                    return;
                }
            }
            else
                pExpr = new SbiExpression( pParser );

            if( bByVal && pExpr->IsLvalue() )
                pExpr->SetByVal();

            //pExpr = bConst ? new SbiConstExpression( pParser )
            //              : new SbiExpression( pParser );
            if( !bAssumeArrayMode )
            {
                if( pParser->Peek() == ASSIGN )
                {
                    // VBA mode: name:=
                    // SbiExpression::Term() hat einen String daraus gemacht
                    aName = pExpr->GetString();
                    delete pExpr;
                    pParser->Next();
                    pExpr = new SbiExpression( pParser );
                    //if( bConst )
                    //  pParser->Error( SbERR_SYNTAX ), bError = sal_True;
                }
                pExpr->GetName() = aName;
            }
        }
        pExpr->pNext = NULL;
        if( !pLast )
            pFirst = pLast = pExpr;
        else
            pLast->pNext = pExpr, pLast = pExpr;
        nExpr++;
        bError |= !pExpr->IsValid();

        if( bAssumeArrayMode )
            break;

        // Naechstes Element?
        eTok = pParser->Peek();
        if( eTok != COMMA )
        {
            if( ( bBracket && eTok == RPAREN ) || pParser->IsEoln( eTok ) )
                break;
            pParser->Error( bBracket
                            ? SbERR_BAD_BRACKETS
                            : SbERR_EXPECTED, COMMA );
            bError = sal_True;
        }
        else
        {
            pParser->Next();
            eTok = pParser->Peek();
            if( ( bBracket && eTok == RPAREN ) || pParser->IsEoln( eTok ) )
                break;
        }
    }
    // Schliessende Klammer
    if( eTok == RPAREN )
    {
        pParser->Next();
        pParser->Peek();
        if( !bBracket )
        {
            pParser->Error( SbERR_BAD_BRACKETS );
            bError = sal_True;
        }
    }
    nDim = nExpr;
}

/***************************************************************************
|*
|*      SbiDimList
|*
***************************************************************************/

// Parsender Konstruktor:
// Eine Liste von Array-Dimensionen wird geparst. Die Ausdruecke werden
// auf numerisch getestet. Das bCONST-Bit wird gesetzt, wenn alle Ausdruecke
// Integer-Konstanten sind.

SbiDimList::SbiDimList( SbiParser* p ) : SbiExprList( p )
{
    bConst = sal_True;

    if( pParser->Next() != LPAREN )
    {
        pParser->Error( SbERR_EXPECTED, LPAREN );
        bError = sal_True; return;
    }

    if( pParser->Peek() != RPAREN )
    {
        SbiExpression *pExpr1, *pExpr2, *pLast = NULL;
        SbiToken eTok;
        for( ;; )
        {
            pExpr1 = new SbiExpression( pParser );
            eTok = pParser->Next();
            if( eTok == TO )
            {
                pExpr2 = new SbiExpression( pParser );
                eTok = pParser->Next();
                bConst &= pExpr1->IsIntConstant() & pExpr2->IsIntConstant();
                bError |= !pExpr1->IsValid();
                bError |= !pExpr2->IsValid();
                pExpr1->pNext = pExpr2;
                if( !pLast )
                    pFirst = pExpr1;
                else
                    pLast->pNext = pExpr1;
                pLast = pExpr2;
                nExpr += 2;
            }
            else
            {
                // Nur eine Dim-Angabe
                pExpr1->SetBased();
                pExpr1->pNext = NULL;
                bConst &= pExpr1->IsIntConstant();
                bError |= !pExpr1->IsValid();
                if( !pLast )
                    pFirst = pLast = pExpr1;
                else
                    pLast->pNext = pExpr1, pLast = pExpr1;
                nExpr++;
            }
            nDim++;
            if( eTok == RPAREN ) break;
            if( eTok != COMMA )
            {
                pParser->Error( SbERR_BAD_BRACKETS );
                pParser->Next();
                break;
            }
        }
    }
    else pParser->Next();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
