/*************************************************************************
 *
 *  $RCSfile: dim.cxx,v $
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

#include <svtools/sbx.hxx>
#include "sbcomp.hxx"
#pragma hdrstop

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBCOMP, SBCOMP_CODE )

// Deklaration einer Variablen
// Bei Fehlern wird bis zum Komma oder Newline geparst.
// Returnwert: eine neue Instanz, die eingefuegt und dann geloescht wird.
// Array-Indexe werden als SbiDimList zurueckgegeben

SbiSymDef* SbiParser::VarDecl( SbiDimList** ppDim, BOOL bStatic, BOOL bConst )
{
    if( !TestSymbol() ) return NULL;
    SbxDataType t = eScanType;
    SbiSymDef* pDef = bConst ? new SbiConstDef( aSym ) : new SbiSymDef( aSym );
    SbiDimList* pDim = NULL;
    // Klammern?
    if( Peek() == LPAREN )
        pDim = new SbiDimList( this );
    pDef->SetType( t );
    if( bStatic )
        pDef->SetStatic();
    TypeDecl( *pDef );
    if( !ppDim && pDim )
    {
        if(pDim->GetDims() )
            Error( SbERR_EXPECTED, "()" );
        delete pDim;
    }
    else if( ppDim )
        *ppDim = pDim;
    return pDef;
}

// Aufloesen einer AS-Typdeklaration
// Der Datentyp wird in die uebergebene Variable eingetragen

void SbiParser::TypeDecl( SbiSymDef& rDef )
{
    SbxDataType eType = rDef.GetType();
    short nSize = 0;
    if( Peek() == AS )
    {
#ifdef VBSCRIPT_TEST
        Error( ERRCODE_BASIC_NOT_IN_VBSCRIPT );
#endif
        Next();
        rDef.SetDefinedAs();
        String aType;
        SbiToken eTok = Next();
        if( eTok == NEW )
        {
            rDef.SetNew();
            eTok = Next();
        }
        switch( eTok )
        {
            case ANY:
                if( rDef.IsNew() )
                    Error( SbERR_SYNTAX );
                eType = SbxVARIANT; break;
            case TINTEGER:
            case TLONG:
            case TSINGLE:
            case TDOUBLE:
            case TCURRENCY:
            case TDATE:
            case TSTRING:
            case TOBJECT:
            case _ERROR_:
            case TBOOLEAN:
            case TVARIANT:
                if( rDef.IsNew() )
                    Error( SbERR_SYNTAX );
                eType = SbxDataType( eTok - TINTEGER + SbxINTEGER );
                if( eType == SbxSTRING )
                {
                    // STRING*n ?
                    if( Peek() == MUL )
                    {       // fixed size!
                        Next();
                        SbiConstExpression aSize( this );
                        nSize = aSize.GetShortValue();
                        if( nSize < 0 )
                            Error( SbERR_OUT_OF_RANGE );
                    }
                }
                break;
            case SYMBOL: // kann nur ein TYPE oder eine Objektklasse sein!
                if( eScanType != SbxVARIANT )
                    Error( SbERR_SYNTAX );
                else
                {
                    String aCompleteName = aSym;

                    // #52709 DIM AS NEW fuer Uno mit voll-qualifizierten Namen
                    if( Peek() == DOT )
                    {
                        String aDotStr( '.' );
                        while( Peek() == DOT )
                        {
                            aCompleteName += aDotStr;
                            Next();
                            if( Peek() == SYMBOL )
                            {
                                Next();
                                aCompleteName += aSym;
                            }
                            else
                            {
                                Next();
                                Error( SbERR_UNEXPECTED, SYMBOL );
                                break;
                            }
                        }
                    }

                    // In den String-Pool uebernehmen
                    rDef.SetTypeId( aGblStrings.Add( aCompleteName ) );
                }
                eType = SbxOBJECT;
                break;
            default:
                Error( SbERR_UNEXPECTED, eTok );
                Next();
        }
        // Die Variable koennte mit Suffix deklariert sein
        if( rDef.GetType() != SbxVARIANT )
        {
            if( rDef.GetType() != eType )
                Error( SbERR_VAR_DEFINED, rDef.GetName() );
            else if( eType == SbxSTRING && rDef.GetLen() != nSize )
                Error( SbERR_VAR_DEFINED, rDef.GetName() );
        }
        rDef.SetType( eType );
        rDef.SetLen( nSize );
    }
}

// Hier werden Variable, Arrays und Strukturen definiert.
// DIM/PRIVATE/PUBLIC/GLOBAL

void SbiParser::Dim()
{
    DefVar( _DIM, FALSE );
}

void SbiParser::DefVar( SbiOpcode eOp, BOOL bStatic )
{
    SbiSymPool* pOldPool = pPool;
    BOOL bSwitchPool = FALSE;
    if( pProc && ( eCurTok == GLOBAL || eCurTok == PUBLIC || eCurTok == PRIVATE ) )
        Error( SbERR_NOT_IN_SUBR, eCurTok );
    if( eCurTok == PUBLIC || eCurTok == GLOBAL )
        bSwitchPool = TRUE;     // im richtigen Moment auf globalen Pool schalten
        //pPool = &aGlobals;
    // PRIVATE ist Synonym fuer DIM
    // _CONST_?
    BOOL bConst = FALSE;
    if( eCurTok == _CONST_ )
        bConst = TRUE;
    else if( Peek() == _CONST_ )
        Next(), bConst = TRUE;
#ifdef SHARED
#define tmpSHARED
#undef SHARED
#endif
    // SHARED wird ignoriert
    if( Peek() == SHARED ) Next();
#ifdef tmpSHARED
#define SHARED
#undef tmpSHARED
#endif
    // PRESERVE nur bei REDIM
    if( Peek() == PRESERVE )
    {
        Next();
        if( eOp == _REDIM )
            eOp = _REDIMP;
        else
            Error( SbERR_UNEXPECTED, eCurTok );
    }
    SbiSymDef* pDef;
    SbiDimList* pDim;

    // AB 9.7.97, #40689, Statics -> Modul-Initialisierung, in Sub ueberspringen
    USHORT nEndOfStaticLbl;
    if( bStatic )
    {
        nEndOfStaticLbl = aGen.Gen( _JUMP, 0 );
        aGen.Statement();   // bei static hier nachholen
    }

    BOOL bDefined = FALSE;
    while( ( pDef = VarDecl( &pDim, bStatic, bConst ) ) != NULL )
    {
        EnableErrors();
        // Variable suchen:
        if( bSwitchPool )
            pPool = &aGlobals;
        SbiSymDef* pOld = pPool->Find( pDef->GetName() );
        // AB 31.3.1996, #25651#, auch in Runtime-Library suchen
        BOOL bRtlSym = FALSE;
        if( !pOld )
        {
            pOld = CheckRTLForSym( pDef->GetName(), SbxVARIANT );
            if( pOld )
                bRtlSym = TRUE;
        }
        if( pOld )
        {
            bDefined = TRUE;
            // Bei RTL-Symbol immer Fehler
            if( !bRtlSym && (eOp == _REDIM || eOp == _REDIMP) )
            {
                // Bei REDIM die Attribute vergleichen
                if( pOld->IsStatic() || pOld->GetType() != pDef->GetType() )
                    Error( SbERR_VAR_DEFINED, pDef->GetName() );
            }
            else
                Error( SbERR_VAR_DEFINED, pDef->GetName() );
            delete pDef; pDef = pOld;
        }
        else
            pPool->Add( pDef );

        // #36374: Variable vor Unterscheidung IsNew() anlegen
        // Sonst Error bei Dim Identifier As New Type und option explicit
        if( !bDefined && ( !bConst || pDef->GetScope() == SbGLOBAL ) )
        {
            // Variable oder globale Konstante deklarieren
            SbiOpcode eOp;
            switch ( pDef->GetScope() )
            {
                case SbGLOBAL:  eOp = _GLOBAL; goto global;
                case SbPUBLIC:  eOp = _PUBLIC;
                                // AB 9.7.97, #40689, kein eigener Opcode mehr
                                /*
                                if( bStatic )
                                {
                                    eOp = _STATIC;
                                    break;
                                }
                                */
                global:         aGen.BackChain( nGblChain );
                                nGblChain = 0;
                                bGblDefs = bNewGblDefs = TRUE;
                                break;
                default:        eOp = _LOCAL;
            }
            aGen.Gen( eOp, pDef->GetId(), pDef->GetType() );
        }

        // Initialisierung fuer selbstdefinierte Datentypen
        // und per NEW angelegte Variable
        if( pDef->GetType() == SbxOBJECT
         && pDef->GetTypeId()
         && pDef->IsNew() )
