/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: frmload.cxx,v $
 * $Revision: 1.90 $
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

#include "frmload.hxx"

#include <framework/interaction.hxx>

#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <ucbhelper/simpleinteractionrequest.hxx>

#include <rtl/ustring.h>
#include <rtl/logfile.hxx>
#include <svtools/itemset.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <vos/mutex.hxx>
#include <svtools/eitem.hxx>
#include <svtools/stritem.hxx>
#include <tools/diagnose_ex.h>
#include <sot/storinfo.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <svtools/moduleoptions.hxx>

#ifndef css
namespace css = ::com::sun::star;
#endif

#include <sfx2/doctempl.hxx>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/dispatch.hxx>
#include <sfx2/sfxuno.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/topfrm.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/sfx.hrc>
#include "brokenpackageint.hxx"
#include "objshimp.hxx"

// do not remove the markers below
/** === begin UNO using === **/
using ::com::sun::star::uno::Reference;
using ::com::sun::star::task::XInteractionHandler;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::container::XContainerQuery;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::frame::XFrame;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::task::XInteractionRequest;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::task::XStatusIndicator;
using ::com::sun::star::document::XTypeDetection;
using ::com::sun::star::util::XCloseable;
using ::com::sun::star::frame::XLoadable;
/** === end UNO using === **/

