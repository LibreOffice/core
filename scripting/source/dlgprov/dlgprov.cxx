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


#include "dlgprov.hxx"
#include "dlgevtatt.hxx"
#include <com/sun/star/awt/UnoControlDialog.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/beans/theIntrospection.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/io/XInputStreamProvider.hpp>
#include <com/sun/star/resource/XStringResourceSupplier.hpp>
#include <com/sun/star/resource/XStringResourceManager.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#include <com/sun/star/uri/XVndSunStarExpandUrl.hpp>
#include <com/sun/star/util/theMacroExpander.hpp>

#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sfx2/app.hxx>
#include <xmlscript/xmldlg_imexp.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <util/MiscUtils.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <i18nlangtag/languagetag.hxx>

using namespace ::com::sun::star;
using namespace awt;
using namespace lang;
using namespace uno;
using namespace script;
using namespace beans;
using namespace document;
using namespace ::sf_misc;

namespace dlgprov
{

    Reference< resource::XStringResourceManager > lcl_getStringResourceManager(const Reference< XComponentContext >& i_xContext, std::u16string_view i_sURL)
    {
        INetURLObject aInetObj( i_sURL );
        OUString aDlgName = aInetObj.GetBase();
        aInetObj.removeSegment();
        OUString aDlgLocation = aInetObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
        css::lang::Locale aLocale = Application::GetSettings().GetUILanguageTag().getLocale();

        Reference< task::XInteractionHandler > xDummyHandler;

        Sequence<Any> aArgs{ Any(aDlgLocation),
                             Any(true), // bReadOnly
                             Any(aLocale),
                             Any(aDlgName),
                             Any(OUString()),
                             Any(xDummyHandler) };

        Reference< XMultiComponentFactory > xSMgr_( i_xContext->getServiceManager(), UNO_SET_THROW );
        // TODO: Ctor
        Reference< resource::XStringResourceManager > xStringResourceManager( xSMgr_->createInstanceWithContext
            ( "com.sun.star.resource.StringResourceWithLocation",
                i_xContext ), UNO_QUERY );
        if( xStringResourceManager.is() )
        {
            Reference< XInitialization > xInit( xStringResourceManager, UNO_QUERY );
            if( xInit.is() )
                xInit->initialize( aArgs );
        }
        return xStringResourceManager;
    }
    Reference< container::XNameContainer > lcl_createControlModel(const Reference< XComponentContext >& i_xContext)
    {
        Reference< XMultiComponentFactory > xSMgr_( i_xContext->getServiceManager(), UNO_SET_THROW );
        Reference< container::XNameContainer > xControlModel( xSMgr_->createInstanceWithContext("com.sun.star.awt.UnoControlDialogModel", i_xContext ), UNO_QUERY_THROW );
        return xControlModel;
    }
    Reference< container::XNameContainer > lcl_createDialogModel( const Reference< XComponentContext >& i_xContext,
        const Reference< io::XInputStream >& xInput,
        const Reference< frame::XModel >& xModel,
        const Reference< resource::XStringResourceManager >& xStringResourceManager,
        const Any &aDialogSourceURL)
    {
        Reference< container::XNameContainer > xDialogModel(  lcl_createControlModel(i_xContext) );

        Reference< beans::XPropertySet > xDlgPropSet( xDialogModel, UNO_QUERY );
        xDlgPropSet->setPropertyValue( "DialogSourceURL", aDialogSourceURL );

        // #TODO we really need to detect the source of the Dialog, is it
        // the dialog. E.g. if the dialog was created from basic ( then we just
        // can't tell  where its from )
        // If we are happy to always substitute the form model for the awt
        // one then maybe the presence of a document model is enough to trigger
        // swapping out the models ( or perhaps we only want to do this
        // for vba mode ) there are a number of feasible and valid possibilities
        ::xmlscript::importDialogModel( xInput, xDialogModel, i_xContext, xModel );

        // Set resource property
        if( xStringResourceManager.is() )
        {
            Reference< beans::XPropertySet > xDlgPSet( xDialogModel, UNO_QUERY );
            Any aStringResourceManagerAny;
            aStringResourceManagerAny <<= xStringResourceManager;
            xDlgPSet->setPropertyValue( "ResourceResolver", aStringResourceManagerAny );
        }

        return xDialogModel;
    }

