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

#include <vcl/builderfactory.hxx>
#include <vcl/layout.hxx>
#include <vcl/tabpage.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/notebookbar/NotebookbarTabControl.hxx>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <sfx2/notebookbar/SfxNotebookBar.hxx>
#include <com/sun/star/uno/Reference.h>
#include <toolkit/awt/vclxmenu.hxx>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <sfx2/sidebar/SidebarToolBox.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/implbase.hxx>

#define ICON_SIZE 25
#define TOOLBAR_STR "private:resource/toolbar/notebookbarshortcuts"

using namespace css::uno;
using namespace css::ui;
using namespace css::frame;

class ChangedUIEventListener : public ::cppu::WeakImplHelper<XUIConfigurationListener>
{
    VclPtr<NotebookbarTabControl> m_pParent;

public:
    explicit ChangedUIEventListener(NotebookbarTabControl *p)
    : m_pParent(p)
    {
        try
        {
            if( SfxViewFrame::Current() )
            {
                Reference<XComponentContext> xContext = comphelper::getProcessComponentContext();
                const Reference<XModuleManager> xModuleManager  = ModuleManager::create( xContext );
                Reference<XFrame> xFrame = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
                OUString aModuleName = xModuleManager->identify( xFrame );

                Reference<XUIConfigurationManager> m_xConfigManager;
                Reference<XModuleUIConfigurationManagerSupplier > xModuleCfgMgrSupplier(
                    theModuleUIConfigurationManagerSupplier::get( xContext ) );
                m_xConfigManager.set( xModuleCfgMgrSupplier->getUIConfigurationManager( aModuleName ) );
                css::uno::Reference< css::ui::XUIConfiguration > xConfig( m_xConfigManager, css::uno::UNO_QUERY_THROW );
                xConfig->addConfigurationListener( this );
            }
        }
        catch( const css::uno::RuntimeException& ) {}
    }

    // XUIConfigurationListener
    virtual void SAL_CALL elementInserted( const ConfigurationEvent& rEvent ) override
    {
        if( rEvent.ResourceURL == TOOLBAR_STR )
        {
            m_pParent->m_bInvalidate = true;
            m_pParent->StateChanged(StateChangedType::UpdateMode);
        }
    }

    virtual void SAL_CALL elementRemoved( const ConfigurationEvent& rEvent ) override
    {
        elementInserted( rEvent );
    }

    virtual void SAL_CALL elementReplaced( const ConfigurationEvent& rEvent ) override
    {
        elementInserted( rEvent );
    }

    virtual void SAL_CALL disposing(const ::css::lang::EventObject&) override
    {
        try
        {
            if( SfxViewFrame::Current() )
            {
                Reference<XComponentContext> xContext = comphelper::getProcessComponentContext();
                const Reference<XModuleManager> xModuleManager  = ModuleManager::create( xContext );
                Reference<XFrame> xFrame = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
                OUString aModuleName = xModuleManager->identify( xFrame );

                Reference<XUIConfigurationManager> m_xConfigManager;
                Reference<XModuleUIConfigurationManagerSupplier > xModuleCfgMgrSupplier(
                    theModuleUIConfigurationManagerSupplier::get( xContext ) );
                m_xConfigManager.set( xModuleCfgMgrSupplier->getUIConfigurationManager( aModuleName ) );
                css::uno::Reference< css::ui::XUIConfiguration > xConfig( m_xConfigManager, css::uno::UNO_QUERY_THROW );
                xConfig->removeConfigurationListener( this );
            }
        }
        catch( const css::uno::RuntimeException& ) {}

        m_pParent.clear();
    }
};

class ShortcutsToolBox : public sfx2::sidebar::SidebarToolBox
{
public:
    ShortcutsToolBox( Window* pParent )
    : sfx2::sidebar::SidebarToolBox( pParent )
    {
        mbUseDefaultButtonSize = false;
        SetToolboxButtonSize(ToolBoxButtonSize::Small);
    }

    virtual void KeyInput( const KeyEvent& rKEvt ) override
    {
        if ( rKEvt.GetKeyCode().IsMod1() )
        {
            sal_uInt16 nCode( rKEvt.GetKeyCode().GetCode() );
            if ( nCode == KEY_RIGHT || nCode == KEY_LEFT )
            {
                GetParent()->KeyInput( rKEvt );
                return;
            }
        }
        return sfx2::sidebar::SidebarToolBox::KeyInput( rKEvt );
    }
};

