/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <sal/macros.h>
#include "objshimp.hxx"
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
#include <sfx2/sfx.hrc>
#include <sfx2/sfxsids.hrc>
#include <sfx2/sfxuno.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfac.hxx>

#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionHandler2.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#include <com/sun/star/frame/XController2.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <comphelper/interaction.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <framework/interaction.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.h>
#include <sot/storinfo.hxx>
#include <svtools/ehdl.hxx>
#include <svl/eitem.hxx>
#include <svl/itemset.hxx>
#include <unotools/moduleoptions.hxx>
#include <svtools/sfxecode.hxx>
#include <svl/stritem.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <ucbhelper/simpleinteractionrequest.hxx>
#include <osl/mutex.hxx>

using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::container::XContainerQuery;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::document::XTypeDetection;
using ::com::sun::star::frame::XFrame;
using ::com::sun::star::frame::XLoadable;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::task::XInteractionHandler;
using ::com::sun::star::task::XInteractionHandler2;
using ::com::sun::star::task::XInteractionRequest;
using ::com::sun::star::task::XStatusIndicator;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::util::XCloseable;
using ::com::sun::star::document::XViewDataSupplier;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::frame::XController2;
using ::com::sun::star::frame::XController;
using ::com::sun::star::frame::XModel2;

namespace {

class SfxFrameLoader_Impl : public ::cppu::WeakImplHelper2< css::frame::XSynchronousFrameLoader, css::lang::XServiceInfo >
{
    css::uno::Reference < css::uno::XComponentContext >  m_aContext;

public:
    SfxFrameLoader_Impl( const css::uno::Reference < css::uno::XComponentContext >& _rxContext );

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException);

    
    
    
    virtual sal_Bool SAL_CALL load( const css::uno::Sequence< css::beans::PropertyValue >& _rArgs, const css::uno::Reference< css::frame::XFrame >& _rxFrame ) throw( css::uno::RuntimeException );
    virtual void SAL_CALL cancel() throw( css::uno::RuntimeException );

protected:
    virtual                 ~SfxFrameLoader_Impl();

private:
    const SfxFilter*    impl_getFilterFromServiceName_nothrow(
                            const OUString& i_rServiceName
                        ) const;

    OUString     impl_askForFilter_nothrow(
                            const css::uno::Reference< css::task::XInteractionHandler >& i_rxHandler,
                            const OUString& i_rDocumentURL
                        ) const;

    const SfxFilter*    impl_detectFilterForURL(
                            const OUString& _rURL,
                            const ::comphelper::NamedValueCollection& i_rDescriptor,
                            const SfxFilterMatcher& rMatcher
                        ) const;

