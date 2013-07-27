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

#include "sal/config.h"

#include <vector>

#include "gridwizard.hxx"
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <tools/string.hxx>
#include <com/sun/star/form/XGridColumnFactory.hpp>
#include <com/sun/star/awt/MouseWheelBehavior.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <tools/debug.hxx>
#include "dbptools.hxx"
#include "dbpilots.hrc"

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
    using namespace ::com::sun::star::awt;
    using namespace ::svt;

    //=====================================================================
    //= OGridWizard
    //=====================================================================
    //---------------------------------------------------------------------
    OGridWizard::OGridWizard( Window* _pParent,
            const Reference< XPropertySet >& _rxObjectModel, const Reference< XComponentContext >& _rxContext )
        :OControlWizard(_pParent, ModuleRes(RID_DLG_GRIDWIZARD), _rxObjectModel, _rxContext)
        ,m_bHadDataSelection(sal_True)
    {
        initControlSettings(&m_aSettings);

        m_pPrevPage->SetHelpId(HID_GRIDWIZARD_PREVIOUS);
        m_pNextPage->SetHelpId(HID_GRIDWIZARD_NEXT);
        m_pCancel->SetHelpId(HID_GRIDWIZARD_CANCEL);
        m_pFinish->SetHelpId(HID_GRIDWIZARD_FINISH);

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

        static const OUString s_sDataFieldProperty   ("DataField");
        static const OUString s_sLabelProperty       ("Label");
        static const OUString s_sWidthProperty       ("Width");
        static const OUString s_sMouseWheelBehavior ("MouseWheelBehavior");
        static const OUString s_sEmptyString;

        // collect "descriptors" for the to-be-created (grid)columns
        std::vector< OUString > aColumnServiceNames;  // service names to be used with the XGridColumnFactory
        std::vector< OUString > aColumnLabelPostfixes;    // postfixes to append to the column labels
        std::vector< OUString > aFormFieldNames;      // data field names

        aColumnServiceNames.reserve(getSettings().aSelectedFields.getLength());
        aColumnLabelPostfixes.reserve(getSettings().aSelectedFields.getLength());
        aFormFieldNames.reserve(getSettings().aSelectedFields.getLength());

        // loop through the selected field names
        const OUString* pSelectedFields = getSettings().aSelectedFields.getConstArray();
        const OUString* pEnd = pSelectedFields + getSettings().aSelectedFields.getLength();
        for (;pSelectedFields < pEnd; ++pSelectedFields)
        {
            // get the information for the selected column
            sal_Int32 nFieldType = DataType::OTHER;
            OControlWizardContext::TNameTypeMap::const_iterator aFind = rContext.aTypes.find(*pSelectedFields);
            if ( aFind != rContext.aTypes.end() )
                nFieldType = aFind->second;

            aFormFieldNames.push_back(*pSelectedFields);
            switch (nFieldType)
            {
                case DataType::BIT:
                case DataType::BOOLEAN:
                    aColumnServiceNames.push_back(OUString("CheckBox"));
                    aColumnLabelPostfixes.push_back(s_sEmptyString);
                    break;

                case DataType::TINYINT:
                case DataType::SMALLINT:
                case DataType::INTEGER:
                    aColumnServiceNames.push_back(OUString("NumericField"));
                    aColumnLabelPostfixes.push_back(s_sEmptyString);
                    break;

                case DataType::FLOAT:
                case DataType::REAL:
                case DataType::DOUBLE:
                case DataType::NUMERIC:
                case DataType::DECIMAL:
                    aColumnServiceNames.push_back(OUString("FormattedField"));
                    aColumnLabelPostfixes.push_back(s_sEmptyString);
                    break;

                case DataType::DATE:
                    aColumnServiceNames.push_back(OUString("DateField"));
                    aColumnLabelPostfixes.push_back(s_sEmptyString);
                    break;

                case DataType::TIME:
                    aColumnServiceNames.push_back(OUString("TimeField"));
                    aColumnLabelPostfixes.push_back(s_sEmptyString);
                    break;

                case DataType::TIMESTAMP:
                    aColumnServiceNames.push_back(OUString("DateField"));
                    aColumnLabelPostfixes.push_back(ModuleRes(RID_STR_DATEPOSTFIX).toString());

                    aFormFieldNames.push_back(*pSelectedFields);
                    aColumnServiceNames.push_back(OUString("TimeField"));
                    aColumnLabelPostfixes.push_back(ModuleRes(RID_STR_TIMEPOSTFIX).toString());
                    break;

                default:
                    aColumnServiceNames.push_back(OUString("TextField"));
                    aColumnLabelPostfixes.push_back(s_sEmptyString);
            }
        }

        DBG_ASSERT( aFormFieldNames.size() == aColumnServiceNames.size()
                &&  aColumnServiceNames.size() == aColumnLabelPostfixes.size(),
                "OGridWizard::implApplySettings: inconsistent descriptor sequences!");

        // now loop through the descriptions and create the (grid)columns out of th descriptors
        {
            Reference< XNameAccess > xExistenceChecker(xColumnContainer.get());

            std::vector< OUString >::const_iterator pColumnServiceName = aColumnServiceNames.begin();
            std::vector< OUString >::const_iterator pColumnLabelPostfix = aColumnLabelPostfixes.begin();
            std::vector< OUString >::const_iterator pFormFieldName = aFormFieldNames.begin();
            std::vector< OUString >::const_iterator pColumnServiceNameEnd = aColumnServiceNames.end();

            for (;pColumnServiceName < pColumnServiceNameEnd; ++pColumnServiceName, ++pColumnLabelPostfix, ++pFormFieldName)
            {
                // create a (grid)column for the (resultset)column
                try
                {
                    Reference< XPropertySet > xColumn( xColumnFactory->createColumn(*pColumnServiceName), UNO_SET_THROW );
                    Reference< XPropertySetInfo > xColumnPSI( xColumn->getPropertySetInfo(), UNO_SET_THROW );

                    OUString sColumnName(*pColumnServiceName);
                    disambiguateName(xExistenceChecker, sColumnName);

                    // the data field the column should be bound to
                    xColumn->setPropertyValue(s_sDataFieldProperty, makeAny(*pFormFieldName));
                    // the label
                    xColumn->setPropertyValue(s_sLabelProperty, makeAny(OUString(*pFormFieldName) += *pColumnLabelPostfix));
                    // the width (<void/> => column will be auto-sized)
                    xColumn->setPropertyValue(s_sWidthProperty, Any());

                    if ( xColumnPSI->hasPropertyByName( s_sMouseWheelBehavior ) )
                        xColumn->setPropertyValue( s_sMouseWheelBehavior, makeAny( MouseWheelBehavior::SCROLL_DISABLED ) );

                    // insert the column
                    xColumnContainer->insertByName(sColumnName, makeAny(xColumn));
                }
                catch(const Exception&)
                {
                    SAL_WARN( "extensions.dbpilots", "OGridWizard::implApplySettings: " <<
                              "unexpected exception while creating the grid column for field " <<
                              pFormFieldName->getStr() << "!" );
                }
            }
        }
    }

    //---------------------------------------------------------------------
    OWizardPage* OGridWizard::createPage(WizardState _nState)
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
    WizardTypes::WizardState OGridWizard::determineNextState( WizardState _nCurrentState ) const
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
    void OGridWizard::enterState(WizardState _nState)
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
    sal_Bool OGridWizard::leaveState(WizardState _nState)
    {
        if (!OControlWizard::leaveState(_nState))
            return sal_False;

        if (GW_STATE_FIELDSELECTION == _nState)
            defaultButton(WZB_NEXT);

        return sal_True;
    }

    //---------------------------------------------------------------------
    sal_Bool OGridWizard::onFinish()
    {
        if ( !OControlWizard::onFinish() )
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
        ,m_aFrame               (this, ModuleRes(FL_FRAME))
        ,m_aExistFieldsLabel    (this, ModuleRes(FT_EXISTING_FIELDS))
        ,m_aExistFields         (this, ModuleRes(LB_EXISTING_FIELDS))
        ,m_aSelectOne           (this, ModuleRes(PB_FIELDRIGHT))
        ,m_aSelectAll           (this, ModuleRes(PB_ALLFIELDSRIGHT))
        ,m_aDeselectOne         (this, ModuleRes(PB_FIELDLEFT))
        ,m_aDeselectAll         (this, ModuleRes(PB_ALLFIELDSLEFT))
        ,m_aSelFieldsLabel      (this, ModuleRes(FT_SELECTED_FIELDS))
        ,m_aSelFields           (this, ModuleRes(LB_SELECTED_FIELDS))
    {
        FreeResource();

        enableFormDatasourceDisplay();

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
    bool OGridFieldsSelection::canAdvance() const
    {
        return false;
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
        const OUString* pSelected = rSettings.aSelectedFields.getConstArray();
        const OUString* pEnd = pSelected + rSettings.aSelectedFields.getLength();
        for (; pSelected < pEnd; ++pSelected)
        {
            m_aSelFields.InsertEntry(*pSelected);
            m_aExistFields.RemoveEntry(*pSelected);
        }

        implCheckButtons();
    }

    //---------------------------------------------------------------------
    sal_Bool OGridFieldsSelection::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
    {
        if (!OGridPage::commitPage(_eReason))
            return sal_False;

        OGridSettings& rSettings = getSettings();
        sal_uInt16 nSelected = m_aSelFields.GetEntryCount();

        rSettings.aSelectedFields.realloc(nSelected);
        OUString* pSelected = rSettings.aSelectedFields.getArray();

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
    IMPL_LINK(OGridFieldsSelection, OnEntrySelected, ListBox*, /*NOTINTERESTEDIN*/)
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
        sal_IntPtr nRelativeIndex = reinterpret_cast<sal_IntPtr>(bMoveRight ? m_aExistFields.GetEntryData(nSelected) : m_aSelFields.GetEntryData(nSelected));

        sal_uInt16 nInsertPos = LISTBOX_APPEND;
        if (!bMoveRight)
        {   // need to determine an insert pos which reflects the original
            nInsertPos = 0;
            while (nInsertPos < rMoveTo.GetEntryCount())
            {
                if (reinterpret_cast<sal_IntPtr>(rMoveTo.GetEntryData(nInsertPos)) > nRelativeIndex)
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
        if (bMoveRight)
        {
            sal_uInt16 nSelectPos = m_aExistFields.GetSelectEntryPos();
            m_aExistFields.RemoveEntry(nSelected);
            if ((LISTBOX_ENTRY_NOTFOUND != nSelectPos) && (nSelectPos < m_aExistFields.GetEntryCount()))
                m_aExistFields.SelectEntryPos(nSelectPos);

            m_aExistFields.GrabFocus();
        }
        else
        {
            sal_uInt16 nSelectPos = m_aSelFields.GetSelectEntryPos();
            m_aSelFields.RemoveEntry(nSelected);
            if ((LISTBOX_ENTRY_NOTFOUND != nSelectPos) && (nSelectPos < m_aSelFields.GetEntryCount()))
                m_aSelFields.SelectEntryPos(nSelectPos);

            m_aSelFields.GrabFocus();
        }

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
