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

#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/DialogProvider.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/resource/XStringResourceSupplier.hpp>
#include <com/sun/star/resource/XStringResourceManager.hpp>
#include <com/sun/star/script/XEventAttacher.hpp>
#include <com/sun/star/script/XAllListener.hpp>
#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#include <com/sun/star/script/XScriptEventsAttacher.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/provider/theMasterScriptProviderFactory.hpp>
#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>

#include <basic/basicmanagerrepository.hxx>
#include <basic/basmgr.hxx>

#include <vcl/svapp.hxx>
#include <xmlscript/xmldlg_imexp.hxx>
#include <sbunoobj.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbuno.hxx>
#include <runtime.hxx>
#include <sbintern.hxx>
#include <eventatt.hxx>

#include <cppuhelper/implbase.hxx>
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::resource;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::reflection;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::cppu;
using namespace ::osl;


void SFURL_firing_impl( const ScriptEvent& aScriptEvent, Any* pRet, const Reference< frame::XModel >& xModel )
{
        SAL_INFO("basic", "Processing script url " << aScriptEvent.ScriptCode);
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
                Reference< XComponentContext > xContext(
                    comphelper::getProcessComponentContext() );
                Reference< provider::XScriptProviderFactory > xFactory =
                    provider::theMasterScriptProviderFactory::get( xContext );

                Any aCtx;
                aCtx <<= OUString("user");
                xScriptProvider.set( xFactory->createScriptProvider( aCtx ), UNO_QUERY );
            }

            if ( !xScriptProvider.is() )
            {
                SAL_INFO("basic", "Failed to create msp");
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
                SAL_INFO("basic", "Failed to Failed to obtain XScript");
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
            SAL_INFO("basic", "Caught RuntimeException reason " << re.Message);
        }
        catch ( const Exception& e )
        {
            SAL_INFO("basic", "Caught Exception reason " << e.Message);
        }

}


class BasicScriptListener_Impl : public WeakImplHelper< XScriptListener >
{
    StarBASICRef maBasicRef;
        Reference< frame::XModel > m_xModel;

    void firing_impl(const ScriptEvent& aScriptEvent, Any* pRet);

public:
    BasicScriptListener_Impl( StarBASIC* pBasic, const Reference< frame::XModel >& xModel )
        : maBasicRef( pBasic ), m_xModel( xModel ) {}

    // Methods of XAllListener
    virtual void SAL_CALL firing(const ScriptEvent& aScriptEvent) override;
    virtual Any SAL_CALL approveFiring(const ScriptEvent& aScriptEvent) override;

    // Methods of XEventListener
    virtual void SAL_CALL disposing(const EventObject& Source) override;
};

// Methods XAllListener
void BasicScriptListener_Impl::firing( const ScriptEvent& aScriptEvent )
{
    SolarMutexGuard g;

    firing_impl( aScriptEvent, nullptr );
}

Any BasicScriptListener_Impl::approveFiring( const ScriptEvent& aScriptEvent )
{
    SolarMutexGuard g;

    Any aRetAny;
    firing_impl( aScriptEvent, &aRetAny );
    return aRetAny;
}

// Methods XEventListener
void BasicScriptListener_Impl::disposing(const EventObject& )
{
    // TODO: ???
    //SolarMutexGuard aGuard;
    //xSbxObj.Clear();
}


