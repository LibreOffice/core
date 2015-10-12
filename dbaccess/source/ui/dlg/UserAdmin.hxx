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
    friend class VclPtr<OUserAdmin>;
protected:
    VclPtr<ListBox>             m_pUSER;
    VclPtr<PushButton>          m_pNEWUSER;
    VclPtr<PushButton>          m_pCHANGEPWD;
    VclPtr<PushButton>          m_pDELETEUSER;
    VclPtr<OTableGrantControl>  m_TableCtrl; // show the grant rights of one user

    css::uno::Reference< css::sdbc::XConnection>          m_xConnection;
    css::uno::Reference< css::container::XNameAccess >    m_xUsers;
    css::uno::Sequence< OUString>                         m_aUserNames;

    OUString            m_UserName;

    // methods
    DECL_LINK_TYPED( ListDblClickHdl, ListBox&, void );
    DECL_LINK_TYPED( UserHdl,   Button *, void );

    void        FillUserNames();

    OUserAdmin( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs);
public:
    static  VclPtr<SfxTabPage> Create( vcl::Window* pParent, const SfxItemSet* _rAttrSet );

    virtual ~OUserAdmin();
    virtual void dispose() override;
    OUString GetUser();

    // subclasses must override this, but it isn't pure virtual
    virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;

    // <method>OGenericAdministrationPage::fillControls</method>
    virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) override;

    // <method>OGenericAdministrationPage::fillWindows</method>
    virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) override;
};
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_USERADMIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
