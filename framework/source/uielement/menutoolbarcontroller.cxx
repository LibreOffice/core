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

#include <uielement/menubarmanager.hxx>
#include <uielement/menutoolbarcontroller.hxx>

#include <vcl/toolbox.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

using namespace css::awt;
using namespace css::container;
using namespace css::frame;
using namespace css::uno;
using namespace css::util;

namespace framework {

MenuToolbarController::MenuToolbarController( const Reference< XComponentContext >& rxContext,
                                              const Reference< XFrame >& rFrame,
                                              ToolBox* pToolBar,
                                              sal_uInt16   nID,
                                              const OUString& aCommand,
                                              const OUString& aModuleIdentifier,
                                              const Reference< XIndexAccess >& xMenuDesc )
    : GenericToolbarController( rxContext, rFrame, pToolBar, nID, aCommand ),
      m_xMenuDesc( xMenuDesc ),
      pMenu( nullptr ),
      m_aModuleIdentifier( aModuleIdentifier )
{
}

MenuToolbarController::~MenuToolbarController()
{
    try
    {
        if ( m_xMenuManager.is() )
            m_xMenuManager->dispose();
    }
    catch( const Exception& ) {}
    if ( pMenu )
    {
        pMenu.disposeAndClear();
    }
}

class Toolbarmenu : public ::PopupMenu
{
    public:
    Toolbarmenu();
    virtual ~Toolbarmenu();
    virtual void dispose() override;
};

Toolbarmenu::Toolbarmenu()
{
    SAL_INFO("fwk.uielement", "constructing Toolbarmenu " << this);
}

Toolbarmenu::~Toolbarmenu()
{
    disposeOnce();
}

void Toolbarmenu::dispose()
{
    SAL_INFO("fwk.uielement", "destructing Toolbarmenu " << this);
    ::PopupMenu::dispose();
}

void SAL_CALL MenuToolbarController::click() throw (RuntimeException, std::exception)
{
    createPopupWindow();
}

Reference< XWindow > SAL_CALL
MenuToolbarController::createPopupWindow() throw (css::uno::RuntimeException, std::exception)
{
    if ( !pMenu )
    {
        Reference< XDispatchProvider > xDispatch;
        Reference< XURLTransformer > xURLTransformer = URLTransformer::create( m_xContext );
        pMenu = VclPtr<Toolbarmenu>::Create();
        m_xMenuManager.set( new MenuBarManager( m_xContext, m_xFrame, xURLTransformer, xDispatch, m_aModuleIdentifier, pMenu, true, true, false ) );
        if (m_xMenuManager.is())
        {
            MenuBarManager& rMgr = dynamic_cast<MenuBarManager&>(*m_xMenuManager.get());
            rMgr.SetItemContainer(m_xMenuDesc);
        }
    }

    if ( !pMenu || !m_pToolbar )
        return nullptr;

    OSL_ENSURE ( pMenu->GetItemCount(), "Empty PopupMenu!" );

    ::Rectangle aRect( m_pToolbar->GetItemRect( m_nID ) );
    pMenu->Execute( m_pToolbar, aRect, PopupMenuFlags::ExecuteDown );

    return nullptr;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
