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

#undef SC_DLLIMPLEMENTATION

#include <pvfundlg.hxx>

#include <com/sun/star/sheet/DataPilotFieldReferenceType.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceItemType.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldShowItemsMode.hpp>

#include <osl/diagnose.h>

#include <scresid.hxx>
#include <dpobject.hxx>
#include <dpsave.hxx>
#include <pvfundlg.hrc>
#include <globstr.hrc>
#include <dputil.hxx>

#include <vector>

using namespace ::com::sun::star::sheet;

using ::com::sun::star::uno::Sequence;
using ::std::vector;

namespace {

/** Appends all strings from the Sequence to the list box.

    Empty strings are replaced by a localized "(empty)" entry and inserted at
    the specified position.

    @return  true = The passed string list contains an empty string entry.
 */

bool lclFillListBox(weld::ComboBox& rLBox, const Sequence< OUString >& rStrings)
{
    bool bEmpty = false;
    for (const OUString& str : rStrings)
    {
        if (!str.isEmpty())
            rLBox.append_text(str);
        else
        {
            rLBox.append_text(ScResId(STR_EMPTYDATA));
            bEmpty = true;
        }
    }
    return bEmpty;
}

bool lclFillListBox(weld::ComboBox& rLBox, const vector<ScDPLabelData::Member>& rMembers, int nEmptyPos)
{
    bool bEmpty = false;
    vector<ScDPLabelData::Member>::const_iterator itr = rMembers.begin(), itrEnd = rMembers.end();
    for (; itr != itrEnd; ++itr)
    {
        OUString aName = itr->getDisplayName();
        if (!aName.isEmpty())
            rLBox.append_text(aName);
        else
        {
            rLBox.insert_text(nEmptyPos, ScResId(STR_EMPTYDATA));
            bEmpty = true;
        }
    }
    return bEmpty;
}

bool lclFillListBox(weld::TreeView& rLBox, const vector<ScDPLabelData::Member>& rMembers)
{
    bool bEmpty = false;
    for (const auto& rMember : rMembers)
    {
        rLBox.append();
        int pos = rLBox.n_children() - 1;
        rLBox.set_toggle(pos, false, 0);
        OUString aName = rMember.getDisplayName();
        if (!aName.isEmpty())
            rLBox.set_text(pos, aName, 1);
        else
        {
            rLBox.set_text(pos, ScResId(STR_EMPTYDATA), 1);
            bEmpty = true;
        }
    }
    return bEmpty;
}

/** This table represents the order of the strings in the resource string array. */
static const PivotFunc spnFunctions[] =
{
    PivotFunc::Sum,
    PivotFunc::Count,
    PivotFunc::Average,
    PivotFunc::Median,
    PivotFunc::Max,
    PivotFunc::Min,
    PivotFunc::Product,
    PivotFunc::CountNum,
    PivotFunc::StdDev,
    PivotFunc::StdDevP,
    PivotFunc::StdVar,
    PivotFunc::StdVarP
};

const sal_uInt16 SC_BASEITEM_PREV_POS = 0;
const sal_uInt16 SC_BASEITEM_NEXT_POS = 1;
const sal_uInt16 SC_BASEITEM_USER_POS = 2;

const sal_uInt16 SC_SORTNAME_POS = 0;
const sal_uInt16 SC_SORTDATA_POS = 1;

const long SC_SHOW_DEFAULT = 10;

} // namespace

ScDPFunctionListBox::ScDPFunctionListBox(std::unique_ptr<weld::TreeView> xControl)
    : m_xControl(std::move(xControl))
{
    FillFunctionNames();
}

void ScDPFunctionListBox::SetSelection( PivotFunc nFuncMask )
{
    if( (nFuncMask == PivotFunc::NONE) || (nFuncMask == PivotFunc::Auto) )
        m_xControl->unselect_all();
    else
    {
        for( sal_Int32 nEntry = 0, nCount = m_xControl->n_children(); nEntry < nCount; ++nEntry )
        {
            if (bool(nFuncMask & spnFunctions[ nEntry ]))
                m_xControl->select(nEntry);
            else
                m_xControl->unselect(nEntry);
        }
    }
}

PivotFunc ScDPFunctionListBox::GetSelection() const
{
    PivotFunc nFuncMask = PivotFunc::NONE;
    std::vector<int> aRows = m_xControl->get_selected_rows();
    for (int nSel : aRows)
        nFuncMask |= spnFunctions[nSel];
    return nFuncMask;
}

