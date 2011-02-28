/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION

#include "pvfundlg.hxx"

#include <com/sun/star/sheet/DataPilotFieldReferenceType.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceItemType.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldShowItemsMode.hpp>

#include <tools/resary.hxx>
#include <vcl/msgbox.hxx>

#include "scresid.hxx"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "pvfundlg.hrc"
#include "globstr.hrc"

#include <vector>

// ============================================================================

using namespace ::com::sun::star::sheet;

using ::rtl::OUString;
using ::com::sun::star::uno::Sequence;
using ::std::vector;

// ============================================================================

namespace {

/** Appends all strings from the Sequence to the list box.

    Empty strings are replaced by a localized "(empty)" entry and inserted at
    the specified position.

    @return  true = The passed string list contains an empty string entry.
 */
template< typename ListBoxType >
bool lclFillListBox( ListBoxType& rLBox, const Sequence< OUString >& rStrings, sal_uInt16 nEmptyPos = LISTBOX_APPEND )
{
    bool bEmpty = false;
    if( const OUString* pStr = rStrings.getConstArray() )
    {
        for( const OUString* pEnd = pStr + rStrings.getLength(); pStr != pEnd; ++pStr )
        {
            if( pStr->getLength() )
                rLBox.InsertEntry( *pStr );
            else
            {
                rLBox.InsertEntry( ScGlobal::GetRscString( STR_EMPTYDATA ), nEmptyPos );
                bEmpty = true;
            }
        }
    }
    return bEmpty;
}

template< typename ListBoxType >
bool lclFillListBox( ListBoxType& rLBox, const vector<ScDPLabelData::Member>& rMembers, sal_uInt16 nEmptyPos = LISTBOX_APPEND )
{
    bool bEmpty = false;
    vector<ScDPLabelData::Member>::const_iterator itr = rMembers.begin(), itrEnd = rMembers.end();
    for (; itr != itrEnd; ++itr)
    {
        OUString aName = itr->getDisplayName();
        if (aName.getLength())
            rLBox.InsertEntry(aName);
        else
        {
            rLBox.InsertEntry(ScGlobal::GetRscString(STR_EMPTYDATA), nEmptyPos);
            bEmpty = true;
        }
    }
    return bEmpty;
}

/** Searches for a listbox entry, starts search at specified position. */
sal_uInt16 lclFindListBoxEntry( const ListBox& rLBox, const String& rEntry, sal_uInt16 nStartPos )
{
    sal_uInt16 nPos = nStartPos;
    while( (nPos < rLBox.GetEntryCount()) && (rLBox.GetEntry( nPos ) != rEntry) )
        ++nPos;
    return (nPos < rLBox.GetEntryCount()) ? nPos : LISTBOX_ENTRY_NOTFOUND;
}

/** This table represents the order of the strings in the resource string array. */
static const sal_uInt16 spnFunctions[] =
{
    PIVOT_FUNC_SUM,
    PIVOT_FUNC_COUNT,
    PIVOT_FUNC_AVERAGE,
    PIVOT_FUNC_MAX,
    PIVOT_FUNC_MIN,
    PIVOT_FUNC_PRODUCT,
    PIVOT_FUNC_COUNT_NUM,
    PIVOT_FUNC_STD_DEV,
    PIVOT_FUNC_STD_DEVP,
    PIVOT_FUNC_STD_VAR,
    PIVOT_FUNC_STD_VARP
};

const sal_uInt16 SC_BASEITEM_PREV_POS = 0;
const sal_uInt16 SC_BASEITEM_NEXT_POS = 1;
const sal_uInt16 SC_BASEITEM_USER_POS = 2;

const sal_uInt16 SC_SORTNAME_POS = 0;
const sal_uInt16 SC_SORTDATA_POS = 1;

const long SC_SHOW_DEFAULT = 10;

static const ScDPListBoxWrapper::MapEntryType spRefTypeMap[] =
{
    { 0,                        DataPilotFieldReferenceType::NONE                       },
    { 1,                        DataPilotFieldReferenceType::ITEM_DIFFERENCE            },
    { 2,                        DataPilotFieldReferenceType::ITEM_PERCENTAGE            },
    { 3,                        DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE },
    { 4,                        DataPilotFieldReferenceType::RUNNING_TOTAL              },
    { 5,                        DataPilotFieldReferenceType::ROW_PERCENTAGE             },
    { 6,                        DataPilotFieldReferenceType::COLUMN_PERCENTAGE          },
    { 7,                        DataPilotFieldReferenceType::TOTAL_PERCENTAGE           },
    { 8,                        DataPilotFieldReferenceType::INDEX                      },
    { LISTBOX_ENTRY_NOTFOUND,   DataPilotFieldReferenceType::NONE                       }
};

static const ScDPListBoxWrapper::MapEntryType spLayoutMap[] =
{
    { 0,                        DataPilotFieldLayoutMode::TABULAR_LAYOUT            },
    { 1,                        DataPilotFieldLayoutMode::OUTLINE_SUBTOTALS_TOP     },
    { 2,                        DataPilotFieldLayoutMode::OUTLINE_SUBTOTALS_BOTTOM  },
    { LISTBOX_ENTRY_NOTFOUND,   DataPilotFieldLayoutMode::TABULAR_LAYOUT            }
};

static const ScDPListBoxWrapper::MapEntryType spShowFromMap[] =
{
    { 0,                        DataPilotFieldShowItemsMode::FROM_TOP       },
    { 1,                        DataPilotFieldShowItemsMode::FROM_BOTTOM    },
    { LISTBOX_ENTRY_NOTFOUND,   DataPilotFieldShowItemsMode::FROM_TOP       }
};

} // namespace

