/*************************************************************************
 *
 *  $RCSfile: step2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:11 $
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

#include <svtools/sbxdef.hxx>
#include <svtools/sbx.hxx>
#include "runtime.hxx"
#pragma hdrstop
#include "iosys.hxx"
#include "image.hxx"
#include "sbintern.hxx"
#include "sbunoobj.hxx"
#include "opcodes.hxx"

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/uno/Any.hxx>

using namespace com::sun::star::container;
using namespace com::sun::star::lang;


#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBRUNTIME, SBRUNTIME_CODE )


/*
// #72488 Spezielle SbxVariable, die beim get das Verhalten
// einer nicht initialisierten Variable simuliert. Wenn als
// Typ SbxOBJECT verlangt wird, geht das jedoch nicht.
class UnoClassSbxVariable : public SbxVariable
{
    SbxDataType eOrgType;
    BOOL bOverWritten;
    const SbiImage* mpImg;
    SbiRuntime* mpRuntime;

public:
    UnoClassSbxVariable( SbxDataType eType, const SbiImage* pImg_, SbiRuntime* pRuntime_ )
        : SbxVariable( SbxVARIANT ), mpImg( pImg_ ), mpRuntime( pRuntime_ )
    {
        eOrgType = eType;
        bOverWritten = FALSE;
    }

    virtual BOOL Get( SbxValues& ) const;
    virtual BOOL Put( const SbxValues& );
};
*/

BOOL UnoClassSbxVariable::Get( SbxValues& rRes ) const
{
    static SbxVariable* pDummy = new SbxVariable( SbxVARIANT );
    if( mbOverWritten || rRes.eType == SbxOBJECT || rRes.eType == SbxVARIANT )
    {
        return SbxVariable::Get( rRes );
    }
    if( mpImg->GetFlag( SBIMG_EXPLICIT ) )
    {
        mpRuntime->Error( SbERR_VAR_UNDEFINED );
        return FALSE;
    }
    return pDummy->Get( rRes );
}

BOOL UnoClassSbxVariable::Put( const SbxValues& rRes )
{
    // Sonst, falls keine Parameter sind, anderen Error Code verwenden
    if( !mbOverWritten )
    {
        if( mpImg->GetFlag( SBIMG_EXPLICIT ) )
        {
            mpRuntime->Error( SbERR_VAR_UNDEFINED );
            return FALSE;
        }
        mbOverWritten = TRUE;

        SetType( meOrgType );
        if( meOrgType != SbxVARIANT )
            SetFlag( SBX_FIXED );
    }
    return SbxVariable::Put( rRes );
}

TYPEINIT1(UnoClassSbxVariable,SbxVariable)


// Suchen eines Elements
// Die Bits im String-ID:
// 0x8000 - Argv ist belegt

