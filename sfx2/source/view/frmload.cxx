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
sal_Bool SfxFrameLoader_Impl::impl_createNewDocWithSlotParam( const USHORT _nSlotID, const Reference< XFrame >& i_rxFrame )
{
    SfxTopFrame* pTargetFrame = SfxTopFrame::Create( i_rxFrame );
    ENSURE_OR_THROW( pTargetFrame, "could not create an SfxFrame" );
    SfxFrameWeak wFrame = pTargetFrame;

    SfxRequest aRequest( _nSlotID, SFX_CALLMODE_SYNCHRON, SFX_APP()->GetPool() );
    aRequest.AppendItem( SfxFrameItem ( SID_DOCFRAME, pTargetFrame ) );

    sal_Bool bSuccess = lcl_getDispatchResult( SFX_APP()->ExecuteSlot( aRequest ) );
    return impl_cleanUp( bSuccess, wFrame );
}

// --------------------------------------------------------------------------------------------------------------------
void SfxFrameLoader_Impl::impl_lockHiddenDocument( SfxObjectShell& i_rDocument, const ::comphelper::NamedValueCollection& i_rDescriptor )
{
    const sal_Bool bHidden = i_rDescriptor.getOrDefault( "Hidden", sal_False );
    if ( bHidden )
    {
        i_rDocument.RestoreNoDelete();
        i_rDocument.OwnerLock( TRUE );
        i_rDocument.Get_Impl()->bHiddenLockedByAPI = TRUE;
    }
}

// --------------------------------------------------------------------------------------------------------------------
sal_Bool SfxFrameLoader_Impl::impl_createNewDoc( const ::comphelper::NamedValueCollection& i_rDescriptor, const Reference< XFrame >& i_rxFrame,
                                                 const ::rtl::OUString& i_rFactoryName )
{
    SfxTopFrame* pTargetFrame = SfxTopFrame::Create( i_rxFrame );
    ENSURE_OR_THROW( pTargetFrame, "could not create an SfxFrame" );
    SfxFrameWeak wFrame = pTargetFrame;

    // create new document
    SfxObjectShellLock xDoc;
    try
    {
        const ::rtl::OUString sServiceName = SfxObjectShell::GetServiceNameFromFactory( i_rFactoryName );
        Reference< XModel > xModel( m_aContext.createComponent( sServiceName ), UNO_QUERY_THROW );

        Reference< XLoadable > xLoadable( xModel, UNO_QUERY_THROW );
        xLoadable->initNew();

        xDoc = impl_findObjectShell( xModel );
        ENSURE_OR_THROW( xDoc.Is(), "no SfxObjectShell for the newly created model" );

        ::comphelper::NamedValueCollection aArgs( i_rDescriptor );
        aArgs.remove( "StatusIndicator" );  // TODO: why this?

        xModel->attachResource( ::rtl::OUString(), aArgs.getPropertyValues() );
    }
    catch( const Exception& )
    {
        // TODO: catch (and handle?) ErrorCodeIOException, and perhaps others ...
        DBG_UNHANDLED_EXCEPTION();
        return sal_False;
    }

    // if the document is created hidden, prevent it being deleted until it is shown or disposed
    impl_lockHiddenDocument( *xDoc, i_rDescriptor );

    const sal_Bool bSuccess = pTargetFrame->InsertDocument_Impl( *xDoc );
    return impl_cleanUp( bSuccess, wFrame );
}

// --------------------------------------------------------------------------------------------------------------------
void SfxFrameLoader_Impl::impl_determineFilter( ::comphelper::NamedValueCollection& io_rDescriptor )
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
sal_Bool SfxFrameLoader_Impl::impl_plugDocIntoFrame( const ::comphelper::NamedValueCollection& i_rDescriptor,
                                                     SfxTopFrame& i_rTargetFrame, SfxObjectShell& i_rDocument )
{
    SfxAllItemSet aSet( SFX_APP()->GetPool() );
    TransformParameters( SID_OPENDOC, i_rDescriptor.getPropertyValues(), aSet );
    i_rTargetFrame.SetItemSet_Impl( &aSet );
    return i_rTargetFrame.InsertDocument_Impl( i_rDocument );
}

