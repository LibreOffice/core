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


#include "runtime.hxx"
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


// the bits in the String-ID:
// 0x8000 - Argv is reserved

SbxVariable* SbiRuntime::FindElement
    ( SbxObject* pObj, sal_uInt32 nOp1, sal_uInt32 nOp2, SbError nNotFound, bool bLocal, bool bStatic )
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
        bool bFatalError = false;
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
            StepARGV();
            nOp1 = nOp1 | 0x8000; // indicate params are present
            aName = rtl::OUString("Evaluate");
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
            bool bSave = rBasic.bNoRtl;
            rBasic.bNoRtl = true;
            pElem = pObj->Find( aName, SbxCLASS_DONTCARE );

            // #110004, #112015: Make private really private
            if( bLocal && pElem )   // Local as flag for global search
            {
                if( pElem->IsSet( SBX_PRIVATE ) )
                {
                    SbiInstance* pInst_ = GetSbData()->pInst;
                    if( pInst_ && pInst_->IsCompatibility() && pObj != pElem->GetParent() )
                        pElem = NULL;   // Found but in wrong module!

                    // Interfaces: Use SBX_EXTFOUND
                }
            }
            rBasic.bNoRtl = bSave;

            // is it a global uno-identifier?
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
                        pElem = VBAConstantHelper::instance().getVBAConstant( aName );
                }

                if( !pElem )
                {
                    // #72382 ATTENTION! ALWAYS returns a result now
                    // because of unknown modules!
                    SbUnoClass* pUnoClass = findUnoClass( aName );
                    if( pUnoClass )
                    {
                        pElem = new SbxVariable( t );
                        SbxValues aRes( SbxOBJECT );
                        aRes.pObj = pUnoClass;
                        pElem->SbxVariable::Put( aRes );
                    }
                }

                // #62939 If an uno-class has been found, the wrapper
                // object has to be held, because the uno-class, e. g.
                // "stardiv", has to be read out of the registry
                // every time again otherwise
                if( pElem )
                {
                    // #63774 May not be saved too!!!
                    pElem->SetFlag( SBX_DONTSTORE );
                    pElem->SetFlag( SBX_NO_MODIFY);

                    // #72382 save locally, all variables that have been declared
                    // implicit would become global automatically otherwise!
                    if ( bSetName )
                        pElem->SetName( aName );
                    refLocals->Put( pElem, refLocals->Count() );
                }
            }

            if( !pElem )
            {
                // not there and not in the object?
                // don't establish if that thing has parameters!
                if( nOp1 & 0x8000 )
                    bFatalError = true;

                // else, if there are parameters, use different error code
                if( !bLocal || pImg->GetFlag( SBIMG_EXPLICIT ) )
                {
                    // #39108 if explicit and as ELEM always a fatal error
                    bFatalError = true;


                    if( !( nOp1 & 0x8000 ) && nNotFound == SbERR_PROC_UNDEFINED )
                        nNotFound = SbERR_VAR_UNDEFINED;
                }
                if( bFatalError )
                {
                    // #39108 use dummy variable instead of fatal error
                    if( !xDummyVar.Is() )
                        xDummyVar = new SbxVariable( SbxVARIANT );
                    pElem = xDummyVar;

                    ClearArgvStack();

                    Error( nNotFound, aName );
                }
                else
                {
                    if ( bStatic )
                        pElem = StepSTATIC_Impl( aName, t );
                    if ( !pElem )
                    {
                        pElem = new SbxVariable( t );
                        if( t != SbxVARIANT )
                            pElem->SetFlag( SBX_FIXED );
                        pElem->SetName( aName );
                        refLocals->Put( pElem, refLocals->Count() );
                    }
                }
            }
        }
        // #39108 Args can already be deleted!
        if( !bFatalError )
            SetupArgs( pElem, nOp1 );

        // because a particular call-type is requested
        if( pElem->IsA( TYPE(SbxMethod) ) )
        {
            // shall the type be converted?
            SbxDataType t2 = pElem->GetType();
            bool bSet = false;
            if( !( pElem->GetFlags() & SBX_FIXED ) )
            {
                if( t != SbxVARIANT && t != t2 &&
                    t >= SbxINTEGER && t <= SbxSTRING )
                    pElem->SetType( t ), bSet = true;
            }
            // assign pElem to a Ref, to delete a temp-var if applicable
            SbxVariableRef refTemp = pElem;

            // remove potential rests of the last call of the SbxMethod
            // free Write before, so that there's no error
            sal_uInt16 nSavFlags = pElem->GetFlags();
            pElem->SetFlag( SBX_READWRITE | SBX_NO_BROADCAST );
            pElem->SbxValue::Clear();
            pElem->SetFlags( nSavFlags );

            // don't touch before setting, as e. g. LEFT()
            // has to know the difference between Left$() and Left()

            // because the methods' parameters are cut away in PopVar()
            SbxVariable* pNew = new SbxMethod( *((SbxMethod*)pElem) );
            //OLD: SbxVariable* pNew = new SbxVariable( *pElem );

            pElem->SetParameters(0);
            pNew->SetFlag( SBX_READWRITE );

            if( bSet )
                pElem->SetType( t2 );
            pElem = pNew;
        }
        // consider index-access for UnoObjects
        // definitely we want this for VBA where properties are often
        // collections ( which need index access ), but lets only do
        // this if we actually have params following
        else if( bVBAEnabled && pElem->ISA(SbUnoProperty) && pElem->GetParameters() )
        {
            SbxVariableRef refTemp = pElem;

            // dissolve the notify while copying variable
            SbxVariable* pNew = new SbxVariable( *((SbxVariable*)pElem) );
            pElem->SetParameters( NULL );
            pElem = pNew;
        }
    }
    return CheckArray( pElem );
}

