/*************************************************************************
 *
 *  $RCSfile: frmload.cxx,v $
 *
 *  $Revision: 1.74 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 13:39:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
#include <svtools/moduleoptions.hxx>

#ifndef css
namespace css = ::com::sun::star;
#endif

#include "app.hxx"
#include "request.hxx"
#include "sfxsids.hrc"
#include "dispatch.hxx"
#include "sfxuno.hxx"
#include "viewfrm.hxx"
#include "topfrm.hxx"
#include "frame.hxx"
#include "docfac.hxx"
#include "fcontnr.hxx"
#include "docfile.hxx"
#include "docfilt.hxx"
#include "brokenpackageint.hxx"
#include "objshimp.hxx"

SfxFrameLoader_Impl::SfxFrameLoader_Impl( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory )
{
}

SfxFrameLoader_Impl::~SfxFrameLoader_Impl()
{
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
    sal_Bool                                              bReadOnlyTest = lDescriptor.getUnpackedValueOrDefault(PROP_READONLY          , sal_False                                              );
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

    // Achtung: beim Abr�umen der Objekte kann die SfxApp destruiert werden, vorher noch Deinitialize_Impl rufen
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

    if ( !pFrame )
        pFrame = SfxTopFrame::Create( rFrame );

    if ( xModel.is() )
    {
        for ( SfxObjectShell* pDoc = SfxObjectShell::GetFirst( NULL, FALSE ); pDoc; pDoc = SfxObjectShell::GetNext( *pDoc, NULL, FALSE ) )
        {
            if ( xModel == pDoc->GetModel() )
            {
                pFrame->SetItemSet_Impl( &aSet );
                return pFrame->InsertDocument( pDoc );
            }
        }
    }

    // check for the URL pattern of our factory URLs
    BOOL bFactoryURL = FALSE;
    const SfxObjectFactory* pFactory = 0;
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
            {
                SfxObjectShell* pDoc = pFrame->GetCurrentDocument();
                if ( !pDoc )
                {
                    css::uno::Reference< css::frame::XFrame > axFrame;
                    pFrame->SetFrameInterface_Impl( axFrame );
                    pFrame->DoClose();
                }

                bLoadState = sal_False;
            }

            return bLoadState;
        }

            if( SfxObjectFactory::GetStandardTemplate( aServiceName ).Len() )
            {
                // standard template set -> load it "AsTemplate"
                aSet.Put( SfxStringItem ( SID_FILE_NAME, SfxObjectFactory::GetStandardTemplate( aServiceName ) ) );
                aSet.Put( SfxBoolItem( SID_TEMPLATE, sal_True ) );
            }
            else
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
                else if ( xListener.is() )
                {
                    if ( !pFrame->GetCurrentDocument() )
                    {
                        css::uno::Reference< css::frame::XFrame > axFrame;
                        pFrame->SetFrameInterface_Impl( axFrame );
                        pFrame->DoClose();
                    }

                    bLoadState = sal_False;
                }

                xFrame.clear();
                return bLoadState;
            }
    }
    else
    {
        // load the desired file
        aSet.Put( SfxStringItem ( SID_FILE_NAME, rURL ) );
    }

    aSet.Put( SfxFrameItem( SID_DOCFRAME, pFrame ) );
    aSet.Put( SfxStringItem( SID_FILTER_NAME, aFilterName ) );

    // !TODO: replace by direct construction of model (needs view factory)
    SfxObjectShell* pDoc = SfxObjectShell::CreateObject( pFilter->GetServiceName() );
    if ( pDoc )
    {
        css::uno::Reference< css::frame::XLoadable > xLoadable = css::uno::Reference< css::frame::XLoadable >( pDoc->GetModel(), css::uno::UNO_QUERY );
        css::uno::Sequence < css::beans::PropertyValue > aLoadArgs;
        TransformItems( SID_OPENDOC, aSet, aLoadArgs );
        try
        {
            xLoadable->load( aLoadArgs );

            SfxMedium* pMedium = pDoc->GetMedium();
            BOOL bHidden = FALSE;
            SFX_ITEMSET_ARG( pMedium->GetItemSet(), pHidItem, SfxBoolItem, SID_HIDDEN, sal_False);
            if ( pHidItem )
                bHidden = pHidItem->GetValue();

            // !TODO: will be done by Framework!
            pMedium->SetUpdatePickList( !bHidden );

            // !TODO: replace by ViewFactory
            if( pFrame->InsertDocument( pDoc ) )
            {
                pFrame->GetCurrentViewFrame()->UpdateDocument_Impl();
                String aURL = pDoc->GetMedium()->GetName();
                SFX_APP()->Broadcast( SfxStringHint( SID_OPENURL, aURL ) );
                bLoadState = sal_True;
            }
        }
        catch ( css::uno::Exception& )
        {
            if ( pFrame && !pFrame->GetCurrentDocument() )
            {
                // document loading was not successful; close SfxFrame (but not XFrame!)
                ::vos::OGuard aGuard( Application::GetSolarMutex() );
                css::uno::Reference< css::frame::XFrame > axFrame;
                pFrame->SetFrameInterface_Impl( axFrame );
                pFrame->DoClose();
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

