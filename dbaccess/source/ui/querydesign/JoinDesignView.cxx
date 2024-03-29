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

#include <JoinDesignView.hxx>
#include <JoinTableView.hxx>
#include <JoinController.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

using namespace ::com::sun::star::uno;

namespace dbaui
{

// OJoinDesignView
OJoinDesignView::OJoinDesignView(vcl::Window* _pParent, OJoinController& _rController,const Reference< XComponentContext >& _rxContext)
    :ODataView( _pParent, _rController, _rxContext )
    ,m_pTableView(nullptr)
    ,m_rController( _rController )
{
    m_pScrollWindow = VclPtr<OScrollWindowHelper>::Create(this);
}

OJoinDesignView::~OJoinDesignView()
{
    disposeOnce();
}

void OJoinDesignView::dispose()
{
    m_pTableView.disposeAndClear();
    m_pScrollWindow.disposeAndClear();
    ODataView::dispose();
}

void OJoinDesignView::Construct()
{
    m_pScrollWindow->setTableView(m_pTableView);
    m_pScrollWindow->Show();
    m_pTableView->Show();

    SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetFaceColor()) );

    ODataView::Construct();
}

void OJoinDesignView::initialize()
{
}

void OJoinDesignView::resizeDocumentView(tools::Rectangle& _rPlayground)
{
    m_pScrollWindow->SetPosSizePixel( _rPlayground.TopLeft(), _rPlayground.GetSize() );

    // just for completeness: there is no space left, we occupied it all ...
    _rPlayground.SetPos( _rPlayground.BottomRight() );
    _rPlayground.SetSize( Size( 0, 0 ) );
}

void OJoinDesignView::setReadOnly(bool /*_bReadOnly*/)
{
}

void OJoinDesignView::SaveTabWinUIConfig(OTableWindow const * pWin)
{
    OJoinController::SaveTabWinPosSize(pWin, m_pScrollWindow->GetHScrollBar().GetThumbPos(), m_pScrollWindow->GetVScrollBar().GetThumbPos());
}

void OJoinDesignView::KeyInput( const KeyEvent& rEvt )
{
    if ( m_pTableView && m_pTableView->IsVisible() )
        m_pTableView->KeyInput( rEvt );
    else
        ODataView::KeyInput(rEvt);
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
