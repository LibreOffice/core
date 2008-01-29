/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: frmload.cxx,v $
 *
 *  $Revision: 1.89 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:29:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include "frmload.hxx"

#include <framework/interaction.hxx>

#ifndef _COM_SUN_STAR_FRAME_XLOADABLE_HPP_
#include <com/sun/star/frame/XLoadable.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERQUERY_HPP_
#include <com/sun/star/container/XContainerQuery.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_WRAPPEDTARGETRUNTIMEEXCEPTION_HPP_
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEAPPEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveAppException.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XTYPEDETECTION_HPP_
#include <com/sun/star/document/XTypeDetection.hpp>
#endif

#ifndef __FRAMEWORK_DISPATCH_INTERACTION_HXX_
#include <framework/interaction.hxx>
#endif

#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/helper/vclunohelper.hxx>
#endif

#ifndef _UCBHELPER_SIMPLEINTERACTIONREQUEST_HXX
#include <ucbhelper/simpleinteractionrequest.hxx>
#endif

#include <rtl/ustring.h>
#include <rtl/logfile.hxx>
#include <svtools/itemset.hxx>
#include <vcl/window.hxx>
#include <svtools/eitem.hxx>
#include <svtools/stritem.hxx>
#include <tools/urlobj.hxx>
#include <vos/mutex.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <sot/storinfo.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <svtools/moduleoptions.hxx>

#ifndef css
namespace css = ::com::sun::star;
#endif

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

SfxFrameLoader_Impl::SfxFrameLoader_Impl( const css::uno::Reference< css::lang::XMultiServiceFactory >& /*xFactory*/ )
{
}

SfxFrameLoader_Impl::~SfxFrameLoader_Impl()
{
}

const SfxFilter* impl_detectFilterForURL(const ::rtl::OUString&                                 sURL    ,
                                         const css::uno::Sequence< css::beans::PropertyValue >& rArgs   ,
                                         const SfxFilterMatcher&                                rMatcher)
{
    static ::rtl::OUString SERVICENAME_TYPEDETECTION = ::rtl::OUString::createFromAscii("com.sun.star.document.TypeDetection");

    ::rtl::OUString sFilter;
    try
    {
        if (!sURL.getLength())
            return 0;

        css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();
        if (!xSMGR.is())
            return 0;

        css::uno::Reference< css::document::XTypeDetection > xDetect(
            xSMGR->createInstance(SERVICENAME_TYPEDETECTION),
            css::uno::UNO_QUERY_THROW);

        ::comphelper::MediaDescriptor lOrgArgs(rArgs);
        css::uno::Reference< css::task::XInteractionHandler > xInteraction = lOrgArgs.getUnpackedValueOrDefault(
            ::comphelper::MediaDescriptor::PROP_INTERACTIONHANDLER(),
            css::uno::Reference< css::task::XInteractionHandler >());
        css::uno::Reference< css::task::XStatusIndicator > xProgress = lOrgArgs.getUnpackedValueOrDefault(
            ::comphelper::MediaDescriptor::PROP_STATUSINDICATOR(),
            css::uno::Reference< css::task::XStatusIndicator >());

        ::comphelper::SequenceAsHashMap lNewArgs;
        lNewArgs[::comphelper::MediaDescriptor::PROP_URL()] <<= sURL;
        if (xInteraction.is())
            lNewArgs[::comphelper::MediaDescriptor::PROP_INTERACTIONHANDLER()] <<= xInteraction;
        if (xProgress.is())
            lNewArgs[::comphelper::MediaDescriptor::PROP_STATUSINDICATOR()] <<= xProgress;

        css::uno::Sequence< css::beans::PropertyValue > lDescr = lNewArgs.getAsConstPropertyValueList();
        ::rtl::OUString sType = xDetect->queryTypeByDescriptor(lDescr, sal_True);
        if (sType.getLength())
        {
            const SfxFilter* pFilter = rMatcher.GetFilter4EA(sType);
            if (pFilter)
                sFilter = pFilter->GetName();
        }
    }
    catch(const css::uno::RuntimeException& exRun)
        { throw exRun; }
    catch(const css::uno::Exception&)
        { sFilter = ::rtl::OUString(); }

    const SfxFilter* pFilter = 0;
    if (sFilter.getLength())
        pFilter = rMatcher.GetFilter4FilterName(sFilter);
    return pFilter;
}

