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
#ifndef DBAUI_USERADMIN_HXX
#define DBAUI_USERADMIN_HXX

#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/group.hxx>
#include "TableGrantCtrl.hxx"
#include "adminpages.hxx"
#include <comphelper/uno3.hxx>

FORWARD_DECLARE_INTERFACE(beans,XPropertySet)
FORWARD_DECLARE_INTERFACE(sdbc,XConnection)
FORWARD_DECLARE_INTERFACE(lang,XMultiServiceFactory)

namespace dbaui
{

class OUserAdmin : public OGenericAdministrationPage
{
protected:
    FixedLine           m_FL_USER;
    FixedText           m_FT_USER;
    ListBox             m_LB_USER;
    PushButton          m_PB_NEWUSER;
    PushButton          m_PB_CHANGEPWD;
    PushButton          m_PB_DELETEUSER;
    FixedLine           m_FL_TABLE_GRANTS;
    OTableGrantControl  m_TableCtrl; // show the grant rights of one user

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>          m_xConnection;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xUsers;
    ::com::sun::star::uno::Sequence< ::rtl::OUString>                               m_aUserNames;

    String              m_UserName;

    // methods
    DECL_LINK( ListDblClickHdl, ListBox * );
    DECL_LINK( CloseHdl,        PushButton * );
    DECL_LINK( UserHdl,         PushButton * );

    void        FillUserNames();

    OUserAdmin( Window* pParent, const SfxItemSet& _rCoreAttrs);
public:
    static  SfxTabPage* Create( Window* pParent, const SfxItemSet& _rAttrSet );

    ~OUserAdmin();
    String GetUser();

    // must be overloaded by subclasses, but it isn't pure virtual
    virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);

    // <method>OGenericAdministrationPage::fillControls</method>
    virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);

    // <method>OGenericAdministrationPage::fillWindows</method>
    virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);
};
}
#endif // DBAUI_USERADMIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
