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
#include <svtools/ruler.hxx>
#include <svx/dialogs.hrc>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>

#include <editeng/lrspitem.hxx>
#include <tabstpge.hxx>
#include <svx/dlgutil.hxx>
#include <sfx2/module.hxx>
#include <svl/cjkoptions.hxx>
#include <unotools/localedatawrapper.hxx>
#include <sfx2/request.hxx>
#include <svl/intitem.hxx>

// class TabWin_Impl -----------------------------------------------------

class TabWin_Impl : public vcl::Window
{
    VclPtr<SvxTabulatorTabPage> mpPage;
private:
    sal_uInt16  nTabStyle;

public:

    TabWin_Impl(vcl::Window* pParent, WinBits nBits)
        : Window(pParent, nBits)
        , mpPage(nullptr)
        , nTabStyle(0)
    {
    }
    virtual ~TabWin_Impl() override { disposeOnce(); }
    virtual void dispose() override { mpPage.clear(); vcl::Window::dispose(); }

    virtual void Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect) override;

    void SetTabulatorTabPage(SvxTabulatorTabPage* pPage) { mpPage = pPage; }
    void SetTabStyle(sal_uInt16 nStyle) {nTabStyle = nStyle; }
};

VCL_BUILDER_FACTORY_ARGS(TabWin_Impl, 0)

const sal_uInt16 SvxTabulatorTabPage::pRanges[] =
{
    SID_ATTR_TABSTOP,
    SID_ATTR_TABSTOP_OFFSET,
    0
};

void FillUpWithDefTabs_Impl( long nDefDist, SvxTabStopItem& rTabs )
{
    if( rTabs.Count() )
        return;
    {
        SvxTabStop aSwTabStop( nDefDist, SvxTabAdjust::Default );
        rTabs.Insert( aSwTabStop );
    }
}

void TabWin_Impl::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle&)
{
    // Paint tabulators
    Point aPoint;
    Size aSize(GetOutputSizePixel());
    aPoint.setX( aSize.Width() / 2 );
    aPoint.setY( aSize.Height() / 2 );
    Ruler::DrawTab(rRenderContext, rRenderContext.GetSettings().GetStyleSettings().GetFontColor(), aPoint, nTabStyle);
}

SvxTabulatorTabPage::SvxTabulatorTabPage(vcl::Window* pParent, const SfxItemSet& rAttr)
    : SfxTabPage(pParent, "ParagraphTabsPage","cui/ui/paratabspage.ui", &rAttr)
    , aCurrentTab(0)
    , aNewTabs(0, 0, SvxTabAdjust::Left, GetWhich(SID_ATTR_TABSTOP))
    , nDefDist(0)
    , eDefUnit(FUNIT_100TH_MM)
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
    m_pLeftWin->SetTabStyle(sal_uInt16(RULER_TAB_LEFT|WB_HORZ));
    m_pRightWin->SetTabStyle(sal_uInt16(RULER_TAB_RIGHT|WB_HORZ));
    m_pCenterWin->SetTabStyle(sal_uInt16(RULER_TAB_CENTER|WB_HORZ));
    m_pDezWin->SetTabStyle(sal_uInt16(RULER_TAB_DECIMAL|WB_HORZ));
    //upper radiobuttons
    SvtCJKOptions aCJKOptions;
    get(m_pLeftTab,  aCJKOptions.IsAsianTypographyEnabled() ? "radiobuttonST_LEFTTAB_ASIAN" : "radiobuttonBTN_TABTYPE_LEFT");
    get(m_pRightTab, aCJKOptions.IsAsianTypographyEnabled() ? "radiobuttonST_RIGHTTAB_ASIAN" : "radiobuttonBTN_TABTYPE_RIGHT");
    m_pLeftTab->Show();
    m_pRightTab->Show();
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

    Link<Button*,void> aLink = LINK( this, SvxTabulatorTabPage, TabTypeCheckHdl_Impl );
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
    aCurrentTab.GetDecimal() = rLocaleWrapper.getNumDecimalSep()[0];
}

SvxTabulatorTabPage::~SvxTabulatorTabPage()
{
    disposeOnce();
}

