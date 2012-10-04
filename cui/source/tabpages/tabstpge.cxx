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

#include <sfx2/app.hxx>
#include <tools/shl.hxx>
#include <svtools/ruler.hxx>
#include <svx/dialogs.hrc>

#define _SVX_TABSTPGE_CXX

#include <cuires.hrc>
#include "tabstpge.hrc"
#include <editeng/lrspitem.hxx>
#include "tabstpge.hxx"
#include <dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <sfx2/module.hxx>
#include <svl/cjkoptions.hxx>
#include <unotools/localedatawrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/request.hxx>
#include <svl/intitem.hxx>

// class TabWin_Impl -----------------------------------------------------

class TabWin_Impl : public Window
{
private:
    sal_uInt16  nTabStyle;

public:
    TabWin_Impl( Window* pParent, const ResId& rId, sal_uInt16 nStyle ) :
        Window( pParent, rId ),
        nTabStyle( nStyle ) {}

    virtual void    Paint( const Rectangle& rRect );
};

// static ----------------------------------------------------------------

static sal_uInt16 pRanges[] =
{
    SID_ATTR_TABSTOP,
    SID_ATTR_TABSTOP_OFFSET,
    0
};

// C function ------------------------------------------------------------

void FillUpWithDefTabs_Impl( long nDefDist, SvxTabStopItem& rTabs )
{
    if( rTabs.Count() )
        return;
    {
        SvxTabStop aSwTabStop( nDefDist, SVX_TAB_ADJUST_DEFAULT );
        rTabs.Insert( aSwTabStop );
    }
}

// class TabWin_Impl -----------------------------------------------------

void TabWin_Impl::Paint( const Rectangle& )
{
    // Paint tabulators
    Point aPnt;
    Size aSize = GetOutputSizePixel();
    aPnt.X() = aSize.Width() / 2;
    aPnt.Y() = aSize.Height() / 2;
    Ruler::DrawTab( this, aPnt, nTabStyle );
}

// class SvxTabulatorTabPage ---------------------------------------------

SvxTabulatorTabPage::SvxTabulatorTabPage( Window* pParent,
                                          const SfxItemSet& rAttr ):

    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_TABULATOR ), rAttr ),

    aTabLabel       ( this, CUI_RES( FL_TABPOS ) ),
    aTabBox         ( this, CUI_RES( ED_TABPOS ) ),
    aTabLabelVert   ( this, CUI_RES( FL_TABPOS_VERT ) ),
    aTabTypeLabel   ( this, CUI_RES( FL_TABTYPE ) ),
    aLeftTab        ( this, CUI_RES( BTN_TABTYPE_LEFT ) ),
    aRightTab       ( this, CUI_RES( BTN_TABTYPE_RIGHT ) ),
    aCenterTab      ( this, CUI_RES( BTN_TABTYPE_CENTER ) ),
    aDezTab         ( this, CUI_RES( BTN_TABTYPE_DECIMAL ) ),
    pLeftWin        ( new TabWin_Impl( this, CUI_RES( WIN_TABLEFT ), (sal_uInt16)(RULER_TAB_LEFT|WB_HORZ) ) ),
    pRightWin       ( new TabWin_Impl( this, CUI_RES( WIN_TABRIGHT ), (sal_uInt16)(RULER_TAB_RIGHT|WB_HORZ) ) ),
    pCenterWin      ( new TabWin_Impl( this, CUI_RES( WIN_TABCENTER ), (sal_uInt16)(RULER_TAB_CENTER|WB_HORZ) ) ),
    pDezWin         ( new TabWin_Impl( this, CUI_RES( WIN_TABDECIMAL ), (sal_uInt16)(RULER_TAB_DECIMAL|WB_HORZ) ) ),
    aDezCharLabel   ( this, CUI_RES( FT_TABTYPE_DECCHAR ) ),
    aDezChar        ( this, CUI_RES( ED_TABTYPE_DECCHAR ) ),
    aFillLabel      ( this, CUI_RES( FL_FILLCHAR ) ),
    aNoFillChar     ( this, CUI_RES( BTN_FILLCHAR_NO ) ),
    aFillPoints     ( this, CUI_RES( BTN_FILLCHAR_POINTS ) ),
    aFillDashLine   ( this, CUI_RES( BTN_FILLCHAR_DASHLINE ) ),
    aFillSolidLine  ( this, CUI_RES( BTN_FILLCHAR_UNDERSCORE ) ),
    aFillSpecial    ( this, CUI_RES( BTN_FILLCHAR_OTHER ) ),
    aFillChar       ( this, CUI_RES( ED_FILLCHAR_OTHER ) ),
    aNewBtn         ( this, CUI_RES( BTN_NEW ) ),
    aDelAllBtn      ( this, CUI_RES( BTN_DELALL ) ),
    aDelBtn         ( this, CUI_RES( BTN_DEL ) ),

    aAktTab     ( 0 ),
    aNewTabs    ( 0, 0, SVX_TAB_ADJUST_LEFT, GetWhich( SID_ATTR_TABSTOP ) ),
    nDefDist    ( 0 ),
    eDefUnit( FUNIT_100TH_MM ),
    bCheck      ( sal_False )

