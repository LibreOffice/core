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

#include <uifactory/menubarfactory.hxx>

#include <uielement/menubarwrapper.hxx>

#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace ::com::sun::star::ui;

namespace framework
{

MenuBarFactory::MenuBarFactory( const css::uno::Reference< css::uno::XComponentContext >& xContext )
    : m_xContext( xContext )
{
}

MenuBarFactory::~MenuBarFactory()
{
}

// XUIElementFactory
Reference< XUIElement > SAL_CALL MenuBarFactory::createUIElement(
    const OUString& ResourceURL,
    const Sequence< PropertyValue >& Args )
throw ( css::container::NoSuchElementException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception )
{
    Reference< css::ui::XUIElement > xMenuBar(
            static_cast<OWeakObject *>(new MenuBarWrapper(m_xContext)), UNO_QUERY);
    CreateUIElement(ResourceURL, Args, "MenuOnly", "private:resource/menubar/", xMenuBar, m_xContext);
    return xMenuBar;
}

void MenuBarFactory::CreateUIElement(const OUString& ResourceURL
                                     ,const Sequence< PropertyValue >& Args
                                     ,const char* _pExtraMode
                                     ,const OUString& ResourceType
                                     ,const Reference< css::ui::XUIElement >& _xMenuBar
                                     ,const css::uno::Reference< css::uno::XComponentContext >& _rxContext)
{
    Reference< XUIConfigurationManager > xCfgMgr;
    Reference< XUIConfigurationManager > xConfigSource;
    Reference< XFrame >                  xFrame;
    OUString                        aResourceURL( ResourceURL );
    bool                             bPersistent( true );
    bool                             bExtraMode( false );

    for ( sal_Int32 n = 0; n < Args.getLength(); n++ )
    {
        if ( Args[n].Name == "ConfigurationSource" )
            Args[n].Value >>= xConfigSource;
        else if ( Args[n].Name == "Frame" )
            Args[n].Value >>= xFrame;
        else if ( Args[n].Name == "ResourceURL" )
            Args[n].Value >>= aResourceURL;
        else if ( Args[n].Name == "Persistent" )
            Args[n].Value >>= bPersistent;
        else if ( _pExtraMode && Args[n].Name.equalsAscii( _pExtraMode ))
            Args[n].Value >>= bExtraMode;
    }
    if (!aResourceURL.startsWith(ResourceType))
        throw IllegalArgumentException();

    // Identify frame and determine document based ui configuration manager/module ui configuration manager
    if ( xFrame.is() && !xConfigSource.is() )
    {
        bool bHasSettings( false );
        Reference< XModel > xModel;

        Reference< XController > xController = xFrame->getController();
        if ( xController.is() )
            xModel = xController->getModel();

        if ( xModel.is() )
        {
            Reference< XUIConfigurationManagerSupplier > xUIConfigurationManagerSupplier( xModel, UNO_QUERY );
            if ( xUIConfigurationManagerSupplier.is() )
            {
                xCfgMgr = xUIConfigurationManagerSupplier->getUIConfigurationManager();
                bHasSettings = xCfgMgr->hasSettings( aResourceURL );
            }
        }

        if ( !bHasSettings )
        {
            Reference< css::frame::XModuleManager2 > xModuleManager =
                ModuleManager::create( _rxContext );
            OUString aModuleIdentifier = xModuleManager->identify( Reference<XInterface>( xFrame, UNO_QUERY ) );
            if ( !aModuleIdentifier.isEmpty() )
            {
                Reference< XModuleUIConfigurationManagerSupplier > xModuleCfgSupplier =
                    theModuleUIConfigurationManagerSupplier::get( _rxContext );
                xCfgMgr = xModuleCfgSupplier->getUIConfigurationManager( aModuleIdentifier );
            }
        }
    }

    PropertyValue aPropValue;
    Sequence< Any > aPropSeq( _pExtraMode ? 5 : 4);
    aPropValue.Name = "Frame";
    aPropValue.Value <<= xFrame;
    aPropSeq[0] <<= aPropValue;
    aPropValue.Name = "ConfigurationSource";
    aPropValue.Value <<= xCfgMgr;
    aPropSeq[1] <<= aPropValue;
    aPropValue.Name = "ResourceURL";
    aPropValue.Value <<= aResourceURL;
    aPropSeq[2] <<= aPropValue;
    aPropValue.Name = "Persistent";
    aPropValue.Value <<= bPersistent;
    aPropSeq[3] <<= aPropValue;
    if ( _pExtraMode )
    {
        aPropValue.Name = OUString::createFromAscii(_pExtraMode);
        aPropValue.Value <<= bExtraMode;
        aPropSeq[4] <<= aPropValue;
    }

    SolarMutexGuard aGuard;
    Reference< XInitialization > xInit( _xMenuBar, UNO_QUERY );
    xInit->initialize( aPropSeq );
}

} // namespace framework

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_MenuBarFactory_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new framework::MenuBarFactory(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