SbxVariable* SbiRuntime::FindElement
    ( SbxObject* pObj, USHORT nOp1, USHORT nOp2, SbError nNotFound, BOOL bLocal )
{
    SbxVariable* pElem = NULL;
    if( !pObj )
    {
        Error( SbERR_NO_OBJECT );
        pElem = new SbxVariable;
    }
    else
    {
        BOOL bFatalError = FALSE;
        SbxDataType t = (SbxDataType) nOp2;
        String aName( pImg->GetString( nOp1 & 0x7FFF ) );
        if( bLocal )
            pElem = refLocals->Find( aName, SbxCLASS_DONTCARE );
        if( !pElem )
        {
            // Die RTL brauchen wir nicht mehr zu durchsuchen!
            BOOL bSave = rBasic.bNoRtl;
            rBasic.bNoRtl = TRUE;
            pElem = pObj->Find( aName, SbxCLASS_DONTCARE );
            rBasic.bNoRtl = bSave;

            // Ist es ein globaler Uno-Bezeichner?
            if( bLocal && !pElem )
            {
                // #72382 VORSICHT! Liefert jetzt wegen unbekannten
                // Modulen IMMER ein Ergebnis!
                SbxVariable* pUnoClass = findUnoClass( aName );
                pElem = new UnoClassSbxVariable( t, pImg, this );
                SbxValues aRes( SbxOBJECT );
                aRes.pObj = pUnoClass;
                pElem->SbxVariable::Put( aRes );
                //pElem->SbxVariable::PutObject( pUnoClass );

                // #62939 Wenn eine Uno-Klasse gefunden wurde, muss
                // das Wrapper-Objekt gehalten werden, da sonst auch
                // die Uno-Klasse, z.B. "stardiv" immer wieder neu
                // aus der Registry gelesen werden muss
                //if( pElem )
                //{
                    // #63774 Darf nicht mit gespeichert werden!!!
                    pElem->SetFlag( SBX_DONTSTORE );
                    pElem->SetFlag( SBX_NO_MODIFY);

                    // #72382 Lokal speichern, sonst werden alle implizit
                    // deklarierten Vars automatisch global !
                    pElem->SetName( aName );
                    refLocals->Put( pElem, refLocals->Count() );
                    // OLD: rBasic.Insert( pElem );
                //}
            }

            if( !pElem )
            {
                // Nicht da und nicht im Objekt?
                // Hat das Ding Parameter, nicht einrichten!
                if( nOp1 & 0x8000 )
                    bFatalError = TRUE;
                    // ALT: StarBASIC::FatalError( nNotFound );

                // Sonst, falls keine Parameter sind, anderen Error Code verwenden
                if( !bLocal || pImg->GetFlag( SBIMG_EXPLICIT ) )
                {
                    // #39108 Bei explizit und als ELEM immer ein Fatal Error
                    bFatalError = TRUE;

                    // Falls keine Parameter sind, anderen Error Code verwenden
                    if( !( nOp1 & 0x8000 ) && nNotFound == SbERR_PROC_UNDEFINED )
                        nNotFound = SbERR_VAR_UNDEFINED;
                }
                if( bFatalError )
                {
                    // #39108 Statt FatalError zu setzen, Dummy-Variable liefern
                    if( !xDummyVar.Is() )
                        xDummyVar = new SbxVariable( SbxVARIANT );
                    pElem = xDummyVar;

                    // Parameter von Hand loeschen
                    ClearArgvStack();

                    // Normalen Error setzen
                    Error( nNotFound );
                }
                else
                {
                    // Sonst Variable neu anlegen
                    pElem = new SbxVariable( t );
                    if( t != SbxVARIANT )
                        pElem->SetFlag( SBX_FIXED );
                    pElem->SetName( aName );
                    refLocals->Put( pElem, refLocals->Count() );
                }
            }
        }
        // #39108 Args koennen schon geloescht sein!
        if( !bFatalError )
            SetupArgs( pElem, nOp1 );
        // Ein bestimmter Call-Type wurde gewuenscht, daher muessen
        // wir hier den Typ setzen und das Ding anfassen, um den
        // korrekten Returnwert zu erhalten!
        if( pElem->IsA( TYPE(SbxMethod) ) )
        {
            // Soll der Typ konvertiert werden?
            SbxDataType t2 = pElem->GetType();
            BOOL bSet = FALSE;
            if( !( pElem->GetFlags() & SBX_FIXED ) )
            {
                if( t != SbxVARIANT && t != t2 &&
                    t >= SbxINTEGER && t <= SbxSTRING )
                    pElem->SetType( t ), bSet = TRUE;
            }
            // pElem auf eine Ref zuweisen, um ggf. eine Temp-Var zu loeschen
            SbxVariableRef refTemp = pElem;

            // Moegliche Reste vom letzten Aufruf der SbxMethod beseitigen
            // Vorher Schreiben freigeben, damit kein Error gesetzt wird.
            USHORT nFlags = pElem->GetFlags();
            pElem->SetFlag( SBX_READWRITE | SBX_NO_BROADCAST );
            pElem->SbxValue::Clear();
            pElem->SetFlags( nFlags );

            // Erst nach dem Setzen anfassen, da z.B. LEFT()
            // den Unterschied zwischen Left$() und Left() kennen muss

            // AB 12.8.96: Da in PopVar() die Parameter von Methoden weggehauen
            // werden, muessen wir hier explizit eine neue SbxMethod anlegen
            SbxVariable* pNew = new SbxMethod( *((SbxMethod*)pElem) ); // das ist der Call!
            //ALT: SbxVariable* pNew = new SbxVariable( *pElem ); // das ist der Call!

            pElem->SetParameters(0); // sonst bleibt Ref auf sich selbst
            pNew->SetFlag( SBX_READWRITE );

            // den Datentypen zuruecksetzen?
            if( bSet )
                pElem->SetType( t2 );
            pElem = pNew;
        }
        // Index-Access bei UnoObjekten beruecksichtigen
        /*
        else if( pElem->ISA(SbUnoProperty) )
        {
            // pElem auf eine Ref zuweisen, um ggf. eine Temp-Var zu loeschen
            SbxVariableRef refTemp = pElem;

            // Variable kopieren und dabei den Notify aufloesen
            SbxVariable* pNew = new SbxVariable( *((SbxVariable*)pElem) ); // das ist der Call!
            pElem->SetParameters( NULL ); // sonst bleibt Ref auf sich selbst
            pElem = pNew;
        }
        */
    }
    return CheckArray( pElem );
}

