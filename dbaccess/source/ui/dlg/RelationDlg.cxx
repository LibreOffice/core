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

#include <RelationDlg.hxx>

#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>

#include <comphelper/diagnose_ex.hxx>
#include <JoinDesignView.hxx>
#include <JoinController.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <RTableConnectionData.hxx>
#include <RelationControl.hxx>
#include <cppuhelper/exc_hlp.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::dbaui;
using namespace ::dbtools;

ORelationDialog::ORelationDialog( OJoinTableView* pParent,
                                 const TTableConnectionData::value_type& pConnectionData,
                                 bool bAllowTableSelect )
    : GenericDialogController(pParent->GetFrameWeld(),
        "dbaccess/ui/relationdialog.ui", "RelationDialog")
    , m_pParent(pParent)
    , m_pOrigConnData(pConnectionData)
    , m_bTriedOneUpdate(false)
    , m_xRB_NoCascUpd(m_xBuilder->weld_radio_button("addaction"))
    , m_xRB_CascUpd(m_xBuilder->weld_radio_button("addcascade"))
    , m_xRB_CascUpdNull(m_xBuilder->weld_radio_button("addnull"))
    , m_xRB_CascUpdDefault(m_xBuilder->weld_radio_button("adddefault"))
    , m_xRB_NoCascDel(m_xBuilder->weld_radio_button("delaction"))
    , m_xRB_CascDel(m_xBuilder->weld_radio_button("delcascade"))
    , m_xRB_CascDelNull(m_xBuilder->weld_radio_button("delnull"))
    , m_xRB_CascDelDefault(m_xBuilder->weld_radio_button("deldefault"))
    , m_xPB_OK(m_xBuilder->weld_button("ok"))
{
    // Copy connection
    m_pConnData = pConnectionData->NewInstance();
    m_pConnData->CopyFrom( *pConnectionData );

    Init(m_pConnData);
    m_xTableControl.reset(new OTableListBoxControl(m_xBuilder.get(), &pParent->GetTabWinMap(), this));

    m_xPB_OK->connect_clicked(LINK(this, ORelationDialog, OKClickHdl));

    m_xTableControl->Init( m_pConnData );
    if ( bAllowTableSelect )
        m_xTableControl->fillListBoxes();
    else
        m_xTableControl->fillAndDisable(pConnectionData);

    m_xTableControl->lateInit();

    m_xTableControl->NotifyCellChange();
}

ORelationDialog::~ORelationDialog()
{
}

void ORelationDialog::Init(const TTableConnectionData::value_type& _pConnectionData)
{
    ORelationTableConnectionData* pConnData = static_cast<ORelationTableConnectionData*>(_pConnectionData.get());
    // Update Rules
    switch (pConnData->GetUpdateRules())
    {
    case KeyRule::NO_ACTION:
    case KeyRule::RESTRICT:
        m_xRB_NoCascUpd->set_active(true);
        break;

    case KeyRule::CASCADE:
        m_xRB_CascUpd->set_active(true);
        break;

    case KeyRule::SET_NULL:
        m_xRB_CascUpdNull->set_active(true);
        break;
    case KeyRule::SET_DEFAULT:
        m_xRB_CascUpdDefault->set_active(true);
        break;
    }

    // Delete Rules
    switch (pConnData->GetDeleteRules())
    {
    case KeyRule::NO_ACTION:
    case KeyRule::RESTRICT:
        m_xRB_NoCascDel->set_active(true);
        break;

    case KeyRule::CASCADE:
        m_xRB_CascDel->set_active(true);
        break;

    case KeyRule::SET_NULL:
        m_xRB_CascDelNull->set_active(true);
        break;
    case KeyRule::SET_DEFAULT:
        m_xRB_CascDelDefault->set_active(true);
        break;
    }
}

IMPL_LINK_NOARG(ORelationDialog, OKClickHdl, weld::Button&, void)
{
    // Read out RadioButtons
    sal_uInt16 nAttrib = 0;

    // Delete Rules
    if( m_xRB_NoCascDel->get_active() )
        nAttrib |= KeyRule::NO_ACTION;
    if( m_xRB_CascDel->get_active() )
        nAttrib |= KeyRule::CASCADE;
    if( m_xRB_CascDelNull->get_active() )
        nAttrib |= KeyRule::SET_NULL;
    if( m_xRB_CascDelDefault->get_active() )
        nAttrib |= KeyRule::SET_DEFAULT;

    ORelationTableConnectionData* pConnData = static_cast<ORelationTableConnectionData*>(m_pConnData.get());
    pConnData->SetDeleteRules( nAttrib );

    // Update Rules
    nAttrib = 0;
    if( m_xRB_NoCascUpd->get_active() )
        nAttrib |= KeyRule::NO_ACTION;
    if( m_xRB_CascUpd->get_active() )
        nAttrib |= KeyRule::CASCADE;
    if( m_xRB_CascUpdNull->get_active() )
        nAttrib |= KeyRule::SET_NULL;
    if( m_xRB_CascUpdDefault->get_active() )
        nAttrib |= KeyRule::SET_DEFAULT;
    pConnData->SetUpdateRules( nAttrib );

    m_xTableControl->SaveModified();

    //// if the ComboBoxes for the table selection are enabled (constructor with bAllowTableSelect==sal_True),
    //// then I must also put the table names into the connection
    //m_pConnData->SetSourceWinName(m_xTableControl->getSourceWinName());
    //m_pConnData->SetDestWinName(m_xTableControl->getDestWinName());

    // try to create the relation
    try
    {
        ORelationTableConnectionData* pOrigConnData = static_cast<ORelationTableConnectionData*>(m_pOrigConnData.get());
        if ( *pConnData == *pOrigConnData || pConnData->Update())
        {
            m_pOrigConnData->CopyFrom( *m_pConnData );
            m_xDialog->response(RET_OK);
            return;
        }
    }
    catch( const SQLException& )
    {
        ::dbtools::showError(SQLExceptionInfo(::cppu::getCaughtException()),
                             m_xDialog->GetXWindow(),
                             m_pParent->getDesignView()->getController().getORB());
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }

    m_bTriedOneUpdate = true;
    // this means that the original connection may be lost (if m_pConnData was not a newly created but an
    // existent conn to be modified), which we reflect by returning RET_NO (see ::Execute)

    // try again
    Init(m_pConnData);
    m_xTableControl->Init( m_pConnData );
    m_xTableControl->lateInit();
}

short ORelationDialog::run()
{
    short nResult = GenericDialogController::run();
    if ((nResult != RET_OK) && m_bTriedOneUpdate)
        return RET_NO;

    return nResult;
}

void ORelationDialog::setValid(bool _bValid)
{
    m_xPB_OK->set_sensitive(_bValid);
}

void ORelationDialog::notifyConnectionChange()
{
    Init(m_pConnData);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
