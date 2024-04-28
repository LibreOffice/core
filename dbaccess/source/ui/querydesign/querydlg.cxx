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

#include "querydlg.hxx"
#include <JoinController.hxx>
#include <JoinDesignView.hxx>
#include <strings.hrc>
#include <comphelper/diagnose_ex.hxx>
#include "QTableConnectionData.hxx"
#include <core_resource.hxx>
#include <QueryTableView.hxx>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <RelationControl.hxx>

#define ID_INNER_JOIN       1
#define ID_LEFT_JOIN        2
#define ID_RIGHT_JOIN       3
#define ID_FULL_JOIN        4
#define ID_CROSS_JOIN       5

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbc;

DlgQryJoin::DlgQryJoin(const OQueryTableView* pParent,
                       const TTableConnectionData::value_type& _pData,
                       const OJoinTableView::OTableWindowMap* _pTableMap,
                       const Reference< XConnection >& _xConnection,
                       bool _bAllowTableSelect)
    : GenericDialogController(pParent->GetFrameWeld(), "dbaccess/ui/joindialog.ui", "JoinDialog")
    , eJoinType(static_cast<OQueryTableConnectionData*>(_pData.get())->GetJoinType())
    , m_pOrigConnData(_pData)
    , m_xConnection(_xConnection)
    , m_xML_HelpText(m_xBuilder->weld_label("helptext"))
    , m_xPB_OK(m_xBuilder->weld_button("ok"))
    , m_xLB_JoinType(m_xBuilder->weld_combo_box("type"))
    , m_xCBNatural(m_xBuilder->weld_check_button("natural"))
{
    Size aSize(m_xML_HelpText->get_approximate_digit_width() * 44,
               m_xML_HelpText->get_text_height() * 6);
    //alternatively loop through the STR_QUERY_* strings with their STR_JOIN_TYPE_HINT
    //suffix to find the longest entry at runtime
    m_xML_HelpText->set_size_request(aSize.Width(), aSize.Height());

    // Copy connection
    m_pConnData = _pData->NewInstance();
    m_pConnData->CopyFrom(*_pData);

    m_xTableControl.reset(new OTableListBoxControl(m_xBuilder.get(), _pTableMap, this));

    m_xCBNatural->set_active(static_cast<OQueryTableConnectionData*>(m_pConnData.get())->isNatural());

    if( _bAllowTableSelect )
    {
        m_xTableControl->Init( m_pConnData );
        m_xTableControl->fillListBoxes();
    }
    else
    {
        m_xTableControl->fillAndDisable(m_pConnData);
        m_xTableControl->Init( m_pConnData );
    }

    m_xTableControl->lateUIInit();

    bool bSupportFullJoin = false;
    Reference<XDatabaseMetaData> xMeta;
    try
    {
        xMeta = m_xConnection->getMetaData();
        if ( xMeta.is() )
            bSupportFullJoin = xMeta->supportsFullOuterJoins();
    }
    catch(SQLException&)
    {
    }
    bool bSupportOuterJoin = false;
    try
    {
        if ( xMeta.is() )
            bSupportOuterJoin= xMeta->supportsOuterJoins();
    }
    catch(SQLException&)
    {
    }

    setJoinType(eJoinType);

    m_xPB_OK->connect_clicked(LINK(this, DlgQryJoin, OKClickHdl));

    m_xLB_JoinType->connect_changed(LINK(this,DlgQryJoin,LBChangeHdl));
    m_xCBNatural->connect_toggled(LINK(this,DlgQryJoin,NaturalToggleHdl));

    if ( pParent->getDesignView()->getController().isReadOnly() )
    {
        m_xLB_JoinType->set_sensitive(false);
        m_xCBNatural->set_sensitive(false);
        m_xTableControl->Disable();
    }
    else
    {
        for (sal_Int32 i = 0; i < m_xLB_JoinType->get_count();)
        {
            const sal_Int32 nJoinTyp = m_xLB_JoinType->get_id(i).toInt32();
            if ( !bSupportFullJoin && nJoinTyp == ID_FULL_JOIN )
                m_xLB_JoinType->remove(i);
            else if ( !bSupportOuterJoin && (nJoinTyp == ID_LEFT_JOIN || nJoinTyp == ID_RIGHT_JOIN) )
                m_xLB_JoinType->remove(i);
            else
                ++i;
        }

        m_xTableControl->NotifyCellChange();
        m_xTableControl->enableRelation(!static_cast<OQueryTableConnectionData*>(m_pConnData.get())->isNatural() && eJoinType != CROSS_JOIN );
    }
}

DlgQryJoin::~DlgQryJoin()
{
}

