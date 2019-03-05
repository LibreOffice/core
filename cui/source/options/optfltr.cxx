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

#include <unotools/moduleoptions.hxx>
#include <unotools/fltrcfg.hxx>
#include "optfltr.hxx"
#include <strings.hrc>
#include <dialmgr.hxx>

#include <vcl/svlbitm.hxx>
#include <vcl/treelistentry.hxx>

enum class MSFltrPg2_CheckBoxEntries {
    Math,
    Writer,
    Calc,
    Impress,
    SmartArt,
    InvalidCBEntry
};


OfaMSFilterTabPage::OfaMSFilterTabPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage( pParent, "OptFltrPage", "cui/ui/optfltrpage.ui", &rSet )
{
    get( aWBasicCodeCB      , "wo_basic"    );
    get( aWBasicWbctblCB    , "wo_exec"     );
    get( aWBasicStgCB       , "wo_saveorig" );
    get( aEBasicCodeCB      , "ex_basic"    );
    get( aEBasicExectblCB   , "ex_exec"     );
    get( aEBasicStgCB       , "ex_saveorig" );
    get( aPBasicCodeCB      , "pp_basic"    );
    get( aPBasicStgCB       , "pp_saveorig" );

    aWBasicCodeCB->SetClickHdl( LINK( this, OfaMSFilterTabPage, LoadWordBasicCheckHdl_Impl ) );
    aEBasicCodeCB->SetClickHdl( LINK( this, OfaMSFilterTabPage, LoadExcelBasicCheckHdl_Impl ) );
}

OfaMSFilterTabPage::~OfaMSFilterTabPage()
{
    disposeOnce();
}

void OfaMSFilterTabPage::dispose()
{
    aWBasicCodeCB.clear();
    aWBasicWbctblCB.clear();
    aWBasicStgCB.clear();
    aEBasicCodeCB.clear();
    aEBasicExectblCB.clear();
    aEBasicStgCB.clear();
    aPBasicCodeCB.clear();
    aPBasicStgCB.clear();
    SfxTabPage::dispose();
}


IMPL_LINK_NOARG(OfaMSFilterTabPage, LoadWordBasicCheckHdl_Impl, Button*, void)
{
    aWBasicWbctblCB->Enable( aWBasicCodeCB->IsChecked() );
}

IMPL_LINK_NOARG(OfaMSFilterTabPage, LoadExcelBasicCheckHdl_Impl, Button*, void)
{
    aEBasicExectblCB->Enable( aEBasicCodeCB->IsChecked() );
}

VclPtr<SfxTabPage> OfaMSFilterTabPage::Create( TabPageParent pParent,
                                               const SfxItemSet* rAttrSet )
{
    return VclPtr<OfaMSFilterTabPage>::Create( pParent.pParent, *rAttrSet );
}

bool OfaMSFilterTabPage::FillItemSet( SfxItemSet* )
{
    SvtFilterOptions& rOpt = SvtFilterOptions::Get();

    if( aWBasicCodeCB->IsValueChangedFromSaved() )
        rOpt.SetLoadWordBasicCode( aWBasicCodeCB->IsChecked() );
    if( aWBasicWbctblCB->IsValueChangedFromSaved() )
        rOpt.SetLoadWordBasicExecutable( aWBasicWbctblCB->IsChecked() );
    if( aWBasicStgCB->IsValueChangedFromSaved() )
        rOpt.SetLoadWordBasicStorage( aWBasicStgCB->IsChecked() );

    if( aEBasicCodeCB->IsValueChangedFromSaved())
        rOpt.SetLoadExcelBasicCode( aEBasicCodeCB->IsChecked() );
    if( aEBasicExectblCB->IsValueChangedFromSaved())
        rOpt.SetLoadExcelBasicExecutable( aEBasicExectblCB->IsChecked() );
    if( aEBasicStgCB->IsValueChangedFromSaved())
        rOpt.SetLoadExcelBasicStorage( aEBasicStgCB->IsChecked() );

    if( aPBasicCodeCB->IsValueChangedFromSaved())
        rOpt.SetLoadPPointBasicCode( aPBasicCodeCB->IsChecked() );
    if( aPBasicStgCB->IsValueChangedFromSaved())
        rOpt.SetLoadPPointBasicStorage( aPBasicStgCB->IsChecked() );

    return false;
}