{
    SvtCJKOptions aCJKOptions;
    if(aCJKOptions.IsAsianTypographyEnabled())
    {
        aLeftTab  .SetText(String(CUI_RES(   ST_LEFTTAB_ASIAN )));
        aRightTab .SetText(String(CUI_RES(   ST_RIGHTTAB_ASIAN )));
    }

    aFillChar.SetAccessibleName(String(CUI_RES(   ST_FILLCHAR_OTHER )));
    // This page needs ExchangeSupport
    SetExchangeSupport();


    // Set metric
    FieldUnit eFUnit = GetModuleFieldUnit( rAttr );
    SetFieldUnit( aTabBox, eFUnit );

    // Initialize buttons
    aNewBtn.SetClickHdl( LINK( this,SvxTabulatorTabPage, NewHdl_Impl ) );
    aDelBtn.SetClickHdl( LINK( this,SvxTabulatorTabPage, DelHdl_Impl ) );
    aDelAllBtn.SetClickHdl( LINK( this,SvxTabulatorTabPage, DelAllHdl_Impl ) );

    Link aLink = LINK( this, SvxTabulatorTabPage, TabTypeCheckHdl_Impl );
    aLeftTab.SetClickHdl( aLink );
    aRightTab.SetClickHdl( aLink );
    aDezTab.SetClickHdl( aLink );
    aCenterTab.SetClickHdl( aLink );

    aDezChar.SetLoseFocusHdl( LINK( this,  SvxTabulatorTabPage, GetDezCharHdl_Impl ) );
    aDezChar.SetMaxTextLen(1);
    aDezChar.Disable();
    aDezCharLabel.Disable();

    aLink = LINK( this, SvxTabulatorTabPage, FillTypeCheckHdl_Impl );
    aNoFillChar.SetClickHdl( aLink );
    aFillPoints.SetClickHdl( aLink );
    aFillDashLine.SetClickHdl( aLink );
    aFillSolidLine.SetClickHdl( aLink );
    aFillSpecial.SetClickHdl( aLink );
    aFillChar.SetLoseFocusHdl( LINK( this,  SvxTabulatorTabPage, GetFillCharHdl_Impl ) );
    aFillChar.SetMaxTextLen(1);
    aFillChar.Disable();

    aTabBox.SetDoubleClickHdl( LINK( this, SvxTabulatorTabPage, SelectHdl_Impl ) );
    aTabBox.SetModifyHdl( LINK( this, SvxTabulatorTabPage, ModifyHdl_Impl ) );

    aLeftTab.SetAccessibleRelationMemberOf( &aTabTypeLabel );
    aRightTab.SetAccessibleRelationMemberOf( &aTabTypeLabel );
    aCenterTab.SetAccessibleRelationMemberOf( &aTabTypeLabel );
    aDezTab.SetAccessibleRelationMemberOf( &aTabTypeLabel );
    aDezCharLabel.SetAccessibleRelationMemberOf( &aTabTypeLabel );
    aDezChar.SetAccessibleRelationMemberOf( &aTabTypeLabel );

    aNoFillChar.SetAccessibleRelationMemberOf( &aFillLabel );
    aFillPoints.SetAccessibleRelationMemberOf( &aFillLabel );
    aFillDashLine.SetAccessibleRelationMemberOf( &aFillLabel );
    aFillSolidLine.SetAccessibleRelationMemberOf( &aFillLabel );
    aFillSpecial.SetAccessibleRelationMemberOf( &aFillLabel );
    aFillChar.SetAccessibleRelationMemberOf( &aFillLabel );

    aFillChar.SetAccessibleRelationLabeledBy(&aFillSpecial);

    // Get the default decimal char from the system
    LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
    aAktTab.GetDecimal() = aLocaleWrapper.getNumDecimalSep()[0];
    FreeResource();
}