// ============================================================================

ScDPFunctionListBox::ScDPFunctionListBox( Window* pParent, const ResId& rResId ) :
    MultiListBox( pParent, rResId )
{
    FillFunctionNames();
}

void ScDPFunctionListBox::SetSelection( sal_uInt16 nFuncMask )
{
    if( (nFuncMask == PIVOT_FUNC_NONE) || (nFuncMask == PIVOT_FUNC_AUTO) )
        SetNoSelection();
    else
        for( sal_uInt16 nEntry = 0, nCount = GetEntryCount(); nEntry < nCount; ++nEntry )
            SelectEntryPos( nEntry, (nFuncMask & spnFunctions[ nEntry ]) != 0 );
}

sal_uInt16 ScDPFunctionListBox::GetSelection() const
{
    sal_uInt16 nFuncMask = PIVOT_FUNC_NONE;
    for( sal_uInt16 nSel = 0, nCount = GetSelectEntryCount(); nSel < nCount; ++nSel )
        nFuncMask |= spnFunctions[ GetSelectEntryPos( nSel ) ];
    return nFuncMask;
}

void ScDPFunctionListBox::FillFunctionNames()
{
    DBG_ASSERT( !GetEntryCount(), "ScDPFunctionListBox::FillFunctionNames - do not add texts to resource" );
    Clear();
    ResStringArray aArr( ScResId( SCSTR_DPFUNCLISTBOX ) );
    for( sal_uInt16 nIndex = 0, nCount = sal::static_int_cast<sal_uInt16>(aArr.Count()); nIndex < nCount; ++nIndex )
        InsertEntry( aArr.GetString( nIndex ) );
}

// ============================================================================

ScDPFunctionDlg::ScDPFunctionDlg(
        Window* pParent, const ScDPLabelDataVector& rLabelVec,
        const ScDPLabelData& rLabelData, const ScPivotFuncData& rFuncData ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_DPDATAFIELD ) ),
    maFlFunc        ( this, ScResId( FL_FUNC ) ),
    maLbFunc        ( this, ScResId( LB_FUNC ) ),
    maFtNameLabel   ( this, ScResId( FT_NAMELABEL ) ),
    maFtName        ( this, ScResId( FT_NAME ) ),
    maFlDisplay     ( this, ScResId( FL_DISPLAY ) ),
    maFtType        ( this, ScResId( FT_TYPE ) ),
    maLbType        ( this, ScResId( LB_TYPE ) ),
    maFtBaseField   ( this, ScResId( FT_BASEFIELD ) ),
    maLbBaseField   ( this, ScResId( LB_BASEFIELD ) ),
    maFtBaseItem    ( this, ScResId( FT_BASEITEM ) ),
    maLbBaseItem    ( this, ScResId( LB_BASEITEM ) ),
    maBtnOk         ( this, ScResId( BTN_OK ) ),
    maBtnCancel     ( this, ScResId( BTN_CANCEL ) ),
    maBtnHelp       ( this, ScResId( BTN_HELP ) ),
    maBtnMore       ( this, ScResId( BTN_MORE ) ),
    maLbTypeWrp     ( maLbType, spRefTypeMap ),
    mrLabelVec      ( rLabelVec ),
    mbEmptyItem     ( false )
{
    FreeResource();
    Init( rLabelData, rFuncData );
}