void BasicScriptListener_Impl::firing_impl( const ScriptEvent& aScriptEvent, Any* pRet )
{
    if( aScriptEvent.ScriptType == "StarBasic" )
    {
        // Full qualified name?
        OUString aMacro( aScriptEvent.ScriptCode );
        OUString aLibName;
        OUString aLocation;
        if( comphelper::string::getTokenCount(aMacro, '.') == 3 )
        {
            sal_Int32 nLast = 0;
            OUString aFullLibName = aMacro.getToken( (sal_Int32)0, (sal_Unicode)'.', nLast );

            sal_Int32 nIndex = aFullLibName.indexOf( (sal_Unicode)':' );
            if (nIndex >= 0)
            {
                aLocation = aFullLibName.copy( 0, nIndex );
                aLibName = aFullLibName.copy( nIndex + 1 );
            }

            aMacro = aMacro.copy( nLast );
        }

        SbxObject* p = maBasicRef.get();
        SbxObject* pParent = p->GetParent();
        SbxObject* pParentParent = pParent ? pParent->GetParent() : nullptr;

        StarBASICRef xAppStandardBasic;
        StarBASICRef xDocStandardBasic;
        if( pParentParent )
        {
            // Own basic must be document library
            xAppStandardBasic = static_cast<StarBASIC*>(pParentParent);
            xDocStandardBasic = static_cast<StarBASIC*>(pParent);
        }
        else if( pParent )
        {
            OUString aName = p->GetName();
            if( aName == "Standard" )
            {
                // Own basic is doc standard lib
                xDocStandardBasic = static_cast<StarBASIC*>(p);
            }
            xAppStandardBasic = static_cast<StarBASIC*>(pParent);
        }
        else
        {
            xAppStandardBasic = static_cast<StarBASIC*>(p);
        }

        bool bSearchLib = true;
        StarBASICRef xLibSearchBasic;
        if( aLocation == "application" )
        {
            xLibSearchBasic = xAppStandardBasic;
        }
        else if( aLocation == "document" )
        {
            xLibSearchBasic = xDocStandardBasic;
        }
        else
        {
            bSearchLib = false;
        }
        SbxVariable* pMethVar = nullptr;
        // Be still tolerant and make default search if no search basic exists
        if( bSearchLib && xLibSearchBasic.is() )
        {
            sal_Int16 nCount = xLibSearchBasic->GetObjects()->Count();
            for( sal_Int16 nObj = -1; nObj < nCount ; nObj++ )
            {
                StarBASIC* pBasic;
                if( nObj == -1 )
                {
                    pBasic = xLibSearchBasic.get();
                }
                else
                {
                    SbxVariable* pVar = xLibSearchBasic->GetObjects()->Get( nObj );
                    pBasic = dynamic_cast<StarBASIC*>( pVar );
                }
                if( pBasic )
                {
                    OUString aName = pBasic->GetName();
                    if( aName == aLibName )
                    {
                        // Search only in the lib, not automatically in application basic
                        SbxFlagBits nFlags = pBasic->GetFlags();
                        pBasic->ResetFlag( SbxFlagBits::GlobalSearch );
                        pMethVar = pBasic->Find( aMacro, SbxClassType::DontCare );
                        pBasic->SetFlags( nFlags );
                        break;
                    }
                }
            }
        }

        // Default: Be tolerant and search everywhere
        if( (!pMethVar || nullptr == dynamic_cast<const SbMethod*>( pMethVar)) && maBasicRef.is() )
        {
            pMethVar = maBasicRef->FindQualified( aMacro, SbxClassType::DontCare );
        }
        SbMethod* pMeth = dynamic_cast<SbMethod*>( pMethVar );
        if( !pMeth )
        {
            return;
        }
        // Setup parameters
        SbxArrayRef xArray;
        sal_Int32 nCnt = aScriptEvent.Arguments.getLength();
        if( nCnt )
        {
            xArray = new SbxArray;
            const Any *pArgs = aScriptEvent.Arguments.getConstArray();
            for( sal_Int32 i = 0; i < nCnt; i++ )
            {
                SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
                unoToSbxValue( xVar.get(), pArgs[i] );
                xArray->Put( xVar.get(), sal::static_int_cast< sal_uInt16 >(i+1) );
            }
        }

        // Call method
        SbxVariableRef xValue = pRet ? new SbxVariable : nullptr;
        if( xArray.is() )
        {
            pMeth->SetParameters( xArray.get() );
        }
        pMeth->Call( xValue.get() );
        if( pRet )
        {
            *pRet = sbxToUnoValue( xValue.get() );
        }
        pMeth->SetParameters( nullptr );
    }
    else // scripting framework script
    {
        //callBasic via scripting framework
        SFURL_firing_impl( aScriptEvent, pRet, m_xModel );
    }
}