void SvxTabulatorTabPage::dispose()
{
    m_pTabBox.clear();
    m_pLeftTab.clear();
    m_pRightTab.clear();
    m_pCenterTab.clear();
    m_pDezTab.clear();
    m_pLeftWin.clear();
    m_pRightWin.clear();
    m_pCenterWin.clear();
    m_pDezWin.clear();
    m_pDezCharLabel.clear();
    m_pDezChar.clear();
    m_pNoFillChar.clear();
    m_pFillPoints.clear();
    m_pFillDashLine.clear();
    m_pFillSolidLine.clear();
    m_pFillSpecial.clear();
    m_pFillChar.clear();
    m_pNewBtn.clear();
    m_pDelAllBtn.clear();
    m_pDelBtn.clear();
    m_pTypeFrame.clear();
    m_pFillFrame.clear();
    SfxTabPage::dispose();
}

bool SvxTabulatorTabPage::FillItemSet(SfxItemSet* rSet)
{
    bool bModified = false;

    // Put the controls' values in here
    if (m_pNewBtn->IsEnabled())
        NewHdl_Impl( nullptr );

    // Call the LoseFocus-Handler first
    GetDezCharHdl_Impl(*m_pDezChar);
    GetFillCharHdl_Impl(*m_pFillChar);

    FillUpWithDefTabs_Impl(nDefDist, aNewTabs);
    SfxItemPool* pPool = rSet->GetPool();
    MapUnit eUnit = pPool->GetMetric(GetWhich(SID_ATTR_TABSTOP));
    const SfxPoolItem* pOld = GetOldItem(*rSet, SID_ATTR_TABSTOP);

    if (MapUnit::Map100thMM != eUnit)
    {
        // If the ItemSet contains a LRSpaceItem with negative first line indent,
        // the TabStopItem needs to have a DefTab at position 0.
        const SfxPoolItem* pLRSpace;
        // If not in the new set, then maybe in the old one
        if (SfxItemState::SET != rSet->GetItemState(GetWhich(SID_ATTR_LRSPACE), true, &pLRSpace))
            pLRSpace = GetOldItem(*rSet, SID_ATTR_LRSPACE);

        if (pLRSpace && static_cast<const SvxLRSpaceItem*>(pLRSpace)->GetTextFirstLineOfst() < 0)
        {
            SvxTabStop aNull(0, SvxTabAdjust::Default);
            aNewTabs.Insert(aNull);
        }

        SvxTabStopItem aTmp(aNewTabs);
        aTmp.Remove(0, aTmp.Count());

        for (sal_uInt16 i = 0; i < aNewTabs.Count(); ++i)
        {
            SvxTabStop aTmpStop = aNewTabs[i];
            aTmpStop.GetTabPos() = LogicToLogic(aTmpStop.GetTabPos(), MapUnit::Map100thMM, eUnit);
            aTmp.Insert(aTmpStop);
        }

        if (!pOld || !(*static_cast<const SvxTabStopItem*>(pOld) == aTmp))
        {
            rSet->Put(aTmp);
            bModified = true;
        }
    }
    else if (!pOld || !( *static_cast<const SvxTabStopItem*>(pOld) == aNewTabs))
    {
        rSet->Put(aNewTabs);
        bModified = true;
    }
    return bModified;
}

VclPtr<SfxTabPage> SvxTabulatorTabPage::Create(TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<SvxTabulatorTabPage>::Create(pParent.pParent, *rSet);
}

void SvxTabulatorTabPage::Reset(const SfxItemSet* rSet)
{
    SfxItemPool* pPool = rSet->GetPool();
    MapUnit eUnit = pPool->GetMetric(GetWhich(SID_ATTR_TABSTOP));

    // Current tabs
    const SfxPoolItem* pItem = GetItem(*rSet, SID_ATTR_TABSTOP);

    if (pItem)
    {
        if (MapUnit::Map100thMM != eUnit)
        {
            SvxTabStopItem aTmp(*static_cast<const SvxTabStopItem*>(pItem));
            aNewTabs.Remove(0, aNewTabs.Count());

            for (sal_uInt16 i = 0; i < aTmp.Count(); ++i)
            {
                SvxTabStop aTmpStop = aTmp[i];
                aTmpStop.GetTabPos() = LogicToLogic(aTmpStop.GetTabPos(), eUnit, MapUnit::Map100thMM);
                aNewTabs.Insert(aTmpStop);
            }
        }
        else
            aNewTabs = *static_cast<const SvxTabStopItem*>(pItem);
    }
    else
        aNewTabs.Remove(0, aNewTabs.Count());

    // Default tab distance
    nDefDist = SVX_TAB_DEFDIST;
    pItem = GetItem(*rSet, SID_ATTR_TABSTOP_DEFAULTS);

    if (pItem)
        nDefDist = LogicToLogic(long(static_cast<const SfxUInt16Item*>(pItem)->GetValue()), eUnit, MapUnit::Map100thMM);

    // Tab pos currently selected
    sal_uInt16 nTabPos = 0;
    pItem = GetItem(*rSet, SID_ATTR_TABSTOP_POS);

    if (pItem)
        nTabPos = static_cast<const SfxUInt16Item*>(pItem)->GetValue();

    InitTabPos_Impl(nTabPos);
}