SfxFrameLoader_Impl::SfxFrameLoader_Impl( const Reference< css::lang::XMultiServiceFactory >& _rxFactory )
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
    catch( const css::uno::Exception& )
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
const SfxFilter* SfxFrameLoader_Impl::impl_getFilterFromServiceName_nothrow( const ::rtl::OUString& _rServiceName ) const
{
    try
    {
        ::comphelper::NamedValueCollection aQuery;
        aQuery.put( "DocumentService", _rServiceName );

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
::rtl::OUString SfxFrameLoader_Impl::impl_askForFilter_nothrow( const Reference< XInteractionHandler >& _rxHandler,
                                                                 const ::rtl::OUString& _rDocumentURL ) const
{
    ENSURE_OR_THROW( _rxHandler.is(), "invalid interaction handler" );

    ::rtl::OUString sFilterName;
    try
    {
        ::framework::RequestFilterSelect* pRequest = new ::framework::RequestFilterSelect( _rDocumentURL );
        Reference< XInteractionRequest > xRequest ( pRequest );
        _rxHandler->handle( xRequest );
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
sal_Bool SfxFrameLoader_Impl::impl_createNewDocWithSlotParam( const USHORT _nSlotID, SfxTopFrame& i_rFrame )
{
    SfxApplication* pApp = SFX_APP();

    SfxRequest aRequest( _nSlotID, SFX_CALLMODE_SYNCHRON, pApp->GetPool() );
    aRequest.AppendItem( SfxFrameItem ( SID_DOCFRAME, &i_rFrame ) );

    return lcl_getDispatchResult( pApp->ExecuteSlot( aRequest ) );
}

// --------------------------------------------------------------------------------------------------------------------
sal_Bool SfxFrameLoader_Impl::impl_createNewDoc( const ::comphelper::NamedValueCollection& i_rDescriptor, SfxTopFrame& i_rFrame,
                                                 const ::rtl::OUString& _rFactoryName )
{
    OSL_PRECOND( i_rFrame.GetCurrentDocument() == NULL,
        "SfxFrameLoader_Impl::impl_createNewDoc: inserting into an already-occupied frame is not supported anymore!" );
    if ( i_rFrame.GetCurrentDocument() != NULL )
        return sal_False;

    SfxErrorContext aEc( ERRCTX_SFX_NEWDOCDIRECT );

    // create new document
    SfxObjectShellLock xDoc = SfxObjectShell::CreateObjectByFactoryName( _rFactoryName );
    if ( !xDoc.Is() || !xDoc->DoInitNew( NULL ) )
        return sal_False;

    Reference< XModel >  xModel( xDoc->GetModel(), UNO_QUERY );
    OSL_ENSURE( xModel.is(), "SfxFrameLoader_Impl::impl_createNewDoc: not sure this is really allowed ..." );
    if ( xModel.is() )
    {
        ::comphelper::NamedValueCollection aArgs( i_rDescriptor );
        aArgs.remove( "StatusIndicator" );  // TODO: why this?

        xModel->attachResource( ::rtl::OUString(), aArgs.getPropertyValues() );
    }

    const sal_Bool bHidden = i_rDescriptor.getOrDefault( "Hidden", sal_False );
    if ( bHidden )
    {
        xDoc->RestoreNoDelete();
        xDoc->OwnerLock( TRUE );
        xDoc->Get_Impl()->bHiddenLockedByAPI = TRUE;
    }

    if ( i_rFrame.InsertDocument_Impl( *xDoc ) )
        return sal_True;

    xDoc->DoClose();
    return sal_False;
}

// --------------------------------------------------------------------------------------------------------------------
const SfxFilter* SfxFrameLoader_Impl::impl_determineFilter( ::comphelper::NamedValueCollection& io_rDescriptor, const SfxFilterMatcher& rMatcher )
{
    const ::rtl::OUString     sURL         = io_rDescriptor.getOrDefault( "URL",                ::rtl::OUString() );
    const ::rtl::OUString     sTypeName    = io_rDescriptor.getOrDefault( "TypeName",           ::rtl::OUString() );
    const ::rtl::OUString     sFilterName  = io_rDescriptor.getOrDefault( "FilterName",         ::rtl::OUString() );
    const ::rtl::OUString     sServiceName = io_rDescriptor.getOrDefault( "DocumentService",    ::rtl::OUString() );
    const Reference< XInteractionHandler >
                              xInteraction = io_rDescriptor.getOrDefault( "InteractionHandler", Reference< XInteractionHandler >() );

    const SfxFilter* pFilter = NULL;

    // get filter by its name directly ...
    if ( sFilterName.getLength() )
        pFilter = rMatcher.GetFilter4FilterName( sFilterName );

    // or search the preferred filter for the detected type ...
    if ( !pFilter && sTypeName.getLength() )
        pFilter = rMatcher.GetFilter4EA( sTypeName );

    // or use given document service for detection too!
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
        io_rDescriptor.put( "FilterName", ::rtl::OUString( pFilter->GetFilterName() ) );

    return pFilter;
}

// --------------------------------------------------------------------------------------------------------------------
SfxAllItemSet SfxFrameLoader_Impl::impl_getInitialItemSet( const ::comphelper::NamedValueCollection& i_rDescriptor,
                                                           SfxTopFrame& i_rTargetFrame ) const
{
    SfxAllItemSet aSet( SFX_APP()->GetPool() );
    TransformParameters( SID_OPENDOC, i_rDescriptor.getPropertyValues(), aSet );

    aSet.Put( SfxFrameItem( SID_DOCFRAME, &i_rTargetFrame ) );

    return aSet;
}

// --------------------------------------------------------------------------------------------------------------------
sal_Bool SfxFrameLoader_Impl::impl_loadExistingDocument( const Reference< XModel >& i_rxDocument,
                                                         const Reference< XFrame >& i_rxTargetFrame,
                                                         const ::comphelper::NamedValueCollection& i_rDescriptor )
{
    ENSURE_OR_THROW( i_rxDocument.is() && i_rxTargetFrame.is(), "invallid model/frame" );

    SfxTopFrame* pTargetFrame = SfxTopFrame::Create( i_rxTargetFrame );
    ENSURE_OR_THROW( pTargetFrame, "could not create an SfxFrame" );

    for ( SfxObjectShell* pDoc = SfxObjectShell::GetFirst( NULL, FALSE ); pDoc; pDoc = SfxObjectShell::GetNext( *pDoc, NULL, FALSE ) )
    {
        if ( i_rxDocument == pDoc->GetModel() )
        {
            i_rxDocument->attachResource( i_rDescriptor.getOrDefault( "URL", ::rtl::OUString() ), i_rDescriptor.getPropertyValues() );

            SfxAllItemSet aSet( impl_getInitialItemSet( i_rDescriptor, *pTargetFrame ) );
            pTargetFrame->SetItemSet_Impl( &aSet );
            return pTargetFrame->InsertDocument_Impl( *pDoc );
        }
    }

    DBG_ERROR("Model is not based on SfxObjectShell - wrong frame loader use!");
    return sal_False;
}

// --------------------------------------------------------------------------------------------------------------------
sal_Bool SfxFrameLoader_Impl::impl_cleanUp( const sal_Bool i_bSuccess, const SfxFrameWeak& i_wFrame )
{
    if ( !i_bSuccess && i_wFrame && !i_wFrame->GetCurrentDocument() )
    {
        i_wFrame->SetFrameInterface_Impl( NULL );
        i_wFrame->DoClose();
    }

    return i_bSuccess;
}

// --------------------------------------------------------------------------------------------------------------------
const SfxFilter* SfxFrameLoader_Impl::impl_determineTemplateDocument( ::comphelper::NamedValueCollection& io_rDescriptor ) const
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
        }
        return pTemplateFilter;
    }

    return NULL;
}

// --------------------------------------------------------------------------------------------------------------------
USHORT SfxFrameLoader_Impl::impl_findSlotParam( ::rtl::OUString& io_rFactoryURL )
{
    ::rtl::OUString sSlotParam;
    const sal_Int32 nParamPos = io_rFactoryURL.indexOf( '?' );
    if ( nParamPos >= 0 )
    {
        // currently only the "slot" parameter is supported
        const sal_Int32 nSlotPos = io_rFactoryURL.indexOfAsciiL( RTL_CONSTASCII_STRINGPARAM( "slot=" ), nParamPos );
        if ( nSlotPos > 0 )
            sSlotParam = io_rFactoryURL.copy( nSlotPos + 5 );
        io_rFactoryURL = io_rFactoryURL.copy( 0, nParamPos );
    }

    if ( sSlotParam.getLength() )
        return USHORT( sSlotParam.toInt32() );

    return 0;
}

// --------------------------------------------------------------------------------------------------------------------
sal_Bool SAL_CALL SfxFrameLoader_Impl::load( const css::uno::Sequence< PropertyValue >& rArgs,
                                             const Reference< css::frame::XFrame >& _rTargetFrame )
    throw( css::uno::RuntimeException )
{
    ENSURE_OR_THROW( _rTargetFrame.is(), "illegal NULL frame" );

    // this methods assumes that the filter is detected before, usually by calling the detect() method below
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mb93783) ::SfxFrameLoader::load" );

    ::comphelper::NamedValueCollection aDescriptor( rArgs );
    if ( !aDescriptor.has( "Referer" ) )
        aDescriptor.put( "Referer", ::rtl::OUString() );

    // if a model is given, just load this into a newly created frame
    const Reference< XModel > xModel = aDescriptor.getOrDefault( "Model", Reference< XModel >() );
    if ( xModel.is() )
        return impl_loadExistingDocument( xModel, _rTargetFrame, aDescriptor );

    // determine the filter to use
    const SfxFilter* pDocumentFilter = NULL;
    const SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();

    pDocumentFilter = impl_determineFilter( aDescriptor, rMatcher );
    if ( !pDocumentFilter )
        return sal_False;

    // If detected filter indicates using of an own template format
    // add property "AsTemplate" to descriptor. But suppress this step
    // if such property already exists.
    if ( pDocumentFilter->IsOwnTemplateFormat() && !aDescriptor.has( "AsTemplate" ) )
        aDescriptor.put( "AsTemplate", sal_Bool( sal_True ) );

    // check for the URL pattern of our factory URLs
    SfxTopFrame* pTargetFrame = SfxTopFrame::Create( _rTargetFrame );
    ENSURE_OR_THROW( pTargetFrame, "could not create an SfxFrame" );
    SfxFrameWeak wFrame = pTargetFrame;

    const ::rtl::OUString sURL = aDescriptor.getOrDefault( "URL", ::rtl::OUString() );
    if ( sURL.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "private:factory/" ) ) == 0 )
    {
        ::rtl::OUString sFactory( sURL.copy( sizeof( "private:factory/" ) -1 ) );

        USHORT nSlotParam = impl_findSlotParam( sFactory );
        if ( nSlotParam != 0 )
        {
            sal_Bool bSuccess = impl_createNewDocWithSlotParam( nSlotParam, *pTargetFrame );
            return impl_cleanUp( bSuccess, wFrame );
        }

        const SfxFilter* pTemplateFilter = impl_determineTemplateDocument( aDescriptor );
        if ( !pTemplateFilter )
        {
            // no or no valid template found => just create a default doc of the desired type, without any template
            sal_Bool bSuccess = impl_createNewDoc( aDescriptor, *pTargetFrame, sFactory );
            return impl_cleanUp( bSuccess, wFrame );
        }

        // #i21583#
        // ignore/forget the filter which was detected for the corresponding "private:factory/xxx" URL.
        pDocumentFilter = pTemplateFilter;
    }
    else
    {
        aDescriptor.put( "FileName", sURL );
    }

    const ::rtl::OUString sFilterName  = aDescriptor.getOrDefault( "FilterName", ::rtl::OUString() );

    // at this point, we need a filter
    DBG_ASSERT( pDocumentFilter, "SfxFrameLoader_Impl::load: could not determine a filter!" );
    if ( !pDocumentFilter )
        return sal_False;

    SfxObjectShell* pDoc = SfxObjectShell::CreateObject( pDocumentFilter->GetServiceName() );
    if ( !pDoc )
        return sal_False;

    aDescriptor.put( "Frame", _rTargetFrame );

    SfxAllItemSet aSet( impl_getInitialItemSet( aDescriptor, *pTargetFrame ) );
    aSet.Put( SfxStringItem( SID_FILTER_NAME, sFilterName ) );

    sal_Bool bLoadSuccess = sal_False;
    sal_Bool bDisaster = sal_False;
    try
    {
        Reference< XLoadable > xLoadable( pDoc->GetModel(), UNO_QUERY );
        Sequence< PropertyValue > aLoadArgs;
        TransformItems( SID_OPENDOC, aSet, aLoadArgs );

        xLoadable->load( aLoadArgs );

        SfxMedium* pDocMedium = pDoc->GetMedium();
        BOOL bHidden = FALSE;
        SFX_ITEMSET_ARG( pDocMedium->GetItemSet(), pHidItem, SfxBoolItem, SID_HIDDEN, sal_False);
        if ( pHidItem )
            bHidden = pHidItem->GetValue();

        // !TODO: will be done by Framework!
        pDocMedium->SetUpdatePickList( !bHidden );

        /*
            #121119#
            We dont know why pTargetFrame can be corrupt here.
            But if it was deleted it shouldnt exists inside our global list.
            May be we can use the damaged pointer to detect if it was removed from
            this global list.
        */
        impl_ensureValidFrame_throw( pTargetFrame );

        aSet.Put( SfxFrameItem( SID_DOCFRAME, pTargetFrame ) );

        pTargetFrame->SetItemSet_Impl( &aSet );
        if ( pTargetFrame->InsertDocument_Impl( *pDoc ) )
        {
            pTargetFrame->GetCurrentViewFrame()->UpdateDocument_Impl();
            String aURL = pDoc->GetMedium()->GetName();
            SFX_APP()->Broadcast( SfxStringHint( SID_OPENURL, aURL ) );
            bLoadSuccess = sal_True;
        }
        else
            bDisaster = sal_True;
    }
    catch ( Exception& )
    {
        bDisaster = sal_True;
    }

    if ( bDisaster )
    {
        // document loading was not successful; close SfxFrame (but not XFrame!) and document
        impl_cleanUp( false, wFrame );

        Reference< XCloseable > xCloseable( pDoc->GetModel(), UNO_QUERY );
        if ( xCloseable.is() )
        {
            try
            {
                xCloseable->close( sal_True );
            }
            catch ( Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }

    return bLoadSuccess;
}

void SfxFrameLoader_Impl::impl_ensureValidFrame_throw( const SfxTopFrame* _pFrame )
{
    SfxFrame* pTmp = NULL;
    for ( pTmp = SfxFrame::GetFirst(); pTmp; pTmp = SfxFrame::GetNext( *pTmp ) )
    {
        if ( _pFrame == pTmp )
            // all fine, frame still alive
            return;
    }

    DBG_ERROR( "#121119# You found the reason for a stacktrace! Frame destroyed while loading document." );
    throw RuntimeException( ::rtl::OUString(), *this );
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
    seqServiceNames.getArray() [0] = UNOOUSTRING::createFromAscii( "com.sun.star.frame.SynchronousFrameLoader" );
    return seqServiceNames ;
}

/* Helper for XServiceInfo */
UNOOUSTRING SfxFrameLoader_Impl::impl_getStaticImplementationName()
{
    return UNOOUSTRING::createFromAscii( "com.sun.star.comp.office.FrameLoader" );
}

/* Helper for registry */
UNOREFERENCE< UNOXINTERFACE > SAL_CALL SfxFrameLoader_Impl::impl_createInstance( const UNOREFERENCE< UNOXMULTISERVICEFACTORY >& xServiceManager ) throw( UNOEXCEPTION )
{
    return UNOREFERENCE< UNOXINTERFACE >( *new SfxFrameLoader_Impl( xServiceManager ) );
}

