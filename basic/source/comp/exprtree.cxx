/*************************************************************************
 *
 *  $RCSfile: exprtree.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ab $ $Date: 2000-10-10 13:02:28 $
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
#include <svtools/sbx.hxx>      // w.g. ...IMPL_REF(...sbxvariable)
#include "expr.hxx"

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBCEXPR, SBCOMP_CODE )

/***************************************************************************
|*
|*      SbiExpression
|*
***************************************************************************/

SbiExpression::SbiExpression( SbiParser* p, SbiExprType t )
{
    pParser = p;
    bError = bByVal = bBased = FALSE;
    eCurExpr = t;
    pNext = NULL;
    pExpr = (t != SbSTDEXPR ) ? Term() : Boolean();
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
    bError = bByVal = bBased = FALSE;
    pExpr = new SbiExprNode( pParser, n, t );
    pExpr->Optimize();
}

SbiExpression::SbiExpression( SbiParser* p, const String& r )
{
    pParser = p;
    pNext = NULL;
    bError = bByVal = bBased = FALSE;
    eCurExpr = SbOPERAND;
    pExpr = new SbiExprNode( pParser, r );
}

SbiExpression::SbiExpression( SbiParser* p, const SbiSymDef& r, SbiExprList* pPar )
{
    pParser = p;
    pNext = NULL;
    bError = bByVal = bBased = FALSE;
    eCurExpr = SbOPERAND;
    pExpr = new SbiExprNode( pParser, r, SbxVARIANT, pPar );
}

