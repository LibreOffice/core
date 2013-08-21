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
#include "QueryTableView.hxx"
#include "dbustrings.hrc"
#include <osl/diagnose.h>
#include "dbaccess_helpid.hrc"
#include "QueryDesignView.hxx"
#include "browserids.hxx"
#include "querycontroller.hxx"
#include <vcl/image.hxx>
#include "TableWindowListBox.hxx"
#include "dbu_qry.hrc"
#include "Query.hrc"
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include "TableFieldInfo.hxx"
#include <comphelper/extract.hxx>
#include <comphelper/string.hxx>
#include <comphelper/uno3.hxx>
#include "UITools.hxx"
#include "svtools/treelistentry.hxx"

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace dbaui;
//========================================================================
// class OQueryTableWindow
//========================================================================
DBG_NAME(OQueryTableWindow)
//------------------------------------------------------------------------------
OQueryTableWindow::OQueryTableWindow( Window* pParent, const TTableWindowData::value_type& pTabWinData, sal_Unicode* pszInitialAlias)
    :OTableWindow( pParent, pTabWinData )
    ,m_nAliasNum(0)
{
    DBG_CTOR(OQueryTableWindow,NULL);
    if (pszInitialAlias != NULL)
        m_strInitialAlias = OUString(pszInitialAlias);
    else
        m_strInitialAlias = GetAliasName();

    // if table name matches alias, do not pass to InitialAlias,
    // as the appending of a possible token could not succeed...
    if (m_strInitialAlias == pTabWinData->GetTableName())
        m_strInitialAlias = OUString();

    SetHelpId(HID_CTL_QRYDGNTAB);
}

//------------------------------------------------------------------------------
OQueryTableWindow::~OQueryTableWindow()
{
    DBG_DTOR(OQueryTableWindow,NULL);
}

//------------------------------------------------------------------------------
sal_Bool OQueryTableWindow::Init()
{
    sal_Bool bSuccess = OTableWindow::Init();
    if(!bSuccess)
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
        return sal_False;

    // Alias with successive number
    if (pContainer->CountTableAlias(sAliasName, m_nAliasNum))
    {
        sAliasName += OUString('_');
        sAliasName += OUString::number(m_nAliasNum);
    }


    sAliasName = comphelper::string::remove(sAliasName, '"');
    SetAliasName(sAliasName);
        // SetAliasName passes it as WinName, hence it uses the base class
    // reset the title
    m_aTitle.SetText( pWinData->GetWinName() );
    m_aTitle.Show();

    if (!bSuccess)
    {   // it should just open a dummy window...
        OSL_ENSURE(!GetAliasName().isEmpty(), "OQueryTableWindow::Init : kein Alias- UND kein Tabellenname geht nicht !");
            // .. but that needs at least an Alias

        // create ::com::sun::star::form::ListBox
        if (!m_pListBox)
            m_pListBox = CreateListBox();

        // set titel
        m_aTitle.SetText(GetAliasName());
        m_aTitle.Show();

        clearListBox();
            // don't need to refill them as I don't have a table
        m_pListBox->Show();
    }

    getTableView()->getDesignView()->getController().InvalidateFeature(ID_BROWSER_QUERY_EXECUTE);
    return bSuccess;
}
// -----------------------------------------------------------------------------
void* OQueryTableWindow::createUserData(const Reference< XPropertySet>& _xColumn,bool _bPrimaryKey)
{
    OTableFieldInfo* pInfo = new OTableFieldInfo();
    pInfo->SetKey(_bPrimaryKey ? TAB_PRIMARY_FIELD : TAB_NORMAL_FIELD);
    if ( _xColumn.is() )
        pInfo->SetDataType(::comphelper::getINT32(_xColumn->getPropertyValue(PROPERTY_TYPE)));
    return pInfo;
}
// -----------------------------------------------------------------------------
void OQueryTableWindow::deleteUserData(void*& _pUserData)
{
    delete static_cast<OTableFieldInfo*>(_pUserData);
    _pUserData = NULL;
}
//------------------------------------------------------------------------------
void OQueryTableWindow::OnEntryDoubleClicked(SvTreeListEntry* pEntry)
{
    OSL_ENSURE(pEntry != NULL, "OQueryTableWindow::OnEntryDoubleClicked : pEntry must not be NULL !");
        // you could also scan that and then return, but like this it could possibly hint to faults at the caller

    if (getTableView()->getDesignView()->getController().isReadOnly())
        return;

    OTableFieldInfo* pInf = static_cast<OTableFieldInfo*>(pEntry->GetUserData());
    OSL_ENSURE(pInf != NULL, "OQueryTableWindow::OnEntryDoubleClicked : field doesn't have FieldInfo !");

    // build up DragInfo
    OTableFieldDescRef aInfo = new OTableFieldDesc(GetTableName(),m_pListBox->GetEntryText(pEntry));
    aInfo->SetTabWindow(this);
    aInfo->SetAlias(GetAliasName());
    aInfo->SetFieldIndex(m_pListBox->GetModel()->GetAbsPos(pEntry));
    aInfo->SetDataType(pInf->GetDataType());

    // and insert corresponding field
    static_cast<OQueryTableView*>(getTableView())->InsertField(aInfo);
}

//------------------------------------------------------------------------------
sal_Bool OQueryTableWindow::ExistsField(const OUString& strFieldName, OTableFieldDescRef& rInfo)
{
    OSL_ENSURE(m_pListBox != NULL, "OQueryTableWindow::ExistsField : doesn't have ::com::sun::star::form::ListBox !");
    OSL_ENSURE(rInfo.is(),"OQueryTableWindow::ExistsField: invalid argument for OTableFieldDescRef!");
    Reference< XConnection> xConnection = getTableView()->getDesignView()->getController().getConnection();
    sal_Bool bExists = sal_False;
    if(xConnection.is())
    {
        SvTreeListEntry* pEntry = m_pListBox->First();
        try
        {
            Reference<XDatabaseMetaData> xMeta = xConnection->getMetaData();
            ::comphelper::UStringMixEqual bCase(xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers());

            while (pEntry)
            {
                if (bCase(strFieldName,OUString(m_pListBox->GetEntryText(pEntry))))
                {
                    OTableFieldInfo* pInf = static_cast<OTableFieldInfo*>(pEntry->GetUserData());
                    OSL_ENSURE(pInf != NULL, "OQueryTableWindow::ExistsField : field doesn't have FieldInfo !");

                    rInfo->SetTabWindow(this);
                    rInfo->SetField(strFieldName);
                    rInfo->SetTable(GetTableName());
                    rInfo->SetAlias(GetAliasName());
                    rInfo->SetFieldIndex(m_pListBox->GetModel()->GetAbsPos(pEntry));
                    rInfo->SetDataType(pInf->GetDataType());
                    bExists = sal_True;
                    break;
                }
                pEntry = m_pListBox->Next(pEntry);
            }
        }
        catch(SQLException&)
        {
        }
    }

    return bExists;
}

//------------------------------------------------------------------------------
sal_Bool OQueryTableWindow::ExistsAVisitedConn() const
{
    return static_cast<const OQueryTableView*>(getTableView())->ExistsAVisitedConn(this);
}

//------------------------------------------------------------------------------
void OQueryTableWindow::KeyInput( const KeyEvent& rEvt )
{
    OTableWindow::KeyInput( rEvt );
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
