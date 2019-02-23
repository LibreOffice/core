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

#include <vcl/wrkwin.hxx>

#include <vcl/svapp.hxx>
#include <dbu_dlg.hxx>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>

#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <UITools.hxx>
#include <JoinDesignView.hxx>
#include <JoinController.hxx>
#include <connectivity/dbexception.hxx>
#include <RTableConnectionData.hxx>
#include <RelationControl.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <algorithm>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::dbaui;
using namespace ::dbtools;

// class ORelationDialog
ORelationDialog::ORelationDialog( OJoinTableView* pParent,
                                 const TTableConnectionData::value_type& pConnectionData,
                                 bool bAllowTableSelect )
    : ModalDialog(pParent, "RelationDialog",
        "dbaccess/ui/relationdialog.ui")
    , m_pOrigConnData(pConnectionData)
    , m_bTriedOneUpdate(false)
{
    get(m_pRB_NoCascUpd, "addaction");
    get(m_pRB_CascUpd, "addcascade");
    get(m_pRB_CascUpdNull, "addnull");
    get(m_pRB_CascUpdDefault, "adddefault");
    get(m_pRB_NoCascDel, "delaction");
    get(m_pRB_CascDel, "delcascade");
    get(m_pRB_CascDelNull, "delnull");
    get(m_pRB_CascDelDefault, "deldefault");
    get(m_pPB_OK, "ok");

    // Copy connection
    m_pConnData.reset( static_cast<ORelationTableConnectionData*>(pConnectionData->NewInstance()) );
    m_pConnData->CopyFrom( *pConnectionData );

    Init(m_pConnData);
    m_xTableControl.reset( new OTableListBoxControl(this, &pParent->GetTabWinMap(), this) );

    m_pPB_OK->SetClickHdl( LINK(this, ORelationDialog, OKClickHdl) );

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
    disposeOnce();
}

void ORelationDialog::dispose()
{
    m_pRB_NoCascUpd.clear();
    m_pRB_CascUpd.clear();
    m_pRB_CascUpdNull.clear();
    m_pRB_CascUpdDefault.clear();
    m_pRB_NoCascDel.clear();
    m_pRB_CascDel.clear();
    m_pRB_CascDelNull.clear();
    m_pRB_CascDelDefault.clear();
    m_pPB_OK.clear();
    ModalDialog::dispose();
}


void ORelationDialog::Init(const TTableConnectionData::value_type& _pConnectionData)
{
    ORelationTableConnectionData* pConnData = static_cast<ORelationTableConnectionData*>(_pConnectionData.get());
    // Update Rules
    switch (pConnData->GetUpdateRules())
    {
    case KeyRule::NO_ACTION:
    case KeyRule::RESTRICT:
        m_pRB_NoCascUpd->Check();
        break;

    case KeyRule::CASCADE:
        m_pRB_CascUpd->Check();
        break;

    case KeyRule::SET_NULL:
        m_pRB_CascUpdNull->Check();
        break;
    case KeyRule::SET_DEFAULT:
        m_pRB_CascUpdDefault->Check();
        break;
    }

    // Delete Rules
    switch (pConnData->GetDeleteRules())
    {
    case KeyRule::NO_ACTION:
    case KeyRule::RESTRICT:
        m_pRB_NoCascDel->Check();
        break;

    case KeyRule::CASCADE:
        m_pRB_CascDel->Check();
        break;

    case KeyRule::SET_NULL:
        m_pRB_CascDelNull->Check();
        break;
    case KeyRule::SET_DEFAULT:
        m_pRB_CascDelDefault->Check();
        break;
    }
}

IMPL_LINK_NOARG( ORelationDialog, OKClickHdl, Button*, void )
{
    // Read out RadioButtons
    sal_uInt16 nAttrib = 0;

    // Delete Rules
    if( m_pRB_NoCascDel->IsChecked() )
        nAttrib |= KeyRule::NO_ACTION;
    if( m_pRB_CascDel->IsChecked() )
        nAttrib |= KeyRule::CASCADE;
    if( m_pRB_CascDelNull->IsChecked() )
        nAttrib |= KeyRule::SET_NULL;
    if( m_pRB_CascDelDefault->IsChecked() )
        nAttrib |= KeyRule::SET_DEFAULT;

    ORelationTableConnectionData* pConnData = static_cast<ORelationTableConnectionData*>(m_pConnData.get());
    pConnData->SetDeleteRules( nAttrib );

    // Update Rules
    nAttrib = 0;
    if( m_pRB_NoCascUpd->IsChecked() )
        nAttrib |= KeyRule::NO_ACTION;
    if( m_pRB_CascUpd->IsChecked() )
        nAttrib |= KeyRule::CASCADE;
    if( m_pRB_CascUpdNull->IsChecked() )
        nAttrib |= KeyRule::SET_NULL;
    if( m_pRB_CascUpdDefault->IsChecked() )
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
            EndDialog( RET_OK );
            return;
        }
    }
    catch( const SQLException& )
    {
        ::dbtools::showError( SQLExceptionInfo( ::cppu::getCaughtException() ),
                            VCLUnoHelper::GetInterface(this),
                            static_cast<OJoinTableView*>(GetParent())->getDesignView()->getController().getORB());
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

short ORelationDialog::Execute()
{
    short nResult = ModalDialog::Execute();
    if ((nResult != RET_OK) && m_bTriedOneUpdate)
        return RET_NO;

    return nResult;
}

void ORelationDialog::setValid(bool _bValid)
{
    m_pPB_OK->Enable(_bValid);
}

void ORelationDialog::notifyConnectionChange()
{
    Init(m_pConnData);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