sal_uInt16 ScDPFunctionDlg::GetFuncMask() const
{
    return maLbFunc.GetSelection();
}

DataPilotFieldReference ScDPFunctionDlg::GetFieldRef() const
{
    DataPilotFieldReference aRef;

    aRef.ReferenceType = maLbTypeWrp.GetControlValue();
    aRef.ReferenceField = maLbBaseField.GetSelectEntry();

    sal_uInt16 nBaseItemPos = maLbBaseItem.GetSelectEntryPos();
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
                aRef.ReferenceItemName = maLbBaseItem.GetSelectEntry();
        }
    }

    return aRef;
}

void ScDPFunctionDlg::Init( const ScDPLabelData& rLabelData, const ScPivotFuncData& rFuncData )
{
    // list box
    sal_uInt16 nFuncMask = (rFuncData.mnFuncMask == PIVOT_FUNC_NONE) ? PIVOT_FUNC_SUM : rFuncData.mnFuncMask;
    maLbFunc.SetSelection( nFuncMask );

    // field name
    maFtName.SetText(rLabelData.getDisplayName());

    // "More button" controls
    maBtnMore.AddWindow( &maFlDisplay );
    maBtnMore.AddWindow( &maFtType );
    maBtnMore.AddWindow( &maLbType );
    maBtnMore.AddWindow( &maFtBaseField );
    maBtnMore.AddWindow( &maLbBaseField );
    maBtnMore.AddWindow( &maFtBaseItem );
    maBtnMore.AddWindow( &maLbBaseItem );

    // handlers
    maLbFunc.SetDoubleClickHdl( LINK( this, ScDPFunctionDlg, DblClickHdl ) );
    maLbType.SetSelectHdl( LINK( this, ScDPFunctionDlg, SelectHdl ) );
    maLbBaseField.SetSelectHdl( LINK( this, ScDPFunctionDlg, SelectHdl ) );

    // base field list box
    for( ScDPLabelDataVector::const_iterator aIt = mrLabelVec.begin(), aEnd = mrLabelVec.end(); aIt != aEnd; ++aIt )
        maLbBaseField.InsertEntry(aIt->getDisplayName());

    // base item list box
    maLbBaseItem.SetSeparatorPos( SC_BASEITEM_USER_POS - 1 );

    // select field reference type
    maLbTypeWrp.SetControlValue( rFuncData.maFieldRef.ReferenceType );
    SelectHdl( &maLbType );         // enables base field/item list boxes

    // select base field
    maLbBaseField.SelectEntry( rFuncData.maFieldRef.ReferenceField );
    if( maLbBaseField.GetSelectEntryPos() >= maLbBaseField.GetEntryCount() )
        maLbBaseField.SelectEntryPos( 0 );
    SelectHdl( &maLbBaseField );    // fills base item list, selects base item

    // select base item
    switch( rFuncData.maFieldRef.ReferenceItemType )
    {
        case DataPilotFieldReferenceItemType::PREVIOUS:
            maLbBaseItem.SelectEntryPos( SC_BASEITEM_PREV_POS );
        break;
        case DataPilotFieldReferenceItemType::NEXT:
            maLbBaseItem.SelectEntryPos( SC_BASEITEM_NEXT_POS );
        break;
        default:
        {
            if( mbEmptyItem && !rFuncData.maFieldRef.ReferenceItemName.getLength() )
            {
                // select special "(empty)" entry added before other items
                maLbBaseItem.SelectEntryPos( SC_BASEITEM_USER_POS );
            }
            else
            {
                sal_uInt16 nStartPos = mbEmptyItem ? (SC_BASEITEM_USER_POS + 1) : SC_BASEITEM_USER_POS;
                sal_uInt16 nPos = lclFindListBoxEntry( maLbBaseItem, rFuncData.maFieldRef.ReferenceItemName, nStartPos );
                if( nPos >= maLbBaseItem.GetEntryCount() )
                    nPos = (maLbBaseItem.GetEntryCount() > SC_BASEITEM_USER_POS) ? SC_BASEITEM_USER_POS : SC_BASEITEM_PREV_POS;
                maLbBaseItem.SelectEntryPos( nPos );
            }
        }
    }
}