void SvxTabulatorTabPage::DisableControls(const TabulatorDisableFlags nFlag)
{
    if (TabulatorDisableFlags::TypeLeft & nFlag)
    {
        m_pLeftTab->Disable();
        m_pLeftWin->Disable();
    }
    if (TabulatorDisableFlags::TypeRight & nFlag)
    {
        m_pRightTab->Disable();
        m_pRightWin->Disable();
    }
    if (TabulatorDisableFlags::TypeCenter & nFlag)
    {
        m_pCenterTab->Disable();
        m_pCenterWin->Disable();
    }
    if (TabulatorDisableFlags::TypeDecimal & nFlag)
    {
        m_pDezTab->Disable();
        m_pDezWin->Disable();
        m_pDezCharLabel->Disable();
        m_pDezChar->Disable();
    }
    if (TabulatorDisableFlags::TypeMask & nFlag)
         m_pTypeFrame->Disable();
    if (TabulatorDisableFlags::FillNone & nFlag)
        m_pNoFillChar->Disable();
    if (TabulatorDisableFlags::FillPoint & nFlag)
        m_pFillPoints->Disable();
    if (TabulatorDisableFlags::FillDashLine & nFlag)
        m_pFillDashLine->Disable();
    if (TabulatorDisableFlags::FillSolidLine & nFlag)
        m_pFillSolidLine->Disable();
    if (TabulatorDisableFlags::FillSpecial & nFlag)
    {
        m_pFillSpecial->Disable();
        m_pFillChar->Disable();
    }
    if (TabulatorDisableFlags::FillMask & nFlag)
        m_pFillFrame->Disable();
}

DeactivateRC SvxTabulatorTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