void OfaMSFilterTabPage::Reset( const SfxItemSet* )
{
    const SvtFilterOptions& rOpt = SvtFilterOptions::Get();

    aWBasicCodeCB->Check( rOpt.IsLoadWordBasicCode() );
    aWBasicCodeCB->SaveValue();
    aWBasicWbctblCB->Check( rOpt.IsLoadWordBasicExecutable() );
    aWBasicWbctblCB->SaveValue();
    aWBasicStgCB->Check( rOpt.IsLoadWordBasicStorage() );
    aWBasicStgCB->SaveValue();
    LoadWordBasicCheckHdl_Impl( aWBasicCodeCB );

    aEBasicCodeCB->Check( rOpt.IsLoadExcelBasicCode() );
    aEBasicCodeCB->SaveValue();
    aEBasicExectblCB->Check( rOpt.IsLoadExcelBasicExecutable() );
    aEBasicExectblCB->SaveValue();
    aEBasicStgCB->Check( rOpt.IsLoadExcelBasicStorage() );
    aEBasicStgCB->SaveValue();
    LoadExcelBasicCheckHdl_Impl( aEBasicCodeCB );

    aPBasicCodeCB->Check( rOpt.IsLoadPPointBasicCode() );
    aPBasicCodeCB->SaveValue();
    aPBasicStgCB->Check( rOpt.IsLoadPPointBasicStorage() );
    aPBasicStgCB->SaveValue();
}

