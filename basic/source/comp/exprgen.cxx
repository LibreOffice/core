/*************************************************************************
 *
 *  $RCSfile: exprgen.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ab $ $Date: 2000-10-10 13:02:03 $
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
#include "expr.hxx"

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBCEXPR, SBCOMP_CODE )

// Umsetztabelle fuer Token-Operatoren und Opcodes

typedef struct {
        SbiToken  eTok;                 // Token
        SbiOpcode eOp;                  // Opcode
} OpTable;

static OpTable aOpTable [] = {
    { EXPON,_EXP },
    { MUL,  _MUL },
    { DIV,  _DIV },
    { IDIV, _IDIV },
    { MOD,  _MOD },
    { PLUS, _PLUS },
    { MINUS,_MINUS },
    { EQ,   _EQ },
    { NE,   _NE },
    { LE,   _LE },
    { GE,   _GE },
    { LT,   _LT },
    { GT,   _GT },
    { AND,  _AND },
    { OR,   _OR },
    { XOR,  _XOR },
    { EQV,  _EQV },
    { IMP,  _IMP },
    { NOT,  _NOT },
    { NEG,  _NEG },
    { CAT,  _CAT },
    { LIKE, _LIKE },
    { IS,   _IS },
    { NIL,  _NOP }};

// Ausgabe eines Elements

void SbiExprNode::Gen()
{
    if( IsConstant() )
    {
        switch( GetType() )
        {
            case SbxEMPTY:   pGen->Gen( _EMPTY ); break;
            case SbxINTEGER: pGen->Gen( _CONST,  (short) nVal ); break;
            case SbxSTRING:  pGen->Gen( _SCONST, nStringId ); break;
            default:
                nStringId = pGen->GetParser()->aGblStrings.Add( nVal, eType );
                pGen->Gen( _NUMBER, nStringId );
        }
    }
    else if( IsOperand() )
    {
        SbiExprNode* pWithParent = NULL;
        SbiOpcode eOp;
        if( aVar.pDef->GetScope() == SbPARAM )
            eOp = _PARAM;
        // AB: 17.12.1995, Spezialbehandlung fuer WITH
        else if( (pWithParent = GetWithParent()) != NULL )
        {
            eOp = _ELEM;            // .-Ausdruck in WITH
        }
        else
        {
            SbiProcDef* pProc = aVar.pDef->GetProcDef();
            // per DECLARE definiert?
            if( pProc && pProc->GetLib().Len() )
                eOp = pProc->IsCdecl() ? _CALLC : _CALL;
            else
                eOp = ( aVar.pDef->GetScope() == SbRTL ) ? _RTL : _FIND;
        }

        for( SbiExprNode* p = this; p; p = p->aVar.pNext )
        {
            if( p == this && pWithParent != NULL )
                pWithParent->Gen();
            p->GenElement( eOp );
            eOp = _ELEM;
        }
    }
    else
    {
        pLeft->Gen();
        if( pRight )
            pRight->Gen();
        for( OpTable* p = aOpTable; p->eTok != NIL; p++ )
        {
            if( p->eTok == eTok )
            {
                pGen->Gen( p->eOp ); break;
            }
        }
    }
}

// Ausgabe eines Operanden-Elements

void SbiExprNode::GenElement( SbiOpcode eOp )
{
#ifndef PRODUCT
    if( eOp < _RTL || eOp > _CALLC )
        pGen->GetParser()->Error( SbERR_INTERNAL_ERROR, "Opcode" );
#endif
    SbiSymDef* pDef = aVar.pDef;
    // Das ID ist entweder die Position oder das String-ID
    // Falls das Bit 0x8000 gesetzt ist, hat die Variable
    // eine Parameterliste.
    USHORT nId = ( eOp == _PARAM ) ? pDef->GetPos() : pDef->GetId();
    // Parameterliste aufbauen
    if( aVar.pPar && aVar.pPar->GetSize() )
    {
        nId |= 0x8000;
        aVar.pPar->Gen();
    }
    SbiProcDef* pProc = aVar.pDef->GetProcDef();
    // per DECLARE definiert?
    if( pProc )
    {
        // Dann evtl. einen LIB-Befehl erzeugen
        if( pProc->GetLib().Len() )
            pGen->Gen( _LIB, pGen->GetParser()->aGblStrings.Add( pProc->GetLib() ) );
        // und den Aliasnamen nehmen
        if( pProc->GetAlias().Len() )
            nId = ( nId & 0x8000 ) | pGen->GetParser()->aGblStrings.Add( pProc->GetAlias() );
    }
    pGen->Gen( eOp, nId, GetType() );
}

// Erzeugen einer Argv-Tabelle
// Das erste Element bleibt immer frei fuer Returnwerte etc.
// Siehe auch SbiProcDef::SbiProcDef() in symtbl.cxx

void SbiExprList::Gen()
{
    if( pFirst )
    {
        pParser->aGen.Gen( _ARGC );
        // AB 10.1.96: Typ-Anpassung bei DECLARE
        USHORT nCount = 1, nParAnz = 0;
        SbiSymPool* pPool;
        if( pProc )
        {
            pPool = &pProc->GetParams();
            nParAnz = pPool->GetSize();
        }
        for( SbiExpression* pExpr = pFirst; pExpr; pExpr = pExpr->pNext,nCount++ )
        {
            pExpr->Gen();
            if( pExpr->GetName().Len() )
            {
                // named arg
                USHORT nSid = pParser->aGblStrings.Add( pExpr->GetName() );
                pParser->aGen.Gen( _ARGN, nSid );

                // AB 10.1.96: Typanpassung bei named -> passenden Parameter suchen
                if( pProc )
                {
                    // Vorerst: Error ausloesen
                    pParser->Error( SbERR_NO_NAMED_ARGS );

                    // Spaeter, wenn Named Args bei DECLARE moeglich
                    /*
                    for( USHORT i = 1 ; i < nParAnz ; i++ )
                    {
                        SbiSymDef* pDef = pPool->Get( i );
                        const String& rName = pDef->GetName();
                        if( rName.Len() )
                        {
                            if( pExpr->GetName().ICompare( rName )
                                == COMPARE_EQUAL )
                            {
                                pParser->aGen.Gen( _ARGTYP, pDef->GetType() );
                                break;
                            }
                        }
                    }
                    */
                }
            }
            else
            {
                pParser->aGen.Gen( _ARGV );

                // Funktion mit DECLARE -> Typ-Anpassung
                if( pProc && nCount < nParAnz )
                {
                    SbiSymDef* pDef = pPool->Get( nCount );
                    USHORT nTyp = pDef->GetType();
                    // Zusätzliches Flag für BYVAL einbauen
                    if( pDef->IsByVal() )
                        nTyp |= 0x8000;
                    pParser->aGen.Gen( _ARGTYP, nTyp );
                }
            }
        }
    }
}

void SbiExpression::Gen()
{
    // AB: 17.12.1995, Spezialbehandlung fuer WITH
    // Wenn pExpr == .-Ausdruck in With, zunaechst Gen fuer Basis-Objekt
    pExpr->Gen();
    if( bBased )
        pParser->aGen.Gen( _BASED, pParser->nBase ),
        pParser->aGen.Gen( _ARGV );
}

