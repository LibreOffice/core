/*************************************************************************
 *
 *  $RCSfile: imagemgr.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 13:32:36 $
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

// includes --------------------------------------------------------------
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XIMAGEMANAGER_HPP_
#include <drafts/com/sun/star/ui/XImageManager.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <drafts/com/sun/star/frame/XModuleManager.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XMODULEUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <drafts/com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_IMAGETYPE_HPP_
#include <drafts/com/sun/star/ui/ImageType.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <drafts/com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#endif

#include <tools/urlobj.hxx>
#include <svtools/imagemgr.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/ustring.hxx>

#include "imgmgr.hxx"
#include "app.hxx"
#include "unoctitm.hxx"
#include "dispatch.hxx"
#include "msg.hxx"
#include "msgpool.hxx"
#include "viewfrm.hxx"
#include "module.hxx"
#include "objsh.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::drafts::com::sun::star::ui;
using namespace ::drafts::com::sun::star::frame;

typedef std::hash_map< ::rtl::OUString,
                       WeakReference< XImageManager >,
                       ::rtl::OUStringHash,
                       ::std::equal_to< ::rtl::OUString > > ModuleIdToImagegMgr;

static WeakReference< XModuleManager >                        m_xModuleManager;
static WeakReference< XModuleUIConfigurationManagerSupplier > m_xModuleCfgMgrSupplier;
static WeakReference< XURLTransformer >                       m_xURLTransformer;
static ModuleIdToImagegMgr                                    m_aModuleIdToImageMgrMap;

Image SAL_CALL GetImage( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const ::rtl::OUString& aURL, BOOL bBig, BOOL bHiContrast )
{
    if ( !rFrame.is() )
        return Image();

    INetURLObject aObj( aURL );
    INetProtocol  nProtocol = aObj.GetProtocol();

    rtl::OUString aCommandURL( aURL );
    if ( nProtocol == INET_PROT_SLOT )
    {
        // Support old way to retrieve image via slot URL
        Reference< XURLTransformer > xURLTransformer = m_xURLTransformer;
        if ( !xURLTransformer.is() )
        {
            xURLTransformer = Reference< XURLTransformer >(
                                ::comphelper::getProcessServiceFactory()->createInstance(
                                    rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )),
                                UNO_QUERY );
            m_xURLTransformer = xURLTransformer;
        }

        URL aTargetURL;
        aTargetURL.Complete = aURL;
        xURLTransformer->parseStrict( aTargetURL );

        SfxViewFrame* pViewFrame = NULL;
        Reference < XController > xController;
        if ( rFrame.is() )
            xController = rFrame->getController();

        Reference < XDispatchProvider > xProvider( xController, UNO_QUERY );
        if ( xProvider.is() )
        {
            Reference < XDispatch > xDisp = xProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
            if ( xDisp.is() )
            {
                Reference< XUnoTunnel > xTunnel( xDisp, UNO_QUERY );
                SfxOfficeDispatch* pDisp = NULL;
                if ( xTunnel.is() )
                {
                    sal_Int64 nImplementation = xTunnel->getSomething(SfxOfficeDispatch::impl_getStaticIdentifier());
                    pDisp = (SfxOfficeDispatch*)(nImplementation);
                }

                if ( pDisp )
                    pViewFrame = pDisp->GetDispatcher_Impl()->GetFrame();
            }
        }

        USHORT nId = ( USHORT ) aTargetURL.Path.toInt32();
        SfxSlotPool& rPool = SFX_APP()->GetSlotPool( pViewFrame );
        const SfxSlot* pSlot = rPool.GetSlot( nId );
        if ( pSlot )
        {
            aCommandURL = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:" ));
            aCommandURL += rtl::OUString::createFromAscii( pSlot->GetUnoName() );
        }
        else
            aCommandURL = rtl::OUString();
    }

    Reference< XImageManager > xDocImgMgr;
    Reference< XController > xController = rFrame->getController();
    Reference< XModel > xModel;
    if ( xController.is() )
    {
        xModel = xController->getModel();
        if ( xModel.is() )
        {
            Reference< XUIConfigurationManagerSupplier > xSupplier( xModel, UNO_QUERY );
            if ( xSupplier.is() )
            {
                Reference< XUIConfigurationManager > xDocUICfgMgr( xSupplier->getUIConfigurationManager(), UNO_QUERY );
                xDocImgMgr = Reference< XImageManager >( xDocUICfgMgr->getImageManager(), UNO_QUERY );
            }
        }
    }

    sal_Int16 nImageType( ::drafts::com::sun::star::ui::ImageType::COLOR_NORMAL|
                            ::drafts::com::sun::star::ui::ImageType::SIZE_DEFAULT );
    if ( bBig )
        nImageType |= ::drafts::com::sun::star::ui::ImageType::SIZE_LARGE;
    if ( bHiContrast )
        nImageType |= ::drafts::com::sun::star::ui::ImageType::COLOR_HIGHCONTRAST;

    if ( xDocImgMgr.is() )
    {
        Sequence< Reference< ::com::sun::star::graphic::XGraphic > > aGraphicSeq;
        Sequence< rtl::OUString > aImageCmdSeq( 1 );
        aImageCmdSeq[0] = aCommandURL;

        try
        {
            aGraphicSeq = xDocImgMgr->getImages( nImageType, aImageCmdSeq );
            Reference< ::com::sun::star::graphic::XGraphic > xGraphic = aGraphicSeq[0];
            Image aImage( xGraphic );

            if ( !!aImage )
                return aImage;
        }
        catch ( Exception& )
        {
        }
    }

    Reference< XModuleManager > xModuleManager = m_xModuleManager;

    if ( !xModuleManager.is() )
    {
        xModuleManager = Reference< XModuleManager >(
                            ::comphelper::getProcessServiceFactory()->createInstance(
                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                    "drafts.com.sun.star.frame.ModuleManager" ))),
                            UNO_QUERY );
        m_xModuleManager = xModuleManager;
    }

    try
    {
        if ( aCommandURL.getLength() > 0 )
        {
            Reference< XImageManager > xModuleImageManager;
            rtl::OUString aModuleId = xModuleManager->identify( rFrame );
            ModuleIdToImagegMgr::iterator pIter = m_aModuleIdToImageMgrMap.find( aModuleId );
            if ( pIter != m_aModuleIdToImageMgrMap.end() )
                xModuleImageManager = pIter->second;
            else
            {
                Reference< XModuleUIConfigurationManagerSupplier > xModuleCfgMgrSupplier = m_xModuleCfgMgrSupplier;

                if ( !xModuleCfgMgrSupplier.is() )
                {
                    xModuleCfgMgrSupplier = Reference< XModuleUIConfigurationManagerSupplier >(
                                                ::comphelper::getProcessServiceFactory()->createInstance(
                                                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                        "drafts.com.sun.star.ui.ModuleUIConfigurationManagerSupplier" ))),
                                                UNO_QUERY );

                    m_xModuleCfgMgrSupplier = xModuleCfgMgrSupplier;
                }

                Reference< XUIConfigurationManager > xUICfgMgr = xModuleCfgMgrSupplier->getUIConfigurationManager( aModuleId );
                xModuleImageManager = Reference< XImageManager >( xUICfgMgr->getImageManager(), UNO_QUERY );
                m_aModuleIdToImageMgrMap.insert( ModuleIdToImagegMgr::value_type( aModuleId, xModuleImageManager ));
            }

            sal_Int16 nImageType( ::drafts::com::sun::star::ui::ImageType::COLOR_NORMAL|
                                  ::drafts::com::sun::star::ui::ImageType::SIZE_DEFAULT );
            if ( bBig )
                nImageType |= ::drafts::com::sun::star::ui::ImageType::SIZE_LARGE;
            if ( bHiContrast )
                nImageType |= ::drafts::com::sun::star::ui::ImageType::COLOR_HIGHCONTRAST;

            Sequence< Reference< ::com::sun::star::graphic::XGraphic > > aGraphicSeq;
            Sequence< rtl::OUString > aImageCmdSeq( 1 );
            aImageCmdSeq[0] = aCommandURL;

            aGraphicSeq = xModuleImageManager->getImages( nImageType, aImageCmdSeq );

            Reference< ::com::sun::star::graphic::XGraphic > xGraphic = aGraphicSeq[0];
            Image aImage( xGraphic );

            if ( !!aImage )
                return aImage;
            else if ( nProtocol != INET_PROT_UNO && nProtocol != INET_PROT_SLOT )
                return SvFileInformationManager::GetImageNoDefault( aObj, bBig, bHiContrast );
        }
    }
    catch ( Exception& )
    {
    }

    return Image();
}
/*
    Reference< XController > xController = rFrame->getController();
    if ( xController.is() )
    {
        Reference< XModel > xModel = xController->getModel();
        if ( xModel )
        {
        }
    }
*/
/*
    m_aModuleIdToImageMgrMap



    Reference< XImageManager >

    INetURLObject aObj( aURL );
    INetProtocol nProtocol = aObj.GetProtocol();
    switch ( nProtocol )
    {
        case INET_PROT_UNO :
        case INET_PROT_SLOT :
        {
            URL aTargetURL;
            aTargetURL.Complete = aURL;
            Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
            xTrans->parseStrict( aTargetURL );

            SfxViewFrame* pViewFrame = NULL;
            Reference < XController > xController;
            if ( rFrame.is() )
                xController = rFrame->getController();

            Reference < XDispatchProvider > xProvider( xController, UNO_QUERY );
            if ( xProvider.is() )
            {
                Reference < XDispatch > xDisp = xProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
                if ( xDisp.is() )
                {
                    Reference< XUnoTunnel > xTunnel( xDisp, UNO_QUERY );
                    SfxOfficeDispatch* pDisp = NULL;
                    if ( xTunnel.is() )
                    {
                        sal_Int64 nImplementation = xTunnel->getSomething(SfxOfficeDispatch::impl_getStaticIdentifier());
                        pDisp = (SfxOfficeDispatch*)(nImplementation);
                    }

                    if ( pDisp )
                        pViewFrame = pDisp->GetDispatcher_Impl()->GetFrame();
                }
            }

            USHORT nId = 0;
            if ( nProtocol == INET_PROT_UNO )
            {
                SfxSlotPool& rPool = SFX_APP()->GetSlotPool( pViewFrame );
                const SfxSlot* pSlot = rPool.GetUnoSlot( aTargetURL.Path );
                if ( pSlot )
                    nId = pSlot->GetSlotId();
            }
            else
                nId = ( USHORT ) aTargetURL.Path.toInt32();

            SfxModule* pModule = pViewFrame ? pViewFrame->GetObjectShell()->GetModule() : NULL;
            if ( nId )
            {
                if ( pViewFrame )
                    return pViewFrame->GetImageManager()->GetImage( nId, pModule, bBig, bHiContrast );
                else
                {
                    return SFX_APP()->GetImageManager_Impl()->GetImage( nId, NULL, bBig, bHiContrast );
                }
            }
            break;
        }

        case INET_PROT_NOT_VALID :
        {
            return Image();
            break;
        }
    }

    return SvFileInformationManager::GetImageNoDefault( aObj, bBig, bHiContrast );
}
*/
