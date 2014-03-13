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
#include "dbu_qry.hrc"
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include "QTableConnectionData.hxx"
#include "querycontroller.hxx"
#include "QueryTableView.hxx"
#include "QueryDesignView.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include "RelationControl.hxx"
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>

#define ID_INNER_JOIN       1
#define ID_LEFT_JOIN        2
#define ID_RIGHT_JOIN       3
#define ID_FULL_JOIN        4
#define ID_CROSS_JOIN       5

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbc;

DlgQryJoin::DlgQryJoin( OQueryTableView * pParent,
                       const TTableConnectionData::value_type& _pData,
                       OJoinTableView::OTableWindowMap* _pTableMap,
                       const Reference< XConnection >& _xConnection,
                       sal_Bool _bAllowTableSelect)
    : ModalDialog( pParent, "JoinDialog", "dbaccess/ui/joindialog.ui" )
    , m_pTableControl( NULL )
    , m_pTableMap(_pTableMap)
    , m_pTableView(pParent)
    , eJoinType(static_cast<OQueryTableConnectionData*>(_pData.get())->GetJoinType())
    , m_pOrigConnData(_pData)
    , m_xConnection(_xConnection)
{
    get(m_pML_HelpText, "helptext");
    get(m_pLB_JoinType, "type");
    get(m_pCBNatural, "natural");
    get(m_pPB_OK, "ok");

    m_pML_HelpText->SetControlBackground( GetSettings().GetStyleSettings().GetFaceColor() );
    // Connection kopieren
    m_pConnData.reset(_pData->NewInstance());
    m_pConnData->CopyFrom(*_pData);

    m_pTableControl = new OTableListBoxControl(this, m_pTableMap, this);

    m_pCBNatural->Check(static_cast<OQueryTableConnectionData*>(m_pConnData.get())->isNatural());

    if( _bAllowTableSelect )
    {
        m_pTableControl->Init( m_pConnData );
        m_pTableControl->fillListBoxes();
    }
    else
    {
        m_pTableControl->fillAndDisable(m_pConnData);
        m_pTableControl->Init( m_pConnData );
    }

    m_pTableControl->lateUIInit();

    sal_Bool bSupportFullJoin = sal_False;
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
    sal_Bool bSupportOuterJoin = sal_False;
    try
    {
        if ( xMeta.is() )
            bSupportOuterJoin= xMeta->supportsOuterJoins();
    }
    catch(SQLException&)
    {
    }

    setJoinType(eJoinType);

    m_pPB_OK->SetClickHdl( LINK(this, DlgQryJoin, OKClickHdl) );

    m_pLB_JoinType->SetSelectHdl(LINK(this,DlgQryJoin,LBChangeHdl));
    m_pCBNatural->SetToggleHdl(LINK(this,DlgQryJoin,NaturalToggleHdl));

    if ( static_cast<OQueryTableView*>(pParent)->getDesignView()->getController().isReadOnly() )
    {
        m_pLB_JoinType->Disable();
        m_pCBNatural->Disable();
        m_pTableControl->Disable();
    }
    else
    {
        const sal_uInt16 nCount = m_pLB_JoinType->GetEntryCount();
        for (sal_uInt16 i = 0; i < nCount; ++i)
        {
            const sal_IntPtr nJoinTyp = reinterpret_cast<sal_IntPtr>(m_pLB_JoinType->GetEntryData(i));
            if ( !bSupportFullJoin && nJoinTyp == ID_FULL_JOIN )
                m_pLB_JoinType->RemoveEntry(i);
            else if ( !bSupportOuterJoin && (nJoinTyp == ID_LEFT_JOIN || nJoinTyp == ID_RIGHT_JOIN) )
                m_pLB_JoinType->RemoveEntry(i);
        }

        m_pTableControl->NotifyCellChange();
        m_pTableControl->enableRelation(!static_cast<OQueryTableConnectionData*>(m_pConnData.get())->isNatural() && eJoinType != CROSS_JOIN );
    }
}

DlgQryJoin::~DlgQryJoin()
{
    delete m_pTableControl;
}

