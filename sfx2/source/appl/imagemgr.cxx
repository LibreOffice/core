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
#include "sfx2/imagemgr.hxx"
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>

#include <tools/urlobj.hxx>
#include <svtools/imagemgr.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/ustring.hxx>
#include <rtl/logfile.hxx>

#include "sfx2/imgmgr.hxx"
#include <sfx2/app.hxx>
#include <sfx2/unoctitm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/module.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfac.hxx>

#include <boost/unordered_map.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::frame;

typedef boost::unordered_map< ::rtl::OUString,
                       WeakReference< XImageManager >,
                       ::rtl::OUStringHash,
                       ::std::equal_to< ::rtl::OUString > > ModuleIdToImagegMgr;


Image SAL_CALL GetImage(
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    const ::rtl::OUString& aURL,
    bool bBig
)
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
        sal_uInt16 nId = ( sal_uInt16 ) String(aURL).Copy(5).ToInt32();
        const SfxSlot* pSlot = 0;
        if ( xModel.is() )
        {
            Reference < XUnoTunnel > xObj( xModel, UNO_QUERY );
            Sequence < sal_Int8 > aSeq( SvGlobalName( SFX_GLOBAL_CLASSID ).GetByteSequence() );
            sal_Int64 nHandle = xObj.is() ? xObj->getSomething( aSeq ) : 0;
            if ( nHandle )
            {
                SfxObjectShell* pDoc = reinterpret_cast<SfxObjectShell*>(sal::static_int_cast<sal_IntPtr>( nHandle ));
                SfxModule* pModule = pDoc->GetFactory().GetModule();
                pSlot = pModule->GetSlotPool()->GetSlot( nId );
            }
        }
        else
            pSlot = SfxSlotPool::GetSlotPool().GetSlot( nId );

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

    static WeakReference< XModuleManager > m_xModuleManager;

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

            static ModuleIdToImagegMgr m_aModuleIdToImageMgrMap;

            ModuleIdToImagegMgr::iterator pIter = m_aModuleIdToImageMgrMap.find( aModuleId );
            if ( pIter != m_aModuleIdToImageMgrMap.end() )
                xModuleImageManager = pIter->second;
            else
            {
                static WeakReference< XModuleUIConfigurationManagerSupplier > m_xModuleCfgMgrSupplier;

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

            Sequence< Reference< ::com::sun::star::graphic::XGraphic > > aGraphicSeq;
            Sequence< rtl::OUString > aImageCmdSeq( 1 );
            aImageCmdSeq[0] = aCommandURL;

            aGraphicSeq = xModuleImageManager->getImages( nImageType, aImageCmdSeq );

            Reference< ::com::sun::star::graphic::XGraphic > xGraphic = aGraphicSeq[0];
            Image aImage( xGraphic );

            if ( !!aImage )
                return aImage;
            else if ( nProtocol != INET_PROT_UNO && nProtocol != INET_PROT_SLOT )
                return SvFileInformationManager::GetImageNoDefault( aObj, bBig );
        }
    }
    catch ( Exception& )
    {
    }

    return Image();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