IMPL_LINK( ScDPFunctionDlg, SelectHdl, ListBox*, pLBox )
{
    if( pLBox == &maLbType )
    {
        bool bEnableField, bEnableItem;
        switch( maLbTypeWrp.GetControlValue() )
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

        bEnableField &= maLbBaseField.GetEntryCount() > 0;
        maFtBaseField.Enable( bEnableField );
        maLbBaseField.Enable( bEnableField );

        bEnableItem &= bEnableField;
        maFtBaseItem.Enable( bEnableItem );
        maLbBaseItem.Enable( bEnableItem );
    }
    else if( pLBox == &maLbBaseField )
    {
        // keep "previous" and "next" entries
        while( maLbBaseItem.GetEntryCount() > SC_BASEITEM_USER_POS )
            maLbBaseItem.RemoveEntry( SC_BASEITEM_USER_POS );

        // update item list for current base field
        mbEmptyItem = false;
        size_t nBasePos = maLbBaseField.GetSelectEntryPos();
        if( nBasePos < mrLabelVec.size() )
            mbEmptyItem = lclFillListBox( maLbBaseItem, mrLabelVec[ nBasePos ].maMembers, SC_BASEITEM_USER_POS );

        // select base item
        sal_uInt16 nItemPos = (maLbBaseItem.GetEntryCount() > SC_BASEITEM_USER_POS) ? SC_BASEITEM_USER_POS : SC_BASEITEM_PREV_POS;
        maLbBaseItem.SelectEntryPos( nItemPos );
    }
    return 0;
}

IMPL_LINK( ScDPFunctionDlg, DblClickHdl, MultiListBox*, EMPTYARG )
{
    maBtnOk.Click();
    return 0;
}

// ============================================================================

ScDPSubtotalDlg::ScDPSubtotalDlg( Window* pParent, ScDPObject& rDPObj,
        const ScDPLabelData& rLabelData, const ScPivotFuncData& rFuncData,
        const ScDPNameVec& rDataFields, bool bEnableLayout ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_PIVOTSUBT ) ),
    maFlSubt        ( this, ScResId( FL_FUNC ) ),
    maRbNone        ( this, ScResId( RB_NONE ) ),
    maRbAuto        ( this, ScResId( RB_AUTO ) ),
    maRbUser        ( this, ScResId( RB_USER ) ),
    maLbFunc        ( this, ScResId( LB_FUNC ) ),
    maFtNameLabel   ( this, ScResId( FT_NAMELABEL ) ),
    maFtName        ( this, ScResId( FT_NAME ) ),
    maCbShowAll     ( this, ScResId( CB_SHOWALL ) ),
    maBtnOk         ( this, ScResId( BTN_OK ) ),
    maBtnCancel     ( this, ScResId( BTN_CANCEL ) ),
    maBtnHelp       ( this, ScResId( BTN_HELP ) ),
    maBtnOptions    ( this, ScResId( BTN_OPTIONS ) ),
    mrDPObj         ( rDPObj ),
    mrDataFields    ( rDataFields ),
    maLabelData     ( rLabelData ),
    mbEnableLayout  ( bEnableLayout )
{
    FreeResource();
    Init( rLabelData, rFuncData );
}

sal_uInt16 ScDPSubtotalDlg::GetFuncMask() const
{
    sal_uInt16 nFuncMask = PIVOT_FUNC_NONE;

    if( maRbAuto.IsChecked() )
        nFuncMask = PIVOT_FUNC_AUTO;
    else if( maRbUser.IsChecked() )
        nFuncMask = maLbFunc.GetSelection();

    return nFuncMask;
}

