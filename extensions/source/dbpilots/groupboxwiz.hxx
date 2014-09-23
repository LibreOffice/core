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

        bool        m_bVisitedDefault   : 1;
        bool        m_bVisitedDB        : 1;

    public:
        OGroupBoxWizard(
            vcl::Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObjectModel,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext
        );

        OOptionGroupSettings& getSettings() { return m_aSettings; }

    protected:
        // OWizardMachine overridables
        virtual ::svt::OWizardPage* createPage( WizardState _nState ) SAL_OVERRIDE;
        virtual WizardState         determineNextState( WizardState _nCurrentState ) const SAL_OVERRIDE;
        virtual void                enterState( WizardState _nState ) SAL_OVERRIDE;
        virtual bool                onFinish() SAL_OVERRIDE;

        virtual bool                approveControl(sal_Int16 _nClassId) SAL_OVERRIDE;

    protected:
        void createRadios();
    };


    //= OGBWPage

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


    //= ORadioSelectionPage

    class ORadioSelectionPage : public OGBWPage
    {
    protected:
        Edit            *m_pRadioName;
        PushButton      *m_pMoveRight;
        PushButton      *m_pMoveLeft;
        ListBox         *m_pExistingRadios;

    public:
        ORadioSelectionPage( OControlWizard* _pParent );

    protected:
        // TabPage overridables
        void ActivatePage() SAL_OVERRIDE;

        // OWizardPage overridables
        virtual void        initializePage() SAL_OVERRIDE;
        virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) SAL_OVERRIDE;
        virtual bool        canAdvance() const SAL_OVERRIDE;

        DECL_LINK( OnMoveEntry, PushButton* );
        DECL_LINK( OnEntrySelected, ListBox* );
        DECL_LINK( OnNameModified, Edit* );

        void implCheckMoveButtons();
    };


    //= ODefaultFieldSelectionPage

    class ODefaultFieldSelectionPage : public OMaybeListSelectionPage
    {
    protected:
        RadioButton     *m_pDefSelYes;
        RadioButton     *m_pDefSelNo;
        ListBox         *m_pDefSelection;

    public:
        ODefaultFieldSelectionPage( OControlWizard* _pParent );

    protected:
        // OWizardPage overridables
        virtual void        initializePage() SAL_OVERRIDE;
        virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) SAL_OVERRIDE;

        OOptionGroupSettings& getSettings() { return static_cast<OGroupBoxWizard*>(getDialog())->getSettings(); }
    };


    //= OOptionValuesPage

    class OOptionValuesPage : public OGBWPage
    {
    protected:
        Edit            *m_pValue;
        ListBox         *m_pOptions;

        StringArray     m_aUncommittedValues;
        ::svt::WizardTypes::WizardState
                        m_nLastSelection;

    public:
        OOptionValuesPage( OControlWizard* _pParent );

    protected:
        // TabPage overridables
        void ActivatePage() SAL_OVERRIDE;

        // OWizardPage overridables
        virtual void        initializePage() SAL_OVERRIDE;
        virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) SAL_OVERRIDE;

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
        virtual OUString& getDBFieldSetting() SAL_OVERRIDE;
    };


    //= OFinalizeGBWPage

    class OFinalizeGBWPage : public OGBWPage
    {
    protected:
        Edit            *m_pName;

    public:
        OFinalizeGBWPage( OControlWizard* _pParent );

    protected:
        // TabPage overridables
        void ActivatePage() SAL_OVERRIDE;

        // OWizardPage overridables
        virtual void        initializePage() SAL_OVERRIDE;
        virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) SAL_OVERRIDE;
        virtual bool        canAdvance() const SAL_OVERRIDE;
    };


}   // namespace dbp


#endif // INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_GROUPBOXWIZ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
