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


#include <osl/mutex.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <com/sun/star/script/XEventAttacher.hpp>
#include <com/sun/star/script/XAllListener.hpp>
#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#include <com/sun/star/script/XScriptEventsAttacher.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/resource/XStringResourceSupplier.hpp>
#include <com/sun/star/resource/XStringResourceManager.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/script/provider/XScriptProviderFactory.hpp>

#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/awt/XDialogProvider.hpp>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <basic/basicmanagerrepository.hxx>
#include <basic/basmgr.hxx>
//==================================================================================================

#include <xmlscript/xmldlg_imexp.hxx>
#include <sbunoobj.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbuno.hxx>
#include <runtime.hxx>
#include <sbintern.hxx>


#include <cppuhelper/implbase1.hxx>
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::resource;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::reflection;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::cppu;
using namespace ::osl;


void SFURL_firing_impl( const ScriptEvent& aScriptEvent, Any* pRet, const Reference< frame::XModel >& xModel )
{
        OSL_TRACE("SFURL_firing_impl() processing script url %s",
            ::rtl::OUStringToOString( aScriptEvent.ScriptCode,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        try
        {
            Reference< provider::XScriptProvider > xScriptProvider;
            if ( xModel.is() )
            {
                Reference< provider::XScriptProviderSupplier > xSupplier( xModel, UNO_QUERY );
                OSL_ENSURE( xSupplier.is(), "SFURL_firing_impl: failed to get script provider supplier" );
                if ( xSupplier.is() )
                    xScriptProvider.set( xSupplier->getScriptProvider() );
            }
            else
            {
                Reference< XComponentContext > xContext;
                Reference< XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), UNO_QUERY );
                OSL_ASSERT( xProps.is() );
                OSL_VERIFY( xProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")) ) >>= xContext );
                if ( xContext.is() )
                {
                    Reference< provider::XScriptProviderFactory > xFactory(
                        xContext->getValueByName(
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.script.provider.theMasterScriptProviderFactory")) ),
                        UNO_QUERY );
                    OSL_ENSURE( xFactory.is(), "SFURL_firing_impl: failed to get master script provider factory" );
                    if ( xFactory.is() )
                    {
                        Any aCtx;
                        aCtx <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("user"));
                        xScriptProvider.set( xFactory->createScriptProvider( aCtx ), UNO_QUERY );
                    }
                }
            }

            if ( !xScriptProvider.is() )
            {
                OSL_TRACE("SFURL_firing_impl() Failed to create msp");
                return;
            }
            Sequence< Any > inArgs( 0 );
            Sequence< Any > outArgs( 0 );
            Sequence< sal_Int16 > outIndex;

            // get Arguments for script
            inArgs = aScriptEvent.Arguments;

            Reference< provider::XScript > xScript = xScriptProvider->getScript( aScriptEvent.ScriptCode );

            if ( !xScript.is() )
            {
                OSL_TRACE("SFURL_firing_impl() Failed to obtain XScript");
                return;
            }

            Any result = xScript->invoke( inArgs, outIndex, outArgs );
            if ( pRet )
            {
                *pRet = result;
            }
        }
        catch ( const RuntimeException& re )
        {
            OSL_TRACE("SFURL_firing_impl() Caught RuntimeException reason %s.",
                ::rtl::OUStringToOString( re.Message,
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }
        catch ( const Exception& e )
        {
            OSL_TRACE("SFURL_firing_impl() Caught Exception reason %s.",
                ::rtl::OUStringToOString( e.Message,
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }

}


class BasicScriptListener_Impl : public WeakImplHelper1< XScriptListener >
{
    StarBASICRef maBasicRef;
        Reference< frame::XModel > m_xModel;

    virtual void firing_impl(const ScriptEvent& aScriptEvent, Any* pRet);

public:
    BasicScriptListener_Impl( StarBASIC* pBasic, const Reference< frame::XModel >& xModel )
        : maBasicRef( pBasic ), m_xModel( xModel ) {}

    // Methods of XAllListener
    virtual void SAL_CALL firing(const ScriptEvent& aScriptEvent)
        throw( RuntimeException );
    virtual Any SAL_CALL approveFiring(const ScriptEvent& aScriptEvent)
        throw( InvocationTargetException, RuntimeException );

    // Methods of XEventListener
    virtual void SAL_CALL disposing(const EventObject& Source)
        throw( RuntimeException );
};

// Methods XAllListener
void BasicScriptListener_Impl::firing( const ScriptEvent& aScriptEvent ) throw ( RuntimeException )
{
    firing_impl( aScriptEvent, NULL );
}

Any BasicScriptListener_Impl::approveFiring( const ScriptEvent& aScriptEvent )
    throw ( InvocationTargetException, RuntimeException )
{
    Any aRetAny;
    firing_impl( aScriptEvent, &aRetAny );
    return aRetAny;
}

// Methods XEventListener
void BasicScriptListener_Impl::disposing(const EventObject& ) throw ( RuntimeException )
{
    // TODO: ???
    //SolarMutexGuard aGuard;
    //xSbxObj.Clear();
}


void BasicScriptListener_Impl::firing_impl( const ScriptEvent& aScriptEvent, Any* pRet )
{
    if( aScriptEvent.ScriptType.compareToAscii( "StarBasic" ) == 0 )
    {
        // Full qualified name?
        String aMacro( aScriptEvent.ScriptCode );
        String aLibName;
        String aLocation;
        if( comphelper::string::getTokenCount(aMacro, '.') == 3 )
        {
            sal_uInt16 nLast = 0;
            ::rtl::OUString aFullLibName = aMacro.GetToken( 0, '.', nLast );

            sal_Int32 nIndex = aFullLibName.indexOf( (sal_Unicode)':' );
            if (nIndex >= 0)
            {
                aLocation = aFullLibName.copy( 0, nIndex );
                aLibName = aFullLibName.copy( nIndex + 1 );
            }

            String aModul = aMacro.GetToken( 0, '.', nLast );
            aMacro.Erase( 0, nLast );
        }

        SbxObject* p = maBasicRef;
        SbxObject* pParent = p->GetParent();
        SbxObject* pParentParent = pParent ? pParent->GetParent() : NULL;

        StarBASICRef xAppStandardBasic;
        StarBASICRef xDocStandardBasic;
        if( pParentParent )
        {
            // Own basic must be document library
            xAppStandardBasic = (StarBASIC*)pParentParent;
            xDocStandardBasic = (StarBASIC*)pParent;
        }
        else if( pParent )
        {
            String aName = p->GetName();
            if( aName.EqualsAscii("Standard") )
            {
                // Own basic is doc standard lib
                xDocStandardBasic = (StarBASIC*)p;
            }
            xAppStandardBasic = (StarBASIC*)pParent;
        }
        else
        {
            xAppStandardBasic = (StarBASIC*)p;
        }

        bool bSearchLib = true;
        StarBASICRef xLibSearchBasic;
        if( aLocation.EqualsAscii("application") )
            xLibSearchBasic = xAppStandardBasic;
        else if( aLocation.EqualsAscii("document") )
            xLibSearchBasic = xDocStandardBasic;
        else
            bSearchLib = false;

        SbxVariable* pMethVar = NULL;
        // Be still tolerant and make default search if no search basic exists
        if( bSearchLib && xLibSearchBasic.Is() )
        {
            StarBASICRef xLibBasic;
            sal_Int16 nCount = xLibSearchBasic->GetObjects()->Count();
            for( sal_Int16 nObj = -1; nObj < nCount ; nObj++ )
            {
                StarBASIC* pBasic;
                if( nObj == -1 )
                {
                    pBasic = (StarBASIC*)xLibSearchBasic;
                }
                else
                {
                    SbxVariable* pVar = xLibSearchBasic->GetObjects()->Get( nObj );
                    pBasic = PTR_CAST(StarBASIC,pVar);
                }
                if( pBasic )
                {
                    String aName = pBasic->GetName();
                    if( aName == aLibName )
                    {
                        // Search only in the lib, not automatically in application basic
                        sal_uInt16 nFlags = pBasic->GetFlags();
                        pBasic->ResetFlag( SBX_GBLSEARCH );
                        pMethVar = pBasic->Find( aMacro, SbxCLASS_DONTCARE );
                        pBasic->SetFlags( nFlags );
                        break;
                    }
                }
            }
        }

        // Default: Be tolerant and search everywhere
        if( (!pMethVar || !pMethVar->ISA(SbMethod)) && maBasicRef.Is() )
            pMethVar = maBasicRef->FindQualified( aMacro, SbxCLASS_DONTCARE );

        SbMethod* pMeth = PTR_CAST(SbMethod,pMethVar);
        if( !pMeth )
            return;

        // Setup parameters
        SbxArrayRef xArray;
        String aTmp;
        sal_Int32 nCnt = aScriptEvent.Arguments.getLength();
        if( nCnt )
        {
            xArray = new SbxArray;
            const Any *pArgs = aScriptEvent.Arguments.getConstArray();
            for( sal_Int32 i = 0; i < nCnt; i++ )
            {
                SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
                unoToSbxValue( (SbxVariable*)xVar, pArgs[i] );
                xArray->Put( xVar, sal::static_int_cast< sal_uInt16 >(i+1) );
            }
        }

        // Call method
        SbxVariableRef xValue = pRet ? new SbxVariable : 0;
        if( xArray.Is() )
            pMeth->SetParameters( xArray );
        pMeth->Call( xValue );
        if( pRet )
            *pRet = sbxToUnoValue( xValue );
        pMeth->SetParameters( NULL );
    }
        else // scripting framework script
        {
            //callBasic via scripting framework
            SFURL_firing_impl( aScriptEvent, pRet, m_xModel );

        }
}

Any implFindDialogLibForDialog( const Any& rDlgAny, SbxObject* pBasic )
{
    Any aRetDlgLibAny;

    SbxVariable* pDlgLibContVar = pBasic->Find(rtl::OUString("DialogLibraries"), SbxCLASS_OBJECT);
    if( pDlgLibContVar && pDlgLibContVar->ISA(SbUnoObject) )
    {
        SbUnoObject* pDlgLibContUnoObj = (SbUnoObject*)(SbxBase*)pDlgLibContVar;
        Any aDlgLibContAny = pDlgLibContUnoObj->getUnoAny();

        Reference< XLibraryContainer > xDlgLibContNameAccess( aDlgLibContAny, UNO_QUERY );
        OSL_ENSURE( xDlgLibContNameAccess.is(), "implFindDialogLibForDialog: no lib container for the given dialog!" );
        if( xDlgLibContNameAccess.is() )
        {
            Sequence< ::rtl::OUString > aLibNames = xDlgLibContNameAccess->getElementNames();
            const ::rtl::OUString* pLibNames = aLibNames.getConstArray();
            sal_Int32 nLibNameCount = aLibNames.getLength();

            for( sal_Int32 iLib = 0 ; iLib < nLibNameCount ; iLib++ )
            {
                if ( !xDlgLibContNameAccess->isLibraryLoaded( pLibNames[ iLib ] ) )
                    // if the library isn't loaded, then the dialog cannot originate from this lib
                    continue;

                Any aDlgLibAny = xDlgLibContNameAccess->getByName( pLibNames[ iLib ] );

                Reference< XNameAccess > xDlgLibNameAccess( aDlgLibAny, UNO_QUERY );
                OSL_ENSURE( xDlgLibNameAccess.is(), "implFindDialogLibForDialog: invalid dialog lib!" );
                if( xDlgLibNameAccess.is() )
                {
                    Sequence< ::rtl::OUString > aDlgNames = xDlgLibNameAccess->getElementNames();
                    const ::rtl::OUString* pDlgNames = aDlgNames.getConstArray();
                    sal_Int32 nDlgNameCount = aDlgNames.getLength();

                    for( sal_Int32 iDlg = 0 ; iDlg < nDlgNameCount ; iDlg++ )
                    {
                        Any aDlgAny = xDlgLibNameAccess->getByName( pDlgNames[ iDlg ] );
                        if( aDlgAny == rDlgAny )
                        {
                            aRetDlgLibAny = aDlgLibAny;
                            break;
                        }
                    }
                }
            }
        }
    }

    return aRetDlgLibAny;
}

Any implFindDialogLibForDialogBasic( const Any& aAnyISP, SbxObject* pBasic, StarBASIC*& pFoundBasic )
{
    Any aDlgLibAny;
    // Find dialog library for dialog, direct access is not possible here
    StarBASIC* pStartedBasic = (StarBASIC*)pBasic;
    SbxObject* pParentBasic = pStartedBasic ? pStartedBasic->GetParent() : NULL;
    SbxObject* pParentParentBasic = pParentBasic ? pParentBasic->GetParent() : NULL;

    SbxObject* pSearchBasic1 = NULL;
    SbxObject* pSearchBasic2 = NULL;
    if( pParentParentBasic )
    {
        pSearchBasic1 = pParentBasic;
        pSearchBasic2 = pParentParentBasic;
    }
    else
    {
        pSearchBasic1 = pStartedBasic;
        pSearchBasic2 = pParentBasic;
    }
    if( pSearchBasic1 )
    {
        aDlgLibAny = implFindDialogLibForDialog( aAnyISP, pSearchBasic1 );

        if ( aDlgLibAny.hasValue() )
            pFoundBasic = (StarBASIC*)pSearchBasic1;

        else if( pSearchBasic2 )
        {
            aDlgLibAny = implFindDialogLibForDialog( aAnyISP, pSearchBasic2 );
            if ( aDlgLibAny.hasValue() )
                pFoundBasic = (StarBASIC*)pSearchBasic2;
        }
    }
    return aDlgLibAny;
}

void RTL_Impl_CreateUnoDialog( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    Reference< XMultiServiceFactory > xMSF( comphelper::getProcessServiceFactory() );
    if( !xMSF.is() )
        return;

    // We need at least 1 parameter
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // Get dialog
    SbxBaseRef pObj = (SbxBase*)rPar.Get( 1 )->GetObject();
    if( !(pObj && pObj->ISA(SbUnoObject)) )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    SbUnoObject* pUnoObj = (SbUnoObject*)(SbxBase*)pObj;
    Any aAnyISP = pUnoObj->getUnoAny();
    TypeClass eType = aAnyISP.getValueType().getTypeClass();

    if( eType != TypeClass_INTERFACE )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // Create new uno dialog
    Reference< XNameContainer > xDialogModel( xMSF->createInstance
        ( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialogModel" ) ) ),
            UNO_QUERY );
    if( !xDialogModel.is() )
        return;

    Reference< XInputStreamProvider > xISP;
    aAnyISP >>= xISP;
    if( !xISP.is() )
        return;

    Reference< XComponentContext > xContext;
    Reference< XPropertySet > xProps( xMSF, UNO_QUERY );
    OSL_ASSERT( xProps.is() );
    OSL_VERIFY( xProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")) ) >>= xContext );

    // Import the DialogModel
    Reference< XInputStream > xInput( xISP->createInputStream() );

    // i83963 Force decoration
    uno::Reference< beans::XPropertySet > xDlgModPropSet( xDialogModel, uno::UNO_QUERY );
    if( xDlgModPropSet.is() )
    {
        bool bDecoration = true;
        try
        {
            ::rtl::OUString aDecorationPropName(RTL_CONSTASCII_USTRINGPARAM("Decoration"));
            Any aDecorationAny = xDlgModPropSet->getPropertyValue( aDecorationPropName );
            aDecorationAny >>= bDecoration;
            if( !bDecoration )
            {
                xDlgModPropSet->setPropertyValue( aDecorationPropName, makeAny( true ) );

                ::rtl::OUString aTitlePropName(RTL_CONSTASCII_USTRINGPARAM("Title"));
                xDlgModPropSet->setPropertyValue( aTitlePropName, makeAny( ::rtl::OUString() ) );
            }
        }
        catch(const UnknownPropertyException& )
        {}
    }

    Any aDlgLibAny;
    bool bDocDialog = false;
    StarBASIC* pFoundBasic = NULL;
    OSL_TRACE("About to try get a hold of ThisComponent");
    Reference< frame::XModel > xModel = StarBASIC::GetModelFromBasic( GetSbData()->pInst->GetBasic() ) ;
    aDlgLibAny = implFindDialogLibForDialogBasic( aAnyISP, GetSbData()->pInst->GetBasic(), pFoundBasic );
    // If we found the dialog then it belongs to the Search basic
    if ( !pFoundBasic )
    {
        Reference< frame::XDesktop > xDesktop( xMSF->createInstance
    ( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ) ) ),
        UNO_QUERY );
        Reference< container::XEnumeration > xModels;
        if ( xDesktop.is() )
        {
            Reference< container::XEnumerationAccess > xComponents( xDesktop->getComponents(), UNO_QUERY );
            if ( xComponents.is() )
                xModels.set( xComponents->createEnumeration(), UNO_QUERY );
            if ( xModels.is() )
            {
                while ( xModels->hasMoreElements() )
                {
                    Reference< frame::XModel > xNextModel( xModels->nextElement(), UNO_QUERY );
                    if ( xNextModel.is() )
                    {
                        BasicManager* pMgr = basic::BasicManagerRepository::getDocumentBasicManager( xNextModel );
                        if ( pMgr )
                            aDlgLibAny = implFindDialogLibForDialogBasic( aAnyISP, pMgr->GetLib(0), pFoundBasic );
                        if ( aDlgLibAny.hasValue() )
                        {
                            bDocDialog = true;
                            xModel = xNextModel;
                            break;
                        }
                    }
                }
            }
        }
    }
    if ( pFoundBasic )
        bDocDialog = pFoundBasic->IsDocBasic();
    Reference< XScriptListener > xScriptListener = new BasicScriptListener_Impl( GetSbData()->pInst->GetBasic(), xModel );

    Sequence< Any > aArgs( 4 );
    if( bDocDialog )
       aArgs[ 0 ] <<= xModel;
    else
       aArgs[ 0 ] <<= uno::Reference< uno::XInterface >();
    aArgs[ 1 ] <<= xInput;
    aArgs[ 2 ] = aDlgLibAny;
    aArgs[ 3 ] <<= xScriptListener;
    // Create a "living" Dialog
    Reference< XControl > xCntrl;
    try
    {
        Reference< XDialogProvider >  xDlgProv( xMSF->createInstanceWithArguments( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.scripting.DialogProvider" ) ), aArgs ), UNO_QUERY );
        xCntrl.set( xDlgProv->createDialog( rtl::OUString() ), UNO_QUERY_THROW );
       // Add dialog model to dispose vector
       Reference< XComponent > xDlgComponent( xCntrl->getModel(), UNO_QUERY );
       GetSbData()->pInst->getComponentVector().push_back( xDlgComponent );
       // need ThisCompoent from calling script
    }
    // preserve existing bad behaviour, it's possible... but probably
    // illegal to open 2 dialogs ( they ARE modal ) when this happens, sometimes
    // create dialog fails.  So, in this case let's not throw, just leave basic
    // detect the unset object.
    catch(const uno::Exception& )
    {
    }

    // Return dialog
    Any aRetVal;
    aRetVal <<= xCntrl;
    SbxVariableRef refVar = rPar.Get(0);
    unoToSbxValue( (SbxVariable*)refVar, aRetVal );
}


//===================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
