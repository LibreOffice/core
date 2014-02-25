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

#ifndef _EXTENSIONS_DBP_GROUPBOXWIZ_HXX_
#define _EXTENSIONS_DBP_GROUPBOXWIZ_HXX_

#include "controlwizard.hxx"
#include "commonpagesdbp.hxx"


namespace dbp
{



    //= OOptionGroupSettings

    struct OOptionGroupSettings : public OControlWizardSettings
    {
        StringArray     aLabels;
        StringArray     aValues;
        OUString        sDefaultField;
        OUString        sDBField;
        OUString        sName;
    };


    //= OGroupBoxWizard

    class OGroupBoxWizard : public OControlWizard
    {
    protected:
        OOptionGroupSettings        m_aSettings;

        sal_Bool        m_bVisitedDefault   : 1;
        sal_Bool        m_bVisitedDB        : 1;

    public:
        OGroupBoxWizard(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObjectModel,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext
        );

        OOptionGroupSettings& getSettings() { return m_aSettings; }

    protected:
        // OWizardMachine overridables
        virtual ::svt::OWizardPage* createPage( WizardState _nState );
        virtual WizardState         determineNextState( WizardState _nCurrentState ) const;
        virtual void                enterState( WizardState _nState );
        virtual sal_Bool            onFinish();

        virtual sal_Bool approveControl(sal_Int16 _nClassId);

    protected:
        void createRadios();
    };


    //= OGBWPage

    class OGBWPage : public OControlWizardPage
    {
    public:
        OGBWPage( OControlWizard* _pParent, const ResId& _rId ) : OControlWizardPage(_pParent, _rId) { }

    protected:
        OOptionGroupSettings& getSettings() { return static_cast<OGroupBoxWizard*>(getDialog())->getSettings(); }
    };


    //= ORadioSelectionPage

    class ORadioSelectionPage : public OGBWPage
    {
    protected:
        FixedLine       m_aFrame;
        FixedText       m_aRadioNameLabel;
        Edit            m_aRadioName;
        PushButton      m_aMoveRight;
        PushButton      m_aMoveLeft;
        FixedText       m_aExistingRadiosLabel;
        ListBox         m_aExistingRadios;

    public:
        ORadioSelectionPage( OControlWizard* _pParent );

    protected:
        // TabPage overridables
        void ActivatePage();

        // OWizardPage overridables
        virtual void        initializePage();
        virtual sal_Bool    commitPage( ::svt::WizardTypes::CommitPageReason _eReason );
        virtual bool        canAdvance() const;

        DECL_LINK( OnMoveEntry, PushButton* );
        DECL_LINK( OnEntrySelected, ListBox* );
        DECL_LINK( OnNameModified, Edit* );

        void implCheckMoveButtons();
    };


    //= ODefaultFieldSelectionPage

    class ODefaultFieldSelectionPage : public OMaybeListSelectionPage
    {
    protected:
        FixedLine       m_aFrame;
        FixedText       m_aDefaultSelectionLabel;
        RadioButton     m_aDefSelYes;
        RadioButton     m_aDefSelNo;
        ListBox         m_aDefSelection;

    public:
        ODefaultFieldSelectionPage( OControlWizard* _pParent );

    protected:
        // OWizardPage overridables
        virtual void        initializePage();
        virtual sal_Bool    commitPage( ::svt::WizardTypes::CommitPageReason _eReason );

        OOptionGroupSettings& getSettings() { return static_cast<OGroupBoxWizard*>(getDialog())->getSettings(); }
    };


    //= OOptionValuesPage

    class OOptionValuesPage : public OGBWPage
    {
    protected:
        FixedLine       m_aFrame;
        FixedText       m_aDescription;
        FixedText       m_aValueLabel;
        Edit            m_aValue;
        FixedText       m_aOptionsLabel;
        ListBox         m_aOptions;

        StringArray     m_aUncommittedValues;
        ::svt::WizardTypes::WizardState
                        m_nLastSelection;

    public:
        OOptionValuesPage( OControlWizard* _pParent );

    protected:
        // TabPage overridables
        void ActivatePage();

        // OWizardPage overridables
        virtual void        initializePage();
        virtual sal_Bool    commitPage( ::svt::WizardTypes::CommitPageReason _eReason );

        void implTraveledOptions();

        DECL_LINK( OnOptionSelected, ListBox* );
    };


    //= OOptionDBFieldPage

    class OOptionDBFieldPage : public ODBFieldPage
    {
    public:
        OOptionDBFieldPage( OControlWizard* _pParent );

    protected:
        OOptionGroupSettings& getSettings() { return static_cast<OGroupBoxWizard*>(getDialog())->getSettings(); }

        // ODBFieldPage overridables
        virtual OUString& getDBFieldSetting();
    };


    //= OFinalizeGBWPage

    class OFinalizeGBWPage : public OGBWPage
    {
    protected:
        FixedLine       m_aFrame;
        FixedText       m_aNameLabel;
        Edit            m_aName;
        FixedText       m_aThatsAll;

    public:
        OFinalizeGBWPage( OControlWizard* _pParent );

    protected:
        // TabPage overridables
        void ActivatePage();

        // OWizardPage overridables
        virtual void        initializePage();
        virtual sal_Bool    commitPage( ::svt::WizardTypes::CommitPageReason _eReason );
        virtual bool        canAdvance() const;
    };


}   // namespace dbp


#endif // _EXTENSIONS_DBP_GROUPBOXWIZ_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