// -----------------------------------------------------------------------

SvxTabulatorTabPage::~SvxTabulatorTabPage()
{
    delete pLeftWin;
    delete pRightWin;
    delete pCenterWin;
    delete pDezWin;
}

// -----------------------------------------------------------------------

sal_uInt16* SvxTabulatorTabPage::GetRanges()
{
    return pRanges;
}

// -----------------------------------------------------------------------

sal_Bool SvxTabulatorTabPage::FillItemSet( SfxItemSet& rSet )
{
    sal_Bool bModified = sal_False;

    // Put the controls' values in here
    if ( aNewBtn.IsEnabled() )
        NewHdl_Impl( 0 );

    // Call the LoseFocus-Handler first
    GetDezCharHdl_Impl( &aDezChar );
    GetFillCharHdl_Impl( &aFillChar );

    FillUpWithDefTabs_Impl( nDefDist, aNewTabs );
    SfxItemPool* pPool = rSet.GetPool();
    MapUnit eUnit = (MapUnit)pPool->GetMetric( GetWhich( SID_ATTR_TABSTOP ) );
    const SfxPoolItem* pOld = GetOldItem( rSet, SID_ATTR_TABSTOP );

    if ( MAP_100TH_MM != eUnit )
    {
        // If the ItemSet contains a LRSpaceItem with negative first line indent,
        // the TabStopItem needs to have a DefTab at position 0.
        const SfxPoolItem* pLRSpace;
        // If not in the new set, then maybe in the old one
        if ( SFX_ITEM_SET !=
             rSet.GetItemState( GetWhich( SID_ATTR_LRSPACE ), sal_True, &pLRSpace ) )
            pLRSpace = GetOldItem( rSet, SID_ATTR_LRSPACE );

        if ( pLRSpace && ( (SvxLRSpaceItem*)pLRSpace )->GetTxtFirstLineOfst() < 0 )
        {
            SvxTabStop aNull( 0, SVX_TAB_ADJUST_DEFAULT );
            aNewTabs.Insert( aNull );
        }

        SvxTabStopItem aTmp( aNewTabs );
        aTmp.Remove( 0, aTmp.Count() );

        for ( sal_uInt16 i = 0; i < aNewTabs.Count(); ++i )
        {
            SvxTabStop aTmpStop = aNewTabs[i];
            aTmpStop.GetTabPos() =
                LogicToLogic( aTmpStop.GetTabPos(), MAP_100TH_MM, eUnit );
            aTmp.Insert( aTmpStop );
        }

        if ( !pOld || !( *( (SvxTabStopItem*)pOld ) == aTmp ) )
        {
            rSet.Put( aTmp );
            bModified = sal_True;
        }
    }
    else if ( !pOld || !( *( (SvxTabStopItem*)pOld ) == aNewTabs ) )
    {
        rSet.Put( aNewTabs );
        bModified = sal_True;
    }
    return bModified;
}

// -----------------------------------------------------------------------