void ScDPFunctionListBox::FillFunctionNames()
{
    OSL_ENSURE( !m_xControl->n_children(), "ScDPMultiFuncListBox::FillFunctionNames - do not add texts to resource" );
    m_xControl->clear();
    m_xControl->freeze();
    for (size_t nIndex = 0; nIndex < SAL_N_ELEMENTS(SCSTR_DPFUNCLISTBOX); ++nIndex)
        m_xControl->append_text(ScResId(SCSTR_DPFUNCLISTBOX[nIndex]));
    m_xControl->thaw();
    assert(m_xControl->n_children() == SAL_N_ELEMENTS(spnFunctions));
}

namespace
{
    int FromDataPilotFieldReferenceType(int eMode)
    {
        switch (eMode)
        {
            case DataPilotFieldReferenceType::NONE:
                return 0;
            case DataPilotFieldReferenceType::ITEM_DIFFERENCE:
                return 1;
            case DataPilotFieldReferenceType::ITEM_PERCENTAGE:
                return 2;
            case DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE:
                return 3;
            case DataPilotFieldReferenceType::RUNNING_TOTAL:
                return 4;
            case DataPilotFieldReferenceType::ROW_PERCENTAGE:
                return 5;
            case DataPilotFieldReferenceType::COLUMN_PERCENTAGE:
                return 6;
            case DataPilotFieldReferenceType::TOTAL_PERCENTAGE:
                return 7;
            case DataPilotFieldReferenceType::INDEX:
                return 8;
        }
        return -1;
    }

    int ToDataPilotFieldReferenceType(int nPos)
    {
        switch (nPos)
        {
            case 0:
                return DataPilotFieldReferenceType::NONE;
            case 1:
                return DataPilotFieldReferenceType::ITEM_DIFFERENCE;
            case 2:
                return DataPilotFieldReferenceType::ITEM_PERCENTAGE;
            case 3:
                return DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE;
            case 4:
                return DataPilotFieldReferenceType::RUNNING_TOTAL;
            case 5:
                return DataPilotFieldReferenceType::ROW_PERCENTAGE;
            case 6:
                return DataPilotFieldReferenceType::COLUMN_PERCENTAGE;
            case 7:
                return DataPilotFieldReferenceType::TOTAL_PERCENTAGE;
            case 8:
                return DataPilotFieldReferenceType::INDEX;
        }
        return DataPilotFieldReferenceType::NONE;

    }
}

ScDPFunctionDlg::ScDPFunctionDlg(
        weld::Window* pParent, const ScDPLabelDataVector& rLabelVec,
        const ScDPLabelData& rLabelData, const ScPivotFuncData& rFuncData)
    : GenericDialogController(pParent, "modules/scalc/ui/datafielddialog.ui", "DataFieldDialog")
    , mxLbFunc(new ScDPFunctionListBox(m_xBuilder->weld_tree_view("functions")))
    , mxFtName(m_xBuilder->weld_label("name"))
    , mxLbType(m_xBuilder->weld_combo_box("type"))
    , mxFtBaseField(m_xBuilder->weld_label("basefieldft"))
    , mxLbBaseField(m_xBuilder->weld_combo_box("basefield"))
    , mxFtBaseItem(m_xBuilder->weld_label("baseitemft"))
    , mxLbBaseItem(m_xBuilder->weld_combo_box("baseitem"))
    , mxBtnOk(m_xBuilder->weld_button("ok"))
    , mrLabelVec(rLabelVec)
    , mbEmptyItem(false)
{
    mxLbFunc->set_size_request(-1, mxLbFunc->get_height_rows(8));

    Init(rLabelData, rFuncData);
}

ScDPFunctionDlg::~ScDPFunctionDlg()
{
}

PivotFunc ScDPFunctionDlg::GetFuncMask() const
{
    return mxLbFunc->GetSelection();
}

DataPilotFieldReference ScDPFunctionDlg::GetFieldRef() const
{
    DataPilotFieldReference aRef;

    aRef.ReferenceType = ToDataPilotFieldReferenceType(mxLbType->get_active());
    aRef.ReferenceField = GetBaseFieldName(mxLbBaseField->get_active_text());

    sal_Int32 nBaseItemPos = mxLbBaseItem->get_active();
    switch( nBaseItemPos )
    {
        case SC_BASEITEM_PREV_POS:
            aRef.ReferenceItemType = DataPilotFieldReferenceItemType::PREVIOUS;
        break;
        case SC_BASEITEM_NEXT_POS:
            aRef.ReferenceItemType = DataPilotFieldReferenceItemType::NEXT;
        break;
        default:
        {
            aRef.ReferenceItemType = DataPilotFieldReferenceItemType::NAMED;
            if( !mbEmptyItem || (nBaseItemPos > SC_BASEITEM_USER_POS) )
                aRef.ReferenceItemName = GetBaseItemName(mxLbBaseItem->get_active_text());
        }
    }

    return aRef;
}

