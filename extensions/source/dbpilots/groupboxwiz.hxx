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

#ifndef INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_GROUPBOXWIZ_HXX
#define INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_GROUPBOXWIZ_HXX

#include "controlwizard.hxx"
#include "commonpagesdbp.hxx"


namespace dbp
{

    struct OOptionGroupSettings : public OControlWizardSettings
    {
        StringArray     aLabels;
        StringArray     aValues;
        OUString        sDefaultField;
        OUString        sDBField;
    };

    class OGroupBoxWizard : public OControlWizard
    {
    protected:
        OOptionGroupSettings        m_aSettings;

        bool        m_bVisitedDefault   : 1;
        bool        m_bVisitedDB        : 1;

    public:
        OGroupBoxWizard(
            vcl::Window* _pParent,
            const css::uno::Reference< css::beans::XPropertySet >& _rxObjectModel,
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext
        );

        OOptionGroupSettings& getSettings() { return m_aSettings; }

    protected:
        // OWizardMachine overridables
        virtual VclPtr<TabPage>     createPage( WizardState _nState ) override;
        virtual WizardState         determineNextState( WizardState _nCurrentState ) const override;
        virtual void                enterState( WizardState _nState ) override;
        virtual bool                onFinish() override;

        virtual bool                approveControl(sal_Int16 _nClassId) override;
    };

    class OGBWPage : public OControlWizardPage
    {
    public:
        OGBWPage(OControlWizard* _pParent, const OString& _rID, const OUString& _rUIXMLDescription)
            : OControlWizardPage(_pParent, _rID, _rUIXMLDescription)
    {
    }

    protected:
        OOptionGroupSettings& getSettings() { return static_cast<OGroupBoxWizard*>(getDialog())->getSettings(); }
    };

    class ORadioSelectionPage : public OGBWPage
    {
    protected:
        VclPtr<Edit>            m_pRadioName;
        VclPtr<PushButton>      m_pMoveRight;
        VclPtr<PushButton>      m_pMoveLeft;
        VclPtr<ListBox>         m_pExistingRadios;

    public:
        explicit ORadioSelectionPage( OControlWizard* _pParent );
        virtual ~ORadioSelectionPage() override;
        virtual void dispose() override;

    protected:
        // TabPage overridables
        void ActivatePage() override;

        // OWizardPage overridables
        virtual void        initializePage() override;
        virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) override;
        virtual bool        canAdvance() const override;

        DECL_LINK( OnMoveEntry, Button*, void );
        DECL_LINK( OnEntrySelected, ListBox&, void );
        DECL_LINK( OnNameModified, Edit&, void );

        void implCheckMoveButtons();
    };

    class ODefaultFieldSelectionPage : public OMaybeListSelectionPage
    {
    protected:
        VclPtr<RadioButton>     m_pDefSelYes;
        VclPtr<RadioButton>     m_pDefSelNo;
        VclPtr<ListBox>         m_pDefSelection;

    public:
        explicit ODefaultFieldSelectionPage( OControlWizard* _pParent );
        virtual ~ODefaultFieldSelectionPage() override;
        virtual void dispose() override;

    protected:
        // OWizardPage overridables
        virtual void        initializePage() override;
        virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) override;

        OOptionGroupSettings& getSettings() { return static_cast<OGroupBoxWizard*>(getDialog())->getSettings(); }
    };

    class OOptionValuesPage : public OGBWPage
    {
    protected:
        VclPtr<Edit>            m_pValue;
        VclPtr<ListBox>         m_pOptions;

        StringArray     m_aUncommittedValues;
        ::svt::WizardTypes::WizardState
                        m_nLastSelection;

    public:
        explicit OOptionValuesPage( OControlWizard* _pParent );
        virtual ~OOptionValuesPage() override;
        virtual void dispose() override;

    protected:
        // TabPage overridables
        void ActivatePage() override;

        // OWizardPage overridables
        virtual void        initializePage() override;
        virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) override;

        void implTraveledOptions();

        DECL_LINK( OnOptionSelected, ListBox&, void );
    };

    class OOptionDBFieldPage : public ODBFieldPage
    {
    public:
        explicit OOptionDBFieldPage( OControlWizard* _pParent );

    protected:
        // ODBFieldPage overridables
        virtual OUString& getDBFieldSetting() override;
    };

    class OFinalizeGBWPage : public OGBWPage
    {
    protected:
        VclPtr<Edit>            m_pName;

    public:
        explicit OFinalizeGBWPage( OControlWizard* _pParent );
        virtual ~OFinalizeGBWPage() override;
        virtual void dispose() override;

    protected:
        // TabPage overridables
        void ActivatePage() override;

        // OWizardPage overridables
        virtual void        initializePage() override;
        virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) override;
        virtual bool        canAdvance() const override;
    };


}   // namespace dbp


#endif // INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_GROUPBOXWIZ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