css::uno::Reference< css::container::XNameContainer > implFindDialogLibForDialog( const Any& rDlgAny, SbxObject* pBasic )
{
    css::uno::Reference< css::container::XNameContainer > aRetDlgLib;

    SbxVariable* pDlgLibContVar = pBasic->Find("DialogLibraries", SbxClassType::Object);
    if( pDlgLibContVar && nullptr != dynamic_cast<const SbUnoObject*>( pDlgLibContVar) )
    {
        SbUnoObject* pDlgLibContUnoObj = static_cast<SbUnoObject*>(static_cast<SbxBase*>(pDlgLibContVar));
        Any aDlgLibContAny = pDlgLibContUnoObj->getUnoAny();

        Reference< XLibraryContainer > xDlgLibContNameAccess( aDlgLibContAny, UNO_QUERY );
        OSL_ENSURE( xDlgLibContNameAccess.is(), "implFindDialogLibForDialog: no lib container for the given dialog!" );
        if( xDlgLibContNameAccess.is() )
        {
            Sequence< OUString > aLibNames = xDlgLibContNameAccess->getElementNames();
            const OUString* pLibNames = aLibNames.getConstArray();
            sal_Int32 nLibNameCount = aLibNames.getLength();

            for( sal_Int32 iLib = 0 ; iLib < nLibNameCount ; iLib++ )
            {
                if ( !xDlgLibContNameAccess->isLibraryLoaded( pLibNames[ iLib ] ) )
                    // if the library isn't loaded, then the dialog cannot originate from this lib
                    continue;

                Any aDlgLibAny = xDlgLibContNameAccess->getByName( pLibNames[ iLib ] );

                Reference< XNameContainer > xDlgLibNameCont( aDlgLibAny, UNO_QUERY );
                OSL_ENSURE( xDlgLibNameCont.is(), "implFindDialogLibForDialog: invalid dialog lib!" );
                if( xDlgLibNameCont.is() )
                {
                    Sequence< OUString > aDlgNames = xDlgLibNameCont->getElementNames();
                    const OUString* pDlgNames = aDlgNames.getConstArray();
                    sal_Int32 nDlgNameCount = aDlgNames.getLength();

                    for( sal_Int32 iDlg = 0 ; iDlg < nDlgNameCount ; iDlg++ )
                    {
                        Any aDlgAny = xDlgLibNameCont->getByName( pDlgNames[ iDlg ] );
                        if( aDlgAny == rDlgAny )
                        {
                            aRetDlgLib = xDlgLibNameCont;
                            break;
                        }
                    }
                }
            }
        }
    }

    return aRetDlgLib;
}

css::uno::Reference< css::container::XNameContainer > implFindDialogLibForDialogBasic( const Any& aAnyISP, SbxObject* pBasic, StarBASIC*& pFoundBasic )
{
    css::uno::Reference< css::container::XNameContainer > aDlgLib;
    // Find dialog library for dialog, direct access is not possible here
    StarBASIC* pStartedBasic = static_cast<StarBASIC*>(pBasic);
    SbxObject* pParentBasic = pStartedBasic ? pStartedBasic->GetParent() : nullptr;
    SbxObject* pParentParentBasic = pParentBasic ? pParentBasic->GetParent() : nullptr;

    SbxObject* pSearchBasic1 = nullptr;
    SbxObject* pSearchBasic2 = nullptr;
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
        aDlgLib = implFindDialogLibForDialog( aAnyISP, pSearchBasic1 );

        if ( aDlgLib.is() )
            pFoundBasic = static_cast<StarBASIC*>(pSearchBasic1);

        else if( pSearchBasic2 )
        {
            aDlgLib = implFindDialogLibForDialog( aAnyISP, pSearchBasic2 );
            if ( aDlgLib.is() )
                pFoundBasic = static_cast<StarBASIC*>(pSearchBasic2);
        }
    }
    return aDlgLib;
}