    // mutex


    ::osl::Mutex& getMutex()
    {
        static ::osl::Mutex s_aMutex;

        return s_aMutex;
    }


    // DialogProviderImpl


    DialogProviderImpl::DialogProviderImpl( const Reference< XComponentContext >& rxContext )
        :m_xContext( rxContext )
    {
    }


    DialogProviderImpl::~DialogProviderImpl()
    {
    }


    static Reference< resource::XStringResourceManager > getStringResourceFromDialogLibrary
        ( const Reference< container::XNameContainer >& xDialogLib )
    {
        Reference< resource::XStringResourceManager > xStringResourceManager;
        if( xDialogLib.is() )
        {
            Reference< resource::XStringResourceSupplier > xStringResourceSupplier( xDialogLib, UNO_QUERY );
            if( xStringResourceSupplier.is() )
            {
                Reference< resource::XStringResourceResolver >
                    xStringResourceResolver = xStringResourceSupplier->getStringResource();

                xStringResourceManager =
                    Reference< resource::XStringResourceManager >( xStringResourceResolver, UNO_QUERY );
            }
        }
        return xStringResourceManager;
    }

    Reference< container::XNameContainer > DialogProviderImpl::createDialogModel(
        const Reference< io::XInputStream >& xInput,
        const Reference< resource::XStringResourceManager >& xStringResourceManager,
        const Any &aDialogSourceURL)
    {
        return lcl_createDialogModel(m_xContext,xInput,m_xModel,xStringResourceManager,aDialogSourceURL);
    }

    Reference< XControlModel > DialogProviderImpl::createDialogModelForBasic()
    {
        if (!m_BasicInfo)
            // shouldn't get here
            throw RuntimeException("No information to create dialog" );
        Reference< resource::XStringResourceManager > xStringResourceManager = getStringResourceFromDialogLibrary( m_BasicInfo->mxDlgLib );

        Any aDialogSourceURL((OUString()));
        Reference< XControlModel > xCtrlModel( createDialogModel( m_BasicInfo->mxInput, xStringResourceManager, aDialogSourceURL ), UNO_QUERY_THROW );
        return xCtrlModel;
    }

