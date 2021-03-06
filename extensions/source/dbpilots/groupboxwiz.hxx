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

    struct OOptionGroupSettings : public OControlWizardSettings
    {
        std::vector<OUString>     aLabels;
        std::vector<OUString>     aValues;
        OUString        sDefaultField;
        OUString        sDBField;
    };

    class OGroupBoxWizard final : public OControlWizard
    {
        OOptionGroupSettings        m_aSettings;

        bool        m_bVisitedDefault   : 1;
        bool        m_bVisitedDB        : 1;

    public:
        OGroupBoxWizard(
            weld::Window* _pParent,
            const css::uno::Reference< css::beans::XPropertySet >& _rxObjectModel,
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext
        );

        OOptionGroupSettings& getSettings() { return m_aSettings; }

    private:
        // OWizardMachine overridables
        virtual std::unique_ptr<BuilderPage> createPage( WizardState _nState ) override;
        virtual WizardState         determineNextState( WizardState _nCurrentState ) const override;
        virtual void                enterState( WizardState _nState ) override;
        virtual bool                onFinish() override;

        virtual bool                approveControl(sal_Int16 _nClassId) override;
    };

    class OGBWPage : public OControlWizardPage
    {
    public:
        OGBWPage(weld::Container* pPage, OControlWizard* pWizard, const OUString& rUIXMLDescription, const OString& rID)
            : OControlWizardPage(pPage, pWizard, rUIXMLDescription, rID)
    {
    }

    protected:
        OOptionGroupSettings& getSettings() { return static_cast<OGroupBoxWizard*>(getDialog())->getSettings(); }
    };

    class ORadioSelectionPage final : public OGBWPage
    {
        std::unique_ptr<weld::Entry> m_xRadioName;
        std::unique_ptr<weld::Button> m_xMoveRight;
        std::unique_ptr<weld::Button> m_xMoveLeft;
        std::unique_ptr<weld::TreeView> m_xExistingRadios;

    public:
        explicit ORadioSelectionPage(weld::Container* pPage, OControlWizard* pWizard);
        virtual ~ORadioSelectionPage() override;

    private:
        // BuilderPage overridables
        void Activate() override;

        // OWizardPage overridables
        virtual void        initializePage() override;
        virtual bool        commitPage( ::vcl::WizardTypes::CommitPageReason _eReason ) override;
        virtual bool        canAdvance() const override;

        DECL_LINK( OnMoveEntry, weld::Button&, void );
        DECL_LINK( OnEntrySelected, weld::TreeView&, void );
        DECL_LINK( OnNameModified, weld::Entry&, void );

        void implCheckMoveButtons();
    };

    class ODefaultFieldSelectionPage final : public OMaybeListSelectionPage
    {
        std::unique_ptr<weld::RadioButton> m_xDefSelYes;
        std::unique_ptr<weld::RadioButton> m_xDefSelNo;
        std::unique_ptr<weld::ComboBox> m_xDefSelection;

    public:
        explicit ODefaultFieldSelectionPage(weld::Container* pPage, OControlWizard* pWizard);
        virtual ~ODefaultFieldSelectionPage() override;

    private:
        // OWizardPage overridables
        virtual void        initializePage() override;
        virtual bool        commitPage( ::vcl::WizardTypes::CommitPageReason _eReason ) override;

        OOptionGroupSettings& getSettings() { return static_cast<OGroupBoxWizard*>(getDialog())->getSettings(); }
    };

    class OOptionValuesPage final : public OGBWPage
    {
        std::unique_ptr<weld::Entry> m_xValue;
        std::unique_ptr<weld::TreeView> m_xOptions;

        std::vector<OUString>   m_aUncommittedValues;
        ::vcl::WizardTypes::WizardState
                        m_nLastSelection;

    public:
        explicit OOptionValuesPage(weld::Container* pPage, OControlWizard* pWizard);
        virtual ~OOptionValuesPage() override;

    private:
        // BuilderPage overridables
        void Activate() override;

        // OWizardPage overridables
        virtual void        initializePage() override;
        virtual bool        commitPage( ::vcl::WizardTypes::CommitPageReason _eReason ) override;

        void implTraveledOptions();

        DECL_LINK( OnOptionSelected, weld::TreeView&, void );
    };

    class OOptionDBFieldPage : public ODBFieldPage
    {
    public:
        explicit OOptionDBFieldPage(weld::Container* pPage, OControlWizard* pWizard);

    protected:
        // ODBFieldPage overridables
        virtual OUString& getDBFieldSetting() override;
    };

    class OFinalizeGBWPage final : public OGBWPage
    {
        std::unique_ptr<weld::Entry> m_xName;

    public:
        explicit OFinalizeGBWPage(weld::Container* pPage, OControlWizard* pWizard);
        virtual ~OFinalizeGBWPage() override;

    private:
        // BuilderPage overridables
        void Activate() override;

        // OWizardPage overridables
        virtual void        initializePage() override;
        virtual bool        commitPage( ::vcl::WizardTypes::CommitPageReason _eReason ) override;
        virtual bool        canAdvance() const override;
    };


}   // namespace dbp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
