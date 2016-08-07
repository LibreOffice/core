/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbui.hxx"
#ifndef DBAUI_JOINDESIGNVIEW_HXX
#include "JoinDesignView.hxx"
#endif
#ifndef DBAUI_JOINTABLEVIEW_HXX
#include "JoinTableView.hxx"
#endif
#ifndef DBAUI_JOINCONTROLLER_HXX
#include "JoinController.hxx"
#endif
#ifndef _UNDO_HXX
#include <svl/undo.hxx>
#endif
#ifndef DBAUI_QYDLGTAB_HXX
#include "adtabdlg.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef _DBU_QRY_HRC_
#include "dbu_qry.hrc"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef DBAUI_TABLECONNECTION_HXX
#include "TableConnection.hxx"
#endif
#ifndef DBAUI_CONNECTIONLINE_HXX
#include "ConnectionLine.hxx"
#endif
#ifndef DBAUI_CONNECTIONLINEDATA_HXX
#include "ConnectionLineData.hxx"
#endif
#ifndef DBAUI_TABLECONNECTIONDATA_HXX
#include "TableConnectionData.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef DBAUI_JOINTABLEVIEW_HXX
#include "JoinTableView.hxx"
#endif
//  #include <com/sun/star/util/URL.hdl>

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
    //  getAddTableDialog()->Update();
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

