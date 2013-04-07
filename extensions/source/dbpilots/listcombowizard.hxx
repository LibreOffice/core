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

#ifndef _EXTENSIONS_DBP_LISTCOMBOWIZARD_HXX_
#define _EXTENSIONS_DBP_LISTCOMBOWIZARD_HXX_

#include "controlwizard.hxx"
#include "commonpagesdbp.hxx"

//.........................................................................
namespace dbp
{
//.........................................................................

#define LCW_STATE_DATASOURCE_SELECTION  0
#define LCW_STATE_TABLESELECTION        1
#define LCW_STATE_FIELDSELECTION        2
#define LCW_STATE_FIELDLINK             3
#define LCW_STATE_COMBODBFIELD          4

    //=====================================================================
    //= OListComboSettings
    //=====================================================================
    struct OListComboSettings : public OControlWizardSettings
    {
        String          sListContentTable;
        String          sListContentField;
        String          sLinkedFormField;
        String          sLinkedListField;
    };

    //=====================================================================
    //= OListComboWizard
    //=====================================================================
    class OListComboWizard : public OControlWizard
    {
    protected:
        OListComboSettings      m_aSettings;
        sal_Bool                m_bListBox : 1;
        sal_Bool                m_bHadDataSelection : 1;

    public:
        OListComboWizard(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObjectModel,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext
        );

        OListComboSettings& getSettings() { return m_aSettings; }

        sal_Bool isListBox() const { return m_bListBox; }

    protected:
        // OWizardMachine overridables
        virtual ::svt::OWizardPage* createPage( WizardState _nState );
        virtual WizardState         determineNextState( WizardState _nCurrentState ) const;
        virtual void                enterState( WizardState _nState );
        virtual sal_Bool            leaveState( WizardState _nState );
        virtual sal_Bool            onFinish();

        virtual sal_Bool approveControl(sal_Int16 _nClassId);

        WizardState getFinalState() const { return isListBox() ? LCW_STATE_FIELDLINK : LCW_STATE_COMBODBFIELD; }

    private:
        void implApplySettings();
    };

    //=====================================================================
    //= OLCPage
    //=====================================================================
    class OLCPage : public OControlWizardPage
    {
    public:
        OLCPage( OListComboWizard* _pParent, const ResId& _rId ) : OControlWizardPage(_pParent, _rId) { }

    protected:
        OListComboSettings& getSettings() { return static_cast<OListComboWizard*>(getDialog())->getSettings(); }
        sal_Bool isListBox() { return static_cast<OListComboWizard*>(getDialog())->isListBox(); }

    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
            getTables(sal_Bool _bNeedIt);
        ::com::sun::star::uno::Sequence< OUString >
            getTableFields(sal_Bool _bNeedIt);
    };

    //=====================================================================
    //= OContentTableSelection
    //=====================================================================
    class OContentTableSelection : public OLCPage
    {
    protected:
        FixedLine       m_aFrame;
        FixedText       m_aSelectTableLabel;
        ListBox         m_aSelectTable;

    public:
        OContentTableSelection( OListComboWizard* _pParent );

    protected:
        // TabPage overridables
        virtual void ActivatePage();

        // OWizardPage overridables
        virtual void        initializePage();
        virtual sal_Bool    commitPage( ::svt::WizardTypes::CommitPageReason _eReason );
        virtual bool        canAdvance() const;

    protected:
        DECL_LINK( OnTableDoubleClicked, ListBox* );
        DECL_LINK( OnTableSelected, ListBox* );
    };

    //=====================================================================
    //= OContentFieldSelection
    //=====================================================================
    class OContentFieldSelection : public OLCPage
    {
    protected:
        FixedLine       m_aFrame;
        FixedText       m_aTableFields;
        ListBox         m_aSelectTableField;
        FixedText       m_aDisplayedFieldLabel;
        Edit            m_aDisplayedField;
        FixedText       m_aInfo;


    public:
        OContentFieldSelection( OListComboWizard* _pParent );

    protected:
        DECL_LINK( OnFieldSelected, ListBox* );
        DECL_LINK( OnTableDoubleClicked, ListBox* );

        // TabPage overridables
        virtual void ActivatePage();

        // OWizardPage overridables
        virtual void        initializePage();
        virtual sal_Bool    commitPage( ::svt::WizardTypes::CommitPageReason _eReason );
        virtual bool        canAdvance() const;
    };

    //=====================================================================
    //= OLinkFieldsPage
    //=====================================================================
    class OLinkFieldsPage : public OLCPage
    {
    protected:
        FixedText       m_aDescription;
        FixedLine       m_aFrame;
        FixedText       m_aValueListFieldLabel;
        ComboBox        m_aValueListField;
        FixedText       m_aTableFieldLabel;
        ComboBox        m_aTableField;


    public:
        OLinkFieldsPage( OListComboWizard* _pParent );

    protected:
        // TabPage overridables
        virtual void ActivatePage();

        // OWizardPage overridables
        virtual void        initializePage();
        virtual sal_Bool    commitPage( ::svt::WizardTypes::CommitPageReason _eReason );
        virtual bool        canAdvance() const;

    private:
        void implCheckFinish();

        DECL_LINK(OnSelectionModified, void*);
    };

    //=====================================================================
    //= OComboDBFieldPage
    //=====================================================================
    class OComboDBFieldPage : public ODBFieldPage
    {
    public:
        OComboDBFieldPage( OControlWizard* _pParent );

    protected:
        OListComboSettings& getSettings() { return static_cast<OListComboWizard*>(getDialog())->getSettings(); }

        // TabPage overridables
        virtual void ActivatePage();

        // OWizardPage overridables
        virtual bool    canAdvance() const;

        // ODBFieldPage overridables
        virtual String& getDBFieldSetting();
    };

//.........................................................................
}   // namespace dbp
//.........................................................................

#endif // _EXTENSIONS_DBP_LISTCOMBOWIZARD_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