    Reference< XControlModel > DialogProviderImpl::createDialogModel( const OUString& sURL )
    {

        OUString aURL( sURL );

        // parse URL
        // TODO: use URL parsing class
        // TODO: decoding of location

        Reference< uri::XUriReferenceFactory > xFac ( uri::UriReferenceFactory::create( m_xContext )  );

        // i75778: Support non-script URLs
        Reference< io::XInputStream > xInput;
        Reference< container::XNameContainer > xDialogLib;

        // Accept file URL to single dialog
        bool bSingleDialog = false;

        Reference< util::XMacroExpander > xMacroExpander =
            util::theMacroExpander::get(m_xContext);

        Reference< uri::XUriReference > uriRef;
        for (;;)
        {
            uriRef = xFac->parse( aURL );
            if ( !uriRef.is() )
            {
                OUString errorMsg = "DialogProviderImpl::getDialogModel: failed to parse URI: " + aURL;
                throw IllegalArgumentException( errorMsg, Reference< XInterface >(), 1 );
            }
            Reference < uri::XVndSunStarExpandUrl > sxUri( uriRef, UNO_QUERY );
            if( !sxUri.is() )
                break;

            aURL = sxUri->expand( xMacroExpander );
        }

        Reference < uri::XVndSunStarScriptUrl > sfUri( uriRef, UNO_QUERY );
        if( !sfUri.is() )
        {
            bSingleDialog = true;

            // Try any other URL with SimpleFileAccess
            Reference< ucb::XSimpleFileAccess3 > xSFI = ucb::SimpleFileAccess::create(m_xContext);

            try
            {
                xInput = xSFI->openFileRead( aURL );
            }
            catch( Exception& )
            {}
        }
        else
        {
            OUString sDescription = sfUri->getName();

            sal_Int32 nIndex = 0;

            OUString sLibName = sDescription.getToken( 0, '.', nIndex );
            OUString sDlgName;
            if ( nIndex != -1 )
                sDlgName = sDescription.getToken( 0, '.', nIndex );

            OUString sLocation = sfUri->getParameter( "location" );


            // get dialog library container
            // TODO: dialogs in packages
            Reference< XLibraryContainer > xLibContainer;

            if ( sLocation == "application" )
            {
                xLibContainer = SfxGetpApp()->GetDialogContainer();
            }
            else if ( sLocation == "document" )
            {
                Reference< XEmbeddedScripts > xDocumentScripts( m_xModel, UNO_QUERY );
                if ( xDocumentScripts.is() )
                {
                    xLibContainer = xDocumentScripts->getDialogLibraries();
                    OSL_ENSURE( xLibContainer.is(),
                        "DialogProviderImpl::createDialogModel: invalid dialog container!" );
                }
            }
            else
            {
                const Sequence< OUString > aOpenDocsTdocURLs( MiscUtils::allOpenTDocUrls( m_xContext ) );
                for ( auto const & tdocURL : aOpenDocsTdocURLs )
                {
                    Reference< frame::XModel > xModel( MiscUtils::tDocUrlToModel( tdocURL ) );
                    OSL_ENSURE( xModel.is(), "DialogProviderImpl::createDialogModel: invalid document model!" );
                    if ( !xModel.is() )
                        continue;

                    OUString sDocURL = xModel->getURL();
                    if ( sDocURL.isEmpty() )
                    {
                        sDocURL = ::comphelper::NamedValueCollection::getOrDefault( xModel->getArgs(), u"Title", sDocURL );
                    }

                    if ( sLocation != sDocURL )
                        continue;

                    Reference< XEmbeddedScripts > xDocumentScripts( m_xModel, UNO_QUERY );
                    if ( !xDocumentScripts.is() )
                        continue;

                    xLibContainer = xDocumentScripts->getDialogLibraries();
                    OSL_ENSURE( xLibContainer.is(),
                        "DialogProviderImpl::createDialogModel: invalid dialog container!" );
                }
            }

            // get input stream provider
            Reference< io::XInputStreamProvider > xISP;
            if ( !xLibContainer.is() )
            {
                throw IllegalArgumentException(
                    "DialogProviderImpl::getDialog: library container not found!",
                    Reference< XInterface >(), 1 );
            }

            // load dialog library
            if ( !xLibContainer->isLibraryLoaded( sLibName ) )
                xLibContainer->loadLibrary( sLibName );

            // get dialog library
            if ( xLibContainer->hasByName( sLibName ) )
            {
                Any aElement = xLibContainer->getByName( sLibName );
                aElement >>= xDialogLib;
            }

            if ( !xDialogLib.is() )
            {
                throw IllegalArgumentException(
                    "DialogProviderImpl::getDialogModel: library not found!",
                    Reference< XInterface >(), 1 );
            }

            // get input stream provider
            if ( xDialogLib->hasByName( sDlgName ) )
            {
                Any aElement = xDialogLib->getByName( sDlgName );
                aElement >>= xISP;
            }

            if ( !xISP.is() )
            {
                throw IllegalArgumentException(
                    "DialogProviderImpl::getDialogModel: dialog not found!",
                    Reference< XInterface >(), 1 );
            }



            if ( xISP.is() )
                xInput = xISP->createInputStream();
            msDialogLibName = sLibName;
        }

        // import dialog model
        Reference< XControlModel > xCtrlModel;
        if ( xInput.is() && m_xContext.is() )
        {
            Reference< resource::XStringResourceManager > xStringResourceManager;
            if( bSingleDialog )
            {
                xStringResourceManager = lcl_getStringResourceManager(m_xContext,aURL);
            }
            else if( xDialogLib.is() )
            {
                xStringResourceManager = getStringResourceFromDialogLibrary( xDialogLib );
            }

            Any aDialogSourceURLAny;
            aDialogSourceURLAny <<= aURL;

            Reference< container::XNameContainer > xDialogModel( createDialogModel( xInput , xStringResourceManager, aDialogSourceURLAny  ), UNO_SET_THROW);

            xCtrlModel.set( xDialogModel, UNO_QUERY );
        }
        return xCtrlModel;
    }