IMPL_LINK_NOARG( DlgQryJoin, LBChangeHdl, weld::ComboBox&, void )
{
    if (!m_xLB_JoinType->get_value_changed_from_saved())
        return;

    m_xLB_JoinType->save_value();
    m_xML_HelpText->set_label(OUString());

    m_xTableControl->enableRelation(true);

    OUString sFirstWinName    = m_pConnData->getReferencingTable()->GetWinName();
    OUString sSecondWinName   = m_pConnData->getReferencedTable()->GetWinName();
    const EJoinType eOldJoinType = eJoinType;
    TranslateId pResId;
    const sal_Int32 nPos = m_xLB_JoinType->get_active();
    const sal_Int32 nJoinType = m_xLB_JoinType->get_id(nPos).toInt32();
    bool bAddHint = true;
    switch ( nJoinType )
    {
        default:
        case ID_INNER_JOIN:
            pResId = STR_QUERY_INNER_JOIN;
            bAddHint = false;
            eJoinType = INNER_JOIN;
            break;
        case ID_LEFT_JOIN:
            pResId = STR_QUERY_LEFTRIGHT_JOIN;
            eJoinType = LEFT_JOIN;
            break;
        case ID_RIGHT_JOIN:
            pResId = STR_QUERY_LEFTRIGHT_JOIN;
            eJoinType = RIGHT_JOIN;
            std::swap( sFirstWinName, sSecondWinName );
            break;
        case ID_FULL_JOIN:
            pResId = STR_QUERY_FULL_JOIN;
            eJoinType = FULL_JOIN;
            break;
        case ID_CROSS_JOIN:
            {
                pResId = STR_QUERY_CROSS_JOIN;
                eJoinType = CROSS_JOIN;

                m_pConnData->ResetConnLines();
                m_xTableControl->lateInit();
                m_xCBNatural->set_active(false);
                m_xTableControl->enableRelation(false);
                m_pConnData->AppendConnLine("","");
                m_xPB_OK->set_sensitive(true);
            }
            break;
    }

    m_xCBNatural->set_sensitive(eJoinType != CROSS_JOIN);

    if ( eJoinType != eOldJoinType && eOldJoinType == CROSS_JOIN )
    {
        m_pConnData->ResetConnLines();
    }
    if ( eJoinType != CROSS_JOIN )
    {
        m_xTableControl->NotifyCellChange();
        NaturalToggleHdl(*m_xCBNatural);
    }

    m_xTableControl->Invalidate();

    OUString sHelpText = DBA_RES(pResId);
    if( nPos )
    {
        sHelpText = sHelpText.replaceFirst( "%1", sFirstWinName );
        sHelpText = sHelpText.replaceFirst( "%2", sSecondWinName );
    }
    if ( bAddHint )
    {
        sHelpText += "\n" + DBA_RES( STR_JOIN_TYPE_HINT );
    }

    m_xML_HelpText->set_label( sHelpText );
}

IMPL_LINK_NOARG(DlgQryJoin, OKClickHdl, weld::Button&, void)
{
    m_pConnData->Update();
    m_pOrigConnData->CopyFrom( *m_pConnData );

    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(DlgQryJoin, NaturalToggleHdl, weld::Toggleable&, void)
{
    bool bChecked = m_xCBNatural->get_active();
    static_cast<OQueryTableConnectionData*>(m_pConnData.get())->setNatural(bChecked);
    m_xTableControl->enableRelation(!bChecked);
    if ( !bChecked )
        return;

    m_pConnData->ResetConnLines();
    try
    {
        Reference<XNameAccess> xReferencedTableColumns(m_pConnData->getReferencedTable()->getColumns());
        for (auto& column : m_pConnData->getReferencingTable()->getColumns()->getElementNames())
        {
            if (xReferencedTableColumns->hasByName(column))
                m_pConnData->AppendConnLine(column, column);
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
    m_xTableControl->NotifyCellChange();
    m_xTableControl->Invalidate();
}

void DlgQryJoin::setValid(bool _bValid)
{
    m_xPB_OK->set_sensitive(_bValid || eJoinType == CROSS_JOIN );
}

void DlgQryJoin::notifyConnectionChange( )
{
    setJoinType( static_cast<OQueryTableConnectionData*>(m_pConnData.get())->GetJoinType() );
    m_xCBNatural->set_active(static_cast<OQueryTableConnectionData*>(m_pConnData.get())->isNatural());
    NaturalToggleHdl(*m_xCBNatural);
}

void DlgQryJoin::setJoinType(EJoinType _eNewJoinType)
{
    eJoinType = _eNewJoinType;
    m_xCBNatural->set_sensitive(eJoinType != CROSS_JOIN);

    sal_Int32 nJoinType = 0;
    switch ( eJoinType )
    {
        default:
        case INNER_JOIN:
            nJoinType = ID_INNER_JOIN;
            break;
        case LEFT_JOIN:
            nJoinType = ID_LEFT_JOIN;
            break;
        case RIGHT_JOIN:
            nJoinType = ID_RIGHT_JOIN;
            break;
        case FULL_JOIN:
            nJoinType = ID_FULL_JOIN;
            break;
        case CROSS_JOIN:
            nJoinType = ID_CROSS_JOIN;
            break;
    }

    const sal_Int32 nCount = m_xLB_JoinType->get_count();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        if (nJoinType == m_xLB_JoinType->get_id(i).toInt32())
        {
            m_xLB_JoinType->set_active(i);
            break;
        }
    }

    LBChangeHdl(*m_xLB_JoinType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
