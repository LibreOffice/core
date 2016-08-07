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