//      && ( pDef->IsNew() || pDef->HabIchAlsTypeDefiniert() ) )
        {
            if( bConst )
            {
                Error( SbERR_SYNTAX );
            }

            if( pDim )
            {
                pDef->SetDims( pDim->GetDims() );
                SbiExpression aExpr( this, *pDef, pDim );
                aExpr.Gen();
                aGen.Gen( _DCREATE, pDef->GetId(), pDef->GetTypeId() );
            }
            else
            {
                SbiExpression aExpr( this, *pDef );
                aExpr.Gen();
                SbiOpcode eOp = pDef->IsNew() ? _CREATE : _TCREATE;
                aGen.Gen( eOp, pDef->GetId(), pDef->GetTypeId() );
                aGen.Gen( _SET );
            }
        }
        else
        {
            if( bConst )
            {
                // Konstanten-Definition
                if( pDim )
                {
                    Error( SbERR_SYNTAX );
                    delete pDim;
                }
                SbiExpression aVar( this, *pDef );
                if( !TestToken( EQ ) )
                    goto MyBreak;   // AB 24.6.1996 (s.u.)
                SbiConstExpression aExpr( this );
                if( !bDefined && aExpr.IsValid() )
                {
                    if( pDef->GetScope() == SbGLOBAL )
                    {
                        // Nur Code fuer globale Konstante erzeugen!
                        aVar.Gen();
                        aExpr.Gen();
                        aGen.Gen( _PUTC );
                    }
                    SbiConstDef* pConst = pDef->GetConstDef();
                    if( aExpr.GetType() == SbxSTRING )
                        pConst->Set( aExpr.GetString() );
                    else
                        pConst->Set( aExpr.GetValue(), aExpr.GetType() );
                }
            }
            else if( pDim )
            {
                // Die Variable dimensionieren
                // Bei REDIM die Var vorher loeschen
                if( eOp == _REDIM )
                {
                    SbiExpression aExpr( this, *pDef, NULL );
                    aExpr.Gen();
                    aGen.Gen( _ERASE );
                }
                pDef->SetDims( pDim->GetDims() );
                SbiExpression aExpr( this, *pDef, pDim );
                aExpr.Gen();
                aGen.Gen( (eOp == _STATIC) ? _DIM : eOp );
            }
        }
        if( !TestComma() )
            goto MyBreak;   // AB 24.6.1996 (s.u.)

        // #27963# AB, 24.6.1996
        // Einfuehrung bSwitchPool (s.o.): pPool darf beim VarDecl-Aufruf
        // noch nicht auf &aGlobals gesetzt sein.
        // Ansonsten soll das Verhalten aber absolut identisch bleiben,
        // d.h. pPool muss immer am Schleifen-Ende zurueckgesetzt werden.
        // auch bei break
        pPool = pOldPool;
        continue;       // MyBreak überspingen
    MyBreak:
        pPool = pOldPool;
        break;
    }

    // AB 9.7.97, #40689, Sprung ueber Statics-Deklaration abschliessen
    if( bStatic )
    {
        // globalen Chain pflegen
        nGblChain = aGen.Gen( _JUMP, 0 );
        bGblDefs = bNewGblDefs = TRUE;

        // fuer Sub Sprung auf Ende der statics eintragen
        aGen.BackChain( nEndOfStaticLbl );
    }

    //pPool = pOldPool;
}

