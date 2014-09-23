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



    //= OGridSettings

    struct OGridSettings : public OControlWizardSettings
    {
        ::com::sun::star::uno::Sequence< OUString >      aSelectedFields;
    };


    //= OGridWizard

    class OGridWizard : public OControlWizard
    {
    protected:
        OGridSettings   m_aSettings;
        bool        m_bHadDataSelection : 1;

    public:
        OGridWizard(
            vcl::Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObjectModel,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext
        );

        OGridSettings& getSettings() { return m_aSettings; }

    protected:
        // OWizardMachine overridables
        virtual ::svt::OWizardPage* createPage( WizardState _nState ) SAL_OVERRIDE;
        virtual WizardState         determineNextState( WizardState _nCurrentState ) const SAL_OVERRIDE;
        virtual void                enterState( WizardState _nState ) SAL_OVERRIDE;
        virtual bool                leaveState( WizardState _nState ) SAL_OVERRIDE;
        virtual bool                onFinish() SAL_OVERRIDE;

        virtual bool                approveControl(sal_Int16 _nClassId) SAL_OVERRIDE;

    protected:
        void implApplySettings();
    };

    //= OGridPage
    class OGridPage : public OControlWizardPage
    {
    public:
        OGridPage( OGridWizard* _pParent, const OString& _rID, const OUString& _rUIXMLDescription ) : OControlWizardPage(_pParent, _rID, _rUIXMLDescription) { }

    protected:
        OGridSettings& getSettings() { return static_cast<OGridWizard*>(getDialog())->getSettings(); }
    };

    //= OGridFieldsSelection
    class OGridFieldsSelection : public OGridPage
    {
    protected:
        ListBox         *m_pExistFields;
        PushButton      *m_pSelectOne;
        PushButton      *m_pSelectAll;
        PushButton      *m_pDeselectOne;
        PushButton      *m_pDeselectAll;
        ListBox         *m_pSelFields;

    public:
        OGridFieldsSelection( OGridWizard* _pParent );

    protected:
        // TabPage overridables
        virtual void ActivatePage() SAL_OVERRIDE;

        // OWizardPage overridables
        virtual void        initializePage() SAL_OVERRIDE;
        virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) SAL_OVERRIDE;
        virtual bool        canAdvance() const SAL_OVERRIDE;

    protected:
        DECL_LINK(OnMoveOneEntry, PushButton*);
        DECL_LINK(OnMoveAllEntries, PushButton*);
        DECL_LINK(OnEntrySelected, ListBox*);
        DECL_LINK(OnEntryDoubleClicked, ListBox*);

        void implCheckButtons();
        void implApplySettings();

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > implGetColumns(bool _bShowError = true);
    };


}   // namespace dbp


#endif // INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_GRIDWIZARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
