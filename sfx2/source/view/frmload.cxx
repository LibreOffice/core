/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sfx2.hxx"

#include <sal/macros.h>
#include "frmload.hxx"
#include "objshimp.hxx"
#include "viewfac.hxx"
#include "sfx2/app.hxx"
#include "sfx2/dispatch.hxx"
#include "sfx2/docfac.hxx"
#include "sfx2/docfile.hxx"
#include "sfx2/docfilt.hxx"
#include "sfx2/doctempl.hxx"
#include "sfx2/fcontnr.hxx"
#include "sfx2/frame.hxx"
#include "sfx2/request.hxx"
#include "sfx2/sfx.hrc"
#include "sfx2/sfxsids.hrc"
#include "sfx2/sfxuno.hxx"
#include "sfx2/viewfrm.hxx"
#include "sfx2/viewsh.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XInteractionHandler2.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
/** === end UNO includes === **/

#include <comphelper/interaction.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <framework/interaction.hxx>
#include <rtl/logfile.hxx>
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

/** === begin UNO using === **/
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
/** === end UNO using === **/

SfxFrameLoader_Impl::SfxFrameLoader_Impl( const Reference< XMultiServiceFactory >& _rxFactory )
    :m_aContext( _rxFactory )
{
}

SfxFrameLoader_Impl::~SfxFrameLoader_Impl()
{
}

// --------------------------------------------------------------------------------------------------------------------
const SfxFilter* SfxFrameLoader_Impl::impl_detectFilterForURL( const ::rtl::OUString& sURL,
        const ::comphelper::NamedValueCollection& i_rDescriptor, const SfxFilterMatcher& rMatcher ) const
{
    ::rtl::OUString sFilter;
    try
    {
        if ( !sURL.getLength() )
            return 0;

        Reference< XTypeDetection > xDetect(
            m_aContext.createComponent( "com.sun.star.document.TypeDetection" ),
            UNO_QUERY_THROW);

        ::comphelper::NamedValueCollection aNewArgs;
        aNewArgs.put( "URL", sURL );

        if ( i_rDescriptor.has( "InteractionHandler" ) )
            aNewArgs.put( "InteractionHandler", i_rDescriptor.get( "InteractionHandler" ) );
        if ( i_rDescriptor.has( "StatusIndicator" ) )
            aNewArgs.put( "StatusIndicator", i_rDescriptor.get( "StatusIndicator" ) );

        Sequence< PropertyValue > aQueryArgs( aNewArgs.getPropertyValues() );
        ::rtl::OUString sType = xDetect->queryTypeByDescriptor( aQueryArgs, sal_True );
        if ( sType.getLength() )
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
        sFilter = ::rtl::OUString();
    }

    const SfxFilter* pFilter = 0;
    if (sFilter.getLength())
        pFilter = rMatcher.GetFilter4FilterName(sFilter);
    return pFilter;
}