void ScDPFunctionDlg::Init( const ScDPLabelData& rLabelData, const ScPivotFuncData& rFuncData )
{
    // list box
    PivotFunc nFuncMask = (rFuncData.mnFuncMask == PivotFunc::NONE) ? PivotFunc::Sum : rFuncData.mnFuncMask;
    mxLbFunc->SetSelection( nFuncMask );

    // field name
    mxFtName->set_label(rLabelData.getDisplayName());

    // handlers
    mxLbFunc->connect_row_activated( LINK( this, ScDPFunctionDlg, DblClickHdl ) );
    mxLbType->connect_changed( LINK( this, ScDPFunctionDlg, SelectHdl ) );
    mxLbBaseField->connect_changed( LINK( this, ScDPFunctionDlg, SelectHdl ) );

    // base field list box
    OUString aSelectedEntry;
    for( const auto& rxLabel : mrLabelVec )
    {
        mxLbBaseField->append_text(rxLabel->getDisplayName());
        maBaseFieldNameMap.emplace(rxLabel->getDisplayName(), rxLabel->maName);
        if (rxLabel->maName == rFuncData.maFieldRef.ReferenceField)
            aSelectedEntry = rxLabel->getDisplayName();
    }

    // select field reference type
    mxLbType->set_active(FromDataPilotFieldReferenceType(rFuncData.maFieldRef.ReferenceType));
    SelectHdl( *mxLbType );         // enables base field/item list boxes

    // select base field
    mxLbBaseField->set_active_text(aSelectedEntry);
    if (mxLbBaseField->get_active() == -1)
        mxLbBaseField->set_active(0);
    SelectHdl( *mxLbBaseField );    // fills base item list, selects base item

    // select base item
    switch( rFuncData.maFieldRef.ReferenceItemType )
    {
        case DataPilotFieldReferenceItemType::PREVIOUS:
            mxLbBaseItem->set_active( SC_BASEITEM_PREV_POS );
        break;
        case DataPilotFieldReferenceItemType::NEXT:
            mxLbBaseItem->set_active( SC_BASEITEM_NEXT_POS );
        break;
        default:
        {
            if( mbEmptyItem && rFuncData.maFieldRef.ReferenceItemName.isEmpty() )
            {
                // select special "(empty)" entry added before other items
                mxLbBaseItem->set_active( SC_BASEITEM_USER_POS );
            }
            else
            {
                sal_Int32 nStartPos = mbEmptyItem ? (SC_BASEITEM_USER_POS + 1) : SC_BASEITEM_USER_POS;
                sal_Int32 nPos = FindBaseItemPos( rFuncData.maFieldRef.ReferenceItemName, nStartPos );
                if( nPos == -1)
                    nPos = (mxLbBaseItem->get_count() > SC_BASEITEM_USER_POS) ? SC_BASEITEM_USER_POS : SC_BASEITEM_PREV_POS;
                mxLbBaseItem->set_active( nPos );
            }
        }
    }
}

const OUString& ScDPFunctionDlg::GetBaseFieldName(const OUString& rLayoutName) const
{
    NameMapType::const_iterator itr = maBaseFieldNameMap.find(rLayoutName);
    return itr == maBaseFieldNameMap.end() ? rLayoutName : itr->second;
}

const OUString& ScDPFunctionDlg::GetBaseItemName(const OUString& rLayoutName) const
{
    NameMapType::const_iterator itr = maBaseItemNameMap.find(rLayoutName);
    return itr == maBaseItemNameMap.end() ? rLayoutName : itr->second;
}

sal_Int32 ScDPFunctionDlg::FindBaseItemPos( const OUString& rEntry, sal_Int32 nStartPos ) const
{
    sal_Int32 nPos = nStartPos;
    bool bFound = false;
    while (nPos < mxLbBaseItem->get_count())
    {
        // translate the displayed field name back to its original field name.
        const OUString& rInName = mxLbBaseItem->get_text(nPos);
        const OUString& rName = GetBaseItemName(rInName);
        if (rName == rEntry)
        {
            bFound = true;
            break;
        }
        ++nPos;
    }
    return bFound ? nPos : -1;
}

