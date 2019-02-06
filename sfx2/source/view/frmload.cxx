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


#include <sal/macros.h>
#include <objshimp.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/doctempl.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/sfxuno.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfac.hxx>

#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionHandler2.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#include <com/sun/star/frame/XController2.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <comphelper/interaction.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <framework/interaction.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.h>
#include <sal/log.hxx>
#include <sot/storinfo.hxx>
#include <svtools/ehdl.hxx>
#include <svl/eitem.hxx>
#include <svl/itemset.hxx>
#include <unotools/moduleoptions.hxx>
#include <svtools/sfxecode.hxx>
#include <svl/stritem.hxx>
#include <tools/diagnose_ex.h>
#include <ucbhelper/simpleinteractionrequest.hxx>

using namespace com::sun::star;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::container::XContainerQuery;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::document::XTypeDetection;
using ::com::sun::star::frame::XFrame;
using ::com::sun::star::frame::XLoadable;
using ::com::sun::star::task::XInteractionHandler;
using ::com::sun::star::task::XInteractionHandler2;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::util::XCloseable;
using ::com::sun::star::document::XViewDataSupplier;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::frame::XController2;
using ::com::sun::star::frame::XModel2;

namespace {

class SfxFrameLoader_Impl : public ::cppu::WeakImplHelper< css::frame::XSynchronousFrameLoader, css::lang::XServiceInfo >
{
    css::uno::Reference < css::uno::XComponentContext >  m_aContext;

public:
    explicit SfxFrameLoader_Impl( const css::uno::Reference < css::uno::XComponentContext >& _rxContext );

    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;


    // XSynchronousFrameLoader

    virtual sal_Bool SAL_CALL load( const css::uno::Sequence< css::beans::PropertyValue >& _rArgs, const css::uno::Reference< css::frame::XFrame >& _rxFrame ) override;
    virtual void SAL_CALL cancel() override;

protected:
    virtual                 ~SfxFrameLoader_Impl() override;

private:
    std::shared_ptr<const SfxFilter>    impl_getFilterFromServiceName_nothrow(
                            const OUString& i_rServiceName
                        ) const;

    static OUString     impl_askForFilter_nothrow(
                            const css::uno::Reference< css::task::XInteractionHandler >& i_rxHandler,
                            const OUString& i_rDocumentURL
                        );

    std::shared_ptr<const SfxFilter>    impl_detectFilterForURL(
                            const OUString& _rURL,
                            const ::comphelper::NamedValueCollection& i_rDescriptor,
                            const SfxFilterMatcher& rMatcher
                        ) const;

    static bool         impl_createNewDocWithSlotParam(
                            const sal_uInt16 _nSlotID,
                            const css::uno::Reference< css::frame::XFrame >& i_rxFrame,
                            const bool i_bHidden
                        );

    void                impl_determineFilter(
                                  ::comphelper::NamedValueCollection& io_rDescriptor
                        ) const;

    bool                impl_determineTemplateDocument(
                            ::comphelper::NamedValueCollection& io_rDescriptor
                        ) const;

    static sal_uInt16   impl_findSlotParam(
                            const OUString& i_rFactoryURL
                        );

    static SfxObjectShellRef   impl_findObjectShell(
                            const css::uno::Reference< css::frame::XModel2 >& i_rxDocument
                        );

    static void         impl_handleCaughtError_nothrow(
                            const css::uno::Any& i_rCaughtError,
                            const ::comphelper::NamedValueCollection& i_rDescriptor
                        );

    static void         impl_removeLoaderArguments(
                            ::comphelper::NamedValueCollection& io_rDescriptor
                        );

    static SfxInterfaceId impl_determineEffectiveViewId_nothrow(
                            const SfxObjectShell& i_rDocument,
                            const ::comphelper::NamedValueCollection& i_rDescriptor
                        );

