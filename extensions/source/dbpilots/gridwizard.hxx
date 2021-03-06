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

#include "controlwizard.hxx"
#include "commonpagesdbp.hxx"

using vcl::WizardTypes::WizardState;
using vcl::WizardTypes::CommitPageReason;

namespace dbp
{
    struct OGridSettings : public OControlWizardSettings
    {
        css::uno::Sequence< OUString >      aSelectedFields;
    };

    class OGridWizard final : public OControlWizard
    {
        OGridSettings   m_aSettings;
        bool        m_bHadDataSelection : 1;

    public:
        OGridWizard(weld::Window* _pParent,
            const css::uno::Reference< css::beans::XPropertySet >& _rxObjectModel,
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext);

        OGridSettings& getSettings() { return m_aSettings; }

    private:
        // OWizardMachine overridables
        virtual std::unique_ptr<BuilderPage> createPage( WizardState _nState ) override;
        virtual WizardState         determineNextState( WizardState _nCurrentState ) const override;
        virtual void                enterState( WizardState _nState ) override;
        virtual bool                leaveState( WizardState _nState ) override;
        virtual bool                onFinish() override;

        virtual bool                approveControl(sal_Int16 _nClassId) override;

        void implApplySettings();
    };

    class OGridPage : public OControlWizardPage
    {
    public:
        OGridPage(weld::Container* pPage, OGridWizard* pWizard, const OUString& rUIXMLDescription, const OString& rID)
            : OControlWizardPage(pPage, pWizard, rUIXMLDescription, rID)
        {
        }
    protected:
        OGridSettings& getSettings() { return static_cast<OGridWizard*>(getDialog())->getSettings(); }
    };

    class OGridFieldsSelection final : public OGridPage
    {
        std::unique_ptr<weld::TreeView> m_xExistFields;
        std::unique_ptr<weld::Button> m_xSelectOne;
        std::unique_ptr<weld::Button> m_xSelectAll;
        std::unique_ptr<weld::Button> m_xDeselectOne;
        std::unique_ptr<weld::Button> m_xDeselectAll;
        std::unique_ptr<weld::TreeView> m_xSelFields;

    public:
        explicit OGridFieldsSelection(weld::Container* pPage, OGridWizard* pWizard);
        virtual ~OGridFieldsSelection() override;

    private:
        // BuilderPage overridables
        virtual void Activate() override;

        // OWizardPage overridables
        virtual void        initializePage() override;
        virtual bool        commitPage( ::vcl::WizardTypes::CommitPageReason _eReason ) override;
        virtual bool        canAdvance() const override;

        DECL_LINK(OnMoveOneEntry, weld::Button&, void);
        DECL_LINK(OnMoveAllEntries, weld::Button&, void);
        DECL_LINK(OnEntrySelected, weld::TreeView&, void);
        DECL_LINK(OnEntryDoubleClicked, weld::TreeView&, bool);

        void implCheckButtons();
    };
}   // namespace dbp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
