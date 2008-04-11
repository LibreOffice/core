/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tableselectionpage.cxx,v $
 * $Revision: 1.10 $
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
#include "tableselectionpage.hxx"
#include "abptypes.hxx"
#include "addresssettings.hxx"
#include "abspilot.hxx"
#include <tools/debug.hxx>

//.........................................................................
namespace abp
{
//.........................................................................

    //=====================================================================
    //= TableSelectionPage
    //=====================================================================
    //---------------------------------------------------------------------
    TableSelectionPage::TableSelectionPage( OAddessBookSourcePilot* _pParent )
        :AddressBookSourcePage(_pParent, ModuleRes(RID_PAGE_TABLESELECTION_AB))
        ,m_aLabel           ( this, ModuleRes( FL_TOOMUCHTABLES ) )
        ,m_aTableList       ( this, ModuleRes( LB_TABLELIST ) )
    {
        FreeResource();

        m_aTableList.SetSelectHdl( LINK( this, TableSelectionPage, OnTableSelected ) );
        m_aTableList.SetDoubleClickHdl( LINK( this, TableSelectionPage, OnTableDoubleClicked ) );
    }

    //---------------------------------------------------------------------
    void TableSelectionPage::ActivatePage()
    {
        AddressBookSourcePage::ActivatePage();

        m_aTableList.GrabFocus();
    }

    //---------------------------------------------------------------------
    void TableSelectionPage::DeactivatePage()
    {
        AddressBookSourcePage::DeactivatePage();
    }

    //---------------------------------------------------------------------
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
        for (   ConstStringBagIterator aTables = aTableNames.begin();
                aTables != aTableNames.end();
                ++aTables
            )
            m_aTableList.InsertEntry( *aTables );

        // initially select the proper table
        m_aTableList.SelectEntry( rSettings.sSelectedTable );
    }

    //---------------------------------------------------------------------
    IMPL_LINK( TableSelectionPage, OnTableDoubleClicked, void*, /*NOTINTERESTEDIN*/ )
    {
        if ( 1 == m_aTableList.GetSelectEntryCount() )
            getDialog()->travelNext();

        return 0L;
    }

    //---------------------------------------------------------------------
    IMPL_LINK( TableSelectionPage, OnTableSelected, void*, /*NOTINTERESTEDIN*/ )
    {
        updateDialogTravelUI();
        return 0L;
    }

    //---------------------------------------------------------------------
    sal_Bool TableSelectionPage::commitPage( CommitPageReason _eReason )
    {
        if (!AddressBookSourcePage::commitPage(_eReason))
            return sal_False;

        AddressSettings& rSettings = getSettings();
        rSettings.sSelectedTable = m_aTableList.GetSelectEntry();

        return sal_True;
    }

    //---------------------------------------------------------------------
    bool TableSelectionPage::canAdvance() const
    {
        return  AddressBookSourcePage::canAdvance()
            &&  ( 0 < m_aTableList.GetSelectEntryCount() );
    }

//.........................................................................
}   // namespace abp
//.........................................................................

