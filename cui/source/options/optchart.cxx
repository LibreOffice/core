/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cui.hxx"
#include <unotools/pathoptions.hxx>
#include <cuires.hrc>
#include "optchart.hxx"
#include "optchart.hrc"
#include <dialmgr.hxx>
#include <svx/svxids.hrc> // for SID_SCH_EDITOPTIONS

// ====================
// class ChartColorLB
// ====================
void ChartColorLB::FillBox( const SvxChartColorTable & rTab )
{
    long nCount = rTab.size();
    SetUpdateMode( sal_False );

    for( long i = 0; i < nCount; i++ )
    {
        Append( const_cast< XColorEntry * >( & rTab[ i ] ));
    }
    SetUpdateMode( sal_True );
}


// ====================
// class SvxDefaultColorOptPage
// ====================
SvxDefaultColorOptPage::SvxDefaultColorOptPage( Window* pParent, const SfxItemSet& rInAttrs ) :

    SfxTabPage( pParent, CUI_RES( RID_OPTPAGE_CHART_DEFCOLORS ), rInAttrs ),

    aGbChartColors  ( this, CUI_RES( FL_CHART_COLOR_LIST ) ),
    aLbChartColors  ( this, CUI_RES( LB_CHART_COLOR_LIST ) ),
    aGbColorBox     ( this, CUI_RES( FL_COLOR_BOX ) ),
    aValSetColorBox ( this, CUI_RES( CT_COLOR_BOX ) ),
    aPBDefault      ( this, CUI_RES( PB_RESET_TO_DEFAULT ) )
{
    FreeResource();

    aPBDefault.SetClickHdl( LINK( this, SvxDefaultColorOptPage, ResetToDefaults ) );
    aLbChartColors.SetSelectHdl( LINK( this, SvxDefaultColorOptPage, ListClickedHdl ) );
    aValSetColorBox.SetSelectHdl( LINK( this, SvxDefaultColorOptPage, BoxClickedHdl ) );

    aValSetColorBox.SetStyle( aValSetColorBox.GetStyle()
                                    | WB_VSCROLL | WB_ITEMBORDER | WB_NAMEFIELD );
    aValSetColorBox.SetColCount( 8 );
    aValSetColorBox.SetLineCount( 12 );
    aValSetColorBox.SetExtraSpacing( 0 );
    aValSetColorBox.Show();

    pChartOptions = new SvxChartOptions;
    pColorTab = new XColorList( SvtPathOptions().GetPalettePath() );

    const SfxPoolItem* pItem = NULL;
    if ( rInAttrs.GetItemState( SID_SCH_EDITOPTIONS, sal_False, &pItem ) == SFX_ITEM_SET )
    {
        pColorConfig = SAL_STATIC_CAST( SvxChartColorTableItem*, pItem->Clone() );
    }
    else
    {
        SvxChartColorTable aTable;
        aTable.useDefault();
        pColorConfig = new SvxChartColorTableItem( SID_SCH_EDITOPTIONS, aTable );
        pColorConfig->SetOptions( pChartOptions );
    }

    Construct();
}

SvxDefaultColorOptPage::~SvxDefaultColorOptPage()
{
    // save changes
    pChartOptions->SetDefaultColors( pColorConfig->GetColorTable() );
    pChartOptions->Commit();

    delete pColorConfig;
    delete pColorTab;
    delete pChartOptions;
}

void SvxDefaultColorOptPage::Construct()
{
    if( pColorConfig )
        aLbChartColors.FillBox( pColorConfig->GetColorTable() );

    FillColorBox();

    aLbChartColors.SelectEntryPos( 0 );
    ListClickedHdl( &aLbChartColors );
}


SfxTabPage* __EXPORT SvxDefaultColorOptPage::Create( Window* pParent, const SfxItemSet& rAttrs )
{
    return new SvxDefaultColorOptPage( pParent, rAttrs );
}

sal_Bool __EXPORT SvxDefaultColorOptPage::FillItemSet( SfxItemSet& rOutAttrs )
{
    if( pColorConfig )
        rOutAttrs.Put( *SAL_STATIC_CAST( SfxPoolItem*, pColorConfig ));

    return sal_True;
}

void __EXPORT SvxDefaultColorOptPage::Reset( const SfxItemSet& )
{
    aLbChartColors.SelectEntryPos( 0 );
    ListClickedHdl( &aLbChartColors );
}

void SvxDefaultColorOptPage::FillColorBox()
{
    if( !pColorTab ) return;

    long nCount = pColorTab->Count();
    XColorEntry* pColorEntry;

    for( long i = 0; i < nCount; i++ )
    {
        pColorEntry = pColorTab->GetColor( i );
        aValSetColorBox.InsertItem( (sal_uInt16) i + 1, pColorEntry->GetColor(), pColorEntry->GetName() );
    }
}


long SvxDefaultColorOptPage::GetColorIndex( const Color& rCol )
{
    if( pColorTab )
    {
        long nCount = pColorTab->Count();
        XColorEntry* pColorEntry;

        for( long i = nCount - 1; i >= 0; i-- )         // default chart colors are at the end of the table
        {
            pColorEntry = pColorTab->GetColor( i );
            if( pColorEntry && pColorEntry->GetColor() == rCol )
                return SAL_STATIC_CAST( XPropertyList*, pColorTab )->Get( pColorEntry->GetName() );
        }
    }
    return -1L;
}



// --------------------
// event handlers
// --------------------

// ResetToDefaults
// ---------------

IMPL_LINK( SvxDefaultColorOptPage, ResetToDefaults, void *, EMPTYARG )
{
    if( pColorConfig )
    {
        pColorConfig->GetColorTable().useDefault();

        aLbChartColors.Clear();
        aLbChartColors.FillBox( pColorConfig->GetColorTable() );

        aLbChartColors.GetFocus();
    }

    return 0L;
}

// ListClickedHdl
// --------------

IMPL_LINK( SvxDefaultColorOptPage, ListClickedHdl, ChartColorLB*,  pColorList )
{
    Color aCol = pColorList->GetSelectEntryColor();

    long nIndex = GetColorIndex( aCol );

    if( nIndex == -1 )      // not found
    {
        aValSetColorBox.SetNoSelection();
    }
    else
    {
        aValSetColorBox.SelectItem( (sal_uInt16)nIndex + 1 );       // ValueSet is 1-based
    }

    return 0L;
}

// BoxClickedHdl
// -------------

IMPL_LINK( SvxDefaultColorOptPage, BoxClickedHdl, ValueSet*, EMPTYARG )
{
    sal_uInt16 nIdx = aLbChartColors.GetSelectEntryPos();
    if( nIdx != LISTBOX_ENTRY_NOTFOUND )
    {
        XColorEntry aEntry( aValSetColorBox.GetItemColor( aValSetColorBox.GetSelectItemId() ),
                            aLbChartColors.GetSelectEntry() );

        aLbChartColors.Modify( & aEntry, nIdx );
        pColorConfig->ReplaceColorByIndex( nIdx, aEntry );

        aLbChartColors.SelectEntryPos( nIdx );  // reselect entry
    }

    return 0L;
}

