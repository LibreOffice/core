/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RelationDesignView.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:31:20 $
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
#ifndef DBAUI_RELATIONDESIGNVIEW_HXX
#include "RelationDesignView.hxx"
#endif
#ifndef DBAUI_RELATION_TABLEVIEW_HXX
#include "RelationTableView.hxx"
#endif
#ifndef DBAUI_RELATIONCONTROLLER_HXX
#include "RelationController.hxx"
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
#ifndef _DBU_REL_HRC_
#include "dbu_rel.hrc"
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
#ifndef DBAUI_RTABLECONNECTION_HXX
#include "RTableConnection.hxx"
#endif
#ifndef DBAUI_CONNECTIONLINE_HXX
#include "ConnectionLine.hxx"
#endif
#ifndef DBAUI_CONNECTIONLINEDATA_HXX
#include "ConnectionLineData.hxx"
#endif
#ifndef DBAUI_RTABLECONNECTIONDATA_HXX
#include "RTableConnectionData.hxx"
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
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

DBG_NAME(ORelationDesignView)
ORelationDesignView::ORelationDesignView(Window* _pParent, ORelationController* _pController,const Reference< XMultiServiceFactory >& _rFactory)
    :OJoinDesignView(_pParent,_pController,_rFactory)
{
    DBG_CTOR(ORelationDesignView,NULL);
}
// -----------------------------------------------------------------------------
ORelationDesignView::~ORelationDesignView()
{
    DBG_DTOR(ORelationDesignView,NULL);
}
// -------------------------------------------------------------------------
void ORelationDesignView::Construct()
{
    m_pTableView    = new ORelationTableView(m_pScrollWindow,this);
    OJoinDesignView::Construct();
}
// -----------------------------------------------------------------------------
void ORelationDesignView::initialize()
{
    m_pTableView->clearLayoutInformation();
    m_pTableView->ReSync();

    OJoinDesignView::initialize();
}
// -----------------------------------------------------------------------------
long ORelationDesignView::PreNotify( NotifyEvent& rNEvt )
{
    long nDone = 0L;
    if(rNEvt.GetType() == EVENT_GETFOCUS)
    {
        if(!m_pTableView->HasChildPathFocus())
        {
            m_pTableView->GrabTabWinFocus();
            nDone = 1L;
        }
    }
    if(!nDone)
        nDone = OJoinDesignView::PreNotify(rNEvt);
    return nDone;
}
// -----------------------------------------------------------------------------
void ORelationDesignView::GetFocus()
{
    OJoinDesignView::GetFocus();
    if ( m_pTableView && m_pTableView->IsVisible() && !m_pTableView->GetTabWinMap()->empty() )
        m_pTableView->GrabTabWinFocus();
}
// -----------------------------------------------------------------------------