void ScDPSubtotalDlg::FillLabelData( ScDPLabelData& rLabelData ) const
{
    rLabelData.mnFuncMask = GetFuncMask();
    rLabelData.mnUsedHier = maLabelData.mnUsedHier;
    rLabelData.mbShowAll = maCbShowAll.IsChecked();
    rLabelData.maMembers = maLabelData.maMembers;
    rLabelData.maSortInfo = maLabelData.maSortInfo;
    rLabelData.maLayoutInfo = maLabelData.maLayoutInfo;
    rLabelData.maShowInfo = maLabelData.maShowInfo;
}

void ScDPSubtotalDlg::Init( const ScDPLabelData& rLabelData, const ScPivotFuncData& rFuncData )
{
    // field name
    maFtName.SetText(rLabelData.getDisplayName());

    // radio buttons
    maRbNone.SetClickHdl( LINK( this, ScDPSubtotalDlg, RadioClickHdl ) );
    maRbAuto.SetClickHdl( LINK( this, ScDPSubtotalDlg, RadioClickHdl ) );
    maRbUser.SetClickHdl( LINK( this, ScDPSubtotalDlg, RadioClickHdl ) );

    RadioButton* pRBtn = 0;
    switch( rFuncData.mnFuncMask )
    {
        case PIVOT_FUNC_NONE:   pRBtn = &maRbNone;  break;
        case PIVOT_FUNC_AUTO:   pRBtn = &maRbAuto;  break;
        default:                pRBtn = &maRbUser;
    }
    pRBtn->Check();
    RadioClickHdl( pRBtn );

    // list box
    maLbFunc.SetSelection( rFuncData.mnFuncMask );
    maLbFunc.SetDoubleClickHdl( LINK( this, ScDPSubtotalDlg, DblClickHdl ) );

    // show all
    maCbShowAll.Check( rLabelData.mbShowAll );

    // options
    maBtnOptions.SetClickHdl( LINK( this, ScDPSubtotalDlg, ClickHdl ) );
}

// ----------------------------------------------------------------------------

IMPL_LINK( ScDPSubtotalDlg, RadioClickHdl, RadioButton*, pBtn )
{
    maLbFunc.Enable( pBtn == &maRbUser );
    return 0;
}

IMPL_LINK( ScDPSubtotalDlg, DblClickHdl, MultiListBox*, EMPTYARG )
{
    maBtnOk.Click();
    return 0;
}

IMPL_LINK( ScDPSubtotalDlg, ClickHdl, PushButton*, pBtn )
{
    if( pBtn == &maBtnOptions )
    {
        ScDPSubtotalOptDlg* pDlg = new ScDPSubtotalOptDlg( this, mrDPObj, maLabelData, mrDataFields, mbEnableLayout );
        if( pDlg->Execute() == RET_OK )
            pDlg->FillLabelData( maLabelData );
        delete pDlg;
    }
    return 0;
}

// ============================================================================