    sal_Bool            impl_createNewDocWithSlotParam(
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

    sal_uInt16              impl_findSlotParam(
                            const OUString& i_rFactoryURL
                        ) const;

    SfxObjectShellRef   impl_findObjectShell(
                            const css::uno::Reference< css::frame::XModel2 >& i_rxDocument
                        ) const;

    void                impl_lockHiddenDocument(
                                  SfxObjectShell& i_rDocument,
                            const ::comphelper::NamedValueCollection& i_rDescriptor
                        ) const;

    void                impl_handleCaughtError_nothrow(
                            const css::uno::Any& i_rCaughtError,
                            const ::comphelper::NamedValueCollection& i_rDescriptor
                        ) const;

    void                impl_removeLoaderArguments(
                            ::comphelper::NamedValueCollection& io_rDescriptor
                        );

    sal_Int16           impl_determineEffectiveViewId_nothrow(
                            const SfxObjectShell& i_rDocument,
                            const ::comphelper::NamedValueCollection& i_rDescriptor
                        );

    ::comphelper::NamedValueCollection
                        impl_extractViewCreationArgs(
                                  ::comphelper::NamedValueCollection& io_rDescriptor
                        );

    css::uno::Reference< css::frame::XController2 >
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


const SfxFilter* SfxFrameLoader_Impl::impl_detectFilterForURL( const OUString& sURL,
        const ::comphelper::NamedValueCollection& i_rDescriptor, const SfxFilterMatcher& rMatcher ) const
{
    OUString sFilter;
    try
    {
        if ( sURL.isEmpty() )
            return 0;

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
        OUString sType = xDetect->queryTypeByDescriptor( aQueryArgs, sal_True );
        if ( !sType.isEmpty() )
        {
            const SfxFilter* pFilter = rMatcher.GetFilter4EA( sType );
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
        DBG_UNHANDLED_EXCEPTION();
        sFilter = OUString();
    }

    const SfxFilter* pFilter = 0;
    if (!sFilter.isEmpty())
        pFilter = rMatcher.GetFilter4FilterName(sFilter);
    return pFilter;
}


const SfxFilter* SfxFrameLoader_Impl::impl_getFilterFromServiceName_nothrow( const OUString& i_rServiceName ) const
{
    try
    {
        ::comphelper::NamedValueCollection aQuery;
        aQuery.put( "DocumentService", i_rServiceName );

        const Reference< XContainerQuery > xQuery(
            m_aContext->getServiceManager()->createInstanceWithContext("com.sun.star.document.FilterFactory", m_aContext),
            UNO_QUERY_THROW );

        const SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();
        const SfxFilterFlags nMust = SFX_FILTER_IMPORT;
        const SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED;

        Reference < XEnumeration > xEnum( xQuery->createSubSetEnumerationByProperties(
            aQuery.getNamedValues() ), UNO_SET_THROW );
        while ( xEnum->hasMoreElements() )
        {
            ::comphelper::NamedValueCollection aType( xEnum->nextElement() );
            OUString sFilterName = aType.getOrDefault( "Name", OUString() );
            if ( sFilterName.isEmpty() )
                continue;

            const SfxFilter* pFilter = rMatcher.GetFilter4FilterName( sFilterName );
            if ( !pFilter )
                continue;

            SfxFilterFlags nFlags = pFilter->GetFilterFlags();
            if  (   ( ( nFlags & nMust ) == nMust )
                &&  ( ( nFlags & nDont ) == 0 )
                )
            {
                return pFilter;
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return NULL;
}


OUString SfxFrameLoader_Impl::impl_askForFilter_nothrow( const Reference< XInteractionHandler >& i_rxHandler,
                                                                 const OUString& i_rDocumentURL ) const
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
        DBG_UNHANDLED_EXCEPTION();
    }

    return sFilterName;
}


namespace
{
    sal_Bool lcl_getDispatchResult( const SfxPoolItem* _pResult )
    {
        if ( !_pResult )
            return sal_False;

        
        
        sal_Bool bSuccess = sal_True;

        
        
        SfxBoolItem *pItem = PTR_CAST( SfxBoolItem, _pResult );
        if ( pItem )
            bSuccess = pItem->GetValue();

        return bSuccess;
    }
}


sal_Bool SfxFrameLoader_Impl::impl_createNewDocWithSlotParam( const sal_uInt16 _nSlotID, const Reference< XFrame >& i_rxFrame,
                                                              const bool i_bHidden )
{
    SfxRequest aRequest( _nSlotID, SFX_CALLMODE_SYNCHRON, SFX_APP()->GetPool() );
    aRequest.AppendItem( SfxUnoFrameItem( SID_FILLFRAME, i_rxFrame ) );
    if ( i_bHidden )
        aRequest.AppendItem( SfxBoolItem( SID_HIDDEN, true ) );
    return lcl_getDispatchResult( SFX_APP()->ExecuteSlot( aRequest ) );
}


void SfxFrameLoader_Impl::impl_determineFilter( ::comphelper::NamedValueCollection& io_rDescriptor ) const
{
    const OUString     sURL         = io_rDescriptor.getOrDefault( "URL",                OUString() );
    const OUString     sTypeName    = io_rDescriptor.getOrDefault( "TypeName",           OUString() );
    const OUString     sFilterName  = io_rDescriptor.getOrDefault( "FilterName",         OUString() );
    const OUString     sServiceName = io_rDescriptor.getOrDefault( "DocumentService",    OUString() );
    const Reference< XInteractionHandler >
                              xInteraction = io_rDescriptor.getOrDefault( "InteractionHandler", Reference< XInteractionHandler >() );

    const SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();
    const SfxFilter* pFilter = NULL;

    
    if ( !sFilterName.isEmpty() )
        pFilter = rMatcher.GetFilter4FilterName( sFilterName );

    
    if ( !pFilter && !sTypeName.isEmpty() )
        pFilter = rMatcher.GetFilter4EA( sTypeName );

    
    if ( !pFilter && !sServiceName.isEmpty() )
        pFilter = impl_getFilterFromServiceName_nothrow( sServiceName );

    
    if ( !pFilter && xInteraction.is() && !sURL.isEmpty() )
    {
        OUString sSelectedFilter = impl_askForFilter_nothrow( xInteraction, sURL );
        if ( !sSelectedFilter.isEmpty() )
            pFilter = rMatcher.GetFilter4FilterName( sSelectedFilter );
    }

    if ( pFilter )
    {
        io_rDescriptor.put( "FilterName", OUString( pFilter->GetFilterName() ) );

        
        
        
        if ( pFilter->IsOwnTemplateFormat() && !io_rDescriptor.has( "AsTemplate" ) )
            io_rDescriptor.put( "AsTemplate", sal_Bool( sal_True ) );

        
        
        io_rDescriptor.put( "DocumentService", OUString( pFilter->GetServiceName() ) );
    }
}


SfxObjectShellRef SfxFrameLoader_Impl::impl_findObjectShell( const Reference< XModel2 >& i_rxDocument ) const
{
    for ( SfxObjectShell* pDoc = SfxObjectShell::GetFirst( NULL, sal_False ); pDoc; pDoc = SfxObjectShell::GetNext( *pDoc, NULL, sal_False ) )
    {
        if ( i_rxDocument == pDoc->GetModel() )
        {
            return pDoc;
        }
    }

    SAL_WARN( "sfx.view", "SfxFrameLoader_Impl::impl_findObjectShell: model is not based on SfxObjectShell - wrong frame loader usage!" );
    return NULL;
}


bool SfxFrameLoader_Impl::impl_determineTemplateDocument( ::comphelper::NamedValueCollection& io_rDescriptor ) const
{
    try
    {
        const OUString sTemplateRegioName = io_rDescriptor.getOrDefault( "TemplateRegionName", OUString() );
        const OUString sTemplateName      = io_rDescriptor.getOrDefault( "TemplateName",       OUString() );
        const OUString sServiceName       = io_rDescriptor.getOrDefault( "DocumentService",    OUString() );
        const OUString sURL               = io_rDescriptor.getOrDefault( "URL",                OUString() );

        
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
            
            
            const SfxFilter* pTemplateFilter = impl_detectFilterForURL( sTemplateURL, io_rDescriptor, SFX_APP()->GetFilterMatcher() );
            if ( pTemplateFilter )
            {
                
                io_rDescriptor.put( "FilterName", OUString( pTemplateFilter->GetName() ) );
                io_rDescriptor.put( "FileName", OUString( sTemplateURL ) );
                io_rDescriptor.put( "AsTemplate", sal_True );

                
                
                
                io_rDescriptor.put( "DocumentService", OUString( pTemplateFilter->GetServiceName() ) );
                return true;
            }
        }
    }
    catch (...)
    {
    }
    return false;
}


sal_uInt16 SfxFrameLoader_Impl::impl_findSlotParam( const OUString& i_rFactoryURL ) const
{
    OUString sSlotParam;
    const sal_Int32 nParamPos = i_rFactoryURL.indexOf( '?' );
    if ( nParamPos >= 0 )
    {
        
        const sal_Int32 nSlotPos = i_rFactoryURL.indexOf( "slot=", nParamPos );
        if ( nSlotPos > 0 )
            sSlotParam = i_rFactoryURL.copy( nSlotPos + 5 );
    }

    if ( !sSlotParam.isEmpty() )
        return sal_uInt16( sSlotParam.toInt32() );

    return 0;
}


void SfxFrameLoader_Impl::impl_handleCaughtError_nothrow( const Any& i_rCaughtError, const ::comphelper::NamedValueCollection& i_rDescriptor ) const
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
        const sal_Bool bHandled =
    #endif
        xHandler.is() && xHandler->handleInteractionRequest( pRequest.get() );

    #if OSL_DEBUG_LEVEL > 0
        if ( !bHandled )
            
            
            ::cppu::throwException( i_rCaughtError );
    #endif
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}


void SfxFrameLoader_Impl::impl_removeLoaderArguments( ::comphelper::NamedValueCollection& io_rDescriptor )
{
    
    io_rDescriptor.remove( "StatusIndicator" );
    io_rDescriptor.remove( "Model" );
}


::comphelper::NamedValueCollection SfxFrameLoader_Impl::impl_extractViewCreationArgs( ::comphelper::NamedValueCollection& io_rDescriptor )
{
    const sal_Char* pKnownViewArgs[] = {
        "JumpMark"
    };

    ::comphelper::NamedValueCollection aViewArgs;
    for ( size_t i=0; i < sizeof( pKnownViewArgs ) / sizeof( pKnownViewArgs[0] ); ++i )
    {
        if ( io_rDescriptor.has( pKnownViewArgs[i] ) )
        {
            aViewArgs.put( pKnownViewArgs[i], io_rDescriptor.get( pKnownViewArgs[i] ) );
            io_rDescriptor.remove( pKnownViewArgs[i] );
        }
    }
    return aViewArgs;
}


sal_Int16 SfxFrameLoader_Impl::impl_determineEffectiveViewId_nothrow( const SfxObjectShell& i_rDocument, const ::comphelper::NamedValueCollection& i_rDescriptor )
{
    sal_Int16 nViewId = i_rDescriptor.getOrDefault( "ViewId", sal_Int16( 0 ) );
    try
    {
        if ( nViewId == 0 ) do
        {
            Reference< XViewDataSupplier > xViewDataSupplier( i_rDocument.GetModel(), UNO_QUERY );
            Reference< XIndexAccess > xViewData;
            if ( xViewDataSupplier.is() )
                xViewData.set( xViewDataSupplier->getViewData() );

            if ( !xViewData.is() || ( xViewData->getCount() == 0 ) )
                
                break;

            
            Sequence< PropertyValue > aViewData;
            if ( !( xViewData->getByIndex( 0 ) >>= aViewData ) )
                break;

            ::comphelper::NamedValueCollection aNamedViewData( aViewData );
            OUString sViewId = aNamedViewData.getOrDefault( "ViewId", OUString() );
            if ( sViewId.isEmpty() )
                break;

            
            

            SfxViewFactory* pViewFactory = i_rDocument.GetFactory().GetViewFactoryByViewName( sViewId );
            if ( pViewFactory )
                nViewId = sal_Int16( pViewFactory->GetOrdinal() );
        }
        while ( false );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    if ( nViewId == 0 )
        nViewId = i_rDocument.GetFactory().GetViewFactory( 0 ).GetOrdinal();
    return nViewId;
}


Reference< XController2 > SfxFrameLoader_Impl::impl_createDocumentView( const Reference< XModel2 >& i_rModel,
        const Reference< XFrame >& i_rFrame, const ::comphelper::NamedValueCollection& i_rViewFactoryArgs,
        const OUString& i_rViewName )
{
    
    const Reference< XController2 > xController( i_rModel->createViewController(
        i_rViewName,
        i_rViewFactoryArgs.getPropertyValues(),
        i_rFrame
    ), UNO_SET_THROW );

    
    xController->attachModel( i_rModel.get() );
    i_rModel->connectController( xController.get() );
    i_rFrame->setComponent( xController->getComponentWindow(), xController.get() );
    xController->attachFrame( i_rFrame );
    i_rModel->setCurrentController( xController.get() );

    return xController;
}


sal_Bool SAL_CALL SfxFrameLoader_Impl::load( const Sequence< PropertyValue >& rArgs,
                                             const Reference< XFrame >& _rTargetFrame )
    throw( RuntimeException )
{
    ENSURE_OR_THROW( _rTargetFrame.is(), "illegal NULL frame" );

    SolarMutexGuard aGuard;

    SAL_INFO( "sfx.view", "SfxFrameLoader::load" );

    ::comphelper::NamedValueCollection aDescriptor( rArgs );

    
    if ( !aDescriptor.has( "Referer" ) )
        aDescriptor.put( "Referer", OUString() );

    
    Reference< XModel2 > xModel = aDescriptor.getOrDefault( "Model", Reference< XModel2 >() );
    const bool bExternalModel = xModel.is();

    
    const OUString sURL = aDescriptor.getOrDefault( "URL", OUString() );
    const bool bIsFactoryURL = sURL.startsWith( "private:factory/" );
    bool bInitNewModel = bIsFactoryURL;
    if ( bIsFactoryURL && !bExternalModel )
    {
        const OUString sFactory = sURL.copy( sizeof( "private:factory/" ) -1 );
        
        const sal_uInt16 nSlotParam = impl_findSlotParam( sFactory );
        if ( nSlotParam != 0 )
        {
            return impl_createNewDocWithSlotParam( nSlotParam, _rTargetFrame, aDescriptor.getOrDefault( "Hidden", false ) );
        }

        const bool bDescribesValidTemplate = impl_determineTemplateDocument( aDescriptor );
        if ( bDescribesValidTemplate )
        {
            
            
            
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
        
        aDescriptor.put( "FileName", aDescriptor.get( "URL" ) );
    }

    sal_Bool bLoadSuccess = sal_False;
    try
    {
        
        ::comphelper::NamedValueCollection aViewCreationArgs( impl_extractViewCreationArgs( aDescriptor ) );

        
        if ( !xModel.is() )
        {
            bool bInternalFilter = aDescriptor.getOrDefault<OUString>("FilterProvider", OUString()).isEmpty();

            if (bInternalFilter && !bInitNewModel)
            {
                
                
                
                impl_determineFilter(aDescriptor);
            }

            
            const OUString sServiceName = aDescriptor.getOrDefault( "DocumentService", OUString() );
            xModel.set( m_aContext->getServiceManager()->createInstanceWithContext(sServiceName, m_aContext), UNO_QUERY_THROW );

            
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
            
            impl_removeLoaderArguments( aDescriptor );
            xModel->attachResource( xModel->getURL(), aDescriptor.getPropertyValues() );
        }

        
        
        
        const SfxObjectShellRef xDoc = impl_findObjectShell( xModel );
        ENSURE_OR_THROW( xDoc.Is(), "no SfxObjectShell for the given model" );

        
        const sal_Int16 nViewId = impl_determineEffectiveViewId_nothrow( *xDoc, aDescriptor );
        const sal_Int16 nViewNo = xDoc->GetFactory().GetViewNo_Impl( nViewId, 0 );
        const OUString sViewName( xDoc->GetFactory().GetViewFactory( nViewNo ).GetAPIViewName() );

        
        impl_createDocumentView( xModel, _rTargetFrame, aViewCreationArgs, sViewName );
        bLoadSuccess = sal_True;
    }
    catch ( Exception& )
    {
        const Any aError( ::cppu::getCaughtException() );
        if ( !aDescriptor.getOrDefault( "Silent", sal_False ) )
            impl_handleCaughtError_nothrow( aError, aDescriptor );
    }

    
    if ( !bLoadSuccess && !bExternalModel )
    {
        try
        {
            const Reference< XCloseable > xCloseable( xModel, UNO_QUERY_THROW );
            xCloseable->close( sal_True );
        }
        catch ( Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    return bLoadSuccess;
}

void SfxFrameLoader_Impl::cancel() throw( RuntimeException )
{
}

/* XServiceInfo */
OUString SAL_CALL SfxFrameLoader_Impl::getImplementationName() throw( RuntimeException )
{
    return OUString("com.sun.star.comp.office.FrameLoader");
}
                                                                                                                                \
/* XServiceInfo */
sal_Bool SAL_CALL SfxFrameLoader_Impl::supportsService( const OUString& sServiceName ) throw( RuntimeException )
{
    return cppu::supportsService(this, sServiceName);
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL SfxFrameLoader_Impl::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > seqServiceNames( 2 );
    seqServiceNames.getArray() [0] = "com.sun.star.frame.SynchronousFrameLoader";
    seqServiceNames.getArray() [1] = "com.sun.star.frame.OfficeFrameLoader";
    return seqServiceNames ;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_office_FrameLoader_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SfxFrameLoader_Impl(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