    Reference< XUnoControlDialog > DialogProviderImpl::createDialogControl
        ( const Reference< XControlModel >& rxDialogModel, const Reference< XWindowPeer >& xParent )
    {
        OSL_ENSURE( rxDialogModel.is(), "DialogProviderImpl::getDialogControl: no dialog model" );

        Reference< XUnoControlDialog > xDialogControl;

        if ( m_xContext.is() )
        {
            xDialogControl = UnoControlDialog::create( m_xContext );

            // set the model
            if ( rxDialogModel.is() )
                xDialogControl->setModel( rxDialogModel );

            // set visible
            xDialogControl->setVisible( false );

            // get the parent of the dialog control
            Reference< XWindowPeer > xPeer;
            if( xParent.is() )
            {
                xPeer = xParent;
            }
            else if ( m_xModel.is() )
            {
                Reference< frame::XController > xController = m_xModel->getCurrentController();
                if ( xController.is() )
                {
                    Reference< frame::XFrame > xFrame = xController->getFrame();
                    if ( xFrame.is() )
                        xPeer.set( xFrame->getContainerWindow(), UNO_QUERY );
                }
            }

            // create a peer
            Reference< XToolkit> xToolkit( Toolkit::create( m_xContext ), UNO_QUERY_THROW );
            xDialogControl->createPeer( xToolkit, xPeer );
        }

        return xDialogControl;
    }


    void DialogProviderImpl::attachControlEvents(
        const Reference< XControl >& rxControl,
        const Reference< XInterface >& rxHandler,
        const Reference< XIntrospectionAccess >& rxIntrospectionAccess,
        bool bDialogProviderMode )
    {
        if ( !rxControl.is() )
            return;

        Reference< XControlContainer > xControlContainer( rxControl, UNO_QUERY );

        if ( !xControlContainer.is() )
            return;

        Sequence< Reference< XControl > > aControls = xControlContainer->getControls();
        sal_Int32 nControlCount = aControls.getLength();

        Sequence< Reference< XInterface > > aObjects( nControlCount + 1 );
        Reference< XInterface >* pObjects = aObjects.getArray();
        std::transform(aControls.begin(), aControls.end(), pObjects,
                       [](auto& xControl) { return xControl.template query<XInterface>(); });

        // also add the dialog control itself to the sequence
        pObjects[nControlCount].set( rxControl, UNO_QUERY );

        Reference<XScriptEventsAttacher> xScriptEventsAttacher
            = new DialogEventsAttacherImpl(
                m_xContext, m_xModel, rxControl, rxHandler, rxIntrospectionAccess,
                bDialogProviderMode,
                (m_BasicInfo ? m_BasicInfo->mxBasicRTLListener : nullptr), msDialogLibName);

        Any aHelper;
        xScriptEventsAttacher->attachEvents( aObjects, Reference< XScriptListener >(), aHelper );
    }

