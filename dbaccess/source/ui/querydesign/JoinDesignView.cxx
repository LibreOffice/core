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

#include "JoinDesignView.hxx"
#include "JoinTableView.hxx"
#include "JoinController.hxx"
#include <svl/undo.hxx>
#include "adtabdlg.hxx"
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include "browserids.hxx"
#include "dbu_qry.hrc"
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include "TableConnection.hxx"
#include "ConnectionLine.hxx"
#include "ConnectionLineData.hxx"
#include "TableConnectionData.hxx"
#include "dbustrings.hrc"
#include <comphelper/extract.hxx>
#include "UITools.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;

namespace dbaui
{

// OJoinDesignView
OJoinDesignView::OJoinDesignView(Window* _pParent, OJoinController& _rController,const Reference< XComponentContext >& _rxContext)
    :ODataView( _pParent, _rController, _rxContext )
    ,m_pTableView(NULL)
    ,m_rController( _rController )
{
    m_pScrollWindow = new OScrollWindowHelper(this);
}

OJoinDesignView::~OJoinDesignView()
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<Window> aT3(m_pScrollWindow);
    SAL_WNODEPRECATED_DECLARATIONS_POP
    m_pScrollWindow = NULL;
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<Window> aT2(m_pTableView);
    SAL_WNODEPRECATED_DECLARATIONS_POP
    m_pTableView = NULL;
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

void OJoinDesignView::resizeDocumentView(Rectangle& _rPlayground)
{
    m_pScrollWindow->SetPosSizePixel( _rPlayground.TopLeft(), _rPlayground.GetSize() );

    // just for completeness: there is no space left, we occupied it all ...
    _rPlayground.SetPos( _rPlayground.BottomRight() );
    _rPlayground.SetSize( Size( 0, 0 ) );
}

void OJoinDesignView::setReadOnly(sal_Bool /*_bReadOnly*/)
{
}

void OJoinDesignView::SaveTabWinUIConfig(OTableWindow* pWin)
{
    getController().SaveTabWinPosSize(pWin, m_pScrollWindow->GetHScrollBar()->GetThumbPos(), m_pScrollWindow->GetVScrollBar()->GetThumbPos());
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