// for current scope (e. g. query from BASIC-IDE)
SbxBase* SbiRuntime::FindElementExtern( const String& rName )
{
    // don't expect pMeth to be != 0, as there are none set
    // in the RunInit yet

    SbxVariable* pElem = NULL;
    if( !pMod || !rName.Len() )
        return NULL;

    if( refLocals )
        pElem = refLocals->Find( rName, SbxCLASS_DONTCARE );

    if ( !pElem && pMeth )
    {
        // for statics, set the method's name in front
        String aMethName = pMeth->GetName();
        aMethName += ':';
        aMethName += rName;
        pElem = pMod->Find(aMethName, SbxCLASS_DONTCARE);
    }

    // search in parameter list
    if( !pElem && pMeth )
    {
        SbxInfo* pInfo = pMeth->GetInfo();
        if( pInfo && refParams )
        {
            sal_uInt16 nParamCount = refParams->Count();
            sal_uInt16 j = 1;
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

    // search in module
    if( !pElem )
    {
        bool bSave = rBasic.bNoRtl;
        rBasic.bNoRtl = true;
        pElem = pMod->Find( rName, SbxCLASS_DONTCARE );
        rBasic.bNoRtl = bSave;
    }
    return pElem;
}



void SbiRuntime::SetupArgs( SbxVariable* p, sal_uInt32 nOp1 )
{
    if( nOp1 & 0x8000 )
    {
        if( !refArgv )
            StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
        bool bHasNamed = false;
        sal_uInt16 i;
        sal_uInt16 nArgCount = refArgv->Count();
        for( i = 1 ; i < nArgCount ; i++ )
        {
            if( refArgv->GetAlias( i ).Len() )
            {
                bHasNamed = true; break;
            }
        }
        if( bHasNamed )
        {
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

                            sal_uInt16 nCurPar = 1;
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
                            if ( !sDefaultMethod.isEmpty() )
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
                sal_uInt16 nCurPar = 1;
                SbxArray* pArg = new SbxArray;
                for( i = 1 ; i < nArgCount ; i++ )
                {
                    SbxVariable* pVar = refArgv->Get( i );
                    const String& rName = refArgv->GetAlias( i );
                    if( rName.Len() )
                    {
                        // nCurPar is set to the found parameter
                        sal_uInt16 j = 1;
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
        // own var as parameter 0
        refArgv->Put( p, 0 );
        p->SetParameters( refArgv );
        PopArgv();
    }
    else
        p->SetParameters( NULL );
}

// getting an array element

SbxVariable* SbiRuntime::CheckArray( SbxVariable* pElem )
{
    SbxArray* pPar;
    if( ( pElem->GetType() & SbxARRAY ) && (SbxVariable*)refRedim != pElem )
    {
        SbxBase* pElemObj = pElem->GetObject();
        SbxDimArray* pDimArray = PTR_CAST(SbxDimArray,pElemObj);
        pPar = pElem->GetParameters();
        if( pDimArray )
        {
            // parameters may be missing, if an array is
            // passed as an argument
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

        // #42940, set parameter 0 to NULL so that var doesn't contain itself
        if( pPar )
            pPar->Put( NULL, 0 );
    }
    // consider index-access for UnoObjects
    else if( pElem->GetType() == SbxOBJECT && !pElem->ISA(SbxMethod) && ( !bVBAEnabled || ( bVBAEnabled && !pElem->ISA(SbxProperty) ) ) )
    {
        pPar = pElem->GetParameters();
        if ( pPar )
        {
            // is it an uno-object?
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
                            if( xIndexAccess.is() )
                            {
                                sal_uInt32 nParamCount = (sal_uInt32)pPar->Count() - 1;
                                if( nParamCount != 1 )
                                {
                                    StarBASIC::Error( SbERR_BAD_ARGUMENT );
                                    return pElem;
                                }

                                // get index
                                sal_Int32 nIndex = pPar->Get( 1 )->GetLong();
                                Reference< XInterface > xRet;
                                try
                                {
                                    Any aAny2 = xIndexAccess->getByIndex( nIndex );
                                    TypeClass eType = aAny2.getValueType().getTypeClass();
                                    if( eType == TypeClass_INTERFACE )
                                        xRet = *(Reference< XInterface >*)aAny2.getValue();
                                }
                                catch (const IndexOutOfBoundsException&)
                                {
                                    // usually expect converting problem
                                    StarBASIC::Error( SbERR_OUT_OF_RANGE );
                                }

                                // #57847 always create a new variable, else error
                                // due to PutObject(NULL) at ReadOnly-properties
                                pElem = new SbxVariable( SbxVARIANT );
                                if( xRet.is() )
                                {
                                    aAny <<= xRet;

                                    // #67173 don't specify a name so that the real class name is entered
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

                            if ( !sDefaultMethod.isEmpty() )
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

                    // #42940, set parameter 0 to NULL so that var doesn't contain itself
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
                if( pParam != NULL && !pElem->IsSet( SBX_VAR_TO_DIM ) )
                    Error( SbERR_NO_OBJECT );
            }
        }
    }

    return pElem;
}

// loading an element from the runtime-library (+StringID+type)

void SbiRuntime::StepRTL( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    PushVar( FindElement( rBasic.pRtl, nOp1, nOp2, SbERR_PROC_UNDEFINED, false ) );
}

void
SbiRuntime::StepFIND_Impl( SbxObject* pObj, sal_uInt32 nOp1, sal_uInt32 nOp2, SbError nNotFound, bool bLocal, bool bStatic )
{
    if( !refLocals )
        refLocals = new SbxArray;
    PushVar( FindElement( pObj, nOp1, nOp2, nNotFound, bLocal, bStatic ) );
}
// loading a local/global variable (+StringID+type)

void SbiRuntime::StepFIND( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    StepFIND_Impl( pMod, nOp1, nOp2, SbERR_PROC_UNDEFINED, true );
}

// Search inside a class module (CM) to enable global search in time
void SbiRuntime::StepFIND_CM( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{

    SbClassModuleObject* pClassModuleObject = PTR_CAST(SbClassModuleObject,pMod);
    if( pClassModuleObject )
        pMod->SetFlag( SBX_GBLSEARCH );

    StepFIND_Impl( pMod, nOp1, nOp2, SbERR_PROC_UNDEFINED, true );

    if( pClassModuleObject )
        pMod->ResetFlag( SBX_GBLSEARCH );
}

void SbiRuntime::StepFIND_STATIC( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    StepFIND_Impl( pMod, nOp1, nOp2, SbERR_PROC_UNDEFINED, true, true );
}

// loading an object-element (+StringID+type)
// the object lies on TOS

void SbiRuntime::StepELEM( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    SbxVariableRef pObjVar = PopVar();

    SbxObject* pObj = PTR_CAST(SbxObject,(SbxVariable*) pObjVar);
    if( !pObj )
    {
        SbxBase* pObjVarObj = pObjVar->GetObject();
        pObj = PTR_CAST(SbxObject,pObjVarObj);
    }

    // #56368 save reference at StepElem, otherwise objects could
    // lose their reference too early in qualification chains like
    // ActiveComponent.Selection(0).Text
    // #74254 now per list
    if( pObj )
        SaveRef( (SbxVariable*)pObj );

    PushVar( FindElement( pObj, nOp1, nOp2, SbERR_NO_METHOD, false ) );
}

// loading a parameter (+offset+type)
// If the data type is wrong, create a copy.
// The data type SbxEMPTY shows that no parameters are given.
// Get( 0 ) may be EMPTY

void SbiRuntime::StepPARAM( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    sal_uInt16 i = static_cast<sal_uInt16>( nOp1 & 0x7FFF );
    SbxDataType t = (SbxDataType) nOp2;
    SbxVariable* p;

    // #57915 solve missing in a cleaner way
    sal_uInt16 nParamCount = refParams->Count();
    if( i >= nParamCount )
    {
        sal_Int16 iLoop = i;
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
                p->PutErr( 448 );       // like in VB: Error-Code 448 (SbERR_NAMED_NOT_FOUND)

            refParams->Put( p, iLoop );
            iLoop--;
        }
    }
    p = refParams->Get( i );

    if( p->GetType() == SbxERROR && ( i ) )
    {
        // if there's a parameter missing, it can be OPTIONAL
        bool bOpt = false;
        if( pMeth )
        {
            SbxInfo* pInfo = pMeth->GetInfo();
            if ( pInfo )
            {
                const SbxParamInfo* pParam = pInfo->GetParam( i );
                if( pParam && ( (pParam->nFlags & SBX_OPTIONAL) != 0 ) )
                {
                    // Default value?
                    sal_uInt16 nDefaultId = sal::static_int_cast< sal_uInt16 >(
                        pParam->nUserData & 0xffff );
                    if( nDefaultId > 0 )
                    {
                        String aDefaultStr = pImg->GetString( nDefaultId );
                        p = new SbxVariable();
                        p->PutString( aDefaultStr );
                        refParams->Put( p, i );
                    }
                    bOpt = true;
                }
            }
        }
        if( !bOpt )
            Error( SbERR_NOT_OPTIONAL );
    }
    else if( t != SbxVARIANT && (SbxDataType)(p->GetType() & 0x0FFF ) != t )
    {
        SbxVariable* q = new SbxVariable( t );
        SaveRef( q );
        *q = *p;
        p = q;
        if ( i )
            refParams->Put( p, i );
    }
    SetupArgs( p, nOp1 );
    PushVar( CheckArray( p ) );
}

// Case-Test (+True-Target+Test-Opcode)

void SbiRuntime::StepCASEIS( sal_uInt32 nOp1, sal_uInt32 nOp2 )
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

// call of a DLL-procedure (+StringID+type)
// the StringID's MSB shows that Argv is occupied

void SbiRuntime::StepCALL( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    String aName = pImg->GetString( static_cast<short>( nOp1 & 0x7FFF ) );
    SbxArray* pArgs = NULL;
    if( nOp1 & 0x8000 )
        pArgs = refArgv;
    DllCall( aName, aLibName, pArgs, (SbxDataType) nOp2, false );
    aLibName = String();
    if( nOp1 & 0x8000 )
        PopArgv();
}

// call of a DLL-procedure after CDecl (+StringID+type)

void SbiRuntime::StepCALLC( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    String aName = pImg->GetString( static_cast<short>( nOp1 & 0x7FFF ) );
    SbxArray* pArgs = NULL;
    if( nOp1 & 0x8000 )
        pArgs = refArgv;
    DllCall( aName, aLibName, pArgs, (SbxDataType) nOp2, true );
    aLibName = String();
    if( nOp1 & 0x8000 )
        PopArgv();
}


// beginning of a statement (+Line+Col)

void SbiRuntime::StepSTMNT( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    // If the Expr-Stack at the beginning of a statement constains a variable,
    // some fool has called X as a function, although it's a variable!
    bool bFatalExpr = false;
    String sUnknownMethodName;
    if( nExprLvl > 1 )
        bFatalExpr = true;
    else if( nExprLvl )
    {
        SbxVariable* p = refExprStk->Get( 0 );
        if( p->GetRefCount() > 1
         && refLocals.Is() && refLocals->Find( p->GetName(), p->GetClass() ) )
        {
            sUnknownMethodName = p->GetName();
            bFatalExpr = true;
        }
    }

    ClearExprStack();

    ClearRefs();

    // We have to cancel hard here because line and column
    // would be wrong later otherwise!
    if( bFatalExpr)
    {
        StarBASIC::FatalError( SbERR_NO_METHOD, sUnknownMethodName );
        return;
    }
    pStmnt = pCode - 9;
    sal_uInt16 nOld = nLine;
    nLine = static_cast<short>( nOp1 );

    // #29955 & 0xFF, to filter out for-loop-level
    nCol1 = static_cast<short>( nOp2 & 0xFF );

    // find the next STMNT-command to set the final column
    // of this statement

    nCol2 = 0xffff;
    sal_uInt16 n1, n2;
    const sal_uInt8* p = pMod->FindNextStmnt( pCode, n1, n2 );
    if( p )
    {
        if( n1 == nOp1 )
        {
            // #29955 & 0xFF, to filter out for-loop-level
            nCol2 = (n2 & 0xFF) - 1;
        }
    }

    // #29955 correct for-loop-level, #67452 NOT in the error-handler
    if( !bInError )
    {
        // (there's a difference here in case of a jump out of a loop)
        sal_uInt16 nExspectedForLevel = static_cast<sal_uInt16>( nOp2 / 0x100 );
        if( pGosubStk )
            nExspectedForLevel = nExspectedForLevel + pGosubStk->nStartForLvl;

        // if the actual for-level is too small it'd jump out
        // of a loop -> corrected
        while( nForLvl > nExspectedForLevel )
            PopFor();
    }

    // 16.10.96: #31460 new concept for StepInto/Over/Out
    // see explanation at _ImplGetBreakCallLevel
    if( pInst->nCallLvl <= pInst->nBreakCallLvl )
    {
        StarBASIC* pStepBasic = GetCurrentBasic( &rBasic );
        sal_uInt16 nNewFlags = pStepBasic->StepPoint( nLine, nCol1, nCol2 );

        pInst->CalcBreakCallLevel( nNewFlags );
    }

    // break points only at STMNT-commands in a new line!
    else if( ( nOp1 != nOld )
        && ( nFlags & SbDEBUG_BREAK )
        && pMod->IsBP( static_cast<sal_uInt16>( nOp1 ) ) )
    {
        StarBASIC* pBreakBasic = GetCurrentBasic( &rBasic );
        sal_uInt16 nNewFlags = pBreakBasic->BreakPoint( nLine, nCol1, nCol2 );

        pInst->CalcBreakCallLevel( nNewFlags );
    }
}

// (+SvStreamFlags+Flags)
// Stack: block length
//        channel number
//        file name

void SbiRuntime::StepOPEN( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    SbxVariableRef pName = PopVar();
    SbxVariableRef pChan = PopVar();
    SbxVariableRef pLen  = PopVar();
    short nBlkLen = pLen->GetInteger();
    short nChan   = pChan->GetInteger();
    rtl::OString aName(rtl::OUStringToOString(pName->GetString(), osl_getThreadTextEncoding()));
    pIosys->Open( nChan, aName, static_cast<short>( nOp1 ),
        static_cast<short>( nOp2 ), nBlkLen );
    Error( pIosys->GetError() );
}

// create object (+StringID+StringID)

void SbiRuntime::StepCREATE( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    String aClass( pImg->GetString( static_cast<short>( nOp2 ) ) );
    SbxObject *pObj = SbxBase::CreateObject( aClass );
    if( !pObj )
        Error( SbERR_INVALID_OBJECT );
    else
    {
        String aName( pImg->GetString( static_cast<short>( nOp1 ) ) );
        pObj->SetName( aName );
    // the object must be able to call the BASIC
        pObj->SetParent( &rBasic );
        SbxVariable* pNew = new SbxVariable;
        pNew->PutObject( pObj );
        PushVar( pNew );
    }
}

void SbiRuntime::StepDCREATE( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    StepDCREATE_IMPL( nOp1, nOp2 );
}

void SbiRuntime::StepDCREATE_REDIMP( sal_uInt32 nOp1, sal_uInt32 nOp2 )
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

// #56204 create object array (+StringID+StringID), DCREATE == Dim-Create
void SbiRuntime::StepDCREATE_IMPL( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    SbxVariableRef refVar = PopVar();

    DimImpl( refVar );

    // fill the array with instances of the requested class
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

        short nDims = pArray->GetDims();
        sal_Int32 nTotalSize = 0;

        // must be a one-dimensional array
        sal_Int32 nLower, nUpper, nSize;
        sal_Int32 i;
        for( i = 0 ; i < nDims ; i++ )
        {
            pArray->GetDim32( i+1, nLower, nUpper );
            nSize = nUpper - nLower + 1;
            if( i == 0 )
                nTotalSize = nSize;
            else
                nTotalSize *= nSize;
        }

        // create objects and insert them into the array
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
                // the object must be able to call the basic
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
        bool bRangeError = false;

        // Store dims to use them for copying later
        sal_Int32* pLowerBounds = new sal_Int32[nDims];
        sal_Int32* pUpperBounds = new sal_Int32[nDims];
        sal_Int32* pActualIndices = new sal_Int32[nDims];
        if( nDimsOld != nDimsNew )
        {
            bRangeError = true;
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

// create object from user-type  (+StringID+StringID)

SbxObject* createUserTypeImpl( const String& rClassName );  // sb.cxx

void SbiRuntime::StepTCREATE( sal_uInt32 nOp1, sal_uInt32 nOp2 )
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

void SbiRuntime::implHandleSbxFlags( SbxVariable* pVar, SbxDataType t, sal_uInt32 nOp2 )
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
        sal_uInt16 nCount = static_cast<sal_uInt16>( nOp2 >> 17 );      // len = all bits above 0x10000
        String aStr;
        aStr.Fill( nCount, 0 );
        pVar->PutString( aStr );
    }

    bool bVarToDim = ((nOp2 & SBX_TYPE_VAR_TO_DIM_FLAG) != 0);
    if( bVarToDim )
        pVar->SetFlag( SBX_VAR_TO_DIM );
}

// establishing a local variable (+StringID+type)

void SbiRuntime::StepLOCAL( sal_uInt32 nOp1, sal_uInt32 nOp2 )
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

// establishing a module-global variable (+StringID+type)

void SbiRuntime::StepPUBLIC_Impl( sal_uInt32 nOp1, sal_uInt32 nOp2, bool bUsedForClassModule )
{
    String aName( pImg->GetString( static_cast<short>( nOp1 ) ) );
    SbxDataType t = (SbxDataType)(SbxDataType)(nOp2 & 0xffff);;
    sal_Bool bFlag = pMod->IsSet( SBX_NO_MODIFY );
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
        // from 2.7.1996: HACK because of 'reference can't be saved'
        pProp->SetFlag( SBX_NO_MODIFY);

        implHandleSbxFlags( pProp, t, nOp2 );
    }
}

void SbiRuntime::StepPUBLIC( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    StepPUBLIC_Impl( nOp1, nOp2, false );
}

void SbiRuntime::StepPUBLIC_P( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    // Creates module variable that isn't reinitialised when
    // between invocations ( for VBASupport & document basic only )
    if( pMod->pImage->bFirstInit )
    {
        bool bUsedForClassModule = pImg->GetFlag( SBIMG_CLASSMODULE );
        StepPUBLIC_Impl( nOp1, nOp2, bUsedForClassModule );
    }
}

// establishing a global variable (+StringID+type)

void SbiRuntime::StepGLOBAL( sal_uInt32 nOp1, sal_uInt32 nOp2 )
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

    sal_Bool bFlag = pStorage->IsSet( SBX_NO_MODIFY );
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
        // from 2.7.1996: HACK because of 'reference can't be saved'
        p->SetFlag( SBX_NO_MODIFY);
    }
}


// Creates global variable that isn't reinitialised when
// basic is restarted, P=PERSIST (+StringID+Typ)

void SbiRuntime::StepGLOBAL_P( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    if( pMod->pImage->bFirstInit )
    {
        StepGLOBAL( nOp1, nOp2 );
    }
}


// Searches for global variable, behavior depends on the fact
// if the variable is initialised for the first time

void SbiRuntime::StepFIND_G( sal_uInt32 nOp1, sal_uInt32 nOp2 )
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
// establishing a static variable (+StringID+type)
void SbiRuntime::StepSTATIC( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    String aName( pImg->GetString( static_cast<short>( nOp1 ) ) );
    SbxDataType t = (SbxDataType) nOp2;
    StepSTATIC_Impl( aName, t );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
