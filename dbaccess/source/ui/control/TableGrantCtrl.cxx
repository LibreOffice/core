/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#ifndef DBAUI_TABLEGRANTCONTROL_HXX
#include "TableGrantCtrl.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGE_HPP_
#include <com/sun/star/sdbcx/Privilege.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGEOBJECT_HPP_
#include <com/sun/star/sdbcx/PrivilegeObject.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XUSERSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XUsersSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XAUTHORIZABLE_HPP_
#include <com/sun/star/sdbcx/XAuthorizable.hpp>
#endif
#ifndef _DBU_CONTROL_HRC_
#include "dbu_control.hrc"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif

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

DBG_NAME(OTableGrantControl)

//================================================================================
// OTableGrantControl
//================================================================================
OTableGrantControl::OTableGrantControl( Window* pParent,const ResId& _RsId)
    :EditBrowseBox( pParent,_RsId, EBBF_SMART_TAB_TRAVEL | EBBF_NOROWPICTURE )
    ,m_pCheckCell( NULL )
    ,m_pEdit( NULL )
    ,m_nDataPos( 0 )
    ,m_bEnable(sal_True)
    ,m_nDeactivateEvent(0)
{
    DBG_CTOR(OTableGrantControl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Spalten einfuegen
    sal_uInt16 i=1;
    InsertDataColumn( i, String(ModuleRes(STR_TABLE_PRIV_NAME)  ), 75);
    FreezeColumn(i++);
    InsertDataColumn( i++, String(ModuleRes(STR_TABLE_PRIV_SELECT)), 75);
    InsertDataColumn( i++, String(ModuleRes(STR_TABLE_PRIV_INSERT)), 75);
    InsertDataColumn( i++, String(ModuleRes(STR_TABLE_PRIV_DELETE)), 75);
    InsertDataColumn( i++, String(ModuleRes(STR_TABLE_PRIV_UPDATE)), 75);
    InsertDataColumn( i++, String(ModuleRes(STR_TABLE_PRIV_ALTER)), 75);
    InsertDataColumn( i++, String(ModuleRes(STR_TABLE_PRIV_REFERENCE)), 75);
    InsertDataColumn( i++, String(ModuleRes(STR_TABLE_PRIV_DROP)), 75);

    while(--i)
        SetColumnWidth(i,GetAutoColumnWidth(i));
}

//------------------------------------------------------------------------
OTableGrantControl::~OTableGrantControl()
{
    DBG_DTOR(OTableGrantControl,NULL);
    if (m_nDeactivateEvent)
    {
        Application::RemoveUserEvent(m_nDeactivateEvent);
        m_nDeactivateEvent = 0;
    }

    delete m_pCheckCell;
    delete m_pEdit;

    m_xTables       = NULL;
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
void OTableGrantControl::setORB(const Reference< ::com::sun::star::lang::XMultiServiceFactory>& _xORB)
{
    m_xORB = _xORB;
}
//------------------------------------------------------------------------
void OTableGrantControl::UpdateTables()
{
    RemoveRows();

    if(m_xTables.is())
        RowInserted(0, m_aTableNames.getLength());
    //  m_bEnable = m_xDb->GetUser() != ((OUserAdmin*)GetParent())->GetUser();
}
//------------------------------------------------------------------------
void OTableGrantControl::Init()
{
    DBG_CHKTHIS(OTableGrantControl,NULL);
    EditBrowseBox::Init();

    //////////////////////////////////////////////////////////////////////
    // ComboBox instanzieren
    if(!m_pCheckCell)
    {
        m_pCheckCell    = new CheckBoxControl( &GetDataWindow() );
        m_pCheckCell->GetBox().EnableTriState(sal_False);

        m_pEdit         = new Edit( &GetDataWindow() );
        m_pEdit->SetReadOnly();
        m_pEdit->Enable(sal_False);
    }

    UpdateTables();
    //////////////////////////////////////////////////////////////////////
    // Browser Mode setzen
    BrowserMode nMode = BROWSER_COLUMNSELECTION | BROWSER_HLINESFULL | BROWSER_VLINESFULL |
                        BROWSER_HIDECURSOR      | BROWSER_HIDESELECT;

    SetMode(nMode);
}

//------------------------------------------------------------------------------
void OTableGrantControl::Resize()
{
    DBG_CHKTHIS(OTableGrantControl,NULL);
    EditBrowseBox::Resize();
}

//------------------------------------------------------------------------------
long OTableGrantControl::PreNotify(NotifyEvent& rNEvt)
{
    DBG_CHKTHIS(OTableGrantControl,NULL);
    if (rNEvt.GetType() == EVENT_LOSEFOCUS)
        if (!HasChildPathFocus())
        {
            if (m_nDeactivateEvent)
                Application::RemoveUserEvent(m_nDeactivateEvent);
            m_nDeactivateEvent = Application::PostUserEvent(LINK(this, OTableGrantControl, AsynchDeactivate));
        }
    if (rNEvt.GetType() == EVENT_GETFOCUS)
    {
        if (m_nDeactivateEvent)
            Application::RemoveUserEvent(m_nDeactivateEvent);
        m_nDeactivateEvent = Application::PostUserEvent(LINK(this, OTableGrantControl, AsynchActivate));
    }
    return EditBrowseBox::PreNotify(rNEvt);
}

//------------------------------------------------------------------------------
IMPL_LINK(OTableGrantControl, AsynchActivate, void*, EMPTYARG)
{
    m_nDeactivateEvent = 0;
    ActivateCell();
    return 0L;
}

//------------------------------------------------------------------------------
IMPL_LINK(OTableGrantControl, AsynchDeactivate, void*, EMPTYARG)
{
    m_nDeactivateEvent = 0;
    DeactivateCell();
    return 0L;
}

//------------------------------------------------------------------------------
sal_Bool OTableGrantControl::IsTabAllowed(sal_Bool bForward) const
{
    DBG_CHKTHIS(OTableGrantControl,NULL);
    long nRow = GetCurRow();
    sal_uInt16 nCol = GetCurColumnId();

    if (bForward && (nCol == 2) && (nRow == GetRowCount() - 1))
        return sal_False;

    if (!bForward && (nCol == 1) && (nRow == 0))
        return sal_False;

    return EditBrowseBox::IsTabAllowed(bForward);
}
//------------------------------------------------------------------------------
#define GRANT_REVOKE_RIGHT(what)                \
    if(m_pCheckCell->GetBox().IsChecked())      \
        xAuth->grantPrivileges(sTableName,PrivilegeObject::TABLE,what);\
    else                                        \
        xAuth->revokePrivileges(sTableName,PrivilegeObject::TABLE,what)

//------------------------------------------------------------------------------
sal_Bool OTableGrantControl::SaveModified()
{
    DBG_CHKTHIS(OTableGrantControl,NULL);

    sal_Int32 nRow = GetCurRow();
    if(nRow == -1 || nRow >= m_aTableNames.getLength())
        return sal_False;

    ::rtl::OUString sTableName = m_aTableNames[nRow];
    sal_Bool bErg = sal_True;
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
        bErg = sal_False;
        ::dbaui::showError(::dbtools::SQLExceptionInfo(e),GetParent(),m_xORB);
    }
    if(bErg && Controller().Is())
        Controller()->ClearModified();
    if(!bErg)
        UpdateTables();

    return bErg;
}

//------------------------------------------------------------------------------
String OTableGrantControl::GetCellText( long nRow, sal_uInt16 nColId ) const
{
    DBG_CHKTHIS(OTableGrantControl,NULL);
    if(COL_TABLE_NAME == nColId)
        return m_aTableNames[nRow];

    sal_Int32 nPriv = 0;
    TTablePrivilegeMap::const_iterator aFind = findPrivilege(nRow);
    if(aFind != m_aPrivMap.end())
        nPriv = aFind->second.nRights;

    return String::CreateFromInt32(isAllowed(nColId,nPriv) ? 1 :0);
}

//------------------------------------------------------------------------------
void OTableGrantControl::InitController( CellControllerRef& /*rController*/, long nRow, sal_uInt16 nColumnId )
{
    DBG_CHKTHIS(OTableGrantControl,NULL);
    String sTablename = m_aTableNames[nRow];
    // special case for tablename
    if(nColumnId == COL_TABLE_NAME)
        m_pEdit->SetText(sTablename);
    else
    {
        // get the privileges from the user
        TTablePrivilegeMap::const_iterator aFind = findPrivilege(nRow);
        m_pCheckCell->GetBox().Check(aFind != m_aPrivMap.end() ? isAllowed(nColumnId,aFind->second.nRights) : sal_False);
    }
}
// -----------------------------------------------------------------------------
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
            ::dbaui::showError(::dbtools::SQLExceptionInfo(e),GetParent(),m_xORB);
        }
        catch(Exception& )
        {
        }
    }
}
// -----------------------------------------------------------------------------
sal_Bool OTableGrantControl::isAllowed(sal_uInt16 _nColumnId,sal_Int32 _nPrivilege) const
{
    sal_Bool bAllowed = sal_False;
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
// -----------------------------------------------------------------------------
void OTableGrantControl::setUserName(const ::rtl::OUString _sUserName)
{
    m_sUserName = _sUserName;
    m_aPrivMap = TTablePrivilegeMap();
}
// -----------------------------------------------------------------------------
void OTableGrantControl::setGrantUser(const Reference< XAuthorizable>& _xGrantUser)
{
    OSL_ENSURE(_xGrantUser.is(),"OTableGrantControl::setGrantUser: GrantUser is null!");
    m_xGrantUser = _xGrantUser;
}
//------------------------------------------------------------------------------
CellController* OTableGrantControl::GetController( long nRow, sal_uInt16 nColumnId )
{
    DBG_CHKTHIS(OTableGrantControl,NULL);

    CellController* pController = NULL;
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
//------------------------------------------------------------------------------
sal_Bool OTableGrantControl::SeekRow( long nRow )
{
    DBG_CHKTHIS(OTableGrantControl,NULL);
    m_nDataPos = nRow;

    return (nRow <= m_aTableNames.getLength());
}

//------------------------------------------------------------------------------
void OTableGrantControl::PaintCell( OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColumnId ) const
{
    DBG_CHKTHIS(OTableGrantControl,NULL);

    if(nColumnId != COL_TABLE_NAME)
    {
        TTablePrivilegeMap::const_iterator aFind = findPrivilege(m_nDataPos);
        if(aFind != m_aPrivMap.end())
            PaintTristate(rDev, rRect, isAllowed(nColumnId,aFind->second.nRights) ? STATE_CHECK : STATE_NOCHECK,isAllowed(nColumnId,aFind->second.nWithGrant));
        else
            PaintTristate(rDev, rRect, STATE_NOCHECK,sal_False);
    }
    else
    {
        String aText(((OTableGrantControl*)this)->GetCellText( m_nDataPos, nColumnId ));
        Point aPos( rRect.TopLeft() );
        sal_Int32 nWidth = GetDataWindow().GetTextWidth( aText );
        sal_Int32 nHeight = GetDataWindow().GetTextHeight();

        if( aPos.X() < rRect.Right() || aPos.X() + nWidth > rRect.Right() ||
            aPos.Y() < rRect.Top() || aPos.Y() + nHeight > rRect.Bottom() )
            rDev.SetClipRegion( rRect );

        rDev.DrawText( aPos, aText );
    }

    if( rDev.IsClipRegion() )
        rDev.SetClipRegion();
}

//------------------------------------------------------------------------
void OTableGrantControl::CellModified()
{
    DBG_CHKTHIS(OTableGrantControl,NULL);
    EditBrowseBox::CellModified();
    SaveModified();
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
Reference< XAccessible > OTableGrantControl::CreateAccessibleCell( sal_Int32 _nRow, sal_uInt16 _nColumnPos )
{
    sal_uInt16 nColumnId = GetColumnId( _nColumnPos );
    if(nColumnId != COL_TABLE_NAME)
    {
        TriState eState = STATE_NOCHECK;
        sal_Bool bEnable = sal_False;
        TTablePrivilegeMap::const_iterator aFind = findPrivilege(_nRow);
        if(aFind != m_aPrivMap.end())
        {
            eState = isAllowed(nColumnId,aFind->second.nRights) ? STATE_CHECK : STATE_NOCHECK;
            bEnable = isAllowed(nColumnId,aFind->second.nWithGrant);
        }
        else
            eState = STATE_NOCHECK;

        return EditBrowseBox::CreateAccessibleCheckBoxCell( _nRow, _nColumnPos,eState,bEnable );
    }
    return EditBrowseBox::CreateAccessibleCell( _nRow, _nColumnPos );
}
// -----------------------------------------------------------------------------