NotebookbarTabControl::NotebookbarTabControl( Window* pParent )
: NotebookbarTabControlBase( pParent )
, m_bInitialized( false )
, m_bInvalidate( true )
{
}

NotebookbarTabControl::~NotebookbarTabControl()
{
}

void NotebookbarTabControl::ArrowStops( sal_uInt16 nCode )
{
    ToolBox* pToolBox( GetToolBox() );
    PushButton* pOpenMenu( GetOpenMenu() );

    if ( nCode == KEY_LEFT )
    {
        if ( HasFocus() )
        {
            if ( pToolBox )
                pToolBox->GrabFocus();
            else if ( pOpenMenu )
                pOpenMenu->GrabFocus();
        }
        else if ( pToolBox && pToolBox->HasFocus() )
        {
            if ( pOpenMenu )
                pOpenMenu->GrabFocus();
            else
                GrabFocus();
        }
        else if ( pOpenMenu && pOpenMenu->HasFocus() )
        {
            GrabFocus();
        }
    }
    else if ( nCode == KEY_RIGHT )
    {
        if ( HasFocus() )
        {
            if ( pOpenMenu )
                pOpenMenu->GrabFocus();
            else if ( pToolBox )
                pToolBox->GrabFocus();
        }
        else if ( pToolBox && pToolBox->HasFocus() )
        {
            GrabFocus();
        }
        else if ( pOpenMenu && pOpenMenu->HasFocus() )
        {
            if ( pToolBox )
                pToolBox->GrabFocus();
            else
                GrabFocus();
        }
    }
}

void NotebookbarTabControl::KeyInput( const KeyEvent& rKEvt )
{
    if ( rKEvt.GetKeyCode().IsMod1() )
    {
        sal_uInt16 nCode( rKEvt.GetKeyCode().GetCode() );
        if ( nCode == KEY_RIGHT || nCode == KEY_LEFT )
        {
            ArrowStops( nCode );
            return;
        }
    }
    return NotebookbarTabControlBase::KeyInput( rKEvt );
}

bool NotebookbarTabControl::EventNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const vcl::KeyCode& rKey = rNEvt.GetKeyEvent()->GetKeyCode();
        sal_uInt16 nCode = rKey.GetCode();
        if ( rKey.IsMod1() && ( nCode == KEY_RIGHT || nCode == KEY_LEFT ) )
        {
            ArrowStops( nCode );
            return true;
        }
    }
    return NotebookbarTabControlBase::EventNotify( rNEvt );
}

void NotebookbarTabControl::StateChanged(StateChangedType nStateChange)
{
    if( !m_bInitialized && SfxViewFrame::Current() )
    {
        VclPtr<ShortcutsToolBox> pShortcuts = VclPtr<ShortcutsToolBox>::Create( this );
        pShortcuts->Show();

        SetToolBox( static_cast<ToolBox*>( pShortcuts.get() ) );
        SetIconClickHdl( LINK( this, NotebookbarTabControl, OpenNotebookbarPopupMenu ) );

        m_pListener = new ChangedUIEventListener( this );

        m_bInitialized = true;
    }
    if( m_bInitialized && m_bInvalidate && SfxViewFrame::Current() )
    {
        ToolBox* pToolBox = GetToolBox();
        if( !pToolBox )
            return;

        pToolBox->Clear();

        Reference<XComponentContext> xContext = comphelper::getProcessComponentContext();
        const Reference<XModuleManager> xModuleManager  = ModuleManager::create( xContext );
        m_xFrame = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
        OUString aModuleName = xModuleManager->identify( m_xFrame );

        FillShortcutsToolBox( xContext, m_xFrame, aModuleName, pToolBox );

        Size aSize( pToolBox->GetOptimalSize() );
        Point aPos( ICON_SIZE + 10, 0 );
        pToolBox->SetPosSizePixel( aPos, aSize );
        ImplPlaceTabs( GetSizePixel().getWidth() );

        m_bInvalidate = false;
    }
    NotebookbarTabControlBase::StateChanged( nStateChange );
}

