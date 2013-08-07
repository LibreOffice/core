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

#include <cuires.hrc>
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
    SvxTabulatorTabPage* mpPage;
private:
    sal_uInt16  nTabStyle;

public:

    TabWin_Impl( Window* pParent, WinBits nBits) :
        Window( pParent, nBits )
    {
    }

    virtual void    Paint( const Rectangle& rRect );

    void SetTabulatorTabPage(SvxTabulatorTabPage* pPage) { mpPage = pPage; }
    void SetTabStyle(sal_uInt16 nStyle) {nTabStyle = nStyle; }
};

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeTabWin_Impl(Window *pParent, VclBuilder::stringmap &)
{
    return new TabWin_Impl(pParent, 0);
}
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
    Ruler::DrawTab( this, GetSettings().GetStyleSettings().GetFontColor(), aPnt, nTabStyle );
}

// class SvxTabulatorTabPage ---------------------------------------------

SvxTabulatorTabPage::SvxTabulatorTabPage( Window* pParent, const SfxItemSet& rAttr ):

    SfxTabPage( pParent, "ParagraphTabsPage","cui/ui/paratabspage.ui", rAttr ),

    aAktTab     ( 0 ),
    aNewTabs    ( 0, 0, SVX_TAB_ADJUST_LEFT, GetWhich( SID_ATTR_TABSTOP ) ),
    nDefDist    ( 0 ),
    eDefUnit    ( FUNIT_100TH_MM ),
    bCheck      ( sal_False )

{
    get(m_pTabBox,"ED_TABPOS");
    //the tab images
    get(m_pLeftWin,"drawingareaWIN_TABLEFT");
    get(m_pRightWin,"drawingareaWIN_TABRIGHT");
    get(m_pCenterWin,"drawingareaWIN_TABCENTER");
    get(m_pDezWin,"drawingareaWIN_TABDECIMAL");
    m_pLeftWin->SetTabulatorTabPage(this);
    m_pRightWin->SetTabulatorTabPage(this);
    m_pCenterWin->SetTabulatorTabPage(this);
    m_pDezWin->SetTabulatorTabPage(this);
    m_pLeftWin->SetTabStyle((sal_uInt16)(RULER_TAB_LEFT|WB_HORZ));
    m_pRightWin->SetTabStyle((sal_uInt16)(RULER_TAB_RIGHT|WB_HORZ));
    m_pCenterWin->SetTabStyle((sal_uInt16)(RULER_TAB_CENTER|WB_HORZ));
    m_pDezWin->SetTabStyle((sal_uInt16)(RULER_TAB_DECIMAL|WB_HORZ));
    //upper radiobuttons
    SvtCJKOptions aCJKOptions;
    get(m_pLeftTab,  aCJKOptions.IsAsianTypographyEnabled() ? "radiobuttonST_LEFTTAB_ASIAN" : "radiobuttonBTN_TABTYPE_LEFT");
    get(m_pRightTab, aCJKOptions.IsAsianTypographyEnabled() ? "radiobuttonST_RIGHTTAB_ASIAN" : "radiobuttonBTN_TABTYPE_RIGHT");
    m_pLeftTab->Show(true);
    m_pRightTab->Show(true);
    get(m_pCenterTab,"radiobuttonBTN_TABTYPE_CENTER");
    get(m_pDezTab,"radiobuttonBTN_TABTYPE_DECIMAL");
    get(m_pDezChar,"entryED_TABTYPE_DECCHAR");
    get(m_pDezCharLabel,"labelFT_TABTYPE_DECCHAR");
    //lower radio buttons
    get(m_pNoFillChar,"radiobuttonBTN_FILLCHAR_NO");
    get(m_pFillPoints,"radiobuttonBTN_FILLCHAR_POINTS");
    get(m_pFillDashLine,"radiobuttonBTN_FILLCHAR_DASHLINE");
    get(m_pFillSolidLine,"radiobuttonBTN_FILLCHAR_UNDERSCORE");
    get(m_pFillSpecial,"radiobuttonBTN_FILLCHAR_OTHER");
    get(m_pFillChar,"entryED_FILLCHAR_OTHER");
    //button bar
    get(m_pNewBtn,"buttonBTN_NEW");
    get(m_pDelAllBtn,"buttonBTN_DELALL");
    get(m_pDelBtn,"buttonBTN_DEL");

    get(m_pTypeFrame, "frameFL_TABTYPE");
    get(m_pFillFrame, "frameFL_FILLCHAR");

    // This page needs ExchangeSupport
    SetExchangeSupport();

    // Set metric
    FieldUnit eFUnit = GetModuleFieldUnit( rAttr );
    SetFieldUnit( *m_pTabBox, eFUnit );

    // Initialize buttons
    m_pNewBtn->SetClickHdl( LINK( this,SvxTabulatorTabPage, NewHdl_Impl ) );
    m_pDelBtn->SetClickHdl( LINK( this,SvxTabulatorTabPage, DelHdl_Impl ) );
    m_pDelAllBtn->SetClickHdl( LINK( this,SvxTabulatorTabPage, DelAllHdl_Impl ) );

    Link aLink = LINK( this, SvxTabulatorTabPage, TabTypeCheckHdl_Impl );
    m_pLeftTab->SetClickHdl( aLink );
    m_pRightTab->SetClickHdl( aLink );
    m_pDezTab->SetClickHdl( aLink );
    m_pCenterTab->SetClickHdl( aLink );

    m_pDezChar->SetLoseFocusHdl( LINK( this,  SvxTabulatorTabPage, GetDezCharHdl_Impl ) );
    m_pDezChar->Disable();
    m_pDezCharLabel->Disable();

    aLink = LINK( this, SvxTabulatorTabPage, FillTypeCheckHdl_Impl );
    m_pNoFillChar->SetClickHdl( aLink );
    m_pFillPoints->SetClickHdl( aLink );
    m_pFillDashLine->SetClickHdl( aLink );
    m_pFillSolidLine->SetClickHdl( aLink );
    m_pFillSpecial->SetClickHdl( aLink );
    m_pFillChar->SetLoseFocusHdl( LINK( this,  SvxTabulatorTabPage, GetFillCharHdl_Impl ) );
    m_pFillChar->Disable();

    m_pTabBox->SetDoubleClickHdl( LINK( this, SvxTabulatorTabPage, SelectHdl_Impl ) );
    m_pTabBox->SetModifyHdl( LINK( this, SvxTabulatorTabPage, ModifyHdl_Impl ) );

    // Get the default decimal char from the system
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    aAktTab.GetDecimal() = rLocaleWrapper.getNumDecimalSep()[0];
}

