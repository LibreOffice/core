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

#include <sfx2/tabdlg.hxx>
#include "IItemSetHelper.hxx"
#include <memory>

namespace com::sun::star {
    namespace sdbc {
        class XConnection;
    }
    namespace lang {
        class XMultiServiceFactory;
    }
}

namespace dbaui
{
    class ODbDataSourceAdministrationHelper;
    // OUserAdminDlg

    /** implements the user admin dialog
    */
    class OUserAdminDlg : public SfxTabDialogController, public IItemSetHelper, public IDatabaseSettingsDialog
    {
        weld::Window* m_pParent;
        std::unique_ptr<ODbDataSourceAdministrationHelper>  m_pImpl;
        SfxItemSet*   m_pItemSet;
        css::uno::Reference< css::sdbc::XConnection>          m_xConnection;
        bool          m_bOwnConnection;
    protected:
        virtual void PageCreated(const OString& rId, SfxTabPage& _rPage) override;
    public:
        OUserAdminDlg(weld::Window* pParent, SfxItemSet* pItems,
                      const css::uno::Reference< css::uno::XComponentContext >& rxORB,
                      const css::uno::Any& rDataSourceName,
                      const css::uno::Reference< css::sdbc::XConnection>& rConnection);

        virtual ~OUserAdminDlg() override;

        virtual const SfxItemSet* getOutputSet() const override;
        virtual SfxItemSet* getWriteOutputSet() override;

        virtual short   run() override;

        // forwards to ODbDataSourceAdministrationHelper
        virtual css::uno::Reference< css::uno::XComponentContext > getORB() const override;
        virtual std::pair< css::uno::Reference< css::sdbc::XConnection >,bool> createConnection() override;
        virtual css::uno::Reference< css::sdbc::XDriver > getDriver() override;
        virtual OUString getDatasourceType(const SfxItemSet& _rSet) const override;
        virtual void clearPassword() override;
        virtual void saveDatasource() override;
        virtual void setTitle(const OUString& _sTitle) override;
        virtual void enableConfirmSettings( bool _bEnable ) override;
    };
}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