OfaMSFilterTabPage2::OfaMSFilterTabPage2( vcl::Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage( pParent, "OptFilterPage", "cui/ui/optfltrembedpage.ui", &rSet ),
    sHeader1(CuiResId(RID_SVXSTR_HEADER1)),
    sHeader2(CuiResId(RID_SVXSTR_HEADER2)),
    sChgToFromMath(CuiResId(RID_SVXSTR_CHG_MATH)),
    sChgToFromWriter(CuiResId(RID_SVXSTR_CHG_WRITER)),
    sChgToFromCalc(CuiResId(RID_SVXSTR_CHG_CALC)),
    sChgToFromImpress(CuiResId(RID_SVXSTR_CHG_IMPRESS)),
    sChgToFromSmartArt(CuiResId(RID_SVXSTR_CHG_SMARTART))
{
    get(m_pCheckLBContainer, "checklbcontainer");

    get( aHighlightingRB, "highlighting");
    get( aShadingRB,      "shading"     );

    Size aControlSize(248, 55);
    aControlSize = LogicToPixel(aControlSize, MapMode(MapUnit::MapAppFont));
    m_pCheckLBContainer->set_width_request(aControlSize.Width());
    m_pCheckLBContainer->set_height_request(aControlSize.Height());

    m_pCheckLB = VclPtr<MSFltrSimpleTable>::Create(*m_pCheckLBContainer);

    static long aStaticTabs[] = { 0, 20, 40 };
    m_pCheckLB->SvSimpleTable::SetTabs( SAL_N_ELEMENTS(aStaticTabs), aStaticTabs );

    OUString sHeader = sHeader1 + "\t" + sHeader2 + "\t";
    m_pCheckLB->InsertHeaderEntry( sHeader, HEADERBAR_APPEND,
                    HeaderBarItemBits::CENTER | HeaderBarItemBits::FIXEDPOS | HeaderBarItemBits::FIXED );

    m_pCheckLB->SetStyle( m_pCheckLB->GetStyle()|WB_HSCROLL| WB_VSCROLL );
}

OfaMSFilterTabPage2::~OfaMSFilterTabPage2()
{
    disposeOnce();
}

void OfaMSFilterTabPage2::dispose()
{
    m_xCheckButtonData.reset();
    m_pCheckLB.disposeAndClear();
    m_pCheckLBContainer.clear();
    aHighlightingRB.clear();
    aShadingRB.clear();

    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> OfaMSFilterTabPage2::Create( TabPageParent pParent,
                                                const SfxItemSet* rAttrSet )
{
    return VclPtr<OfaMSFilterTabPage2>::Create( pParent.pParent, *rAttrSet );
}

bool OfaMSFilterTabPage2::FillItemSet( SfxItemSet* )
{
    SvtFilterOptions& rOpt = SvtFilterOptions::Get();

    static struct ChkCBoxEntries{
        MSFltrPg2_CheckBoxEntries eType;
        bool (SvtFilterOptions:: *FnIs)() const;
        void (SvtFilterOptions:: *FnSet)( bool bFlag );
    } const aChkArr[] = {
        { MSFltrPg2_CheckBoxEntries::Math,     &SvtFilterOptions::IsMathType2Math,
                        &SvtFilterOptions::SetMathType2Math },
        { MSFltrPg2_CheckBoxEntries::Math,     &SvtFilterOptions::IsMath2MathType,
                        &SvtFilterOptions::SetMath2MathType },
        { MSFltrPg2_CheckBoxEntries::Writer,   &SvtFilterOptions::IsWinWord2Writer,
                        &SvtFilterOptions::SetWinWord2Writer },
        { MSFltrPg2_CheckBoxEntries::Writer,   &SvtFilterOptions::IsWriter2WinWord,
                        &SvtFilterOptions::SetWriter2WinWord },
        { MSFltrPg2_CheckBoxEntries::Calc,     &SvtFilterOptions::IsExcel2Calc,
                        &SvtFilterOptions::SetExcel2Calc },
        { MSFltrPg2_CheckBoxEntries::Calc,     &SvtFilterOptions::IsCalc2Excel,
                        &SvtFilterOptions::SetCalc2Excel },
        { MSFltrPg2_CheckBoxEntries::Impress,  &SvtFilterOptions::IsPowerPoint2Impress,
                        &SvtFilterOptions::SetPowerPoint2Impress },
        { MSFltrPg2_CheckBoxEntries::Impress,  &SvtFilterOptions::IsImpress2PowerPoint,
                        &SvtFilterOptions::SetImpress2PowerPoint },
        { MSFltrPg2_CheckBoxEntries::SmartArt,  &SvtFilterOptions::IsSmartArt2Shape,
                        &SvtFilterOptions::SetSmartArt2Shape },
    };

    bool bCheck, bFirst = true;
    for( const ChkCBoxEntries & rEntry : aChkArr )
    {
        sal_uInt16 nCol = bFirst ? 1 : 2;
        if (bFirst)
            bFirst = false;
        SvTreeListEntry* pEntry = GetEntry4Type( rEntry.eType );
        if( pEntry )
        {
            SvLBoxButton& rItem = static_cast<SvLBoxButton&>(pEntry->GetItem( nCol ));
            if (rItem.GetType() == SvLBoxItemType::Button)
            {
                SvItemStateFlags nButtonFlags = rItem.GetButtonFlags();
                bCheck = SvButtonState::Checked ==
                        SvLBoxButtonData::ConvertToButtonState( nButtonFlags );

                if( bCheck != (rOpt.*rEntry.FnIs)() )
                    (rOpt.*rEntry.FnSet)( bCheck );
            }
        }
    }

    if( aHighlightingRB->IsValueChangedFromSaved() )
    {
        if( aHighlightingRB->IsChecked() )
            rOpt.SetCharBackground2Highlighting();
        else
            rOpt.SetCharBackground2Shading();
    }

    return true;
}

void OfaMSFilterTabPage2::Reset( const SfxItemSet* )
{
    SvtFilterOptions& rOpt = SvtFilterOptions::Get();

    m_pCheckLB->SetUpdateMode(false);
    m_pCheckLB->Clear();

    SvtModuleOptions aModuleOpt;

    // int the same sequence as the enums of MSFltrPg2_CheckBoxEntries
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::MATH ) )
        InsertEntry( sChgToFromMath, MSFltrPg2_CheckBoxEntries::Math );
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::WRITER ) )
        InsertEntry( sChgToFromWriter, MSFltrPg2_CheckBoxEntries::Writer );
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::CALC ) )
        InsertEntry( sChgToFromCalc, MSFltrPg2_CheckBoxEntries::Calc );
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::IMPRESS ) )
        InsertEntry( sChgToFromImpress, MSFltrPg2_CheckBoxEntries::Impress );
    InsertEntry( sChgToFromSmartArt, MSFltrPg2_CheckBoxEntries::SmartArt, false );

    static struct ChkCBoxEntries{
        MSFltrPg2_CheckBoxEntries eType;
        bool (SvtFilterOptions:: *FnIs)() const;
    } const aChkArr[] = {
        { MSFltrPg2_CheckBoxEntries::Math,     &SvtFilterOptions::IsMathType2Math },
        { MSFltrPg2_CheckBoxEntries::Math,     &SvtFilterOptions::IsMath2MathType },
        { MSFltrPg2_CheckBoxEntries::Writer,   &SvtFilterOptions::IsWinWord2Writer },
        { MSFltrPg2_CheckBoxEntries::Writer,   &SvtFilterOptions::IsWriter2WinWord },
        { MSFltrPg2_CheckBoxEntries::Calc,     &SvtFilterOptions::IsExcel2Calc },
        { MSFltrPg2_CheckBoxEntries::Calc,     &SvtFilterOptions::IsCalc2Excel },
        { MSFltrPg2_CheckBoxEntries::Impress,  &SvtFilterOptions::IsPowerPoint2Impress },
        { MSFltrPg2_CheckBoxEntries::Impress,  &SvtFilterOptions::IsImpress2PowerPoint },
        { MSFltrPg2_CheckBoxEntries::SmartArt, &SvtFilterOptions::IsSmartArt2Shape },
    };

    bool bFirst = true;
    for( const ChkCBoxEntries & rArr : aChkArr )
    {
        sal_uInt16 nCol = bFirst ? 1 : 2;
        if (bFirst)
            bFirst = false;
        SvTreeListEntry* pEntry = GetEntry4Type( rArr.eType );
        if( pEntry )
        {
            SvLBoxButton& rItem = static_cast<SvLBoxButton&>(pEntry->GetItem( nCol ));
            if (rItem.GetType() == SvLBoxItemType::Button)
            {
                if( (rOpt.*rArr.FnIs)() )
                    rItem.SetStateChecked();
                else
                    rItem.SetStateUnchecked();
                m_pCheckLB->InvalidateEntry( pEntry );
            }
        }
    }
    m_pCheckLB->SetUpdateMode( true );

    if (rOpt.IsCharBackground2Highlighting())
        aHighlightingRB->Check();
    else
        aShadingRB->Check();

    aHighlightingRB->SaveValue();
}

