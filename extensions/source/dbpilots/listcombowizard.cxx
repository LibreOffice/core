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

#include "listcombowizard.hxx"
#include "commonpagesdbp.hxx"
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/form/ListSourceType.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <connectivity/dbtools.hxx>
#include <helpids.h>
#include <osl/diagnose.h>


namespace dbp
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::form;
    using namespace ::dbtools;

    OListComboWizard::OListComboWizard(weld::Window* _pParent,
            const Reference< XPropertySet >& _rxObjectModel, const Reference< XComponentContext >& _rxContext )
        : OControlWizard(_pParent, _rxObjectModel, _rxContext)
        , m_bListBox(false)
        , m_bHadDataSelection(true)
    {
        initControlSettings(&m_aSettings);

        m_xPrevPage->set_help_id(HID_LISTWIZARD_PREVIOUS);
        m_xNextPage->set_help_id(HID_LISTWIZARD_NEXT);
        m_xCancel->set_help_id(HID_LISTWIZARD_CANCEL);
        m_xFinish->set_help_id(HID_LISTWIZARD_FINISH);

        // if we do not need the data source selection page ...
        if (!needDatasourceSelection())
        {   // ... skip it!
            skip();
            m_bHadDataSelection = false;
        }
    }

    bool OListComboWizard::approveControl(sal_Int16 _nClassId)
    {
        switch (_nClassId)
        {
            case FormComponentType::LISTBOX:
                m_bListBox = true;
                setTitleBase(compmodule::ModuleRes(RID_STR_LISTWIZARD_TITLE));
                return true;
            case FormComponentType::COMBOBOX:
                m_bListBox = false;
                setTitleBase(compmodule::ModuleRes(RID_STR_COMBOWIZARD_TITLE));
                return true;
        }
        return false;
    }

    std::unique_ptr<BuilderPage> OListComboWizard::createPage(WizardState _nState)
    {
        OUString sIdent(OUString::number(_nState));
        weld::Container* pPageContainer = m_xAssistant->append_page(sIdent);

        switch (_nState)
        {
            case LCW_STATE_DATASOURCE_SELECTION:
                return std::make_unique<OTableSelectionPage>(pPageContainer, this);
            case LCW_STATE_TABLESELECTION:
                return std::make_unique<OContentTableSelection>(pPageContainer, this);
            case LCW_STATE_FIELDSELECTION:
                return std::make_unique<OContentFieldSelection>(pPageContainer, this);
            case LCW_STATE_FIELDLINK:
                return std::make_unique<OLinkFieldsPage>(pPageContainer, this);
            case LCW_STATE_COMBODBFIELD:
                return std::make_unique<OComboDBFieldPage>(pPageContainer, this);
        }

        return nullptr;
    }

    vcl::WizardTypes::WizardState OListComboWizard::determineNextState( WizardState _nCurrentState ) const
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

    void OListComboWizard::enterState(WizardState _nState)
    {
        OControlWizard::enterState(_nState);

        enableButtons(WizardButtonFlags::PREVIOUS, m_bHadDataSelection ? (LCW_STATE_DATASOURCE_SELECTION < _nState) : LCW_STATE_TABLESELECTION < _nState);
        enableButtons(WizardButtonFlags::NEXT, getFinalState() != _nState);
        if (_nState < getFinalState())
            enableButtons(WizardButtonFlags::FINISH, false);

        if (getFinalState() == _nState)
            defaultButton(WizardButtonFlags::FINISH);
    }


    bool OListComboWizard::leaveState(WizardState _nState)
    {
        if (!OControlWizard::leaveState(_nState))
            return false;

        if (getFinalState() == _nState)
            defaultButton(WizardButtonFlags::NEXT);

        return true;
    }


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
                OUString sQuoteString = xMetaData->getIdentifierQuoteString();
                if (isListBox()) // only when we have a listbox this should be not empty
                    getSettings().sLinkedListField = quoteName(sQuoteString, getSettings().sLinkedListField);

                OUString sCatalog, sSchema, sName;
                ::dbtools::qualifiedNameComponents( xMetaData, getSettings().sListContentTable, sCatalog, sSchema, sName, ::dbtools::EComposeRule::InDataManipulation );
                getSettings().sListContentTable = ::dbtools::composeTableNameForSelect( xConn, sCatalog, sSchema, sName );

                getSettings().sListContentField = quoteName(sQuoteString, getSettings().sListContentField);
            }

            // ListSourceType: SQL
            getContext().xObjectModel->setPropertyValue("ListSourceType", Any(sal_Int32(ListSourceType_SQL)));

            if (isListBox())
            {
                // BoundColumn: 1
                getContext().xObjectModel->setPropertyValue("BoundColumn", Any(sal_Int16(1)));

                Sequence< OUString > aListSource {
                    // build the statement to set as list source
                    OUString("SELECT " +
                        getSettings().sListContentField +  ", " + getSettings().sLinkedListField +
                        " FROM " + getSettings().sListContentTable)
                };
                getContext().xObjectModel->setPropertyValue("ListSource", Any(aListSource));
            }
            else
            {
                // build the statement to set as list source
                OUString sStatement = "SELECT DISTINCT " +
                    getSettings().sListContentField +
                    " FROM " + getSettings().sListContentTable;
                getContext().xObjectModel->setPropertyValue( "ListSource", Any(sStatement));
            }

            // the bound field
            getContext().xObjectModel->setPropertyValue("DataField", Any(getSettings().sLinkedFormField));
        }
        catch(const Exception&)
        {
            OSL_FAIL("OListComboWizard::implApplySettings: could not set the property values for the listbox!");
        }
    }


    bool OListComboWizard::onFinish()
    {
        if ( !OControlWizard::onFinish() )
            return false;

        implApplySettings();
        return true;
    }

    Reference< XNameAccess > OLCPage::getTables() const
    {
        Reference< XConnection > xConn = getFormConnection();
        DBG_ASSERT(xConn.is(), "OLCPage::getTables: should have an active connection when reaching this page!");

        Reference< XTablesSupplier > xSuppTables(xConn, UNO_QUERY);
        Reference< XNameAccess > xTables;
        if (xSuppTables.is())
            xTables = xSuppTables->getTables();

        DBG_ASSERT(xTables.is() || !xConn.is(), "OLCPage::getTables: got no tables from the connection!");

        return xTables;
    }


    Sequence< OUString > OLCPage::getTableFields()
    {
        Reference< XNameAccess > xTables = getTables();
        Sequence< OUString > aColumnNames;
        if (xTables.is())
        {
            try
            {
                // the list table as XColumnsSupplier
                Reference< XColumnsSupplier > xSuppCols;
                xTables->getByName(getSettings().sListContentTable) >>= xSuppCols;
                DBG_ASSERT(xSuppCols.is(), "OLCPage::getTableFields: no columns supplier!");

                // the columns
                Reference< XNameAccess > xColumns;
                if (xSuppCols.is())
                    xColumns = xSuppCols->getColumns();

                // the column names
                if (xColumns.is())
                    aColumnNames = xColumns->getElementNames();
            }
            catch(const Exception&)
            {
                TOOLS_WARN_EXCEPTION( "extensions.dbpilots", "OLinkFieldsPage::initializePage: caught an exception while retrieving the columns");
            }
        }
        return aColumnNames;
    }

    OContentTableSelection::OContentTableSelection(weld::Container* pPage, OListComboWizard* pWizard)
        : OLCPage(pPage, pWizard, "modules/sabpilot/ui/contenttablepage.ui", "TableSelectionPage")
        , m_xSelectTable(m_xBuilder->weld_tree_view("table"))
    {
        enableFormDatasourceDisplay();

        m_xSelectTable->connect_row_activated(LINK(this, OContentTableSelection, OnTableDoubleClicked));
        m_xSelectTable->connect_changed(LINK(this, OContentTableSelection, OnTableSelected));
    }

    OContentTableSelection::~OContentTableSelection()
    {
    }

    void OContentTableSelection::Activate()
    {
        OLCPage::Activate();
        m_xSelectTable->grab_focus();
    }

    bool OContentTableSelection::canAdvance() const
    {
        if (!OLCPage::canAdvance())
            return false;

        return 0 != m_xSelectTable->count_selected_rows();
    }

    IMPL_LINK_NOARG( OContentTableSelection, OnTableSelected, weld::TreeView&, void )
    {
        updateDialogTravelUI();
    }

    IMPL_LINK( OContentTableSelection, OnTableDoubleClicked, weld::TreeView&, _rListBox, bool )
    {
        if (_rListBox.count_selected_rows())
            getDialog()->travelNext();
        return true;
    }

    void OContentTableSelection::initializePage()
    {
        OLCPage::initializePage();

        // fill the list with the table name
        m_xSelectTable->clear();
        try
        {
            Reference< XNameAccess > xTables = getTables();
            Sequence< OUString > aTableNames;
            if (xTables.is())
                aTableNames = xTables->getElementNames();
            fillListBox(*m_xSelectTable, aTableNames);
        }
        catch(const Exception&)
        {
            OSL_FAIL("OContentTableSelection::initializePage: could not retrieve the table names!");
        }

        m_xSelectTable->select_text(getSettings().sListContentTable);
    }


    bool OContentTableSelection::commitPage( ::vcl::WizardTypes::CommitPageReason _eReason )
    {
        if (!OLCPage::commitPage(_eReason))
            return false;

        OListComboSettings& rSettings = getSettings();
        rSettings.sListContentTable = m_xSelectTable->get_selected_text();
        if (rSettings.sListContentTable.isEmpty() && (::vcl::WizardTypes::eTravelBackward != _eReason))
            // need to select a table
            return false;

        return true;
    }

    OContentFieldSelection::OContentFieldSelection(weld::Container* pPage, OListComboWizard* pWizard)
        : OLCPage(pPage, pWizard, "modules/sabpilot/ui/contentfieldpage.ui", "FieldSelectionPage")
        , m_xSelectTableField(m_xBuilder->weld_tree_view("selectfield"))
        , m_xDisplayedField(m_xBuilder->weld_entry("displayfield"))
        , m_xInfo(m_xBuilder->weld_label("info"))
    {
        m_xInfo->set_label(compmodule::ModuleRes( isListBox() ? RID_STR_FIELDINFO_LISTBOX : RID_STR_FIELDINFO_COMBOBOX));
        m_xSelectTableField->connect_changed(LINK(this, OContentFieldSelection, OnFieldSelected));
        m_xSelectTableField->connect_row_activated(LINK(this, OContentFieldSelection, OnTableDoubleClicked));
    }

    OContentFieldSelection::~OContentFieldSelection()
    {
    }

    void OContentFieldSelection::initializePage()
    {
        OLCPage::initializePage();

        // fill the list of fields
        fillListBox(*m_xSelectTableField, getTableFields());

        m_xSelectTableField->select_text(getSettings().sListContentField);
        m_xDisplayedField->set_text(getSettings().sListContentField);
    }

    bool OContentFieldSelection::canAdvance() const
    {
        if (!OLCPage::canAdvance())
            return false;

        return 0 != m_xSelectTableField->count_selected_rows();
    }

    IMPL_LINK_NOARG( OContentFieldSelection, OnTableDoubleClicked, weld::TreeView&, bool )
    {
        if (m_xSelectTableField->count_selected_rows())
            getDialog()->travelNext();
        return true;
    }

    IMPL_LINK_NOARG( OContentFieldSelection, OnFieldSelected, weld::TreeView&, void )
    {
        updateDialogTravelUI();
        m_xDisplayedField->set_text(m_xSelectTableField->get_selected_text());
    }

    bool OContentFieldSelection::commitPage( ::vcl::WizardTypes::CommitPageReason _eReason )
    {
        if (!OLCPage::commitPage(_eReason))
            return false;

        getSettings().sListContentField = m_xSelectTableField->get_selected_text();

        return true;
    }

    OLinkFieldsPage::OLinkFieldsPage(weld::Container* pPage, OListComboWizard* pWizard)
        : OLCPage(pPage, pWizard, "modules/sabpilot/ui/fieldlinkpage.ui", "FieldLinkPage")
        , m_xValueListField(m_xBuilder->weld_combo_box("valuefield"))
        , m_xTableField(m_xBuilder->weld_combo_box("listtable"))
    {
        m_xValueListField->connect_changed(LINK(this, OLinkFieldsPage, OnSelectionModified));
        m_xTableField->connect_changed(LINK(this, OLinkFieldsPage, OnSelectionModified));
    }

    OLinkFieldsPage::~OLinkFieldsPage()
    {
    }

    void OLinkFieldsPage::Activate()
    {
        OLCPage::Activate();
        m_xValueListField->grab_focus();
    }

    void OLinkFieldsPage::initializePage()
    {
        OLCPage::initializePage();

        // fill the value list
        fillListBox(*m_xValueListField, getContext().aFieldNames);
        // fill the table field list
        fillListBox(*m_xTableField, getTableFields());

        // the initial selections
        m_xValueListField->set_entry_text(getSettings().sLinkedFormField);
        m_xTableField->set_entry_text(getSettings().sLinkedListField);

        implCheckFinish();
    }

    bool OLinkFieldsPage::canAdvance() const
    {
        // we're on the last page here, no travelNext allowed ...
        return false;
    }

    void OLinkFieldsPage::implCheckFinish()
    {
        bool bInvalidSelection = (-1 == m_xValueListField->find_text(m_xValueListField->get_active_text()));
        bInvalidSelection |= (-1 == m_xTableField->find_text(m_xTableField->get_active_text()));
        getDialog()->enableButtons(WizardButtonFlags::FINISH, !bInvalidSelection);
    }

    IMPL_LINK_NOARG(OLinkFieldsPage, OnSelectionModified, weld::ComboBox&, void)
    {
        implCheckFinish();
    }

    bool OLinkFieldsPage::commitPage( ::vcl::WizardTypes::CommitPageReason _eReason )
    {
        if (!OLCPage::commitPage(_eReason))
            return false;

        getSettings().sLinkedFormField = m_xValueListField->get_active_text();
        getSettings().sLinkedListField = m_xTableField->get_active_text();

        return true;
    }

    OComboDBFieldPage::OComboDBFieldPage(weld::Container* pPage, OControlWizard* pWizard)
        : ODBFieldPage(pPage, pWizard)
    {
        setDescriptionText(compmodule::ModuleRes(RID_STR_COMBOWIZ_DBFIELD));
    }

    OUString& OComboDBFieldPage::getDBFieldSetting()
    {
        return static_cast<OListComboWizard*>(getDialog())->getSettings().sLinkedFormField;
    }

    void OComboDBFieldPage::Activate()
    {
        ODBFieldPage::Activate();
        getDialog()->enableButtons(WizardButtonFlags::FINISH, true);
    }

    bool OComboDBFieldPage::canAdvance() const
    {
        // we're on the last page here, no travelNext allowed ...
        return false;
    }

}   // namespace dbp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
