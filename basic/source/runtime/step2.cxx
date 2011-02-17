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

#include "runtime.hxx"
#ifndef GCC
#endif
#include "iosys.hxx"
#include "image.hxx"
#include "sbintern.hxx"
#include "sbunoobj.hxx"
#include "opcodes.hxx"

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/script/XDefaultMethod.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <comphelper/processfactory.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::script;

using com::sun::star::uno::Reference;

SbxVariable* getVBAConstant( const String& rName );

// Suchen eines Elements
// Die Bits im String-ID:
// 0x8000 - Argv ist belegt

SbxVariable* SbiRuntime::FindElement
    ( SbxObject* pObj, UINT32 nOp1, UINT32 nOp2, SbError nNotFound, BOOL bLocal, BOOL bStatic )
{
    bool bIsVBAInterOp = SbiRuntime::isVBAEnabled();
    if( bIsVBAInterOp )
    {
        StarBASIC* pMSOMacroRuntimeLib = GetSbData()->pMSOMacroRuntimLib;
        if( pMSOMacroRuntimeLib != NULL )
            pMSOMacroRuntimeLib->ResetFlag( SBX_EXTSEARCH );
    }

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
        String aName( pImg->GetString( static_cast<short>( nOp1 & 0x7FFF ) ) );
        // Hacky capture of Evaluate [] syntax
        // this should be tackled I feel at the pcode level
        if ( bIsVBAInterOp && aName.Search('[') == 0 )
        {
            // emulate pcode here
            StepARGC();
            // psuedo StepLOADSC
            String sArg = aName.Copy( 1, aName.Len() - 2 );
            SbxVariable* p = new SbxVariable;
            p->PutString( sArg );
            PushVar( p );
            //
            StepARGV();
            nOp1 = nOp1 | 0x8000; // indicate params are present
            aName = String::CreateFromAscii("Evaluate");
        }
        if( bLocal )
        {
            if ( bStatic )
            {
                if ( pMeth )
                    pElem = pMeth->GetStatics()->Find( aName, SbxCLASS_DONTCARE );
            }

            if ( !pElem )
                pElem = refLocals->Find( aName, SbxCLASS_DONTCARE );
        }
        if( !pElem )
        {
            // Die RTL brauchen wir nicht mehr zu durchsuchen!
            BOOL bSave = rBasic.bNoRtl;
            rBasic.bNoRtl = TRUE;
            pElem = pObj->Find( aName, SbxCLASS_DONTCARE );

            // #110004, #112015: Make private really private
            if( bLocal && pElem )   // Local as flag for global search
            {
                if( pElem->IsSet( SBX_PRIVATE ) )
                {
                    SbiInstance* pInst_ = pINST;
                    if( pInst_ && pInst_->IsCompatibility() && pObj != pElem->GetParent() )
                        pElem = NULL;   // Found but in wrong module!

                    // Interfaces: Use SBX_EXTFOUND
                }
            }
            rBasic.bNoRtl = bSave;

            // Ist es ein globaler Uno-Bezeichner?
            if( bLocal && !pElem )
            {
                bool bSetName = true; // preserve normal behaviour

                // i#i68894# if VBAInterOp favour searching vba globals
                // over searching for uno classess
                if ( bVBAEnabled )
                {
                    // Try Find in VBA symbols space
                    pElem = rBasic.VBAFind( aName, SbxCLASS_DONTCARE );
                    if ( pElem )
                        bSetName = false; // don't overwrite uno name
                    else
                        pElem = getVBAConstant( aName );
                }

                if( !pElem )
                {
                    // #72382 VORSICHT! Liefert jetzt wegen unbekannten
                    // Modulen IMMER ein Ergebnis!
                    SbUnoClass* pUnoClass = findUnoClass( aName );
                    if( pUnoClass )
                    {
                        pElem = new SbxVariable( t );
                        SbxValues aRes( SbxOBJECT );
                        aRes.pObj = pUnoClass;
                        pElem->SbxVariable::Put( aRes );
                    }
                }

                // #62939 Wenn eine Uno-Klasse gefunden wurde, muss
                // das Wrapper-Objekt gehalten werden, da sonst auch
                // die Uno-Klasse, z.B. "stardiv" immer wieder neu
                // aus der Registry gelesen werden muss
                if( pElem )
                {
                    // #63774 Darf nicht mit gespeichert werden!!!
                    pElem->SetFlag( SBX_DONTSTORE );
                    pElem->SetFlag( SBX_NO_MODIFY);

                    // #72382 Lokal speichern, sonst werden alle implizit
                    // deklarierten Vars automatisch global !
                    if ( bSetName )
                        pElem->SetName( aName );
                    refLocals->Put( pElem, refLocals->Count() );
                }
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
                    Error( nNotFound, aName );
                }
                else
                {
                    if ( bStatic )
                        pElem = StepSTATIC_Impl( aName, t );
                    if ( !pElem )
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
            USHORT nSavFlags = pElem->GetFlags();
            pElem->SetFlag( SBX_READWRITE | SBX_NO_BROADCAST );
            pElem->SbxValue::Clear();
            pElem->SetFlags( nSavFlags );

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
        // definitely we want this for VBA where properties are often
        // collections ( which need index access ), but lets only do
        // this if we actually have params following
        else if( bVBAEnabled && pElem->ISA(SbUnoProperty) && pElem->GetParameters() )
        {
            // pElem auf eine Ref zuweisen, um ggf. eine Temp-Var zu loeschen
            SbxVariableRef refTemp = pElem;

            // Variable kopieren und dabei den Notify aufloesen
            SbxVariable* pNew = new SbxVariable( *((SbxVariable*)pElem) ); // das ist der Call!
            pElem->SetParameters( NULL ); // sonst bleibt Ref auf sich selbst
            pElem = pNew;
        }
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
            USHORT nParamCount = refParams->Count();
            USHORT j = 1;
            const SbxParamInfo* pParam = pInfo->GetParam( j );
            while( pParam )
            {
                if( pParam->aName.EqualsIgnoreCaseAscii( rName ) )
                {
                    if( j >= nParamCount )
                    {
                        // Parameter is missing
                        pElem = new SbxVariable( SbxSTRING );
                        pElem->PutString( String( RTL_CONSTASCII_USTRINGPARAM("<missing parameter>" ) ) );
                    }
                    else
                    {
                        pElem = refParams->Get( j );
                    }
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

void SbiRuntime::SetupArgs( SbxVariable* p, UINT32 nOp1 )
{
    if( nOp1 & 0x8000 )
    {
        if( !refArgv )
            StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
        BOOL bHasNamed = FALSE;
        USHORT i;
        USHORT nArgCount = refArgv->Count();
        for( i = 1 ; i < nArgCount ; i++ )
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
            {
                bool bError_ = true;

                SbUnoMethod* pUnoMethod = PTR_CAST(SbUnoMethod,p);
                SbUnoProperty* pUnoProperty = PTR_CAST(SbUnoProperty,p);
                if( pUnoMethod || pUnoProperty )
                {
                    SbUnoObject* pParentUnoObj = PTR_CAST( SbUnoObject,p->GetParent() );
                    if( pParentUnoObj )
                    {
                        Any aUnoAny = pParentUnoObj->getUnoAny();
                        Reference< XInvocation > xInvocation;
                        aUnoAny >>= xInvocation;
                        if( xInvocation.is() )  // TODO: if( xOLEAutomation.is() )
                        {
                            bError_ = false;

                            USHORT nCurPar = 1;
                            AutomationNamedArgsSbxArray* pArg =
                                new AutomationNamedArgsSbxArray( nArgCount );
                            ::rtl::OUString* pNames = pArg->getNames().getArray();
                            for( i = 1 ; i < nArgCount ; i++ )
                            {
                                SbxVariable* pVar = refArgv->Get( i );
                                const String& rName = refArgv->GetAlias( i );
                                if( rName.Len() )
                                    pNames[i] = rName;
                                pArg->Put( pVar, nCurPar++ );
                            }
                            refArgv = pArg;
                        }
                    }
                }
                else if( bVBAEnabled && p->GetType() == SbxOBJECT && (!p->ISA(SbxMethod) || !p->IsBroadcaster()) )
                {
                    // Check for default method with named parameters
                    SbxBaseRef pObj = (SbxBase*)p->GetObject();
                    if( pObj && pObj->ISA(SbUnoObject) )
                    {
                        SbUnoObject* pUnoObj = (SbUnoObject*)(SbxBase*)pObj;
                        Any aAny = pUnoObj->getUnoAny();

                        if( aAny.getValueType().getTypeClass() == TypeClass_INTERFACE )
                        {
                            Reference< XInterface > x = *(Reference< XInterface >*)aAny.getValue();
                            Reference< XDefaultMethod > xDfltMethod( x, UNO_QUERY );

                            rtl::OUString sDefaultMethod;
                            if ( xDfltMethod.is() )
                                sDefaultMethod = xDfltMethod->getDefaultMethodName();
                            if ( sDefaultMethod.getLength() )
                            {
                                SbxVariable* meth = pUnoObj->Find( sDefaultMethod, SbxCLASS_METHOD );
                                if( meth != NULL )
                                    pInfo = meth->GetInfo();
                                if( pInfo )
                                    bError_ = false;
                            }
                        }
                    }
                }
                if( bError_ )
                    Error( SbERR_NO_NAMED_ARGS );
            }
            else
            {
                USHORT nCurPar = 1;
                SbxArray* pArg = new SbxArray;
                for( i = 1 ; i < nArgCount ; i++ )
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
    else if( pElem->GetType() == SbxOBJECT && (!pElem->ISA(SbxMethod) || (bVBAEnabled && !pElem->IsBroadcaster()) ) )
    {
        pPar = pElem->GetParameters();
        if ( pPar )
        {
            // Ist es ein Uno-Objekt?
            SbxBaseRef pObj = (SbxBase*)pElem->GetObject();
            if( pObj )
            {
                if( pObj->ISA(SbUnoObject) )
                {
                    SbUnoObject* pUnoObj = (SbUnoObject*)(SbxBase*)pObj;
                    Any aAny = pUnoObj->getUnoAny();

                    if( aAny.getValueType().getTypeClass() == TypeClass_INTERFACE )
                    {
                        Reference< XInterface > x = *(Reference< XInterface >*)aAny.getValue();
                        Reference< XIndexAccess > xIndexAccess( x, UNO_QUERY );
                        if ( !bVBAEnabled )
                        {
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
                                    Any aAny2 = xIndexAccess->getByIndex( nIndex );
                                    TypeClass eType = aAny2.getValueType().getTypeClass();
                                    if( eType == TypeClass_INTERFACE )
                                        xRet = *(Reference< XInterface >*)aAny2.getValue();
                                }
                                catch (IndexOutOfBoundsException&)
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
                        else
                        {
                            rtl::OUString sDefaultMethod;

                            Reference< XDefaultMethod > xDfltMethod( x, UNO_QUERY );

                            if ( xDfltMethod.is() )
                                sDefaultMethod = xDfltMethod->getDefaultMethodName();
                            else if( xIndexAccess.is() )
                                sDefaultMethod = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "getByIndex" ) );

                            if ( sDefaultMethod.getLength() )
                            {
                                SbxVariable* meth = pUnoObj->Find( sDefaultMethod, SbxCLASS_METHOD );
                                SbxVariableRef refTemp = meth;
                                if ( refTemp )
                                {
                                    meth->SetParameters( pPar );
                                    SbxVariable* pNew = new SbxMethod( *(SbxMethod*)meth );
                                    pElem = pNew;
                                }
                            }
                        }
                    }

                    // #42940, 0.Parameter zu NULL setzen, damit sich Var nicht selbst haelt
                    pPar->Put( NULL, 0 );
                }
                else if( pObj->ISA(BasicCollection) )
                {
                    BasicCollection* pCol = (BasicCollection*)(SbxBase*)pObj;
                    pElem = new SbxVariable( SbxVARIANT );
                    pPar->Put( pElem, 0 );
                    pCol->CollItem( pPar );
                }
            }
            else if( bVBAEnabled )  // !pObj
            {
                SbxArray* pParam = pElem->GetParameters();
                if( pParam != NULL )
                    Error( SbERR_NO_OBJECT );
            }
        }
    }

    return pElem;
}

// Laden eines Elements aus der Runtime-Library (+StringID+Typ)

void SbiRuntime::StepRTL( UINT32 nOp1, UINT32 nOp2 )
{
    PushVar( FindElement( rBasic.pRtl, nOp1, nOp2, SbERR_PROC_UNDEFINED, FALSE ) );
}

void
SbiRuntime::StepFIND_Impl( SbxObject* pObj, UINT32 nOp1, UINT32 nOp2, SbError nNotFound, BOOL bLocal, BOOL bStatic )
{
    if( !refLocals )
        refLocals = new SbxArray;
    PushVar( FindElement( pObj, nOp1, nOp2, nNotFound, bLocal, bStatic ) );
}
// Laden einer lokalen/globalen Variablen (+StringID+Typ)

void SbiRuntime::StepFIND( UINT32 nOp1, UINT32 nOp2 )
{
    StepFIND_Impl( pMod, nOp1, nOp2, SbERR_PROC_UNDEFINED, TRUE );
}

// Search inside a class module (CM) to enable global search in time
void SbiRuntime::StepFIND_CM( UINT32 nOp1, UINT32 nOp2 )
{

    SbClassModuleObject* pClassModuleObject = PTR_CAST(SbClassModuleObject,pMod);
    if( pClassModuleObject )
        pMod->SetFlag( SBX_GBLSEARCH );

    StepFIND_Impl( pMod, nOp1, nOp2, SbERR_PROC_UNDEFINED, TRUE );

    if( pClassModuleObject )
        pMod->ResetFlag( SBX_GBLSEARCH );
}

void SbiRuntime::StepFIND_STATIC( UINT32 nOp1, UINT32 nOp2 )
{
    StepFIND_Impl( pMod, nOp1, nOp2, SbERR_PROC_UNDEFINED, TRUE, TRUE );
}

// Laden eines Objekt-Elements (+StringID+Typ)
// Das Objekt liegt auf TOS

void SbiRuntime::StepELEM( UINT32 nOp1, UINT32 nOp2 )
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
// Der Datentyp SbxEMPTY zeigt an, daa kein Parameter angegeben ist.
// Get( 0 ) darf EMPTY sein

void SbiRuntime::StepPARAM( UINT32 nOp1, UINT32 nOp2 )
{
    USHORT i = static_cast<USHORT>( nOp1 & 0x7FFF );
    SbxDataType t = (SbxDataType) nOp2;
    SbxVariable* p;

    // #57915 Missing sauberer loesen
    USHORT nParamCount = refParams->Count();
    if( i >= nParamCount )
    {
        INT16 iLoop = i;
        while( iLoop >= nParamCount )
        {
            p = new SbxVariable();

            if( SbiRuntime::isVBAEnabled() &&
                (t == SbxOBJECT || t == SbxSTRING) )
            {
                if( t == SbxOBJECT )
                    p->PutObject( NULL );
                else
                    p->PutString( String() );
            }
            else
                p->PutErr( 448 );       // Wie in VB: Error-Code 448 (SbERR_NAMED_NOT_FOUND)

            refParams->Put( p, iLoop );
            iLoop--;
        }
    }
    p = refParams->Get( i );

    if( p->GetType() == SbxERROR && ( i ) )
    //if( p->GetType() == SbxEMPTY && ( i ) )
    {
        // Wenn ein Parameter fehlt, kann er OPTIONAL sein
        BOOL bOpt = FALSE;
        if( pMeth )
        {
            SbxInfo* pInfo = pMeth->GetInfo();
            if ( pInfo )
            {
                const SbxParamInfo* pParam = pInfo->GetParam( i );
                if( pParam && ( (pParam->nFlags & SBX_OPTIONAL) != 0 ) )
                {
                    // Default value?
                    USHORT nDefaultId = sal::static_int_cast< USHORT >(
                        pParam->nUserData & 0xffff );
                    if( nDefaultId > 0 )
                    {
                        String aDefaultStr = pImg->GetString( nDefaultId );
                        p = new SbxVariable();
                        p->PutString( aDefaultStr );
                        refParams->Put( p, i );
                    }
                    bOpt = TRUE;
                }
            }
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

void SbiRuntime::StepCASEIS( UINT32 nOp1, UINT32 nOp2 )
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

void SbiRuntime::StepCALL( UINT32 nOp1, UINT32 nOp2 )
{
    String aName = pImg->GetString( static_cast<short>( nOp1 & 0x7FFF ) );
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

void SbiRuntime::StepCALLC( UINT32 nOp1, UINT32 nOp2 )
{
    String aName = pImg->GetString( static_cast<short>( nOp1 & 0x7FFF ) );
    SbxArray* pArgs = NULL;
    if( nOp1 & 0x8000 )
        pArgs = refArgv;
    DllCall( aName, aLibName, pArgs, (SbxDataType) nOp2, TRUE );
    aLibName = String();
    if( nOp1 & 0x8000 )
        PopArgv();
}


// Beginn eines Statements (+Line+Col)

void SbiRuntime::StepSTMNT( UINT32 nOp1, UINT32 nOp2 )
{
    // Wenn der Expr-Stack am Anfang einen Statements eine Variable enthaelt,
    // hat ein Trottel X als Funktion aufgerufen, obwohl es eine Variable ist!
    BOOL bFatalExpr = FALSE;
    String sUnknownMethodName;
    if( nExprLvl > 1 )
        bFatalExpr = TRUE;
    else if( nExprLvl )
    {
        SbxVariable* p = refExprStk->Get( 0 );
        if( p->GetRefCount() > 1
         && refLocals.Is() && refLocals->Find( p->GetName(), p->GetClass() ) )
        {
            sUnknownMethodName = p->GetName();
            bFatalExpr = TRUE;
        }
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
        StarBASIC::FatalError( SbERR_NO_METHOD, sUnknownMethodName );
        return;
    }
    pStmnt = pCode - 9;
    USHORT nOld = nLine;
    nLine = static_cast<short>( nOp1 );

    // #29955 & 0xFF, um for-Schleifen-Ebene wegzufiltern
    nCol1 = static_cast<short>( nOp2 & 0xFF );

    // Suchen des naechsten STMNT-Befehls,
    // um die End-Spalte dieses Statements zu setzen
    // Searches of the next STMNT instruction,
    // around the final column of this statement to set

    nCol2 = 0xffff;
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
        USHORT nExspectedForLevel = static_cast<USHORT>( nOp2 / 0x100 );
        if( pGosubStk )
            nExspectedForLevel = nExspectedForLevel + pGosubStk->nStartForLvl;

        // Wenn der tatsaechliche For-Level zu klein ist, wurde aus
        // einer Schleife heraus gesprungen -> korrigieren
        while( nForLvl > nExspectedForLevel )
            PopFor();
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
        && pMod->IsBP( static_cast<USHORT>( nOp1 ) ) )
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

void SbiRuntime::StepOPEN( UINT32 nOp1, UINT32 nOp2 )
{
    SbxVariableRef pName = PopVar();
    SbxVariableRef pChan = PopVar();
    SbxVariableRef pLen  = PopVar();
    short nBlkLen = pLen->GetInteger();
    short nChan   = pChan->GetInteger();
    ByteString aName( pName->GetString(), gsl_getSystemTextEncoding() );
    pIosys->Open( nChan, aName, static_cast<short>( nOp1 ),
        static_cast<short>( nOp2 ), nBlkLen );
    Error( pIosys->GetError() );
}

// Objekt kreieren (+StringID+StringID)

void SbiRuntime::StepCREATE( UINT32 nOp1, UINT32 nOp2 )
{
    String aClass( pImg->GetString( static_cast<short>( nOp2 ) ) );
    SbxObject *pObj = SbxBase::CreateObject( aClass );
    if( !pObj )
        Error( SbERR_INVALID_OBJECT );
    else
    {
        String aName( pImg->GetString( static_cast<short>( nOp1 ) ) );
        pObj->SetName( aName );
    // Das Objekt muss BASIC rufen koennen
        pObj->SetParent( &rBasic );
        SbxVariable* pNew = new SbxVariable;
        pNew->PutObject( pObj );
        PushVar( pNew );
    }
}

void SbiRuntime::StepDCREATE( UINT32 nOp1, UINT32 nOp2 )
{
    StepDCREATE_IMPL( nOp1, nOp2 );
}

void SbiRuntime::StepDCREATE_REDIMP( UINT32 nOp1, UINT32 nOp2 )
{
    StepDCREATE_IMPL( nOp1, nOp2 );
}


// Helper function for StepDCREATE_IMPL / bRedimp = true
void implCopyDimArray_DCREATE( SbxDimArray* pNewArray, SbxDimArray* pOldArray, short nMaxDimIndex,
    short nActualDim, sal_Int32* pActualIndices, sal_Int32* pLowerBounds, sal_Int32* pUpperBounds )
{
    sal_Int32& ri = pActualIndices[nActualDim];
    for( ri = pLowerBounds[nActualDim] ; ri <= pUpperBounds[nActualDim] ; ri++ )
    {
        if( nActualDim < nMaxDimIndex )
        {
            implCopyDimArray_DCREATE( pNewArray, pOldArray, nMaxDimIndex, nActualDim + 1,
                pActualIndices, pLowerBounds, pUpperBounds );
        }
        else
        {
            SbxVariable* pSource = pOldArray->Get32( pActualIndices );
            pNewArray->Put32( pSource, pActualIndices );
        }
    }
}

// #56204 Objekt-Array kreieren (+StringID+StringID), DCREATE == Dim-Create
void SbiRuntime::StepDCREATE_IMPL( UINT32 nOp1, UINT32 nOp2 )
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

    SbxDimArray* pArray = 0;
    if( xObj->ISA(SbxDimArray) )
    {
        SbxBase* pObj = (SbxBase*)xObj;
        pArray = (SbxDimArray*)pObj;

        // Dimensionen auswerten
        short nDims = pArray->GetDims();
        INT32 nTotalSize = 0;

        // es muss ein eindimensionales Array sein
        INT32 nLower, nUpper, nSize;
        INT32 i;
        for( i = 0 ; i < nDims ; i++ )
        {
            pArray->GetDim32( i+1, nLower, nUpper );
            nSize = nUpper - nLower + 1;
            if( i == 0 )
                nTotalSize = nSize;
            else
                nTotalSize *= nSize;
        }

        // Objekte anlegen und ins Array eintragen
        String aClass( pImg->GetString( static_cast<short>( nOp2 ) ) );
        for( i = 0 ; i < nTotalSize ; i++ )
        {
            SbxObject *pClassObj = SbxBase::CreateObject( aClass );
            if( !pClassObj )
            {
                Error( SbERR_INVALID_OBJECT );
                break;
            }
            else
            {
                String aName( pImg->GetString( static_cast<short>( nOp1 ) ) );
                pClassObj->SetName( aName );
                // Das Objekt muss BASIC rufen koennen
                pClassObj->SetParent( &rBasic );
                pArray->SbxArray::Put32( pClassObj, i );
            }
        }
    }

    SbxDimArray* pOldArray = (SbxDimArray*)(SbxArray*)refRedimpArray;
    if( pArray && pOldArray )
    {
        short nDimsNew = pArray->GetDims();
        short nDimsOld = pOldArray->GetDims();
        short nDims = nDimsNew;
        BOOL bRangeError = FALSE;

        // Store dims to use them for copying later
        sal_Int32* pLowerBounds = new sal_Int32[nDims];
        sal_Int32* pUpperBounds = new sal_Int32[nDims];
        sal_Int32* pActualIndices = new sal_Int32[nDims];
        if( nDimsOld != nDimsNew )
        {
            bRangeError = TRUE;
        }
        else
        {
            // Compare bounds
            for( short i = 1 ; i <= nDims ; i++ )
            {
                sal_Int32 lBoundNew, uBoundNew;
                sal_Int32 lBoundOld, uBoundOld;
                pArray->GetDim32( i, lBoundNew, uBoundNew );
                pOldArray->GetDim32( i, lBoundOld, uBoundOld );

                lBoundNew = std::max( lBoundNew, lBoundOld );
                uBoundNew = std::min( uBoundNew, uBoundOld );
                short j = i - 1;
                pActualIndices[j] = pLowerBounds[j] = lBoundNew;
                pUpperBounds[j] = uBoundNew;
            }
        }

        if( bRangeError )
        {
            StarBASIC::Error( SbERR_OUT_OF_RANGE );
        }
        else
        {
            // Copy data from old array by going recursively through all dimensions
            // (It would be faster to work on the flat internal data array of an
            // SbyArray but this solution is clearer and easier)
            implCopyDimArray_DCREATE( pArray, pOldArray, nDims - 1,
                0, pActualIndices, pLowerBounds, pUpperBounds );
        }
        delete [] pUpperBounds;
        delete [] pLowerBounds;
        delete [] pActualIndices;
        refRedimpArray = NULL;
    }
}

// Objekt aus User-Type kreieren  (+StringID+StringID)

SbxObject* createUserTypeImpl( const String& rClassName );  // sb.cxx

void SbiRuntime::StepTCREATE( UINT32 nOp1, UINT32 nOp2 )
{
    String aName( pImg->GetString( static_cast<short>( nOp1 ) ) );
    String aClass( pImg->GetString( static_cast<short>( nOp2 ) ) );

    SbxObject* pCopyObj = createUserTypeImpl( aClass );
    if( pCopyObj )
        pCopyObj->SetName( aName );
    SbxVariable* pNew = new SbxVariable;
    pNew->PutObject( pCopyObj );
    pNew->SetDeclareClassName( aClass );
    PushVar( pNew );
}

void SbiRuntime::implHandleSbxFlags( SbxVariable* pVar, SbxDataType t, UINT32 nOp2 )
{
    bool bWithEvents = ((t & 0xff) == SbxOBJECT && (nOp2 & SBX_TYPE_WITH_EVENTS_FLAG) != 0);
    if( bWithEvents )
        pVar->SetFlag( SBX_WITH_EVENTS );

    bool bDimAsNew = ((nOp2 & SBX_TYPE_DIM_AS_NEW_FLAG) != 0);
    if( bDimAsNew )
        pVar->SetFlag( SBX_DIM_AS_NEW );

    bool bFixedString = ((t & 0xff) == SbxSTRING && (nOp2 & SBX_FIXED_LEN_STRING_FLAG) != 0);
    if( bFixedString )
    {
        USHORT nCount = static_cast<USHORT>( nOp2 >> 17 );      // len = all bits above 0x10000
        String aStr;
        aStr.Fill( nCount, 0 );
        pVar->PutString( aStr );
    }
}

// Einrichten einer lokalen Variablen (+StringID+Typ)

void SbiRuntime::StepLOCAL( UINT32 nOp1, UINT32 nOp2 )
{
    if( !refLocals.Is() )
        refLocals = new SbxArray;
    String aName( pImg->GetString( static_cast<short>( nOp1 ) ) );
    if( refLocals->Find( aName, SbxCLASS_DONTCARE ) == NULL )
    {
        SbxDataType t = (SbxDataType)(nOp2 & 0xffff);
        SbxVariable* p = new SbxVariable( t );
        p->SetName( aName );
        implHandleSbxFlags( p, t, nOp2 );
        refLocals->Put( p, refLocals->Count() );
    }
}

// Einrichten einer modulglobalen Variablen (+StringID+Typ)

void SbiRuntime::StepPUBLIC_Impl( UINT32 nOp1, UINT32 nOp2, bool bUsedForClassModule )
{
    String aName( pImg->GetString( static_cast<short>( nOp1 ) ) );
    SbxDataType t = (SbxDataType)(SbxDataType)(nOp2 & 0xffff);;
    BOOL bFlag = pMod->IsSet( SBX_NO_MODIFY );
    pMod->SetFlag( SBX_NO_MODIFY );
    SbxVariableRef p = pMod->Find( aName, SbxCLASS_PROPERTY );
    if( p.Is() )
        pMod->Remove (p);
    SbProperty* pProp = pMod->GetProperty( aName, t );
    if( !bUsedForClassModule )
        pProp->SetFlag( SBX_PRIVATE );
    if( !bFlag )
        pMod->ResetFlag( SBX_NO_MODIFY );
    if( pProp )
    {
        pProp->SetFlag( SBX_DONTSTORE );
        // AB: 2.7.1996: HACK wegen 'Referenz kann nicht gesichert werden'
        pProp->SetFlag( SBX_NO_MODIFY);

        implHandleSbxFlags( pProp, t, nOp2 );
    }
}

void SbiRuntime::StepPUBLIC( UINT32 nOp1, UINT32 nOp2 )
{
    StepPUBLIC_Impl( nOp1, nOp2, false );
}

void SbiRuntime::StepPUBLIC_P( UINT32 nOp1, UINT32 nOp2 )
{
    // Creates module variable that isn't reinitialised when
    // between invocations ( for VBASupport & document basic only )
    if( pMod->pImage->bFirstInit )
    {
        bool bUsedForClassModule = pImg->GetFlag( SBIMG_CLASSMODULE );
        StepPUBLIC_Impl( nOp1, nOp2, bUsedForClassModule );
    }
}

// Einrichten einer globalen Variablen (+StringID+Typ)

void SbiRuntime::StepGLOBAL( UINT32 nOp1, UINT32 nOp2 )
{
    if( pImg->GetFlag( SBIMG_CLASSMODULE ) )
        StepPUBLIC_Impl( nOp1, nOp2, true );

    String aName( pImg->GetString( static_cast<short>( nOp1 ) ) );
    SbxDataType t = (SbxDataType)(nOp2 & 0xffff);

    // Store module scope variables at module scope
    // in non vba mode these are stored at the library level :/
    // not sure if this really should not be enabled for ALL basic
    SbxObject* pStorage = &rBasic;
    if ( SbiRuntime::isVBAEnabled() )
    {
        pStorage = pMod;
        pMod->AddVarName( aName );
    }

    BOOL bFlag = pStorage->IsSet( SBX_NO_MODIFY );
    rBasic.SetFlag( SBX_NO_MODIFY );
    SbxVariableRef p = pStorage->Find( aName, SbxCLASS_PROPERTY );
    if( p.Is() )
        pStorage->Remove (p);
    p = pStorage->Make( aName, SbxCLASS_PROPERTY, t );
    if( !bFlag )
        pStorage->ResetFlag( SBX_NO_MODIFY );
    if( p )
    {
        p->SetFlag( SBX_DONTSTORE );
        // AB: 2.7.1996: HACK wegen 'Referenz kann nicht gesichert werden'
        p->SetFlag( SBX_NO_MODIFY);
    }
}


// Creates global variable that isn't reinitialised when
// basic is restarted, P=PERSIST (+StringID+Typ)

void SbiRuntime::StepGLOBAL_P( UINT32 nOp1, UINT32 nOp2 )
{
    if( pMod->pImage->bFirstInit )
    {
        StepGLOBAL( nOp1, nOp2 );
    }
}


// Searches for global variable, behavior depends on the fact
// if the variable is initialised for the first time

void SbiRuntime::StepFIND_G( UINT32 nOp1, UINT32 nOp2 )
{
    if( pMod->pImage->bFirstInit )
    {
        // Behave like always during first init
        StepFIND( nOp1, nOp2 );
    }
    else
    {
        // Return dummy variable
        SbxDataType t = (SbxDataType) nOp2;
        String aName( pImg->GetString( static_cast<short>( nOp1 & 0x7FFF ) ) );

        SbxVariable* pDummyVar = new SbxVariable( t );
        pDummyVar->SetName( aName );
        PushVar( pDummyVar );
    }
}


SbxVariable* SbiRuntime::StepSTATIC_Impl( String& aName, SbxDataType& t )
{
    SbxVariable* p = NULL;
    if ( pMeth )
    {
        SbxArray* pStatics = pMeth->GetStatics();
        if( pStatics && ( pStatics->Find( aName, SbxCLASS_DONTCARE ) == NULL ) )
        {
            p = new SbxVariable( t );
            if( t != SbxVARIANT )
                p->SetFlag( SBX_FIXED );
            p->SetName( aName );
            pStatics->Put( p, pStatics->Count() );
        }
    }
    return p;
}
// Einrichten einer statischen Variablen (+StringID+Typ)
void SbiRuntime::StepSTATIC( UINT32 nOp1, UINT32 nOp2 )
{
    String aName( pImg->GetString( static_cast<short>( nOp1 ) ) );
    SbxDataType t = (SbxDataType) nOp2;
    StepSTATIC_Impl( aName, t );
}