IMPL_LINK( ScDPFunctionDlg, SelectHdl, weld::ComboBox&, rLBox, void )
{
    if (&rLBox == mxLbType.get())
    {
        bool bEnableField, bEnableItem;
        switch (ToDataPilotFieldReferenceType(mxLbType->get_active()))
        {
            case DataPilotFieldReferenceType::ITEM_DIFFERENCE:
            case DataPilotFieldReferenceType::ITEM_PERCENTAGE:
            case DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE:
                bEnableField = bEnableItem = true;
            break;

            case DataPilotFieldReferenceType::RUNNING_TOTAL:
                bEnableField = true;
                bEnableItem = false;
            break;

            default:
                bEnableField = bEnableItem = false;
        }

        bEnableField &= (mxLbBaseField->get_count() > 0);
        mxFtBaseField->set_sensitive( bEnableField );
        mxLbBaseField->set_sensitive( bEnableField );

        bEnableItem &= bEnableField;
        mxFtBaseItem->set_sensitive( bEnableItem );
        mxLbBaseItem->set_sensitive( bEnableItem );
    }
    else if (&rLBox == mxLbBaseField.get())
    {
        // keep "previous" and "next" entries
        while (mxLbBaseItem->get_count() > SC_BASEITEM_USER_POS)
             mxLbBaseItem->remove(SC_BASEITEM_USER_POS);

        // update item list for current base field
        mbEmptyItem = false;
        size_t nBasePos = mxLbBaseField->get_active();
        if (nBasePos < mrLabelVec.size())
        {
            const vector<ScDPLabelData::Member>& rMembers = mrLabelVec[nBasePos]->maMembers;
            mbEmptyItem = lclFillListBox(*mxLbBaseItem, rMembers, SC_BASEITEM_USER_POS);
            // build cache for base names.
            NameMapType aMap;
            for (const auto& rMember : rMembers)
                aMap.emplace(rMember.getDisplayName(), rMember.maName);
            maBaseItemNameMap.swap(aMap);
        }

        // select base item
        sal_uInt16 nItemPos = (mxLbBaseItem->get_count() > SC_BASEITEM_USER_POS) ? SC_BASEITEM_USER_POS : SC_BASEITEM_PREV_POS;
        mxLbBaseItem->set_active( nItemPos );
    }
}

IMPL_LINK_NOARG(ScDPFunctionDlg, DblClickHdl, weld::TreeView&, void)
{
    m_xDialog->response(RET_OK);
}

ScDPSubtotalDlg::ScDPSubtotalDlg(weld::Window* pParent, ScDPObject& rDPObj,
        const ScDPLabelData& rLabelData, const ScPivotFuncData& rFuncData,
        const ScDPNameVec& rDataFields, bool bEnableLayout)
    : GenericDialogController(pParent, "modules/scalc/ui/pivotfielddialog.ui", "PivotFieldDialog")
    , mrDPObj(rDPObj)
    , mrDataFields(rDataFields)
    , maLabelData(rLabelData)
    , mbEnableLayout(bEnableLayout)
    , mxRbNone(m_xBuilder->weld_radio_button("none"))
    , mxRbAuto(m_xBuilder->weld_radio_button("auto"))
    , mxRbUser(m_xBuilder->weld_radio_button("user"))
    , mxLbFunc(new ScDPFunctionListBox(m_xBuilder->weld_tree_view("functions")))
    , mxFtName(m_xBuilder->weld_label("name"))
    , mxCbShowAll(m_xBuilder->weld_check_button("showall"))
    , mxBtnOk(m_xBuilder->weld_button("ok"))
    , mxBtnOptions(m_xBuilder->weld_button("options"))
{
    mxLbFunc->set_selection_mode(SelectionMode::Multiple);
    mxLbFunc->set_size_request(-1, mxLbFunc->get_height_rows(8));
    Init(rLabelData, rFuncData);
}

ScDPSubtotalDlg::~ScDPSubtotalDlg()
{
}

PivotFunc ScDPSubtotalDlg::GetFuncMask() const
{
    PivotFunc nFuncMask = PivotFunc::NONE;

    if (mxRbAuto->get_active())
        nFuncMask = PivotFunc::Auto;
    else if (mxRbUser->get_active())
        nFuncMask = mxLbFunc->GetSelection();

    return nFuncMask;
}

void ScDPSubtotalDlg::FillLabelData( ScDPLabelData& rLabelData ) const
{
    rLabelData.mnFuncMask = GetFuncMask();
    rLabelData.mnUsedHier = maLabelData.mnUsedHier;
    rLabelData.mbShowAll = mxCbShowAll->get_active();
    rLabelData.maMembers = maLabelData.maMembers;
    rLabelData.maSortInfo = maLabelData.maSortInfo;
    rLabelData.maLayoutInfo = maLabelData.maLayoutInfo;
    rLabelData.maShowInfo = maLabelData.maShowInfo;
    rLabelData.mbRepeatItemLabels = maLabelData.mbRepeatItemLabels;
}