// Find-Funktion ueber Name fuer aktuellen Scope (z.B. Abfrage aus BASIC-IDE)
SbxBase* SbiRuntime::FindElementExtern( const String& rName )
{
    // Hinweis zu #35281#: Es darf nicht davon ausgegangen werden, dass
    // pMeth != null, da im RunInit noch keine gesetzt ist.

    SbxVariable* pElem = NULL;
    if( !pMod || !rName.Len() )
        return NULL;

    // Lokal suchen
    if( refLocals )
        pElem = refLocals->Find( rName, SbxCLASS_DONTCARE );

    // In Statics suchen
    if ( !pElem && pMeth )
    {
        // Bei Statics, Name der Methode davor setzen
        String aMethName = pMeth->GetName();
        aMethName += ':';
        aMethName += rName;
        pElem = pMod->Find(aMethName, SbxCLASS_DONTCARE);
    }

    // In Parameter-Liste suchen
    if( !pElem && pMeth )
    {
        SbxInfo* pInfo = pMeth->GetInfo();
        if( pInfo && refParams )
        {
            USHORT j = 1;
            const SbxParamInfo* pParam = pInfo->GetParam( j );
            while( pParam )
            {
                if( pParam->aName.EqualsIgnoreCaseAscii( rName ) )
                {
                    pElem = refParams->Get( j );
                    break;
                }
                pParam = pInfo->GetParam( ++j );
            }
        }
    }

    // Im Modul suchen
    if( !pElem )
    {
        // RTL nicht durchsuchen!
        BOOL bSave = rBasic.bNoRtl;
        rBasic.bNoRtl = TRUE;
        pElem = pMod->Find( rName, SbxCLASS_DONTCARE );
        rBasic.bNoRtl = bSave;
    }
    return pElem;
}


// Argumente eines Elements setzen
// Dabei auch die Argumente umsetzen, falls benannte Parameter
// verwendet wurden

void SbiRuntime::SetupArgs( SbxVariable* p, USHORT nOp1 )
{
    if( nOp1 & 0x8000 )
    {
        if( !refArgv )
            StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
        BOOL bHasNamed = FALSE;
        USHORT i;
        for( i = 1; i < refArgv->Count(); i++ )
        {
            if( refArgv->GetAlias( i ).Len() )
            {
                bHasNamed = TRUE; break;
            }
        }
        if( bHasNamed )
        {
            // Wir haben mindestens einen benannten Parameter!
            // Wir muessen also umsortieren
            // Gibt es Parameter-Infos?
            SbxInfo* pInfo = p->GetInfo();
            if( !pInfo )
                Error( SbERR_NO_NAMED_ARGS );
            else
            {
                USHORT nCurPar = 1;
                SbxArray* pArg = new SbxArray;
                for( i = 1; i < refArgv->Count(); i++ )
                {
                    SbxVariable* pVar = refArgv->Get( i );
                    const String& rName = refArgv->GetAlias( i );
                    if( rName.Len() )
                    {
                        // nCurPar wird auf den gefundenen Parameter gesetzt
                        USHORT j = 1;
                        const SbxParamInfo* pParam = pInfo->GetParam( j );
                        while( pParam )
                        {
                            if( pParam->aName.EqualsIgnoreCaseAscii( rName ) )
                            {
                                nCurPar = j;
                                break;
                            }
                            pParam = pInfo->GetParam( ++j );
                        }
                        if( !pParam )
                        {
                            Error( SbERR_NAMED_NOT_FOUND ); break;
                        }
                    }
                    pArg->Put( pVar, nCurPar++ );
                }
                refArgv = pArg;
            }
        }
        // Eigene Var als Parameter 0
        refArgv->Put( p, 0 );
        p->SetParameters( refArgv );
        PopArgv();
    }
    else
        p->SetParameters( NULL );
}