    Reference< XIntrospectionAccess > DialogProviderImpl::inspectHandler( const Reference< XInterface >& rxHandler )
    {
        Reference< XIntrospectionAccess > xIntrospectionAccess;
        static Reference< XIntrospection > xIntrospection;

        if( !rxHandler.is() )
            return xIntrospectionAccess;

        if( !xIntrospection.is() )
        {
            // Get introspection service
            xIntrospection = theIntrospection::get( m_xContext );
        }

        // Do introspection
        try
        {
            Any aHandlerAny;
            aHandlerAny <<= rxHandler;
            xIntrospectionAccess = xIntrospection->inspect( aHandlerAny );
        }
        catch( RuntimeException& )
        {
            xIntrospectionAccess.clear();
        }
        return xIntrospectionAccess;
    }


    // XServiceInfo


    OUString DialogProviderImpl::getImplementationName(  )
    {
        return "com.sun.star.comp.scripting.DialogProvider";
    }

    sal_Bool DialogProviderImpl::supportsService( const OUString& rServiceName )
    {
        return cppu::supportsService(this, rServiceName);
    }

    Sequence< OUString > DialogProviderImpl::getSupportedServiceNames(  )
    {
        return { "com.sun.star.awt.DialogProvider",
                 "com.sun.star.awt.DialogProvider2",
                 "com.sun.star.awt.ContainerWindowProvider" };
    }


    // XInitialization


    void DialogProviderImpl::initialize( const Sequence< Any >& aArguments )
    {
        ::osl::MutexGuard aGuard( getMutex() );

        if ( aArguments.getLength() == 1 )
        {
            aArguments[0] >>= m_xModel;

            if ( !m_xModel.is() )
            {
                throw RuntimeException( "DialogProviderImpl::initialize: invalid argument format!" );
            }
        }
        else if ( aArguments.getLength() == 4 )
        {
            // call from RTL_Impl_CreateUnoDialog
            aArguments[0] >>= m_xModel;
            m_BasicInfo.reset( new BasicRTLParams );
            m_BasicInfo->mxInput.set( aArguments[ 1 ], UNO_QUERY_THROW );
            // allow null mxDlgLib, a document dialog instantiated from
            // from application basic is unable to provide ( or find ) it's
            // Library
            aArguments[ 2 ] >>= m_BasicInfo->mxDlgLib;
            // leave the possibility to optionally allow the old dialog creation
            // to use the new XScriptListener ( which converts the old style macro
            // to a SF url )
            m_BasicInfo->mxBasicRTLListener.set( aArguments[ 3 ], UNO_QUERY);
        }
        else if ( aArguments.getLength() > 4 )
        {
            throw RuntimeException( "DialogProviderImpl::initialize: invalid number of arguments!" );
        }
    }


    // XDialogProvider


    constexpr OUString aDecorationPropName = u"Decoration"_ustr;

    Reference < XControl > DialogProviderImpl::createDialogImpl(
        const OUString& URL, const Reference< XInterface >& xHandler,
        const Reference< XWindowPeer >& xParent, bool bDialogProviderMode )
    {
        // if the dialog is located in a document, the document must already be open!

        ::osl::MutexGuard aGuard( getMutex() );


        // m_xHandler = xHandler;

        //Reference< XDialog > xDialog;
        Reference< XControl > xCtrl;
        Reference< XControlModel > xCtrlMod;
        try
        {
            // add support for basic RTL_FUNCTION
            if (m_BasicInfo)
                xCtrlMod = createDialogModelForBasic();
            else
            {
                OSL_ENSURE( !URL.isEmpty(), "DialogProviderImpl::getDialog: no URL!" );
                xCtrlMod = createDialogModel( URL );
            }
        }
        catch ( const RuntimeException& ) { throw; }
        catch ( const Exception& )
        {
            const Any aError( ::cppu::getCaughtException() );
            throw WrappedTargetRuntimeException( OUString(), *this, aError );
        }
        if ( xCtrlMod.is() )
        {
            // i83963 Force decoration
            if( bDialogProviderMode )
            {
                uno::Reference< beans::XPropertySet > xDlgModPropSet( xCtrlMod, uno::UNO_QUERY );
                if( xDlgModPropSet.is() )
                {
                    try
                    {
                        bool bDecoration = true;
                        Any aDecorationAny = xDlgModPropSet->getPropertyValue( aDecorationPropName );
                        aDecorationAny >>= bDecoration;
                        if( !bDecoration )
                        {
                            xDlgModPropSet->setPropertyValue( aDecorationPropName, Any( true ) );
                            xDlgModPropSet->setPropertyValue( "Title", Any( OUString() ) );
                        }
                    }
                    catch( UnknownPropertyException& )
                    {}
                }
            }

            xCtrl.set( createDialogControl( xCtrlMod, xParent ) );
            if ( xCtrl.is() )
            {
                Reference< XIntrospectionAccess > xIntrospectionAccess = inspectHandler( xHandler );
                attachControlEvents( xCtrl, xHandler, xIntrospectionAccess, bDialogProviderMode );
            }
        }

        return xCtrl;
    }