// Hier werden Arrays redimensioniert.

void SbiParser::ReDim()
{
    DefVar( _REDIM, FALSE );
}

// ERASE array, ...

void SbiParser::Erase()
{
    while( !bAbort )
    {
        if( !TestSymbol() ) return;
        String aName( aSym );
        SbxDataType eType = eScanType;
        SbiSymDef* pDef = pPool->Find( aName );
        if( !pDef )
        {
            if( bExplicit )
                Error( SbERR_UNDEF_VAR, aName );
            pDef = pPool->AddSym( aName );
            pDef->SetType( eType );
        }
        SbiExpression aExpr( this, *pDef );
        aExpr.Gen();
        aGen.Gen( _ERASE );
        if( !TestComma() ) break;
    }
}

// Deklaration eines Datentyps

void SbiParser::Type()
{
    // Neues Token lesen, es muss ein Symbol sein
    if (!TestSymbol())
        return;

    if (rTypeArray->Find(aSym,SbxCLASS_OBJECT))
    {
        Error( SbERR_VAR_DEFINED, aSym );
        return;
    }

    SbxObject *pType = new SbxObject(aSym);

    SbiSymDef* pElem;
    SbiDimList* pDim;
    BOOL bDone = FALSE;

    while( !bDone && !IsEof() )
    {
        switch (Next())
        {
            case ENDTYPE :
                pElem = NULL;
                bDone = TRUE;
            break;

            case EOLN :
                pElem = NULL;
            break;

            default:
                pDim = NULL;
                pElem = VarDecl(&pDim,FALSE,FALSE);
                if( pDim )
                {
                    // HOT FIX, to be updated
                    delete pDim;
                    Error( SbERR_NO_STRINGS_ARRAYS );
                }

        }
        if( pElem )
        {
            SbxArray *pTypeMembers = pType -> GetProperties();
            if (pTypeMembers -> Find (pElem->GetName(),SbxCLASS_DONTCARE))
                Error (SbERR_VAR_DEFINED);
            else
            {
                SbxProperty *pTypeElem = new SbxProperty (pElem->GetName(),pElem->GetType());
                pTypeMembers -> Insert (pTypeElem,pTypeMembers->Count());
            }
            delete pElem;
        }
    }
    rTypeArray->Insert (pType,rTypeArray->Count());
}
/**********************************************************
    // Variablennamen einlesen:
    if( !TestSymbol() ) return;
    String aName( aSym );
    if( pDEFS->Find( aName ) )
    {
        Error( SbERR_VAR_DEFINED, aName ); return;
    }
    SbTypeDef* pDef = pDEFS->AddTypeDef( aName );
    TestEoln();

    // Deklarationen parsen:
    SbiSymDef* pElem;
    SbiDimList* pDim;
    BOOL bDone = FALSE;
    while( !IsEof() && !bDone )
    {
        switch( Next() )
        {
            case SUB:
            case FUNCTION:
                pElem = ProcDecl( TRUE ); break;
            case END:
                pElem = NULL;
                bDone = TRUE;
                if( Next() != TYPE )
                    Error( SbERR_EXPECTED, TYPE );
                break;
            default:
                pElem = VarDecl( &pDim );
                if( pDim )
                {
                    // HOT FIX, to be updated
                    delete pDim;
                    Error( SbERR_NO_STRINGS_ARRAYS );
                }
        }
        if( pElem )
            pDef->GetPool().Add( *pElem );
        delete pElem;
    }
}
************************************************/

