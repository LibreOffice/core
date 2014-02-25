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



    //= TableSelectionPage


    TableSelectionPage::TableSelectionPage( OAddessBookSourcePilot* _pParent )
        :AddressBookSourcePage(_pParent, ModuleRes(RID_PAGE_TABLESELECTION_AB))
        ,m_aLabel           ( this, ModuleRes( FL_TOOMUCHTABLES ) )
        ,m_aTableList       ( this, ModuleRes( LB_TABLELIST ) )
    {
        FreeResource();

        m_aTableList.SetSelectHdl( LINK( this, TableSelectionPage, OnTableSelected ) );
        m_aTableList.SetDoubleClickHdl( LINK( this, TableSelectionPage, OnTableDoubleClicked ) );
    }


    void TableSelectionPage::ActivatePage()
    {
        AddressBookSourcePage::ActivatePage();

        m_aTableList.GrabFocus();
    }


    void TableSelectionPage::DeactivatePage()
    {
        AddressBookSourcePage::DeactivatePage();
    }


    void TableSelectionPage::initializePage()
    {
        AddressBookSourcePage::initializePage();

        const AddressSettings& rSettings = getSettings();

        m_aTableList.Clear();

        // get the table names
        const StringBag& aTableNames = getDialog()->getDataSource().getTableNames();
        DBG_ASSERT( aTableNames.size() > 1, "TableSelectionPage::initializePage: to be called for more than one table only!");
            // this page should never bother the user if there is 1 or less tables.

        // fill the list
        for (   StringBag::const_iterator aTables = aTableNames.begin();
                aTables != aTableNames.end();
                ++aTables
            )
            m_aTableList.InsertEntry( *aTables );

        // initially select the proper table
        m_aTableList.SelectEntry( rSettings.sSelectedTable );
    }


    IMPL_LINK( TableSelectionPage, OnTableDoubleClicked, void*, /*NOTINTERESTEDIN*/ )
    {
        if ( 1 == m_aTableList.GetSelectEntryCount() )
            getDialog()->travelNext();

        return 0L;
    }


    IMPL_LINK( TableSelectionPage, OnTableSelected, void*, /*NOTINTERESTEDIN*/ )
    {
        updateDialogTravelUI();
        return 0L;
    }


    sal_Bool TableSelectionPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
    {
        if (!AddressBookSourcePage::commitPage(_eReason))
            return sal_False;

        AddressSettings& rSettings = getSettings();
        rSettings.sSelectedTable = m_aTableList.GetSelectEntry();

        return sal_True;
    }


    bool TableSelectionPage::canAdvance() const
    {
        return  AddressBookSourcePage::canAdvance()
            &&  ( 0 < m_aTableList.GetSelectEntryCount() );
    }


}   // namespace abp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
