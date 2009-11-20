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
        const Sequence< PropertyValue >& rArgs, const SfxFilterMatcher& rMatcher ) const
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

        ::comphelper::NamedValueCollection aOrgArgs( rArgs );
        if ( aOrgArgs.has( "InteractionHandler" ) )
            aNewArgs.put( "InteractionHandler", aOrgArgs.get( "InteractionHandler" ) );
        if ( aOrgArgs.has( "StatusIndicator" ) )
            aNewArgs.put( "StatusIndicator", aOrgArgs.get( "StatusIndicator" ) );

        ::rtl::OUString sType = xDetect->queryTypeByDescriptor( aNewArgs.getPropertyValues(), sal_True );
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
    const SfxFilter* pFilter = NULL;
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
sal_Bool SfxFrameLoader_Impl::impl_createNewDocWithSlotParam( const sal_uInt16 _nSlotID, SfxFrame* _pFrame )
{
    SfxApplication* pApp = SFX_APP();

    SfxRequest aRequest( _nSlotID, SFX_CALLMODE_SYNCHRON, pApp->GetPool() );
    aRequest.AppendItem( SfxFrameItem ( SID_DOCFRAME, _pFrame ) );

    return lcl_getDispatchResult( pApp->ExecuteSlot( aRequest ) );
}