void ScDPSubtotalDlg::Init( const ScDPLabelData& rLabelData, const ScPivotFuncData& rFuncData )
{
    // field name
    mxFtName->set_label(rLabelData.getDisplayName());

    // radio buttons
    mxRbNone->connect_clicked( LINK( this, ScDPSubtotalDlg, RadioClickHdl ) );
    mxRbAuto->connect_clicked( LINK( this, ScDPSubtotalDlg, RadioClickHdl ) );
    mxRbUser->connect_clicked( LINK( this, ScDPSubtotalDlg, RadioClickHdl ) );

    weld::RadioButton* pRBtn = nullptr;
    switch( rFuncData.mnFuncMask )
    {
        case PivotFunc::NONE:   pRBtn = mxRbNone.get();  break;
        case PivotFunc::Auto:   pRBtn = mxRbAuto.get();  break;
        default:                pRBtn = mxRbUser.get();
    }
    pRBtn->set_active(true);
    RadioClickHdl(*pRBtn);

    // list box
    mxLbFunc->SetSelection( rFuncData.mnFuncMask );
    mxLbFunc->connect_row_activated( LINK( this, ScDPSubtotalDlg, DblClickHdl ) );

    // show all
    mxCbShowAll->set_active( rLabelData.mbShowAll );

    // options
    mxBtnOptions->connect_clicked( LINK( this, ScDPSubtotalDlg, ClickHdl ) );
}

IMPL_LINK(ScDPSubtotalDlg, RadioClickHdl, weld::Button&, rBtn, void)
{
    mxLbFunc->set_sensitive(&rBtn == mxRbUser.get());
}

IMPL_LINK_NOARG(ScDPSubtotalDlg, DblClickHdl, weld::TreeView&, void)
{
    m_xDialog->response(RET_OK);
}

IMPL_LINK(ScDPSubtotalDlg, ClickHdl, weld::Button&, rBtn, void)
{
    if (&rBtn == mxBtnOptions.get())
    {
        ScDPSubtotalOptDlg aDlg(m_xDialog.get(), mrDPObj, maLabelData, mrDataFields, mbEnableLayout);
        if (aDlg.run() == RET_OK)
            aDlg.FillLabelData(maLabelData);
    }
}

namespace
{
    int FromDataPilotFieldLayoutMode(int eMode)
    {
        switch (eMode)
        {
            case DataPilotFieldLayoutMode::TABULAR_LAYOUT:
                return 0;
            case DataPilotFieldLayoutMode::OUTLINE_SUBTOTALS_TOP:
                return 1;
            case DataPilotFieldLayoutMode::OUTLINE_SUBTOTALS_BOTTOM:
                return 2;
        }
        return -1;
    }

    int ToDataPilotFieldLayoutMode(int nPos)
    {
        switch (nPos)
        {
            case 0:
                return DataPilotFieldLayoutMode::TABULAR_LAYOUT;
            case 1:
                return DataPilotFieldLayoutMode::OUTLINE_SUBTOTALS_TOP;
            case 2:
                return DataPilotFieldLayoutMode::OUTLINE_SUBTOTALS_BOTTOM;
        }
        return DataPilotFieldLayoutMode::TABULAR_LAYOUT;
    }

    int FromDataPilotFieldShowItemsMode(int eMode)
    {
        switch (eMode)
        {
            case DataPilotFieldShowItemsMode::FROM_TOP:
                return 0;
            case DataPilotFieldShowItemsMode::FROM_BOTTOM:
                return 1;
        }
        return -1;
    }

    int ToDataPilotFieldShowItemsMode(int nPos)
    {
        switch (nPos)
        {
            case 0:
                return DataPilotFieldShowItemsMode::FROM_TOP;
            case 1:
                return DataPilotFieldShowItemsMode::FROM_BOTTOM;
        }
        return DataPilotFieldShowItemsMode::FROM_TOP;
    }
}

