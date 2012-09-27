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
        m_strInitialAlias = ::rtl::OUString(pszInitialAlias);
    else
        m_strInitialAlias = GetAliasName();

    // wenn der Tabellen- gleich dem Aliasnamen ist, dann darf ich das nicht an InitialAlias weiterreichen, denn das Anhaengen
    // eines eventuelle Tokens nicht klappen ...
    if (m_strInitialAlias == pTabWinData->GetTableName())
        m_strInitialAlias = ::rtl::OUString();

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

    // zuerst Alias bestimmen
    ::rtl::OUString sAliasName;

    TTableWindowData::value_type pWinData = GetData();

    if (!m_strInitialAlias.isEmpty() )
        // Der Alias wurde explizit mit angegeben
        sAliasName = m_strInitialAlias;
    else if ( GetTable().is() )
        GetTable()->getPropertyValue( PROPERTY_NAME ) >>= sAliasName;
    else
        return sal_False;

    // Alias mit fortlaufender Nummer versehen
    if (pContainer->CountTableAlias(sAliasName, m_nAliasNum))
    {
        sAliasName += ::rtl::OUString('_');
        sAliasName += ::rtl::OUString::valueOf(m_nAliasNum);
    }


    sAliasName = comphelper::string::remove(sAliasName, '"');
    SetAliasName(sAliasName);
        // SetAliasName reicht das als WinName weiter, dadurch benutzt es die Basisklasse
    // reset the title
    m_aTitle.SetText( pWinData->GetWinName() );
    m_aTitle.Show();

    if (!bSuccess)
    {   // es soll nur ein Dummy-Window aufgemacht werden ...
        OSL_ENSURE(!GetAliasName().isEmpty(), "OQueryTableWindow::Init : kein Alias- UND kein Tabellenname geht nicht !");
            // .. aber das braucht wenigstens einen Alias

        // ::com::sun::star::form::ListBox anlegen
        if (!m_pListBox)
            m_pListBox = CreateListBox();

        // Titel setzen
        m_aTitle.SetText(GetAliasName());
        m_aTitle.Show();

        clearListBox();
            // neu zu fuellen brauche ich die nicht, da ich ja keine Tabelle habe
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
void OQueryTableWindow::OnEntryDoubleClicked(SvLBoxEntry* pEntry)
{
    OSL_ENSURE(pEntry != NULL, "OQueryTableWindow::OnEntryDoubleClicked : pEntry darf nicht NULL sein !");
        // man koennte das auch abfragen und dann ein return hinsetzen, aber so weist es vielleicht auf Fehler bei Aufrufer hin

    if (getTableView()->getDesignView()->getController().isReadOnly())
        return;

    OTableFieldInfo* pInf = static_cast<OTableFieldInfo*>(pEntry->GetUserData());
    OSL_ENSURE(pInf != NULL, "OQueryTableWindow::OnEntryDoubleClicked : Feld hat keine FieldInfo !");

    // eine DragInfo aufbauen
    OTableFieldDescRef aInfo = new OTableFieldDesc(GetTableName(),m_pListBox->GetEntryText(pEntry));
    aInfo->SetTabWindow(this);
    aInfo->SetAlias(GetAliasName());
    aInfo->SetFieldIndex(m_pListBox->GetModel()->GetAbsPos(pEntry));
    aInfo->SetDataType(pInf->GetDataType());

    // und das entsprechende Feld einfuegen
    static_cast<OQueryTableView*>(getTableView())->InsertField(aInfo);
}

//------------------------------------------------------------------------------
sal_Bool OQueryTableWindow::ExistsField(const ::rtl::OUString& strFieldName, OTableFieldDescRef& rInfo)
{
    OSL_ENSURE(m_pListBox != NULL, "OQueryTableWindow::ExistsField : habe keine ::com::sun::star::form::ListBox !");
    OSL_ENSURE(rInfo.is(),"OQueryTableWindow::ExistsField: invlid argument for OTableFieldDescRef!");
    Reference< XConnection> xConnection = getTableView()->getDesignView()->getController().getConnection();
    sal_Bool bExists = sal_False;
    if(xConnection.is())
    {
        SvLBoxEntry* pEntry = m_pListBox->First();
        try
        {
            Reference<XDatabaseMetaData> xMeta = xConnection->getMetaData();
            ::comphelper::UStringMixEqual bCase(xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers());

            while (pEntry)
            {
                if (bCase(strFieldName,::rtl::OUString(m_pListBox->GetEntryText(pEntry))))
                {
                    OTableFieldInfo* pInf = static_cast<OTableFieldInfo*>(pEntry->GetUserData());
                    OSL_ENSURE(pInf != NULL, "OQueryTableWindow::ExistsField : Feld hat keine FieldInfo !");

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
