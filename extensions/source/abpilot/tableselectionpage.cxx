/*************************************************************************
 *
 *  $RCSfile: tableselectionpage.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 16:00:52 $
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

#ifndef EXTENSIONS_ABP_TABLESELECTIONPAGE_HXX
#include "tableselectionpage.hxx"
#endif
#ifndef EXTENSIONS_ABP_ABPTYPES_HXX
#include "abptypes.hxx"
#endif
#ifndef EXTENSIONS_ABP_ADDRESSSETTINGS_HXX
#include "addresssettings.hxx"
#endif
#ifndef EXTENSIONS_ABSPILOT_HXX
#include "abspilot.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//.........................................................................
namespace abp
{
//.........................................................................

    //=====================================================================
    //= TableSelectionPage
    //=====================================================================
    //---------------------------------------------------------------------
    TableSelectionPage::TableSelectionPage( OAddessBookSourcePilot* _pParent )
        :AddressBookSourcePage(_pParent, ModuleRes(RID_PAGE_TABLESELECTION))
        ,m_aLabel           ( this, ResId( FL_TOOMUCHTABLES ) )
        ,m_aTableList       ( this, ResId( LB_TABLELIST ) )
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
    IMPL_LINK( TableSelectionPage, OnTableDoubleClicked, void*, NOTINTERESTEDIN )
    {
        if ( 1 == m_aTableList.GetSelectEntryCount() )
            getDialog()->travelNext();

        return 0L;
    }

    //---------------------------------------------------------------------
    IMPL_LINK( TableSelectionPage, OnTableSelected, void*, NOTINTERESTEDIN )
    {
        implCheckNextButton();
        return 0L;
    }

    //---------------------------------------------------------------------
    sal_Bool TableSelectionPage::commitPage(COMMIT_REASON _eReason)
    {
        if (!AddressBookSourcePage::commitPage(_eReason))
            return sal_False;

        AddressSettings& rSettings = getSettings();
        rSettings.sSelectedTable = m_aTableList.GetSelectEntry();

        return sal_True;
    }

    //---------------------------------------------------------------------
    sal_Bool TableSelectionPage::determineNextButtonState()
    {
        return  AddressBookSourcePage::determineNextButtonState()
            &&  ( 0 < m_aTableList.GetSelectEntryCount() );
    }

//.........................................................................
}   // namespace abp
//.........................................................................

