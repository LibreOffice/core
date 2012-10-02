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


#include "DialogModelProvider.hxx"
#include "dlgprov.hxx"
#include "dlgevtatt.hxx"
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/io/XInputStreamProvider.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/resource/XStringResourceSupplier.hpp>
#include <com/sun/star/resource/XStringResourceManager.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess2.hpp>
#include <com/sun/star/resource/XStringResourceWithLocation.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <xmlscript/xmldlg_imexp.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/namedvaluecollection.hxx>

#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#include <com/sun/star/uri/XVndSunStarExpandUrl.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>

#include <util/MiscUtils.hxx>

using namespace ::com::sun::star;
using namespace awt;
using namespace lang;
using namespace uno;
using namespace script;
using namespace beans;
using namespace document;
using namespace ::sf_misc;

// component helper namespace
namespace comp_DialogModelProvider
{

    ::rtl::OUString SAL_CALL _getImplementationName()
    {
        return ::rtl::OUString("com.sun.star.comp.scripting.DialogModelProvider");
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames()
    {
        uno::Sequence< ::rtl::OUString > s(1);
        s[0] = ::rtl::OUString("com.sun.star.awt.UnoControlDialogModelProvider");
        return s;
    }

    uno::Reference< uno::XInterface > SAL_CALL _create(const uno::Reference< uno::XComponentContext > & context) SAL_THROW((uno::Exception))
    {
        return static_cast< ::cppu::OWeakObject * >(new dlgprov::DialogModelProvider(context));
    }
} // closing component helper namespace
//.........................................................................
namespace dlgprov
{
//.........................................................................

static ::rtl::OUString aResourceResolverPropName("ResourceResolver");

    Reference< resource::XStringResourceManager > lcl_getStringResourceManager(const Reference< XComponentContext >& i_xContext,const ::rtl::OUString& i_sURL)
    {
        INetURLObject aInetObj( i_sURL );
        ::rtl::OUString aDlgName = aInetObj.GetBase();
        aInetObj.removeSegment();
        ::rtl::OUString aDlgLocation = aInetObj.GetMainURL( INetURLObject::NO_DECODE );
        bool bReadOnly = true;
        ::com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();
        ::rtl::OUString aComment;

        Sequence<Any> aArgs( 6 );
        aArgs[0] <<= aDlgLocation;
        aArgs[1] <<= bReadOnly;
        aArgs[2] <<= aLocale;
        aArgs[3] <<= aDlgName;
        aArgs[4] <<= aComment;

        Reference< task::XInteractionHandler > xDummyHandler;
        aArgs[5] <<= xDummyHandler;
        Reference< XMultiComponentFactory > xSMgr_( i_xContext->getServiceManager(), UNO_QUERY_THROW );
        // TODO: Ctor
        Reference< resource::XStringResourceManager > xStringResourceManager( xSMgr_->createInstanceWithContext
            ( ::rtl::OUString("com.sun.star.resource.StringResourceWithLocation"),
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
        Reference< XMultiComponentFactory > xSMgr_( i_xContext->getServiceManager(), UNO_QUERY_THROW );
        Reference< container::XNameContainer > xControlModel( xSMgr_->createInstanceWithContext( ::rtl::OUString( "com.sun.star.awt.UnoControlDialogModel"  ), i_xContext ), UNO_QUERY_THROW );
        return xControlModel;
    }
    Reference< container::XNameContainer > lcl_createDialogModel( const Reference< XComponentContext >& i_xContext,
        const Reference< io::XInputStream >& xInput,
        const Reference< frame::XModel >& xModel,
        const Reference< resource::XStringResourceManager >& xStringResourceManager,
        const Any &aDialogSourceURL) throw ( Exception )
    {
        Reference< container::XNameContainer > xDialogModel(  lcl_createControlModel(i_xContext) );

        ::rtl::OUString aDlgSrcUrlPropName( "DialogSourceURL"  );
        Reference< beans::XPropertySet > xDlgPropSet( xDialogModel, UNO_QUERY );
        xDlgPropSet->setPropertyValue( aDlgSrcUrlPropName, aDialogSourceURL );

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
            xDlgPSet->setPropertyValue( aResourceResolverPropName, aStringResourceManagerAny );
        }

        return xDialogModel;
    }
    // =============================================================================
    // component operations
    // =============================================================================

    static ::rtl::OUString getImplementationName_DialogProviderImpl()
    {
        static ::rtl::OUString* pImplName = 0;
        if ( !pImplName )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pImplName )
            {
                static ::rtl::OUString aImplName( "com.sun.star.comp.scripting.DialogProvider"  );
                pImplName = &aImplName;
            }
        }
        return *pImplName;
    }

