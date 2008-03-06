/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gridwizard.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:41:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _EXTENSIONS_DBP_GRIDWIZARD_HXX_
#define _EXTENSIONS_DBP_GRIDWIZARD_HXX_

#ifndef _EXTENSIONS_DBP_CONTROLWIZARD_HXX
#include "controlwizard.hxx"
#endif
#ifndef _EXTENSIONS_DBP_COMMONPAGESDBP_HXX_
#include "commonpagesdbp.hxx"
#endif

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