SfxTabPage* SvxTabulatorTabPage::Create( Window* pParent,
                                         const SfxItemSet& rSet)
{
    return ( new SvxTabulatorTabPage( pParent, rSet ) );
}

// -----------------------------------------------------------------------

void SvxTabulatorTabPage::Reset( const SfxItemSet& rSet )
{
    SfxItemPool* pPool = rSet.GetPool();
    MapUnit eUnit = (MapUnit)pPool->GetMetric( GetWhich( SID_ATTR_TABSTOP ) );

    // Current tabs
    const SfxPoolItem* pItem = GetItem( rSet, SID_ATTR_TABSTOP );

    if ( pItem )
    {
        if ( MAP_100TH_MM != eUnit )
        {
            SvxTabStopItem aTmp( *( (const SvxTabStopItem*)pItem ) );
            aNewTabs.Remove( 0, aNewTabs.Count() );

            for ( sal_uInt16 i = 0; i < aTmp.Count(); ++i )
            {
                SvxTabStop aTmpStop = aTmp[i];
                aTmpStop.GetTabPos() =
                    LogicToLogic( aTmpStop.GetTabPos(), eUnit, MAP_100TH_MM );
                aNewTabs.Insert( aTmpStop );
            }
        }
        else
            aNewTabs = *( (const SvxTabStopItem*)pItem );
    }
    else
        aNewTabs.Remove( 0, aNewTabs.Count() );

    // Defaul tab distance
    nDefDist = SVX_TAB_DEFDIST;
    pItem = GetItem( rSet, SID_ATTR_TABSTOP_DEFAULTS );

    if ( pItem )
        nDefDist = LogicToLogic(
            (long)((const SfxUInt16Item*)pItem)->GetValue(), eUnit, MAP_100TH_MM );

    // Tab pos currently selected
    sal_uInt16 nTabPos = 0;
    pItem = GetItem( rSet, SID_ATTR_TABSTOP_POS );

    if ( pItem )
        nTabPos = ( (const SfxUInt16Item*)pItem )->GetValue();

    InitTabPos_Impl( nTabPos );
}

// -----------------------------------------------------------------------

void SvxTabulatorTabPage::DisableControls( const sal_uInt16 nFlag )
{
    if ( ( TABTYPE_LEFT & nFlag ) == TABTYPE_LEFT )
    {
        aLeftTab.Disable();
        pLeftWin->Disable();
    }
    if ( ( TABTYPE_RIGHT & nFlag ) == TABTYPE_RIGHT )
    {
        aRightTab.Disable();
        pRightWin->Disable();
    }
    if ( ( TABTYPE_CENTER & nFlag ) == TABTYPE_CENTER )
    {
        aCenterTab.Disable();
        pCenterWin->Disable();
    }
    if ( ( TABTYPE_DEZIMAL & nFlag ) == TABTYPE_DEZIMAL )
    {
        aDezTab.Disable();
        pDezWin->Disable();
        aDezCharLabel.Disable();
        aDezChar.Disable();
    }
    if ( ( TABTYPE_ALL & nFlag ) == TABTYPE_ALL )
        aTabTypeLabel.Disable();
    if ( ( TABFILL_NONE & nFlag ) == TABFILL_NONE )
        aNoFillChar.Disable();
    if ( ( TABFILL_POINT & nFlag ) == TABFILL_POINT )
        aFillPoints.Disable();
    if ( ( TABFILL_DASHLINE & nFlag ) == TABFILL_DASHLINE )
        aFillDashLine.Disable();
    if ( ( TABFILL_SOLIDLINE & nFlag ) == TABFILL_SOLIDLINE )
        aFillSolidLine.Disable();
    if ( ( TABFILL_SPECIAL & nFlag ) == TABFILL_SPECIAL )
    {
        aFillSpecial.Disable();
        aFillChar.Disable();
    }
    if ( ( TABFILL_ALL & nFlag ) == TABFILL_ALL )
        aFillLabel.Disable();
}

// -----------------------------------------------------------------------

int SvxTabulatorTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( *_pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

void SvxTabulatorTabPage::InitTabPos_Impl( sal_uInt16 nTabPos )
{
    aTabBox.Clear();

    long nOffset = 0;
    const SfxPoolItem* pItem = 0;
    if ( GetItemSet().GetItemState( SID_ATTR_TABSTOP_OFFSET, sal_True, &pItem )
            == SFX_ITEM_SET )
    {
        nOffset = ( (const SfxInt32Item*)pItem )->GetValue();
        MapUnit eUnit = (MapUnit)GetItemSet().GetPool()->GetMetric( GetWhich( SID_ATTR_TABSTOP ) );
        nOffset = OutputDevice::LogicToLogic( nOffset, eUnit, MAP_100TH_MM  );
    }

    // Correct current TabPos and default tabs
    for ( sal_uInt16 i = 0; i < aNewTabs.Count(); i++ )
    {
        if ( aNewTabs[i].GetAdjustment() != SVX_TAB_ADJUST_DEFAULT )
        {
            aTabBox.InsertValue( aTabBox.Normalize(
                aNewTabs[i].GetTabPos() + nOffset ), eDefUnit );
        }
        else
            aNewTabs.Remove( i-- );
    }

    // Select current tabulator
    const sal_uInt16 nSize = aNewTabs.Count();

    if ( nTabPos >= nSize )
        nTabPos = 0;

    // Switch off all RadioButtons for a start
    aLeftTab.Check( sal_True );
    aNoFillChar.Check( sal_True );

    if( aTabBox.GetEntryCount() > 0 )
    {
        aTabBox.SetText( aTabBox.GetEntry( nTabPos ) );
        aAktTab = aNewTabs[nTabPos];

        SetFillAndTabType_Impl();
        aNewBtn.Disable();
        aDelBtn.Enable();
    }
    else
    {   // If no entry, 0 is the default value
        aTabBox.SetValue( 0, eDefUnit );

        aNewBtn.Enable();
        aDelBtn.Disable();
    }
}

// -----------------------------------------------------------------------

void SvxTabulatorTabPage::SetFillAndTabType_Impl()
{
    RadioButton* pTypeBtn = 0;
    RadioButton* pFillBtn = 0;

    aDezChar.Disable();
    aDezCharLabel.Disable();

    if ( aAktTab.GetAdjustment() == SVX_TAB_ADJUST_LEFT )
        pTypeBtn = &aLeftTab;
    else if ( aAktTab.GetAdjustment() == SVX_TAB_ADJUST_RIGHT )
        pTypeBtn = &aRightTab;
    else if ( aAktTab.GetAdjustment() == SVX_TAB_ADJUST_DECIMAL )
    {
        pTypeBtn = &aDezTab;
        aDezChar.Enable();
        aDezCharLabel.Enable();
        aDezChar.SetText( rtl::OUString( (sal_Unicode)aAktTab.GetDecimal() ) );
    }
    else if ( aAktTab.GetAdjustment() == SVX_TAB_ADJUST_CENTER )
        pTypeBtn = &aCenterTab;

    if ( pTypeBtn )
        pTypeBtn->Check();

    aFillChar.Disable();
    aFillChar.SetText( String() );

    if ( aAktTab.GetFill() == ' ' )
        pFillBtn = &aNoFillChar;
    else if ( aAktTab.GetFill() == '-' )
        pFillBtn = &aFillDashLine;
    else if ( aAktTab.GetFill() == '_' )
        pFillBtn = &aFillSolidLine;
    else if ( aAktTab.GetFill() == '.' )
        pFillBtn = &aFillPoints;
    else
    {
        pFillBtn = &aFillSpecial;
        aFillChar.Enable();
        aFillChar.SetText( rtl::OUString( (sal_Unicode)aAktTab.GetFill() ) );
    }
    pFillBtn->Check();
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxTabulatorTabPage, NewHdl_Impl, Button *, pBtn )
{
    // Add a new one and select it
    // Get the value from the display
    long nVal = static_cast<long>(aTabBox.Denormalize( aTabBox.GetValue( eDefUnit ) ));

    // If the pBtn == 0 && the value == 0 then do not create a tab, because we create via OK
    if ( nVal == 0 && pBtn == 0 )
        return 0;

    long nOffset = 0;
    const SfxPoolItem* pItem = 0;

    if ( GetItemSet().GetItemState( SID_ATTR_TABSTOP_OFFSET, sal_True, &pItem ) ==
         SFX_ITEM_SET )
    {
        nOffset = ( (const SfxInt32Item*)pItem )->GetValue();
        MapUnit eUnit = (MapUnit)GetItemSet().GetPool()->GetMetric( GetWhich( SID_ATTR_TABSTOP ) );
        nOffset = OutputDevice::LogicToLogic( nOffset, eUnit, MAP_100TH_MM  );
    }
    const long nReal = nVal - nOffset;
    sal_uInt16 nSize = aTabBox.GetEntryCount();

    sal_uInt16 i;
    for( i = 0; i < nSize; i++ )
    {
        if ( nReal < aNewTabs[i].GetTabPos() )
            break;
    }

    // Make ListBox entry
    aTabBox.InsertValue( aTabBox.Normalize( nVal ), eDefUnit, i );
    aAktTab.GetTabPos() = nReal;
    SvxTabAdjust eAdj = SVX_TAB_ADJUST_LEFT;

    if ( aRightTab.IsChecked() )
        eAdj = SVX_TAB_ADJUST_RIGHT;
    else if ( aCenterTab.IsChecked() )
        eAdj = SVX_TAB_ADJUST_CENTER;
    else if ( aDezTab.IsChecked() )
        eAdj = SVX_TAB_ADJUST_DECIMAL;

    aAktTab.GetAdjustment() = eAdj;
    aNewTabs.Insert( aAktTab );

    aNewBtn.Disable();
    aDelBtn.Enable();
    aTabBox.GrabFocus();

    // If no RadioButton was clicked, we need to put anyway
    bCheck |= sal_True;
    // Set the selection into the position Edit
    aTabBox.SetSelection(Selection(0, aTabBox.GetText().Len()));
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxTabulatorTabPage, DelHdl_Impl)
{
    sal_uInt16 nPos = aTabBox.GetValuePos( aTabBox.GetValue() );

    if ( nPos == COMBOBOX_ENTRY_NOTFOUND )
        return 0;

    if ( aTabBox.GetEntryCount() == 1 )
    {
        DelAllHdl_Impl( 0 );
        return 0;
    }

    // Delete Tab
    aTabBox.RemoveEntry( nPos );
    aNewTabs.Remove( nPos );

    // Reset aAktTab
    const sal_uInt16 nSize = aNewTabs.Count();

    if ( nSize > 0 )
    {
        // Correct Pos
        nPos = ( ( nSize - 1 ) >= nPos) ? nPos : nPos - 1;
        aTabBox.SetValue( aTabBox.GetValue( nPos ) );
        aAktTab = aNewTabs[nPos];
    }

    // If no Tabs Enable Disable Controls
    if ( aTabBox.GetEntryCount() == 0 )
    {
        aDelBtn.Disable();
        aNewBtn.Enable();
        aTabBox.GrabFocus();
    }

    // If no RadioButton was clicked, we need to put anyway
    bCheck |= sal_True;
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxTabulatorTabPage, DelAllHdl_Impl)
{
    if ( aNewTabs.Count() )
    {
        aNewTabs = SvxTabStopItem( 0 );
        InitTabPos_Impl();

        // So that we put in FillItemSet()
        bCheck |= sal_True;
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxTabulatorTabPage, TabTypeCheckHdl_Impl, RadioButton *, pBox )
{
    bCheck |= sal_True;
    SvxTabAdjust eAdj;
    aDezChar.Disable();
    aDezCharLabel.Disable();
    aDezChar.SetText( String() );

    if ( pBox == &aLeftTab )
        eAdj = SVX_TAB_ADJUST_LEFT;
    else if ( pBox == &aRightTab )
        eAdj = SVX_TAB_ADJUST_RIGHT;
    else if ( pBox == &aCenterTab )
        eAdj = SVX_TAB_ADJUST_CENTER;
    else
    {
        eAdj = SVX_TAB_ADJUST_DECIMAL;
        aDezChar.Enable();
        aDezCharLabel.Enable();
        aDezChar.SetText( rtl::OUString( (sal_Unicode)aAktTab.GetDecimal() ) );
    }

    aAktTab.GetAdjustment() = eAdj;
    sal_uInt16 nPos = aTabBox.GetValuePos( aTabBox.GetValue( eDefUnit ), eDefUnit );

    if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
    {
        aNewTabs.Remove( nPos );
        aNewTabs.Insert( aAktTab );
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxTabulatorTabPage, FillTypeCheckHdl_Impl, RadioButton *, pBox )
{
    bCheck |= sal_True;
    sal_uInt8 cFill = ' ';
    aFillChar.SetText( String() );
    aFillChar.Disable();

    if( pBox == &aFillSpecial )
        aFillChar.Enable();
    else if ( pBox == &aNoFillChar )
        cFill = ' ';
    else if ( pBox == &aFillSolidLine )
        cFill = '_';
    else if ( pBox == &aFillPoints )
        cFill = '.';
    else if ( pBox == &aFillDashLine )
        cFill = '-';

    aAktTab.GetFill() = cFill;
    sal_uInt16 nPos = aTabBox.GetValuePos( aTabBox.GetValue( eDefUnit ), eDefUnit );

    if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
    {
        aNewTabs.Remove( nPos );
        aNewTabs.Insert( aAktTab );
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxTabulatorTabPage, GetFillCharHdl_Impl, Edit *, pEdit )
{
    String aChar( pEdit->GetText() );

    if ( aChar.Len() > 0 )
        aAktTab.GetFill() = aChar.GetChar( 0 );

    const sal_uInt16 nPos = aTabBox.GetValuePos( aTabBox.GetValue( eDefUnit ), eDefUnit);
    if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
    {
        aNewTabs.Remove( nPos );
        aNewTabs.Insert( aAktTab );
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxTabulatorTabPage, GetDezCharHdl_Impl, Edit *, pEdit )
{
    String aChar( pEdit->GetText() );
    if ( aChar.Len() > 0 && ( aChar.GetChar( 0 ) >= ' '))
        aAktTab.GetDecimal() = aChar.GetChar( 0 );

    sal_uInt16 nPos = aTabBox.GetValuePos( aTabBox.GetValue( eDefUnit ), eDefUnit );
    if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
    {
        aNewTabs.Remove( nPos );
        aNewTabs.Insert( aAktTab );
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxTabulatorTabPage, SelectHdl_Impl)
{
    sal_uInt16 nPos = aTabBox.GetValuePos( aTabBox.GetValue( eDefUnit ), eDefUnit );
    if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
    {
        aAktTab = aNewTabs[nPos];
        aNewBtn.Disable();
        SetFillAndTabType_Impl();
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxTabulatorTabPage, ModifyHdl_Impl)
{
    sal_uInt16 nPos = aTabBox.GetValuePos( aTabBox.GetValue( eDefUnit ), eDefUnit );
    if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
    {
        aAktTab = aNewTabs[nPos];
        SetFillAndTabType_Impl();

        aAktTab.GetTabPos() =
            static_cast<long>(aTabBox.Denormalize( aTabBox.GetValue( eDefUnit ) ));

        aNewBtn.Disable();
        aDelBtn.Enable();
        return 0;
    }
    aNewBtn.Enable();
    aDelBtn.Disable();
    return 0;
}

void SvxTabulatorTabPage::PageCreated(SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pControlItem,SfxUInt16Item,SID_SVXTABULATORTABPAGE_CONTROLFLAGS,sal_False);
    if (pControlItem)
        DisableControls(pControlItem->GetValue());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
