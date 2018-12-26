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

#include <RelationDesignView.hxx>
#include <RelationTableView.hxx>
#include <RelationController.hxx>
#include <svl/undo.hxx>
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include <browserids.hxx>
#include <connectivity/dbtools.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include "RTableConnection.hxx"
#include <ConnectionLine.hxx>
#include <ConnectionLineData.hxx>
#include <RTableConnectionData.hxx>
#include <stringconstants.hxx>
#include <UITools.hxx>

using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

ORelationDesignView::ORelationDesignView(vcl::Window* _pParent, ORelationController& _rController,const Reference< XComponentContext >& _rxContext)
    :OJoinDesignView( _pParent, _rController, _rxContext )
{
}

void ORelationDesignView::Construct()
{
    m_pTableView    = VclPtr<ORelationTableView>::Create(m_pScrollWindow,this);
    OJoinDesignView::Construct();
}

void ORelationDesignView::initialize()
{
    m_pTableView->clearLayoutInformation();
    m_pTableView->ReSync();

    OJoinDesignView::initialize();
}

bool ORelationDesignView::PreNotify( NotifyEvent& rNEvt )
{
    bool bDone = false;
    if(rNEvt.GetType() == MouseNotifyEvent::GETFOCUS)
    {
        if(m_pTableView && !m_pTableView->HasChildPathFocus())
        {
            m_pTableView->GrabTabWinFocus();
            bDone = true;
        }
    }
    if(!bDone)
        bDone = OJoinDesignView::PreNotify(rNEvt);
    return bDone;
}

void ORelationDesignView::GetFocus()
{
    OJoinDesignView::GetFocus();
    if ( m_pTableView && m_pTableView->IsVisible() && !m_pTableView->GetTabWinMap().empty() )
        m_pTableView->GrabTabWinFocus();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