// Holen eines Array-Elements

SbxVariable* SbiRuntime::CheckArray( SbxVariable* pElem )
{
    // Falls wir ein Array haben, wollen wir bitte das Array-Element!
    SbxArray* pPar;
    if( pElem->GetType() & SbxARRAY )
    {
        SbxBase* pElemObj = pElem->GetObject();
        SbxDimArray* pDimArray = PTR_CAST(SbxDimArray,pElemObj);
        pPar = pElem->GetParameters();
        if( pDimArray )
        {
            // Die Parameter koennen fehlen, wenn ein Array als
            // Argument uebergeben wird.
            if( pPar )
                pElem = pDimArray->Get( pPar );
        }
        else
        {
            SbxArray* pArray = PTR_CAST(SbxArray,pElemObj);
            if( pArray )
            {
                if( !pPar )
                {
                    Error( SbERR_OUT_OF_RANGE );
                    pElem = new SbxVariable;
                }
                else
                    pElem = pArray->Get( pPar->Get( 1 )->GetInteger() );
            }
        }

        // #42940, 0.Parameter zu NULL setzen, damit sich Var nicht selbst haelt
        if( pPar )
            pPar->Put( NULL, 0 );
    }
    // Index-Access bei UnoObjekten beruecksichtigen
    else if( pElem->GetType() == SbxOBJECT && !pElem->ISA(SbxMethod) && (pPar = pElem->GetParameters()) )
    {
        // Ist es ein Uno-Objekt?
        SbxBaseRef pObj = (SbxBase*)pElem->GetObject();
        if( pObj && pObj->ISA(SbUnoObject) )
        {
            SbUnoObject* pUnoObj = (SbUnoObject*)(SbxBase*)pObj;
            Any aAny = pUnoObj->getUnoAny();

            if( aAny.getValueType().getTypeClass() == TypeClass_INTERFACE )
            {
                Reference< XInterface > x = *(Reference< XInterface >*)aAny.getValue();
                Reference< XIndexAccess > xIndexAccess( x, UNO_QUERY );

                // Haben wir Index-Access?
                if( xIndexAccess.is() )
                {
                    UINT32 nParamCount = (UINT32)pPar->Count() - 1;
                    if( nParamCount != 1 )
                    {
                        StarBASIC::Error( SbERR_BAD_ARGUMENT );
                        return pElem;
                    }

                    // Index holen
                    INT32 nIndex = pPar->Get( 1 )->GetLong();
                    Reference< XInterface > xRet;
                    try
                    {
                        Any aAny = xIndexAccess->getByIndex( nIndex );
                        TypeClass eType = aAny.getValueType().getTypeClass();
                        if( eType == TypeClass_INTERFACE )
                            xRet = *(Reference< XInterface >*)aAny.getValue();
                    }
                    catch (IndexOutOfBoundsException& e1)
                    {
                        // Bei Exception erstmal immer von Konvertierungs-Problem ausgehen
                        StarBASIC::Error( SbERR_OUT_OF_RANGE );
                    }

                    // #57847 Immer neue Variable anlegen, sonst Fehler
                    // durch PutObject(NULL) bei ReadOnly-Properties.
                    pElem = new SbxVariable( SbxVARIANT );
                    if( xRet.is() )
                    {
                        aAny <<= xRet;

                        // #67173 Kein Namen angeben, damit echter Klassen-Namen eintragen wird
                        String aName;
                        SbxObjectRef xWrapper = (SbxObject*)new SbUnoObject( aName, aAny );
                        pElem->PutObject( xWrapper );
                    }
                    else
                    {
                        pElem->PutObject( NULL );
                    }
                }
            }
        }

        // #42940, 0.Parameter zu NULL setzen, damit sich Var nicht selbst haelt
        if( pPar )
            pPar->Put( NULL, 0 );
    }

    return pElem;
}

