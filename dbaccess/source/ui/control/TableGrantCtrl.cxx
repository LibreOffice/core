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

#include <TableGrantCtrl.hxx>
#include <core_resource.hxx>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/PrivilegeObject.hpp>
#include <com/sun/star/sdbcx/XUsersSupplier.hpp>
#include <com/sun/star/sdbcx/XAuthorizable.hpp>
#include <vcl/svapp.hxx>
#include <osl/diagnose.h>
#include <strings.hrc>
#include <UITools.hxx>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::dbaui;
using namespace ::svt;

const sal_uInt16 COL_TABLE_NAME = 1;
const sal_uInt16 COL_SELECT     = 2;
const sal_uInt16 COL_INSERT     = 3;
const sal_uInt16 COL_DELETE     = 4;
const sal_uInt16 COL_UPDATE     = 5;
const sal_uInt16 COL_ALTER      = 6;
const sal_uInt16 COL_REF        = 7;
const sal_uInt16 COL_DROP       = 8;


// OTableGrantControl
OTableGrantControl::OTableGrantControl( vcl::Window* pParent, WinBits nBits)
    :EditBrowseBox( pParent, EditBrowseBoxFlags::SMART_TAB_TRAVEL | EditBrowseBoxFlags::NO_HANDLE_COLUMN_CONTENT, nBits )
    ,m_pCheckCell( nullptr )
    ,m_pEdit( nullptr )
    ,m_nDataPos( 0 )
    ,m_nDeactivateEvent(nullptr)
{
    // insert columns
    sal_uInt16 i=1;
    InsertDataColumn( i, DBA_RES(STR_TABLE_PRIV_NAME), 75);
    FreezeColumn(i++);
    InsertDataColumn( i++, DBA_RES(STR_TABLE_PRIV_SELECT), 75);
    InsertDataColumn( i++, DBA_RES(STR_TABLE_PRIV_INSERT), 75);
    InsertDataColumn( i++, DBA_RES(STR_TABLE_PRIV_DELETE), 75);
    InsertDataColumn( i++, DBA_RES(STR_TABLE_PRIV_UPDATE), 75);
    InsertDataColumn( i++, DBA_RES(STR_TABLE_PRIV_ALTER), 75);
    InsertDataColumn( i++, DBA_RES(STR_TABLE_PRIV_REFERENCE), 75);
    InsertDataColumn( i++, DBA_RES(STR_TABLE_PRIV_DROP), 75);

    while(--i)
        SetColumnWidth(i,GetAutoColumnWidth(i));
}

OTableGrantControl::~OTableGrantControl()
{
    disposeOnce();
}

void OTableGrantControl::dispose()
{
    if (m_nDeactivateEvent)
    {
        Application::RemoveUserEvent(m_nDeactivateEvent);
        m_nDeactivateEvent = nullptr;
    }

    m_pCheckCell.disposeAndClear();
    m_pEdit.disposeAndClear();

    m_xTables       = nullptr;
    ::svt::EditBrowseBox::dispose();
}

void OTableGrantControl::setTablesSupplier(const Reference< XTablesSupplier >& _xTablesSup)
{
    // first we need the users
    Reference< XUsersSupplier> xUserSup(_xTablesSup,UNO_QUERY);
    if(xUserSup.is())
        m_xUsers = xUserSup->getUsers();

    // second we need the tables to determine which privileges the user has
    if(_xTablesSup.is())
        m_xTables = _xTablesSup->getTables();

    if(m_xTables.is())
        m_aTableNames = m_xTables->getElementNames();

    OSL_ENSURE(m_xUsers.is(),"No user access supported!");
    OSL_ENSURE(m_xTables.is(),"No tables supported!");
}

void OTableGrantControl::setComponentContext(const Reference< css::uno::XComponentContext>& _rxContext)
{
  m_xContext = _rxContext;
}

void OTableGrantControl::UpdateTables()
{
    RemoveRows();

    if(m_xTables.is())
        RowInserted(0, m_aTableNames.getLength());
    //  m_bEnable = m_xDb->GetUser() != ((OUserAdmin*)GetParent())->GetUser();
}

