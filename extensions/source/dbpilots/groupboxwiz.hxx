/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: groupboxwiz.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:41:35 $
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

#ifndef _EXTENSIONS_DBP_GROUPBOXWIZ_HXX_
#define _EXTENSIONS_DBP_GROUPBOXWIZ_HXX_

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
    //= OOptionGroupSettings
    //=====================================================================
    struct OOptionGroupSettings : public OControlWizardSettings
    {
        StringArray     aLabels;
        StringArray     aValues;
        String          sDefaultField;
        String          sDBField;
        String          sName;
    };

    //=====================================================================
    //= OGroupBoxWizard
    //=====================================================================
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
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );

        OOptionGroupSettings& getSettings() { return m_aSettings; }

    protected:
        // OWizardMachine overridables
        virtual ::svt::OWizardPage* createPage( WizardState _nState );
        virtual WizardState         determineNextState( WizardState _nCurrentState ) const;
        virtual void                enterState( WizardState _nState );

        virtual sal_Bool onFinish(sal_Int32 _nResult);

        virtual sal_Bool approveControl(sal_Int16 _nClassId);

    protected:
        void createRadios();
    };

    //=====================================================================
    //= OGBWPage
    //=====================================================================
    class OGBWPage : public OControlWizardPage
    {
    public:
        OGBWPage( OControlWizard* _pParent, const ResId& _rId ) : OControlWizardPage(_pParent, _rId) { }

    protected:
        OOptionGroupSettings& getSettings() { return static_cast<OGroupBoxWizard*>(getDialog())->getSettings(); }
    };

    //=====================================================================
    //= ORadioSelectionPage
    //=====================================================================
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
        virtual sal_Bool    commitPage( CommitPageReason _eReason );
        virtual bool        canAdvance() const;

        DECL_LINK( OnMoveEntry, PushButton* );
        DECL_LINK( OnEntrySelected, ListBox* );
        DECL_LINK( OnNameModified, Edit* );

        void implCheckMoveButtons();
    };

    //=====================================================================
    //= ODefaultFieldSelectionPage
    //=====================================================================
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
        virtual sal_Bool    commitPage( CommitPageReason _eReason );

        OOptionGroupSettings& getSettings() { return static_cast<OGroupBoxWizard*>(getDialog())->getSettings(); }
    };

    //=====================================================================
    //= OOptionValuesPage
    //=====================================================================
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
        WizardState     m_nLastSelection;

    public:
        OOptionValuesPage( OControlWizard* _pParent );

    protected:
        // TabPage overridables
        void ActivatePage();

        // OWizardPage overridables
        virtual void        initializePage();
        virtual sal_Bool    commitPage( CommitPageReason _eReason );

        void implTraveledOptions();

        DECL_LINK( OnOptionSelected, ListBox* );
    };

    //=====================================================================
    //= OOptionDBFieldPage
    //=====================================================================
    class OOptionDBFieldPage : public ODBFieldPage
    {
    public:
        OOptionDBFieldPage( OControlWizard* _pParent );

    protected:
        OOptionGroupSettings& getSettings() { return static_cast<OGroupBoxWizard*>(getDialog())->getSettings(); }

        // ODBFieldPage overridables
        virtual String& getDBFieldSetting();
    };

    //=====================================================================
    //= OFinalizeGBWPage
    //=====================================================================
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
        virtual sal_Bool    commitPage( CommitPageReason _eReason );
        virtual bool        canAdvance() const;
    };

//.........................................................................
}   // namespace dbp
//.........................................................................

#endif // _EXTENSIONS_DBP_GROUPBOXWIZ_HXX_