// --------------------------------------------------------------------------------------------------------------------
SfxObjectShellLock SfxFrameLoader_Impl::impl_findObjectShell( const Reference< XModel >& i_rxDocument )
{
    for ( SfxObjectShell* pDoc = SfxObjectShell::GetFirst( NULL, FALSE ); pDoc; pDoc = SfxObjectShell::GetNext( *pDoc, NULL, FALSE ) )
    {
        if ( i_rxDocument == pDoc->GetModel() )
        {
            return pDoc;
        }
    }

    DBG_ERROR( "SfxFrameLoader_Impl::impl_findObjectShell: model is not based on SfxObjectShell - wrong frame loader usage!" );
    return NULL;
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
USHORT SfxFrameLoader_Impl::impl_findSlotParam( const ::rtl::OUString& i_rFactoryURL )
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
sal_Bool SAL_CALL SfxFrameLoader_Impl::load( const css::uno::Sequence< PropertyValue >& rArgs,
                                             const Reference< css::frame::XFrame >& _rTargetFrame )
    throw( css::uno::RuntimeException )
{
    ENSURE_OR_THROW( _rTargetFrame.is(), "illegal NULL frame" );

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mb93783) ::SfxFrameLoader::load" );

    ::comphelper::NamedValueCollection aDescriptor( rArgs );

    // ensure the descriptor contains a referrer
    if ( !aDescriptor.has( "Referer" ) )
        aDescriptor.put( "Referer", ::rtl::OUString() );

    // TODO: is this needed? finally, when loading is successfull, then ther should be no need for this item,
    // as the document can always obtain its frame. In particular, there might be situations where this frame
    // is access, but already disposed: Imagine the user loading a document, opening a second view on it, and
    // then closing the first view/frame.
    aDescriptor.put( "Frame", _rTargetFrame );

    // check for factory URLs to create a new doc, instead of loading one
    const ::rtl::OUString sURL = aDescriptor.getOrDefault( "URL", ::rtl::OUString() );
    const bool bIsFactoryURL = ( sURL.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "private:factory/" ) ) == 0 );
    if ( bIsFactoryURL )
    {
        OSL_ENSURE( !aDescriptor.has( "Model" ), "SfxFrameLoader_Impl::load: sure you know what you're doing?" );
        // Before the loader refactoring, the model would have won over the URL, that is, the model would have been
        // loaded into a newly created frame. /me thinks this doesn't make sense at all, also, it made the code
        // in this method more complex. So, the order was changed, now the factory URL wins over the Model.
        // If somebody *rightfully* passes both of them, we might need to re-consider the behavior.

        const ::rtl::OUString sFactory = sURL.copy( sizeof( "private:factory/" ) -1 );
        // special handling for some weird factory URLs a la private:factory/swriter?slot=21053
        USHORT nSlotParam = impl_findSlotParam( sFactory );
        if ( nSlotParam != 0 )
        {
            return impl_createNewDocWithSlotParam( nSlotParam, _rTargetFrame );
        }

        bool bDescribesValidTemplate = impl_determineTemplateDocument( aDescriptor );
        if ( !bDescribesValidTemplate )
        {
            // no or no valid template found => just create a default doc of the desired type, without any template
            return impl_createNewDoc( aDescriptor, _rTargetFrame, sFactory );
        }
    }
    else
    {
        // compatibility
        aDescriptor.put( "FileName", aDescriptor.get( "URL" ) );
    }

    // did the caller already pass a model?
    Reference< XModel > xModel = aDescriptor.getOrDefault( "Model", Reference< XModel >() );
    const bool bExternalModel = xModel.is();

    sal_Bool bLoadSuccess = sal_False;
    SfxFrameWeak wFrame;
    try
    {
        // no model passed from outside? => create one from scratch
        if ( !xModel.is() )
        {
            // beforehand, determine the filter to use, and update the descriptor with its information
            impl_determineFilter( aDescriptor );

            // create the new doc
            ::rtl::OUString sServiceName = aDescriptor.getOrDefault( "DocumentService", ::rtl::OUString() );
            xModel.set( m_aContext.createComponent( sServiceName ), UNO_QUERY_THROW );

            // load it
            Reference< XLoadable > xLoadable( xModel, UNO_QUERY_THROW );
            xLoadable->load( aDescriptor.getPropertyValues() );
        }
        else
        {
            // tell the doc its load args.
            xModel->attachResource( aDescriptor.getOrDefault( "URL", ::rtl::OUString() ), aDescriptor.getPropertyValues() );

            // TODO: not sure this is correct. The original, pre-refactoring code did it this way. However, I could
            // imagine scenarios where it is *not* correct to overrule the *existing* model args (XModel::getArgs)
            // with the ones passed to the loader here. For instance, what about the MacroExecutionMode? The document
            // might have a mode other than the one passed to the loader, and we always overwrite the former with
            // the latter.
        }

        // create a frame
        SfxTopFrame* pTargetFrame = SfxTopFrame::Create( _rTargetFrame );
        ENSURE_OR_THROW( pTargetFrame, "could not create an SfxFrame" );
        wFrame = pTargetFrame;

        // get the SfxObjectShell (still needed at the moment)
        SfxObjectShellLock xDoc = impl_findObjectShell( xModel );
        ENSURE_OR_THROW( xDoc.Is(), "no SfxObjectShell for the given model" );

        // if the document is created hidden, prevent it being deleted until it is shown or disposed
        impl_lockHiddenDocument( *xDoc, aDescriptor );

        // insert the document into the frame
        if ( !impl_plugDocIntoFrame( aDescriptor, *pTargetFrame, *xDoc ) )
            throw RuntimeException();

        if ( !bExternalModel )
        {
            // TODO: which of those statements are allowed in the ExternalModel-case, too?
            // I think the broadcast isn't, but the UpdateDocument_Impl might.
            pTargetFrame->GetCurrentViewFrame()->UpdateDocument_Impl();
            String aURL = xDoc->GetMedium()->GetName();
            SFX_APP()->Broadcast( SfxStringHint( SID_OPENURL, aURL ) );
                // TODO: grokking suggests nobody might be interested in this SID_OPENURL broadcast, anyway?
        }

        bLoadSuccess = sal_True;
    }
    catch ( Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    if ( !bLoadSuccess )
    {
        // document loading was not successful; close SfxFrame (but not XFrame!) and document
        impl_cleanUp( false, wFrame );

        if ( !bExternalModel )
        {
            try
            {
                Reference< XCloseable > xCloseable( xModel, UNO_QUERY_THROW );
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

