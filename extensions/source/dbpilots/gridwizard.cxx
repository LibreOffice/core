/*************************************************************************
 *
 *  $RCSfile: gridwizard.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-05 14:52:24 $
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

#ifndef _EXTENSIONS_DBP_GRIDWIZARD_HXX_
#include "gridwizard.hxx"
#endif

#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _COM_SUN_STAR_FORM_XGRIDCOLUMNFACTORY_HPP_
#include <com/sun/star/form/XGridColumnFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _EXTENSIONS_DBP_DBPTOOLS_HXX_
#include "dbptools.hxx"
#endif

#define GW_STATE_DATASOURCE_SELECTION   0
#define GW_STATE_FIELDSELECTION         1

//.........................................................................
namespace dbp
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::form;
    using namespace ::svt;

    //=====================================================================
    //= OGridWizard
    //=====================================================================
    //---------------------------------------------------------------------
    OGridWizard::OGridWizard( Window* _pParent,
            const Reference< XPropertySet >& _rxObjectModel, const Reference< XMultiServiceFactory >& _rxORB )
        :OControlWizard(_pParent, ModuleRes(RID_DLG_GRIDWIZARD), _rxObjectModel, _rxORB)
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
    sal_Bool OGridWizard::approveControl(sal_Int16 _nClassId)
    {
        if (FormComponentType::GRIDCONTROL != _nClassId)
            return sal_False;

        Reference< XGridColumnFactory > xColumnFactory(getContext().xObjectModel, UNO_QUERY);
        if (!xColumnFactory.is())
            return sal_False;

        return sal_True;
    }

    //---------------------------------------------------------------------
    void OGridWizard::implApplySettings()
    {
        const OControlWizardContext& rContext = getContext();

        // the factory for the columns
        Reference< XGridColumnFactory > xColumnFactory(rContext.xObjectModel, UNO_QUERY);
        DBG_ASSERT(xColumnFactory.is(), "OGridWizard::implApplySettings: should never have made it 'til here!");
            // (if we're here, what the hell happened in approveControl??)

        // the container for the columns
        Reference< XNameContainer > xColumnContainer(rContext.xObjectModel, UNO_QUERY);
        DBG_ASSERT(xColumnContainer.is(), "OGridWizard::implApplySettings: no container!");

        if (!xColumnFactory.is() || !xColumnContainer.is())
            return;

        static const ::rtl::OUString s_sFieldTypeProperty   = ::rtl::OUString::createFromAscii("Type");
        static const ::rtl::OUString s_sDataFieldProperty   = ::rtl::OUString::createFromAscii("DataField");
        static const ::rtl::OUString s_sLabelProperty       = ::rtl::OUString::createFromAscii("Label");
        static const ::rtl::OUString s_sWidthProperty       = ::rtl::OUString::createFromAscii("Width");
        static const ::rtl::OUString s_sEmptyString;

        // collect "descriptors" for the to-be-created (grid)columns
        DECLARE_STL_VECTOR( ::rtl::OUString, StringArray );
        StringArray aColumnServiceNames;    // service names to be used with the XGridColumnFactory
        StringArray aColumnLabelPostfixes;  // postfixes to append to the column labels
        StringArray aFormFieldNames;        // data field names

        aColumnServiceNames.reserve(getSettings().aSelectedFields.getLength());
        aColumnLabelPostfixes.reserve(getSettings().aSelectedFields.getLength());
        aFormFieldNames.reserve(getSettings().aSelectedFields.getLength());

        // loop through the selected field names
        const ::rtl::OUString* pSelectedFields = getSettings().aSelectedFields.getConstArray();
        const ::rtl::OUString* pEnd = pSelectedFields + getSettings().aSelectedFields.getLength();
        for (;pSelectedFields < pEnd; ++pSelectedFields)
        {
            // get the information for the selected column
            sal_Int32 nFieldType = DataType::OTHER;
            try
            {
                Reference< XPropertySet > xColumn;
                rContext.xFields->getByName(*pSelectedFields) >>= xColumn;
                xColumn->getPropertyValue(s_sFieldTypeProperty) >>= nFieldType;
            }
            catch(Exception&)
            {
                DBG_ERROR("OGridWizard::implApplySettings: unexpected exception while gathering column information!");
            }

            aFormFieldNames.push_back(*pSelectedFields);
            switch (nFieldType)
            {
                case DataType::BIT:
                    aColumnServiceNames.push_back(::rtl::OUString::createFromAscii("CheckBox"));
                    aColumnLabelPostfixes.push_back(s_sEmptyString);
                    break;

                case DataType::TINYINT:
                case DataType::SMALLINT:
                case DataType::INTEGER:
                    aColumnServiceNames.push_back(::rtl::OUString::createFromAscii("NumericField"));
                    aColumnLabelPostfixes.push_back(s_sEmptyString);
                    break;

                case DataType::FLOAT:
                case DataType::REAL:
                case DataType::DOUBLE:
                case DataType::NUMERIC:
                case DataType::DECIMAL:
                    aColumnServiceNames.push_back(::rtl::OUString::createFromAscii("FormattedField"));
                    aColumnLabelPostfixes.push_back(s_sEmptyString);
                    break;

                case DataType::DATE:
                    aColumnServiceNames.push_back(::rtl::OUString::createFromAscii("DateField"));
                    aColumnLabelPostfixes.push_back(s_sEmptyString);
                    break;

                case DataType::TIME:
                    aColumnServiceNames.push_back(::rtl::OUString::createFromAscii("TimeField"));
                    aColumnLabelPostfixes.push_back(s_sEmptyString);
                    break;

                case DataType::TIMESTAMP:
                    aColumnServiceNames.push_back(::rtl::OUString::createFromAscii("DateField"));
                    aColumnLabelPostfixes.push_back(String(ModuleRes(RID_STR_DATEPOSTFIX)));

                    aFormFieldNames.push_back(*pSelectedFields);
                    aColumnServiceNames.push_back(::rtl::OUString::createFromAscii("TimeField"));
                    aColumnLabelPostfixes.push_back(String(ModuleRes(RID_STR_TIMEPOSTFIX)));
                    break;

                default:
                    aColumnServiceNames.push_back(::rtl::OUString::createFromAscii("TextField"));
                    aColumnLabelPostfixes.push_back(s_sEmptyString);
            }
        }

        DBG_ASSERT( aFormFieldNames.size() == aColumnServiceNames.size()
                &&  aColumnServiceNames.size() == aColumnLabelPostfixes.size(),
                "OGridWizard::implApplySettings: inconsistent descriptor sequences!");

        // now loop through the descriptions and create the (grid)columns out of th descriptors
        {
            Reference< XNameAccess > xExistenceChecker(xColumnContainer.get());

            ConstStringArrayIterator pColumnServiceName = aColumnServiceNames.begin();
            ConstStringArrayIterator pColumnLabelPostfix = aColumnLabelPostfixes.begin();
            ConstStringArrayIterator pFormFieldName = aFormFieldNames.begin();
            ConstStringArrayIterator pEnd = aColumnServiceNames.end();

            for (;pColumnServiceName < pEnd; ++pColumnServiceName, ++pColumnLabelPostfix, ++pFormFieldName)
            {
                // create a (grid)column for the (resultset)column
                try
                {
                    Reference< XPropertySet > xColumn = xColumnFactory->createColumn(*pColumnServiceName);

                    ::rtl::OUString sColumnName(*pColumnServiceName);
                    disambiguateName(xExistenceChecker, sColumnName);

                    if (xColumn.is())
                    {
                        // the data field the column should be bound to
                        xColumn->setPropertyValue(s_sDataFieldProperty, makeAny(*pFormFieldName));
                        // the label
                        xColumn->setPropertyValue(s_sLabelProperty, makeAny(::rtl::OUString(*pFormFieldName) += *pColumnLabelPostfix));
                        // the width (0 => column will be auto-sized)
                        xColumn->setPropertyValue(s_sWidthProperty, makeAny(sal_Int32(0)));

                        // insert the column
                        xColumnContainer->insertByName(sColumnName, makeAny(xColumn));
                    }
                }
                catch(Exception&)
                {
                    DBG_ERROR(  ::rtl::OString("OGridWizard::implApplySettings: unexpected exception while creating the grid column for field ")
                            +=  ::rtl::OString(pFormFieldName->getStr(), pFormFieldName->getLength(), gsl_getSystemTextEncoding())
                            +=  ::rtl::OString("!"));
                }
            }
        }
    }

    //---------------------------------------------------------------------
    OWizardPage* OGridWizard::createPage(sal_uInt16 _nState)
    {
        switch (_nState)
        {
            case GW_STATE_DATASOURCE_SELECTION:
                return new OTableSelectionPage(this);
            case GW_STATE_FIELDSELECTION:
                return new OGridFieldsSelection(this);
        }

        return NULL;
    }

    //---------------------------------------------------------------------
    sal_uInt16 OGridWizard::determineNextState(sal_uInt16 _nCurrentState)
    {
        switch (_nCurrentState)
        {
            case GW_STATE_DATASOURCE_SELECTION:
                return GW_STATE_FIELDSELECTION;
            case GW_STATE_FIELDSELECTION:
                return WZS_INVALID_STATE;
        }

        return WZS_INVALID_STATE;
    }

    //---------------------------------------------------------------------
    void OGridWizard::enterState(sal_uInt16 _nState)
    {
        OControlWizard::enterState(_nState);

        enableButtons(WZB_PREVIOUS, m_bHadDataSelection ? (GW_STATE_DATASOURCE_SELECTION < _nState) : GW_STATE_FIELDSELECTION < _nState);
        enableButtons(WZB_NEXT, GW_STATE_FIELDSELECTION != _nState);
        if (_nState < GW_STATE_FIELDSELECTION)
            enableButtons(WZB_FINISH, sal_False);

        if (GW_STATE_FIELDSELECTION == _nState)
            defaultButton(WZB_FINISH);
    }

    //---------------------------------------------------------------------
    sal_Bool OGridWizard::leaveState(sal_uInt16 _nState)
    {
        if (!OControlWizard::leaveState(_nState))
            return sal_False;

        if (GW_STATE_FIELDSELECTION == _nState)
            defaultButton(WZB_NEXT);

        return sal_True;
    }

    //---------------------------------------------------------------------
    sal_Bool OGridWizard::onFinish(sal_Int32 _nResult)
    {
        if (!OControlWizard::onFinish(_nResult))
            return sal_False;

        implApplySettings();

        return sal_True;
    }

    //=====================================================================
    //= OGridFieldsSelection
    //=====================================================================
    //---------------------------------------------------------------------
    OGridFieldsSelection::OGridFieldsSelection( OGridWizard* _pParent )
        :OGridPage(_pParent, ModuleRes(RID_PAGE_GW_FIELDSELECTION))
        ,m_aFrame               (this, ResId(FL_FRAME))
        ,m_aExistFieldsLabel    (this, ResId(FT_EXISTING_FIELDS))
        ,m_aExistFields         (this, ResId(LB_EXISTING_FIELDS))
        ,m_aSelectOne           (this, ResId(PB_FIELDRIGHT))
        ,m_aSelectAll           (this, ResId(PB_ALLFIELDSRIGHT))
        ,m_aDeselectOne         (this, ResId(PB_FIELDLEFT))
        ,m_aDeselectAll         (this, ResId(PB_ALLFIELDSLEFT))
        ,m_aSelFieldsLabel      (this, ResId(FT_SELECTED_FIELDS))
        ,m_aSelFields           (this, ResId(LB_SELECTED_FIELDS))
    {
        FreeResource();

        m_aSelectOne.SetClickHdl(LINK(this, OGridFieldsSelection, OnMoveOneEntry));
        m_aSelectAll.SetClickHdl(LINK(this, OGridFieldsSelection, OnMoveAllEntries));
        m_aDeselectOne.SetClickHdl(LINK(this, OGridFieldsSelection, OnMoveOneEntry));
        m_aDeselectAll.SetClickHdl(LINK(this, OGridFieldsSelection, OnMoveAllEntries));

        m_aExistFields.SetSelectHdl(LINK(this, OGridFieldsSelection, OnEntrySelected));
        m_aSelFields.SetSelectHdl(LINK(this, OGridFieldsSelection, OnEntrySelected));
        m_aExistFields.SetDoubleClickHdl(LINK(this, OGridFieldsSelection, OnEntryDoubleClicked));
        m_aSelFields.SetDoubleClickHdl(LINK(this, OGridFieldsSelection, OnEntryDoubleClicked));
    }

    //---------------------------------------------------------------------
    void OGridFieldsSelection::ActivatePage()
    {
        OGridPage::ActivatePage();
        m_aExistFields.GrabFocus();
    }

    //---------------------------------------------------------------------
    sal_Bool OGridFieldsSelection::determineNextButtonState()
    {
        return sal_False;
            // we're the last page in our wizard
    }

    //---------------------------------------------------------------------
    void OGridFieldsSelection::initializePage()
    {
        OGridPage::initializePage();

        const OControlWizardContext& rContext = getContext();
        fillListBox(m_aExistFields, rContext.aFieldNames);

        m_aSelFields.Clear();
        const OGridSettings& rSettings = getSettings();
        const ::rtl::OUString* pSelected = rSettings.aSelectedFields.getConstArray();
        const ::rtl::OUString* pEnd = pSelected + rSettings.aSelectedFields.getLength();
        for (; pSelected < pEnd; ++pSelected)
        {
            m_aSelFields.InsertEntry(*pSelected);
            m_aExistFields.RemoveEntry(*pSelected);
        }

        implCheckButtons();
    }

    //---------------------------------------------------------------------
    sal_Bool OGridFieldsSelection::commitPage(COMMIT_REASON _eReason)
    {
        if (!OGridPage::commitPage(_eReason))
            return sal_False;

        OGridSettings& rSettings = getSettings();
        sal_uInt16 nSelected = m_aSelFields.GetEntryCount();

        rSettings.aSelectedFields.realloc(nSelected);
        ::rtl::OUString* pSelected = rSettings.aSelectedFields.getArray();

        for (sal_uInt16 i=0; i<nSelected; ++i, ++pSelected)
            *pSelected = m_aSelFields.GetEntry(i);

        return sal_True;
    }

    //---------------------------------------------------------------------
    void OGridFieldsSelection::implCheckButtons()
    {
        m_aSelectOne.Enable(m_aExistFields.GetSelectEntryCount() != 0);
        m_aSelectAll.Enable(m_aExistFields.GetEntryCount() != 0);

        m_aDeselectOne.Enable(m_aSelFields.GetSelectEntryCount() != 0);
        m_aDeselectAll.Enable(m_aSelFields.GetEntryCount() != 0);

        getDialog()->enableButtons(WZB_FINISH, 0 != m_aSelFields.GetEntryCount());
    }

    //---------------------------------------------------------------------
    IMPL_LINK(OGridFieldsSelection, OnEntryDoubleClicked, ListBox*, _pList)
    {
        PushButton* pSimulateButton = &m_aExistFields == _pList ? &m_aSelectOne : &m_aDeselectOne;
        if (pSimulateButton->IsEnabled())
            return OnMoveOneEntry( pSimulateButton );
        else
            return 1L;
    }

    //---------------------------------------------------------------------
    IMPL_LINK(OGridFieldsSelection, OnEntrySelected, ListBox*, NOTINTERESTEDIN)
    {
        implCheckButtons();
        return 0L;
    }

    //---------------------------------------------------------------------
    IMPL_LINK(OGridFieldsSelection, OnMoveOneEntry, PushButton*, _pButton)
    {
        sal_Bool bMoveRight = (&m_aSelectOne == _pButton);
        ListBox& rMoveTo = bMoveRight ? m_aSelFields : m_aExistFields;

        // the index of the selected entry
        sal_uInt16 nSelected = bMoveRight ? m_aExistFields.GetSelectEntryPos() : m_aSelFields.GetSelectEntryPos();
        // the (original) relative position of the entry
        sal_Int32 nRelativeIndex = reinterpret_cast<sal_Int32>(bMoveRight ? m_aExistFields.GetEntryData(nSelected) : m_aSelFields.GetEntryData(nSelected));

        sal_uInt16 nInsertPos = LISTBOX_APPEND;
        if (!bMoveRight)
        {   // need to determine an insert pos which reflects the original
            nInsertPos = 0;
            while (nInsertPos < rMoveTo.GetEntryCount())
            {
                if (reinterpret_cast<sal_Int32>(rMoveTo.GetEntryData(nInsertPos)) > nRelativeIndex)
                    break;
                ++nInsertPos;
            }
        }

        // the text of the entry to move
        String sMovingEntry = bMoveRight ? m_aExistFields.GetEntry(nSelected) : m_aSelFields.GetEntry(nSelected);

        // insert the entry
        nInsertPos = rMoveTo.InsertEntry(sMovingEntry, nInsertPos);
        // preserve it's "relative position" entry data
        rMoveTo.SetEntryData(nInsertPos, reinterpret_cast<void*>(nRelativeIndex));

        // remove the entry from it's old list
        bMoveRight ? m_aExistFields.RemoveEntry(nSelected) : m_aSelFields.RemoveEntry(nSelected);

        implCheckButtons();
        return 0;
    }

    //---------------------------------------------------------------------
    IMPL_LINK(OGridFieldsSelection, OnMoveAllEntries, PushButton*, _pButton)
    {
        sal_Bool bMoveRight = (&m_aSelectAll == _pButton);
        m_aExistFields.Clear();
        m_aSelFields.Clear();
        fillListBox(bMoveRight ? m_aSelFields : m_aExistFields, getContext().aFieldNames);

        implCheckButtons();
        return 0;
    }

//.........................................................................
}   // namespace dbp
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 05.03.01 09:24:22  fs
 ************************************************************************/

