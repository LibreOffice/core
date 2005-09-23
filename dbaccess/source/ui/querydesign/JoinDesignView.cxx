/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: JoinDesignView.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 12:42:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
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
#include <svtools/undo.hxx>
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

using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
DBG_NAME(OJoinDesignView)

OJoinDesignView::OJoinDesignView(Window* _pParent, OJoinController* _pController,const Reference< XMultiServiceFactory >& _rFactory)
    :ODataView(_pParent,_pController,_rFactory)
    ,m_pController(_pController)
    ,m_pTableView(NULL)
    ,m_pAddTabDlg(NULL)
{
    DBG_CTOR(OJoinDesignView,NULL);

    m_pScrollWindow = new OScrollWindowHelper(this);
}
// -----------------------------------------------------------------------------
OJoinDesignView::~OJoinDesignView()
{
    if ( m_pController )
        m_pController->clearAddTableDialog();
    ::std::auto_ptr<Window> aT3(m_pScrollWindow);
    m_pScrollWindow = NULL;
    ::std::auto_ptr<Window> aT2(m_pTableView);
    m_pTableView = NULL;
    ::std::auto_ptr<Window> aT1(m_pAddTabDlg);
    m_pAddTabDlg = NULL;

    DBG_DTOR(OJoinDesignView,NULL);
}
// -------------------------------------------------------------------------
void OJoinDesignView::Construct()
{
    m_pScrollWindow->setTableView(m_pTableView);
    m_pScrollWindow->Show();
    m_pTableView->Show();

    m_pAddTabDlg = new OAddTableDlg(m_pScrollWindow,m_pTableView);
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
void OJoinDesignView::setReadOnly(sal_Bool _bReadOnly)
{
}
// -----------------------------------------------------------------------------
void OJoinDesignView::TableDeleted(const ::rtl::OUString& rAliasName)
{
}
// -----------------------------------------------------------------------------
void OJoinDesignView::zoomTableView(const Fraction& _rFraction)
{
    m_pTableView->SetZoom(_rFraction);
}
// -----------------------------------------------------------------------------
void OJoinDesignView::SaveUIConfig()
{
    OJoinController* pCtrl = getController();
    if (pCtrl)
        pCtrl->SaveTabWinsPosSize( m_pTableView->GetTabWinMap(), m_pScrollWindow->GetHScrollBar()->GetThumbPos(), m_pScrollWindow->GetVScrollBar()->GetThumbPos() );
}
// -----------------------------------------------------------------------------
void OJoinDesignView::SaveTabWinUIConfig(OTableWindow* pWin)
{
    getController()->SaveTabWinPosSize(pWin, m_pScrollWindow->GetHScrollBar()->GetThumbPos(), m_pScrollWindow->GetVScrollBar()->GetThumbPos());
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