void SvxTabulatorTabPage::InitTabPos_Impl( sal_uInt16 nTabPos )
{
    m_pTabBox->Clear();

    long nOffset = 0;
    const SfxPoolItem* pItem = nullptr;
    if (GetItemSet().GetItemState(SID_ATTR_TABSTOP_OFFSET, true, &pItem) == SfxItemState::SET)
    {
        nOffset = static_cast<const SfxInt32Item*>(pItem)->GetValue();
        MapUnit eUnit = GetItemSet().GetPool()->GetMetric(GetWhich(SID_ATTR_TABSTOP));
        nOffset = OutputDevice::LogicToLogic(nOffset, eUnit, MapUnit::Map100thMM);
    }

    // Correct current TabPos and default tabs
    for ( sal_uInt16 i = 0; i < aNewTabs.Count(); i++ )
    {
        if ( aNewTabs[i].GetAdjustment() != SvxTabAdjust::Default )
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
    m_pLeftTab->Check();
    m_pNoFillChar->Check();

    if( m_pTabBox->GetEntryCount() > 0 )
    {
        m_pTabBox->SetText( m_pTabBox->GetEntry( nTabPos ) );
        aCurrentTab = aNewTabs[nTabPos];

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

void SvxTabulatorTabPage::SetFillAndTabType_Impl()
{
    RadioButton* pTypeBtn = nullptr;
    RadioButton* pFillBtn = nullptr;

    m_pDezChar->Disable();
    m_pDezCharLabel->Disable();

    if ( aCurrentTab.GetAdjustment() == SvxTabAdjust::Left )
        pTypeBtn = m_pLeftTab;
    else if ( aCurrentTab.GetAdjustment() == SvxTabAdjust::Right )
        pTypeBtn = m_pRightTab;
    else if ( aCurrentTab.GetAdjustment() == SvxTabAdjust::Decimal )
    {
        pTypeBtn = m_pDezTab;
        m_pDezChar->Enable();
        m_pDezCharLabel->Enable();
        m_pDezChar->SetText( OUString( aCurrentTab.GetDecimal() ) );
    }
    else if ( aCurrentTab.GetAdjustment() == SvxTabAdjust::Center )
        pTypeBtn = m_pCenterTab;

    if ( pTypeBtn )
        pTypeBtn->Check();

    m_pFillChar->Disable();
    m_pFillChar->SetText( "" );

    if ( aCurrentTab.GetFill() == ' ' )
        pFillBtn = m_pNoFillChar;
    else if ( aCurrentTab.GetFill() == '-' )
        pFillBtn = m_pFillDashLine;
    else if ( aCurrentTab.GetFill() == '_' )
        pFillBtn = m_pFillSolidLine;
    else if ( aCurrentTab.GetFill() == '.' )
        pFillBtn = m_pFillPoints;
    else
    {
        pFillBtn = m_pFillSpecial;
        m_pFillChar->Enable();
        m_pFillChar->SetText( OUString( aCurrentTab.GetFill() ) );
    }
    pFillBtn->Check();
}

IMPL_LINK( SvxTabulatorTabPage, NewHdl_Impl, Button *, pBtn, void )
{
    // Add a new one and select it
    // Get the value from the display
    long nVal = static_cast<long>(m_pTabBox->Denormalize( m_pTabBox->GetValue( eDefUnit ) ));

    // If the pBtn == 0 && the value == 0 then do not create a tab, because we create via OK
    if ( nVal == 0 && pBtn == nullptr )
        return;

    long nOffset = 0;
    const SfxPoolItem* pItem = nullptr;

    if ( GetItemSet().GetItemState( SID_ATTR_TABSTOP_OFFSET, true, &pItem ) ==
         SfxItemState::SET )
    {
        nOffset = static_cast<const SfxInt32Item*>(pItem)->GetValue();
        MapUnit eUnit = GetItemSet().GetPool()->GetMetric( GetWhich( SID_ATTR_TABSTOP ) );
        nOffset = OutputDevice::LogicToLogic( nOffset, eUnit, MapUnit::Map100thMM  );
    }
    const long nReal = nVal - nOffset;
    sal_Int32 nSize = m_pTabBox->GetEntryCount();

    sal_Int32 i;
    for( i = 0; i < nSize; i++ )
    {
        if ( nReal < aNewTabs[i].GetTabPos() )
            break;
    }

    // Make ListBox entry
    m_pTabBox->InsertValue( m_pTabBox->Normalize( nVal ), eDefUnit, i );
    aCurrentTab.GetTabPos() = nReal;
    SvxTabAdjust eAdj = SvxTabAdjust::Left;

    if ( m_pRightTab->IsChecked() )
        eAdj = SvxTabAdjust::Right;
    else if ( m_pCenterTab->IsChecked() )
        eAdj = SvxTabAdjust::Center;
    else if ( m_pDezTab->IsChecked() )
        eAdj = SvxTabAdjust::Decimal;

    aCurrentTab.GetAdjustment() = eAdj;
    aNewTabs.Insert( aCurrentTab );

    m_pNewBtn->Disable();
    m_pDelBtn->Enable();
    m_pTabBox->GrabFocus();

    // Set the selection into the position Edit
    m_pTabBox->SetSelection(Selection(0, m_pTabBox->GetText().getLength()));
}

IMPL_LINK_NOARG(SvxTabulatorTabPage, DelHdl_Impl, Button*, void)
{
    sal_Int32 nPos = m_pTabBox->GetValuePos( m_pTabBox->GetValue() );

    if ( nPos == COMBOBOX_ENTRY_NOTFOUND )
        return;

    if ( m_pTabBox->GetEntryCount() == 1 )
    {
        DelAllHdl_Impl( nullptr );
        return;
    }

    // Delete Tab
    m_pTabBox->RemoveEntryAt(nPos);
    aNewTabs.Remove( nPos );

    // Reset aCurrentTab
    const sal_uInt16 nSize = aNewTabs.Count();

    if ( nSize > 0 )
    {
        // Correct Pos
        nPos = ( ( nSize - 1 ) >= nPos) ? nPos : nPos - 1;
        m_pTabBox->SetValue( m_pTabBox->GetValue( nPos ) );
        aCurrentTab = aNewTabs[nPos];
    }

    // If no Tabs Enable Disable Controls
    if ( m_pTabBox->GetEntryCount() == 0 )
    {
        m_pDelBtn->Disable();
        m_pNewBtn->Enable();
        m_pTabBox->GrabFocus();
    }
}

IMPL_LINK_NOARG(SvxTabulatorTabPage, DelAllHdl_Impl, Button*, void)
{
    if ( aNewTabs.Count() )
    {
        aNewTabs = SvxTabStopItem( 0 );
        InitTabPos_Impl();
    }
}

IMPL_LINK( SvxTabulatorTabPage, TabTypeCheckHdl_Impl, Button *, pBox, void )
{
    SvxTabAdjust eAdj;
    m_pDezChar->Disable();
    m_pDezCharLabel->Disable();
    m_pDezChar->SetText( "" );

    if ( pBox == m_pLeftTab )
        eAdj = SvxTabAdjust::Left;
    else if ( pBox == m_pRightTab )
        eAdj = SvxTabAdjust::Right;
    else if ( pBox == m_pCenterTab )
        eAdj = SvxTabAdjust::Center;
    else
    {
        eAdj = SvxTabAdjust::Decimal;
        m_pDezChar->Enable();
        m_pDezCharLabel->Enable();
        m_pDezChar->SetText( OUString( aCurrentTab.GetDecimal() ) );
    }

    aCurrentTab.GetAdjustment() = eAdj;
    sal_Int32 nPos = m_pTabBox->GetValuePos( m_pTabBox->GetValue( eDefUnit ), eDefUnit );

    if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
    {
        aNewTabs.Remove( nPos );
        aNewTabs.Insert( aCurrentTab );
    }
}

IMPL_LINK( SvxTabulatorTabPage, FillTypeCheckHdl_Impl, Button *, pBox, void )
{
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

    aCurrentTab.GetFill() = cFill;
    sal_Int32 nPos = m_pTabBox->GetValuePos( m_pTabBox->GetValue( eDefUnit ), eDefUnit );

    if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
    {
        aNewTabs.Remove( nPos );
        aNewTabs.Insert( aCurrentTab );
    }
}

IMPL_LINK( SvxTabulatorTabPage, GetFillCharHdl_Impl, Control&, rControl, void )
{
    OUString aChar( static_cast<Edit&>(rControl).GetText() );

    if ( !aChar.isEmpty() )
        aCurrentTab.GetFill() = aChar[0];

    const sal_Int32 nPos = m_pTabBox->GetValuePos( m_pTabBox->GetValue( eDefUnit ), eDefUnit);
    if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
    {
        aNewTabs.Remove( nPos );
        aNewTabs.Insert( aCurrentTab );
    }
}

IMPL_LINK( SvxTabulatorTabPage, GetDezCharHdl_Impl, Control&, rControl, void )
{
    OUString aChar( static_cast<Edit*>(&rControl)->GetText() );
    if ( !aChar.isEmpty() && ( aChar[0] >= ' '))
        aCurrentTab.GetDecimal() = aChar[0];

    sal_Int32 nPos = m_pTabBox->GetValuePos( m_pTabBox->GetValue( eDefUnit ), eDefUnit );
    if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
    {
        aNewTabs.Remove( nPos );
        aNewTabs.Insert( aCurrentTab );
    }
}

IMPL_LINK_NOARG(SvxTabulatorTabPage, SelectHdl_Impl, ComboBox&, void)
{
    sal_Int32 nPos = m_pTabBox->GetValuePos( m_pTabBox->GetValue( eDefUnit ), eDefUnit );
    if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
    {
        aCurrentTab = aNewTabs[nPos];
        m_pNewBtn->Disable();
        SetFillAndTabType_Impl();
    }
}

IMPL_LINK_NOARG(SvxTabulatorTabPage, ModifyHdl_Impl, Edit&, void)
{
    sal_Int32 nPos = m_pTabBox->GetValuePos( m_pTabBox->GetValue( eDefUnit ), eDefUnit );
    if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
    {
        aCurrentTab = aNewTabs[nPos];
        SetFillAndTabType_Impl();

        aCurrentTab.GetTabPos() =
            static_cast<long>(m_pTabBox->Denormalize( m_pTabBox->GetValue( eDefUnit ) ));

        m_pNewBtn->Disable();
        m_pDelBtn->Enable();
        return;
    }
    m_pNewBtn->Enable();
    m_pDelBtn->Disable();
}

void SvxTabulatorTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxUInt16Item* pControlItem = aSet.GetItem<SfxUInt16Item>(SID_SVXTABULATORTABPAGE_DISABLEFLAGS, false);
    if (pControlItem)
        DisableControls(static_cast<TabulatorDisableFlags>(pControlItem->GetValue()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
