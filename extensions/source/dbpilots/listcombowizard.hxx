/*************************************************************************
 *
 *  $RCSfile: listcombowizard.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2001-02-28 09:18:30 $
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

#ifndef _EXTENSIONS_DBP_LISTCOMBOWIZARD_HXX_
#define _EXTENSIONS_DBP_LISTCOMBOWIZARD_HXX_

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
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );

        OListComboSettings& getSettings() { return m_aSettings; }

        sal_Bool isListBox() const { return m_bListBox; }

    protected:
        // OWizardMachine overridables
        virtual ::svt::OWizardPage*     createPage(sal_uInt16 _nState);
        virtual sal_uInt16      determineNextState(sal_uInt16 _nCurrentState);
        virtual void            enterState(sal_uInt16 _nState);
        virtual sal_Bool        leaveState(sal_uInt16 _nState);

        virtual sal_Bool onFinish(sal_Int32 _nResult);

        virtual sal_Bool approveControlType(sal_Int16 _nClassId);

        sal_uInt16 getFinalState() const { return isListBox() ? LCW_STATE_FIELDLINK : LCW_STATE_COMBODBFIELD; }

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
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
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
        virtual sal_Bool    commitPage(COMMIT_REASON _eReason);
        sal_Bool            determineNextButtonState();

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
        virtual sal_Bool    commitPage(COMMIT_REASON _eReason);
        sal_Bool            determineNextButtonState();
    };

    //=====================================================================
    //= OLinkFieldsPage
    //=====================================================================
    class OLinkFieldsPage : public OLCPage
    {
    protected:
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
        virtual sal_Bool    commitPage(COMMIT_REASON _eReason);
        sal_Bool            determineNextButtonState();

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
        virtual sal_Bool determineNextButtonState();

        // ODBFieldPage overridables
        virtual String& getDBFieldSetting();
    };

//.........................................................................
}   // namespace dbp
//.........................................................................

#endif // _EXTENSIONS_DBP_LISTCOMBOWIZARD_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2001/02/23 15:20:18  fs
 *  initial checkin - list-/combobox wizard (not comlpletely finished yet)
 *
 *
 *  Revision 1.0 21.02.01 15:46:59  fs
 ************************************************************************/

