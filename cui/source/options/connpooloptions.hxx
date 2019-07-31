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

#ifndef INCLUDED_CUI_SOURCE_OPTIONS_CONNPOOLOPTIONS_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_CONNPOOLOPTIONS_HXX

#include <sfx2/tabdlg.hxx>

#include "connpoolsettings.hxx"

namespace offapp
{
    class ConnectionPoolOptionsPage final : public SfxTabPage
    {
        using TabPage::ActivatePage;

        OUString m_sYes;
        OUString m_sNo;
        DriverPoolingSettings m_aSettings;
        DriverPoolingSettings m_aSavedSettings;

        std::unique_ptr<weld::CheckButton> m_xEnablePooling;
        std::unique_ptr<weld::Label> m_xDriversLabel;
        std::unique_ptr<weld::TreeView> m_xDriverList;
        std::unique_ptr<weld::Label> m_xDriverLabel;
        std::unique_ptr<weld::Label> m_xDriver;
        std::unique_ptr<weld::CheckButton> m_xDriverPoolingEnabled;
        std::unique_ptr<weld::Label> m_xTimeoutLabel;
        std::unique_ptr<weld::SpinButton> m_xTimeout;

    public:
        ConnectionPoolOptionsPage(TabPageParent _pParent, const SfxItemSet& _rAttrSet);
        virtual ~ConnectionPoolOptionsPage() override;
        static VclPtr<SfxTabPage>  Create(TabPageParent _pParent, const SfxItemSet* _rAttrSet);

    private:
        virtual bool        FillItemSet(SfxItemSet* _rSet) override;
        virtual void        Reset(const SfxItemSet* _rSet) override;
        virtual void        ActivatePage( const SfxItemSet& _rSet) override;

        void updateRow(size_t nRow);
        void updateCurrentRow();
        void UpdateDriverList(const DriverPoolingSettings& _rSettings);
        bool isModifiedDriverList() const;
        void saveDriverList() { m_aSavedSettings = m_aSettings; }

        DECL_LINK(OnEnabledDisabled, weld::Button&, void);
        DECL_LINK(OnSpinValueChanged, weld::SpinButton&, void);
        DECL_LINK(OnDriverRowChanged, weld::TreeView&, void);

        void implInitControls(const SfxItemSet& _rSet);

        void commitTimeoutField();
    };

} // namespace offapp

#endif // INCLUDED_CUI_SOURCE_OPTIONS_CONNPOOLOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