// Laden eines Elements aus der Runtime-Library (+StringID+Typ)

void SbiRuntime::StepRTL( USHORT nOp1, USHORT nOp2 )
{
    PushVar( FindElement( rBasic.pRtl, nOp1, nOp2, SbERR_PROC_UNDEFINED, FALSE ) );
}

// Laden einer lokalen/globalen Variablen (+StringID+Typ)

void SbiRuntime::StepFIND( USHORT nOp1, USHORT nOp2 )
{
    if( !refLocals )
        refLocals = new SbxArray;
    PushVar( FindElement( pMod, nOp1, nOp2, SbERR_PROC_UNDEFINED, TRUE ) );
}

// Laden eines Objekt-Elements (+StringID+Typ)
// Das Objekt liegt auf TOS

void SbiRuntime::StepELEM( USHORT nOp1, USHORT nOp2 )
{
    // Liegt auf dem TOS ein Objekt?
    SbxVariableRef pObjVar = PopVar();

    SbxObject* pObj = PTR_CAST(SbxObject,(SbxVariable*) pObjVar);
    if( !pObj )
    {
        SbxBase* pObjVarObj = pObjVar->GetObject();
        pObj = PTR_CAST(SbxObject,pObjVarObj);
    }

    // #56368 Bei StepElem Referenz sichern, sonst koennen Objekte
    // in Qualifizierungsketten wie ActiveComponent.Selection(0).Text
    // zu fueh die Referenz verlieren
    // #74254 Jetzt per Liste
    if( pObj )
        SaveRef( (SbxVariable*)pObj );

    PushVar( FindElement( pObj, nOp1, nOp2, SbERR_NO_METHOD, FALSE ) );
}

// Laden eines Parameters (+Offset+Typ)
// Wenn der Datentyp nicht stimmen sollte, eine Kopie anlegen
// Der Datentyp SbxEMPTY zeigt an, daá kein Parameter angegeben ist.
// Get( 0 ) darf EMPTY sein

void SbiRuntime::StepPARAM( USHORT nOp1, USHORT nOp2 )
{
    USHORT i = nOp1 & 0x7FFF;
    SbxDataType t = (SbxDataType) nOp2;
    SbxVariable* p;

    // #57915 Missing sauberer loesen
    BOOL bIsMissing = FALSE;
    USHORT nParamCount = refParams->Count();
    // Wurden ueberhaupt genug Parameter uebergeben
    if( i >= nParamCount )
    {
        p = new SbxVariable();
        p->PutErr( 448 );       // Wie in VB: Error-Code 448 (SbERR_NAMED_NOT_FOUND)
        refParams->Put( p, i );
    }
    else
    {
        p = refParams->Get( i );
    }
    if( p->GetType() == SbxERROR && ( i ) )
    //if( p->GetType() == SbxEMPTY && ( i ) )
    {
        // Wenn ein Parameter fehlt, kann er OPTIONAL sein
        BOOL bOpt = FALSE;
        SbxInfo* pInfo;
        if( pMeth && ( pInfo = pMeth->GetInfo() ) )
        {
            const SbxParamInfo* pParam = pInfo->GetParam( i );
            if( pParam && ( (pParam->nFlags & SBX_OPTIONAL) != 0 ) )
                bOpt = TRUE;
        }
        if( bOpt == FALSE )
            Error( SbERR_NOT_OPTIONAL );
    }
    else if( t != SbxVARIANT && (SbxDataType)(p->GetType() & 0x0FFF ) != t )
    {
        SbxVariable* q = new SbxVariable( t );
        SaveRef( q );
        *q = *p;
        p = q;
    }
    SetupArgs( p, nOp1 );
    PushVar( CheckArray( p ) );
}

// Case-Test (+True-Target+Test-Opcode)

