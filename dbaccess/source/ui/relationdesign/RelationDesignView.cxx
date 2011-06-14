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
#include "RelationDesignView.hxx"
#include "RelationTableView.hxx"
#include "RelationController.hxx"
#include <svl/undo.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include "browserids.hxx"
#include "dbu_rel.hrc"
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include "RTableConnection.hxx"
#include "ConnectionLine.hxx"
#include "ConnectionLineData.hxx"
#include "RTableConnectionData.hxx"
#include "dbustrings.hrc"
#include <comphelper/extract.hxx>
#include "UITools.hxx"
#include <tools/debug.hxx>

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







/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
