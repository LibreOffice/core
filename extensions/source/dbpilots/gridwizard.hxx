/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: gridwizard.hxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _EXTENSIONS_DBP_GRIDWIZARD_HXX_
#define _EXTENSIONS_DBP_GRIDWIZARD_HXX_

#include "controlwizard.hxx"
#include "commonpagesdbp.hxx"

//.........................................................................
namespace dbp
{
//.........................................................................

    //=====================================================================
    //= OGridSettings
    //=====================================================================
    struct OGridSettings : public OControlWizardSettings
    {
        ::com::sun::star::uno::Sequence< ::rtl::OUString >      aSelectedFields;
    };

    //=====================================================================
    //= OGridWizard
    //=====================================================================
    class OGridWizard : public OControlWizard
    {
    protected:
        OGridSettings   m_aSettings;
        sal_Bool        m_bHadDataSelection : 1;

    public:
        OGridWizard(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObjectModel,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );

        OGridSettings& getSettings() { return m_aSettings; }

    protected:
        // OWizardMachine overridables
        virtual ::svt::OWizardPage* createPage( WizardState _nState );
        virtual WizardState         determineNextState( WizardState _nCurrentState ) const;
        virtual void                enterState( WizardState _nState );
        virtual sal_Bool            leaveState( WizardState _nState );

        virtual sal_Bool onFinish(sal_Int32 _nResult);

        virtual sal_Bool approveControl(sal_Int16 _nClassId);

    protected:
        void implApplySettings();
    };

    //=====================================================================
    //= OGridPage
    //=====================================================================
    class OGridPage : public OControlWizardPage
    {
    public:
        OGridPage( OGridWizard* _pParent, const ResId& _rId ) : OControlWizardPage(_pParent, _rId) { }

    protected:
        OGridSettings& getSettings() { return static_cast<OGridWizard*>(getDialog())->getSettings(); }
    };

    //=====================================================================
    //= OGridFieldsSelection
    //=====================================================================
    class OGridFieldsSelection : public OGridPage
    {
    protected:
        FixedLine       m_aFrame;
        FixedText       m_aExistFieldsLabel;
        ListBox         m_aExistFields;
        PushButton      m_aSelectOne;
        PushButton      m_aSelectAll;
        PushButton      m_aDeselectOne;
        PushButton      m_aDeselectAll;
        FixedText       m_aSelFieldsLabel;
        ListBox         m_aSelFields;

    public:
        OGridFieldsSelection( OGridWizard* _pParent );

    protected:
        // TabPage overridables
        virtual void ActivatePage();

        // OWizardPage overridables
        virtual void        initializePage();
        virtual sal_Bool    commitPage( CommitPageReason _eReason );
        virtual bool        canAdvance() const;

    protected:
        DECL_LINK(OnMoveOneEntry, PushButton*);
        DECL_LINK(OnMoveAllEntries, PushButton*);
        DECL_LINK(OnEntrySelected, ListBox*);
        DECL_LINK(OnEntryDoubleClicked, ListBox*);

        void implCheckButtons();
        void implApplySettings();

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > implGetColumns(sal_Bool _bShowError = sal_True);
    };

//.........................................................................
}   // namespace dbp
//.........................................................................

#endif // _EXTENSIONS_DBP_GRIDWIZARD_HXX_