sal_Bool SAL_CALL SfxFrameLoader_Impl::load( const css::uno::Sequence< css::beans::PropertyValue >& rArgs  ,
                                             const css::uno::Reference< css::frame::XFrame >&       rFrame )
    throw( css::uno::RuntimeException )
{
    static ::rtl::OUString PROP_URL                = ::rtl::OUString::createFromAscii("URL"               );
    static ::rtl::OUString PROP_FILENAME           = ::rtl::OUString::createFromAscii("FileName"          );
    static ::rtl::OUString PROP_TYPENAME           = ::rtl::OUString::createFromAscii("TypeName"          );
    static ::rtl::OUString PROP_FILTERNAME         = ::rtl::OUString::createFromAscii("FilterName"        );
    static ::rtl::OUString PROP_MODEL              = ::rtl::OUString::createFromAscii("Model"             );
    static ::rtl::OUString PROP_DOCUMENTSERVICE    = ::rtl::OUString::createFromAscii("DocumentService"   );
    static ::rtl::OUString PROP_READONLY           = ::rtl::OUString::createFromAscii("ReadOnly"          );
    static ::rtl::OUString PROP_ASTEMPLATE         = ::rtl::OUString::createFromAscii("AsTemplate"        );
    static ::rtl::OUString PROP_INTERACTIONHANDLER = ::rtl::OUString::createFromAscii("InteractionHandler");

    sal_Bool bLoadState = sal_False;

    // this methods assumes that the filter is detected before, usually by calling the detect() method below
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mb93783) ::SfxFrameLoader::load" );

    if ( !rFrame.is() )
        return sal_False;

    ::comphelper::SequenceAsHashMap lDescriptor(rArgs);
    String                                                rURL          = lDescriptor.getUnpackedValueOrDefault(PROP_URL               , ::rtl::OUString()                                      );
    String                                                aTypeName     = lDescriptor.getUnpackedValueOrDefault(PROP_TYPENAME          , ::rtl::OUString()                                      );
                                                          aFilterName   = lDescriptor.getUnpackedValueOrDefault(PROP_FILTERNAME        , ::rtl::OUString()                                      );
    String                                                aServiceName  = lDescriptor.getUnpackedValueOrDefault(PROP_DOCUMENTSERVICE   , ::rtl::OUString()                                      );
    css::uno::Reference< css::task::XInteractionHandler > xInteraction  = lDescriptor.getUnpackedValueOrDefault(PROP_INTERACTIONHANDLER, css::uno::Reference< css::task::XInteractionHandler >());
    css::uno::Reference< css::frame::XModel >             xModel        = lDescriptor.getUnpackedValueOrDefault(PROP_MODEL             , css::uno::Reference< css::frame::XModel >()            );

    const SfxFilter* pFilter = NULL;
    const SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();

    /* special mode: use already loaded model ...
        In such case no filter name will be selected and no URL will be given!
        Such informations are not neccessary. We have to create a new view only
        and call setComponent() at the corresponding frame. */
    if( !xModel.is() )
    {

        // get filter by its name directly ...
        if( aFilterName.Len() )
            pFilter = rMatcher.GetFilter4FilterName( aFilterName );

        // or search the preferred filter for the detected type ...
        if( !pFilter && aTypeName.Len() )
            pFilter = rMatcher.GetFilter4EA(aTypeName);

        // or use given document service for detection too!
        if (!pFilter && aServiceName.Len())
        {
            ::comphelper::SequenceAsHashMap lQuery;

            if (aServiceName.Len())
                lQuery[::rtl::OUString::createFromAscii("DocumentService")] <<= ::rtl::OUString(aServiceName);

            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerQuery > xQuery         ( xServiceManager->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.FilterFactory" ) ), ::com::sun::star::uno::UNO_QUERY );

            SfxFilterFlags nMust = SFX_FILTER_IMPORT;
            SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED;

            ::com::sun::star::uno::Reference < com::sun::star::container::XEnumeration > xEnum = xQuery->createSubSetEnumerationByProperties(lQuery.getAsConstNamedValueList());
            while ( xEnum->hasMoreElements() )
            {
                ::comphelper::SequenceAsHashMap aType( xEnum->nextElement() );
                aFilterName = aType.getUnpackedValueOrDefault(::rtl::OUString::createFromAscii("Name"), ::rtl::OUString());
                if (!aFilterName.Len())
                    continue;
                pFilter = rMatcher.GetFilter4FilterName(aFilterName);
                if (!pFilter)
                    continue;
                SfxFilterFlags nFlags = pFilter->GetFilterFlags();
                if (
                    ((nFlags & nMust) == nMust) &&
                    (!(nFlags & nDont ))
                   )
                {
                    break;
                }
                pFilter = 0; //! in case we reach end of enumeration we must have a valid value ...
            }
        }

        // or use interaction to ask user for right filter.
        if ( !pFilter && xInteraction.is() && rURL.Len() )
        {
            ::framework::RequestFilterSelect*                     pRequest = new ::framework::RequestFilterSelect( rURL );
            css::uno::Reference< css::task::XInteractionRequest > xRequest ( pRequest );
            xInteraction->handle( xRequest );
            if( !pRequest->isAbort() )
            {
                aFilterName = pRequest->getFilter();
                pFilter     = rMatcher.GetFilter4FilterName( aFilterName );
            }
        }

        if( !pFilter )
            return sal_False;

        aTypeName = pFilter->GetTypeName();
        // use filter names without prefix
        aFilterName = pFilter->GetFilterName();

        // If detected filter indicates using of an own template format
        // add property "AsTemplate" to descriptor. But supress this step
        // if such property already exists.
        if( pFilter->IsOwnTemplateFormat())
        {
            ::comphelper::SequenceAsHashMap::iterator pIt = lDescriptor.find(PROP_ASTEMPLATE);
            if (pIt == lDescriptor.end())
                lDescriptor[PROP_ASTEMPLATE] <<= sal_True;
        }
    }

    xFrame = rFrame;

    // Achtung: beim Abraeumen der Objekte kann die SfxApp destruiert werden, vorher noch Deinitialize_Impl rufen
    SfxApplication* pApp = SFX_APP();

    // Attention! Because lDescriptor is a copy of rArgs
    // and was might by changed (e.g. for AsTemplate)
    // move all changes there back!
    SfxAllItemSet aSet( pApp->GetPool() );
    TransformParameters( SID_OPENDOC, lDescriptor.getAsConstPropertyValueList(), aSet );

    SFX_ITEMSET_ARG( &aSet, pRefererItem, SfxStringItem, SID_REFERER, FALSE );
    if ( !pRefererItem )
        aSet.Put( SfxStringItem( SID_REFERER, String() ) );

    SfxFrame* pFrame=0;
    for ( pFrame = SfxFrame::GetFirst(); pFrame; pFrame = SfxFrame::GetNext( *pFrame ) )
    {
        if ( pFrame->GetFrameInterface() == xFrame )
            break;
    }

    BOOL bFrameCreated = FALSE;
    if ( !pFrame )
    {
        pFrame = SfxTopFrame::Create( rFrame );
        bFrameCreated = TRUE;
    }

    if ( xModel.is() )
    {
        // !TODO: replace by ViewFactory
        if ( pFrame->GetFrameInterface()->getController().is() )
        {
            // remove old component
            // if a frame was created already, it can't be an SfxComponent!
            pFrame->GetFrameInterface()->setComponent( 0, 0 );
            if ( !bFrameCreated )
                pFrame = SfxTopFrame::Create( rFrame );
        }

        aSet.Put( SfxFrameItem( SID_DOCFRAME, pFrame ) );

        for ( SfxObjectShell* pDoc = SfxObjectShell::GetFirst( NULL, FALSE ); pDoc; pDoc = SfxObjectShell::GetNext( *pDoc, NULL, FALSE ) )
        {
            if ( xModel == pDoc->GetModel() )
            {
                pFrame->SetItemSet_Impl( &aSet );
                ::comphelper::SequenceAsHashMap lUpdateDescr( lDescriptor );
                lUpdateDescr.erase( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Model" ) ) );
                lUpdateDescr.erase( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) ) );

                xModel->attachResource( rURL, lUpdateDescr.getAsConstPropertyValueList() );
                return pFrame->InsertDocument( pDoc );
            }
        }

        DBG_ERROR("Model is not based on SfxObjectShell - wrong frame loader use!");
        return sal_False;
    }

    // check for the URL pattern of our factory URLs
    SfxFrameWeak wFrame = pFrame;
    String aPrefix = String::CreateFromAscii( "private:factory/" );
    String aFact( rURL );
    if ( aPrefix.Len() == aFact.Match( aPrefix ) )
    {
        if ( !aServiceName.Len() )
            aServiceName = SfxObjectShell::GetServiceNameFromFactory( rURL );

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

        WildCard aSearchedFac( aFact.EraseAllChars('4').ToUpperAscii() );
        SFX_ITEMSET_ARG( &aSet, pDocumentTitleItem, SfxStringItem, SID_DOCINFO_TITLE, FALSE );
        INetURLObject aObj( rURL );

        if ( aParam.Len() )
        {
            sal_uInt16 nSlotId = (sal_uInt16) aParam.ToInt32();
            SfxRequest aReq( nSlotId, SFX_CALLMODE_SYNCHRON, pApp->GetPool() );
            aReq.AppendItem( SfxFrameItem ( SID_DOCFRAME, pFrame ) );
            const SfxPoolItem* pRet = pApp->ExecuteSlot( aReq );
            if ( pRet )
            {
                // default must be set to true, because some return values
                // cant be checked ... but indicates "success"!
                bLoadState = sal_True;

                // On the other side some special slots return a boolean state,
                // which can be set to FALSE.
                SfxBoolItem *pItem = PTR_CAST( SfxBoolItem, pRet );
                if (pItem)
                    bLoadState = pItem->GetValue();
            }
            else
                bLoadState = sal_False;

            if ( !bLoadState && bFrameCreated && wFrame && !wFrame->GetCurrentDocument() )
            {
                css::uno::Reference< css::frame::XFrame > axFrame;
                wFrame->SetFrameInterface_Impl( axFrame );
                wFrame->DoClose();
            }

            xFrame.clear();
            xListener.clear();
            return bLoadState;
        }

            String sStandardTemplate   = SfxObjectFactory::GetStandardTemplate( aServiceName );
            BOOL   bUseDefaultTemplate = (sStandardTemplate.Len()>0);
            if( bUseDefaultTemplate )
            {
                // #i21583#
                // Forget the filter, which was detected for the corresponding "private:factory/xxx" URL.
                // We must use the right filter, matching to this document ... not to the private URL!
                const SfxFilter* pTemplateFilter = impl_detectFilterForURL(sStandardTemplate, rArgs, rMatcher);
                if (pTemplateFilter)
                {
                    pFilter     = pTemplateFilter;
                    aFilterName = pTemplateFilter->GetName();
                    // standard template set -> load it "AsTemplate"
                    aSet.Put( SfxStringItem ( SID_FILE_NAME, sStandardTemplate ) );
                    aSet.Put( SfxBoolItem( SID_TEMPLATE, sal_True ) );
                }

                // #119268#
                // something is wrong with the set default template (e.g. unknown format, missing file etcpp)
                // The we have to jump into the following special code, where "private:factory/ URL's are handled.
                // We cant "load" such private/factory URL's!
                else
                    bUseDefaultTemplate = FALSE;
            }

            if ( !bUseDefaultTemplate )
            {
                // execute "NewDocument" request
                /* Attention!
                    #107913#
                    Pointers can't be used to check if two objects are equals!
                    E.g. the memory manager can reuse freed memory ...
                    and then the holded copy of a pointer will point to another
                    (and different!) object - may using the same type then before.
                    In such case we compare one object with itself ...
                 */
                SfxRequest aReq( SID_NEWDOCDIRECT, SFX_CALLMODE_SYNCHRON, aSet );
                aReq.AppendItem( SfxFrameItem( SID_DOCFRAME, pFrame ) );
                aReq.AppendItem( SfxStringItem( SID_NEWDOCDIRECT, aFact ) );

                if ( pDocumentTitleItem )
                    aReq.AppendItem( *pDocumentTitleItem );

                const SfxPoolItem* pRet = pApp->NewDocDirectExec_ImplOld(aReq);
                if (pRet)
                {
                    // default must be set to true, because some return values
                    // cant be checked ... but indicates "success"!
                    bLoadState = sal_True;

                    // On the other side some special slots return a boolean state,
                    // which can be set to FALSE.
                    SfxBoolItem *pItem = PTR_CAST( SfxBoolItem, pRet );
                    if (pItem)
                        bLoadState = pItem->GetValue();
                }
                else
                    bLoadState = sal_False;

                if ( !bLoadState && bFrameCreated && wFrame && !wFrame->GetCurrentDocument() )
                {
                    css::uno::Reference< css::frame::XFrame > axFrame;
                    wFrame->SetFrameInterface_Impl( axFrame );
                    wFrame->DoClose();
                }

                xFrame.clear();
                xListener.clear();
                return bLoadState;
            }
    }
    else
    {
        // load the desired file
        aSet.Put( SfxStringItem ( SID_FILE_NAME, rURL ) );
    }

    aSet.Put( SfxFrameItem( SID_DOCFRAME, pFrame ) );
    aSet.Put( SfxUnoAnyItem( SID_FILLFRAME, css::uno::makeAny(xFrame) ) );
    aSet.Put( SfxStringItem( SID_FILTER_NAME, aFilterName ) );

    // !TODO: replace by direct construction of model (needs view factory)
    DBG_ASSERT( pFilter, "No filter set!" );
    if ( !pFilter )
        return sal_False;

    sal_Bool bDisaster = sal_False;
    SfxObjectShell* pDoc = SfxObjectShell::CreateObject( pFilter->GetServiceName() );
    if ( pDoc )
    {
        css::uno::Reference< css::frame::XLoadable > xLoadable = css::uno::Reference< css::frame::XLoadable >( pDoc->GetModel(), css::uno::UNO_QUERY );
        css::uno::Sequence < css::beans::PropertyValue > aLoadArgs;
        TransformItems( SID_OPENDOC, aSet, aLoadArgs );
        try
        {
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
                We dont know why pFrame can be corrupt here.
                But if it was deleted it shouldnt exists inside our global list.
                May be we can use the damaged pointer to detect if it was removed from
                this global list.
            */
            SfxFrame* pTmp=0;
            for ( pTmp = SfxFrame::GetFirst(); pTmp; pTmp = SfxFrame::GetNext( *pTmp ) )
            {
                if ( pFrame == pTmp )
                    break;
            }

            if ( pTmp == pFrame )
            {
                // !TODO: replace by ViewFactory
                if ( pFrame->GetFrameInterface()->getController().is() )
                {
                    // remove old component
                    // if a frame was created already, it can't be an SfxComponent!
                    // pFrame->GetFrameInterface()->setComponent( 0, 0 );
                    if ( !bFrameCreated )
                        pFrame = SfxTopFrame::Create( rFrame );
                }

                wFrame = pFrame;
                aSet.Put( SfxFrameItem( SID_DOCFRAME, pFrame ) );
                if( pFrame->InsertDocument( pDoc ) )
                {
                    pFrame->GetCurrentViewFrame()->UpdateDocument_Impl();
                    String aURL = pDoc->GetMedium()->GetName();
                    SFX_APP()->Broadcast( SfxStringHint( SID_OPENURL, aURL ) );
                    bLoadState = sal_True;
                }
                else
                    bDisaster = sal_True;
            }
            else
            {
                DBG_ERROR("#121119# You found the reason for a stacktrace! Frame destroyed while loading document.");
                bLoadState = sal_False;
            }
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
                css::uno::Reference< css::frame::XFrame > axFrame;
                wFrame->SetFrameInterface_Impl( axFrame );
                wFrame->DoClose();
                aSet.ClearItem( SID_DOCFRAME );
            }

            css::uno::Reference< css::util::XCloseable > xCloseable( xLoadable, css::uno::UNO_QUERY );
            if ( xCloseable.is() )
            {
                try {
                    xCloseable->close( sal_True );
                } catch ( css::uno::Exception& )
                {}
            }
        }
    }

    xFrame.clear();
    xListener.clear();
    return bLoadState;
}

void SfxFrameLoader_Impl::cancel() throw( RUNTIME_EXCEPTION )
{
}

SFX_IMPL_SINGLEFACTORY( SfxFrameLoader_Impl )

/* XServiceInfo */
UNOOUSTRING SAL_CALL SfxFrameLoader_Impl::getImplementationName() throw( UNORUNTIMEEXCEPTION )
{
    return impl_getStaticImplementationName();
}
                                                                                                                                \
/* XServiceInfo */
sal_Bool SAL_CALL SfxFrameLoader_Impl::supportsService( const UNOOUSTRING& sServiceName ) throw( UNORUNTIMEEXCEPTION )
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
UNOSEQUENCE< UNOOUSTRING > SAL_CALL SfxFrameLoader_Impl::getSupportedServiceNames() throw( UNORUNTIMEEXCEPTION )
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

