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

#ifndef INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_GRIDWIZARD_HXX
#define INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_GRIDWIZARD_HXX

#include "controlwizard.hxx"
#include "commonpagesdbp.hxx"


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
        OGridWizard(
            vcl::Window* _pParent,
            const css::uno::Reference< css::beans::XPropertySet >& _rxObjectModel,
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext
        );

        OGridSettings& getSettings() { return m_aSettings; }

    private:
        // OWizardMachine overridables
        virtual VclPtr<TabPage>     createPage( WizardState _nState ) override;
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
        OGridPage( OGridWizard* _pParent, const OString& _rID, const OUString& _rUIXMLDescription ) : OControlWizardPage(_pParent, _rID, _rUIXMLDescription) { }

    protected:
        OGridSettings& getSettings() { return static_cast<OGridWizard*>(getDialog())->getSettings(); }
    };

    class OGridFieldsSelection final : public OGridPage
    {
        VclPtr<ListBox>         m_pExistFields;
        VclPtr<PushButton>      m_pSelectOne;
        VclPtr<PushButton>      m_pSelectAll;
        VclPtr<PushButton>      m_pDeselectOne;
        VclPtr<PushButton>      m_pDeselectAll;
        VclPtr<ListBox>         m_pSelFields;

    public:
        explicit OGridFieldsSelection( OGridWizard* _pParent );
        virtual ~OGridFieldsSelection() override;
        virtual void dispose() override;

    private:
        // TabPage overridables
        virtual void ActivatePage() override;

        // OWizardPage overridables
        virtual void        initializePage() override;
        virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) override;
        virtual bool        canAdvance() const override;

        DECL_LINK(OnMoveOneEntry, Button*, void);
        DECL_LINK(OnMoveAllEntries, Button*, void);
        DECL_LINK(OnEntrySelected, ListBox&, void);
        DECL_LINK(OnEntryDoubleClicked, ListBox&, void);

        void implCheckButtons();
    };


}   // namespace dbp


#endif // INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_GRIDWIZARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
