/*************************************************************************
 *
 *  $RCSfile: pvfundlg.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 12:58:39 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

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

// ============================================================================

using namespace ::com::sun::star::sheet;

using ::rtl::OUString;
using ::com::sun::star::uno::Sequence;

// ============================================================================

namespace {

/** Appends all strings from the Sequence to the list box.

    Empty strings are replaced by a localized "(empty)" entry and inserted at
    the specified position.

    @return  true = The passed string list contains an empty string entry.
 */
template< typename ListBoxType >
bool lclFillListBox( ListBoxType& rLBox, const Sequence< OUString >& rStrings, USHORT nEmptyPos = LISTBOX_APPEND )
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

/** Searches for a listbox entry, starts search at specified position. */
USHORT lclFindListBoxEntry( const ListBox& rLBox, const String& rEntry, USHORT nStartPos )
{
    USHORT nPos = nStartPos;
    while( (nPos < rLBox.GetEntryCount()) && (rLBox.GetEntry( nPos ) != rEntry) )
        ++nPos;
    return (nPos < rLBox.GetEntryCount()) ? nPos : LISTBOX_ENTRY_NOTFOUND;
}

/** This table represents the order of the strings in the resource string array. */
static const USHORT spnFunctions[] =
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

const USHORT SC_BASEITEM_PREV_POS = 0;
const USHORT SC_BASEITEM_NEXT_POS = 1;
const USHORT SC_BASEITEM_USER_POS = 2;

const USHORT SC_SORTNAME_POS = 0;
const USHORT SC_SORTDATA_POS = 1;

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

ScDPFunctionListBox::ScDPFunctionListBox( Window* pParent, WinBits nStyle ) :
    MultiListBox( pParent, nStyle )
{
    FillFunctionNames();
}

void ScDPFunctionListBox::SetSelection( USHORT nFuncMask )
{
    if( (nFuncMask == PIVOT_FUNC_NONE) || (nFuncMask == PIVOT_FUNC_AUTO) )
        SetNoSelection();
    else
        for( USHORT nEntry = 0, nCount = GetEntryCount(); nEntry < nCount; ++nEntry )
            SelectEntryPos( nEntry, (nFuncMask & spnFunctions[ nEntry ]) != 0 );
}

USHORT ScDPFunctionListBox::GetSelection() const
{
    USHORT nFuncMask = PIVOT_FUNC_NONE;
    for( USHORT nSel = 0, nCount = GetSelectEntryCount(); nSel < nCount; ++nSel )
        nFuncMask |= spnFunctions[ GetSelectEntryPos( nSel ) ];
    return nFuncMask;
}

void ScDPFunctionListBox::FillFunctionNames()
{
    DBG_ASSERT( !GetEntryCount(), "ScDPFunctionListBox::FillFunctionNames - do not add texts to resource" );
    Clear();
    ResStringArray aArr( ScResId( SCSTR_DPFUNCLISTBOX ) );
    for( USHORT nIndex = 0, nCount = aArr.Count(); nIndex < nCount; ++nIndex )
        InsertEntry( aArr.GetString( nIndex ) );
}

// ============================================================================

ScDPFunctionDlg::ScDPFunctionDlg(
        Window* pParent, const ScDPLabelDataVec& rLabelVec,
        const ScDPLabelData& rLabelData, const ScDPFuncData& rFuncData ) :
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

USHORT ScDPFunctionDlg::GetFuncMask() const
{
    return maLbFunc.GetSelection();
}

