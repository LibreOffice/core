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

#include "QTableWindow.hxx"
#include <QueryTableView.hxx>
#include <JoinController.hxx>
#include <JoinDesignView.hxx>
#include <osl/diagnose.h>
#include <helpids.h>
#include <browserids.hxx>
#include <TableWindowListBox.hxx>
#include <strings.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include "TableFieldInfo.hxx"
#include <comphelper/stl_types.hxx>
#include <comphelper/types.hxx>

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace dbaui;
OQueryTableWindow::OQueryTableWindow( vcl::Window* pParent, const TTableWindowData::value_type& pTabWinData)
    :OTableWindow( pParent, pTabWinData )
    ,m_nAliasNum(0)
{
    m_strInitialAlias = GetAliasName();

    // if table name matches alias, do not pass to InitialAlias,
    // as the appending of a possible token could not succeed...
    if (m_strInitialAlias == pTabWinData->GetTableName())
        m_strInitialAlias.clear();

    SetHelpId(HID_CTL_QRYDGNTAB);
}

bool OQueryTableWindow::Init()
{
    bool bSuccess = OTableWindow::Init();
    if (!bSuccess)
        return bSuccess;

    OQueryTableView* pContainer = static_cast<OQueryTableView*>(getTableView());

    // first determine Alias
    OUString sAliasName;

    TTableWindowData::value_type pWinData = GetData();

    if (!m_strInitialAlias.isEmpty() )
        // Alias was explicitly given
        sAliasName = m_strInitialAlias;
    else if ( GetTable().is() )
        GetTable()->getPropertyValue( PROPERTY_NAME ) >>= sAliasName;
    else
        return false;

    // Alias with successive number
    if (pContainer->CountTableAlias(sAliasName, m_nAliasNum))
    {
        sAliasName += "_" + OUString::number(m_nAliasNum);
    }

    sAliasName = sAliasName.replaceAll("\"", "");
    SetAliasName(sAliasName);
        // SetAliasName passes it as WinName, hence it uses the base class
    // reset the title
    m_xTitle->SetText( pWinData->GetWinName() );
    m_xTitle->Show();

    getTableView()->getDesignView()->getController().InvalidateFeature(ID_BROWSER_QUERY_EXECUTE);
    return bSuccess;
}

void* OQueryTableWindow::createUserData(const Reference< XPropertySet>& _xColumn,bool _bPrimaryKey)
{
    OTableFieldInfo* pInfo = new OTableFieldInfo();
    pInfo->SetKey(_bPrimaryKey ? TAB_PRIMARY_FIELD : TAB_NORMAL_FIELD);
    if ( _xColumn.is() )
        pInfo->SetDataType(::comphelper::getINT32(_xColumn->getPropertyValue(PROPERTY_TYPE)));
    return pInfo;
}

void OQueryTableWindow::deleteUserData(void*& _pUserData)
{
    delete static_cast<OTableFieldInfo*>(_pUserData);
    _pUserData = nullptr;
}

void OQueryTableWindow::OnEntryDoubleClicked(weld::TreeIter& rEntry)
{
    if (getTableView()->getDesignView()->getController().isReadOnly())
        return;

    weld::TreeView& rTreeView = m_xListBox->get_widget();
    OTableFieldInfo* pInf = reinterpret_cast<OTableFieldInfo*>(rTreeView.get_id(rEntry).toUInt64());
    OSL_ENSURE(pInf != nullptr, "OQueryTableWindow::OnEntryDoubleClicked : field doesn't have FieldInfo !");

    // build up DragInfo
    OTableFieldDescRef aInfo = new OTableFieldDesc(GetTableName(), rTreeView.get_text(rEntry));
    aInfo->SetTabWindow(this);
    aInfo->SetAlias(GetAliasName());
    aInfo->SetFieldIndex(rTreeView.get_iter_index_in_parent(rEntry));
    aInfo->SetDataType(pInf->GetDataType());

    // and insert corresponding field
    static_cast<OQueryTableView*>(getTableView())->InsertField(aInfo);
}

bool OQueryTableWindow::ExistsField(const OUString& strFieldName, OTableFieldDescRef const & rInfo)
{
    OSL_ENSURE(m_xListBox != nullptr, "OQueryTableWindow::ExistsField : doesn't have css::form::ListBox !");
    OSL_ENSURE(rInfo.is(),"OQueryTableWindow::ExistsField: invalid argument for OTableFieldDescRef!");
    Reference< XConnection> xConnection = getTableView()->getDesignView()->getController().getConnection();
    bool bExists = false;
    if(xConnection.is())
    {
        weld::TreeView& rTreeView = m_xListBox->get_widget();
        std::unique_ptr<weld::TreeIter> xEntry(rTreeView.make_iterator());
        bool bEntry = rTreeView.get_iter_first(*xEntry);
        try
        {
            Reference<XDatabaseMetaData> xMeta = xConnection->getMetaData();
            ::comphelper::UStringMixEqual bCase(xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers());

            while (bEntry)
            {
                if (bCase(strFieldName, rTreeView.get_text(*xEntry)))
                {
                    OTableFieldInfo* pInf = reinterpret_cast<OTableFieldInfo*>(rTreeView.get_id(*xEntry).toUInt64());
                    assert(pInf && "OQueryTableWindow::ExistsField : field doesn't have FieldInfo !");

                    rInfo->SetTabWindow(this);
                    rInfo->SetField(strFieldName);
                    rInfo->SetTable(GetTableName());
                    rInfo->SetAlias(GetAliasName());
                    rInfo->SetFieldIndex(rTreeView.get_iter_index_in_parent(*xEntry));
                    rInfo->SetDataType(pInf->GetDataType());
                    bExists = true;
                    break;
                }
                bEntry = rTreeView.iter_next(*xEntry);
            }
        }
        catch(SQLException&)
        {
        }
    }

    return bExists;
}

bool OQueryTableWindow::ExistsAVisitedConn() const
{
    return static_cast<const OQueryTableView*>(getTableView())->ExistsAVisitedConn(this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