IMPL_LINK( DlgQryJoin, LBChangeHdl, ListBox*, /*pListBox*/ )
{
    if (m_pLB_JoinType->GetSelectEntryPos() == m_pLB_JoinType->GetSavedValue() )
        return 1;

    m_pLB_JoinType->SaveValue();
    m_pML_HelpText->SetText(OUString());

    m_pTableControl->enableRelation(true);

    OUString sFirstWinName    = m_pConnData->getReferencingTable()->GetWinName();
    OUString sSecondWinName   = m_pConnData->getReferencedTable()->GetWinName();
    const EJoinType eOldJoinType = eJoinType;
    sal_uInt16 nResId = 0;
    const sal_uInt16 nPos = m_pLB_JoinType->GetSelectEntryPos();
    const sal_IntPtr nJoinType = reinterpret_cast<sal_IntPtr>(m_pLB_JoinType->GetEntryData(nPos));
    sal_Bool bAddHint = sal_True;
    switch ( nJoinType )
    {
        default:
        case ID_INNER_JOIN:
            nResId = STR_QUERY_INNER_JOIN;
            bAddHint = sal_False;
            eJoinType = INNER_JOIN;
            break;
        case ID_LEFT_JOIN:
            nResId = STR_QUERY_LEFTRIGHT_JOIN;
            eJoinType = LEFT_JOIN;
            break;
        case ID_RIGHT_JOIN:
            {
                nResId = STR_QUERY_LEFTRIGHT_JOIN;
                eJoinType = RIGHT_JOIN;
                OUString sTemp = sFirstWinName;
                sFirstWinName = sSecondWinName;
                sSecondWinName = sTemp;
            }
            break;
        case ID_FULL_JOIN:
            nResId = STR_QUERY_FULL_JOIN;
            eJoinType = FULL_JOIN;
            break;
        case ID_CROSS_JOIN:
            {
                nResId = STR_QUERY_CROSS_JOIN;
                eJoinType = CROSS_JOIN;

                m_pConnData->ResetConnLines();
                m_pTableControl->lateInit();
                m_pCBNatural->Check(false);
                m_pTableControl->enableRelation(false);
                OUString sEmpty;
                m_pConnData->AppendConnLine(sEmpty,sEmpty);
                m_pPB_OK->Enable(true);
            }
            break;
    }

    m_pCBNatural->Enable(eJoinType != CROSS_JOIN);

    if ( eJoinType != eOldJoinType && eOldJoinType == CROSS_JOIN )
    {
        m_pConnData->ResetConnLines();
    }
    if ( eJoinType != CROSS_JOIN )
    {
        m_pTableControl->NotifyCellChange();
        NaturalToggleHdl(m_pCBNatural);
    }

    m_pTableControl->Invalidate();

    OUString sHelpText = ModuleRes( nResId );
    if( nPos )
    {
        sHelpText = sHelpText.replaceFirst( "%1", sFirstWinName );
        sHelpText = sHelpText.replaceFirst( "%2", sSecondWinName );
    }
    if ( bAddHint )
    {
        sHelpText += "\n";
        sHelpText += ModuleRes( STR_JOIN_TYPE_HINT );
    }

    m_pML_HelpText->SetText( sHelpText );
    return 1;
}

IMPL_LINK( DlgQryJoin, OKClickHdl, Button*, /*pButton*/ )
{

    m_pConnData->Update();
    m_pOrigConnData->CopyFrom( *m_pConnData );

    EndDialog(RET_OK);
    return 1;
}

IMPL_LINK( DlgQryJoin, NaturalToggleHdl, CheckBox*, /*pButton*/ )
{
    sal_Bool bChecked = m_pCBNatural->IsChecked();
    static_cast<OQueryTableConnectionData*>(m_pConnData.get())->setNatural(bChecked);
    m_pTableControl->enableRelation(!bChecked);
    if ( bChecked )
    {
        m_pConnData->ResetConnLines();
        try
        {
            Reference<XNameAccess> xReferencedTableColumns(m_pConnData->getReferencedTable()->getColumns());
            Sequence< OUString> aSeq = m_pConnData->getReferencingTable()->getColumns()->getElementNames();
            const OUString* pIter = aSeq.getConstArray();
            const OUString* pEnd   = pIter + aSeq.getLength();
            for(;pIter != pEnd;++pIter)
            {
                if ( xReferencedTableColumns->hasByName(*pIter) )
                    m_pConnData->AppendConnLine(*pIter,*pIter);
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        m_pTableControl->NotifyCellChange();
        m_pTableControl->Invalidate();
    }

    return 1;
}

TTableConnectionData::value_type DlgQryJoin::getConnectionData() const
{
    return m_pConnData;
}

void DlgQryJoin::setValid(sal_Bool _bValid)
{
    m_pPB_OK->Enable(_bValid || eJoinType == CROSS_JOIN );
}

void DlgQryJoin::notifyConnectionChange( )
{
    setJoinType( static_cast<OQueryTableConnectionData*>(m_pConnData.get())->GetJoinType() );
    m_pCBNatural->Check(static_cast<OQueryTableConnectionData*>(m_pConnData.get())->isNatural());
    NaturalToggleHdl(m_pCBNatural);
}

void DlgQryJoin::setJoinType(EJoinType _eNewJoinType)
{
    eJoinType = _eNewJoinType;
    m_pCBNatural->Enable(eJoinType != CROSS_JOIN);

    sal_IntPtr nJoinType = 0;
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

    const sal_uInt16 nCount = m_pLB_JoinType->GetEntryCount();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        if ( nJoinType == reinterpret_cast<sal_IntPtr>(m_pLB_JoinType->GetEntryData(i)) )
        {
            m_pLB_JoinType->SelectEntryPos(i);
            break;
        }
    }

    LBChangeHdl(m_pLB_JoinType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