ScDPSubtotalOptDlg::ScDPSubtotalOptDlg( Window* pParent, ScDPObject& rDPObj,
        const ScDPLabelData& rLabelData, const ScDPNameVec& rDataFields,
        bool bEnableLayout ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_DPSUBTOTAL_OPT ) ),
    maFlSortBy      ( this, ScResId( FL_SORT_BY ) ),
    maLbSortBy      ( this, ScResId( LB_SORT_BY ) ),
    maRbSortAsc     ( this, ScResId( RB_SORT_ASC ) ),
    maRbSortDesc    ( this, ScResId( RB_SORT_DESC ) ),
    maRbSortMan     ( this, ScResId( RB_SORT_MAN ) ),
    maFlLayout      ( this, ScResId( FL_LAYOUT ) ),
    maFtLayout      ( this, ScResId( FT_LAYOUT ) ),
    maLbLayout      ( this, ScResId( LB_LAYOUT ) ),
    maCbLayoutEmpty ( this, ScResId( CB_LAYOUT_EMPTY ) ),
    maFlAutoShow    ( this, ScResId( FL_AUTOSHOW ) ),
    maCbShow        ( this, ScResId( CB_SHOW ) ),
    maNfShow        ( this, ScResId( NF_SHOW ) ),
    maFtShow        ( this, ScResId( FT_SHOW ) ),
    maFtShowFrom    ( this, ScResId( FT_SHOW_FROM ) ),
    maLbShowFrom    ( this, ScResId( LB_SHOW_FROM ) ),
    maFtShowUsing   ( this, ScResId( FT_SHOW_USING ) ),
    maLbShowUsing   ( this, ScResId( LB_SHOW_USING ) ),
    maFlHide        ( this, ScResId( FL_HIDE ) ),
    maLbHide        ( this, ScResId( CT_HIDE ) ),
    maFtHierarchy   ( this, ScResId( FT_HIERARCHY ) ),
    maLbHierarchy   ( this, ScResId( LB_HIERARCHY ) ),
    maBtnOk         ( this, ScResId( BTN_OK ) ),
    maBtnCancel     ( this, ScResId( BTN_CANCEL ) ),
    maBtnHelp       ( this, ScResId( BTN_HELP ) ),
    maLbLayoutWrp   ( maLbLayout, spLayoutMap ),
    maLbShowFromWrp ( maLbShowFrom, spShowFromMap ),
    mrDPObj         ( rDPObj ),
    maLabelData     ( rLabelData )
{
    FreeResource();
    Init( rDataFields, bEnableLayout );
}

void ScDPSubtotalOptDlg::FillLabelData( ScDPLabelData& rLabelData ) const
{
    // *** SORTING ***

    if( maRbSortMan.IsChecked() )
        rLabelData.maSortInfo.Mode = DataPilotFieldSortMode::MANUAL;
    else if( maLbSortBy.GetSelectEntryPos() == SC_SORTNAME_POS )
        rLabelData.maSortInfo.Mode = DataPilotFieldSortMode::NAME;
    else
        rLabelData.maSortInfo.Mode = DataPilotFieldSortMode::DATA;

    rLabelData.maSortInfo.Field = maLbSortBy.GetSelectEntry();
    rLabelData.maSortInfo.IsAscending = maRbSortAsc.IsChecked();

    // *** LAYOUT MODE ***

    rLabelData.maLayoutInfo.LayoutMode = maLbLayoutWrp.GetControlValue();
    rLabelData.maLayoutInfo.AddEmptyLines = maCbLayoutEmpty.IsChecked();

    // *** AUTO SHOW ***

    rLabelData.maShowInfo.IsEnabled = maCbShow.IsChecked();
    rLabelData.maShowInfo.ShowItemsMode = maLbShowFromWrp.GetControlValue();
    rLabelData.maShowInfo.ItemCount = sal::static_int_cast<sal_Int32>( maNfShow.GetValue() );
    rLabelData.maShowInfo.DataField = maLbShowUsing.GetSelectEntry();

    // *** HIDDEN ITEMS ***

    rLabelData.maMembers = maLabelData.maMembers;
    sal_uLong nVisCount = maLbHide.GetEntryCount();
    for( sal_uInt16 nPos = 0; nPos < nVisCount; ++nPos )
        rLabelData.maMembers[nPos].mbVisible = !maLbHide.IsChecked(nPos);

    // *** HIERARCHY ***

    rLabelData.mnUsedHier = maLbHierarchy.GetSelectEntryCount() ? maLbHierarchy.GetSelectEntryPos() : 0;
}

