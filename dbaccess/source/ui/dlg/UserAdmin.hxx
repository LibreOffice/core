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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_DLG_USERADMIN_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_DLG_USERADMIN_HXX

#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/group.hxx>
#include "TableGrantCtrl.hxx"
#include "adminpages.hxx"
#include <comphelper/uno3.hxx>

namespace com { namespace sun { namespace star {
    namespace sdbc {
        class XConnection;
    }
}}}

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
    ::com::sun::star::uno::Sequence< OUString>                               m_aUserNames;

    OUString            m_UserName;

    // methods
    DECL_LINK( ListDblClickHdl, ListBox * );
    DECL_LINK( CloseHdl,        PushButton * );
    DECL_LINK( UserHdl,         PushButton * );

    void        FillUserNames();

    OUserAdmin( Window* pParent, const SfxItemSet& _rCoreAttrs);
public:
    static  SfxTabPage* Create( Window* pParent, const SfxItemSet& _rAttrSet );

    virtual ~OUserAdmin();
    OUString GetUser();

    // must be overloaded by subclasses, but it isn't pure virtual
    virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue) SAL_OVERRIDE;

    // <method>OGenericAdministrationPage::fillControls</method>
    virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;

    // <method>OGenericAdministrationPage::fillWindows</method>
    virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
};
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_USERADMIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
