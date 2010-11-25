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
#include "precompiled_dbaccess.hxx"
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
#include "JoinTableView.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;

namespace dbaui
{

// =============================================================================
// = OJoinDesignView
// =============================================================================
// -----------------------------------------------------------------------------
OJoinDesignView::OJoinDesignView(Window* _pParent, OJoinController& _rController,const Reference< XMultiServiceFactory >& _rFactory)
    :ODataView( _pParent, _rController, _rFactory )
    ,m_pTableView(NULL)
    ,m_rController( _rController )
{
    m_pScrollWindow = new OScrollWindowHelper(this);
}
// -----------------------------------------------------------------------------
OJoinDesignView::~OJoinDesignView()
{
    ::std::auto_ptr<Window> aT3(m_pScrollWindow);
    m_pScrollWindow = NULL;
    ::std::auto_ptr<Window> aT2(m_pTableView);
    m_pTableView = NULL;
}
// -------------------------------------------------------------------------
void OJoinDesignView::Construct()
{
    m_pScrollWindow->setTableView(m_pTableView);
    m_pScrollWindow->Show();
    m_pTableView->Show();

    SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetFaceColor()) );

    ODataView::Construct();
}
// -----------------------------------------------------------------------------
void OJoinDesignView::initialize()
{
}
// -------------------------------------------------------------------------
void OJoinDesignView::resizeDocumentView(Rectangle& _rPlayground)
{
    m_pScrollWindow->SetPosSizePixel( _rPlayground.TopLeft(), _rPlayground.GetSize() );

    // just for completeness: there is no space left, we occupied it all ...
    _rPlayground.SetPos( _rPlayground.BottomRight() );
    _rPlayground.SetSize( Size( 0, 0 ) );
}
// -----------------------------------------------------------------------------
void OJoinDesignView::setReadOnly(sal_Bool /*_bReadOnly*/)
{
}
// -----------------------------------------------------------------------------
void OJoinDesignView::SaveTabWinUIConfig(OTableWindow* pWin)
{
    getController().SaveTabWinPosSize(pWin, m_pScrollWindow->GetHScrollBar()->GetThumbPos(), m_pScrollWindow->GetVScrollBar()->GetThumbPos());
}
// -----------------------------------------------------------------------------
void OJoinDesignView::KeyInput( const KeyEvent& rEvt )
{
    if ( m_pTableView && m_pTableView->IsVisible() )
        m_pTableView->KeyInput( rEvt );
    else
        ODataView::KeyInput(rEvt);
}
// -----------------------------------------------------------------------------

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