void ScDPSubtotalOptDlg::Init( const ScDPNameVec& rDataFields, bool bEnableLayout )
{
    // *** SORTING ***

    sal_Int32 nSortMode = maLabelData.maSortInfo.Mode;

    // sort fields list box
    maLbSortBy.InsertEntry(maLabelData.getDisplayName());

    for( ScDPNameVec::const_iterator aIt = rDataFields.begin(), aEnd = rDataFields.end(); aIt != aEnd; ++aIt )
    {
        maLbSortBy.InsertEntry( *aIt );
        maLbShowUsing.InsertEntry( *aIt );  // for AutoShow
    }
    if( maLbSortBy.GetEntryCount() > SC_SORTDATA_POS )
        maLbSortBy.SetSeparatorPos( SC_SORTDATA_POS - 1 );

    sal_uInt16 nSortPos = SC_SORTNAME_POS;
    if( nSortMode == DataPilotFieldSortMode::DATA )
    {
        nSortPos = lclFindListBoxEntry( maLbSortBy, maLabelData.maSortInfo.Field, SC_SORTDATA_POS );
        if( nSortPos >= maLbSortBy.GetEntryCount() )
        {
            nSortPos = SC_SORTNAME_POS;
            nSortMode = DataPilotFieldSortMode::MANUAL;
        }
    }
    maLbSortBy.SelectEntryPos( nSortPos );

    // sorting mode
    maRbSortAsc.SetClickHdl( LINK( this, ScDPSubtotalOptDlg, RadioClickHdl ) );
    maRbSortDesc.SetClickHdl( LINK( this, ScDPSubtotalOptDlg, RadioClickHdl ) );
    maRbSortMan.SetClickHdl( LINK( this, ScDPSubtotalOptDlg, RadioClickHdl ) );

    RadioButton* pRBtn = 0;
    switch( nSortMode )
    {
        case DataPilotFieldSortMode::NONE:
        case DataPilotFieldSortMode::MANUAL:
            pRBtn = &maRbSortMan;
        break;
        default:
            pRBtn = maLabelData.maSortInfo.IsAscending ? &maRbSortAsc : &maRbSortDesc;
    }
    pRBtn->Check();
    RadioClickHdl( pRBtn );

    // *** LAYOUT MODE ***

    maFlLayout.Enable( bEnableLayout );
    maFtLayout.Enable( bEnableLayout );
    maLbLayout.Enable( bEnableLayout );
    maCbLayoutEmpty.Enable( bEnableLayout );

    maLbLayoutWrp.SetControlValue( maLabelData.maLayoutInfo.LayoutMode );
    maCbLayoutEmpty.Check( maLabelData.maLayoutInfo.AddEmptyLines );

    // *** AUTO SHOW ***

    maCbShow.Check( maLabelData.maShowInfo.IsEnabled );
    maCbShow.SetClickHdl( LINK( this, ScDPSubtotalOptDlg, CheckHdl ) );

    maLbShowFromWrp.SetControlValue( maLabelData.maShowInfo.ShowItemsMode );
    long nCount = static_cast< long >( maLabelData.maShowInfo.ItemCount );
    if( nCount < 1 )
        nCount = SC_SHOW_DEFAULT;
    maNfShow.SetValue( nCount );

    // maLbShowUsing already filled above
    maLbShowUsing.SelectEntry( maLabelData.maShowInfo.DataField );
    if( maLbShowUsing.GetSelectEntryPos() >= maLbShowUsing.GetEntryCount() )
        maLbShowUsing.SelectEntryPos( 0 );

    CheckHdl( &maCbShow );      // enable/disable dependent controls

    // *** HIDDEN ITEMS ***

    maLbHide.SetHelpId( HID_SC_DPSUBT_HIDE );
    InitHideListBox();

    // *** HIERARCHY ***

    if( maLabelData.maHiers.getLength() > 1 )
    {
        lclFillListBox( maLbHierarchy, maLabelData.maHiers );
        sal_Int32 nHier = maLabelData.mnUsedHier;
        if( (nHier < 0) || (nHier >= maLabelData.maHiers.getLength()) ) nHier = 0;
        maLbHierarchy.SelectEntryPos( static_cast< sal_uInt16 >( nHier ) );
        maLbHierarchy.SetSelectHdl( LINK( this, ScDPSubtotalOptDlg, SelectHdl ) );
    }
    else
    {
        maFtHierarchy.Disable();
        maLbHierarchy.Disable();
    }
}

void ScDPSubtotalOptDlg::InitHideListBox()
{
    maLbHide.Clear();
    lclFillListBox( maLbHide, maLabelData.maMembers );
    size_t n = maLabelData.maMembers.size();
    for (size_t i = 0; i < n; ++i)
        maLbHide.CheckEntryPos(static_cast<sal_uInt16>(i), !maLabelData.maMembers[i].mbVisible);
    bool bEnable = maLbHide.GetEntryCount() > 0;
    maFlHide.Enable( bEnable );
    maLbHide.Enable( bEnable );
}

