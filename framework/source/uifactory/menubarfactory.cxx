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

#include <utility>
#include <vcl/svapp.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace ::com::sun::star::ui;

namespace framework
{

MenuBarFactory::MenuBarFactory( css::uno::Reference< css::uno::XComponentContext >  xContext )
    : m_xContext(std::move( xContext ))
{
}

MenuBarFactory::~MenuBarFactory()
{
}

// XUIElementFactory
Reference< XUIElement > SAL_CALL MenuBarFactory::createUIElement(
    const OUString& ResourceURL,
    const Sequence< PropertyValue >& Args )
{
    Reference< css::ui::XUIElement > xMenuBar = new MenuBarWrapper(m_xContext);
    CreateUIElement(ResourceURL, Args, u"private:resource/menubar/", xMenuBar, m_xContext);
    return xMenuBar;
}

void MenuBarFactory::CreateUIElement(const OUString& ResourceURL
                                     ,const Sequence< PropertyValue >& Args
                                     ,std::u16string_view ResourceType
                                     ,const Reference< css::ui::XUIElement >& _xMenuBar
                                     ,const css::uno::Reference< css::uno::XComponentContext >& _rxContext)
{
    sal_Int32 nConfigPropertyIndex( Args.getLength() );
    sal_Int32 nURLPropertyIndex( Args.getLength() );
    Reference< XUIConfigurationManager > xCfgMgr;
    Reference< XFrame >                  xFrame;
    OUString                        aResourceURL( ResourceURL );

    for ( sal_Int32 n = 0; n < Args.getLength(); n++ )
    {
        if ( Args[n].Name == "ConfigurationSource" )
        {
            nConfigPropertyIndex = n;
            Args[n].Value >>= xCfgMgr;
        }
        else if ( Args[n].Name == "ResourceURL" )
        {
            nURLPropertyIndex = n;
            Args[n].Value >>= aResourceURL;
        }
        else if ( Args[n].Name == "Frame" )
            Args[n].Value >>= xFrame;
    }
    if (!aResourceURL.startsWith(ResourceType))
        throw IllegalArgumentException();

    // Identify frame and determine document based ui configuration manager/module ui configuration manager
    if ( xFrame.is() && !xCfgMgr.is() )
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

    sal_Int32 nSeqLength( Args.getLength() );
    if ( Args.getLength() == nConfigPropertyIndex )
        nSeqLength++;
    if ( Args.getLength() == nURLPropertyIndex )
        nSeqLength++;
    if ( nConfigPropertyIndex == nURLPropertyIndex )
        nURLPropertyIndex++;

    Sequence< Any > aPropSeq( nSeqLength );
    auto aPropSeqRange = asNonConstRange(aPropSeq);
    for ( sal_Int32 n = 0; n < aPropSeq.getLength(); n++ )
    {
        PropertyValue aPropValue;
        if ( n == nURLPropertyIndex )
        {
            aPropValue.Name = "ResourceURL";
            aPropValue.Value <<= aResourceURL;
        }
        else if ( n == nConfigPropertyIndex )
        {
            aPropValue.Name = "ConfigurationSource";
            aPropValue.Value <<= xCfgMgr;
        }
        else
            aPropValue = Args[n];

        aPropSeqRange[n] <<= aPropValue;
    }

    SolarMutexGuard aGuard;
    Reference< XInitialization > xInit( _xMenuBar, UNO_QUERY );
    xInit->initialize( aPropSeq );
}

} // namespace framework

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_MenuBarFactory_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new framework::MenuBarFactory(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