void RTL_Impl_CreateUnoDialog( StarBASIC* pBasic, SbxArray& rPar, bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );

    // We need at least 1 parameter
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    // Get dialog
    SbxBaseRef pObj = rPar.Get( 1 )->GetObject();
    if( !(pObj.is() && nullptr != dynamic_cast<const SbUnoObject*>( pObj.get() )) )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    SbUnoObject* pUnoObj = static_cast<SbUnoObject*>(pObj.get());
    Any aAnyISP = pUnoObj->getUnoAny();
    TypeClass eType = aAnyISP.getValueType().getTypeClass();

    if( eType != TypeClass_INTERFACE )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    // Create new uno dialog
    Reference< XNameContainer > xDialogModel( xContext->getServiceManager()->createInstanceWithContext(
                      "com.sun.star.awt.UnoControlDialogModel", xContext), UNO_QUERY );
    if( !xDialogModel.is() )
    {
        return;
    }
    Reference< XInputStreamProvider > xISP;
    aAnyISP >>= xISP;
    if( !xISP.is() )
    {
        return;
    }

    // Import the DialogModel
    Reference< XInputStream > xInput( xISP->createInputStream() );

    // i83963 Force decoration
    uno::Reference< beans::XPropertySet > xDlgModPropSet( xDialogModel, uno::UNO_QUERY );
    if( xDlgModPropSet.is() )
    {
        try
        {
            bool bDecoration = true;
            OUString aDecorationPropName("Decoration");
            Any aDecorationAny = xDlgModPropSet->getPropertyValue( aDecorationPropName );
            aDecorationAny >>= bDecoration;
            if( !bDecoration )
            {
                xDlgModPropSet->setPropertyValue( aDecorationPropName, makeAny( true ) );
                xDlgModPropSet->setPropertyValue( "Title", makeAny( OUString() ) );
            }
        }
        catch(const UnknownPropertyException& )
        {}
    }

    css::uno::Reference< css::container::XNameContainer > aDlgLib;
    bool bDocDialog = false;
    StarBASIC* pFoundBasic = nullptr;
    SAL_INFO("basic", "About to try get a hold of ThisComponent");
    Reference< frame::XModel > xModel = StarBASIC::GetModelFromBasic( GetSbData()->pInst->GetBasic() ) ;
    aDlgLib = implFindDialogLibForDialogBasic( aAnyISP, GetSbData()->pInst->GetBasic(), pFoundBasic );
    // If we found the dialog then it belongs to the Search basic
    if ( !pFoundBasic )
    {
        Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create( xContext );
        Reference< container::XEnumeration > xModels;
        Reference< container::XEnumerationAccess > xComponents( xDesktop->getComponents(), UNO_QUERY );
        if ( xComponents.is() )
        {
            xModels.set( xComponents->createEnumeration(), UNO_QUERY );
        }
        if ( xModels.is() )
        {
            while ( xModels->hasMoreElements() )
            {
                Reference< frame::XModel > xNextModel( xModels->nextElement(), UNO_QUERY );
                if ( xNextModel.is() )
                {
                    BasicManager* pMgr = basic::BasicManagerRepository::getDocumentBasicManager( xNextModel );
                    if ( pMgr )
                    {
                        aDlgLib = implFindDialogLibForDialogBasic( aAnyISP, pMgr->GetLib(0), pFoundBasic );
                    }
                    if ( aDlgLib.is() )
                    {
                        bDocDialog = true;
                        xModel = xNextModel;
                        break;
                    }
                }
            }
        }
    }
    if ( pFoundBasic )
    {
        bDocDialog = pFoundBasic->IsDocBasic();
    }
    Reference< XScriptListener > xScriptListener = new BasicScriptListener_Impl( GetSbData()->pInst->GetBasic(), xModel );

    // Create a "living" Dialog
    Reference< XControl > xCntrl;
    try
    {
       Reference< XDialogProvider >  xDlgProv;
       if( bDocDialog )
           xDlgProv = css::awt::DialogProvider::createWithModelAndScripting( xContext, xModel, xInput, aDlgLib, xScriptListener );
       else
           xDlgProv = css::awt::DialogProvider::createWithModelAndScripting( xContext, uno::Reference< frame::XModel >(), xInput, aDlgLib, xScriptListener );

       xCntrl.set( xDlgProv->createDialog(OUString() ), UNO_QUERY_THROW );
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
    unoToSbxValue( refVar.get(), aRetVal );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
