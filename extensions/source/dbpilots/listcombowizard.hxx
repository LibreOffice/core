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

#define LCW_STATE_DATASOURCE_SELECTION  0
#define LCW_STATE_TABLESELECTION        1
#define LCW_STATE_FIELDSELECTION        2
#define LCW_STATE_FIELDLINK             3
#define LCW_STATE_COMBODBFIELD          4

    struct OListComboSettings : public OControlWizardSettings
    {
        OUString          sListContentTable;
        OUString          sListContentField;
        OUString          sLinkedFormField;
        OUString          sLinkedListField;
    };

    class OListComboWizard final : public OControlWizard
    {
        OListComboSettings      m_aSettings;
        bool                m_bListBox : 1;
        bool                m_bHadDataSelection : 1;

    public:
        OListComboWizard(
            weld::Window* pParent,
            const css::uno::Reference< css::beans::XPropertySet >& _rxObjectModel,
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext
        );

        OListComboSettings& getSettings() { return m_aSettings; }

        bool isListBox() const { return m_bListBox; }

    private:
        // OWizardMachine overridables
        virtual std::unique_ptr<BuilderPage> createPage( WizardState _nState ) override;
        virtual WizardState         determineNextState( WizardState _nCurrentState ) const override;
        virtual void                enterState( WizardState _nState ) override;
        virtual bool                leaveState( WizardState _nState ) override;
        virtual bool                onFinish() override;

        virtual bool                approveControl(sal_Int16 _nClassId) override;

        WizardState getFinalState() const { return isListBox() ? LCW_STATE_FIELDLINK : LCW_STATE_COMBODBFIELD; }

        void implApplySettings();
    };

    class OLCPage : public OControlWizardPage
    {
    public:
        OLCPage(weld::Container* pPage, OListComboWizard* pWizard, const OUString& rUIXMLDescription, const OString& rID)
            : OControlWizardPage(pPage, pWizard, rUIXMLDescription, rID)
    {
    }

    protected:
        OListComboSettings& getSettings() { return static_cast<OListComboWizard*>(getDialog())->getSettings(); }
        bool isListBox() { return static_cast<OListComboWizard*>(getDialog())->isListBox(); }

    protected:
        css::uno::Reference< css::container::XNameAccess >  getTables() const;
        css::uno::Sequence< OUString >                      getTableFields();
    };

    class OContentTableSelection final : public OLCPage
    {
        std::unique_ptr<weld::TreeView> m_xSelectTable;

    public:
        explicit OContentTableSelection(weld::Container* pPage, OListComboWizard* pWizard);
        virtual ~OContentTableSelection() override;

    private:
        // BuilderPage overridables
        virtual void Activate() override;

        // OWizardPage overridables
        virtual void        initializePage() override;
        virtual bool        commitPage( ::vcl::WizardTypes::CommitPageReason _eReason ) override;
        virtual bool        canAdvance() const override;

        DECL_LINK( OnTableDoubleClicked, weld::TreeView&, bool );
        DECL_LINK( OnTableSelected, weld::TreeView&, void );
    };

    class OContentFieldSelection final : public OLCPage
    {
        std::unique_ptr<weld::TreeView> m_xSelectTableField;
        std::unique_ptr<weld::Entry> m_xDisplayedField;
        std::unique_ptr<weld::Label> m_xInfo;

    public:
        explicit OContentFieldSelection(weld::Container* pPage, OListComboWizard* pWizard);
        virtual ~OContentFieldSelection() override;

    private:
        DECL_LINK( OnFieldSelected, weld::TreeView&, void );
        DECL_LINK( OnTableDoubleClicked, weld::TreeView&, bool );

        // OWizardPage overridables
        virtual void        initializePage() override;
        virtual bool        commitPage( ::vcl::WizardTypes::CommitPageReason _eReason ) override;
        virtual bool        canAdvance() const override;
    };

    class OLinkFieldsPage final : public OLCPage
    {
        std::unique_ptr<weld::ComboBox> m_xValueListField;
        std::unique_ptr<weld::ComboBox> m_xTableField;

    public:
        explicit OLinkFieldsPage(weld::Container* pPage, OListComboWizard* pWizard);
        virtual ~OLinkFieldsPage() override;

    private:
        // BuilderPage overridables
        virtual void Activate() override;

        // OWizardPage overridables
        virtual void        initializePage() override;
        virtual bool        commitPage( ::vcl::WizardTypes::CommitPageReason _eReason ) override;
        virtual bool        canAdvance() const override;

        void implCheckFinish();

        DECL_LINK(OnSelectionModified, weld::ComboBox&, void);
    };

    class OComboDBFieldPage : public ODBFieldPage
    {
    public:
        explicit OComboDBFieldPage(weld::Container* pPage, OControlWizard* pWizard);

    protected:
        // BuilderPage overridables
        virtual void Activate() override;

        // OWizardPage overridables
        virtual bool    canAdvance() const override;

        // ODBFieldPage overridables
        virtual OUString& getDBFieldSetting() override;
    };


}   // namespace dbp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
