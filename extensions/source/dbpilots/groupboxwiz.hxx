/*************************************************************************
 *
 *  $RCSfile: groupboxwiz.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-02 09:42:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
        virtual ::svt::OWizardPage*     createPage(sal_uInt16 _nState);
        virtual sal_uInt16      determineNextState(sal_uInt16 _nCurrentState);
        virtual void            enterState(sal_uInt16 _nState);
        virtual sal_Bool        leaveState(sal_uInt16 _nState);

        virtual sal_Bool onFinish(sal_Int32 _nResult);

        virtual sal_Bool approveControlType(sal_Int16 _nClassId);

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
        virtual sal_Bool    commitPage(COMMIT_REASON _eReason);
        virtual sal_Bool    determineNextButtonState();

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
        virtual sal_Bool    commitPage(COMMIT_REASON _eReason);

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
        sal_uInt16      m_nLastSelection;

    public:
        OOptionValuesPage( OControlWizard* _pParent );

    protected:
        // TabPage overridables
        void ActivatePage();

        // OWizardPage overridables
        virtual void        initializePage();
        virtual sal_Bool    commitPage(COMMIT_REASON _eReason);

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
        virtual sal_Bool    commitPage(COMMIT_REASON _eReason);
        virtual sal_Bool    determineNextButtonState();
    };

//.........................................................................
}   // namespace dbp
//.........................................................................

#endif // _EXTENSIONS_DBP_GROUPBOXWIZ_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.3  2001/02/28 09:18:30  fs
 *  finalized the list/combo wizard
 *
 *  Revision 1.2  2001/02/23 15:19:08  fs
 *  some changes / centralizations - added the list-/combobox wizard
 *
 *  Revision 1.1  2001/02/21 09:24:04  fs
 *  initial checkin - form control auto pilots
 *
 *
 *  Revision 1.0 14.02.01 10:39:33  fs
 ************************************************************************/

