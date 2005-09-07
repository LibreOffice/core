/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imagemgr.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:40:32 $
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

#include "sal/config.h"

#include "imagemgr.hxx"

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XIMAGEMANAGER_HPP_
#include <com/sun/star/ui/XImageManager.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <com/sun/star/frame/XModuleManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XMODULEUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_IMAGETYPE_HPP_
#include <com/sun/star/ui/ImageType.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#endif

#include <tools/urlobj.hxx>
#include <svtools/imagemgr.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/ustring.hxx>
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#include "imgmgr.hxx"
#include "app.hxx"
#include "unoctitm.hxx"
#include "dispatch.hxx"
#include "msg.hxx"
#include "msgpool.hxx"
#include "viewfrm.hxx"
#include "module.hxx"
#include "objsh.hxx"
#include "docfac.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::frame;

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
    // TODO/LATeR: shouldn't this become a method at SfxViewFrame?! That would save the UnoTunnel
    if ( !rFrame.is() )
        return Image();

    INetURLObject aObj( aURL );
    INetProtocol  nProtocol = aObj.GetProtocol();

    Reference < XController > xController;
    Reference < XModel > xModel;
    if ( rFrame.is() )
        xController = rFrame->getController();
    if ( xController.is() )
        xModel = xController->getModel();

    rtl::OUString aCommandURL( aURL );
    if ( nProtocol == INET_PROT_SLOT )
    {
        /*
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
        USHORT nId = ( USHORT ) aTargetURL.Path.toInt32();*/
        USHORT nId = ( USHORT ) String(aURL).Copy(5).ToInt32();
        const SfxSlot* pSlot = 0;
        if ( xModel.is() )
        {
            Reference < XUnoTunnel > xObj( xModel, UNO_QUERY );
            Sequence < sal_Int8 > aSeq( SvGlobalName( SFX_GLOBAL_CLASSID ).GetByteSequence() );
            sal_Int64 nHandle = xObj.is() ? xObj->getSomething( aSeq ) : 0;
            if ( nHandle )
            {
                SfxObjectShell* pDoc = (SfxObjectShell*) (sal_Int32*) nHandle;
                SfxModule* pModule = pDoc->GetFactory().GetModule();
                pSlot = pModule->GetSlotPool()->GetSlot( nId );
            }
        }
        else
            pSlot = SFX_APP()->GetSlotPool().GetSlot( nId );

        if ( pSlot )
        {
            aCommandURL = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:" ));
            aCommandURL += rtl::OUString::createFromAscii( pSlot->GetUnoName() );
        }
        else
            aCommandURL = rtl::OUString();
    }

    Reference< XImageManager > xDocImgMgr;
    if ( xModel.is() )
    {
        Reference< XUIConfigurationManagerSupplier > xSupplier( xModel, UNO_QUERY );
        if ( xSupplier.is() )
        {
            Reference< XUIConfigurationManager > xDocUICfgMgr( xSupplier->getUIConfigurationManager(), UNO_QUERY );
            xDocImgMgr = Reference< XImageManager >( xDocUICfgMgr->getImageManager(), UNO_QUERY );
        }
    }

    sal_Int16 nImageType( ::com::sun::star::ui::ImageType::COLOR_NORMAL|
                            ::com::sun::star::ui::ImageType::SIZE_DEFAULT );
    if ( bBig )
        nImageType |= ::com::sun::star::ui::ImageType::SIZE_LARGE;
    if ( bHiContrast )
        nImageType |= ::com::sun::star::ui::ImageType::COLOR_HIGHCONTRAST;

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
                                    "com.sun.star.frame.ModuleManager" ))),
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
                                                        "com.sun.star.ui.ModuleUIConfigurationManagerSupplier" ))),
                                                UNO_QUERY );

                    m_xModuleCfgMgrSupplier = xModuleCfgMgrSupplier;
                }

                Reference< XUIConfigurationManager > xUICfgMgr = xModuleCfgMgrSupplier->getUIConfigurationManager( aModuleId );
                xModuleImageManager = Reference< XImageManager >( xUICfgMgr->getImageManager(), UNO_QUERY );
                m_aModuleIdToImageMgrMap.insert( ModuleIdToImagegMgr::value_type( aModuleId, xModuleImageManager ));
            }

            sal_Int16 nImageType( ::com::sun::star::ui::ImageType::COLOR_NORMAL|
                                  ::com::sun::star::ui::ImageType::SIZE_DEFAULT );
            if ( bBig )
                nImageType |= ::com::sun::star::ui::ImageType::SIZE_LARGE;
            if ( bHiContrast )
                nImageType |= ::com::sun::star::ui::ImageType::COLOR_HIGHCONTRAST;

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
