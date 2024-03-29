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

#include <uielement/toolbarmodemenucontroller.hxx>
#include <services.h>

#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>


#include <toolkit/awt/vclxmenu.hxx>
#include <officecfg/Office/Common.hxx>
#include <vcl/svapp.hxx>
#include <vcl/EnumContext.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/types.hxx>
#include <unotools/confignode.hxx>
#include <cppuhelper/supportsservice.hxx>

//  Defines

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;

namespace framework
{

// XInterface, XTypeProvider, XServiceInfo

OUString SAL_CALL ToolbarModeMenuController::getImplementationName()
{
    return "com.sun.star.comp.framework.ToolbarModeMenuController";
}

sal_Bool SAL_CALL ToolbarModeMenuController::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ToolbarModeMenuController::getSupportedServiceNames()
{
    return { SERVICENAME_POPUPMENUCONTROLLER };
}


ToolbarModeMenuController::ToolbarModeMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext ) :
    svt::PopupMenuControllerBase( xContext ),
    m_xContext( xContext )
{
}

ToolbarModeMenuController::~ToolbarModeMenuController()
{
}

void ToolbarModeMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu > const & rPopupMenu )
{
    if ( officecfg::Office::Common::Misc::DisableUICustomization::get() )
        return;

    SolarMutexGuard aSolarMutexGuard;
    resetPopupMenu( rPopupMenu );

    const Reference<XComponentContext> xContext (::comphelper::getProcessComponentContext() );
    const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( xContext );
    vcl::EnumContext::Application eApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(m_xFrame));

    OUStringBuffer aPath("org.openoffice.Office.UI.ToolbarMode/Applications/");
    switch ( eApp )
    {
        case vcl::EnumContext::Application::Writer:
            aPath.append("Writer");
            break;
        case vcl::EnumContext::Application::Calc:
            aPath.append("Calc");
            break;
        case vcl::EnumContext::Application::Impress:
            aPath.append("Impress");
            break;
        case vcl::EnumContext::Application::Draw:
            aPath.append("Draw");
            break;
        case vcl::EnumContext::Application::Formula:
            aPath.append("Formula");
            break;
        case vcl::EnumContext::Application::Base:
            aPath.append("Base");
            break;
        default:
            break;
    }
    aPath.append("/Modes");

    const utl::OConfigurationTreeRoot aModesNode(
                                        m_xContext,
                                        aPath.makeStringAndClear(),
                                        false);
    if ( !aModesNode.isValid() )
        return;

    const Sequence<OUString> aModeNodeNames (aModesNode.getNodeNames());
    const sal_Int32 nCount(aModeNodeNames.getLength());
    tools::Long nCountToolbar = 0;

    for ( sal_Int32 nReadIndex = 0; nReadIndex < nCount; ++nReadIndex )
    {
        const utl::OConfigurationNode aModeNode(aModesNode.openNode(aModeNodeNames[nReadIndex]));
        if ( !aModeNode.isValid() )
            continue;

        OUString aLabel = comphelper::getString( aModeNode.getNodeValue( "Label" ) );
        OUString aCommandArg = comphelper::getString( aModeNode.getNodeValue( "CommandArg" ) );
        tools::Long nPosition = comphelper::getINT32( aModeNode.getNodeValue( "MenuPosition" ) );
        bool isExperimental = comphelper::getBOOL( aModeNode.getNodeValue( "IsExperimental" ) );
        bool hasNotebookbar = comphelper::getBOOL( aModeNode.getNodeValue( "HasNotebookbar" ) );

        // Allow Notebookbar only in experimental mode
        if ( isExperimental && !officecfg::Office::Common::Misc::ExperimentalMode::get() )
            continue;
        if (!hasNotebookbar)
            nCountToolbar++;

        m_xPopupMenu->insertItem( nReadIndex+1, aLabel, css::awt::MenuItemStyle::RADIOCHECK, nPosition );
        rPopupMenu->setCommand( nReadIndex+1, aCommandArg );
    }
    rPopupMenu->insertSeparator(nCountToolbar);
}

