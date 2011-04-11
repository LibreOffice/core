/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef DBAUI_TABLEGRANTCONTROL_HXX
#define DBAUI_TABLEGRANTCONTROL_HXX

#include <svtools/editbrowsebox.hxx>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XAuthorizable.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/stl_types.hxx>
#include "moduledbu.hxx"

class Edit;
namespace dbaui
{

class OTableGrantControl : public ::svt::EditBrowseBox
{
    typedef struct
    {
        sal_Int32 nRights;
        sal_Int32 nWithGrant;
    } TPrivileges;

    DECLARE_STL_USTRINGACCESS_MAP(TPrivileges,TTablePrivilegeMap);

    OModuleClient        m_aModuleClient;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xUsers;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xTables;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> m_xORB;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XAuthorizable>       m_xGrantUser;
    ::com::sun::star::uno::Sequence< ::rtl::OUString>                               m_aTableNames;

    mutable TTablePrivilegeMap  m_aPrivMap;
    ::rtl::OUString             m_sUserName;
    ::svt::CheckBoxControl*     m_pCheckCell;
    Edit*                       m_pEdit;
    long                        m_nDataPos;
    sal_Bool                        m_bEnable;
    sal_uLong                       m_nDeactivateEvent;

public:
    OTableGrantControl( Window* pParent,const ResId& _RsId);
    virtual ~OTableGrantControl();
    void UpdateTables();
    void setUserName(const ::rtl::OUString _sUserName);
    void setGrantUser(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XAuthorizable>& _xGrantUser);

    void setTablesSupplier(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier >& _xTablesSup);
    void setORB(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _xORB);

    virtual void Init();

    // IAccessibleTableProvider
    /** Creates the accessible object of a data table cell.
        @param nRow  The row index of the cell.
        @param nColumnId  The column ID of the cell.
        @return  The XAccessible interface of the specified cell. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >
    CreateAccessibleCell( sal_Int32 nRow, sal_uInt16 nColumnId );

protected:
    virtual void Resize();

    virtual long PreNotify(NotifyEvent& rNEvt );

    virtual sal_Bool IsTabAllowed(sal_Bool bForward) const;
    virtual void InitController( ::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol );
    virtual ::svt::CellController* GetController( long nRow, sal_uInt16 nCol );
    virtual void PaintCell( OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColId ) const;
    virtual sal_Bool SeekRow( long nRow );
    virtual sal_Bool SaveModified();
    virtual String GetCellText( long nRow, sal_uInt16 nColId ) const;

    virtual void CellModified();

private:
    DECL_LINK( AsynchActivate, void* );
    DECL_LINK( AsynchDeactivate, void* );

    sal_Bool    isAllowed(sal_uInt16 _nColumnId,sal_Int32 _nPrivilege) const;
    void        fillPrivilege(sal_Int32 _nRow) const;
    TTablePrivilegeMap::const_iterator findPrivilege(sal_Int32 _nRow) const;
};

}

#endif // DBAUI_TABLEGRANTCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
