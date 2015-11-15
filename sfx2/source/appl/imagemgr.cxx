/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sfx2/imagemgr.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>

#include <tools/urlobj.hxx>
#include <svtools/imagemgr.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/ustring.hxx>

#include <sfx2/imgmgr.hxx>
#include <sfx2/app.hxx>
#include <sfx2/unoctitm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/module.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfac.hxx>

#include <unordered_map>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::ui;

typedef std::unordered_map< OUString,
                       WeakReference< XImageManager >,
                       OUStringHash,
                       ::std::equal_to< OUString > > ModuleIdToImagegMgr;


Image SAL_CALL GetImage(
    const css::uno::Reference< css::frame::XFrame >& rFrame,
    const OUString& aURL,
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

    OUString aCommandURL( aURL );
    if ( nProtocol == INetProtocol::Slot )
    {
        sal_uInt16 nId = ( sal_uInt16 ) aURL.copy(5).toInt32();
        const SfxSlot* pSlot = nullptr;
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
            aCommandURL = ".uno:";
            aCommandURL += OUString::createFromAscii( pSlot->GetUnoName() );
        }
        else
            aCommandURL.clear();
    }

    Reference< XImageManager > xDocImgMgr;
    if ( xModel.is() )
    {
        Reference< XUIConfigurationManagerSupplier > xSupplier( xModel, UNO_QUERY );
        if ( xSupplier.is() )
        {
            Reference< XUIConfigurationManager > xDocUICfgMgr( xSupplier->getUIConfigurationManager(), UNO_QUERY );
            xDocImgMgr.set( xDocUICfgMgr->getImageManager(), UNO_QUERY );
        }
    }

    sal_Int16 nImageType( css::ui::ImageType::COLOR_NORMAL| css::ui::ImageType::SIZE_DEFAULT );
    if ( bBig )
        nImageType |= css::ui::ImageType::SIZE_LARGE;

    if ( xDocImgMgr.is() )
    {
        Sequence< Reference< css::graphic::XGraphic > > aGraphicSeq;
        Sequence<OUString> aImageCmdSeq { aCommandURL };

        try
        {
            aGraphicSeq = xDocImgMgr->getImages( nImageType, aImageCmdSeq );
            Reference< css::graphic::XGraphic > xGraphic = aGraphicSeq[0];
            Image aImage( xGraphic );

            if ( !!aImage )
                return aImage;
        }
        catch (const Exception&)
        {
        }
    }

    static WeakReference< XModuleManager2 > m_xModuleManager;

    Reference< XModuleManager2 > xModuleManager = m_xModuleManager;

    if ( !xModuleManager.is() )
    {
        xModuleManager = ModuleManager::create(::comphelper::getProcessComponentContext());
        m_xModuleManager = xModuleManager;
    }

    try
    {
        if ( !aCommandURL.isEmpty() )
        {
            Reference< XImageManager > xModuleImageManager;
            OUString aModuleId = xModuleManager->identify( rFrame );

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
                    xModuleCfgMgrSupplier = theModuleUIConfigurationManagerSupplier::get(
                                              ::comphelper::getProcessComponentContext() );

                    m_xModuleCfgMgrSupplier = xModuleCfgMgrSupplier;
                }

                Reference< XUIConfigurationManager > xUICfgMgr = xModuleCfgMgrSupplier->getUIConfigurationManager( aModuleId );
                xModuleImageManager.set( xUICfgMgr->getImageManager(), UNO_QUERY );
                m_aModuleIdToImageMgrMap.insert( ModuleIdToImagegMgr::value_type( aModuleId, xModuleImageManager ));
            }

            Sequence< Reference< css::graphic::XGraphic > > aGraphicSeq;
            Sequence<OUString> aImageCmdSeq { aCommandURL };

            aGraphicSeq = xModuleImageManager->getImages( nImageType, aImageCmdSeq );

            Reference< css::graphic::XGraphic > xGraphic = aGraphicSeq[0];
            Image aImage( xGraphic );

            if ( !!aImage )
                return aImage;
            else if ( nProtocol != INetProtocol::Uno && nProtocol != INetProtocol::Slot )
                return SvFileInformationManager::GetImageNoDefault( aObj, bBig );
        }
    }
    catch (const Exception&)
    {
    }

    return Image();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