// XEventListener
void SAL_CALL ToolbarModeMenuController::disposing( const EventObject& )
{
    Reference< css::awt::XMenuListener > xHolder(this);

    std::unique_lock aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(this) );
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL ToolbarModeMenuController::statusChanged( const FeatureStateEvent& Event )
{
    OUString aFeatureURL( Event.FeatureURL.Complete );

    // All other status events will be processed here
    std::unique_lock aLock( m_aMutex );
    Reference< css::awt::XPopupMenu > xPopupMenu( m_xPopupMenu );
    aLock.unlock();

    if ( !xPopupMenu.is() )
        return;

    SolarMutexGuard aGuard;

    bool bSetCheckmark      = false;
    bool bCheckmark         = false;
    for (sal_Int16 i = 0, nCount = xPopupMenu->getItemCount(); i < nCount; ++i)
    {
        sal_Int16 nId = xPopupMenu->getItemId(i);
        if ( nId == 0 )
            continue;

        OUString aCmd = xPopupMenu->getCommand(nId);
        if ( aCmd == aFeatureURL )
        {
            // Enable/disable item
            xPopupMenu->enableItem(nId, Event.IsEnabled);

            // Checkmark
            if ( Event.State >>= bCheckmark )
                bSetCheckmark = true;

            if ( bSetCheckmark )
                xPopupMenu->checkItem(nId, bCheckmark);
            else
            {
                OUString aItemText;

                if ( Event.State >>= aItemText )
                    xPopupMenu->setItemText(nId, aItemText);
            }
        }
    }
}

// XMenuListener
void SAL_CALL ToolbarModeMenuController::itemSelected( const css::awt::MenuEvent& rEvent )
{
    auto aArgs(comphelper::InitPropertySequence({{"Mode", Any(m_xPopupMenu->getCommand(rEvent.MenuId))}}));
    dispatchCommand(m_aCommandURL, aArgs);
}

void SAL_CALL ToolbarModeMenuController::itemActivated( const css::awt::MenuEvent& )
{
    const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( m_xContext );
    vcl::EnumContext::Application eApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(m_xFrame));

    OUStringBuffer aPath("org.openoffice.Office.UI.ToolbarMode/Applications/");
    switch ( eApp )
    {
        case vcl::EnumContext::Application::Writer:
            aPath.append("Writer");
            break;
        case vcl::EnumContext::Application::Calc:
            aPath.append("Calc");
            break;
        case vcl::EnumContext::Application::Impress:
            aPath.append("Impress");
            break;
        case vcl::EnumContext::Application::Draw:
            aPath.append("Draw");
            break;
        case vcl::EnumContext::Application::Formula:
            aPath.append("Formula");
            break;
        case vcl::EnumContext::Application::Base:
            aPath.append("Base");
            break;
        default:
            break;
    }

    const utl::OConfigurationTreeRoot aModesNode(
                                        m_xContext,
                                        aPath.makeStringAndClear(),
                                        false);
    if ( !aModesNode.isValid() )
        return;

    OUString aMode = comphelper::getString( aModesNode.getNodeValue( "Active" ) );

    for ( int i = 0; i < m_xPopupMenu->getItemCount(); ++i )
    {
        sal_Int16 nItemId(m_xPopupMenu->getItemId(i));
        m_xPopupMenu->checkItem(nItemId, aMode == m_xPopupMenu->getCommand(nItemId));
    }
}

// XPopupMenuController
void SAL_CALL ToolbarModeMenuController::setPopupMenu( const Reference< css::awt::XPopupMenu >& xPopupMenu )
{
    std::unique_lock aLock( m_aMutex );

    throwIfDisposed(aLock);

    if ( m_xFrame.is() && !m_xPopupMenu.is() )
    {
        // Create popup menu on demand
        SolarMutexGuard aSolarMutexGuard;

        m_xPopupMenu = dynamic_cast<VCLXPopupMenu*>(xPopupMenu.get());
        assert(bool(xPopupMenu) == bool(m_xPopupMenu) && "we only support VCLXPopupMenu");
        m_xPopupMenu->addMenuListener( Reference< css::awt::XMenuListener >(this) );
        fillPopupMenu( m_xPopupMenu );
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
framework_ToolbarModeMenuController_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new framework::ToolbarModeMenuController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
