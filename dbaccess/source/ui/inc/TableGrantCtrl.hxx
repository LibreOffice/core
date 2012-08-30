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