    static ::comphelper::NamedValueCollection
                        impl_extractViewCreationArgs(
                                  ::comphelper::NamedValueCollection& io_rDescriptor
                        );

    static css::uno::Reference< css::frame::XController2 >
                        impl_createDocumentView(
                            const css::uno::Reference< css::frame::XModel2 >& i_rModel,
                            const css::uno::Reference< css::frame::XFrame >& i_rFrame,
                            const ::comphelper::NamedValueCollection& i_rViewFactoryArgs,
                            const OUString& i_rViewName
                        );
};

SfxFrameLoader_Impl::SfxFrameLoader_Impl( const Reference< css::uno::XComponentContext >& _rxContext )
    :m_aContext( _rxContext )
{
}

SfxFrameLoader_Impl::~SfxFrameLoader_Impl()
{
}


std::shared_ptr<const SfxFilter> SfxFrameLoader_Impl::impl_detectFilterForURL( const OUString& sURL,
        const ::comphelper::NamedValueCollection& i_rDescriptor, const SfxFilterMatcher& rMatcher ) const
{
    OUString sFilter;
    try
    {
        if ( sURL.isEmpty() )
            return nullptr;

        Reference< XTypeDetection > xDetect(
            m_aContext->getServiceManager()->createInstanceWithContext("com.sun.star.document.TypeDetection", m_aContext),
            UNO_QUERY_THROW);

        ::comphelper::NamedValueCollection aNewArgs;
        aNewArgs.put( "URL", sURL );

        if ( i_rDescriptor.has( "InteractionHandler" ) )
            aNewArgs.put( "InteractionHandler", i_rDescriptor.get( "InteractionHandler" ) );
        if ( i_rDescriptor.has( "StatusIndicator" ) )
            aNewArgs.put( "StatusIndicator", i_rDescriptor.get( "StatusIndicator" ) );

        Sequence< PropertyValue > aQueryArgs( aNewArgs.getPropertyValues() );
        OUString sType = xDetect->queryTypeByDescriptor( aQueryArgs, true );
        if ( !sType.isEmpty() )
        {
            std::shared_ptr<const SfxFilter> pFilter = rMatcher.GetFilter4EA( sType );
            if ( pFilter )
                sFilter = pFilter->GetName();
        }
    }
    catch ( const RuntimeException& )
    {
        throw;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("sfx.view");
        sFilter.clear();
    }

    std::shared_ptr<const SfxFilter> pFilter;
    if (!sFilter.isEmpty())
        pFilter = rMatcher.GetFilter4FilterName(sFilter);
    return pFilter;
}


std::shared_ptr<const SfxFilter> SfxFrameLoader_Impl::impl_getFilterFromServiceName_nothrow( const OUString& i_rServiceName ) const
{
    try
    {
        ::comphelper::NamedValueCollection aQuery;
        aQuery.put( "DocumentService", i_rServiceName );

        const Reference< XContainerQuery > xQuery(
            m_aContext->getServiceManager()->createInstanceWithContext("com.sun.star.document.FilterFactory", m_aContext),
            UNO_QUERY_THROW );

        const SfxFilterMatcher& rMatcher = SfxGetpApp()->GetFilterMatcher();
        const SfxFilterFlags nMust = SfxFilterFlags::IMPORT;
        const SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED;

        Reference < XEnumeration > xEnum( xQuery->createSubSetEnumerationByProperties(
            aQuery.getNamedValues() ), UNO_SET_THROW );
        while ( xEnum->hasMoreElements() )
        {
            ::comphelper::NamedValueCollection aType( xEnum->nextElement() );
            OUString sFilterName = aType.getOrDefault( "Name", OUString() );
            if ( sFilterName.isEmpty() )
                continue;

            std::shared_ptr<const SfxFilter> pFilter = rMatcher.GetFilter4FilterName( sFilterName );
            if ( !pFilter )
                continue;

            SfxFilterFlags nFlags = pFilter->GetFilterFlags();
            if  (   ( ( nFlags & nMust ) == nMust )
                &&  ( ( nFlags & nDont ) == SfxFilterFlags::NONE )
                )
            {
                return pFilter;
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("sfx.view");
    }
    return nullptr;
}


OUString SfxFrameLoader_Impl::impl_askForFilter_nothrow( const Reference< XInteractionHandler >& i_rxHandler,
                                                                 const OUString& i_rDocumentURL )
{
    ENSURE_OR_THROW( i_rxHandler.is(), "invalid interaction handler" );

    OUString sFilterName;
    try
    {
        ::framework::RequestFilterSelect aRequest( i_rDocumentURL );
        i_rxHandler->handle( aRequest.GetRequest() );
        if( !aRequest.isAbort() )
            sFilterName = aRequest.getFilter();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("sfx.view");
    }

    return sFilterName;
}

bool lcl_getDispatchResult( const SfxPoolItem* _pResult )
{
    if ( !_pResult )
        return false;

    // default must be set to true, because some return values
    // can't be checked, but nonetheless indicate "success"!
    bool bSuccess = true;

    // On the other side some special slots return a boolean state,
    // which can be set to FALSE.
    const SfxBoolItem *pItem = dynamic_cast<const SfxBoolItem*>( _pResult  );
    if ( pItem )
        bSuccess = pItem->GetValue();

    return bSuccess;
}

bool SfxFrameLoader_Impl::impl_createNewDocWithSlotParam( const sal_uInt16 _nSlotID, const Reference< XFrame >& i_rxFrame,
                                                              const bool i_bHidden )
{
    SfxRequest aRequest( _nSlotID, SfxCallMode::SYNCHRON, SfxGetpApp()->GetPool() );
    aRequest.AppendItem( SfxUnoFrameItem( SID_FILLFRAME, i_rxFrame ) );
    if ( i_bHidden )
        aRequest.AppendItem( SfxBoolItem( SID_HIDDEN, true ) );
    return lcl_getDispatchResult( SfxGetpApp()->ExecuteSlot( aRequest ) );
}


void SfxFrameLoader_Impl::impl_determineFilter( ::comphelper::NamedValueCollection& io_rDescriptor ) const
{
    const OUString     sURL         = io_rDescriptor.getOrDefault( "URL",                OUString() );
    const OUString     sTypeName    = io_rDescriptor.getOrDefault( "TypeName",           OUString() );
    const OUString     sFilterName  = io_rDescriptor.getOrDefault( "FilterName",         OUString() );
    const OUString     sServiceName = io_rDescriptor.getOrDefault( "DocumentService",    OUString() );
    const Reference< XInteractionHandler >
                              xInteraction = io_rDescriptor.getOrDefault( "InteractionHandler", Reference< XInteractionHandler >() );

    const SfxFilterMatcher& rMatcher = SfxGetpApp()->GetFilterMatcher();
    std::shared_ptr<const SfxFilter> pFilter;

    // get filter by its name directly ...
    if ( !sFilterName.isEmpty() )
        pFilter = rMatcher.GetFilter4FilterName( sFilterName );

    // or search the preferred filter for the detected type ...
    if ( !pFilter && !sTypeName.isEmpty() )
        pFilter = rMatcher.GetFilter4EA( sTypeName );

    // or use given document service for detection, too
    if ( !pFilter && !sServiceName.isEmpty() )
        pFilter = impl_getFilterFromServiceName_nothrow( sServiceName );

    // or use interaction to ask user for right filter.
    if ( !pFilter && xInteraction.is() && !sURL.isEmpty() )
    {
        OUString sSelectedFilter = impl_askForFilter_nothrow( xInteraction, sURL );
        if ( !sSelectedFilter.isEmpty() )
            pFilter = rMatcher.GetFilter4FilterName( sSelectedFilter );
    }

    if ( !pFilter )
        return;

    io_rDescriptor.put( "FilterName", pFilter->GetFilterName() );

    // If detected filter indicates using of an own template format
    // add property "AsTemplate" to descriptor. But suppress this step
    // if such property already exists.
    if ( pFilter->IsOwnTemplateFormat() && !io_rDescriptor.has( "AsTemplate" ) )
        io_rDescriptor.put( "AsTemplate", true );

    // The DocumentService property will finally be used to determine the document type to create, so
    // override it with the service name as indicated by the found filter.
    io_rDescriptor.put( "DocumentService", pFilter->GetServiceName() );
}


SfxObjectShellRef SfxFrameLoader_Impl::impl_findObjectShell( const Reference< XModel2 >& i_rxDocument )
{
    for ( SfxObjectShell* pDoc = SfxObjectShell::GetFirst( nullptr, false ); pDoc;
                                    pDoc = SfxObjectShell::GetNext( *pDoc, nullptr, false ) )
    {
        if ( i_rxDocument == pDoc->GetModel() )
        {
            return pDoc;
        }
    }

    SAL_WARN( "sfx.view", "SfxFrameLoader_Impl::impl_findObjectShell: model is not based on SfxObjectShell - wrong frame loader usage!" );
    return nullptr;
}


bool SfxFrameLoader_Impl::impl_determineTemplateDocument( ::comphelper::NamedValueCollection& io_rDescriptor ) const
{
    try
    {
        const OUString sTemplateRegioName = io_rDescriptor.getOrDefault( "TemplateRegionName", OUString() );
        const OUString sTemplateName      = io_rDescriptor.getOrDefault( "TemplateName",       OUString() );
        const OUString sServiceName       = io_rDescriptor.getOrDefault( "DocumentService",    OUString() );
        const OUString sURL               = io_rDescriptor.getOrDefault( "URL",                OUString() );

        // determine the full URL of the template to use, if any
        OUString sTemplateURL;
        if ( !sTemplateRegioName.isEmpty() && !sTemplateName.isEmpty() )
        {
            SfxDocumentTemplates aTmpFac;
            aTmpFac.GetFull( sTemplateRegioName, sTemplateName, sTemplateURL );
        }
        else
        {
            if ( !sServiceName.isEmpty() )
                sTemplateURL = SfxObjectFactory::GetStandardTemplate( sServiceName );
            else
                sTemplateURL = SfxObjectFactory::GetStandardTemplate( SfxObjectShell::GetServiceNameFromFactory( sURL ) );
        }

        if ( !sTemplateURL.isEmpty() )
        {
            // detect the filter for the template. Might still be NULL (if the template is broken, or does not
            // exist, or some such), but this is handled by our caller the same way as if no template/URL was present.
            std::shared_ptr<const SfxFilter> pTemplateFilter = impl_detectFilterForURL( sTemplateURL, io_rDescriptor, SfxGetpApp()->GetFilterMatcher() );
            if ( pTemplateFilter )
            {
                // load the template document, but, well, "as template"
                io_rDescriptor.put( "FilterName", pTemplateFilter->GetName() );
                io_rDescriptor.put( "FileName", sTemplateURL );
                io_rDescriptor.put( "AsTemplate", true );

                // #i21583#
                // the DocumentService property will finally be used to create the document. Thus, override any possibly
                // present value with the document service of the template.
                io_rDescriptor.put( "DocumentService", pTemplateFilter->GetServiceName() );
                return true;
            }
        }
    }
    catch (...)
    {
    }
    return false;
}


sal_uInt16 SfxFrameLoader_Impl::impl_findSlotParam( const OUString& i_rFactoryURL )
{
    OUString sSlotParam;
    const sal_Int32 nParamPos = i_rFactoryURL.indexOf( '?' );
    if ( nParamPos >= 0 )
    {
        // currently only the "slot" parameter is supported
        const sal_Int32 nSlotPos = i_rFactoryURL.indexOf( "slot=", nParamPos );
        if ( nSlotPos > 0 )
            sSlotParam = i_rFactoryURL.copy( nSlotPos + 5 );
    }

    if ( !sSlotParam.isEmpty() )
        return sal_uInt16( sSlotParam.toInt32() );

    return 0;
}


void SfxFrameLoader_Impl::impl_handleCaughtError_nothrow( const Any& i_rCaughtError, const ::comphelper::NamedValueCollection& i_rDescriptor )
{
    try
    {
        const Reference< XInteractionHandler > xInteraction =
            i_rDescriptor.getOrDefault( "InteractionHandler", Reference< XInteractionHandler >() );
        if ( !xInteraction.is() )
            return;
        ::rtl::Reference< ::comphelper::OInteractionRequest > pRequest( new ::comphelper::OInteractionRequest( i_rCaughtError ) );
        ::rtl::Reference< ::comphelper::OInteractionApprove > pApprove( new ::comphelper::OInteractionApprove );
        pRequest->addContinuation( pApprove.get() );

        const Reference< XInteractionHandler2 > xHandler( xInteraction, UNO_QUERY );
    #if OSL_DEBUG_LEVEL > 0
        const bool bHandled =
    #endif
        xHandler.is() && xHandler->handleInteractionRequest( pRequest.get() );

    #if OSL_DEBUG_LEVEL > 0
        if ( !bHandled )
            // the interaction handler couldn't deal with this error
            // => report it as assertion, at least (done in the DBG_UNHANDLED_EXCEPTION below)
            ::cppu::throwException( i_rCaughtError );
    #endif
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("sfx.view");
    }
}


void SfxFrameLoader_Impl::impl_removeLoaderArguments( ::comphelper::NamedValueCollection& io_rDescriptor )
{
    // remove the arguments which are for the loader only, and not for a call to attachResource
    io_rDescriptor.remove( "StatusIndicator" );
    io_rDescriptor.remove( "Model" );
}


::comphelper::NamedValueCollection SfxFrameLoader_Impl::impl_extractViewCreationArgs( ::comphelper::NamedValueCollection& io_rDescriptor )
{
    static const char* pKnownViewArgs[] = {
        "JumpMark",
        "PickListEntry"
    };

    ::comphelper::NamedValueCollection aViewArgs;
    for (const char* pKnownViewArg : pKnownViewArgs)
    {
        if ( io_rDescriptor.has( pKnownViewArg ) )
        {
            aViewArgs.put( pKnownViewArg, io_rDescriptor.get( pKnownViewArg ) );
            io_rDescriptor.remove( pKnownViewArg );
        }
    }
    return aViewArgs;
}


SfxInterfaceId SfxFrameLoader_Impl::impl_determineEffectiveViewId_nothrow( const SfxObjectShell& i_rDocument, const ::comphelper::NamedValueCollection& i_rDescriptor )
{
    SfxInterfaceId nViewId(i_rDescriptor.getOrDefault( "ViewId", sal_Int16( 0 ) ));
    try
    {
        if ( nViewId == SFX_INTERFACE_NONE )
        do
        {
            Reference< XViewDataSupplier > xViewDataSupplier( i_rDocument.GetModel(), UNO_QUERY );
            Reference< XIndexAccess > xViewData;
            if ( xViewDataSupplier.is() )
                xViewData.set( xViewDataSupplier->getViewData() );

            if ( !xViewData.is() || ( xViewData->getCount() == 0 ) )
                // no view data stored together with the model
                break;

            // obtain the ViewID from the view data
            Sequence< PropertyValue > aViewData;
            if ( !( xViewData->getByIndex( 0 ) >>= aViewData ) )
                break;

            ::comphelper::NamedValueCollection aNamedViewData( aViewData );
            OUString sViewId = aNamedViewData.getOrDefault( "ViewId", OUString() );
            if ( sViewId.isEmpty() )
                break;

            // somewhat weird convention here ... in the view data, the ViewId is a string, effectively describing
            // a view name. In the document load descriptor, the ViewId is in fact the numeric ID.

            SfxViewFactory* pViewFactory = i_rDocument.GetFactory().GetViewFactoryByViewName( sViewId );
            if ( pViewFactory )
                nViewId = pViewFactory->GetOrdinal();
        }
        while ( false );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("sfx.view");
    }

    if ( nViewId == SFX_INTERFACE_NONE )
        nViewId = i_rDocument.GetFactory().GetViewFactory().GetOrdinal();
    return nViewId;
}


Reference< XController2 > SfxFrameLoader_Impl::impl_createDocumentView( const Reference< XModel2 >& i_rModel,
        const Reference< XFrame >& i_rFrame, const ::comphelper::NamedValueCollection& i_rViewFactoryArgs,
        const OUString& i_rViewName )
{
    // let the model create a new controller
    const Reference< XController2 > xController( i_rModel->createViewController(
        i_rViewName,
        i_rViewFactoryArgs.getPropertyValues(),
        i_rFrame
    ), UNO_SET_THROW );

    // introduce model/view/controller to each other
    xController->attachModel( i_rModel.get() );
    i_rModel->connectController( xController.get() );
    i_rFrame->setComponent( xController->getComponentWindow(), xController.get() );
    xController->attachFrame( i_rFrame );
    i_rModel->setCurrentController( xController.get() );

    return xController;
}


sal_Bool SAL_CALL SfxFrameLoader_Impl::load( const Sequence< PropertyValue >& rArgs,
                                             const Reference< XFrame >& _rTargetFrame )
{
    ENSURE_OR_THROW( _rTargetFrame.is(), "illegal NULL frame" );

    SolarMutexGuard aGuard;

    SAL_INFO( "sfx.view", "SfxFrameLoader::load" );

    ::comphelper::NamedValueCollection aDescriptor( rArgs );

    // ensure the descriptor contains a referrer
    if ( !aDescriptor.has( "Referer" ) )
        aDescriptor.put( "Referer", OUString() );

    // did the caller already pass a model?
    Reference< XModel2 > xModel = aDescriptor.getOrDefault( "Model", Reference< XModel2 >() );
    const bool bExternalModel = xModel.is();

    // check for factory URLs to create a new doc, instead of loading one
    const OUString sURL = aDescriptor.getOrDefault( "URL", OUString() );
    const bool bIsFactoryURL = sURL.startsWith( "private:factory/" );
    bool bInitNewModel = bIsFactoryURL;
    if ( bIsFactoryURL && !bExternalModel )
    {
        const OUString sFactory = sURL.copy( sizeof( "private:factory/" ) -1 );
        // special handling for some weird factory URLs a la private:factory/swriter?slot=21053
        const sal_uInt16 nSlotParam = impl_findSlotParam( sFactory );
        if ( nSlotParam != 0 )
        {
            return impl_createNewDocWithSlotParam( nSlotParam, _rTargetFrame, aDescriptor.getOrDefault( "Hidden", false ) );
        }

        const bool bDescribesValidTemplate = impl_determineTemplateDocument( aDescriptor );
        if ( bDescribesValidTemplate )
        {
            // if the media descriptor allowed us to determine a template document to create the new document
            // from, then do not init a new document model from scratch (below), but instead load the
            // template document
            bInitNewModel = false;
        }
        else
        {
            const OUString sServiceName = SfxObjectShell::GetServiceNameFromFactory( sFactory );
            aDescriptor.put( "DocumentService", sServiceName );
        }
    }
    else
    {
        // compatibility
        aDescriptor.put( "FileName", aDescriptor.get( "URL" ) );
    }

    bool bLoadSuccess = false;
    try
    {
        // extract view relevant arguments from the loader args
        ::comphelper::NamedValueCollection aViewCreationArgs( impl_extractViewCreationArgs( aDescriptor ) );

        // no model passed from outside? => create one from scratch
        if ( !bExternalModel )
        {
            bool bInternalFilter = aDescriptor.getOrDefault<OUString>("FilterProvider", OUString()).isEmpty();

            if (bInternalFilter && !bInitNewModel)
            {
                // Ensure that the current SfxFilter instance is loaded before
                // going further.  We don't need to do this for external
                // filter providers.
                impl_determineFilter(aDescriptor);
            }

            // create the new doc
            const OUString sServiceName = aDescriptor.getOrDefault( "DocumentService", OUString() );
            xModel.set( m_aContext->getServiceManager()->createInstanceWithContext(sServiceName, m_aContext), UNO_QUERY_THROW );

            // load resp. init it
            const Reference< XLoadable > xLoadable( xModel, UNO_QUERY_THROW );
            if ( bInitNewModel )
            {
                xLoadable->initNew();

                impl_removeLoaderArguments( aDescriptor );
                xModel->attachResource( OUString(), aDescriptor.getPropertyValues() );
            }
            else
            {
                xLoadable->load( aDescriptor.getPropertyValues() );
            }
        }
        else
        {
            // tell the doc its (current) load args.
            impl_removeLoaderArguments( aDescriptor );
            xModel->attachResource( xModel->getURL(), aDescriptor.getPropertyValues() );
        }

        // get the SfxObjectShell (still needed at the moment)
        // SfxObjectShellRef is used here ( instead of ...Lock ) since the model is closed below if necessary
        // SfxObjectShellLock would be even dangerous here, since the lifetime control should be done outside in case of success
        const SfxObjectShellRef xDoc = impl_findObjectShell( xModel );
        ENSURE_OR_THROW( xDoc.is(), "no SfxObjectShell for the given model" );

        // ensure the ID of the to-be-created view is in the descriptor, if possible
        const SfxInterfaceId nViewId = impl_determineEffectiveViewId_nothrow( *xDoc, aDescriptor );
        const sal_Int16 nViewNo = xDoc->GetFactory().GetViewNo_Impl( nViewId, 0 );
        const OUString sViewName( xDoc->GetFactory().GetViewFactory( nViewNo ).GetAPIViewName() );

        // plug the document into the frame
        Reference<XController2> xController =
            impl_createDocumentView( xModel, _rTargetFrame, aViewCreationArgs, sViewName );

        Reference<lang::XInitialization> xInit(xController, UNO_QUERY);
        if (xInit.is())
        {
            uno::Sequence<uno::Any> aArgs; // empty for now.
            xInit->initialize(aArgs);
        }

        bLoadSuccess = true;
    }
    catch ( Exception& )
    {
        const Any aError( ::cppu::getCaughtException() );
        if ( !aDescriptor.getOrDefault( "Silent", false ) )
            impl_handleCaughtError_nothrow( aError, aDescriptor );
    }

    // if loading was not successful, close the document
    if ( !bLoadSuccess && !bExternalModel )
    {
        try
        {
            const Reference< XCloseable > xCloseable( xModel, UNO_QUERY_THROW );
            xCloseable->close( true );
        }
        catch ( Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("sfx.view");
        }
    }

    return bLoadSuccess;
}

void SfxFrameLoader_Impl::cancel()
{
}

/* XServiceInfo */
OUString SAL_CALL SfxFrameLoader_Impl::getImplementationName()
{
    return OUString("com.sun.star.comp.office.FrameLoader");
}

/* XServiceInfo */
sal_Bool SAL_CALL SfxFrameLoader_Impl::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL SfxFrameLoader_Impl::getSupportedServiceNames()
{
    Sequence< OUString > seqServiceNames( 2 );
    seqServiceNames.getArray() [0] = "com.sun.star.frame.SynchronousFrameLoader";
    seqServiceNames.getArray() [1] = "com.sun.star.frame.OfficeFrameLoader";
    return seqServiceNames ;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_office_FrameLoader_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SfxFrameLoader_Impl(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
