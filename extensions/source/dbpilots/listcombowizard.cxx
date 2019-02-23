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
#include <tools/diagnose_ex.h>
#include <connectivity/dbtools.hxx>
#include <helpids.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>


namespace dbp
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::form;
    using namespace ::svt;
    using namespace ::dbtools;

    OListComboWizard::OListComboWizard( vcl::Window* _pParent,
            const Reference< XPropertySet >& _rxObjectModel, const Reference< XComponentContext >& _rxContext )
        :OControlWizard(_pParent, _rxObjectModel, _rxContext)
        ,m_bListBox(false)
        ,m_bHadDataSelection(true)
    {
        initControlSettings(&m_aSettings);

        m_pPrevPage->SetHelpId(HID_LISTWIZARD_PREVIOUS);
        m_pNextPage->SetHelpId(HID_LISTWIZARD_NEXT);
        m_pCancel->SetHelpId(HID_LISTWIZARD_CANCEL);
        m_pFinish->SetHelpId(HID_LISTWIZARD_FINISH);

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


    VclPtr<TabPage> OListComboWizard::createPage(WizardState _nState)
    {
        switch (_nState)
        {
            case LCW_STATE_DATASOURCE_SELECTION:
                return VclPtr<OTableSelectionPage>::Create(this);
            case LCW_STATE_TABLESELECTION:
                return VclPtr<OContentTableSelection>::Create(this);
            case LCW_STATE_FIELDSELECTION:
                return VclPtr<OContentFieldSelection>::Create(this);
            case LCW_STATE_FIELDLINK:
                return VclPtr<OLinkFieldsPage>::Create(this);
            case LCW_STATE_COMBODBFIELD:
                return VclPtr<OComboDBFieldPage>::Create(this);
        }

        return VclPtr<TabPage>();
    }


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
            getContext().xObjectModel->setPropertyValue("ListSourceType", makeAny(sal_Int32(ListSourceType_SQL)));

            if (isListBox())
            {
                // BoundColumn: 1
                getContext().xObjectModel->setPropertyValue("BoundColumn", makeAny(sal_Int16(1)));

                // build the statement to set as list source
                OUString sStatement = "SELECT " +
                    getSettings().sListContentField +  ", " + getSettings().sLinkedListField +
                    " FROM " + getSettings().sListContentTable;
                Sequence< OUString > aListSource { sStatement };
                getContext().xObjectModel->setPropertyValue("ListSource", makeAny(aListSource));
            }
            else
            {
                // build the statement to set as list source
                OUString sStatement = "SELECT DISTINCT " +
                    getSettings().sListContentField +
                    " FROM " + getSettings().sListContentTable;
                getContext().xObjectModel->setPropertyValue( "ListSource", makeAny(sStatement));
            }

            // the bound field
            getContext().xObjectModel->setPropertyValue("DataField", makeAny(getSettings().sLinkedFormField));
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

    Reference< XNameAccess > OLCPage::getTables()
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
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN( "extensions.dbpilots", "OLinkFieldsPage::initializePage: caught an exception while retrieving the columns! " << exceptionToString(ex));
            }
        }
        return aColumnNames;
    }

    OContentTableSelection::OContentTableSelection( OListComboWizard* _pParent )
        :OLCPage(_pParent, "TableSelectionPage", "modules/sabpilot/ui/contenttablepage.ui")
    {
        get(m_pSelectTable, "table");

        enableFormDatasourceDisplay();

        m_pSelectTable->SetDoubleClickHdl(LINK(this, OContentTableSelection, OnTableDoubleClicked));
        m_pSelectTable->SetSelectHdl(LINK(this, OContentTableSelection, OnTableSelected));
    }

    OContentTableSelection::~OContentTableSelection()
    {
        disposeOnce();
    }

    void OContentTableSelection::dispose()
    {
        m_pSelectTable.clear();
        OLCPage::dispose();
    }

    void OContentTableSelection::ActivatePage()
    {
        OLCPage::ActivatePage();
        m_pSelectTable->GrabFocus();
    }


    bool OContentTableSelection::canAdvance() const
    {
        if (!OLCPage::canAdvance())
            return false;

        return 0 != m_pSelectTable->GetSelectedEntryCount();
    }


    IMPL_LINK_NOARG( OContentTableSelection, OnTableSelected, ListBox&, void )
    {
        updateDialogTravelUI();
    }


    IMPL_LINK( OContentTableSelection, OnTableDoubleClicked, ListBox&, _rListBox, void )
    {
        if (_rListBox.GetSelectedEntryCount())
            getDialog()->travelNext();
    }


    void OContentTableSelection::initializePage()
    {
        OLCPage::initializePage();

        // fill the list with the table name
        m_pSelectTable->Clear();
        try
        {
            Reference< XNameAccess > xTables = getTables();
            Sequence< OUString > aTableNames;
            if (xTables.is())
                aTableNames = xTables->getElementNames();
            fillListBox(*m_pSelectTable, aTableNames);
        }
        catch(const Exception&)
        {
            OSL_FAIL("OContentTableSelection::initializePage: could not retrieve the table names!");
        }

        m_pSelectTable->SelectEntry(getSettings().sListContentTable);
    }


    bool OContentTableSelection::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
    {
        if (!OLCPage::commitPage(_eReason))
            return false;

        OListComboSettings& rSettings = getSettings();
        rSettings.sListContentTable = m_pSelectTable->GetSelectedEntry();
        if (rSettings.sListContentTable.isEmpty() && (::svt::WizardTypes::eTravelBackward != _eReason))
            // need to select a table
            return false;

        return true;
    }

    OContentFieldSelection::OContentFieldSelection( OListComboWizard* _pParent )
        :OLCPage(_pParent, "FieldSelectionPage", "modules/sabpilot/ui/contentfieldpage.ui")
    {
        get(m_pSelectTableField, "selectfield");
        get(m_pDisplayedField, "displayfield");
        get(m_pInfo, "info");
        m_pInfo->SetText(compmodule::ModuleRes( isListBox() ? RID_STR_FIELDINFO_LISTBOX : RID_STR_FIELDINFO_COMBOBOX));
        m_pSelectTableField->SetSelectHdl(LINK(this, OContentFieldSelection, OnFieldSelected));
        m_pSelectTableField->SetDoubleClickHdl(LINK(this, OContentFieldSelection, OnTableDoubleClicked));
    }

    OContentFieldSelection::~OContentFieldSelection()
    {
        disposeOnce();
    }

    void OContentFieldSelection::dispose()
    {
        m_pSelectTableField.clear();
        m_pDisplayedField.clear();
        m_pInfo.clear();
        OLCPage::dispose();
    }


    void OContentFieldSelection::initializePage()
    {
        OLCPage::initializePage();

        // fill the list of fields
        fillListBox(*m_pSelectTableField, getTableFields());

        m_pSelectTableField->SelectEntry(getSettings().sListContentField);
        m_pDisplayedField->SetText(getSettings().sListContentField);
    }


    bool OContentFieldSelection::canAdvance() const
    {
        if (!OLCPage::canAdvance())
            return false;

        return 0 != m_pSelectTableField->GetSelectedEntryCount();
    }


    IMPL_LINK_NOARG( OContentFieldSelection, OnTableDoubleClicked, ListBox&, void )
    {
        if (m_pSelectTableField->GetSelectedEntryCount())
            getDialog()->travelNext();
    }


    IMPL_LINK_NOARG( OContentFieldSelection, OnFieldSelected, ListBox&, void )
    {
        updateDialogTravelUI();
        m_pDisplayedField->SetText(m_pSelectTableField->GetSelectedEntry());
    }


    bool OContentFieldSelection::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
    {
        if (!OLCPage::commitPage(_eReason))
            return false;

        getSettings().sListContentField = m_pSelectTableField->GetSelectedEntry();

        return true;
    }

    OLinkFieldsPage::OLinkFieldsPage( OListComboWizard* _pParent )
        :OLCPage(_pParent, "FieldLinkPage", "modules/sabpilot/ui/fieldlinkpage.ui")
    {
        get(m_pValueListField, "valuefield");
        get(m_pTableField, "listtable");

        m_pValueListField->SetModifyHdl(LINK(this, OLinkFieldsPage, OnSelectionModified));
        m_pTableField->SetModifyHdl(LINK(this, OLinkFieldsPage, OnSelectionModified));
        m_pValueListField->SetSelectHdl(LINK(this, OLinkFieldsPage, OnSelectionModifiedCombBox));
        m_pTableField->SetSelectHdl(LINK(this, OLinkFieldsPage, OnSelectionModifiedCombBox));
    }

    OLinkFieldsPage::~OLinkFieldsPage()
    {
        disposeOnce();
    }

    void OLinkFieldsPage::dispose()
    {
        m_pValueListField.clear();
        m_pTableField.clear();
        OLCPage::dispose();
    }

    void OLinkFieldsPage::ActivatePage()
    {
        OLCPage::ActivatePage();
        m_pValueListField->GrabFocus();
    }


    void OLinkFieldsPage::initializePage()
    {
        OLCPage::initializePage();

        // fill the value list
        fillListBox(*m_pValueListField, getContext().aFieldNames);
        // fill the table field list
        fillListBox(*m_pTableField, getTableFields());

        // the initial selections
        m_pValueListField->SetText(getSettings().sLinkedFormField);
        m_pTableField->SetText(getSettings().sLinkedListField);

        implCheckFinish();
    }


    bool OLinkFieldsPage::canAdvance() const
    {
        // we're on the last page here, no travelNext allowed ...
        return false;
    }


    void OLinkFieldsPage::implCheckFinish()
    {
        bool bInvalidSelection = (COMBOBOX_ENTRY_NOTFOUND == m_pValueListField->GetEntryPos(m_pValueListField->GetText()));
        bInvalidSelection |= (COMBOBOX_ENTRY_NOTFOUND == m_pTableField->GetEntryPos(m_pTableField->GetText()));
        getDialog()->enableButtons(WizardButtonFlags::FINISH, !bInvalidSelection);
    }


    IMPL_LINK_NOARG(OLinkFieldsPage, OnSelectionModified, Edit&, void)
    {
        implCheckFinish();
    }

    IMPL_LINK_NOARG(OLinkFieldsPage, OnSelectionModifiedCombBox, ComboBox&, void)
    {
        implCheckFinish();
    }

    bool OLinkFieldsPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
    {
        if (!OLCPage::commitPage(_eReason))
            return false;

        getSettings().sLinkedFormField = m_pValueListField->GetText();
        getSettings().sLinkedListField = m_pTableField->GetText();

        return true;
    }

    OComboDBFieldPage::OComboDBFieldPage( OControlWizard* _pParent )
        :ODBFieldPage(_pParent)
    {
        setDescriptionText(compmodule::ModuleRes(RID_STR_COMBOWIZ_DBFIELD));
    }

    OUString& OComboDBFieldPage::getDBFieldSetting()
    {
        return static_cast<OListComboWizard*>(getDialog())->getSettings().sLinkedFormField;
    }

    void OComboDBFieldPage::ActivatePage()
    {
        ODBFieldPage::ActivatePage();
        getDialog()->enableButtons(WizardButtonFlags::FINISH, true);
    }

    bool OComboDBFieldPage::canAdvance() const
    {
        // we're on the last page here, no travelNext allowed ...
        return false;
    }

}   // namespace dbp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