ScDPSubtotalOptDlg::ScDPSubtotalOptDlg(weld::Window* pParent, ScDPObject& rDPObj,
        const ScDPLabelData& rLabelData, const ScDPNameVec& rDataFields,
        bool bEnableLayout )
    : GenericDialogController(pParent, "modules/scalc/ui/datafieldoptionsdialog.ui",
                              "DataFieldOptionsDialog")
    , m_xLbSortBy(m_xBuilder->weld_combo_box("sortby"))
    , m_xRbSortAsc(m_xBuilder->weld_radio_button("ascending"))
    , m_xRbSortDesc(m_xBuilder->weld_radio_button("descending"))
    , m_xRbSortMan(m_xBuilder->weld_radio_button("manual"))
    , m_xLayoutFrame(m_xBuilder->weld_widget("layoutframe"))
    , m_xLbLayout(m_xBuilder->weld_combo_box("layout"))
    , m_xCbLayoutEmpty(m_xBuilder->weld_check_button("emptyline"))
    , m_xCbRepeatItemLabels(m_xBuilder->weld_check_button("repeatitemlabels"))
    , m_xCbShow(m_xBuilder->weld_check_button("show"))
    , m_xNfShow(m_xBuilder->weld_spin_button("items"))
    , m_xFtShow(m_xBuilder->weld_label("showft"))
    , m_xFtShowFrom(m_xBuilder->weld_label("showfromft"))
    , m_xLbShowFrom(m_xBuilder->weld_combo_box("from"))
    , m_xFtShowUsing(m_xBuilder->weld_label("usingft"))
    , m_xLbShowUsing(m_xBuilder->weld_combo_box("using"))
    , m_xHideFrame(m_xBuilder->weld_widget("hideframe"))
    , m_xLbHide(m_xBuilder->weld_tree_view("hideitems"))
    , m_xFtHierarchy(m_xBuilder->weld_label("hierarchyft"))
    , m_xLbHierarchy(m_xBuilder->weld_combo_box("hierarchy"))
    , mrDPObj(rDPObj)
    , maLabelData(rLabelData)
{
    std::vector<int> aWidths;
    aWidths.push_back(m_xLbHide->get_checkbox_column_width());
    m_xLbHide->set_column_fixed_widths(aWidths);

    m_xLbSortBy->set_size_request(m_xLbSortBy->get_approximate_digit_width() * 18, -1);
    m_xLbHide->set_size_request(-1, m_xLbHide->get_height_rows(5));
    Init(rDataFields, bEnableLayout);
}

ScDPSubtotalOptDlg::~ScDPSubtotalOptDlg()
{
}

void ScDPSubtotalOptDlg::FillLabelData( ScDPLabelData& rLabelData ) const
{
    // *** SORTING ***

    if (m_xRbSortMan->get_active())
        rLabelData.maSortInfo.Mode = DataPilotFieldSortMode::MANUAL;
    else if (m_xLbSortBy->get_active() == SC_SORTNAME_POS)
        rLabelData.maSortInfo.Mode = DataPilotFieldSortMode::NAME;
    else
        rLabelData.maSortInfo.Mode = DataPilotFieldSortMode::DATA;

    ScDPName aFieldName = GetFieldName(m_xLbSortBy->get_active_text());
    if (!aFieldName.maName.isEmpty())
    {
        rLabelData.maSortInfo.Field =
            ScDPUtil::createDuplicateDimensionName(aFieldName.maName, aFieldName.mnDupCount);
        rLabelData.maSortInfo.IsAscending = m_xRbSortAsc->get_active();
    }

    // *** LAYOUT MODE ***

    rLabelData.maLayoutInfo.LayoutMode = ToDataPilotFieldLayoutMode(m_xLbLayout->get_active());
    rLabelData.maLayoutInfo.AddEmptyLines = m_xCbLayoutEmpty->get_active();
    rLabelData.mbRepeatItemLabels = m_xCbRepeatItemLabels->get_active();

    // *** AUTO SHOW ***

    aFieldName = GetFieldName(m_xLbShowUsing->get_active_text());
    if (!aFieldName.maName.isEmpty())
    {
        rLabelData.maShowInfo.IsEnabled = m_xCbShow->get_active();
        rLabelData.maShowInfo.ShowItemsMode = ToDataPilotFieldShowItemsMode(m_xLbShowFrom->get_active());
        rLabelData.maShowInfo.ItemCount = sal::static_int_cast<sal_Int32>( m_xNfShow->get_value() );
        rLabelData.maShowInfo.DataField =
            ScDPUtil::createDuplicateDimensionName(aFieldName.maName, aFieldName.mnDupCount);
    }

    // *** HIDDEN ITEMS ***

    rLabelData.maMembers = maLabelData.maMembers;
    int nVisCount = m_xLbHide->n_children();
    for (int nPos = 0; nPos < nVisCount; ++nPos)
        rLabelData.maMembers[nPos].mbVisible = !m_xLbHide->get_toggle(nPos, 0);

    // *** HIERARCHY ***

    rLabelData.mnUsedHier = m_xLbHierarchy->get_active() != -1 ? m_xLbHierarchy->get_active() : 0;
}