// Prozedur-Deklaration
// das erste Token ist bereits eingelesen (SUB/FUNCTION)
// xxx Name [LIB "name"[ALIAS "name"]][(Parameter)][AS TYPE]

SbiProcDef* SbiParser::ProcDecl( BOOL bDecl )
{
    BOOL bFunc = BOOL( eCurTok == FUNCTION );
    if( !TestSymbol() ) return NULL;
    String aName( aSym );
    SbxDataType eType = eScanType;
    SbiProcDef* pDef = new SbiProcDef( this, aName );
    pDef->SetType( eType );
    if( Peek() == _CDECL_ )
    {
        Next(); pDef->SetCdecl();
    }
    if( Peek() == LIB )
    {
        Next();
        if( Next() == FIXSTRING )
            pDef->GetLib() = aSym;
        else
            Error( SbERR_SYNTAX );
    }
    if( Peek() == ALIAS )
    {
        Next();
        if( Next() == FIXSTRING )
            pDef->GetAlias() = aSym;
        else
            Error( SbERR_SYNTAX );
    }
    if( !bDecl )
    {
        // CDECL, LIB und ALIAS sind unzulaessig
        if( pDef->GetLib().Len() )
            Error( SbERR_UNEXPECTED, LIB );
        if( pDef->GetAlias().Len() )
            Error( SbERR_UNEXPECTED, ALIAS );
        if( pDef->IsCdecl() )
            Error( SbERR_UNEXPECTED, _CDECL_ );
        pDef->SetCdecl( FALSE );
        pDef->GetLib().Erase();
        pDef->GetAlias().Erase();
    }
    else if( !pDef->GetLib().Len() )
    {
        // ALIAS und CDECL nur zusammen mit LIB
        if( pDef->GetAlias().Len() )
            Error( SbERR_UNEXPECTED, ALIAS );
        if( pDef->IsCdecl() )
            Error( SbERR_UNEXPECTED, _CDECL_ );
        pDef->SetCdecl( FALSE );
        pDef->GetAlias().Erase();
    }
    // Klammern?
    if( Peek() == LPAREN )
    {
        Next();
        if( Peek() == RPAREN )
            Next();
        else
          for(;;) {
            BOOL bByVal = FALSE;
            BOOL bOptional = FALSE;
            while( Peek() == BYVAL || Peek() == _OPTIONAL_ )
            {
                if      ( Peek() == BYVAL )     Next(), bByVal = TRUE;
                else if ( Peek() == _OPTIONAL_ )    Next(), bOptional = TRUE;
            }
            SbiSymDef* pPar = VarDecl( NULL, FALSE, FALSE );
            if( !pPar )
                break;
            if( bByVal )
                pPar->SetByVal();
            if( bOptional )
                pPar->SetOptional();
            pDef->GetParams().Add( pPar );
            SbiToken eTok = Next();
            if( eTok != COMMA && eTok != RPAREN )
            {
                Error( SbERR_EXPECTED, RPAREN );
                break;
            }
            if( eTok == RPAREN )
                break;
        }
    }
    TypeDecl( *pDef );
    if( eType != SbxVARIANT && pDef->GetType() != eType )
        Error( SbERR_BAD_DECLARATION, aName );
//  if( pDef->GetType() == SbxOBJECT )
//      pDef->SetType( SbxVARIANT ),
//      Error( SbERR_SYNTAX );
    if( pDef->GetType() == SbxVARIANT && !bFunc )
        pDef->SetType( SbxEMPTY );
    return pDef;
}

