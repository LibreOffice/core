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

#ifndef INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_LISTCOMBOWIZARD_HXX
#define INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_LISTCOMBOWIZARD_HXX

#include "controlwizard.hxx"
#include "commonpagesdbp.hxx"


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

    class OListComboWizard : public OControlWizard
    {
    protected:
        OListComboSettings      m_aSettings;
        bool                m_bListBox : 1;
        bool                m_bHadDataSelection : 1;

    public:
        OListComboWizard(
            vcl::Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObjectModel,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext
        );

        OListComboSettings& getSettings() { return m_aSettings; }

        bool isListBox() const { return m_bListBox; }

    protected:
        // OWizardMachine overridables
        virtual VclPtr<TabPage>     createPage( WizardState _nState ) SAL_OVERRIDE;
        virtual WizardState         determineNextState( WizardState _nCurrentState ) const SAL_OVERRIDE;
        virtual void                enterState( WizardState _nState ) SAL_OVERRIDE;
        virtual bool                leaveState( WizardState _nState ) SAL_OVERRIDE;
        virtual bool                onFinish() SAL_OVERRIDE;

        virtual bool                approveControl(sal_Int16 _nClassId) SAL_OVERRIDE;

        WizardState getFinalState() const { return isListBox() ? LCW_STATE_FIELDLINK : LCW_STATE_COMBODBFIELD; }

    private:
        void implApplySettings();
    };

    class OLCPage : public OControlWizardPage
    {
    public:
        OLCPage(OListComboWizard* _pParent, const OString& rID, const OUString& rUIXMLDescription)
            : OControlWizardPage(_pParent, rID, rUIXMLDescription)
    {
    }

    protected:
        OListComboSettings& getSettings() { return static_cast<OListComboWizard*>(getDialog())->getSettings(); }
        bool isListBox() { return static_cast<OListComboWizard*>(getDialog())->isListBox(); }

    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
            getTables(bool _bNeedIt);
        ::com::sun::star::uno::Sequence< OUString >
            getTableFields(bool _bNeedIt);
    };

    class OContentTableSelection : public OLCPage
    {
    protected:
        VclPtr<ListBox>         m_pSelectTable;

    public:
        explicit OContentTableSelection( OListComboWizard* _pParent );
        virtual ~OContentTableSelection();
        virtual void dispose() SAL_OVERRIDE;

    protected:
        // TabPage overridables
        virtual void ActivatePage() SAL_OVERRIDE;

        // OWizardPage overridables
        virtual void        initializePage() SAL_OVERRIDE;
        virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) SAL_OVERRIDE;
        virtual bool        canAdvance() const SAL_OVERRIDE;

    protected:
        DECL_LINK_TYPED( OnTableDoubleClicked, ListBox&, void );
        DECL_LINK( OnTableSelected, ListBox* );
    };

    class OContentFieldSelection : public OLCPage
    {
    protected:
        VclPtr<ListBox>         m_pSelectTableField;
        VclPtr<Edit>            m_pDisplayedField;
        VclPtr<FixedText>       m_pInfo;


    public:
        explicit OContentFieldSelection( OListComboWizard* _pParent );
        virtual ~OContentFieldSelection();
        virtual void dispose() SAL_OVERRIDE;

    protected:
        DECL_LINK( OnFieldSelected, ListBox* );
        DECL_LINK_TYPED( OnTableDoubleClicked, ListBox&, void );

        // TabPage overridables
        virtual void ActivatePage() SAL_OVERRIDE;

        // OWizardPage overridables
        virtual void        initializePage() SAL_OVERRIDE;
        virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) SAL_OVERRIDE;
        virtual bool        canAdvance() const SAL_OVERRIDE;
    };

    class OLinkFieldsPage : public OLCPage
    {
    protected:
        VclPtr<ComboBox>        m_pValueListField;
        VclPtr<ComboBox>        m_pTableField;


    public:
        explicit OLinkFieldsPage( OListComboWizard* _pParent );
        virtual ~OLinkFieldsPage();
        virtual void dispose() SAL_OVERRIDE;

    protected:
        // TabPage overridables
        virtual void ActivatePage() SAL_OVERRIDE;

        // OWizardPage overridables
        virtual void        initializePage() SAL_OVERRIDE;
        virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) SAL_OVERRIDE;
        virtual bool        canAdvance() const SAL_OVERRIDE;

    private:
        void implCheckFinish();

        DECL_LINK(OnSelectionModified, void*);
    };

    class OComboDBFieldPage : public ODBFieldPage
    {
    public:
        explicit OComboDBFieldPage( OControlWizard* _pParent );

    protected:
        OListComboSettings& getSettings() { return static_cast<OListComboWizard*>(getDialog())->getSettings(); }

        // TabPage overridables
        virtual void ActivatePage() SAL_OVERRIDE;

        // OWizardPage overridables
        virtual bool    canAdvance() const SAL_OVERRIDE;

        // ODBFieldPage overridables
        virtual OUString& getDBFieldSetting() SAL_OVERRIDE;
    };


}   // namespace dbp


#endif // INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_LISTCOMBOWIZARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