void ScDPSubtotalOptDlg::Init( const ScDPNameVec& rDataFields, bool bEnableLayout )
{
    // *** SORTING ***

    sal_Int32 nSortMode = maLabelData.maSortInfo.Mode;

    // sort fields list box
    m_xLbSortBy->append_text(maLabelData.getDisplayName());

    for( const auto& rDataField : rDataFields )
    {
        // Cache names for later lookup.
        maDataFieldNameMap.emplace(rDataField.maLayoutName, rDataField);

        m_xLbSortBy->append_text(rDataField.maLayoutName);
        m_xLbShowUsing->append_text(rDataField.maLayoutName);  // for AutoShow
    }

    sal_Int32 nSortPos = SC_SORTNAME_POS;
    if( nSortMode == DataPilotFieldSortMode::DATA )
    {
        nSortPos = FindListBoxEntry( *m_xLbSortBy, maLabelData.maSortInfo.Field, SC_SORTDATA_POS );
        if( nSortPos == -1 )
        {
            nSortPos = SC_SORTNAME_POS;
            nSortMode = DataPilotFieldSortMode::MANUAL;
        }
    }
    m_xLbSortBy->set_active(nSortPos);

    // sorting mode
    m_xRbSortAsc->connect_clicked( LINK( this, ScDPSubtotalOptDlg, RadioClickHdl ) );
    m_xRbSortDesc->connect_clicked( LINK( this, ScDPSubtotalOptDlg, RadioClickHdl ) );
    m_xRbSortMan->connect_clicked( LINK( this, ScDPSubtotalOptDlg, RadioClickHdl ) );

    weld::RadioButton* pRBtn = nullptr;
    switch( nSortMode )
    {
        case DataPilotFieldSortMode::NONE:
        case DataPilotFieldSortMode::MANUAL:
            pRBtn = m_xRbSortMan.get();
        break;
        default:
            pRBtn = maLabelData.maSortInfo.IsAscending ? m_xRbSortAsc.get() : m_xRbSortDesc.get();
    }
    pRBtn->set_active(true);
    RadioClickHdl(*pRBtn);

    // *** LAYOUT MODE ***

    m_xLayoutFrame->set_sensitive(bEnableLayout);

    m_xLbLayout->set_active(FromDataPilotFieldLayoutMode(maLabelData.maLayoutInfo.LayoutMode));
    m_xCbLayoutEmpty->set_active( maLabelData.maLayoutInfo.AddEmptyLines );
    m_xCbRepeatItemLabels->set_active( maLabelData.mbRepeatItemLabels );

    // *** AUTO SHOW ***

    m_xCbShow->set_active( maLabelData.maShowInfo.IsEnabled );
    m_xCbShow->connect_clicked( LINK( this, ScDPSubtotalOptDlg, CheckHdl ) );

    m_xLbShowFrom->set_active(FromDataPilotFieldShowItemsMode(maLabelData.maShowInfo.ShowItemsMode));
    long nCount = static_cast< long >( maLabelData.maShowInfo.ItemCount );
    if( nCount < 1 )
        nCount = SC_SHOW_DEFAULT;
    m_xNfShow->set_value( nCount );

    // m_xLbShowUsing already filled above
    m_xLbShowUsing->set_active_text(maLabelData.maShowInfo.DataField);
    if (m_xLbShowUsing->get_active() == -1)
        m_xLbShowUsing->set_active(0);

    CheckHdl(*m_xCbShow);      // enable/disable dependent controls

    // *** HIDDEN ITEMS ***

    InitHideListBox();

    // *** HIERARCHY ***

    if( maLabelData.maHiers.getLength() > 1 )
    {
        lclFillListBox(*m_xLbHierarchy, maLabelData.maHiers);
        sal_Int32 nHier = maLabelData.mnUsedHier;
        if( (nHier < 0) || (nHier >= maLabelData.maHiers.getLength()) ) nHier = 0;
        m_xLbHierarchy->set_active( nHier );
        m_xLbHierarchy->connect_changed( LINK( this, ScDPSubtotalOptDlg, SelectHdl ) );
    }
    else
    {
        m_xFtHierarchy->set_sensitive(false);
        m_xLbHierarchy->set_sensitive(false);
    }
}

void ScDPSubtotalOptDlg::InitHideListBox()
{
    m_xLbHide->clear();
    lclFillListBox(*m_xLbHide, maLabelData.maMembers);
    size_t n = maLabelData.maMembers.size();
    for (size_t i = 0; i < n; ++i)
        m_xLbHide->set_toggle(i, !maLabelData.maMembers[i].mbVisible, 0);
    bool bEnable = m_xLbHide->n_children() > 0;
    m_xHideFrame->set_sensitive(bEnable);
}

ScDPName ScDPSubtotalOptDlg::GetFieldName(const OUString& rLayoutName) const
{
    NameMapType::const_iterator itr = maDataFieldNameMap.find(rLayoutName);
    return itr == maDataFieldNameMap.end() ? ScDPName() : itr->second;
}

