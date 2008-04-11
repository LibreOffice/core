/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: listcombowizard.cxx,v $
 * $Revision: 1.22 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include "listcombowizard.hxx"
#include "commonpagesdbp.hxx"
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/form/ListSourceType.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <tools/debug.hxx>
#include <vcl/msgbox.hxx>
#include <connectivity/dbtools.hxx>
#ifndef EXTENSIONS_INC_EXTENSIO_HRC
#include "extensio.hrc"
#endif
#include <comphelper/extract.hxx>

//.........................................................................
namespace dbp
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::form;
    using namespace ::svt;
    using namespace ::dbtools;

    //=====================================================================
    //= OListComboWizard
    //=====================================================================
    //---------------------------------------------------------------------
    OListComboWizard::OListComboWizard( Window* _pParent,
            const Reference< XPropertySet >& _rxObjectModel, const Reference< XMultiServiceFactory >& _rxORB )
        :OControlWizard(_pParent, ModuleRes(RID_DLG_LISTCOMBOWIZARD), _rxObjectModel, _rxORB)
        ,m_bListBox(sal_False)
        ,m_bHadDataSelection(sal_True)
    {
        initControlSettings(&m_aSettings);

        m_pPrevPage->SetHelpId(HID_LISTWIZARD_PREVIOUS);
        m_pNextPage->SetHelpId(HID_LISTWIZARD_NEXT);
        m_pCancel->SetHelpId(HID_LISTWIZARD_CANCEL);
        m_pFinish->SetHelpId(HID_LISTWIZARD_FINISH);

        // if we do not need the data source selection page ...
        if (!needDatasourceSelection())
        {   // ... skip it!
            skip(1);
            m_bHadDataSelection = sal_False;
        }
    }

    //---------------------------------------------------------------------
    sal_Bool OListComboWizard::approveControl(sal_Int16 _nClassId)
    {
        switch (_nClassId)
        {
            case FormComponentType::LISTBOX:
                m_bListBox = sal_True;
                setTitleBase(String(ModuleRes(RID_STR_LISTWIZARD_TITLE)));
                return sal_True;
            case FormComponentType::COMBOBOX:
                m_bListBox = sal_False;
                setTitleBase(String(ModuleRes(RID_STR_COMBOWIZARD_TITLE)));
                return sal_True;
        }
        return sal_False;
    }

    //---------------------------------------------------------------------
    OWizardPage* OListComboWizard::createPage(WizardState _nState)
    {
        switch (_nState)
        {
            case LCW_STATE_DATASOURCE_SELECTION:
                return new OTableSelectionPage(this);
            case LCW_STATE_TABLESELECTION:
                return new OContentTableSelection(this);
            case LCW_STATE_FIELDSELECTION:
                return new OContentFieldSelection(this);
            case LCW_STATE_FIELDLINK:
                return new OLinkFieldsPage(this);
            case LCW_STATE_COMBODBFIELD:
                return new OComboDBFieldPage(this);
        }

        return NULL;
    }

    //---------------------------------------------------------------------
    WizardTypes::WizardState OListComboWizard::determineNextState( WizardState _nCurrentState ) const
    {
        switch (_nCurrentState)
        {
            case LCW_STATE_DATASOURCE_SELECTION:
                return LCW_STATE_TABLESELECTION;
            case LCW_STATE_TABLESELECTION:
                return LCW_STATE_FIELDSELECTION;
            case LCW_STATE_FIELDSELECTION:
                return getFinalState();
        }

        return WZS_INVALID_STATE;
    }

    //---------------------------------------------------------------------
    void OListComboWizard::enterState(WizardState _nState)
    {
        OControlWizard::enterState(_nState);

        enableButtons(WZB_PREVIOUS, m_bHadDataSelection ? (LCW_STATE_DATASOURCE_SELECTION < _nState) : LCW_STATE_TABLESELECTION < _nState);
        enableButtons(WZB_NEXT, getFinalState() != _nState);
        if (_nState < getFinalState())
            enableButtons(WZB_FINISH, sal_False);

        if (getFinalState() == _nState)
            defaultButton(WZB_FINISH);
    }

    //---------------------------------------------------------------------
    sal_Bool OListComboWizard::leaveState(WizardState _nState)
    {
        if (!OControlWizard::leaveState(_nState))
            return sal_False;

        if (getFinalState() == _nState)
            defaultButton(WZB_NEXT);

        return sal_True;
    }

    //---------------------------------------------------------------------
    void OListComboWizard::implApplySettings()
    {
        try
        {
            // for quoting identifiers, we need the connection meta data
            Reference< XConnection > xConn = getFormConnection();
            DBG_ASSERT(xConn.is(), "OListComboWizard::implApplySettings: no connection, unable to quote!");
            Reference< XDatabaseMetaData > xMetaData;
            if (xConn.is())
                xMetaData = xConn->getMetaData();

            // do some quotings
            if (xMetaData.is())
            {
                ::rtl::OUString sQuoteString = xMetaData->getIdentifierQuoteString();
                if (isListBox()) // only when we have a listbox this should be not empty
                    getSettings().sLinkedListField = quoteName(sQuoteString, getSettings().sLinkedListField);

                ::rtl::OUString sCatalog, sSchema, sName;
                ::dbtools::qualifiedNameComponents( xMetaData, getSettings().sListContentTable, sCatalog, sSchema, sName, ::dbtools::eInDataManipulation );
                getSettings().sListContentTable = ::dbtools::composeTableNameForSelect( xConn, sCatalog, sSchema, sName );

                getSettings().sListContentField = quoteName(sQuoteString, getSettings().sListContentField);
            }

            // ListSourceType: SQL
            getContext().xObjectModel->setPropertyValue(::rtl::OUString::createFromAscii("ListSourceType"), makeAny((sal_Int32)ListSourceType_SQL));

            if (isListBox())
            {
                // BoundColumn: 1
                getContext().xObjectModel->setPropertyValue(::rtl::OUString::createFromAscii("BoundColumn"), makeAny((sal_Int16)1));

                // build the statement to set as list source
                String sStatement;
                sStatement.AppendAscii("SELECT ");
                sStatement += getSettings().sListContentField;
                sStatement.AppendAscii(", ");
                sStatement += getSettings().sLinkedListField;
                sStatement.AppendAscii(" FROM ");
                sStatement += getSettings().sListContentTable;
                Sequence< ::rtl::OUString > aListSource(1);
                aListSource[0] = sStatement;
                getContext().xObjectModel->setPropertyValue(::rtl::OUString::createFromAscii("ListSource"), makeAny(aListSource));
            }
            else
            {
                // build the statement to set as list source
                String sStatement;
                sStatement.AppendAscii("SELECT DISTINCT ");
                sStatement += getSettings().sListContentField;
                sStatement.AppendAscii(" FROM ");
                sStatement += getSettings().sListContentTable;
                getContext().xObjectModel->setPropertyValue(::rtl::OUString::createFromAscii("ListSource"), makeAny(::rtl::OUString(sStatement)));
            }

            // the bound field
            getContext().xObjectModel->setPropertyValue(::rtl::OUString::createFromAscii("DataField"), makeAny(::rtl::OUString(getSettings().sLinkedFormField)));

            // by default, create a drop down control
            getContext().xObjectModel->setPropertyValue(::rtl::OUString::createFromAscii("Dropdown"), ::cppu::bool2any(sal_True));
        }
        catch(Exception&)
        {
            DBG_ERROR("OListComboWizard::implApplySettings: could not set the property values for the listbox!");
        }
    }

    //---------------------------------------------------------------------
    sal_Bool OListComboWizard::onFinish(sal_Int32 _nResult)
    {
        if (!OControlWizard::onFinish(_nResult))
            return sal_False;

        if (RET_OK != _nResult)
            return sal_True;

        implApplySettings();
        return sal_True;
    }

    //=====================================================================
    //= OLCPage
    //=====================================================================
    //---------------------------------------------------------------------
    Reference< XNameAccess > OLCPage::getTables(sal_Bool _bNeedIt)
    {
        Reference< XConnection > xConn = getFormConnection();
        DBG_ASSERT(!_bNeedIt || xConn.is(), "OLCPage::getTables: should have an active connection when reaching this page!");
        (void)_bNeedIt;

        Reference< XTablesSupplier > xSuppTables(xConn, UNO_QUERY);
        Reference< XNameAccess > xTables;
        if (xSuppTables.is())
            xTables = xSuppTables->getTables();

        DBG_ASSERT(!_bNeedIt || xTables.is() || !xConn.is(), "OLCPage::getTables: got no tables from the connection!");

        return xTables;
    }

    //---------------------------------------------------------------------
    Sequence< ::rtl::OUString > OLCPage::getTableFields(sal_Bool _bNeedIt)
    {
        Reference< XNameAccess > xTables = getTables(_bNeedIt);
        Sequence< ::rtl::OUString > aColumnNames;
        if (xTables.is())
        {
            try
            {
                // the list table as XColumnsSupplier
                Reference< XColumnsSupplier > xSuppCols;
                xTables->getByName(getSettings().sListContentTable) >>= xSuppCols;
                DBG_ASSERT(!_bNeedIt || xSuppCols.is(), "OLCPage::getTableFields: no columns supplier!");

                // the columns
                Reference< XNameAccess > xColumns;
                if (xSuppCols.is())
                    xColumns = xSuppCols->getColumns();

                // the column names
                if (xColumns.is())
                    aColumnNames = xColumns->getElementNames();
            }
            catch(Exception&)
            {
                DBG_ASSERT(!_bNeedIt, "OLinkFieldsPage::initializePage: caught an exception while retrieving the columns!");
            }
        }
        return aColumnNames;
    }

    //=====================================================================
    //= OContentTableSelection
    //=====================================================================
    //---------------------------------------------------------------------
    OContentTableSelection::OContentTableSelection( OListComboWizard* _pParent )
        :OLCPage(_pParent, ModuleRes(RID_PAGE_LCW_CONTENTSELECTION_TABLE))
        ,m_aFrame               (this, ModuleRes(FL_FRAME))
        ,m_aSelectTableLabel    (this, ModuleRes(FT_SELECTTABLE_LABEL))
        ,m_aSelectTable         (this, ModuleRes(LB_SELECTTABLE))
    {
        FreeResource();

        enableFormDatasourceDisplay();

        m_aSelectTable.SetDoubleClickHdl(LINK(this, OContentTableSelection, OnTableDoubleClicked));
        m_aSelectTable.SetSelectHdl(LINK(this, OContentTableSelection, OnTableSelected));
    }

    //---------------------------------------------------------------------
    void OContentTableSelection::ActivatePage()
    {
        OLCPage::ActivatePage();
        m_aSelectTable.GrabFocus();
    }

    //---------------------------------------------------------------------
    bool OContentTableSelection::canAdvance() const
    {
        if (!OLCPage::canAdvance())
            return false;

        return 0 != m_aSelectTable.GetSelectEntryCount();
    }

    //---------------------------------------------------------------------
    IMPL_LINK( OContentTableSelection, OnTableSelected, ListBox*, /*_pListBox*/ )
    {
        updateDialogTravelUI();
        return 0L;
    }

    //---------------------------------------------------------------------
    IMPL_LINK( OContentTableSelection, OnTableDoubleClicked, ListBox*, _pListBox )
    {
        if (_pListBox->GetSelectEntryCount())
            getDialog()->travelNext();
        return 0L;
    }

    //---------------------------------------------------------------------
    void OContentTableSelection::initializePage()
    {
        OLCPage::initializePage();

        // fill the list with the table name
        m_aSelectTable.Clear();
        try
        {
            Reference< XNameAccess > xTables = getTables(sal_True);
            Sequence< ::rtl::OUString > aTableNames;
            if (xTables.is())
                aTableNames = xTables->getElementNames();
            fillListBox(m_aSelectTable, aTableNames);
        }
        catch(Exception&)
        {
            DBG_ERROR("OContentTableSelection::initializePage: could not retrieve the table names!");
        }

        m_aSelectTable.SelectEntry(getSettings().sListContentTable);
    }

    //---------------------------------------------------------------------
    sal_Bool OContentTableSelection::commitPage( CommitPageReason _eReason )
    {
        if (!OLCPage::commitPage(_eReason))
            return sal_False;

        OListComboSettings& rSettings = getSettings();
        rSettings.sListContentTable = m_aSelectTable.GetSelectEntry();
        if (!rSettings.sListContentTable.Len() && (eTravelBackward != _eReason))
            // need to select a table
            return sal_False;

        return sal_True;
    }

    //=====================================================================
    //= OContentFieldSelection
    //=====================================================================
    //---------------------------------------------------------------------
    OContentFieldSelection::OContentFieldSelection( OListComboWizard* _pParent )
        :OLCPage(_pParent, ModuleRes(RID_PAGE_LCW_CONTENTSELECTION_FIELD))
        ,m_aFrame               (this, ModuleRes(FL_FRAME))
        ,m_aTableFields         (this, ModuleRes(FT_TABLEFIELDS))
        ,m_aSelectTableField    (this, ModuleRes(LB_SELECTFIELD))
        ,m_aDisplayedFieldLabel (this, ModuleRes(FT_DISPLAYEDFIELD))
        ,m_aDisplayedField      (this, ModuleRes(ET_DISPLAYEDFIELD))
        ,m_aInfo                (this, ModuleRes(FT_CONTENTFIELD_INFO))
    {
        m_aInfo.SetText(String(ModuleRes( isListBox() ? STR_FIELDINFO_LISTBOX : STR_FIELDINFO_COMBOBOX)));
        FreeResource();
        m_aSelectTableField.SetSelectHdl(LINK(this, OContentFieldSelection, OnFieldSelected));
        m_aSelectTableField.SetDoubleClickHdl(LINK(this, OContentFieldSelection, OnTableDoubleClicked));
    }

    //---------------------------------------------------------------------
    void OContentFieldSelection::ActivatePage()
    {
        OLCPage::ActivatePage();
        m_aTableFields.GrabFocus();
    }

    //---------------------------------------------------------------------
    void OContentFieldSelection::initializePage()
    {
        OLCPage::initializePage();

        // fill the list of fields
        fillListBox(m_aSelectTableField, getTableFields(sal_True));

        m_aSelectTableField.SelectEntry(getSettings().sListContentField);
        m_aDisplayedField.SetText(getSettings().sListContentField);
    }

    //---------------------------------------------------------------------
    bool OContentFieldSelection::canAdvance() const
    {
        if (!OLCPage::canAdvance())
            return false;

        return 0 != m_aSelectTableField.GetSelectEntryCount();
    }

    //---------------------------------------------------------------------
    IMPL_LINK( OContentFieldSelection, OnTableDoubleClicked, ListBox*, /*NOTINTERESTEDIN*/ )
    {
        if (m_aSelectTableField.GetSelectEntryCount())
            getDialog()->travelNext();
        return 0L;
    }

    //---------------------------------------------------------------------
    IMPL_LINK( OContentFieldSelection, OnFieldSelected, ListBox*, /*NOTINTERESTEDIN*/ )
    {
        updateDialogTravelUI();
        m_aDisplayedField.SetText(m_aSelectTableField.GetSelectEntry());
        return 0L;
    }

    //---------------------------------------------------------------------
    sal_Bool OContentFieldSelection::commitPage( CommitPageReason _eReason )
    {
        if (!OLCPage::commitPage(_eReason))
            return sal_False;

        getSettings().sListContentField = m_aSelectTableField.GetSelectEntry();

        return sal_True;
    }

    //=====================================================================
    //= OLinkFieldsPage
    //=====================================================================
    //---------------------------------------------------------------------
    OLinkFieldsPage::OLinkFieldsPage( OListComboWizard* _pParent )
        :OLCPage(_pParent, ModuleRes(RID_PAGE_LCW_FIELDLINK))
        ,m_aDescription         (this, ModuleRes(FT_FIELDLINK_DESC))
        ,m_aFrame               (this, ModuleRes(FL_FRAME))
        ,m_aValueListFieldLabel (this, ModuleRes(FT_VALUELISTFIELD))
        ,m_aValueListField      (this, ModuleRes(CMB_VALUELISTFIELD))
        ,m_aTableFieldLabel     (this, ModuleRes(FT_TABLEFIELD))
        ,m_aTableField          (this, ModuleRes(CMB_TABLEFIELD))
    {
        FreeResource();

        m_aValueListField.SetModifyHdl(LINK(this, OLinkFieldsPage, OnSelectionModified));
        m_aTableField.SetModifyHdl(LINK(this, OLinkFieldsPage, OnSelectionModified));
        m_aValueListField.SetSelectHdl(LINK(this, OLinkFieldsPage, OnSelectionModified));
        m_aTableField.SetSelectHdl(LINK(this, OLinkFieldsPage, OnSelectionModified));
    }

    //---------------------------------------------------------------------
    void OLinkFieldsPage::ActivatePage()
    {
        OLCPage::ActivatePage();
        m_aValueListField.GrabFocus();
    }

    //---------------------------------------------------------------------
    void OLinkFieldsPage::initializePage()
    {
        OLCPage::initializePage();

        // fill the value list
        fillListBox(m_aValueListField, getContext().aFieldNames);
        // fill the table field list
        fillListBox(m_aTableField, getTableFields(sal_True));

        // the initial selections
        m_aValueListField.SetText(getSettings().sLinkedFormField);
        m_aTableField.SetText(getSettings().sLinkedListField);

        implCheckFinish();
    }

    //---------------------------------------------------------------------
    bool OLinkFieldsPage::canAdvance() const
    {
        // we're on the last page here, no travelNext allowed ...
        return false;
    }

    //---------------------------------------------------------------------
    void OLinkFieldsPage::implCheckFinish()
    {
        sal_Bool bInvalidSelection = (COMBOBOX_ENTRY_NOTFOUND == m_aValueListField.GetEntryPos(m_aValueListField.GetText()));
        bInvalidSelection |= (COMBOBOX_ENTRY_NOTFOUND == m_aTableField.GetEntryPos(m_aTableField.GetText()));
        getDialog()->enableButtons(WZB_FINISH, !bInvalidSelection);
    }

    //---------------------------------------------------------------------
    IMPL_LINK(OLinkFieldsPage, OnSelectionModified, void*, EMPTYARG)
    {
        implCheckFinish();
        return 0L;
    }

    //---------------------------------------------------------------------
    sal_Bool OLinkFieldsPage::commitPage( CommitPageReason _eReason )
    {
        if (!OLCPage::commitPage(_eReason))
            return sal_False;

        getSettings().sLinkedFormField = m_aValueListField.GetText();
        getSettings().sLinkedListField = m_aTableField.GetText();

        return sal_True;
    }

    //=====================================================================
    //= OComboDBFieldPage
    //=====================================================================
    //---------------------------------------------------------------------
    OComboDBFieldPage::OComboDBFieldPage( OControlWizard* _pParent )
        :ODBFieldPage(_pParent)
    {
        setDescriptionText(String(ModuleRes(RID_STR_COMBOWIZ_DBFIELD)));
    }

    //---------------------------------------------------------------------
    String& OComboDBFieldPage::getDBFieldSetting()
    {
        return getSettings().sLinkedFormField;
    }

    //---------------------------------------------------------------------
    void OComboDBFieldPage::ActivatePage()
    {
        ODBFieldPage::ActivatePage();
        getDialog()->enableButtons(WZB_FINISH, sal_True);
    }

    //---------------------------------------------------------------------
    bool OComboDBFieldPage::canAdvance() const
    {
        // we're on the last page here, no travelNext allowed ...
        return false;
    }

//.........................................................................
}   // namespace dbp
//.........................................................................

