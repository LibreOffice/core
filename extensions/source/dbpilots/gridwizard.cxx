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

#include <sal/config.h>

#include <vector>

#include "gridwizard.hxx"
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/form/XGridColumnFactory.hpp>
#include <com/sun/star/awt/MouseWheelBehavior.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include "dbptools.hxx"
#include <helpids.h>

#define GW_STATE_DATASOURCE_SELECTION   0
#define GW_STATE_FIELDSELECTION         1


namespace dbp
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::awt;
    using namespace ::svt;

    OGridWizard::OGridWizard( vcl::Window* _pParent,
            const Reference< XPropertySet >& _rxObjectModel, const Reference< XComponentContext >& _rxContext )
        :OControlWizard(_pParent, _rxObjectModel, _rxContext)
        ,m_bHadDataSelection(true)
    {
        initControlSettings(&m_aSettings);

        m_pPrevPage->SetHelpId(HID_GRIDWIZARD_PREVIOUS);
        m_pNextPage->SetHelpId(HID_GRIDWIZARD_NEXT);
        m_pCancel->SetHelpId(HID_GRIDWIZARD_CANCEL);
        m_pFinish->SetHelpId(HID_GRIDWIZARD_FINISH);
        setTitleBase(compmodule::ModuleRes(RID_STR_GRIDWIZARD_TITLE));

        // if we do not need the data source selection page ...
        if (!needDatasourceSelection())
        {   // ... skip it!
            skip();
            m_bHadDataSelection = false;
        }
    }


    bool OGridWizard::approveControl(sal_Int16 _nClassId)
    {
        if (FormComponentType::GRIDCONTROL != _nClassId)
            return false;

        Reference< XGridColumnFactory > xColumnFactory(getContext().xObjectModel, UNO_QUERY);
        return xColumnFactory.is();
    }


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

        static const char s_sMouseWheelBehavior[] = "MouseWheelBehavior";
        static const char s_sEmptyString[] = "";

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
                    aColumnLabelPostfixes.push_back(compmodule::ModuleRes(RID_STR_DATEPOSTFIX));

                    aFormFieldNames.push_back(*pSelectedFields);
                    aColumnServiceNames.push_back(OUString("TimeField"));
                    aColumnLabelPostfixes.push_back(compmodule::ModuleRes(RID_STR_TIMEPOSTFIX));
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

            std::vector< OUString >::const_iterator pColumnLabelPostfix = aColumnLabelPostfixes.begin();
            std::vector< OUString >::const_iterator pFormFieldName = aFormFieldNames.begin();

            for (const auto& rColumnServiceName : aColumnServiceNames)
            {
                // create a (grid)column for the (resultset)column
                try
                {
                    Reference< XPropertySet > xColumn( xColumnFactory->createColumn(rColumnServiceName), UNO_SET_THROW );
                    Reference< XPropertySetInfo > xColumnPSI( xColumn->getPropertySetInfo(), UNO_SET_THROW );

                    OUString sColumnName(rColumnServiceName);
                    disambiguateName(xExistenceChecker, sColumnName);

                    // the data field the column should be bound to
                    xColumn->setPropertyValue("DataField", makeAny(*pFormFieldName));
                    // the label
                    xColumn->setPropertyValue("Label", makeAny(*pFormFieldName + *pColumnLabelPostfix));
                    // the width (<void/> => column will be auto-sized)
                    xColumn->setPropertyValue("Width", Any());

                    if ( xColumnPSI->hasPropertyByName( s_sMouseWheelBehavior ) )
                        xColumn->setPropertyValue( s_sMouseWheelBehavior, makeAny( MouseWheelBehavior::SCROLL_DISABLED ) );

                    // insert the column
                    xColumnContainer->insertByName(sColumnName, makeAny(xColumn));
                }
                catch(const Exception&)
                {
                    SAL_WARN( "extensions.dbpilots", "OGridWizard::implApplySettings: "
                              "unexpected exception while creating the grid column for field " <<
                              *pFormFieldName );
                }

                ++pColumnLabelPostfix;
                ++pFormFieldName;
            }
        }
    }


    VclPtr<TabPage> OGridWizard::createPage(WizardState _nState)
    {
        switch (_nState)
        {
            case GW_STATE_DATASOURCE_SELECTION:
                return VclPtr<OTableSelectionPage>::Create(this);
            case GW_STATE_FIELDSELECTION:
                return VclPtr<OGridFieldsSelection>::Create(this);
        }
        return VclPtr<TabPage>();
    }


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


    void OGridWizard::enterState(WizardState _nState)
    {
        OControlWizard::enterState(_nState);

        enableButtons(WizardButtonFlags::PREVIOUS, m_bHadDataSelection ? (GW_STATE_DATASOURCE_SELECTION < _nState) : GW_STATE_FIELDSELECTION < _nState);
        enableButtons(WizardButtonFlags::NEXT, GW_STATE_FIELDSELECTION != _nState);
        if (_nState < GW_STATE_FIELDSELECTION)
            enableButtons(WizardButtonFlags::FINISH, false);

        if (GW_STATE_FIELDSELECTION == _nState)
            defaultButton(WizardButtonFlags::FINISH);
    }


    bool OGridWizard::leaveState(WizardState _nState)
    {
        if (!OControlWizard::leaveState(_nState))
            return false;

        if (GW_STATE_FIELDSELECTION == _nState)
            defaultButton(WizardButtonFlags::NEXT);

        return true;
    }


    bool OGridWizard::onFinish()
    {
        if ( !OControlWizard::onFinish() )
            return false;

        implApplySettings();

        return true;
    }

    OGridFieldsSelection::OGridFieldsSelection( OGridWizard* _pParent )
        :OGridPage(_pParent, "GridFieldsSelection", "modules/sabpilot/ui/gridfieldsselectionpage.ui")
    {
        get(m_pExistFields,"existingfields");
        get(m_pSelectOne,"fieldright");
        get(m_pSelectAll,"allfieldsright");
        get(m_pDeselectOne,"fieldleft");
        get(m_pDeselectAll,"allfieldsleft");
        get(m_pSelFields,"selectedfields");

        enableFormDatasourceDisplay();

        m_pSelectOne->SetClickHdl(LINK(this, OGridFieldsSelection, OnMoveOneEntry));
        m_pSelectAll->SetClickHdl(LINK(this, OGridFieldsSelection, OnMoveAllEntries));
        m_pDeselectOne->SetClickHdl(LINK(this, OGridFieldsSelection, OnMoveOneEntry));
        m_pDeselectAll->SetClickHdl(LINK(this, OGridFieldsSelection, OnMoveAllEntries));

        m_pExistFields->SetSelectHdl(LINK(this, OGridFieldsSelection, OnEntrySelected));
        m_pSelFields->SetSelectHdl(LINK(this, OGridFieldsSelection, OnEntrySelected));
        m_pExistFields->SetDoubleClickHdl(LINK(this, OGridFieldsSelection, OnEntryDoubleClicked));
        m_pSelFields->SetDoubleClickHdl(LINK(this, OGridFieldsSelection, OnEntryDoubleClicked));
    }

    OGridFieldsSelection::~OGridFieldsSelection()
    {
        disposeOnce();
    }

    void OGridFieldsSelection::dispose()
    {
        m_pExistFields.clear();
        m_pSelectOne.clear();
        m_pSelectAll.clear();
        m_pDeselectOne.clear();
        m_pDeselectAll.clear();
        m_pSelFields.clear();
        OGridPage::dispose();
    }

    void OGridFieldsSelection::ActivatePage()
    {
        OGridPage::ActivatePage();
        m_pExistFields->GrabFocus();
    }


    bool OGridFieldsSelection::canAdvance() const
    {
        return false;
            // we're the last page in our wizard
    }


    void OGridFieldsSelection::initializePage()
    {
        OGridPage::initializePage();

        const OControlWizardContext& rContext = getContext();
        fillListBox(*m_pExistFields, rContext.aFieldNames);

        m_pSelFields->Clear();
        const OGridSettings& rSettings = getSettings();
        const OUString* pSelected = rSettings.aSelectedFields.getConstArray();
        const OUString* pEnd = pSelected + rSettings.aSelectedFields.getLength();
        for (; pSelected < pEnd; ++pSelected)
        {
            m_pSelFields->InsertEntry(*pSelected);
            m_pExistFields->RemoveEntry(*pSelected);
        }

        implCheckButtons();
    }


    bool OGridFieldsSelection::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
    {
        if (!OGridPage::commitPage(_eReason))
            return false;

        OGridSettings& rSettings = getSettings();
        const sal_Int32 nSelected = m_pSelFields->GetEntryCount();

        rSettings.aSelectedFields.realloc(nSelected);
        OUString* pSelected = rSettings.aSelectedFields.getArray();

        for (sal_Int32 i=0; i<nSelected; ++i, ++pSelected)
            *pSelected = m_pSelFields->GetEntry(i);

        return true;
    }


    void OGridFieldsSelection::implCheckButtons()
    {
        m_pSelectOne->Enable(m_pExistFields->GetSelectedEntryCount() != 0);
        m_pSelectAll->Enable(m_pExistFields->GetEntryCount() != 0);

        m_pDeselectOne->Enable(m_pSelFields->GetSelectedEntryCount() != 0);
        m_pDeselectAll->Enable(m_pSelFields->GetEntryCount() != 0);

        getDialog()->enableButtons(WizardButtonFlags::FINISH, 0 != m_pSelFields->GetEntryCount());
    }


    IMPL_LINK(OGridFieldsSelection, OnEntryDoubleClicked, ListBox&, _rList, void)
    {
        PushButton* pSimulateButton = m_pExistFields == &_rList ? m_pSelectOne.get() : m_pDeselectOne.get();
        if (pSimulateButton->IsEnabled())
        {
            OnMoveOneEntry( pSimulateButton );
        }
    }


    IMPL_LINK_NOARG(OGridFieldsSelection, OnEntrySelected, ListBox&, void)
    {
        implCheckButtons();
    }


    IMPL_LINK(OGridFieldsSelection, OnMoveOneEntry, Button*, _pButton, void)
    {
        bool bMoveRight = (m_pSelectOne == _pButton);
        ListBox& rMoveTo = bMoveRight ? *m_pSelFields : *m_pExistFields;

        // the index of the selected entry
        const sal_Int32 nSelected = bMoveRight ? m_pExistFields->GetSelectedEntryPos() : m_pSelFields->GetSelectedEntryPos();
        // the (original) relative position of the entry
        sal_IntPtr nRelativeIndex = reinterpret_cast<sal_IntPtr>(bMoveRight ? m_pExistFields->GetEntryData(nSelected) : m_pSelFields->GetEntryData(nSelected));

        sal_Int32 nInsertPos = LISTBOX_APPEND;
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
        OUString sMovingEntry = bMoveRight ? m_pExistFields->GetEntry(nSelected) : m_pSelFields->GetEntry(nSelected);

        // insert the entry
        nInsertPos = rMoveTo.InsertEntry(sMovingEntry, nInsertPos);
        // preserve it's "relative position" entry data
        rMoveTo.SetEntryData(nInsertPos, reinterpret_cast<void*>(nRelativeIndex));

        // remove the entry from its old list
        if (bMoveRight)
        {
            sal_Int32 nSelectPos = m_pExistFields->GetSelectedEntryPos();
            m_pExistFields->RemoveEntry(nSelected);
            if ((LISTBOX_ENTRY_NOTFOUND != nSelectPos) && (nSelectPos < m_pExistFields->GetEntryCount()))
                m_pExistFields->SelectEntryPos(nSelectPos);

            m_pExistFields->GrabFocus();
        }
        else
        {
            sal_Int32 nSelectPos = m_pSelFields->GetSelectedEntryPos();
            m_pSelFields->RemoveEntry(nSelected);
            if ((LISTBOX_ENTRY_NOTFOUND != nSelectPos) && (nSelectPos < m_pSelFields->GetEntryCount()))
                m_pSelFields->SelectEntryPos(nSelectPos);

            m_pSelFields->GrabFocus();
        }

        implCheckButtons();
    }


    IMPL_LINK(OGridFieldsSelection, OnMoveAllEntries, Button*, _pButton, void)
    {
        bool bMoveRight = (m_pSelectAll == _pButton);
        m_pExistFields->Clear();
        m_pSelFields->Clear();
        fillListBox(bMoveRight ? *m_pSelFields : *m_pExistFields, getContext().aFieldNames);

        implCheckButtons();
    }


}   // namespace dbp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