void SbiRuntime::StepCASEIS( USHORT nOp1, USHORT nOp2 )
{
    if( !refCaseStk || !refCaseStk->Count() )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    else
    {
        SbxVariableRef xComp = PopVar();
        SbxVariableRef xCase = refCaseStk->Get( refCaseStk->Count() - 1 );
        if( xCase->Compare( (SbxOperator) nOp2, *xComp ) )
            StepJUMP( nOp1 );
    }
}

// Aufruf einer DLL-Prozedur (+StringID+Typ)
// Auch hier zeigt das MSB des StringIDs an, dass Argv belegt ist

void SbiRuntime::StepCALL( USHORT nOp1, USHORT nOp2 )
{
    String aName = pImg->GetString( nOp1 & 0x7FFF );
    SbxArray* pArgs = NULL;
    if( nOp1 & 0x8000 )
        pArgs = refArgv;
    DllCall( aName, aLibName, pArgs, (SbxDataType) nOp2, FALSE );
    aLibName = String();
    if( nOp1 & 0x8000 )
        PopArgv();
}

// Aufruf einer DLL-Prozedur nach CDecl (+StringID+Typ)
// Auch hier zeigt das MSB des StringIDs an, dass Argv belegt ist

void SbiRuntime::StepCALLC( USHORT nOp1, USHORT nOp2 )
{
    String aName = pImg->GetString( nOp1 & 0x7FFF );
    SbxArray* pArgs = NULL;
    if( nOp1 & 0x8000 )
        pArgs = refArgv;
    DllCall( aName, aLibName, pArgs, (SbxDataType) nOp2, TRUE );
    aLibName = String();
    if( nOp1 & 0x8000 )
        PopArgv();
}


// Beginn eines Statements (+Line+Col)

void SbiRuntime::StepSTMNT( USHORT nOp1, USHORT nOp2 )
{
    // Wenn der Expr-Stack am Anfang einen Statements eine Variable enthaelt,
    // hat ein Trottel X als Funktion aufgerufen, obwohl es eine Variable ist!
    BOOL bFatalExpr = FALSE;
    if( nExprLvl > 1 )
        bFatalExpr = TRUE;
    else if( nExprLvl )
    {
        SbxVariable* p = refExprStk->Get( 0 );
        if( p->GetRefCount() > 1
         && refLocals.Is() && refLocals->Find( p->GetName(), p->GetClass() ) )
            bFatalExpr = TRUE;
    }
    // Der Expr-Stack ist nun nicht mehr notwendig
    ClearExprStack();

    // #56368 Kuenstliche Referenz fuer StepElem wieder freigeben,
    // damit sie nicht ueber ein Statement hinaus erhalten bleibt
    //refSaveObj = NULL;
    // #74254 Jetzt per Liste
    ClearRefs();

    // Wir muessen hier hart abbrechen, da sonst Zeile und Spalte nicht mehr
    // stimmen!
    if( bFatalExpr)
    {
        StarBASIC::FatalError( SbERR_NO_METHOD );
        return;
    }
    pStmnt = pCode - 5;
    USHORT nOld = nLine;
    nLine = nOp1;

    // #29955 & 0xFF, um for-Schleifen-Ebene wegzufiltern
    nCol1 = nOp2 & 0xFF;

    // Suchen des naechsten STMNT-Befehls,
    // um die End-Spalte dieses Statements zu setzen
    nCol2 = -1;
    USHORT n1, n2;
    const BYTE* p = pMod->FindNextStmnt( pCode, n1, n2 );
    if( p )
    {
        if( n1 == nOp1 )
        {
            // #29955 & 0xFF, um for-Schleifen-Ebene wegzufiltern
            nCol2 = (n2 & 0xFF) - 1;
        }
    }

    // #29955 for-Schleifen-Ebene korrigieren, #67452 NICHT im Error-Handler sonst Chaos
    if( !bInError )
    {
        // (Bei Sprüngen aus Schleifen tritt hier eine Differenz auf)
        USHORT nExspectedForLevel = nOp2 / 0x100;
        USHORT nRealForLevel = 0;
        SbiForStack* pFor = pForStk;
        while( pFor )
        {
            nRealForLevel++;
            pFor = pFor->pNext;
        }

        // Wenn der tatsaechliche For-Level zu klein ist, wurde aus
        // einer Schleife heraus gesprungen -> korrigieren
        while( nRealForLevel > nExspectedForLevel )
        {
            PopFor();
            nRealForLevel--;
        }
    }

    // 16.10.96: #31460 Neues Konzept fuer StepInto/Over/Out
    // Erklärung siehe bei _ImplGetBreakCallLevel.
    if( pInst->nCallLvl <= pInst->nBreakCallLvl )
    //if( nFlags & SbDEBUG_STEPINTO )
    {
        StarBASIC* pStepBasic = GetCurrentBasic( &rBasic );
        USHORT nNewFlags = pStepBasic->StepPoint( nLine, nCol1, nCol2 );

        // Neuen BreakCallLevel ermitteln
        pInst->CalcBreakCallLevel( nNewFlags );
    }

    // Breakpoints nur bei STMNT-Befehlen in neuer Zeile!
    else if( ( nOp1 != nOld )
        && ( nFlags & SbDEBUG_BREAK )
        && pMod->IsBP( nOp1 ) )
    {
        StarBASIC* pBreakBasic = GetCurrentBasic( &rBasic );
        USHORT nNewFlags = pBreakBasic->BreakPoint( nLine, nCol1, nCol2 );

        // Neuen BreakCallLevel ermitteln
        pInst->CalcBreakCallLevel( nNewFlags );
        //16.10.96, ALT:
        //if( nNewFlags != SbDEBUG_CONTINUE )
        //  nFlags = nNewFlags;
    }
}