void OTableGrantControl::Init()
{
    EditBrowseBox::Init();

    // instantiate ComboBox
    if(!m_pCheckCell)
    {
        m_pCheckCell    = VclPtr<CheckBoxControl>::Create( &GetDataWindow() );
        m_pCheckCell->GetBox().EnableTriState(false);

        m_pEdit         = VclPtr<Edit>::Create( &GetDataWindow() );
        m_pEdit->SetReadOnly();
        m_pEdit->Enable(false);
    }

    UpdateTables();
    // set browser mode
    BrowserMode const nMode = BrowserMode::COLUMNSELECTION | BrowserMode::HLINES | BrowserMode::VLINES |
                              BrowserMode::HIDECURSOR      | BrowserMode::HIDESELECT;

    SetMode(nMode);
}

bool OTableGrantControl::PreNotify(NotifyEvent& rNEvt)
{
    if (rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS)
        if (!HasChildPathFocus())
        {
            if (m_nDeactivateEvent)
                Application::RemoveUserEvent(m_nDeactivateEvent);
            m_nDeactivateEvent = Application::PostUserEvent(LINK(this, OTableGrantControl, AsynchDeactivate), nullptr, true);
        }
    if (rNEvt.GetType() == MouseNotifyEvent::GETFOCUS)
    {
        if (m_nDeactivateEvent)
            Application::RemoveUserEvent(m_nDeactivateEvent);
        m_nDeactivateEvent = Application::PostUserEvent(LINK(this, OTableGrantControl, AsynchActivate), nullptr, true);
    }
    return EditBrowseBox::PreNotify(rNEvt);
}

IMPL_LINK_NOARG(OTableGrantControl, AsynchActivate, void*, void)
{
    m_nDeactivateEvent = nullptr;
    ActivateCell();
}

IMPL_LINK_NOARG(OTableGrantControl, AsynchDeactivate, void*, void)
{
    m_nDeactivateEvent = nullptr;
    DeactivateCell();
}

bool OTableGrantControl::IsTabAllowed(bool bForward) const
{
    long nRow = GetCurRow();
    sal_uInt16 nCol = GetCurColumnId();

    if (bForward && (nCol == 2) && (nRow == GetRowCount() - 1))
        return false;

    if (!bForward && (nCol == 1) && (nRow == 0))
        return false;

    return EditBrowseBox::IsTabAllowed(bForward);
}

#define GRANT_REVOKE_RIGHT(what)                \
    if(m_pCheckCell->GetBox().IsChecked())      \
        xAuth->grantPrivileges(sTableName,PrivilegeObject::TABLE,what);\
    else                                        \
        xAuth->revokePrivileges(sTableName,PrivilegeObject::TABLE,what)

bool OTableGrantControl::SaveModified()
{

    sal_Int32 nRow = GetCurRow();
    if(nRow == -1 || nRow >= m_aTableNames.getLength())
        return false;

    OUString sTableName = m_aTableNames[nRow];
    bool bErg = true;
    try
    {

        if ( m_xUsers->hasByName(m_sUserName) )
        {
            Reference<XAuthorizable> xAuth(m_xUsers->getByName(m_sUserName),UNO_QUERY);
            if ( xAuth.is() )
            {
                switch( GetCurColumnId() )
                {
                    case COL_INSERT:
                        GRANT_REVOKE_RIGHT(Privilege::INSERT);
                        break;
                    case COL_DELETE:
                        GRANT_REVOKE_RIGHT(Privilege::DELETE);
                        break;
                    case COL_UPDATE:
                        GRANT_REVOKE_RIGHT(Privilege::UPDATE);
                        break;
                    case COL_ALTER:
                        GRANT_REVOKE_RIGHT(Privilege::ALTER);
                        break;
                    case COL_SELECT:
                        GRANT_REVOKE_RIGHT(Privilege::SELECT);
                        break;
                    case COL_REF:
                        GRANT_REVOKE_RIGHT(Privilege::REFERENCE);
                        break;
                    case COL_DROP:
                        GRANT_REVOKE_RIGHT(Privilege::DROP);
                        break;
                }
                fillPrivilege(nRow);
            }
        }
    }
    catch(SQLException& e)
    {
        bErg = false;
        ::dbtools::showError(::dbtools::SQLExceptionInfo(e),VCLUnoHelper::GetInterface(GetParent()),m_xContext);
    }
    if(bErg && Controller().is())
        Controller()->ClearModified();
    if(!bErg)
        UpdateTables();

    return bErg;
}

