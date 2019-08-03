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

#include "tableselectionpage.hxx"
#include "abptypes.hxx"
#include "addresssettings.hxx"
#include "abspilot.hxx"
#include <tools/debug.hxx>


namespace abp
{

    TableSelectionPage::TableSelectionPage( OAddressBookSourcePilot* _pParent )
        :AddressBookSourcePage(_pParent, "SelectTablePage",
          "modules/sabpilot/ui/selecttablepage.ui")
    {
        get(m_pTableList, "table");
        m_pTableList->SetSelectHdl( LINK( this, TableSelectionPage, OnTableSelected ) );
        m_pTableList->SetDoubleClickHdl( LINK( this, TableSelectionPage, OnTableDoubleClicked ) );
    }

    TableSelectionPage::~TableSelectionPage()
    {
        disposeOnce();
    }

    void TableSelectionPage::dispose()
    {
        m_pTableList.clear();
        AddressBookSourcePage::dispose();
    }

    void TableSelectionPage::ActivatePage()
    {
        AddressBookSourcePage::ActivatePage();

        m_pTableList->GrabFocus();
    }


    void TableSelectionPage::initializePage()
    {
        AddressBookSourcePage::initializePage();

        const AddressSettings& rSettings = getSettings();

        m_pTableList->Clear();

        // get the table names
        const StringBag& aTableNames = getDialog()->getDataSource().getTableNames();
        DBG_ASSERT( aTableNames.size() > 1, "TableSelectionPage::initializePage: to be called for more than one table only!");
            // this page should never bother the user if there is 1 or less tables.

        // fill the list
        for (auto const& tableName : aTableNames)
            m_pTableList->InsertEntry(tableName);

        // initially select the proper table
        m_pTableList->SelectEntry( rSettings.sSelectedTable );
    }


    IMPL_LINK_NOARG( TableSelectionPage, OnTableDoubleClicked, ListBox&, void )
    {
        if ( 1 == m_pTableList->GetSelectedEntryCount() )
            getDialog()->travelNext();
    }


    IMPL_LINK_NOARG( TableSelectionPage, OnTableSelected, ListBox&, void )
    {
        updateDialogTravelUI();
    }


    bool TableSelectionPage::commitPage( ::vcl::WizardTypes::CommitPageReason _eReason )
    {
        if (!AddressBookSourcePage::commitPage(_eReason))
            return false;

        AddressSettings& rSettings = getSettings();
        rSettings.sSelectedTable = m_pTableList->GetSelectedEntry();

        return true;
    }


    bool TableSelectionPage::canAdvance() const
    {
        return  AddressBookSourcePage::canAdvance()
            &&  ( 0 < m_pTableList->GetSelectedEntryCount() );
    }


}   // namespace abp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
