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
#pragma once

#include <TableGrantCtrl.hxx>
#include "adminpages.hxx"

namespace com::sun::star {
    namespace sdbc {
        class XConnection;
    }
}

namespace dbaui
{

class OUserAdmin final : public OGenericAdministrationPage
{
    std::unique_ptr<weld::ComboBox> m_xUSER;
    std::unique_ptr<weld::Button> m_xNEWUSER;
    std::unique_ptr<weld::Button> m_xCHANGEPWD;
    std::unique_ptr<weld::Button> m_xDELETEUSER;
    std::unique_ptr<weld::Container> m_xTable;
    css::uno::Reference<css::awt::XWindow> m_xTableCtrlParent;
    VclPtr<OTableGrantControl> m_xTableCtrl; // show the grant rights of one user

    css::uno::Reference< css::sdbc::XConnection>          m_xConnection;
    css::uno::Reference< css::container::XNameAccess >    m_xUsers;
    css::uno::Sequence< OUString>                         m_aUserNames;

    OUString            m_UserName;

    // methods
    DECL_LINK(ListDblClickHdl, weld::ComboBox&, void);
    DECL_LINK(UserHdl, weld::Button&, void);

    void        FillUserNames();

public:
    OUserAdmin(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rCoreAttrs);
    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet);
    virtual ~OUserAdmin() override;

    OUString GetUser() const;

    // subclasses must override this, but it isn't pure virtual
    virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;

    // <method>OGenericAdministrationPage::fillControls</method>
    virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;

    // <method>OGenericAdministrationPage::fillWindows</method>
    virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