void NotebookbarTabControl::FillShortcutsToolBox(Reference<XComponentContext> const & xContext,
                                          const Reference<XFrame>& xFrame,
                                          const OUString& aModuleName,
                                          ToolBox* pShortcuts
)
{
    Reference<::com::sun::star::container::XIndexAccess> xIndex;

    try
    {
        Reference<XUIConfigurationManager> m_xConfigManager;
        Reference<XModuleUIConfigurationManagerSupplier > xModuleCfgMgrSupplier(
            theModuleUIConfigurationManagerSupplier::get( xContext ) );
        m_xConfigManager.set( xModuleCfgMgrSupplier->getUIConfigurationManager( aModuleName ) );
        xIndex = m_xConfigManager->getSettings( TOOLBAR_STR, false );
    }
    catch( const Exception& ) {}

    if ( !xIndex.is() )
        return;

    Sequence< css::beans::PropertyValue > aPropSequence;
    for ( sal_Int32 i = 0; i < xIndex->getCount(); ++i )
    {
        try
        {
            if ( xIndex->getByIndex( i ) >>= aPropSequence )
            {
                OUString aCommandURL;
                sal_uInt16 nType = ItemType::DEFAULT;
                bool bVisible = true;

                for ( const auto& aProp: aPropSequence )
                {
                    if ( aProp.Name == "CommandURL" )
                        aProp.Value >>= aCommandURL;
                    else if ( aProp.Name == "Type" )
                        aProp.Value >>= nType;
                    else if ( aProp.Name == "IsVisible" )
                        aProp.Value >>= bVisible;
                }
                if ( bVisible && ( nType == ItemType::DEFAULT ) )
                    pShortcuts->InsertItem( aCommandURL, xFrame, ToolBoxItemBits::ICON_ONLY, Size( ICON_SIZE, ICON_SIZE ) );
            }
        }
        catch ( const Exception& )
        {
            break;
        }
    }
}

IMPL_LINK(NotebookbarTabControl, OpenNotebookbarPopupMenu, NotebookBar*, pNotebookbar, void)
{
    if (!pNotebookbar || !m_xFrame.is())
        return;

    Sequence<Any> aArgs {
        makeAny(comphelper::makePropertyValue("Value", OUString("notebookbar"))),
        makeAny(comphelper::makePropertyValue("Frame", m_xFrame)) };

    Reference<XComponentContext> xContext = comphelper::getProcessComponentContext();
    Reference<XPopupMenuController> xPopupController(
        xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
        "com.sun.star.comp.framework.ResourceMenuController", aArgs, xContext), UNO_QUERY);

    Reference<css::awt::XPopupMenu> xPopupMenu(xContext->getServiceManager()->createInstanceWithContext(
        "com.sun.star.awt.PopupMenu", xContext), UNO_QUERY);

    if (!xPopupController.is() || !xPopupMenu.is())
        return;

    xPopupController->setPopupMenu(xPopupMenu);
    VCLXMenu* pAwtMenu = VCLXMenu::GetImplementation(xPopupMenu);
    PopupMenu* pVCLMenu = static_cast<PopupMenu*>(pAwtMenu->GetMenu());
    Point aPos(pNotebookbar->GetSizePixel().getWidth(), NotebookbarTabControl::GetHeaderHeight() - ICON_SIZE + 10);
    pVCLMenu->Execute(pNotebookbar, tools::Rectangle(aPos, aPos),PopupMenuFlags::ExecuteDown|PopupMenuFlags::NoMouseUpClose);

    Reference<css::lang::XComponent> xComponent(xPopupController, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
}

Size NotebookbarTabControl::calculateRequisition() const
{
    Size aSize = NotebookbarTabControlBase::calculateRequisition();

    for (int i = 0; i < GetPageCount(); i++)
    {
        vcl::Window* pChild = static_cast<vcl::Window*>(GetTabPage(TabControl::GetPageId(i)));

        if (pChild)
        {
            Size aChildSize = VclAlignment::getLayoutRequisition(*pChild);

            if (aChildSize.getWidth() < aSize.getWidth())
                aSize.setWidth( aChildSize.Width() );
        }
    }

    if (aSize.Width() < 400)
        aSize.setWidth( 400 );

    return aSize;
}

VCL_BUILDER_FACTORY( NotebookbarTabControl )

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