IMPL_LINK( ScDPSubtotalOptDlg, RadioClickHdl, RadioButton*, pBtn )
{
    maLbSortBy.Enable( pBtn != &maRbSortMan );
    return 0;
}

IMPL_LINK( ScDPSubtotalOptDlg, CheckHdl, CheckBox*, pCBox )
{
    if( pCBox == &maCbShow )
    {
        bool bEnable = maCbShow.IsChecked();
        maNfShow.Enable( bEnable );
        maFtShow.Enable( bEnable );
        maFtShowFrom.Enable( bEnable );
        maLbShowFrom.Enable( bEnable );

        bool bEnableUsing = bEnable && (maLbShowUsing.GetEntryCount() > 0);
        maFtShowUsing.Enable( bEnableUsing );
        maLbShowUsing.Enable( bEnableUsing );
    }
    return 0;
}

IMPL_LINK( ScDPSubtotalOptDlg, SelectHdl, ListBox*, pLBox )
{
    if( pLBox == &maLbHierarchy )
    {
        mrDPObj.GetMembers(maLabelData.mnCol, maLbHierarchy.GetSelectEntryPos(), maLabelData.maMembers);
        InitHideListBox();
    }
    return 0;
}

// ============================================================================

ScDPShowDetailDlg::ScDPShowDetailDlg( Window* pParent, ScDPObject& rDPObj, sal_uInt16 nOrient ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_DPSHOWDETAIL ) ),
    maFtDims        ( this, ScResId( FT_DIMS ) ),
    maLbDims        ( this, ScResId( LB_DIMS ) ),
    maBtnOk         ( this, ScResId( BTN_OK ) ),
    maBtnCancel     ( this, ScResId( BTN_CANCEL ) ),
    maBtnHelp       ( this, ScResId( BTN_HELP ) ),

    mrDPObj(rDPObj)
{
    FreeResource();

    ScDPSaveData* pSaveData = rDPObj.GetSaveData();
    long nDimCount = rDPObj.GetDimCount();
    for (long nDim=0; nDim<nDimCount; nDim++)
    {
        sal_Bool bIsDataLayout;
        sal_Int32 nDimFlags = 0;
        String aName = rDPObj.GetDimName( nDim, bIsDataLayout, &nDimFlags );
        if ( !bIsDataLayout && !rDPObj.IsDuplicated( nDim ) && ScDPObject::IsOrientationAllowed( nOrient, nDimFlags ) )
        {
            const ScDPSaveDimension* pDimension = pSaveData ? pSaveData->GetExistingDimensionByName(aName) : 0;
            if ( !pDimension || (pDimension->GetOrientation() != nOrient) )
            {
                if (pDimension)
                {
                    const OUString* pLayoutName = pDimension->GetLayoutName();
                    if (pLayoutName)
                        aName = *pLayoutName;
                }
                maLbDims.InsertEntry( aName );
                maNameIndexMap.insert(DimNameIndexMap::value_type(aName, nDim));
            }
        }
    }
    if( maLbDims.GetEntryCount() )
        maLbDims.SelectEntryPos( 0 );

    maLbDims.SetDoubleClickHdl( LINK( this, ScDPShowDetailDlg, DblClickHdl ) );
}

short ScDPShowDetailDlg::Execute()
{
    return maLbDims.GetEntryCount() ? ModalDialog::Execute() : RET_CANCEL;
}

String ScDPShowDetailDlg::GetDimensionName() const
{
    // Look up the internal dimension name which may be different from the
    // displayed field name.
    String aSelectedName = maLbDims.GetSelectEntry();
    DimNameIndexMap::const_iterator itr = maNameIndexMap.find(aSelectedName);
    if (itr == maNameIndexMap.end())
        // This should never happen!
        return aSelectedName;

    long nDim = itr->second;
    sal_Bool bIsDataLayout = false;
    return mrDPObj.GetDimName(nDim, bIsDataLayout);
}

IMPL_LINK( ScDPShowDetailDlg, DblClickHdl, ListBox*, pLBox )
{
    if( pLBox == &maLbDims )
        maBtnOk.Click();
    return 0;
}

// ============================================================================