// (+SvStreamFlags+Flags)
// Stack: Blocklaenge
//        Kanalnummer
//        Dateiname

void SbiRuntime::StepOPEN( USHORT nOp1, USHORT nOp2 )
{
    SbxVariableRef pName = PopVar();
    SbxVariableRef pChan = PopVar();
    SbxVariableRef pLen  = PopVar();
    short nBlkLen = pLen->GetInteger();
    short nChan   = pChan->GetInteger();
    ByteString aName( pName->GetString(), gsl_getSystemTextEncoding() );
    pIosys->Open( nChan, aName, nOp1, nOp2, nBlkLen );
    Error( pIosys->GetError() );
}

// Objekt kreieren (+StringID+StringID)

void SbiRuntime::StepCREATE( USHORT nOp1, USHORT nOp2 )
{
    String aClass( pImg->GetString( nOp2 ) );
    SbxObject *pObj = SbxBase::CreateObject( aClass );
    if( !pObj )
        Error( SbERR_INVALID_OBJECT );
    else
    {
        String aName( pImg->GetString( nOp1 ) );
        pObj->SetName( aName );
    // Das Objekt muss BASIC rufen koennen
        pObj->SetParent( &rBasic );
        SbxVariable* pNew = new SbxVariable;
        pNew->PutObject( pObj );
        PushVar( pNew );
    }
}

// #56204 Objekt-Array kreieren (+StringID+StringID), DCREATE == Dim-Create
void SbiRuntime::StepDCREATE( USHORT nOp1, USHORT nOp2 )
{
    SbxVariableRef refVar = PopVar();
    DimImpl( refVar );

    // Das Array mit Instanzen der geforderten Klasse fuellen
    SbxBaseRef xObj = (SbxBase*)refVar->GetObject();
    if( !xObj )
    {
        StarBASIC::Error( SbERR_INVALID_OBJECT );
        return;
    }

    if( xObj->ISA(SbxDimArray) )
    {
        SbxBase* pObj = (SbxBase*)xObj;
        SbxDimArray* pArray = (SbxDimArray*)pObj;

        // Dimensionen auswerten
        short nDims = pArray->GetDims();
        USHORT nTotalSize = 0;

        // es muss ein eindimensionales Array sein
        short nLower, nUpper, nSize;
        USHORT i;
        for( i = 0 ; i < nDims ; i++ )
        {
            pArray->GetDim( i+1, nLower, nUpper );
            nSize = nUpper - nLower + 1;
            if( i == 0 )
                nTotalSize = nSize;
            else
                nTotalSize *= nSize;
        }

        // Objekte anlegen und ins Array eintragen
        String aClass( pImg->GetString( nOp2 ) );
        for( i = 0 ; i < nTotalSize ; i++ )
        {
            SbxObject *pObj = SbxBase::CreateObject( aClass );
            if( !pObj )
            {
                Error( SbERR_INVALID_OBJECT );
                break;
            }
            else
            {
                String aName( pImg->GetString( nOp1 ) );
                pObj->SetName( aName );
                // Das Objekt muss BASIC rufen koennen
                pObj->SetParent( &rBasic );
                pArray->SbxArray::Put( pObj, i );
            }
        }
    }
}

