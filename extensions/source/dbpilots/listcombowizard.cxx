/*************************************************************************
 *
 *  $RCSfile: listcombowizard.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2001-02-23 15:20:11 $
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
#include "listcombowizard.hxx"
#endif
#ifndef _EXTENSIONS_DBP_COMMONPAGESDBP_HXX_
#include "commonpagesdbp.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_LISTSOURCETYPE_HPP_
#include <com/sun/star/form/ListSourceType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif

#define LCW_STATE_DATASOURCE_SELECTION  0
#define LCW_STATE_TABLESELECTION        1
#define LCW_STATE_FIELDSELECTION        2
#define LCW_STATE_FIELDLINK             3

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

        // if we do not need the data source selection page ...
        if (!needDatasourceSelection())
        {   // ... skip it!
            skip(1);
            m_bHadDataSelection = sal_False;
        }
    }

    //---------------------------------------------------------------------
    sal_Bool OListComboWizard::approveControlType(sal_Int16 _nClassId)
    {
        switch (_nClassId)
        {
            case FormComponentType::LISTBOX:
                m_bListBox = sal_True;
                return sal_True;
            case FormComponentType::COMBOBOX:
                m_bListBox = sal_False;
                return sal_True;
        }
        return sal_False;
    }

    //---------------------------------------------------------------------
    OWizardPage* OListComboWizard::createPage(sal_uInt16 _nState)
    {
        // TODO
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
        }

        return NULL;
    }

    //---------------------------------------------------------------------
    sal_uInt16 OListComboWizard::determineNextState(sal_uInt16 _nCurrentState)
    {
        switch (_nCurrentState)
        {
            case LCW_STATE_DATASOURCE_SELECTION:
                return LCW_STATE_TABLESELECTION;
            case LCW_STATE_TABLESELECTION:
                return LCW_STATE_FIELDSELECTION;
            case LCW_STATE_FIELDSELECTION:
                return LCW_STATE_FIELDLINK;
        }

        // TODO
        return WZS_INVALID_STATE;
    }

    //---------------------------------------------------------------------
    void OListComboWizard::enterState(sal_uInt16 _nState)
    {
        OControlWizard::enterState(_nState);

        enableButtons(WZB_PREVIOUS, m_bHadDataSelection ? (LCW_STATE_DATASOURCE_SELECTION < _nState) : LCW_STATE_TABLESELECTION < _nState);
        enableButtons(WZB_NEXT, LCW_STATE_FIELDLINK != _nState);
        if (_nState < LCW_STATE_FIELDLINK)
            enableButtons(WZB_FINISH, sal_False);

        if (LCW_STATE_FIELDLINK == _nState)
            defaultButton(WZB_FINISH);
    }

    //---------------------------------------------------------------------
    sal_Bool OListComboWizard::leaveState(sal_uInt16 _nState)
    {
        if (!OControlWizard::leaveState(_nState))
            return sal_False;

        if (LCW_STATE_FIELDLINK == _nState)
            defaultButton(WZB_NEXT);

        return sal_True;
    }

    //---------------------------------------------------------------------
    void OListComboWizard::implApplySettings()
    {
        try
        {
            if (isListBox())
            {
                // some defaults:
                // ListSourceType: SQL
                getContext().xObjectModel->setPropertyValue(::rtl::OUString::createFromAscii("ListSourceType"), makeAny((sal_Int32)ListSourceType_SQL));
                // BoundColumn: 1
                getContext().xObjectModel->setPropertyValue(::rtl::OUString::createFromAscii("BoundColumn"), makeAny((sal_Int16)1));

                // for quoting identifiers, we need the connection meta data
                Reference< XConnection > xConn;
                getContext().xForm->getPropertyValue(::rtl::OUString::createFromAscii("ActiveConnection")) >>= xConn;
                DBG_ASSERT(xConn.is(), "OListComboWizard::implApplySettings: no connection, unable to quote!");
                Reference< XDatabaseMetaData > xMetaData;
                if (xConn.is())
                    xMetaData = xConn->getMetaData();

                // do some quotings
                if (xMetaData.is())
                {
                    getSettings().sLinkedListField = quoteTableName(xMetaData, getSettings().sLinkedListField);
                    getSettings().sListContentTable = quoteTableName(xMetaData, getSettings().sListContentTable);
                    getSettings().sListContentField = quoteTableName(xMetaData, getSettings().sListContentField);
                }

                // build the statement to set as list source
                String sStatement;
                sStatement.AppendAscii("SELECT ");
                sStatement += getSettings().sListContentField;
                sStatement.AppendAscii(", ");
                sStatement += getSettings().sLinkedListField;
                sStatement.AppendAscii(" FROM ");
                sStatement += getSettings().sListContentTable;
                    // TODO: Identifier quoting
                Sequence< ::rtl::OUString > aListSource(1);
                aListSource[0] = sStatement;
                getContext().xObjectModel->setPropertyValue(::rtl::OUString::createFromAscii("ListSource"), makeAny(aListSource));

                // the bound field
                getContext().xObjectModel->setPropertyValue(::rtl::OUString::createFromAscii("DataField"), makeAny(::rtl::OUString(getSettings().sLinkedFormField)));
            }
            else
            {
                // TODO
            }
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

        implApplySettings();
        return sal_True;
    }

    //=====================================================================
    //= OLCPage
    //=====================================================================
    //---------------------------------------------------------------------
    Reference< XNameAccess > OLCPage::getTables(sal_Bool _bNeedIt)
    {
        Reference< XConnection > xConn;
        getContext().xForm->getPropertyValue(::rtl::OUString::createFromAscii("ActiveConnection")) >>= xConn;
        DBG_ASSERT(!_bNeedIt || xConn.is(), "OLCPage::getTables: should have an active connection when reaching this page!");

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
        ,m_aFrame               (this, ResId(FL_FRAME))
        ,m_aSelectTableLabel    (this, ResId(FT_SELECTTABLE_LABEL))
        ,m_aSelectTable         (this, ResId(LB_SELECTTABLE))
    {
        FreeResource();

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
    sal_Bool OContentTableSelection::determineNextButtonState()
    {
        if (!OLCPage::determineNextButtonState())
            return sal_False;

        return 0 != m_aSelectTable.GetSelectEntryCount();
    }

    //---------------------------------------------------------------------
    IMPL_LINK( OContentTableSelection, OnTableSelected, ListBox*, _pListBox )
    {
        implCheckNextButton();
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
    sal_Bool OContentTableSelection::commitPage(COMMIT_REASON _eReason)
    {
        if (!OLCPage::commitPage(_eReason))
            return sal_False;

        OListComboSettings& rSettings = getSettings();
        rSettings.sListContentTable = m_aSelectTable.GetSelectEntry();
        if (!rSettings.sListContentTable.Len() && (CR_TRAVEL_PREVIOUS != _eReason))
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
        ,m_aFrame               (this, ResId(FL_FRAME))
        ,m_aTableFields         (this, ResId(FT_TABLEFIELDS))
        ,m_aSelectTableField    (this, ResId(LB_SELECTFIELD))
        ,m_aDisplayedFieldLabel (this, ResId(FT_DISPLAYEDFIELD))
        ,m_aDisplayedField      (this, ResId(ET_DISPLAYEDFIELD))
        ,m_aInfo                (this, ResId(FT_CONTENTFIELD_INFO))
    {
        m_aInfo.SetText(String(ResId( isListBox() ? STR_FIELDINFO_LISTBOX : STR_FIELDINFO_COMBOBOX)));
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
    sal_Bool OContentFieldSelection::determineNextButtonState()
    {
        if (!OLCPage::determineNextButtonState())
            return sal_False;

        return 0 != m_aSelectTableField.GetSelectEntryCount();
    }

    //---------------------------------------------------------------------
    IMPL_LINK( OContentFieldSelection, OnTableDoubleClicked, ListBox*, NOTINTERESTEDIN )
    {
        if (m_aSelectTableField.GetSelectEntryCount())
            getDialog()->travelNext();
        return 0L;
    }

    //---------------------------------------------------------------------
    IMPL_LINK( OContentFieldSelection, OnFieldSelected, ListBox*, NOTINTERESTEDIN )
    {
        implCheckNextButton();
        m_aDisplayedField.SetText(m_aSelectTableField.GetSelectEntry());
        return 0L;
    }

    //---------------------------------------------------------------------
    sal_Bool OContentFieldSelection::commitPage(COMMIT_REASON _eReason)
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
        ,m_aFrame               (this, ResId(FL_FRAME))
        ,m_aValueListFieldLabel (this, ResId(FT_VALUELISTFIELD))
        ,m_aValueListField      (this, ResId(CMB_VALUELISTFIELD))
        ,m_aTableFieldLabel     (this, ResId(FT_TABLEFIELD))
        ,m_aTableField          (this, ResId(CMB_TABLEFIELD))
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
    sal_Bool OLinkFieldsPage::determineNextButtonState()
    {
        // we're on the last page here, no travelNext allowed ...
        return sal_False;
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
    sal_Bool OLinkFieldsPage::commitPage(COMMIT_REASON _eReason)
    {
        if (!OLCPage::commitPage(_eReason))
            return sal_False;

        getSettings().sLinkedFormField = m_aValueListField.GetText();
        getSettings().sLinkedListField = m_aTableField.GetText();

        return sal_True;
    }


//.........................................................................
}   // namespace dbp
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 21.02.01 15:51:07  fs
 ************************************************************************/