void OfaMSFilterTabPage2::InsertEntry( const OUString& _rTxt, MSFltrPg2_CheckBoxEntries _nType )
{
    InsertEntry( _rTxt, _nType, true );
}

void OfaMSFilterTabPage2::InsertEntry( const OUString& _rTxt, MSFltrPg2_CheckBoxEntries _nType,
                                       bool saveEnabled )
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;

    if (!m_xCheckButtonData)
        m_xCheckButtonData.reset(new SvLBoxButtonData(m_pCheckLB));

    pEntry->AddItem(std::make_unique<SvLBoxContextBmp>(
        Image(), Image(), false));
    pEntry->AddItem(std::make_unique<SvLBoxButton>(
        SvLBoxButtonKind::EnabledCheckbox,
               m_xCheckButtonData.get()));
    pEntry->AddItem(std::make_unique<SvLBoxButton>(
        saveEnabled ? SvLBoxButtonKind::EnabledCheckbox
                                     : SvLBoxButtonKind::DisabledCheckbox,
               m_xCheckButtonData.get()));
    pEntry->AddItem(std::make_unique<SvLBoxString>(_rTxt));

    pEntry->SetUserData( reinterpret_cast<void*>(_nType) );
    m_pCheckLB->Insert( pEntry );
}

SvTreeListEntry* OfaMSFilterTabPage2::GetEntry4Type( MSFltrPg2_CheckBoxEntries _nType ) const
{
    SvTreeListEntry* pEntry = m_pCheckLB->First();
    while ( pEntry )
    {
        if ( _nType == static_cast<MSFltrPg2_CheckBoxEntries>( reinterpret_cast<sal_IntPtr>( pEntry->GetUserData() ) ) )
            return pEntry;
        pEntry = m_pCheckLB->Next( pEntry );
    }
    return nullptr;
}