// --------------------------------------------------------------------------------------------------------------------
const SfxFilter* SfxFrameLoader_Impl::impl_getFilterFromServiceName_nothrow( const ::rtl::OUString& i_rServiceName ) const
{
    try
    {
        ::comphelper::NamedValueCollection aQuery;
        aQuery.put( "DocumentService", i_rServiceName );

        const Reference< XContainerQuery > xQuery(
            m_aContext.createComponent( "com.sun.star.document.FilterFactory" ),
            UNO_QUERY_THROW );

        const SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();
        const SfxFilterFlags nMust = SFX_FILTER_IMPORT;
        const SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED;

        Reference < XEnumeration > xEnum( xQuery->createSubSetEnumerationByProperties(
            aQuery.getNamedValues() ), UNO_SET_THROW );
        while ( xEnum->hasMoreElements() )
        {
            ::comphelper::NamedValueCollection aType( xEnum->nextElement() );
            ::rtl::OUString sFilterName = aType.getOrDefault( "Name", ::rtl::OUString() );
            if ( !sFilterName.getLength() )
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

// --------------------------------------------------------------------------------------------------------------------
::rtl::OUString SfxFrameLoader_Impl::impl_askForFilter_nothrow( const Reference< XInteractionHandler >& i_rxHandler,
                                                                 const ::rtl::OUString& i_rDocumentURL ) const
{
    ENSURE_OR_THROW( i_rxHandler.is(), "invalid interaction handler" );

    ::rtl::OUString sFilterName;
    try
    {
        ::framework::RequestFilterSelect* pRequest = new ::framework::RequestFilterSelect( i_rDocumentURL );
        Reference< XInteractionRequest > xRequest ( pRequest );
        i_rxHandler->handle( xRequest );
        if( !pRequest->isAbort() )
            sFilterName = pRequest->getFilter();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return sFilterName;
}

// --------------------------------------------------------------------------------------------------------------------
namespace
{
    sal_Bool lcl_getDispatchResult( const SfxPoolItem* _pResult )
    {
        if ( !_pResult )
            return sal_False;

        // default must be set to true, because some return values
        // cant be checked, but nonetheless indicate "success"!
        sal_Bool bSuccess = sal_True;

        // On the other side some special slots return a boolean state,
        // which can be set to FALSE.
        SfxBoolItem *pItem = PTR_CAST( SfxBoolItem, _pResult );
        if ( pItem )
            bSuccess = pItem->GetValue();

        return bSuccess;
    }
}

// --------------------------------------------------------------------------------------------------------------------
sal_Bool SfxFrameLoader_Impl::impl_createNewDocWithSlotParam( const USHORT _nSlotID, const Reference< XFrame >& i_rxFrame,
                                                              const bool i_bHidden )
{
    SfxRequest aRequest( _nSlotID, SFX_CALLMODE_SYNCHRON, SFX_APP()->GetPool() );
    aRequest.AppendItem( SfxUnoFrameItem( SID_FILLFRAME, i_rxFrame ) );
    if ( i_bHidden )
        aRequest.AppendItem( SfxBoolItem( SID_HIDDEN, TRUE ) );
    return lcl_getDispatchResult( SFX_APP()->ExecuteSlot( aRequest ) );
}

// --------------------------------------------------------------------------------------------------------------------
void SfxFrameLoader_Impl::impl_lockHiddenDocument( SfxObjectShell& i_rDocument, const ::comphelper::NamedValueCollection& i_rDescriptor ) const
{
    const sal_Bool bHidden = i_rDescriptor.getOrDefault( "Hidden", sal_False );
    if ( !bHidden )
        return;

    const SfxViewFrame* pExistingViewFrame = SfxViewFrame::GetFirst( &i_rDocument );
    if ( pExistingViewFrame )
        return;

    // the document is to be loaded hidden, and it is not yet displayed in any other frame
    // To prevent it from being closed when the loader returns, increase its OwnerLock
    // (the OwnerLock is normally increased by every frame in which the document is displayed, and by this loader)
    i_rDocument.RestoreNoDelete();
    i_rDocument.OwnerLock( TRUE );
    i_rDocument.Get_Impl()->bHiddenLockedByAPI = TRUE;
}

// --------------------------------------------------------------------------------------------------------------------
void SfxFrameLoader_Impl::impl_determineFilter( ::comphelper::NamedValueCollection& io_rDescriptor ) const
{
    const ::rtl::OUString     sURL         = io_rDescriptor.getOrDefault( "URL",                ::rtl::OUString() );
    const ::rtl::OUString     sTypeName    = io_rDescriptor.getOrDefault( "TypeName",           ::rtl::OUString() );
    const ::rtl::OUString     sFilterName  = io_rDescriptor.getOrDefault( "FilterName",         ::rtl::OUString() );
    const ::rtl::OUString     sServiceName = io_rDescriptor.getOrDefault( "DocumentService",    ::rtl::OUString() );
    const Reference< XInteractionHandler >
                              xInteraction = io_rDescriptor.getOrDefault( "InteractionHandler", Reference< XInteractionHandler >() );

    const SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();
    const SfxFilter* pFilter = NULL;

    // get filter by its name directly ...
    if ( sFilterName.getLength() )
        pFilter = rMatcher.GetFilter4FilterName( sFilterName );

    // or search the preferred filter for the detected type ...
    if ( !pFilter && sTypeName.getLength() )
        pFilter = rMatcher.GetFilter4EA( sTypeName );

    // or use given document service for detection, too
    if ( !pFilter && sServiceName.getLength() )
        pFilter = impl_getFilterFromServiceName_nothrow( sServiceName );

    // or use interaction to ask user for right filter.
    if ( !pFilter && xInteraction.is() && sURL.getLength() )
    {
        ::rtl::OUString sSelectedFilter = impl_askForFilter_nothrow( xInteraction, sURL );
        if ( sSelectedFilter.getLength() )
            pFilter = rMatcher.GetFilter4FilterName( sSelectedFilter );
    }

    if ( pFilter )
    {
        io_rDescriptor.put( "FilterName", ::rtl::OUString( pFilter->GetFilterName() ) );

        // If detected filter indicates using of an own template format
        // add property "AsTemplate" to descriptor. But suppress this step
        // if such property already exists.
        if ( pFilter->IsOwnTemplateFormat() && !io_rDescriptor.has( "AsTemplate" ) )
            io_rDescriptor.put( "AsTemplate", sal_Bool( sal_True ) );

        // The DocumentService property will finally be used to determine the document type to create, so
        // override it with the service name as indicated by the found filter.
        io_rDescriptor.put( "DocumentService", ::rtl::OUString( pFilter->GetServiceName() ) );
    }
}

// --------------------------------------------------------------------------------------------------------------------
SfxObjectShellLock SfxFrameLoader_Impl::impl_findObjectShell( const Reference< XModel2 >& i_rxDocument ) const
{
    for ( SfxObjectShell* pDoc = SfxObjectShell::GetFirst( NULL, FALSE ); pDoc; pDoc = SfxObjectShell::GetNext( *pDoc, NULL, FALSE ) )
    {
        if ( i_rxDocument == pDoc->GetModel() )
        {
            return pDoc;
        }
    }

    OSL_FAIL( "SfxFrameLoader_Impl::impl_findObjectShell: model is not based on SfxObjectShell - wrong frame loader usage!" );
    return NULL;
}

// --------------------------------------------------------------------------------------------------------------------
bool SfxFrameLoader_Impl::impl_determineTemplateDocument( ::comphelper::NamedValueCollection& io_rDescriptor ) const
{
    const ::rtl::OUString sTemplateRegioName = io_rDescriptor.getOrDefault( "TemplateRegionName", ::rtl::OUString() );
    const ::rtl::OUString sTemplateName      = io_rDescriptor.getOrDefault( "TemplateName",       ::rtl::OUString() );
    const ::rtl::OUString sServiceName       = io_rDescriptor.getOrDefault( "DocumentService",    ::rtl::OUString() );
    const ::rtl::OUString sURL               = io_rDescriptor.getOrDefault( "URL",                ::rtl::OUString() );

    // determine the full URL of the template to use, if any
    String sTemplateURL;
    if ( sTemplateRegioName.getLength() && sTemplateName.getLength() )
    {
        SfxDocumentTemplates aTmpFac;
        aTmpFac.GetFull( sTemplateRegioName, sTemplateName, sTemplateURL );
    }
    else
    {
        if ( sServiceName.getLength() )
            sTemplateURL = SfxObjectFactory::GetStandardTemplate( sServiceName );
        else
            sTemplateURL = SfxObjectFactory::GetStandardTemplate( SfxObjectShell::GetServiceNameFromFactory( sURL ) );
    }

    if ( sTemplateURL.Len() > 0 )
    {
        // detect the filter for the template. Might still be NULL (if the template is broken, or does not
        // exist, or some such), but this is handled by our caller the same way as if no template/URL was present.
        const SfxFilter* pTemplateFilter = impl_detectFilterForURL( sTemplateURL, io_rDescriptor, SFX_APP()->GetFilterMatcher() );
        if ( pTemplateFilter )
        {
            // load the template document, but, well, "as template"
            io_rDescriptor.put( "FilterName", ::rtl::OUString( pTemplateFilter->GetName() ) );
            io_rDescriptor.put( "FileName", ::rtl::OUString( sTemplateURL ) );
            io_rDescriptor.put( "AsTemplate", sal_True );

            // #i21583#
            // the DocumentService property will finally be used to create the document. Thus, override any possibly
            // present value with the document service of the template.
            io_rDescriptor.put( "DocumentService", ::rtl::OUString( pTemplateFilter->GetServiceName() ) );
            return true;
        }
    }
    return false;
}

// --------------------------------------------------------------------------------------------------------------------
USHORT SfxFrameLoader_Impl::impl_findSlotParam( const ::rtl::OUString& i_rFactoryURL ) const
{
    ::rtl::OUString sSlotParam;
    const sal_Int32 nParamPos = i_rFactoryURL.indexOf( '?' );
    if ( nParamPos >= 0 )
    {
        // currently only the "slot" parameter is supported
        const sal_Int32 nSlotPos = i_rFactoryURL.indexOfAsciiL( RTL_CONSTASCII_STRINGPARAM( "slot=" ), nParamPos );
        if ( nSlotPos > 0 )
            sSlotParam = i_rFactoryURL.copy( nSlotPos + 5 );
    }

    if ( sSlotParam.getLength() )
        return USHORT( sSlotParam.toInt32() );

    return 0;
}

// --------------------------------------------------------------------------------------------------------------------
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
            // the interaction handler couldn't deal with this error
            // => report it as assertion, at least (done in the DBG_UNHANDLED_EXCEPTION below)
            ::cppu::throwException( i_rCaughtError );
    #endif
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// --------------------------------------------------------------------------------------------------------------------
void SfxFrameLoader_Impl::impl_removeLoaderArguments( ::comphelper::NamedValueCollection& io_rDescriptor )
{
    // remove the arguments which are for the loader only, and not for a call to attachResource
    io_rDescriptor.remove( "StatusIndicator" );
    io_rDescriptor.remove( "Model" );
}

// --------------------------------------------------------------------------------------------------------------------
::comphelper::NamedValueCollection SfxFrameLoader_Impl::impl_extractViewCreationArgs( ::comphelper::NamedValueCollection& io_rDescriptor )
{
    const sal_Char* pKnownViewArgs[] = {
        "JumpMark"
    };

    ::comphelper::NamedValueCollection aViewArgs;
    for ( size_t i=0; i < SAL_N_ELEMENTS( pKnownViewArgs ); ++i )
    {
        if ( io_rDescriptor.has( pKnownViewArgs[i] ) )
        {
            aViewArgs.put( pKnownViewArgs[i], io_rDescriptor.get( pKnownViewArgs[i] ) );
            io_rDescriptor.remove( pKnownViewArgs[i] );
        }
    }
    return aViewArgs;
}

// --------------------------------------------------------------------------------------------------------------------
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
                // no view data stored together with the model
                break;

            // obtain the ViewID from the view data
            Sequence< PropertyValue > aViewData;
            if ( !( xViewData->getByIndex( 0 ) >>= aViewData ) )
                break;

            ::comphelper::NamedValueCollection aNamedViewData( aViewData );
            ::rtl::OUString sViewId = aNamedViewData.getOrDefault( "ViewId", ::rtl::OUString() );
            if ( !sViewId.getLength() )
                break;

            // somewhat weird convention here ... in the view data, the ViewId is a string, effectively describing
            // a view name. In the document load descriptor, the ViewId is in fact the numeric ID.

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

// --------------------------------------------------------------------------------------------------------------------
Reference< XController2 > SfxFrameLoader_Impl::impl_createDocumentView( const Reference< XModel2 >& i_rModel,
        const Reference< XFrame >& i_rFrame, const ::comphelper::NamedValueCollection& i_rViewFactoryArgs,
        const ::rtl::OUString& i_rViewName )
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

// --------------------------------------------------------------------------------------------------------------------
sal_Bool SAL_CALL SfxFrameLoader_Impl::load( const Sequence< PropertyValue >& rArgs,
                                             const Reference< XFrame >& _rTargetFrame )
    throw( RuntimeException )
{
    ENSURE_OR_THROW( _rTargetFrame.is(), "illegal NULL frame" );

    SolarMutexGuard aGuard;

    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mb93783) ::SfxFrameLoader::load" );

    ::comphelper::NamedValueCollection aDescriptor( rArgs );

    // ensure the descriptor contains a referrer
    if ( !aDescriptor.has( "Referer" ) )
        aDescriptor.put( "Referer", ::rtl::OUString() );

    // TODO: is this needed? finally, when loading is successfull, then there should be no need for this item,
    // as the document can always obtain its frame. In particular, there might be situations where this frame
    // is accessed, but already disposed: Imagine the user loading a document, opening a second view on it, and
    // then closing the first view/frame.
    aDescriptor.put( "Frame", _rTargetFrame );

    // did the caller already pass a model?
    Reference< XModel2 > xModel = aDescriptor.getOrDefault( "Model", Reference< XModel2 >() );
    const bool bExternalModel = xModel.is();

    // check for factory URLs to create a new doc, instead of loading one
    const ::rtl::OUString sURL = aDescriptor.getOrDefault( "URL", ::rtl::OUString() );
    const bool bIsFactoryURL = ( sURL.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "private:factory/" ) ) == 0 );
    bool bInitNewModel = bIsFactoryURL;
    if ( bIsFactoryURL && !bExternalModel )
    {
        const ::rtl::OUString sFactory = sURL.copy( sizeof( "private:factory/" ) -1 );
        // special handling for some weird factory URLs a la private:factory/swriter?slot=21053
        const USHORT nSlotParam = impl_findSlotParam( sFactory );
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
            const ::rtl::OUString sServiceName = SfxObjectShell::GetServiceNameFromFactory( sFactory );
            aDescriptor.put( "DocumentService", sServiceName );
        }
    }
    else
    {
        // compatibility
        aDescriptor.put( "FileName", aDescriptor.get( "URL" ) );
    }

    sal_Bool bLoadSuccess = sal_False;
    try
    {
        // extract view releant arguments from the loader args
        ::comphelper::NamedValueCollection aViewCreationArgs( impl_extractViewCreationArgs( aDescriptor ) );

        // no model passed from outside? => create one from scratch
        if ( !xModel.is() )
        {
            // beforehand, determine the filter to use, and update the descriptor with its information
            if ( !bInitNewModel )
            {
                impl_determineFilter( aDescriptor );
            }

            // create the new doc
            const ::rtl::OUString sServiceName = aDescriptor.getOrDefault( "DocumentService", ::rtl::OUString() );
            xModel.set( m_aContext.createComponent( sServiceName ), UNO_QUERY_THROW );

            // load resp. init it
            const Reference< XLoadable > xLoadable( xModel, UNO_QUERY_THROW );
            if ( bInitNewModel )
            {
                xLoadable->initNew();

                impl_removeLoaderArguments( aDescriptor );
                xModel->attachResource( ::rtl::OUString(), aDescriptor.getPropertyValues() );
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
                // TODO: not sure this is correct. The original, pre-refactoring code did it this way. However, I could
                // imagine scenarios where it is *not* correct to overrule the *existing* model args (XModel::getArgs)
                // with the ones passed to the loader here. For instance, what about the MacroExecutionMode? The document
                // might have a mode other than the one passed to the loader, and we always overwrite the former with
                // the latter.
        }

        // get the SfxObjectShell (still needed at the moment)
        const SfxObjectShellLock xDoc = impl_findObjectShell( xModel );
        ENSURE_OR_THROW( xDoc.Is(), "no SfxObjectShell for the given model" );

        // ensure the ID of the to-be-created view is in the descriptor, if possible
        const sal_Int16 nViewId = impl_determineEffectiveViewId_nothrow( *xDoc, aDescriptor );
        const sal_Int16 nViewNo = xDoc->GetFactory().GetViewNo_Impl( nViewId, 0 );
        const ::rtl::OUString sViewName( xDoc->GetFactory().GetViewFactory( nViewNo ).GetAPIViewName() );

        // if the document is created hidden, prevent it from being deleted until it is shown or disposed
        impl_lockHiddenDocument( *xDoc, aDescriptor );
            // TODO; if we wouldn't use a SfxObjectShellLock instance for xDoc, but a simple SfxObjectShellRef,
            // then this would not be necessary, /me thinks. That is, the *Lock classes inc/dec a "Lock" counter
            // (additional to the ref counter) in their ctor/dtor, and if the lock counter goes to 0, the
            // object is closed (DoClose). The impl_lockHiddenDocument is to prevent exactly that premature
            // closing. However, a *Ref object wouldn't close, anyway. And in case of unsuccessfull loading, the
            // code at the very end of this method cares for closing the XModel, which should also close the
            // ObjectShell.

        // plug the document into the frame
        impl_createDocumentView( xModel, _rTargetFrame, aViewCreationArgs, sViewName );
        bLoadSuccess = sal_True;
    }
    catch ( Exception& )
    {
        const Any aError( ::cppu::getCaughtException() );
        if ( !aDescriptor.getOrDefault( "Silent", sal_False ) )
            impl_handleCaughtError_nothrow( aError, aDescriptor );
    }

    // if loading was not successful, close the document
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

SFX_IMPL_SINGLEFACTORY( SfxFrameLoader_Impl )

/* XServiceInfo */
UNOOUSTRING SAL_CALL SfxFrameLoader_Impl::getImplementationName() throw( RuntimeException )
{
    return impl_getStaticImplementationName();
}
                                                                                                                                \
/* XServiceInfo */
sal_Bool SAL_CALL SfxFrameLoader_Impl::supportsService( const UNOOUSTRING& sServiceName ) throw( RuntimeException )
{
    UNOSEQUENCE< UNOOUSTRING >  seqServiceNames =   getSupportedServiceNames();
    const UNOOUSTRING*          pArray          =   seqServiceNames.getConstArray();
    for ( sal_Int32 nCounter=0; nCounter<seqServiceNames.getLength(); nCounter++ )
    {
        if ( pArray[nCounter] == sServiceName )
        {
            return sal_True ;
        }
    }
    return sal_False ;
}

/* XServiceInfo */
UNOSEQUENCE< UNOOUSTRING > SAL_CALL SfxFrameLoader_Impl::getSupportedServiceNames() throw( RuntimeException )
{
    return impl_getStaticSupportedServiceNames();
}

/* Helper for XServiceInfo */
UNOSEQUENCE< UNOOUSTRING > SfxFrameLoader_Impl::impl_getStaticSupportedServiceNames()
{
    UNOMUTEXGUARD aGuard( UNOMUTEX::getGlobalMutex() );
    UNOSEQUENCE< UNOOUSTRING > seqServiceNames( 1 );
    seqServiceNames.getArray() [0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.SynchronousFrameLoader" ));
    return seqServiceNames ;
}

/* Helper for XServiceInfo */
UNOOUSTRING SfxFrameLoader_Impl::impl_getStaticImplementationName()
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.office.FrameLoader" ));
}

/* Helper for registry */
UNOREFERENCE< UNOXINTERFACE > SAL_CALL SfxFrameLoader_Impl::impl_createInstance( const UNOREFERENCE< UNOXMULTISERVICEFACTORY >& xServiceManager ) throw( UNOEXCEPTION )
{
    return UNOREFERENCE< UNOXINTERFACE >( *new SfxFrameLoader_Impl( xServiceManager ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