    // -----------------------------------------------------------------------------

    static Sequence< ::rtl::OUString > getSupportedServiceNames_DialogProviderImpl()
    {
        static Sequence< ::rtl::OUString >* pNames = 0;
        if ( !pNames )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pNames )
            {
                static Sequence< ::rtl::OUString > aNames(3);
                aNames.getArray()[0] = ::rtl::OUString( "com.sun.star.awt.DialogProvider"  );
                aNames.getArray()[1] = ::rtl::OUString( "com.sun.star.awt.DialogProvider2"  );
                aNames.getArray()[2] = ::rtl::OUString( "com.sun.star.awt.ContainerWindowProvider"  );
                pNames = &aNames;
            }
        }
        return *pNames;
    }


    // =============================================================================
    // mutex
    // =============================================================================

    ::osl::Mutex& getMutex()
    {
        static ::osl::Mutex* s_pMutex = 0;
        if ( !s_pMutex )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !s_pMutex )
            {
                static ::osl::Mutex s_aMutex;
                s_pMutex = &s_aMutex;
            }
        }
        return *s_pMutex;
    }


    // =============================================================================
    // DialogProviderImpl
    // =============================================================================

    DialogProviderImpl::DialogProviderImpl( const Reference< XComponentContext >& rxContext )
        :m_xContext( rxContext )
        ,m_xModel( 0 )
    {
    }

    // -----------------------------------------------------------------------------

    DialogProviderImpl::~DialogProviderImpl()
    {
    }

    // -----------------------------------------------------------------------------

    Reference< resource::XStringResourceManager > getStringResourceFromDialogLibrary
        ( Reference< container::XNameContainer > xDialogLib )
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
        const Any &aDialogSourceURL) throw ( Exception )
    {
        return lcl_createDialogModel(m_xContext,xInput,m_xModel,xStringResourceManager,aDialogSourceURL);
    }

    Reference< XControlModel > DialogProviderImpl::createDialogModelForBasic() throw ( Exception )
    {
        if ( !m_BasicInfo.get() )
            // shouln't get here
            throw RuntimeException( ::rtl::OUString( "No information to create dialog"  ), Reference< XInterface >() );
        Reference< resource::XStringResourceManager > xStringResourceManager = getStringResourceFromDialogLibrary( m_BasicInfo->mxDlgLib );

        rtl::OUString aURL("" );
        Any aDialogSourceURL;
        aDialogSourceURL <<= aURL;
        Reference< XControlModel > xCtrlModel( createDialogModel( m_BasicInfo->mxInput, xStringResourceManager, aDialogSourceURL ), UNO_QUERY_THROW );
        return xCtrlModel;
    }

    Reference< XControlModel > DialogProviderImpl::createDialogModel( const ::rtl::OUString& sURL )
    {

        ::rtl::OUString aURL( sURL );

        // parse URL
        // TODO: use URL parsing class
        // TODO: decoding of location
        Reference< XMultiComponentFactory > xSMgr( m_xContext->getServiceManager(), UNO_QUERY );

        if ( !xSMgr.is() )
        {
            throw RuntimeException(
                ::rtl::OUString( "DialogProviderImpl::getDialogModel: Couldn't instantiate MultiComponent factory"  ),
                    Reference< XInterface >() );
        }

        Reference< uri::XUriReferenceFactory > xFac (
            xSMgr->createInstanceWithContext( rtl::OUString(
            "com.sun.star.uri.UriReferenceFactory"), m_xContext ) , UNO_QUERY );

        if  ( !xFac.is() )
        {
            throw RuntimeException(
                ::rtl::OUString("DialogProviderImpl::getDialogModel(), could not instatiate UriReferenceFactory."),
                Reference< XInterface >() );
        }

        // i75778: Support non-script URLs
        Reference< io::XInputStream > xInput;
        Reference< container::XNameContainer > xDialogLib;

        // Accept file URL to single dialog
        bool bSingleDialog = false;

        Reference< util::XMacroExpander > xMacroExpander(
            m_xContext->getValueByName(
            ::rtl::OUString("/singletons/com.sun.star.util.theMacroExpander") ),
            UNO_QUERY_THROW );

        Reference< uri::XUriReference > uriRef;
        for (;;)
        {
            uriRef = Reference< uri::XUriReference >( xFac->parse( aURL ), UNO_QUERY );
            if ( !uriRef.is() )
            {
                ::rtl::OUString errorMsg("DialogProviderImpl::getDialogModel: failed to parse URI: ");
                errorMsg += aURL;
                throw IllegalArgumentException( errorMsg,
                                                Reference< XInterface >(), 1 );
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
            Reference< ucb::XSimpleFileAccess2 > xSFI = ucb::SimpleFileAccess::create(m_xContext);

            try
            {
                xInput = xSFI->openFileRead( aURL );
            }
            catch( Exception& )
            {}
        }
        else
        {
            ::rtl::OUString sDescription = sfUri->getName();

            sal_Int32 nIndex = 0;

            ::rtl::OUString sLibName = sDescription.getToken( 0, (sal_Unicode)'.', nIndex );
            ::rtl::OUString sDlgName;
            if ( nIndex != -1 )
                sDlgName = sDescription.getToken( 0, (sal_Unicode)'.', nIndex );

            ::rtl::OUString sLocation = sfUri->getParameter(
                ::rtl::OUString("location") );


            // get dialog library container
            // TODO: dialogs in packages
            Reference< XLibraryContainer > xLibContainer;

            if ( sLocation == "application" )
            {
                xLibContainer = Reference< XLibraryContainer >( SFX_APP()->GetDialogContainer(), UNO_QUERY );
            }
            else if ( sLocation == "document" )
            {
                Reference< XEmbeddedScripts > xDocumentScripts( m_xModel, UNO_QUERY );
                if ( xDocumentScripts.is() )
                {
                    xLibContainer.set( xDocumentScripts->getDialogLibraries(), UNO_QUERY );
                    OSL_ENSURE( xLibContainer.is(),
                        "DialogProviderImpl::createDialogModel: invalid dialog container!" );
                }
            }
            else
            {
                Sequence< ::rtl::OUString > aOpenDocsTdocURLs( MiscUtils::allOpenTDocUrls( m_xContext ) );
                const ::rtl::OUString* pTdocURL = aOpenDocsTdocURLs.getConstArray();
                const ::rtl::OUString* pTdocURLEnd = aOpenDocsTdocURLs.getConstArray() + aOpenDocsTdocURLs.getLength();
                for ( ; pTdocURL != pTdocURLEnd; ++pTdocURL )
                {
                    Reference< frame::XModel > xModel( MiscUtils::tDocUrlToModel( *pTdocURL ) );
                    OSL_ENSURE( xModel.is(), "DialogProviderImpl::createDialogModel: invalid document model!" );
                    if ( !xModel.is() )
                        continue;

                    ::rtl::OUString sDocURL = xModel->getURL();
                    if ( sDocURL.isEmpty() )
                    {
                        ::comphelper::NamedValueCollection aModelArgs( xModel->getArgs() );
                        sDocURL = aModelArgs.getOrDefault( "Title", sDocURL );
                    }

                    if ( sLocation != sDocURL )
                        continue;

                    Reference< XEmbeddedScripts > xDocumentScripts( m_xModel, UNO_QUERY );
                    if ( !xDocumentScripts.is() )
                        continue;

                    xLibContainer.set( xDocumentScripts->getDialogLibraries(), UNO_QUERY );
                    OSL_ENSURE( xLibContainer.is(),
                        "DialogProviderImpl::createDialogModel: invalid dialog container!" );
                }
            }

            // get input stream provider
            Reference< io::XInputStreamProvider > xISP;
            if ( xLibContainer.is() )
            {
                // load dialog library
                if ( !xLibContainer->isLibraryLoaded( sLibName ) )
                    xLibContainer->loadLibrary( sLibName );

                // get dialog library
                if ( xLibContainer->hasByName( sLibName ) )
                {
                    Any aElement = xLibContainer->getByName( sLibName );
                    aElement >>= xDialogLib;
                }

                if ( xDialogLib.is() )
                {
                    // get input stream provider
                    if ( xDialogLib->hasByName( sDlgName ) )
                    {
                        Any aElement = xDialogLib->getByName( sDlgName );
                        aElement >>= xISP;
                    }

                    if ( !xISP.is() )
                    {
                        throw IllegalArgumentException(
                            ::rtl::OUString( "DialogProviderImpl::getDialogModel: dialog not found!"  ),
                            Reference< XInterface >(), 1 );
                    }
                }
                else
                {
                    throw IllegalArgumentException(
                        ::rtl::OUString( "DialogProviderImpl::getDialogModel: library not found!"  ),
                        Reference< XInterface >(), 1 );
                }
            }
            else
            {
                throw IllegalArgumentException(
                    ::rtl::OUString( "DialogProviderImpl::getDialog: library container not found!"  ),
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

            Reference< container::XNameContainer > xDialogModel( createDialogModel( xInput , xStringResourceManager, aDialogSourceURLAny  ), UNO_QUERY_THROW);

            xCtrlModel = Reference< XControlModel >( xDialogModel, UNO_QUERY );
        }
        return xCtrlModel;
    }

    // -----------------------------------------------------------------------------

    Reference< XControl > DialogProviderImpl::createDialogControl
        ( const Reference< XControlModel >& rxDialogModel, const Reference< XWindowPeer >& xParent )
    {
        OSL_ENSURE( rxDialogModel.is(), "DialogProviderImpl::getDialogControl: no dialog model" );

        Reference< XControl > xDialogControl;

        if ( m_xContext.is() )
        {
            Reference< XMultiComponentFactory > xSMgr( m_xContext->getServiceManager() );

            if ( xSMgr.is() )
            {
                xDialogControl = Reference< XControl >( xSMgr->createInstanceWithContext(
                    ::rtl::OUString( "com.sun.star.awt.UnoControlDialog"  ), m_xContext ), UNO_QUERY );

                if ( xDialogControl.is() )
                {
                    // set the model
                    if ( rxDialogModel.is() )
                        xDialogControl->setModel( rxDialogModel );

                    // set visible
                    Reference< XWindow > xW( xDialogControl, UNO_QUERY );
                    if ( xW.is() )
                        xW->setVisible( sal_False );

                    // get the parent of the dialog control
                    Reference< XWindowPeer > xPeer;
                    if( xParent.is() )
                    {
                        xPeer = xParent;
                    }
                    else if ( m_xModel.is() )
                    {
                        Reference< frame::XController > xController( m_xModel->getCurrentController(), UNO_QUERY );
                        if ( xController.is() )
                        {
                            Reference< frame::XFrame > xFrame( xController->getFrame(), UNO_QUERY );
                            if ( xFrame.is() )
                                xPeer = Reference< XWindowPeer>( xFrame->getContainerWindow(), UNO_QUERY );
                        }
                    }

                    // create a peer
                    Reference< XToolkit> xToolkit( xSMgr->createInstanceWithContext(
                        ::rtl::OUString( "com.sun.star.awt.Toolkit"  ), m_xContext ), UNO_QUERY );
                    if ( xToolkit.is() )
                        xDialogControl->createPeer( xToolkit, xPeer );
                }
            }
        }

        return xDialogControl;
    }

    // -----------------------------------------------------------------------------

    void DialogProviderImpl::attachControlEvents(
        const Reference< XControl >& rxControl,
        const Reference< XInterface >& rxHandler,
        const Reference< XIntrospectionAccess >& rxIntrospectionAccess,
        bool bDialogProviderMode )
    {
        if ( rxControl.is() )
        {
            Reference< XControlContainer > xControlContainer( rxControl, UNO_QUERY );

            if ( xControlContainer.is() )
            {
                Sequence< Reference< XControl > > aControls = xControlContainer->getControls();
                const Reference< XControl >* pControls = aControls.getConstArray();
                sal_Int32 nControlCount = aControls.getLength();

                Sequence< Reference< XInterface > > aObjects( nControlCount + 1 );
                Reference< XInterface >* pObjects = aObjects.getArray();
                for ( sal_Int32 i = 0; i < nControlCount; ++i )
                {
                    pObjects[i] = Reference< XInterface >( pControls[i], UNO_QUERY );
                }

                // also add the dialog control itself to the sequence
                pObjects[nControlCount] = Reference< XInterface >( rxControl, UNO_QUERY );

                Reference< XScriptEventsAttacher > xScriptEventsAttacher = new DialogEventsAttacherImpl
                    ( m_xContext, m_xModel, rxControl, rxHandler, rxIntrospectionAccess,
                      bDialogProviderMode, ( m_BasicInfo.get() ? m_BasicInfo->mxBasicRTLListener : NULL ), msDialogLibName );

                Any aHelper;
                xScriptEventsAttacher->attachEvents( aObjects, Reference< XScriptListener >(), aHelper );
            }
        }
    }

    Reference< XIntrospectionAccess > DialogProviderImpl::inspectHandler( const Reference< XInterface >& rxHandler )
    {
        Reference< XIntrospectionAccess > xIntrospectionAccess;
        static Reference< XIntrospection > xIntrospection;

        if( !rxHandler.is() )
            return xIntrospectionAccess;

        if( !xIntrospection.is() )
        {
            Reference< XMultiComponentFactory > xSMgr( m_xContext->getServiceManager(), UNO_QUERY );
            if ( !xSMgr.is() )
            {
                throw RuntimeException(
                    ::rtl::OUString( "DialogProviderImpl::getIntrospectionAccess: Couldn't instantiate MultiComponent factory"  ),
                        Reference< XInterface >() );
            }

            // Get introspection service
            Reference< XInterface > xI = xSMgr->createInstanceWithContext
                ( rtl::OUString("com.sun.star.beans.Introspection"), m_xContext );
            if (xI.is())
                xIntrospection = Reference< XIntrospection >::query( xI );
        }

        if( xIntrospection.is() )
        {
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
        }
        return xIntrospectionAccess;
    }


    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------

    ::rtl::OUString DialogProviderImpl::getImplementationName(  ) throw (RuntimeException)
    {
        return getImplementationName_DialogProviderImpl();
    }

    // -----------------------------------------------------------------------------

    sal_Bool DialogProviderImpl::supportsService( const ::rtl::OUString& rServiceName ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aNames( getSupportedServiceNames() );
        const ::rtl::OUString* pNames = aNames.getConstArray();
        const ::rtl::OUString* pEnd = pNames + aNames.getLength();
        for ( ; pNames != pEnd && !pNames->equals( rServiceName ); ++pNames )
            ;

        return pNames != pEnd;
    }

    // -----------------------------------------------------------------------------

    Sequence< ::rtl::OUString > DialogProviderImpl::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return getSupportedServiceNames_DialogProviderImpl();
    }

    // -----------------------------------------------------------------------------
    // XInitialization
    // -----------------------------------------------------------------------------

    void DialogProviderImpl::initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException)
    {
        ::osl::MutexGuard aGuard( getMutex() );

        if ( aArguments.getLength() == 1 )
        {
            aArguments[0] >>= m_xModel;

            if ( !m_xModel.is() )
            {
                throw RuntimeException(
                    ::rtl::OUString( "DialogProviderImpl::initialize: invalid argument format!"  ),
                    Reference< XInterface >() );
            }
        }
        else if ( aArguments.getLength() == 4 )
        {
            // call from RTL_Impl_CreateUnoDialog
            aArguments[0] >>= m_xModel;
            m_BasicInfo.reset( new BasicRTLParams() );
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
            throw RuntimeException(
                ::rtl::OUString( "DialogProviderImpl::initialize: invalid number of arguments!"  ),
                Reference< XInterface >() );
        }
    }

    // -----------------------------------------------------------------------------
    // XDialogProvider
    // -----------------------------------------------------------------------------

    static ::rtl::OUString aDecorationPropName("Decoration");
    static ::rtl::OUString aTitlePropName("Title");

    Reference < XControl > DialogProviderImpl::createDialogImpl(
        const ::rtl::OUString& URL, const Reference< XInterface >& xHandler,
        const Reference< XWindowPeer >& xParent, bool bDialogProviderMode )
            throw (IllegalArgumentException, RuntimeException)
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
            if ( m_BasicInfo.get() )
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
            throw WrappedTargetRuntimeException( ::rtl::OUString(), *this, aError );
        }
        if ( xCtrlMod.is() )
        {
            // i83963 Force decoration
            if( bDialogProviderMode )
            {
                uno::Reference< beans::XPropertySet > xDlgModPropSet( xCtrlMod, uno::UNO_QUERY );
                if( xDlgModPropSet.is() )
                {
                    bool bDecoration = true;
                    try
                    {
                        Any aDecorationAny = xDlgModPropSet->getPropertyValue( aDecorationPropName );
                        aDecorationAny >>= bDecoration;
                        if( !bDecoration )
                        {
                            xDlgModPropSet->setPropertyValue( aDecorationPropName, makeAny( true ) );
                            xDlgModPropSet->setPropertyValue( aTitlePropName, makeAny( ::rtl::OUString() ) );
                        }
                    }
                    catch( UnknownPropertyException& )
                    {}
                }
            }

            xCtrl = Reference< XControl >( createDialogControl( xCtrlMod, xParent ) );
            if ( xCtrl.is() )
            {
                //xDialog = Reference< XDialog >( xCtrl, UNO_QUERY );
                Reference< XIntrospectionAccess > xIntrospectionAccess = inspectHandler( xHandler );
                attachControlEvents( xCtrl, xHandler, xIntrospectionAccess, bDialogProviderMode );
            }
        }

        return xCtrl;
    }

    Reference < XDialog > DialogProviderImpl::createDialog( const ::rtl::OUString& URL )
        throw (IllegalArgumentException, RuntimeException)
    {
        Reference< XInterface > xDummyHandler;
        Reference< XWindowPeer > xDummyPeer;
        Reference < XControl > xControl = DialogProviderImpl::createDialogImpl( URL, xDummyHandler, xDummyPeer, true );
        Reference< XDialog > xDialog( xControl, UNO_QUERY );
        return xDialog;
    }

    Reference < XDialog > DialogProviderImpl::createDialogWithHandler(
        const ::rtl::OUString& URL, const Reference< XInterface >& xHandler )
            throw (IllegalArgumentException, RuntimeException)
    {
        if( !xHandler.is() )
        {
            throw IllegalArgumentException(
                ::rtl::OUString( "DialogProviderImpl::createDialogWithHandler: Invalid xHandler!"  ),
                Reference< XInterface >(), 1 );
        }
        Reference< XWindowPeer > xDummyPeer;
        Reference < XControl > xControl = DialogProviderImpl::createDialogImpl( URL, xHandler, xDummyPeer, true );
        Reference< XDialog > xDialog( xControl, UNO_QUERY );
        return xDialog;
    }

    Reference < XDialog > DialogProviderImpl::createDialogWithArguments(
        const ::rtl::OUString& URL, const Sequence< NamedValue >& Arguments )
            throw (IllegalArgumentException, RuntimeException)
    {
        ::comphelper::NamedValueCollection aArguments( Arguments );

        Reference< XWindowPeer > xParentPeer;
        if ( aArguments.has( "ParentWindow" ) )
        {
            const Any aParentWindow( aArguments.get( "ParentWindow" ) );
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
        const ::rtl::OUString& URL, const ::rtl::OUString& WindowType,
        const Reference< XWindowPeer >& xParent, const Reference< XInterface >& xHandler )
            throw (lang::IllegalArgumentException, RuntimeException)
    {
        (void)WindowType;   // for future use
        if( !xParent.is() )
        {
            throw IllegalArgumentException(
                ::rtl::OUString( "DialogProviderImpl::createContainerWindow: Invalid xParent!"  ),
                Reference< XInterface >(), 1 );
        }
        Reference < XControl > xControl = DialogProviderImpl::createDialogImpl( URL, xHandler, xParent, false );
        Reference< XWindow> xWindow( xControl, UNO_QUERY );
        return xWindow;
    }


    // =============================================================================
    // component operations
    // =============================================================================

    static Reference< XInterface > SAL_CALL create_DialogProviderImpl(
        Reference< XComponentContext > const & xContext )
        SAL_THROW(())
    {
        return static_cast< lang::XTypeProvider * >( new DialogProviderImpl( xContext ) );
    }

    // -----------------------------------------------------------------------------

    static struct ::cppu::ImplementationEntry s_component_entries [] =
    {
        {create_DialogProviderImpl, getImplementationName_DialogProviderImpl,getSupportedServiceNames_DialogProviderImpl, ::cppu::createSingleComponentFactory,0, 0},
        { &comp_DialogModelProvider::_create,&comp_DialogModelProvider::_getImplementationName,&comp_DialogModelProvider::_getSupportedServiceNames,&::cppu::createSingleComponentFactory, 0, 0 },
        { 0, 0, 0, 0, 0, 0 }
    };

    // -----------------------------------------------------------------------------

//.........................................................................
}   // namespace dlgprov
//.........................................................................


// =============================================================================
// component exports
// =============================================================================

extern "C"
{
    SAL_DLLPUBLIC_EXPORT void * SAL_CALL dlgprov_component_getFactory(
        const sal_Char * pImplName, lang::XMultiServiceFactory * pServiceManager,
        registry::XRegistryKey * pRegistryKey )
    {
        return ::cppu::component_getFactoryHelper(
            pImplName, pServiceManager, pRegistryKey, ::dlgprov::s_component_entries );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