// Objekt aus User-Type kreieren  (+StringID+StringID)
void SbiRuntime::StepTCREATE( USHORT nOp1, USHORT nOp2 )
{
    String aName( pImg->GetString( nOp1 ) );
    String aClass( pImg->GetString( nOp2 ) );
    const SbxObject* pObj = pImg->FindType(aClass);
    if (pObj)
    {
        SbxObject *pCopyObj = new SbxObject(*pObj);
        pCopyObj->SetName(pImg->GetString( nOp1 ));
        SbxVariable* pNew = new SbxVariable;
        pNew->PutObject( pCopyObj );
        PushVar( pNew );
    }
    else
        Error( SbERR_INVALID_OBJECT );
}



// Einrichten einer lokalen Variablen (+StringID+Typ)

void SbiRuntime::StepLOCAL( USHORT nOp1, USHORT nOp2 )
{
    if( !refLocals.Is() )
        refLocals = new SbxArray;
    String aName( pImg->GetString( nOp1 ) );
    SbxDataType t = (SbxDataType) nOp2;
    SbxVariable* p = new SbxVariable( t );
    p->SetName( aName );
    refLocals->Put( p, refLocals->Count() );
}

// Einrichten einer modulglobalen Variablen (+StringID+Typ)

void SbiRuntime::StepPUBLIC( USHORT nOp1, USHORT nOp2 )
{
    String aName( pImg->GetString( nOp1 ) );
    SbxDataType t = (SbxDataType) nOp2;
    BOOL bFlag = pMod->IsSet( SBX_NO_MODIFY );
    pMod->SetFlag( SBX_NO_MODIFY );
    SbProperty* pProp = pMod->GetProperty( aName, t );
    if( !bFlag )
        pMod->ResetFlag( SBX_NO_MODIFY );
    if( pProp )
    {
        pProp->SetFlag( SBX_DONTSTORE );
        // AB: 2.7.1996: HACK wegen 'Referenz kann nicht gesichert werden'
        pProp->SetFlag( SBX_NO_MODIFY);
    }

}

// Einrichten einer globalen Variablen (+StringID+Typ)

void SbiRuntime::StepGLOBAL( USHORT nOp1, USHORT nOp2 )
{
    String aName( pImg->GetString( nOp1 ) );
    SbxDataType t = (SbxDataType) nOp2;
    BOOL bFlag = rBasic.IsSet( SBX_NO_MODIFY );
    rBasic.SetFlag( SBX_NO_MODIFY );
    SbxVariableRef p = rBasic.Find( aName, SbxCLASS_PROPERTY );
    if( p.Is() )
        rBasic.Remove (p);
    p = rBasic.Make( aName, SbxCLASS_PROPERTY, t );
    if( !bFlag )
        rBasic.ResetFlag( SBX_NO_MODIFY );
    if( p )
    {
        p->SetFlag( SBX_DONTSTORE );
        // AB: 2.7.1996: HACK wegen 'Referenz kann nicht gesichert werden'
        p->SetFlag( SBX_NO_MODIFY);
    }

}

// Einrichten einer statischen Variablen (+StringID+Typ)

void SbiRuntime::StepSTATIC( USHORT nOp1, USHORT nOp2 )
{
    /* AB #40689, wird nicht mehr verwendet
    String aName( pImg->GetString( nOp1 ) );
    SbxDataType t = (SbxDataType) nOp2;
    SbxVariable* p = new SbxVariable( t );
    p->SetName( aName );
    pInst -> GetStatics()->Put( p, pInst->GetStatics()->Count() );
    */
}