OUString OTableGrantControl::GetCellText( long nRow, sal_uInt16 nColId ) const
{
    if(COL_TABLE_NAME == nColId)
        return m_aTableNames[nRow];

    sal_Int32 nPriv = 0;
    TTablePrivilegeMap::const_iterator aFind = findPrivilege(nRow);
    if(aFind != m_aPrivMap.end())
        nPriv = aFind->second.nRights;

    return OUString::number(isAllowed(nColId,nPriv) ? 1 :0);
}

void OTableGrantControl::InitController( CellControllerRef& /*rController*/, long nRow, sal_uInt16 nColumnId )
{
    OUString sTablename = m_aTableNames[nRow];
    // special case for tablename
    if(nColumnId == COL_TABLE_NAME)
        m_pEdit->SetText(sTablename);
    else
    {
        // get the privileges from the user
        TTablePrivilegeMap::const_iterator aFind = findPrivilege(nRow);
        m_pCheckCell->GetBox().Check(aFind != m_aPrivMap.end() && isAllowed(nColumnId,aFind->second.nRights));
    }
}

void OTableGrantControl::fillPrivilege(sal_Int32 _nRow) const
{

    if ( m_xUsers->hasByName(m_sUserName) )
    {
        try
        {
            Reference<XAuthorizable> xAuth(m_xUsers->getByName(m_sUserName),UNO_QUERY);
            if ( xAuth.is() )
            {
                // get the privileges
                TPrivileges nRights;
                nRights.nRights = xAuth->getPrivileges(m_aTableNames[_nRow],PrivilegeObject::TABLE);
                if(m_xGrantUser.is())
                    nRights.nWithGrant = m_xGrantUser->getGrantablePrivileges(m_aTableNames[_nRow],PrivilegeObject::TABLE);
                else
                    nRights.nWithGrant = 0;

                m_aPrivMap[m_aTableNames[_nRow]] = nRights;
            }
        }
        catch(SQLException& e)
        {
            ::dbtools::showError(::dbtools::SQLExceptionInfo(e),VCLUnoHelper::GetInterface(GetParent()),m_xContext);
        }
        catch(Exception& )
        {
        }
    }
}

bool OTableGrantControl::isAllowed(sal_uInt16 _nColumnId,sal_Int32 _nPrivilege)
{
    bool bAllowed = false;
    switch (_nColumnId)
    {
        case COL_INSERT:
            bAllowed = (Privilege::INSERT & _nPrivilege) == Privilege::INSERT;
            break;
        case COL_DELETE:
            bAllowed = (Privilege::DELETE & _nPrivilege) == Privilege::DELETE;
            break;
        case COL_UPDATE:
            bAllowed = (Privilege::UPDATE & _nPrivilege) == Privilege::UPDATE;
            break;
        case COL_ALTER:
            bAllowed = (Privilege::ALTER & _nPrivilege) == Privilege::ALTER;
            break;
        case COL_SELECT:
            bAllowed = (Privilege::SELECT & _nPrivilege) == Privilege::SELECT;
            break;
        case COL_REF:
            bAllowed = (Privilege::REFERENCE & _nPrivilege) == Privilege::REFERENCE;
            break;
        case COL_DROP:
            bAllowed = (Privilege::DROP & _nPrivilege) == Privilege::DROP;
            break;
    }
    return bAllowed;
}

void OTableGrantControl::setUserName(const OUString& _sUserName)
{
    m_sUserName = _sUserName;
    m_aPrivMap = TTablePrivilegeMap();
}