    Reference < XDialog > DialogProviderImpl::createDialog( const OUString& URL )
    {
        Reference< XInterface > xDummyHandler;
        Reference< XWindowPeer > xDummyPeer;
        Reference < XControl > xControl = DialogProviderImpl::createDialogImpl( URL, xDummyHandler, xDummyPeer, true );
        Reference< XDialog > xDialog( xControl, UNO_QUERY );
        return xDialog;
    }

    Reference < XDialog > DialogProviderImpl::createDialogWithHandler(
        const OUString& URL, const Reference< XInterface >& xHandler )
    {
        if( !xHandler.is() )
        {
            throw IllegalArgumentException(
                "DialogProviderImpl::createDialogWithHandler: Invalid xHandler!",
                Reference< XInterface >(), 1 );
        }
        Reference< XWindowPeer > xDummyPeer;
        Reference < XControl > xControl = DialogProviderImpl::createDialogImpl( URL, xHandler, xDummyPeer, true );
        Reference< XDialog > xDialog( xControl, UNO_QUERY );
        return xDialog;
    }

    Reference < XDialog > DialogProviderImpl::createDialogWithArguments(
        const OUString& URL, const Sequence< NamedValue >& Arguments )
    {
        ::comphelper::NamedValueCollection aArguments( Arguments );

        Reference< XWindowPeer > xParentPeer;
        if ( aArguments.has( "ParentWindow" ) )
        {
            const Any& aParentWindow( aArguments.get( "ParentWindow" ) );
            if ( !( aParentWindow >>= xParentPeer ) )
            {
                const Reference< XControl > xParentControl( aParentWindow, UNO_QUERY );
                if ( xParentControl.is() )
                    xParentPeer = xParentControl->getPeer();
            }
        }

        const Reference< XInterface > xHandler( aArguments.get( "EventHandler" ), UNO_QUERY );

        Reference < XControl > xControl = DialogProviderImpl::createDialogImpl( URL, xHandler, xParentPeer, true );
        Reference< XDialog > xDialog( xControl, UNO_QUERY );
        return xDialog;
    }

    Reference< XWindow > DialogProviderImpl::createContainerWindow(
        const OUString& URL, const OUString&,
        const Reference< XWindowPeer >& xParent, const Reference< XInterface >& xHandler )
    {
        if( !xParent.is() )
        {
            throw IllegalArgumentException(
                "DialogProviderImpl::createContainerWindow: Invalid xParent!",
                Reference< XInterface >(), 1 );
        }
        Reference < XControl > xControl = DialogProviderImpl::createDialogImpl( URL, xHandler, xParent, false );
        Reference< XWindow> xWindow( xControl, UNO_QUERY );
        return xWindow;
    }


    // component operations


    extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
    scripting_DialogProviderImpl_get_implementation(
        css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
    {
        return cppu::acquire(new DialogProviderImpl(context));
    }

}   // namespace dlgprov


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
