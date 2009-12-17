/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: RelationDesignView.cxx,v $
 * $Revision: 1.16 $
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
#include <svl/undo.hxx>
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
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

DBG_NAME(ORelationDesignView)
ORelationDesignView::ORelationDesignView(Window* _pParent, ORelationController& _rController,const Reference< XMultiServiceFactory >& _rFactory)
    :OJoinDesignView( _pParent, _rController, _rFactory )
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







