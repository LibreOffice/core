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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEGRANTCTRL_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEGRANTCTRL_HXX

#include <sal/config.h>

#include <map>

#include <svtools/editbrowsebox.hxx>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XAuthorizable.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

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

    typedef std::map<OUString, TPrivileges> TTablePrivilegeMap;

    css::uno::Reference< css::container::XNameAccess >    m_xUsers;
    css::uno::Reference< css::container::XNameAccess >    m_xTables;
    css::uno::Reference< css::uno::XComponentContext>     m_xContext;
    css::uno::Reference< css::sdbcx::XAuthorizable>       m_xGrantUser;
    css::uno::Sequence< OUString>                         m_aTableNames;

    mutable TTablePrivilegeMap  m_aPrivMap;
    OUString                    m_sUserName;
    VclPtr< ::svt::CheckBoxControl>     m_pCheckCell;
    VclPtr<Edit>                        m_pEdit;
    long                        m_nDataPos;
    ImplSVEvent *               m_nDeactivateEvent;

public:
    OTableGrantControl( vcl::Window* pParent, WinBits nBits);
    virtual ~OTableGrantControl() override;
    virtual void dispose() override;
    void UpdateTables();
    void setUserName(const OUString& _sUserName);
    void setGrantUser(const css::uno::Reference< css::sdbcx::XAuthorizable>& _xGrantUser);

    void setTablesSupplier(const css::uno::Reference< css::sdbcx::XTablesSupplier >& _xTablesSup);
    void setComponentContext(const css::uno::Reference< css::uno::XComponentContext>& _rxContext);

    virtual void Init() override;

    // IAccessibleTableProvider
    /** Creates the accessible object of a data table cell.
        @param nRow  The row index of the cell.
        @param nColumnId  The column ID of the cell.
        @return  The XAccessible interface of the specified cell. */
    virtual css::uno::Reference<
        css::accessibility::XAccessible >
    CreateAccessibleCell( sal_Int32 nRow, sal_uInt16 nColumnId ) override;

protected:
    virtual bool PreNotify(NotifyEvent& rNEvt ) override;

    virtual bool IsTabAllowed(bool bForward) const override;
    virtual void InitController( ::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol ) override;
    virtual ::svt::CellController* GetController( long nRow, sal_uInt16 nCol ) override;
    virtual void PaintCell( OutputDevice& rDev, const tools::Rectangle& rRect, sal_uInt16 nColId ) const override;
    virtual bool SeekRow( long nRow ) override;
    virtual bool SaveModified() override;
    virtual OUString GetCellText( long nRow, sal_uInt16 nColId ) const override;

    virtual void CellModified() override;

private:
    DECL_LINK( AsynchActivate, void*, void );
    DECL_LINK( AsynchDeactivate, void*, void );

    static bool isAllowed(sal_uInt16 _nColumnId,sal_Int32 _nPrivilege);
    void        fillPrivilege(sal_Int32 _nRow) const;
    TTablePrivilegeMap::const_iterator findPrivilege(sal_Int32 _nRow) const;
};

}

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEGRANTCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