void OTableGrantControl::setGrantUser(const Reference< XAuthorizable>& _xGrantUser)
{
    OSL_ENSURE(_xGrantUser.is(),"OTableGrantControl::setGrantUser: GrantUser is null!");
    m_xGrantUser = _xGrantUser;
}

CellController* OTableGrantControl::GetController( long nRow, sal_uInt16 nColumnId )
{

    CellController* pController = nullptr;
    switch( nColumnId )
    {
        case COL_TABLE_NAME:
            break;
        case COL_INSERT:
        case COL_DELETE:
        case COL_UPDATE:
        case COL_ALTER:
        case COL_SELECT:
        case COL_REF:
        case COL_DROP:
            {
                TTablePrivilegeMap::const_iterator aFind = findPrivilege(nRow);
                if(aFind != m_aPrivMap.end() && isAllowed(nColumnId,aFind->second.nWithGrant))
                    pController = new CheckBoxCellController( m_pCheckCell );
            }
            break;
        default:
            ;
    }
    return pController;
}

bool OTableGrantControl::SeekRow( long nRow )
{
    m_nDataPos = nRow;

    return (nRow <= m_aTableNames.getLength());
}

void OTableGrantControl::PaintCell( OutputDevice& rDev, const tools::Rectangle& rRect, sal_uInt16 nColumnId ) const
{

    if(nColumnId != COL_TABLE_NAME)
    {
        TTablePrivilegeMap::const_iterator aFind = findPrivilege(m_nDataPos);
        if(aFind != m_aPrivMap.end())
            PaintTristate(rRect, isAllowed(nColumnId,aFind->second.nRights) ? TRISTATE_TRUE : TRISTATE_FALSE,isAllowed(nColumnId,aFind->second.nWithGrant));
        else
            PaintTristate(rRect, TRISTATE_FALSE, false);
    }
    else
    {
        OUString aText(GetCellText( m_nDataPos, nColumnId ));
        Point aPos( rRect.TopLeft() );
        sal_Int32 nWidth = GetDataWindow().GetTextWidth( aText );
        sal_Int32 nHeight = GetDataWindow().GetTextHeight();

        if( aPos.X() < rRect.Left() || aPos.X() + nWidth > rRect.Right() ||
            aPos.Y() < rRect.Top() || aPos.Y() + nHeight > rRect.Bottom() )
        {
            rDev.SetClipRegion(vcl::Region(rRect));
        }

        rDev.DrawText( aPos, aText );
    }

    if( rDev.IsClipRegion() )
        rDev.SetClipRegion();
}

void OTableGrantControl::CellModified()
{
    EditBrowseBox::CellModified();
    SaveModified();
}

OTableGrantControl::TTablePrivilegeMap::const_iterator OTableGrantControl::findPrivilege(sal_Int32 _nRow) const
{
    TTablePrivilegeMap::const_iterator aFind = m_aPrivMap.find(m_aTableNames[_nRow]);
    if(aFind == m_aPrivMap.end())
    {
        fillPrivilege(_nRow);
        aFind = m_aPrivMap.find(m_aTableNames[_nRow]);
    }
    return aFind;
}

Reference< XAccessible > OTableGrantControl::CreateAccessibleCell( sal_Int32 _nRow, sal_uInt16 _nColumnPos )
{
    sal_uInt16 nColumnId = GetColumnId( _nColumnPos );
    if(nColumnId != COL_TABLE_NAME)
    {
        TriState eState = TRISTATE_FALSE;
        TTablePrivilegeMap::const_iterator aFind = findPrivilege(_nRow);
        if(aFind != m_aPrivMap.end())
        {
            eState = isAllowed(nColumnId,aFind->second.nRights) ? TRISTATE_TRUE : TRISTATE_FALSE;
        }
        else
            eState = TRISTATE_FALSE;

        return EditBrowseBox::CreateAccessibleCheckBoxCell( _nRow, _nColumnPos,eState );
    }
    return EditBrowseBox::CreateAccessibleCell( _nRow, _nColumnPos );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
