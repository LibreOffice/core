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
ORelationDesignView::ORelationDesignView(Window* _pParent, ORelationController& _rController,const Reference< XComponentContext >& _rxContext)
    :OJoinDesignView( _pParent, _rController, _rxContext )
{
    DBG_CTOR(ORelationDesignView,NULL);
}

ORelationDesignView::~ORelationDesignView()
{
    DBG_DTOR(ORelationDesignView,NULL);
}

void ORelationDesignView::Construct()
{
    m_pTableView    = new ORelationTableView(m_pScrollWindow,this);
    OJoinDesignView::Construct();
}

void ORelationDesignView::initialize()
{
    m_pTableView->clearLayoutInformation();
    m_pTableView->ReSync();

    OJoinDesignView::initialize();
}

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

void ORelationDesignView::GetFocus()
{
    OJoinDesignView::GetFocus();
    if ( m_pTableView && m_pTableView->IsVisible() && !m_pTableView->GetTabWinMap()->empty() )
        m_pTableView->GrabTabWinFocus();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