// DECLARE

void SbiParser::Declare()
{
    Next();
    if( eCurTok != SUB && eCurTok != FUNCTION )
      Error( SbERR_UNEXPECTED, eCurTok );
    else
    {
        SbiProcDef* pDef = ProcDecl( TRUE );
        if( pDef )
        {
            if( !pDef->GetLib().Len() )
                Error( SbERR_EXPECTED, LIB );
            // gibts den schon?
            SbiSymDef* pOld = aPublics.Find( pDef->GetName() );
            if( pOld )
            {
                SbiProcDef* p = pOld->GetProcDef();
                if( !p )
                {
                    // Als Variable deklariert
                    Error( SbERR_BAD_DECLARATION, pDef->GetName() );
                    delete pDef;
                }
                else
                    pDef->Match( p );
            }
            else
                aPublics.Add( pDef );
        }
    }
}

// Aufruf einer SUB oder FUNCTION

void SbiParser::Call()
{
    String aName( aSym );
    SbiExpression aVar( this, SbSYMBOL );
    aVar.Gen();
    aGen.Gen( _GET );
}

// SUB/FUNCTION

void SbiParser::SubFunc()
{
    DefProc( FALSE );
}

// Einlesen einer Prozedur

void SbiParser::DefProc( BOOL bStatic )
{
    USHORT l1 = nLine, l2 = nLine;
    BOOL bSub = BOOL( eCurTok == SUB );
    SbiToken eExit = eCurTok;
    SbiProcDef* pDef = ProcDecl( FALSE );
    if( !pDef )
        return;

    // Ist die Proc bereits deklariert?
    SbiSymDef* pOld = aPublics.Find( pDef->GetName() );
    if( pOld )
    {
        pProc = pOld->GetProcDef();
        if( !pProc )
        {
            // Als Variable deklariert
            Error( SbERR_BAD_DECLARATION, pDef->GetName() );
            delete pDef;
            pProc = NULL;
        }
        else
        {
            pDef->Match( pProc );
            pProc = pDef;
        }
    }
    else
        aPublics.Add( pDef ), pProc = pDef;

    if( !pProc )
        return;

    // Nun setzen wir die Suchhierarchie fuer Symbole sowie die aktuelle
    // Prozedur.
    aPublics.SetProcId( pProc->GetId() );
    pProc->GetParams().SetParent( &aPublics );
    if( !bStatic )
    {
        // Normalfall: Lokale Variable->Parameter->Globale Variable
        pProc->GetLocals().SetParent( &pProc->GetParams() );
        pPool = &pProc->GetLocals();
    }
    else
    {
        Error( SbERR_NOT_IMPLEMENTED ); // STATIC SUB ...
    }

    pProc->Define();
    OpenBlock( eExit );
    StmntBlock( bSub ? ENDSUB : ENDFUNC );
    l2 = nLine;
    pProc->SetLine1( l1 );
    pProc->SetLine2( l2 );
    pPool = &aPublics;
    aPublics.SetProcId( 0 );
    // Offene Labels?
    pProc->GetLabels().CheckRefs();
    CloseBlock();
    aGen.Gen( _LEAVE );
    pProc = NULL;
}

// STATIC variable|procedure

void SbiParser::Static()
{
    switch( Peek() )
    {
        case SUB:
        case FUNCTION:
            Next();
            DefProc( TRUE );
            break;
        default: {
            if( !pProc )
                Error( SbERR_NOT_IN_SUBR );
            // Pool umsetzen, damit STATIC-Deklarationen im globalen
            // Pool landen
            SbiSymPool* p = pPool; pPool = &aPublics;
            DefVar( _STATIC, TRUE );
            pPool = p;
            } break;
    }
}