// -----------------------------------------------------------------------

SvxTabulatorTabPage::~SvxTabulatorTabPage()
{
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
    if ( m_pNewBtn->IsEnabled() )
        NewHdl_Impl( 0 );

    // Call the LoseFocus-Handler first
    GetDezCharHdl_Impl( m_pDezChar );
    GetFillCharHdl_Impl( m_pFillChar );

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
        if ( SFX_ITEM_SET != rSet.GetItemState( GetWhich( SID_ATTR_LRSPACE ), sal_True, &pLRSpace ) )
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
                aTmpStop.GetTabPos() = LogicToLogic( aTmpStop.GetTabPos(), eUnit, MAP_100TH_MM );
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
        m_pLeftTab->Disable();
        m_pLeftWin->Disable();
    }
    if ( ( TABTYPE_RIGHT & nFlag ) == TABTYPE_RIGHT )
    {
        m_pRightTab->Disable();
        m_pRightWin->Disable();
    }
    if ( ( TABTYPE_CENTER & nFlag ) == TABTYPE_CENTER )
    {
        m_pCenterTab->Disable();
        m_pCenterWin->Disable();
    }
    if ( ( TABTYPE_DEZIMAL & nFlag ) == TABTYPE_DEZIMAL )
    {
        m_pDezTab->Disable();
        m_pDezWin->Disable();
        m_pDezCharLabel->Disable();
        m_pDezChar->Disable();
    }
     if ( ( TABTYPE_ALL & nFlag ) == TABTYPE_ALL )
         m_pTypeFrame->Disable();
    if ( ( TABFILL_NONE & nFlag ) == TABFILL_NONE )
        m_pNoFillChar->Disable();
    if ( ( TABFILL_POINT & nFlag ) == TABFILL_POINT )
        m_pFillPoints->Disable();
    if ( ( TABFILL_DASHLINE & nFlag ) == TABFILL_DASHLINE )
        m_pFillDashLine->Disable();
    if ( ( TABFILL_SOLIDLINE & nFlag ) == TABFILL_SOLIDLINE )
        m_pFillSolidLine->Disable();
    if ( ( TABFILL_SPECIAL & nFlag ) == TABFILL_SPECIAL )
    {
        m_pFillSpecial->Disable();
        m_pFillChar->Disable();
    }
     if ( ( TABFILL_ALL & nFlag ) == TABFILL_ALL )
         m_pFillFrame->Disable();
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
    m_pTabBox->Clear();

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
            m_pTabBox->InsertValue( m_pTabBox->Normalize(
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
    m_pLeftTab->Check( sal_True );
    m_pNoFillChar->Check( sal_True );

    if( m_pTabBox->GetEntryCount() > 0 )
    {
        m_pTabBox->SetText( m_pTabBox->GetEntry( nTabPos ) );
        aAktTab = aNewTabs[nTabPos];

        SetFillAndTabType_Impl();
        m_pNewBtn->Disable();
        m_pDelBtn->Enable();
    }
    else
    {   // If no entry, 0 is the default value
        m_pTabBox->SetValue( 0, eDefUnit );

        m_pNewBtn->Enable();
        m_pDelBtn->Disable();
    }
}

// -----------------------------------------------------------------------

void SvxTabulatorTabPage::SetFillAndTabType_Impl()
{
    RadioButton* pTypeBtn = 0;
    RadioButton* pFillBtn = 0;

    m_pDezChar->Disable();
    m_pDezCharLabel->Disable();

    if ( aAktTab.GetAdjustment() == SVX_TAB_ADJUST_LEFT )
        pTypeBtn = m_pLeftTab;
    else if ( aAktTab.GetAdjustment() == SVX_TAB_ADJUST_RIGHT )
        pTypeBtn = m_pRightTab;
    else if ( aAktTab.GetAdjustment() == SVX_TAB_ADJUST_DECIMAL )
    {
        pTypeBtn = m_pDezTab;
        m_pDezChar->Enable();
        m_pDezCharLabel->Enable();
        m_pDezChar->SetText( OUString( (sal_Unicode)aAktTab.GetDecimal() ) );
    }
    else if ( aAktTab.GetAdjustment() == SVX_TAB_ADJUST_CENTER )
        pTypeBtn = m_pCenterTab;

    if ( pTypeBtn )
        pTypeBtn->Check();

    m_pFillChar->Disable();
    m_pFillChar->SetText( "" );

    if ( aAktTab.GetFill() == ' ' )
        pFillBtn = m_pNoFillChar;
    else if ( aAktTab.GetFill() == '-' )
        pFillBtn = m_pFillDashLine;
    else if ( aAktTab.GetFill() == '_' )
        pFillBtn = m_pFillSolidLine;
    else if ( aAktTab.GetFill() == '.' )
        pFillBtn = m_pFillPoints;
    else
    {
        pFillBtn = m_pFillSpecial;
        m_pFillChar->Enable();
        m_pFillChar->SetText( OUString( (sal_Unicode)aAktTab.GetFill() ) );
    }
    pFillBtn->Check();
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxTabulatorTabPage, NewHdl_Impl, Button *, pBtn )
{
    // Add a new one and select it
    // Get the value from the display
    long nVal = static_cast<long>(m_pTabBox->Denormalize( m_pTabBox->GetValue( eDefUnit ) ));

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
    sal_uInt16 nSize = m_pTabBox->GetEntryCount();

    sal_uInt16 i;
    for( i = 0; i < nSize; i++ )
    {
        if ( nReal < aNewTabs[i].GetTabPos() )
            break;
    }

    // Make ListBox entry
    m_pTabBox->InsertValue( m_pTabBox->Normalize( nVal ), eDefUnit, i );
    aAktTab.GetTabPos() = nReal;
    SvxTabAdjust eAdj = SVX_TAB_ADJUST_LEFT;

    if ( m_pRightTab->IsChecked() )
        eAdj = SVX_TAB_ADJUST_RIGHT;
    else if ( m_pCenterTab->IsChecked() )
        eAdj = SVX_TAB_ADJUST_CENTER;
    else if ( m_pDezTab->IsChecked() )
        eAdj = SVX_TAB_ADJUST_DECIMAL;

    aAktTab.GetAdjustment() = eAdj;
    aNewTabs.Insert( aAktTab );

    m_pNewBtn->Disable();
    m_pDelBtn->Enable();
    m_pTabBox->GrabFocus();

    // If no RadioButton was clicked, we need to put anyway
    bCheck |= sal_True;
    // Set the selection into the position Edit
    m_pTabBox->SetSelection(Selection(0, m_pTabBox->GetText().getLength()));
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxTabulatorTabPage, DelHdl_Impl)
{
    sal_uInt16 nPos = m_pTabBox->GetValuePos( m_pTabBox->GetValue() );

    if ( nPos == COMBOBOX_ENTRY_NOTFOUND )
        return 0;

    if ( m_pTabBox->GetEntryCount() == 1 )
    {
        DelAllHdl_Impl( 0 );
        return 0;
    }

    // Delete Tab
    m_pTabBox->RemoveEntryAt(nPos);
    aNewTabs.Remove( nPos );

    // Reset aAktTab
    const sal_uInt16 nSize = aNewTabs.Count();

    if ( nSize > 0 )
    {
        // Correct Pos
        nPos = ( ( nSize - 1 ) >= nPos) ? nPos : nPos - 1;
        m_pTabBox->SetValue( m_pTabBox->GetValue( nPos ) );
        aAktTab = aNewTabs[nPos];
    }

    // If no Tabs Enable Disable Controls
    if ( m_pTabBox->GetEntryCount() == 0 )
    {
        m_pDelBtn->Disable();
        m_pNewBtn->Enable();
        m_pTabBox->GrabFocus();
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
    m_pDezChar->Disable();
    m_pDezCharLabel->Disable();
    m_pDezChar->SetText( "" );

    if ( pBox == m_pLeftTab )
        eAdj = SVX_TAB_ADJUST_LEFT;
    else if ( pBox == m_pRightTab )
        eAdj = SVX_TAB_ADJUST_RIGHT;
    else if ( pBox == m_pCenterTab )
        eAdj = SVX_TAB_ADJUST_CENTER;
    else
    {
        eAdj = SVX_TAB_ADJUST_DECIMAL;
        m_pDezChar->Enable();
        m_pDezCharLabel->Enable();
        m_pDezChar->SetText( OUString( (sal_Unicode)aAktTab.GetDecimal() ) );
    }

    aAktTab.GetAdjustment() = eAdj;
    sal_uInt16 nPos = m_pTabBox->GetValuePos( m_pTabBox->GetValue( eDefUnit ), eDefUnit );

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
    m_pFillChar->SetText( "" );
    m_pFillChar->Disable();

    if( pBox == m_pFillSpecial )
        m_pFillChar->Enable();
    else if ( pBox == m_pNoFillChar )
        cFill = ' ';
    else if ( pBox == m_pFillSolidLine )
        cFill = '_';
    else if ( pBox == m_pFillPoints )
        cFill = '.';
    else if ( pBox == m_pFillDashLine )
        cFill = '-';

    aAktTab.GetFill() = cFill;
    sal_uInt16 nPos = m_pTabBox->GetValuePos( m_pTabBox->GetValue( eDefUnit ), eDefUnit );

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

    if ( aChar.Len() > 0)
        aAktTab.GetFill() = aChar.GetChar( 0 );

    const sal_uInt16 nPos = m_pTabBox->GetValuePos( m_pTabBox->GetValue( eDefUnit ), eDefUnit);
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

    sal_uInt16 nPos = m_pTabBox->GetValuePos( m_pTabBox->GetValue( eDefUnit ), eDefUnit );
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
    sal_uInt16 nPos = m_pTabBox->GetValuePos( m_pTabBox->GetValue( eDefUnit ), eDefUnit );
    if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
    {
        aAktTab = aNewTabs[nPos];
        m_pNewBtn->Disable();
        SetFillAndTabType_Impl();
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxTabulatorTabPage, ModifyHdl_Impl)
{
    sal_uInt16 nPos = m_pTabBox->GetValuePos( m_pTabBox->GetValue( eDefUnit ), eDefUnit );
    if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
    {
        aAktTab = aNewTabs[nPos];
        SetFillAndTabType_Impl();

        aAktTab.GetTabPos() =
            static_cast<long>(m_pTabBox->Denormalize( m_pTabBox->GetValue( eDefUnit ) ));

        m_pNewBtn->Disable();
        m_pDelBtn->Enable();
        return 0;
    }
    m_pNewBtn->Enable();
    m_pDelBtn->Disable();
    return 0;
}

void SvxTabulatorTabPage::PageCreated(SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pControlItem,SfxUInt16Item,SID_SVXTABULATORTABPAGE_CONTROLFLAGS,sal_False);
    if (pControlItem)
        DisableControls(pControlItem->GetValue());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