void OfaMSFilterTabPage2::MSFltrSimpleTable::SetTabs()
{
    SvSimpleTable::SetTabs();
    SvLBoxTabFlags nAdjust = SvLBoxTabFlags::ADJUST_RIGHT|SvLBoxTabFlags::ADJUST_LEFT|SvLBoxTabFlags::ADJUST_CENTER|SvLBoxTabFlags::FORCE;

    if( aTabs.size() > 1 )
    {
        SvLBoxTab* pTab = aTabs[1].get();
        pTab->nFlags &= ~nAdjust;
        pTab->nFlags |= SvLBoxTabFlags::ADJUST_CENTER|SvLBoxTabFlags::FORCE;
    }
    if( aTabs.size() > 2 )
    {
        SvLBoxTab* pTab = aTabs[2].get();
        pTab->nFlags &= ~nAdjust;
        pTab->nFlags |= SvLBoxTabFlags::ADJUST_CENTER|SvLBoxTabFlags::FORCE;
    }
}

void OfaMSFilterTabPage2::MSFltrSimpleTable::HBarClick()
{
    // sorting is stopped by this override
}

void OfaMSFilterTabPage2::MSFltrSimpleTable::SetCheckButtonState(
                            SvTreeListEntry* pEntry, sal_uInt16 nCol, SvButtonState eState)
{
    SvLBoxButton& rItem = static_cast<SvLBoxButton&>(pEntry->GetItem(nCol + 1));

    if (rItem.GetType() == SvLBoxItemType::Button)
    {
        switch( eState )
        {
            case SvButtonState::Checked:
                rItem.SetStateChecked();
                break;

            case SvButtonState::Unchecked:
                rItem.SetStateUnchecked();
                break;

            case SvButtonState::Tristate:
                rItem.SetStateTristate();
                break;
        }
        InvalidateEntry( pEntry );
    }
}

SvButtonState OfaMSFilterTabPage2::MSFltrSimpleTable::GetCheckButtonState(
                                    SvTreeListEntry* pEntry, sal_uInt16 nCol )
{
    SvButtonState eState = SvButtonState::Unchecked;
    SvLBoxButton& rItem = static_cast<SvLBoxButton&>(pEntry->GetItem(nCol + 1));

    if (rItem.GetType() == SvLBoxItemType::Button)
    {
        SvItemStateFlags nButtonFlags = rItem.GetButtonFlags();
        eState = SvLBoxButtonData::ConvertToButtonState( nButtonFlags );
    }

    return eState;
}

void OfaMSFilterTabPage2::MSFltrSimpleTable::CheckEntryPos(sal_uLong nPos, sal_uInt16 nCol, bool bChecked)
{
    if ( nPos < GetEntryCount() )
        SetCheckButtonState(
            GetEntry(nPos),
            nCol,
            bChecked ? SvButtonState::Checked : SvButtonState::Unchecked );
}

void OfaMSFilterTabPage2::MSFltrSimpleTable::KeyInput( const KeyEvent& rKEvt )
{
    if(!rKEvt.GetKeyCode().GetModifier() &&
        KEY_SPACE == rKEvt.GetKeyCode().GetCode())
    {
        sal_uLong nSelPos = GetModel()->GetAbsPos(GetCurEntry());
        sal_uInt16 nCol = GetCurrentTabPos() - 1;
        if ( nCol < 2 )
        {
            SvTreeListEntry* pEntry = GetEntry( nSelPos );
            bool bIsChecked = ( GetCheckButtonState( pEntry, nCol ) == SvButtonState::Checked );
            CheckEntryPos( nSelPos, nCol, !bIsChecked );
            CallImplEventListeners( VclEventId::CheckboxToggle, static_cast<void*>(pEntry) );
        }
        else
        {
            sal_uInt16 nCheck = GetCheckButtonState( GetEntry(nSelPos), 1 ) == SvButtonState::Checked ? 1 : 0;
            if(GetCheckButtonState( GetEntry(nSelPos), 0 ) != SvButtonState::Unchecked)
                nCheck += 2;
            nCheck--;
            nCheck &= 3;
            CheckEntryPos(nSelPos, 1, 0 != (nCheck & 1));
            CheckEntryPos(nSelPos, 0, 0 != (nCheck & 2));
        }
    }
    else
        SvSimpleTable::KeyInput(rKEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