sal_Int32 ScDPSubtotalOptDlg::FindListBoxEntry(
    const weld::ComboBox& rLBox, const OUString& rEntry, sal_Int32 nStartPos ) const
{
    sal_Int32 nPos = nStartPos;
    bool bFound = false;
    while (nPos < rLBox.get_count())
    {
        // translate the displayed field name back to its original field name.
        ScDPName aName = GetFieldName(rLBox.get_text(nPos));
        OUString aUnoName = ScDPUtil::createDuplicateDimensionName(aName.maName, aName.mnDupCount);
        if (aUnoName == rEntry)
        {
            bFound = true;
            break;
        }
        ++nPos;
    }
    return bFound ? nPos : -1;
}

IMPL_LINK(ScDPSubtotalOptDlg, RadioClickHdl, weld::Button&, rBtn, void)
{
    m_xLbSortBy->set_sensitive(&rBtn != m_xRbSortMan.get());
}

IMPL_LINK(ScDPSubtotalOptDlg, CheckHdl, weld::Button&, rCBox, void)
{
    if (&rCBox == m_xCbShow.get())
    {
        bool bEnable = m_xCbShow->get_active();
        m_xNfShow->set_sensitive( bEnable );
        m_xFtShow->set_sensitive( bEnable );
        m_xFtShowFrom->set_sensitive( bEnable );
        m_xLbShowFrom->set_sensitive( bEnable );

        bool bEnableUsing = bEnable && (m_xLbShowUsing->get_count() > 0);
        m_xFtShowUsing->set_sensitive(bEnableUsing);
        m_xLbShowUsing->set_sensitive(bEnableUsing);
    }
}

IMPL_LINK_NOARG(ScDPSubtotalOptDlg, SelectHdl, weld::ComboBox&, void)
{
    mrDPObj.GetMembers(maLabelData.mnCol, m_xLbHierarchy->get_active(), maLabelData.maMembers);
    InitHideListBox();
}

ScDPShowDetailDlg::ScDPShowDetailDlg(weld::Window* pParent, ScDPObject& rDPObj, css::sheet::DataPilotFieldOrientation nOrient)
    : GenericDialogController(pParent, "modules/scalc/ui/showdetaildialog.ui", "ShowDetail")
    , mrDPObj(rDPObj)
    , mxLbDims(m_xBuilder->weld_tree_view("dimsTreeview"))
{
    ScDPSaveData* pSaveData = rDPObj.GetSaveData();
    long nDimCount = rDPObj.GetDimCount();
    for (long nDim=0; nDim<nDimCount; nDim++)
    {
        bool bIsDataLayout;
        sal_Int32 nDimFlags = 0;
        OUString aName = rDPObj.GetDimName( nDim, bIsDataLayout, &nDimFlags );
        if ( !bIsDataLayout && !rDPObj.IsDuplicated( nDim ) && ScDPObject::IsOrientationAllowed( nOrient, nDimFlags ) )
        {
            const ScDPSaveDimension* pDimension = pSaveData ? pSaveData->GetExistingDimensionByName(aName) : nullptr;
            if ( !pDimension || (pDimension->GetOrientation() != nOrient) )
            {
                if (pDimension)
                {
                    const boost::optional<OUString> & pLayoutName = pDimension->GetLayoutName();
                    if (pLayoutName)
                        aName = *pLayoutName;
                }
                mxLbDims->append_text(aName);
                maNameIndexMap.emplace(aName, nDim);
            }
        }
    }
    if (mxLbDims->n_children())
       mxLbDims->select(0);

    mxLbDims->connect_row_activated(LINK(this, ScDPShowDetailDlg, DblClickHdl));
}

ScDPShowDetailDlg::~ScDPShowDetailDlg()
{
}

short ScDPShowDetailDlg::run()
{
    return mxLbDims->n_children() ? GenericDialogController::run() : static_cast<short>(RET_CANCEL);
}

OUString ScDPShowDetailDlg::GetDimensionName() const
{
    // Look up the internal dimension name which may be different from the
    // displayed field name.
    OUString aSelectedName = mxLbDims->get_selected_text();
    DimNameIndexMap::const_iterator itr = maNameIndexMap.find(aSelectedName);
    if (itr == maNameIndexMap.end())
        // This should never happen!
        return aSelectedName;

    long nDim = itr->second;
    bool bIsDataLayout = false;
    return mrDPObj.GetDimName(nDim, bIsDataLayout);
}

IMPL_LINK_NOARG(ScDPShowDetailDlg, DblClickHdl, weld::TreeView&, void)
{
    m_xDialog->response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