SbiExpression::SbiExpression( SbiParser* p, SbiToken t )
{
    pParser = p;
    pNext = NULL;
    bError = bByVal = bBased = FALSE;
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

static BOOL DoParametersFollow( SbiParser* p, SbiExprType eCurExpr, SbiToken eTok )
{
    if( eTok == LPAREN )
        return TRUE;
    // Aber nur, wenn CALL-aehnlich!
    if( !p->WhiteSpace() || eCurExpr != SbSYMBOL )
        return FALSE;
    return BOOL(
           eTok == NUMBER || eTok == FIXSTRING
        || eTok == SYMBOL || eTok == COMMA  || eTok == DOT );
}

// Definition eines neuen Symbols

static SbiSymDef* AddSym
    ( SbiToken eTok, SbiSymPool& rPool, SbiExprType eCurExpr,
      const String& rName, SbxDataType eType, SbiParameters* pPar )
{
    SbiSymDef* pDef;
    // A= ist keine Prozedur
    BOOL bHasType = BOOL( eTok == EQ || eTok == DOT );
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
            bHasType = TRUE;

        pDef = pProc;
        pDef->SetType( bHasType ? eType : SbxEMPTY );
        if( pPar )
        {
            // Dummy-Parameter generieren
            USHORT n = 1;
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

SbiExprNode* SbiExpression::Term()
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

    SbiToken eTok = pParser->Next();
    // Anfang des Parsings merken
    pParser->LockColumn();
    String aSym( pParser->GetSym() );
    SbxDataType eType = pParser->GetType();
    SbiParameters* pPar = NULL;
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
        pParser->Error( SbERR_SYNTAX );
        bError = TRUE;
    }

    if( DoParametersFollow( pParser, eCurExpr, eTok = eNextTok ) )
    {
        pPar = new SbiParameters( pParser );
        bError |= !pPar->IsValid();
        eTok = pParser->Peek();
    }
    // Es koennte ein Objektteil sein, wenn . oder ! folgt
    // Bei . muss aber die Variable bereits definiert sein; wenn pDef
    // nach der Suche NULL ist, isses ein Objekt!
    BOOL bObj = BOOL( ( eTok == DOT || eTok == EXCLAM )
                    && !pParser->WhiteSpace() );
    if( bObj )
    {
        if( eType == SbxVARIANT )
            eType = SbxOBJECT;
        else
        {
            // Name%. geht wirklich nicht!
            pParser->Error( SbERR_BAD_DECLARATION, aSym );
            bError = TRUE;
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
    }
    if( !pDef )
    {
        // Falls ein Punkt angegeben war, isses Teil eines Objekts,
        // also muss der Returnwert ein Objekt sein
        if( bObj )
            eType = SbxOBJECT;
        pDef = AddSym( eTok, *pParser->pPool, eCurExpr, aSym, eType, pPar );
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
            if( !pPar
            || ( pPar->GetSize() && pPar->GetSize() != pDef->GetDims() ) )
                pParser->Error( SbERR_WRONG_DIMS );
        }
        if( pDef->IsDefinedAs() )
        {
            if( eType >= SbxINTEGER && eType <= SbxSTRING )
            {
                // Wie? Erst mit AS definieren und dann einen Suffix nehmen?
                pParser->Error( SbERR_BAD_DECLARATION, aSym );
                bError = TRUE;
            }
            else if ( eType == SbxVARIANT )
                // Falls nix angegeben, den Typ des Eintrags nehmen
                // aber nur, wenn die Var nicht mit AS XXX definiert ist
                // damit erwischen wir n% = 5 : print n
                eType = pDef->GetType();
        }
        // Funktion?
        if( pDef->GetProcDef() )
        {
            SbiProcDef* pProc = pDef->GetProcDef();
            if( pPar && pProc->GetLib().Len() )     // DECLARE benutzt?
                pPar->SetProc( pProc );
            // Wenn keine Pars, vorerst nichts machen
            // Pruefung auf Typ-Anzahl waere denkbar
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
                bError = TRUE;
            }
        }
    }
    SbiExprNode* pNd = new SbiExprNode( pParser, *pDef, eType );
    if( !pPar )
        pPar = new SbiParameters( pParser,FALSE,FALSE );
    pNd->aVar.pPar = pPar;
    if( bObj )
    {
        // AB, 8.1.95: Objekt kann auch vom Typ SbxVARIANT sein
        if( pDef->GetType() == SbxVARIANT )
            pDef->SetType( SbxOBJECT );
        // Falls wir etwas mit Punkt einscannen, muss der
        // Typ SbxOBJECT sein
        if( pDef->GetType() != SbxOBJECT && pDef->GetType() != SbxVARIANT )
        {
            pParser->Error( SbERR_BAD_DECLARATION, aSym );
            bError = TRUE;
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
            bError = TRUE;
        }
    }
    else
    {
        if( pParser->GetType() != SbxVARIANT )
            pParser->Error( SbERR_SYNTAX ), bError = TRUE;
    }
    if( bError )
        return NULL;

    String aSym( pParser->GetSym() );
    SbxDataType eType = pParser->GetType();
    SbiParameters* pPar = NULL;
    eTok = pParser->Peek();
    // Parameter?
    if( DoParametersFollow( pParser, eCurExpr, eTok ) )
    {
        pPar = new SbiParameters( pParser );
        bError |= !pPar->IsValid();
        eTok = pParser->Peek();
    }
    BOOL bObj = BOOL( ( eTok == DOT || eTok == EXCLAM ) && !pParser->WhiteSpace() );
    if( bObj )
    {
        if( eType == SbxVARIANT )
            eType = SbxOBJECT;
        else
        {
            // Name%. geht wirklich nicht!
            pParser->Error( SbERR_BAD_DECLARATION, aSym );
            bError = TRUE;
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
            bError = TRUE;
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

SbiExprNode* SbiExpression::Operand()
{
    SbiExprNode *pRes;
    SbiToken eTok;

    // Operand testen:
    switch( eTok = pParser->Peek() )
    {
        case SYMBOL:
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
            pRes = Boolean();
            if( pParser->Peek() != RPAREN )
                pParser->Error( SbERR_BAD_BRACKETS );
            else pParser->Next();
            pRes->bComposite = TRUE;
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
        case NOT:
            pParser->Next();
            pNd = new SbiExprNode( pParser, Unary(), eTok, NULL );
            break;
        case PLUS:
            pParser->Next();
            pNd = Unary();
            break;
        default:
            pNd = Operand();
    }
    return pNd;
}

SbiExprNode* SbiExpression::Exp()
{
    SbiExprNode* pNd = Unary();
    while( pParser->Peek() == EXPON ) {
        SbiToken eTok = pParser->Next();
        pNd = new SbiExprNode( pParser, pNd, eTok, Unary() );
    }
    return pNd;
}

SbiExprNode* SbiExpression::MulDiv()
{
    SbiExprNode* pNd = Exp();
    for( ;; )
    {
        SbiToken eTok = pParser->Peek();
        if( eTok != MUL && eTok != DIV )
            break;
        eTok = pParser->Next();
        pNd = new SbiExprNode( pParser, pNd, eTok, Exp() );
    }
    return pNd;
}

SbiExprNode* SbiExpression::IntDiv()
{
    SbiExprNode* pNd = MulDiv();
    while( pParser->Peek() == IDIV ) {
        SbiToken eTok = pParser->Next();
        pNd = new SbiExprNode( pParser, pNd, eTok, MulDiv() );
    }
    return pNd;
}

SbiExprNode* SbiExpression::Mod()
{
    SbiExprNode* pNd = IntDiv();
    while( pParser->Peek() == MOD ) {
        SbiToken eTok = pParser->Next();
        pNd = new SbiExprNode( pParser, pNd, eTok, IntDiv() );
    }
    return pNd;
}

SbiExprNode* SbiExpression::AddSub()
{
    SbiExprNode* pNd = Mod();
    for( ;; )
    {
        SbiToken eTok = pParser->Peek();
        if( eTok != PLUS && eTok != MINUS )
            break;
        eTok = pParser->Next();
        pNd = new SbiExprNode( pParser, pNd, eTok, Mod() );
    }
    return pNd;
}

SbiExprNode* SbiExpression::Cat()
{
    SbiExprNode* pNd = AddSub();
    for( ;; )
    {
        SbiToken eTok = pParser->Peek();
        if( eTok != CAT )
            break;
        eTok = pParser->Next();
        pNd = new SbiExprNode( pParser, pNd, eTok, AddSub() );
    }
    return pNd;
}

SbiExprNode* SbiExpression::Comp()
{
    SbiExprNode* pNd = Cat();
    short nCount = 0;
    for( ;; )
    {
        SbiToken eTok = pParser->Peek();
        if( eTok != EQ && eTok != NE && eTok != LT
         && eTok != GT && eTok != LE && eTok != GE )
            break;
        eTok = pParser->Next();
        pNd = new SbiExprNode( pParser, pNd, eTok, Cat() );
        nCount++;
    }
    // Mehrere Operatoren hintereinander gehen nicht
    if( nCount > 1 )
    {
        pParser->Error( SbERR_SYNTAX );
        bError = TRUE;
    }
    return pNd;
}

SbiExprNode* SbiExpression::Like()
{
    SbiExprNode* pNd = Comp();
    short nCount = 0;
    while( pParser->Peek() == LIKE ) {
        SbiToken eTok = pParser->Next();
        pNd = new SbiExprNode( pParser, pNd, eTok, Comp() ), nCount++;
    }
    // Mehrere Operatoren hintereinander gehen nicht
    if( nCount > 1 )
    {
        pParser->Error( SbERR_SYNTAX );
        bError = TRUE;
    }
    return pNd;
}

SbiExprNode* SbiExpression::Boolean()
{
    SbiExprNode* pNd = Like();
    for( ;; )
    {
        SbiToken eTok = pParser->Peek();
        if( eTok != AND && eTok != OR && eTok != XOR
         && eTok != EQV && eTok != IMP && eTok != IS )
            break;
        eTok = pParser->Next();
        pNd = new SbiExprNode( pParser, pNd, eTok, Like() );
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
            nVal = pExpr->nVal;
        else
            nVal = 0, aVal = pParser->aGblStrings.Find( pExpr->nStringId );
    }
    else
    {
        // #40204 Spezialbehandlung fuer BOOL-Konstanten
        BOOL bIsBool = FALSE;
        if( pExpr->eNodeType == SbxVARVAL )
        {
            SbiSymDef* pVarDef = pExpr->GetVar();

            // Ist es eine BOOL-Konstante?
            BOOL bBoolVal;
            if( pVarDef->GetName().EqualsIgnoreCaseAscii( "true" ) )
            //if( pVarDef->GetName().ICompare( "true" ) == COMPARE_EQUAL )
            {
                bIsBool = TRUE;
                bBoolVal = TRUE;
            }
            else if( pVarDef->GetName().EqualsIgnoreCaseAscii( "false" ) )
            //else if( pVarDef->GetName().ICompare( "false" ) == COMPARE_EQUAL )
            {
                bIsBool = TRUE;
                bBoolVal = FALSE;
            }

            // Wenn es ein BOOL ist, Node austauschen
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
    pProc = NULL;
    nExpr  =
    nDim   = 0;
    bError =
    bBracket = FALSE;
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

SbiParameters::SbiParameters( SbiParser* p, BOOL bConst, BOOL bPar) :
    SbiExprList( p )
{
    if (bPar)
    {
        SbiExpression *pExpr;
        SbiToken eTok = pParser->Peek();

        // evtl. Klammer auf weg:
        if( eTok == LPAREN )
        {
            bBracket = TRUE; pParser->Next(); eTok = pParser->Peek();
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
                if( bConst )
                    pParser->Error( SbERR_SYNTAX ), bError = TRUE;
            }
            // Benannte Argumente: entweder .name= oder name:=
            else
            {
                if( eTok == DOT )
                {
                    // VB mode: .name=
                    pParser->Next();
                    pParser->TestSymbol( TRUE );    // Keywords sind OK
                    aName = pParser->GetSym();
                    pParser->TestToken( EQ );
                    pExpr = bConst ? new SbiConstExpression( pParser )
                                   : new SbiExpression( pParser );
                }
                else
                {
                    pExpr = bConst ? new SbiConstExpression( pParser )
                                   : new SbiExpression( pParser );
                    if( pParser->Peek() == ASSIGN )
                    {
                        // VBA mode: name:=
                        // SbiExpression::Term() hat einen String daraus gemacht
                        aName = pExpr->GetString();
                        delete pExpr;
                        pParser->Next();
                        pExpr = new SbiExpression( pParser );
                        if( bConst )
                            pParser->Error( SbERR_SYNTAX ), bError = TRUE;
                    }
                }
                pExpr->GetName() = aName;
            }
            pExpr->pNext = NULL;
            if( !pLast )
                pFirst = pLast = pExpr;
            else
                pLast->pNext = pExpr, pLast = pExpr;
            nExpr++;
            bError |= !pExpr->IsValid();
            // Naechstes Element?
            eTok = pParser->Peek();
            if( eTok != COMMA )
            {
                if( ( bBracket && eTok == RPAREN ) || pParser->IsEoln( eTok ) )
                    break;
                pParser->Error( bBracket
                                ? SbERR_BAD_BRACKETS
                                : SbERR_EXPECTED, COMMA );
                bError = TRUE;
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
                bError = TRUE;
            }
        }
        nDim = nExpr;
    }
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
    bConst = TRUE;

    if( pParser->Next() != LPAREN )
    {
        pParser->Error( SbERR_EXPECTED, LPAREN );
        bError = TRUE; return;
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

