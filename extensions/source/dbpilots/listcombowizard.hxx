/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: listcombowizard.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:42:07 $
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
        virtual ::svt::OWizardPage* createPage( WizardState _nState );
        virtual WizardState         determineNextState( WizardState _nCurrentState ) const;
        virtual void                enterState( WizardState _nState );
        virtual sal_Bool            leaveState( WizardState _nState );

        virtual sal_Bool onFinish(sal_Int32 _nResult);

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
        virtual sal_Bool    commitPage( CommitPageReason _eReason );
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
        virtual sal_Bool    commitPage( CommitPageReason _eReason );
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
        virtual sal_Bool    commitPage( CommitPageReason _eReason );
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

