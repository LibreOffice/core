/*************************************************************************
 *
 *  $RCSfile: frmload.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: mba $ $Date: 2001-03-09 10:33:31 $
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

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
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

#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/helper/vclunohelper.hxx>
#endif

#include <rtl/ustring.h>
#include <svtools/itemset.hxx>
#include <vcl/window.hxx>
#include <svtools/eitem.hxx>
#include <svtools/stritem.hxx>
#include <tools/urlobj.hxx>
#include <vos/mutex.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

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
#include "loadenv.hxx"
#include "docfile.hxx"
#include "docfilt.hxx"

Reference< XInterface > SAL_CALL SfxFrameLoaderFactory::createInstance(void) throw(Exception, RuntimeException)
{
    Reference < XFrameLoader > xLoader( pCreateFunction( xSMgr ), UNO_QUERY );
    SfxFrameLoader* pLoader = (SfxFrameLoader*) xLoader.get();
    pLoader->SetFilterName( aImplementationName );
    return xLoader;
}

Reference< XInterface > SAL_CALL SfxFrameLoaderFactory::createInstanceWithArguments(const Sequence<Any>& Arguments) throw(Exception, RuntimeException)
{
    return createInstance();
}

::rtl::OUString SAL_CALL SfxFrameLoaderFactory::getImplementationName() throw(RuntimeException)
{
    return aImplementationName;
}

sal_Bool SAL_CALL SfxFrameLoaderFactory::supportsService(const ::rtl::OUString& ServiceName) throw(RuntimeException)
{
    if ( ServiceName.compareToAscii("com.sun.star.frame.FrameLoader") == COMPARE_EQUAL )
        return sal_True;
    else
        return sal_False;
}

Sequence< ::rtl::OUString > SAL_CALL SfxFrameLoaderFactory::getSupportedServiceNames(void) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aRet(1);
    *aRet.getArray() = ::rtl::OUString::createFromAscii("com.sun.star.frame.FrameLoader");
    return aRet;
}

SfxFrameLoader::SfxFrameLoader( const REFERENCE < ::com::sun::star::lang::XMultiServiceFactory >& xFactory )
    : pMatcher( 0 )
    , pLoader( 0 )
    , bLoadDone( sal_False )
    , bLoadState( sal_False )
{
}

SfxFrameLoader::~SfxFrameLoader()
{
    if ( pLoader )
        pLoader->ReleaseRef();
    delete pMatcher;
}

sal_Bool SAL_CALL SfxFrameLoader::load( const Sequence< PropertyValue >& rArgs, const Reference< XFrame >& rFrame ) throw( RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // Extract URL from given descriptor.
    String rURL;
    rtl::OUString aTypeName;
    String aPreselectedFilterName;

    sal_uInt32 nPropertyCount = rArgs.getLength();
    for( sal_uInt32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        if( rArgs[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("FileName")) )
        {
            ::rtl::OUString sTemp;
            rArgs[nProperty].Value >>= sTemp;
            rURL = sTemp;
        }
        if( rArgs[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("TypeName")) )
        {
            ::rtl::OUString sTemp;
            rArgs[nProperty].Value >>= sTemp;

            // Convert new type name to filter name in old format ( always gives the name of the preferred filter )
            aFilterName = SfxFilterContainer::ConvertToOldFilterName(sTemp);
        }
        if( rArgs[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("FilterName")) )
        {
            ::rtl::OUString sTemp;
            rArgs[nProperty].Value >>= sTemp;
            aPreselectedFilterName = sTemp;
        }
    }

    if ( aPreselectedFilterName.Len() )
        aFilterName = aPreselectedFilterName;

    xFrame = rFrame;

    // Achtung: beim Abräumen der Objekte kann die SfxApp destruiert werden, vorher noch Deinitialize_Impl rufen
    SfxApplication* pApp = SFX_APP();

    SfxAllItemSet aSet( pApp->GetPool() );
    TransformParameters( SID_OPENDOC, rArgs, aSet );

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

    BOOL bFactoryURL = FALSE;
    const SfxObjectFactory* pFactory = 0;
    String aFact( rURL );
    String aPrefix = String::CreateFromAscii( "private:factory/" );
    String aParam;
    if ( aPrefix.Len() == aFact.Match( aPrefix ) )
    {
        bFactoryURL = TRUE;
        aFact.Erase( 0, aPrefix.Len() );
        USHORT nPos = aFact.Search( '?' );
        if ( nPos != STRING_NOTFOUND )
        {
            USHORT nParamPos = aFact.Search( String::CreateFromAscii("slot="), nPos );
            if ( nParamPos != STRING_NOTFOUND )
                aParam = aFact.Copy( nParamPos+5, aFact.Len() );
            aFact.Erase( nPos, aFact.Len() );
        }
    }
    else
        aFact = aFilterName.GetToken( 0, ':' );

    WildCard aSearchedFac( aFact.EraseAllChars('4').ToUpperAscii() );
    for( USHORT n = SfxObjectFactory::GetObjectFactoryCount_Impl(); !pFactory && n--; )
    {
        pFactory = &SfxObjectFactory::GetObjectFactory_Impl( n );
        if( !aSearchedFac.Matches( String::CreateFromAscii( pFactory->GetShortName() ).ToUpperAscii() ) )
            pFactory = 0;
    }

    if ( bFactoryURL && pFactory )
    {
        INetURLObject aObj( rURL );
        if ( aParam.Len() )
        {
            sal_uInt16 nSlotId = (sal_uInt16) aParam.ToInt32();
            SfxModule* pMod = pFactory->GetModule()->Load();
            SfxRequest aReq( nSlotId, SFX_CALLMODE_SYNCHRON, pMod->GetPool() );
            aReq.AppendItem( SfxStringItem ( SID_FILE_NAME, rURL ) );
            aReq.AppendItem( SfxFrameItem ( SID_DOCFRAME, pFrame ) );
            const SfxPoolItem* pRet = pMod->ExecuteSlot( aReq );
            if ( pRet )
                bLoadState = sal_True;
            else
            {
                if ( !pFrame->GetCurrentDocument() )
                {
                    pFrame->SetFrameInterface_Impl( Reference < XFrame >() );
                    pFrame->DoClose();
                }
                bLoadState = sal_False;
            }
            return bLoadState;
        }

        String aPathName( aObj.GetMainURL() );
        if( pFactory->GetStandardTemplate().Len() )
        {
            aSet.Put( SfxStringItem ( SID_FILE_NAME, pFactory->GetStandardTemplate() ) );
            aSet.Put( SfxBoolItem( SID_TEMPLATE, sal_True ) );
        }
        else
        {
            SfxViewShell *pView = pFrame->GetCurrentViewFrame() ? pFrame->GetCurrentViewFrame()->GetViewShell() : NULL;
            SfxRequest aReq( SID_NEWDOCDIRECT, SFX_CALLMODE_SYNCHRON, aSet );
            aReq.AppendItem( SfxFrameItem( SID_DOCFRAME, pFrame ) );
            aReq.AppendItem( SfxStringItem( SID_NEWDOCDIRECT, String::CreateFromAscii(pFactory->GetShortName()) ) );
            const SfxPoolItem* pRet = pApp->ExecuteSlot( aReq );
            if ( pFrame->GetCurrentViewFrame() && pView != pFrame->GetCurrentViewFrame()->GetViewShell() )
            {
                bLoadState = sal_True;
            }
            else if ( xListener.is() )
            {
                if ( !pFrame->GetCurrentDocument() )
                {
                    pFrame->SetFrameInterface_Impl( Reference < XFrame >() );
                    pFrame->DoClose();
                }
                bLoadState = sal_False;
            }

            xFrame = Reference < XFrame >();
            return bLoadState;
        }
    }
    else
    {
        aSet.Put( SfxStringItem ( SID_FILE_NAME, rURL ) );
    }

    aSet.Put( SfxFrameItem( SID_DOCFRAME, pFrame ) );
    aSet.Put( SfxStringItem( SID_FILTER_NAME, aFilterName ) );
    pLoader = LoadEnvironment_Impl::Create( aSet );
    pLoader->AddRef();
    pLoader->SetDoneLink( LINK( this, SfxFrameLoader, LoadDone_Impl ) );

    if ( !pFactory )
    {
        const SfxFilter* pFilter = SFX_APP()->GetFilterMatcher().GetFilter4FilterName( aFilterName );
        SfxFactoryFilterContainer* pCont = pFilter ? (SfxFactoryFilterContainer*) pFilter->GetFilterContainer() : NULL;
        if ( pCont )
            pFactory = &pCont->GetFactory();
    }

    if ( pFactory )
    {
        bLoadDone = sal_False;
        pMatcher = new SfxFilterMatcher( pFactory->GetFilterContainer() );
        pLoader->SetFilterMatcher( pMatcher );
        pLoader->Start();
        while( bLoadDone == sal_False )
        {
            Application::Yield();
        }
    }

    return bLoadState;
}

void SfxFrameLoader::cancel() throw( RUNTIME_EXCEPTION )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( pLoader )
        pLoader->CancelTransfers();
}

IMPL_LINK( SfxFrameLoader, LoadDone_Impl, void*, pVoid )
{
    DBG_ASSERT( pLoader, "No Loader created, but LoadDone ?!" );

    if ( pLoader->GetError() )
    {
        SfxFrame* pFrame = pLoader->GetFrame();
        if ( pFrame && !pFrame->GetCurrentDocument() )
        {
            ::vos::OGuard aGuard( Application::GetSolarMutex() );
            pFrame->SetFrameInterface_Impl( Reference < XFrame >() );
            pFrame->DoClose();
        }
        bLoadDone  = sal_True ;
        bLoadState = sal_False;
    }
    else
    {
        bLoadDone  = sal_True;
        bLoadState = sal_True;
    }

    xFrame = Reference < XFrame >();
    xListener = Reference < XLoadEventListener >();
    return NULL;
}

SfxObjectFactory& SfxFrameLoader_Impl::GetFactory()
{
    SfxObjectFactory* pFactory = 0;
    String aFact = GetFilterName().GetToken( ':', 0 );
    WildCard aSearchedFac( aFact.EraseAllChars('4').ToUpperAscii() );
    for( USHORT n = SfxObjectFactory::GetObjectFactoryCount_Impl(); !pFactory && n--; )
    {
        pFactory = (SfxObjectFactory*) &SfxObjectFactory::GetObjectFactory_Impl( n );
        if( !aSearchedFac.Matches( String::CreateFromAscii( pFactory->GetShortName() ).ToUpperAscii() ) )
            pFactory = 0;
    }

    return *pFactory;
}

::rtl::OUString SAL_CALL SfxFrameLoader::detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lDescriptor ) throw( ::com::sun::star::uno::RuntimeException )
{
    // Extract URL from given descriptor.
    String aURL;
    ::rtl::OUString sTemp;
    rtl::OUString aTypeName;
    String aPreselectedFilterName;

    sal_uInt32 nPropertyCount = lDescriptor.getLength();
    sal_Int32 nIndexOfFilterName = nPropertyCount;
    for( sal_uInt32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("FileName")) )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aURL = sTemp;
        }
        if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("TypeName")) )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aTypeName = sTemp;
            aFilterName = SfxFilterContainer::ConvertToOldFilterName( aTypeName );
        }
        if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("FilterName")) )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aPreselectedFilterName = sTemp;
            nIndexOfFilterName = nProperty;
        }
    }

    // If url protocol = "private:factory/*" ... there is no document to detect!
    // That will create a new one. We can break detection.
    String aPrefix = String::CreateFromAscii( "private:factory/" );
    if( aURL.Match( aPrefix ) == aPrefix.Len() )
    {
        // Convert old to new filter name.
        aFilterName = aTypeName;
    }
    else if ( !aFilterName.Len() && !aPreselectedFilterName.Len() && aTypeName.getLength() )
    {
        // generic detector will not be called for new types except when this type does not support deep detection
        // in this case it is correct to use the result of the shallow detection ( if there is any )
        return aTypeName;
    }
    else
    {
        SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();
        SfxErrorContext aCtx( ERRCTX_SFX_OPENDOC, aURL );
        const SfxFilter* pNew = NULL;

        SfxApplication* pApp = SFX_APP();
        SfxAllItemSet *pSet = new SfxAllItemSet( pApp->GetPool() );
        TransformParameters( SID_OPENDOC, lDescriptor, *pSet );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        SfxMedium aMedium( aURL, (STREAM_READ | STREAM_SHARE_DENYNONE), sal_False, NULL, pSet );
        if ( aMedium.IsStorage() )
            aMedium.GetStorage();
        else
            aMedium.GetInStream();

        // Access to Medium was successful ?
        const SfxFilter* pFilter = NULL;
        if ( aMedium.GetErrorCode() == ERRCODE_NONE )
        {
        /*      String aMime;
                aMedium.GetMIMEAndRedirect( aMime );
                if( aMime.Len() )
                    pFilter = rMatcher.GetFilter4Mime( aMime );
        */
            // can't detect filter for external filters!
            SfxFilterFlags nMust = SFX_FILTER_IMPORT, nDont = SFX_FILTER_NOTINSTALLED | SFX_FILTER_STARONE;
            ErrCode nErr = ERRCODE_ABORT;
            const SfxFilter* pOldFilter = NULL;
            if ( aPreselectedFilterName.Len() )
            {
                // first check the preselected filter
                pOldFilter = pFilter = rMatcher.GetFilter( aPreselectedFilterName );
                if ( pFilter )
                {
                    SfxFilterFlags nFlags = pFilter->GetFilterFlags();
                    if ( ( nFlags & nMust ) == nMust && ( nFlags & nDont ) == 0 )
                    {
                        pSet->Put( SfxStringItem( SID_FILTER_NAME, aPreselectedFilterName ) );
                        nErr = pFilter->GetFilterContainer()->GetFilter4Content( aMedium, &pFilter );
                    }
                    else
                        // filterflags not suitable
                        pFilter = NULL;
                }

                if ( !pFilter )
                    // preselected filter doesn't fit, erase filter name from media descriptor
                    lDescriptor[nIndexOfFilterName].Value <<= ::rtl::OUString();
            }

            if ( !pFilter )
            {
                // preselected filter was not set or was bullshit
                if( !aFilterName.Len() && aTypeName.getLength() )
                    // preselected type name may be a new internal filter ( name is not in the conversion table )
                    // or an external type name that may also be identical with an external filter name
                    aFilterName = aTypeName;

                // next try the detected type
                pOldFilter = pFilter = rMatcher.GetFilter( aFilterName );
                if ( !pFilter || ( pFilter->GetFlags() & SFX_FILTER_STARONE ) )
                    // external types or filters can't be detected with this service
                    return aTypeName;

                SfxFilterFlags nFlags = pFilter->GetFilterFlags();
                if ( ( nFlags & nMust ) == nMust && ( nFlags & nDont ) == 0 )
                {
                    pSet->Put( SfxStringItem( SID_FILTER_NAME, aFilterName ) );
                    nErr = pFilter->GetFilterContainer()->GetFilter4Content( aMedium, &pFilter );
                }
                else
                    // filterflags not suitable
                    pFilter = NULL;
            }

            // No error while reading from medium ?
            if ( aMedium.GetErrorCode() == ERRCODE_NONE )
            {
                if ( !pFilter || pOldFilter == pFilter && nErr != ERRCODE_NONE )
                {
                    // try simplest file lookup: clipboard format in storage
                    pFilter = NULL;
                    SvStorageRef aStor = aMedium.GetStorage();
                    SfxFilterFlags nFlags = SFX_FILTER_IMPORT | SFX_FILTER_PREFERED;
                    if ( aStor.Is() )
                    {
                        pFilter = rMatcher.GetFilter4ClipBoardId( aStor->GetFormat(), nFlags );
                        if ( !pFilter || ( (pFilter)->GetFilterFlags() & nMust ) != nMust || ( (pFilter)->GetFilterFlags() & nDont ) != 0 )
                            pFilter = rMatcher.GetFilter4ClipBoardId( aStor->GetFormat() );
                    }
                    if ( pFilter )
                        nErr = pFilter->GetFilterContainer()->GetFilter4Content( aMedium, &pFilter );
                }

                // No error while reading from medium ?
                if ( aMedium.GetErrorCode() == ERRCODE_NONE )
                {
                    if ( !pFilter || pOldFilter == pFilter && nErr != ERRCODE_NONE )
                    {
                        pFilter = NULL;
                        nErr = rMatcher.GetFilter4Content( aMedium, &pFilter );
                    }
                }
            }

            if ( aMedium.GetErrorCode() != ERRCODE_NONE )
            {
                // when access to medium gives an error, the filter can't be valid
                pFilter = NULL;
                ErrorHandler::HandleError( aMedium.GetError() );
            }
        }

        if ( !pFilter )
            aFilterName.Erase();
        else
            aFilterName = SfxFilterContainer::ConvertToNewFilterName( pFilter->GetName() );
    }

    if ( aFilterName.Len() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >  xMan = ::comphelper::getProcessServiceFactory();
        ::com::sun::star::uno::Reference < ::com::sun::star::container::XNameAccess > xFilters (
                xMan->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.FilterFactory" ) ), ::com::sun::star::uno::UNO_QUERY );

            ::com::sun::star::uno::Any aAny = xFilters->getByName( aFilterName );
            ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue > aProps;
            if ( aAny >>= aProps )
            {
                // evaluate properties : get document service name
                const ::com::sun::star::beans::PropertyValue* pProps = aProps.getConstArray();
                ::rtl::OUString aTmp;
                aAny = aProps[0].Value;
                if ( aAny >>= aTmp )
                    aTypeName = aTmp;
            }
    }

    return aTypeName;
}

SFX_IMPL_XINTERFACE_0( SfxFrameLoader_Impl, SfxFrameLoader )
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
    UNOSEQUENCE< UNOOUSTRING > seqServiceNames( 2 );
    seqServiceNames.getArray() [0] = UNOOUSTRING::createFromAscii( "com.sun.star.frame.FrameLoader" );
    seqServiceNames.getArray() [1] = UNOOUSTRING::createFromAscii( "com.sun.star.frame.ExtendedFilterDetect" );
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

SfxFrameLoader_Impl::SfxFrameLoader_Impl( const Reference < XMultiServiceFactory >& xFactory )
    : SfxFrameLoader( xFactory )
{
}