DataPilotFieldReference ScDPFunctionDlg::GetFieldRef() const
{
    DataPilotFieldReference aRef;

    aRef.ReferenceType = maLbTypeWrp.GetControlValue();
    aRef.ReferenceField = maLbBaseField.GetSelectEntry();

    USHORT nBaseItemPos = maLbBaseItem.GetSelectEntryPos();
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

void ScDPFunctionDlg::Init( const ScDPLabelData& rLabelData, const ScDPFuncData& rFuncData )
{
    // list box
    USHORT nFuncMask = (rFuncData.mnFuncMask == PIVOT_FUNC_NONE) ? PIVOT_FUNC_SUM : rFuncData.mnFuncMask;
    maLbFunc.SetSelection( nFuncMask );

    // field name
    maFtName.SetText( rLabelData.maName );

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
    for( ScDPLabelDataVec::const_iterator aIt = mrLabelVec.begin(), aEnd = mrLabelVec.end(); aIt != aEnd; ++aIt )
        maLbBaseField.InsertEntry( aIt->maName );

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
                USHORT nStartPos = mbEmptyItem ? (SC_BASEITEM_USER_POS + 1) : SC_BASEITEM_USER_POS;
                USHORT nPos = lclFindListBoxEntry( maLbBaseItem, rFuncData.maFieldRef.ReferenceItemName, nStartPos );
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
        USHORT nItemPos = (maLbBaseItem.GetEntryCount() > SC_BASEITEM_USER_POS) ? SC_BASEITEM_USER_POS : SC_BASEITEM_PREV_POS;
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
        const ScDPLabelData& rLabelData, const ScDPFuncData& rFuncData,
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

USHORT ScDPSubtotalDlg::GetFuncMask() const
{
    USHORT nFuncMask = PIVOT_FUNC_NONE;

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
    rLabelData.maVisible = maLabelData.maVisible;
    rLabelData.maSortInfo = maLabelData.maSortInfo;
    rLabelData.maLayoutInfo = maLabelData.maLayoutInfo;
    rLabelData.maShowInfo = maLabelData.maShowInfo;
}

void ScDPSubtotalDlg::Init( const ScDPLabelData& rLabelData, const ScDPFuncData& rFuncData )
{
    // field name
    maFtName.SetText( rLabelData.maName );

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
    rLabelData.maShowInfo.ItemCount = maNfShow.GetValue();
    rLabelData.maShowInfo.DataField = maLbShowUsing.GetSelectEntry();

    // *** HIDDEN ITEMS ***

    rLabelData.maMembers = maLabelData.maMembers;
    ULONG nVisCount = maLbHide.GetEntryCount();
    rLabelData.maVisible.realloc( nVisCount );
    for( USHORT nPos = 0; nPos < nVisCount; ++nPos )
        rLabelData.maVisible[ nPos ] = !maLbHide.IsChecked( nPos );

    // *** HIERARCHY ***

    rLabelData.mnUsedHier = maLbHierarchy.GetSelectEntryCount() ? maLbHierarchy.GetSelectEntryPos() : 0;
}

void ScDPSubtotalOptDlg::Init( const ScDPNameVec& rDataFields, bool bEnableLayout )
{
    // *** SORTING ***

    sal_Int32 nSortMode = maLabelData.maSortInfo.Mode;

    // sort fields list box
    maLbSortBy.InsertEntry( maLabelData.maName );
    for( ScDPNameVec::const_iterator aIt = rDataFields.begin(), aEnd = rDataFields.end(); aIt != aEnd; ++aIt )
    {
        maLbSortBy.InsertEntry( *aIt );
        maLbShowUsing.InsertEntry( *aIt );  // for AutoShow
    }
    if( maLbSortBy.GetEntryCount() > SC_SORTDATA_POS )
        maLbSortBy.SetSeparatorPos( SC_SORTDATA_POS - 1 );

    USHORT nSortPos = SC_SORTNAME_POS;
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
        maLbHierarchy.SelectEntryPos( static_cast< USHORT >( nHier ) );
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
    for( sal_Int32 nVisIdx = 0, nVisSize = maLabelData.maVisible.getLength(); nVisIdx < nVisSize; ++nVisIdx )
        maLbHide.CheckEntryPos( static_cast< USHORT >( nVisIdx ), !maLabelData.maVisible[ nVisIdx ] );
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
        mrDPObj.GetMembers( maLabelData.mnCol, maLbHierarchy.GetSelectEntryPos(),
            maLabelData.maMembers, &maLabelData.maVisible );
        InitHideListBox();
    }
    return 0;
}

// ============================================================================

ScDPShowDetailDlg::ScDPShowDetailDlg( Window* pParent, ScDPObject& rDPObj, USHORT nOrient ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_DPSHOWDETAIL ) ),
    maFtDims        ( this, ScResId( FT_DIMS ) ),
    maLbDims        ( this, ScResId( LB_DIMS ) ),
    maBtnOk         ( this, ScResId( BTN_OK ) ),
    maBtnCancel     ( this, ScResId( BTN_CANCEL ) ),
    maBtnHelp       ( this, ScResId( BTN_HELP ) )
{
    FreeResource();

    ScDPSaveData* pSaveData = rDPObj.GetSaveData();
    long nDimCount = rDPObj.GetDimCount();
    for (long nDim=0; nDim<nDimCount; nDim++)
    {
        BOOL bIsDataLayout;
        String aName = rDPObj.GetDimName( nDim, bIsDataLayout );
        if ( !bIsDataLayout && !rDPObj.IsDuplicated( nDim ) )
        {
            const ScDPSaveDimension* pDimension = pSaveData ? pSaveData->GetExistingDimensionByName(aName) : 0;
            if ( !pDimension || (pDimension->GetOrientation() != nOrient) )
                maLbDims.InsertEntry( aName );
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
    return maLbDims.GetSelectEntry();
}

IMPL_LINK( ScDPShowDetailDlg, DblClickHdl, ListBox*, pLBox )
{
    if( pLBox == &maLbDims )
        maBtnOk.Click();
    return 0;
}

// ============================================================================

