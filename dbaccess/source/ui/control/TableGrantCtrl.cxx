/*************************************************************************
 *
 *  $RCSfile: TableGrantCtrl.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-06-20 06:58:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
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
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace dbaui;

const USHORT COL_TABLE_NAME = 1;
const USHORT COL_SELECT     = 2;
const USHORT COL_INSERT     = 3;
const USHORT COL_DELETE     = 4;
const USHORT COL_UPDATE     = 5;
const USHORT COL_ALTER      = 6;
const USHORT COL_REF        = 7;
const USHORT COL_DROP       = 8;

DBG_NAME(OTableGrantControl);
//================================================================================
// OTableGrantControl
//================================================================================
OTableGrantControl::OTableGrantControl( Window* pParent,const ResId& _RsId)
    :DbBrowseBox( pParent,_RsId, DBBF_NOROWPICTURE)
    ,m_pCheckCell( NULL )
    ,m_pEdit( NULL )
    ,m_nDeActivateEvent(0)
    ,m_bEnable(TRUE)
{
    DBG_CTOR(OTableGrantControl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Spalten einfuegen
    USHORT i=1;
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
    if (m_nDeActivateEvent)
    {
        Application::RemoveUserEvent(m_nDeActivateEvent);
        m_nDeActivateEvent = 0;
    }

    delete m_pCheckCell;
    delete m_pEdit;

    m_xTables       = NULL;

    DBG_DTOR(OTableGrantControl,NULL);
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
    DbBrowseBox::Init();

    //////////////////////////////////////////////////////////////////////
    // ComboBox instanzieren
    if(!m_pCheckCell)
    {
        m_pCheckCell    = new DbCheckBoxCtrl( &GetDataWindow() );
        m_pCheckCell->GetBox().EnableTriState(FALSE);

        m_pEdit         = new Edit( &GetDataWindow() );
        m_pEdit->SetReadOnly();
        m_pEdit->Enable(FALSE);
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
    DbBrowseBox::Resize();
}

//------------------------------------------------------------------------------
long OTableGrantControl::PreParentNotify(NotifyEvent& rNEvt)
{
    DBG_CHKTHIS(OTableGrantControl,NULL);
    if (rNEvt.GetType() == EVENT_LOSEFOCUS)
        if (!HasChildPathFocus())
        {
            if (m_nDeActivateEvent)
                Application::RemoveUserEvent(m_nDeActivateEvent);
            m_nDeActivateEvent = Application::PostUserEvent(LINK(this, OTableGrantControl, AsynchDeactivate));
        }
    if (rNEvt.GetType() == EVENT_GETFOCUS)
    {
        if (m_nDeActivateEvent)
            Application::RemoveUserEvent(m_nDeActivateEvent);
        m_nDeActivateEvent = Application::PostUserEvent(LINK(this, OTableGrantControl, AsynchActivate));
    }
    return DbBrowseBox::PreParentNotify(rNEvt);
}

//------------------------------------------------------------------------------
IMPL_LINK(OTableGrantControl, AsynchActivate, void*, EMPTYARG)
{
    m_nDeActivateEvent = 0;
    ActivateCell();
    return 0L;
}

//------------------------------------------------------------------------------
IMPL_LINK(OTableGrantControl, AsynchDeactivate, void*, EMPTYARG)
{
    m_nDeActivateEvent = 0;
    DeactivateCell();
    return 0L;
}

//------------------------------------------------------------------------------
BOOL OTableGrantControl::IsTabAllowed(BOOL bForward) const
{
    DBG_CHKTHIS(OTableGrantControl,NULL);
    long nRow = GetCurRow();
    USHORT nCol = GetCurColumnId();

    if (bForward && (nCol == 2) && (nRow == GetRowCount() - 1))
        return FALSE;

    if (!bForward && (nCol == 1) && (nRow == 0))
        return FALSE;

    return DbBrowseBox::IsTabAllowed(bForward);
}
//------------------------------------------------------------------------------
#define GRANT_REVOKE_RIGHT(what)                \
    if(m_pCheckCell->GetBox().IsChecked())      \
        xAuth->grantPrivileges(sTableName,PrivilegeObject::TABLE,what);\
    else                                        \
        xAuth->revokePrivileges(sTableName,PrivilegeObject::TABLE,what)

//------------------------------------------------------------------------------
BOOL OTableGrantControl::SaveModified()
{
    DBG_CHKTHIS(OTableGrantControl,NULL);

    sal_Int32 nRow = m_nDataPos;
    if(nRow == -1 || nRow >= m_aTableNames.getLength())
        return FALSE;

    ::rtl::OUString sTableName = m_aTableNames[m_nDataPos];
    BOOL bErg = TRUE;
    try
    {
        Reference<XAuthorizable> xAuth;
        if(m_xUsers->hasByName(m_sUserName))
        {
            m_xUsers->getByName(m_sUserName) >>= xAuth;
            if(xAuth.is())
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
            }
        }
    }
    catch(SQLException& e)
    {
        bErg = FALSE;
        ::dbaui::showError(::dbtools::SQLExceptionInfo(e),GetParent(),m_xORB);
    }
    if(bErg && Controller().Is())
        Controller()->ClearModified();
    if(!bErg)
        UpdateTables();

    return bErg;
}

//------------------------------------------------------------------------------
String OTableGrantControl::GetCellText( long nRow, USHORT nColId )
{
    DBG_CHKTHIS(OTableGrantControl,NULL);
    if(COL_TABLE_NAME == nColId)
        return m_aTableNames[nRow];

    sal_Int32 nPriv = 0;
    TTablePrivilegeMap::const_iterator aFind = m_aPrivMap.find(m_aTableNames[nRow]);
    if(aFind == m_aPrivMap.end())
        nPriv = fillPrivilege(nRow);
    else
        nPriv = aFind->second;

    return String::CreateFromInt32(isAllowed(nColId,nPriv) ? 1 :0);
}

//------------------------------------------------------------------------------
void OTableGrantControl::InitController( DbCellControllerRef& rController, long nRow, USHORT nColumnId )
{
    DBG_CHKTHIS(OTableGrantControl,NULL);
    String sTablename = m_aTableNames[nRow];
    // special case for tablename
    if(nColumnId == COL_TABLE_NAME)
        m_pEdit->SetText(sTablename);
    else
    {
        // get the privileges from the user
        TTablePrivilegeMap::const_iterator aFind = m_aPrivMap.find(sTablename);
        if(aFind == m_aPrivMap.end())
            m_pCheckCell->GetBox().Check(isAllowed(nColumnId,fillPrivilege(nRow)));
        else
            m_pCheckCell->GetBox().Check(isAllowed(nColumnId,aFind->second));
    }
}
// -----------------------------------------------------------------------------
sal_Int32 OTableGrantControl::fillPrivilege(sal_Int32 _nRow)
{
    sal_Int32 nPrivs = 0;
    Reference<XAuthorizable> xAuth;
    if(m_xUsers->hasByName(m_sUserName))
    {
        try
        {
            m_xUsers->getByName(m_sUserName) >>= xAuth;
            if(xAuth.is())
            {
                nPrivs = xAuth->getPrivileges(m_aTableNames[_nRow],PrivilegeObject::TABLE);
                m_aPrivMap[m_aTableNames[_nRow]] = nPrivs;
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
    return nPrivs;
}
// -----------------------------------------------------------------------------
sal_Bool OTableGrantControl::isAllowed(USHORT _nColumnId,sal_Int32 _nPrivilege) const
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
//------------------------------------------------------------------------------
DbCellController* OTableGrantControl::GetController( long nRow, USHORT nColumnId )
{
    DBG_CHKTHIS(OTableGrantControl,NULL);

    switch( nColumnId )
    {
        case COL_TABLE_NAME:
            return NULL;//new DbEditCellController( m_pEdit );
            break;
        case COL_INSERT:
        case COL_DELETE:
        case COL_UPDATE:
        case COL_ALTER:
        case COL_SELECT:
        case COL_REF:
        case COL_DROP:
            {
                if(!m_bEnable)
                    return NULL;
                return new DbCheckBoxCellController( m_pCheckCell );
            }
            break;
        default:
            return NULL;
    }
}
//------------------------------------------------------------------------------
BOOL OTableGrantControl::SeekRow( long nRow )
{
    DBG_CHKTHIS(OTableGrantControl,NULL);
    m_nDataPos = nRow;

    return (nRow <= m_aTableNames.getLength());
}

//------------------------------------------------------------------------------
void OTableGrantControl::PaintCell( OutputDevice& rDev, const Rectangle& rRect, USHORT nColumnId ) const
{
    DBG_CHKTHIS(OTableGrantControl,NULL);

    String aText(((OTableGrantControl*)this)->GetCellText( m_nDataPos, nColumnId ));
    if(nColumnId != COL_TABLE_NAME)
        PaintTristate(rDev, rRect, aText.ToInt32() == 1 ? STATE_CHECK : STATE_NOCHECK,m_bEnable);
    else
    {
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
    DbBrowseBox::CellModified();
    SaveModified();
}
// -----------------------------------------------------------------------------