// --------------------------------------------------------------------------------------------------------------------
sal_Bool SfxFrameLoader_Impl::impl_createNewDoc( const SfxItemSet& _rSet, SfxFrame* _pFrame, const ::rtl::OUString& _rFactoryURL )
{
    SfxApplication* pApp = SFX_APP();

    SfxRequest aRequest( SID_NEWDOCDIRECT, SFX_CALLMODE_SYNCHRON, _rSet );
    aRequest.AppendItem( SfxFrameItem( SID_DOCFRAME, _pFrame ) );
    aRequest.AppendItem( SfxStringItem( SID_NEWDOCDIRECT, _rFactoryURL ) );

    SFX_ITEMSET_ARG( &_rSet, pDocumentTitleItem, SfxStringItem, SID_DOCINFO_TITLE, FALSE );
    if ( pDocumentTitleItem )
        aRequest.AppendItem( *pDocumentTitleItem );

    return lcl_getDispatchResult( pApp->NewDocDirectExec_ImplOld( aRequest ) );
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
    const ::rtl::OUString     sURL         = aDescriptor.getOrDefault( "URL",                ::rtl::OUString() );
    const ::rtl::OUString     sTypeName    = aDescriptor.getOrDefault( "TypeName",           ::rtl::OUString() );
          ::rtl::OUString     sFilterName  = aDescriptor.getOrDefault( "FilterName",         ::rtl::OUString() );
    const ::rtl::OUString     sServiceName = aDescriptor.getOrDefault( "DocumentService",    ::rtl::OUString() );
    const Reference< XModel > xModel       = aDescriptor.getOrDefault( "Model",              Reference< XModel >() );
    const Reference< XInteractionHandler >
                              xInteraction = aDescriptor.getOrDefault( "InteractionHandler", Reference< XInteractionHandler >() );

    const SfxFilter* pFilter = NULL;
    const SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();

    /* special mode: use already loaded model ...
        In such case no filter name will be selected and no URL will be given!
        Such informations are not neccessary. We have to create a new view only
        and call setComponent() at the corresponding frame. */
    if( !xModel.is() )
    {

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

        if( !pFilter )
            return sal_False;

        // use filter names without prefix
        sFilterName = pFilter->GetFilterName();

        // If detected filter indicates using of an own template format
        // add property "AsTemplate" to descriptor. But suppress this step
        // if such property already exists.
        if ( pFilter->IsOwnTemplateFormat() && !aDescriptor.has( "AsTemplate" ) )
        {
            aDescriptor.put( "AsTemplate", sal_Bool( sal_True ) );
        }
    }

    // Achtung: beim Abraeumen der Objekte kann die SfxApp destruiert werden, vorher noch Deinitialize_Impl rufen
    SfxApplication* pApp = SFX_APP();

    // attention: Don't use rArgs, but instead aDescriptor, which is rArgs plus some
    // changes
    SfxAllItemSet aSet( pApp->GetPool() );
    TransformParameters( SID_OPENDOC, aDescriptor.getPropertyValues(), aSet );

    SFX_ITEMSET_ARG( &aSet, pRefererItem, SfxStringItem, SID_REFERER, FALSE );
    if ( !pRefererItem )
        aSet.Put( SfxStringItem( SID_REFERER, String() ) );

    SfxFrame* pTargetFrame = NULL;
    for ( pTargetFrame = SfxFrame::GetFirst(); pTargetFrame; pTargetFrame = SfxFrame::GetNext( *pTargetFrame ) )
    {
        if ( pTargetFrame->GetFrameInterface() == _rTargetFrame )
            break;
    }

    BOOL bFrameCreated = FALSE;
    if ( !pTargetFrame )
    {
        pTargetFrame = SfxTopFrame::Create( _rTargetFrame );
        bFrameCreated = TRUE;
    }

    if ( xModel.is() )
    {
        // !TODO: replace by ViewFactory
        if ( _rTargetFrame->getController().is() )
        {
            // remove old component
            // if a frame was created already, it can't be an SfxComponent!
            _rTargetFrame->setComponent( NULL, NULL );
            if ( !bFrameCreated )
                pTargetFrame = SfxTopFrame::Create( _rTargetFrame );
        }

        aSet.Put( SfxFrameItem( SID_DOCFRAME, pTargetFrame ) );

        for ( SfxObjectShell* pDoc = SfxObjectShell::GetFirst( NULL, FALSE ); pDoc; pDoc = SfxObjectShell::GetNext( *pDoc, NULL, FALSE ) )
        {
            if ( xModel == pDoc->GetModel() )
            {
                SfxTopFrame* pTopFrame = dynamic_cast< SfxTopFrame* >( pTargetFrame );
                OSL_ENSURE( pTopFrame, "SfxFrameLoader_Impl::load: An SfxFrame which is no SfxTopFrame?!" );
                if ( !pTopFrame )
                    return sal_False;

                pTargetFrame->SetItemSet_Impl( &aSet );

                aDescriptor.remove( "Model" );
                aDescriptor.remove( "URL" );

                xModel->attachResource( sURL, aDescriptor.getPropertyValues() );

                return pTopFrame->InsertDocument_Impl( *pDoc );
            }
        }

        DBG_ERROR("Model is not based on SfxObjectShell - wrong frame loader use!");
        return sal_False;
    }

    // check for the URL pattern of our factory URLs
    SfxFrameWeak wFrame = pTargetFrame;
    String aPrefix = String::CreateFromAscii( "private:factory/" );
    String aFact( sURL );
    if ( aPrefix.Len() == aFact.Match( aPrefix ) )
    {
        // it's a factory URL
        aFact.Erase( 0, aPrefix.Len() );

        // look for parameters
        String aParam;
        USHORT nPos = aFact.Search( '?' );
        if ( nPos != STRING_NOTFOUND )
        {
            // currently only the "slot" parameter is supported
            USHORT nParamPos = aFact.Search( String::CreateFromAscii("slot="), nPos );
            if ( nParamPos != STRING_NOTFOUND )
                aParam = aFact.Copy( nParamPos+5, aFact.Len() );
            aFact.Erase( nPos, aFact.Len() );
        }

        if ( aParam.Len() )
        {
            sal_Bool bSuccess = impl_createNewDocWithSlotParam( (sal_uInt16)aParam.ToInt32(), pTargetFrame );
            if ( !bSuccess && bFrameCreated && wFrame && !wFrame->GetCurrentDocument() )
            {
                wFrame->SetFrameInterface_Impl( NULL );
                wFrame->DoClose();
            }

            return bSuccess;
        }

        String sTemplateURL;
        SFX_ITEMSET_ARG( &aSet, pTemplateRegionItem, SfxStringItem, SID_TEMPLATE_REGIONNAME, FALSE );
        SFX_ITEMSET_ARG( &aSet, pTemplateNameItem, SfxStringItem, SID_TEMPLATE_NAME, FALSE );
        if ( pTemplateRegionItem && pTemplateNameItem )
        {
            SfxDocumentTemplates aTmpFac;
            aTmpFac.GetFull( pTemplateRegionItem->GetValue(), pTemplateNameItem->GetValue(), sTemplateURL );
        }
        else
        {
            if ( sServiceName.getLength() )
                sTemplateURL = SfxObjectFactory::GetStandardTemplate( sServiceName );
            else
                sTemplateURL = SfxObjectFactory::GetStandardTemplate( SfxObjectShell::GetServiceNameFromFactory( sURL ) );
        }

        BOOL bUseTemplate = (sTemplateURL.Len()>0);
        if( bUseTemplate )
        {
            // #i21583#
            // Forget the filter, which was detected for the corresponding "private:factory/xxx" URL.
            // We must use the right filter, matching to this document ... not to the private URL!
            const SfxFilter* pTemplateFilter = impl_detectFilterForURL(sTemplateURL, rArgs, rMatcher);
            if (pTemplateFilter)
            {
                pFilter     = pTemplateFilter;
                sFilterName = pTemplateFilter->GetName();
                // standard template set -> load it "AsTemplate"
                aSet.Put( SfxStringItem ( SID_FILE_NAME, sTemplateURL ) );
                aSet.Put( SfxBoolItem( SID_TEMPLATE, sal_True ) );
            }

            // #119268#
            // something is wrong with the set default template (e.g. unknown format, missing file etcpp)
            // The we have to jump into the following special code, where "private:factory/ URL's are handled.
            // We cant "load" such private/factory URL's!
            else
                bUseTemplate = FALSE;
        }

        if ( !bUseTemplate )
        {
            // execute "NewDocument" request
            sal_Bool bSuccess = impl_createNewDoc( aSet, pTargetFrame, aFact );;
            if ( !bSuccess && bFrameCreated && wFrame && !wFrame->GetCurrentDocument() )
            {
                wFrame->SetFrameInterface_Impl( NULL );
                wFrame->DoClose();
            }
            return bSuccess;
        }
    }
    else
    {
        // load the desired file
        aSet.Put( SfxStringItem ( SID_FILE_NAME, sURL ) );
    }

    DBG_ASSERT( pFilter, "SfxFrameLoader_Impl::load: could not determine a filter!" );
    if ( !pFilter )
        return sal_False;

    // !TODO: replace by direct construction of model (needs view factory)
    sal_Bool bDisaster = sal_False;
    SfxObjectShell* pDoc = SfxObjectShell::CreateObject( pFilter->GetServiceName() );
    if ( !pDoc )
        return sal_False;

    aSet.Put( SfxFrameItem( SID_DOCFRAME, pTargetFrame ) );
    aSet.Put( SfxUnoAnyItem( SID_FILLFRAME, makeAny( _rTargetFrame ) ) );
    aSet.Put( SfxStringItem( SID_FILTER_NAME, sFilterName ) );

    sal_Bool bLoadSuccess = sal_False;
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

        impl_ensureValidFrame_throw( pTargetFrame );

        // !TODO: replace by ViewFactory
        if ( pTargetFrame->GetFrameInterface()->getController().is() )
        {
            // remove old component
            // if a frame was created already, it can't be an SfxComponent!
            //pTargetFrame->GetFrameInterface()->setComponent( 0, 0 );
            if ( !bFrameCreated )
                pTargetFrame = SfxTopFrame::Create( _rTargetFrame );
        }

        wFrame = pTargetFrame;
        aSet.Put( SfxFrameItem( SID_DOCFRAME, pTargetFrame ) );
        pTargetFrame->SetItemSet_Impl( &aSet );

        SfxTopFrame* pTopFrame = dynamic_cast< SfxTopFrame* >( pTargetFrame );
        OSL_ENSURE( pTopFrame, "SfxFrameLoader_Impl::load: An SfxFrame which is no SfxTopFrame?!" );
        if ( pTopFrame && pTopFrame->InsertDocument_Impl( *pDoc ) )
        {
            pTargetFrame->GetCurrentViewFrame()->UpdateDocument_Impl();
            String aURL = pDoc->GetMedium()->GetName();
            SFX_APP()->Broadcast( SfxStringHint( SID_OPENURL, aURL ) );
            bLoadSuccess = sal_True;
        }
        else
            bDisaster = sal_True;
    }
    catch ( css::uno::Exception& )
    {
        bDisaster = sal_True;
    }

    if ( bDisaster )
    {
        if ( wFrame && !wFrame->GetCurrentDocument() )
        {
            // document loading was not successful; close SfxFrame (but not XFrame!)
            wFrame->SetFrameInterface_Impl( NULL );
            wFrame->DoClose();
        }

        Reference< XCloseable > xCloseable( pDoc->GetModel(), UNO_QUERY );
        if ( xCloseable.is() )
        {
            try
            {
                xCloseable->close( sal_True );
            }
            catch ( css::uno::Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }

    return bLoadSuccess;
}

void SfxFrameLoader_Impl::impl_ensureValidFrame_throw( const SfxFrame* _pFrame )
{
    /*
        #121119#
        We dont know why pTargetFrame can be corrupt here.
        But if it was deleted it shouldnt exists inside our global list.
        May be we can use the damaged pointer to detect if it was removed from
        this global list.
    */
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

