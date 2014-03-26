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
#include <cuires.hrc>
#include "helpid.hrc"
#include <dialmgr.hxx>

#include "svtools/svlbitm.hxx"
#include "svtools/treelistentry.hxx"

enum MSFltrPg2_CheckBoxEntries {
    Math,
    Writer,
    Calc,
    Impress,
    SmartArt,
    InvalidCBEntry
};



OfaMSFilterTabPage::OfaMSFilterTabPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage( pParent, "OptFltrPage", "cui/ui/optfltrpage.ui", rSet )
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
}

IMPL_LINK_NOARG(OfaMSFilterTabPage, LoadWordBasicCheckHdl_Impl)
{
    aWBasicWbctblCB->Enable( aWBasicCodeCB->IsChecked() );
    return 0;
}

IMPL_LINK_NOARG(OfaMSFilterTabPage, LoadExcelBasicCheckHdl_Impl)
{
    aEBasicExectblCB->Enable( aEBasicCodeCB->IsChecked() );
    return 0;
}

SfxTabPage* OfaMSFilterTabPage::Create( Window* pParent,
                                        const SfxItemSet& rAttrSet )
{
    return new OfaMSFilterTabPage( pParent, rAttrSet );
}

bool OfaMSFilterTabPage::FillItemSet( SfxItemSet& )
{
    SvtFilterOptions& rOpt = SvtFilterOptions::Get();

    sal_Bool bFlag;
    if( aWBasicCodeCB->GetSavedValue() != (bFlag = aWBasicCodeCB->IsChecked()))
        rOpt.SetLoadWordBasicCode( bFlag );
    if( aWBasicWbctblCB->GetSavedValue() != (bFlag = aWBasicWbctblCB->IsChecked()))
        rOpt.SetLoadWordBasicExecutable( bFlag );
    if( aWBasicStgCB->GetSavedValue() != (bFlag = aWBasicStgCB->IsChecked()))
        rOpt.SetLoadWordBasicStorage( bFlag );

    if( aEBasicCodeCB->GetSavedValue() != (bFlag = aEBasicCodeCB->IsChecked()))
        rOpt.SetLoadExcelBasicCode( bFlag );
    if( aEBasicExectblCB->GetSavedValue() != (bFlag = aEBasicExectblCB->IsChecked()))
        rOpt.SetLoadExcelBasicExecutable( bFlag );
    if( aEBasicStgCB->GetSavedValue() != (bFlag = aEBasicStgCB->IsChecked()))
        rOpt.SetLoadExcelBasicStorage( bFlag );

    if( aPBasicCodeCB->GetSavedValue() != (bFlag = aPBasicCodeCB->IsChecked()))
        rOpt.SetLoadPPointBasicCode( bFlag );
    if( aPBasicStgCB->GetSavedValue() != (bFlag = aPBasicStgCB->IsChecked()))
        rOpt.SetLoadPPointBasicStorage( bFlag );

    return false;
}

void OfaMSFilterTabPage::Reset( const SfxItemSet& )
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

OfaMSFilterTabPage2::OfaMSFilterTabPage2( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage( pParent, "OptFilterPage", "cui/ui/optfltrembedpage.ui", rSet ),
    sHeader1(CUI_RES(RID_SVXSTR_HEADER1)),
    sHeader2(CUI_RES(RID_SVXSTR_HEADER2)),
    sChgToFromMath(CUI_RES(RID_SVXSTR_CHG_MATH)),
    sChgToFromWriter(CUI_RES(RID_SVXSTR_CHG_WRITER)),
    sChgToFromCalc(CUI_RES(RID_SVXSTR_CHG_CALC)),
    sChgToFromImpress(CUI_RES(RID_SVXSTR_CHG_IMPRESS)),
    sChgToFromSmartArt(CUI_RES(RID_SVXSTR_CHG_SMARTART)),
    pCheckButtonData(0)
{
    get(m_pCheckLBContainer, "checklbcontainer");
    Size aControlSize(248, 55);
    aControlSize = LogicToPixel(aControlSize, MAP_APPFONT);
    m_pCheckLBContainer->set_width_request(aControlSize.Width());
    m_pCheckLBContainer->set_height_request(aControlSize.Height());

    m_pCheckLB = new MSFltrSimpleTable(*m_pCheckLBContainer);

    static long aStaticTabs[] = { 3, 0, 20, 40 };
    m_pCheckLB->SvSimpleTable::SetTabs( aStaticTabs );

    OUString sHeader = sHeader1 + "\t" + sHeader2 + "\t";
    m_pCheckLB->InsertHeaderEntry( sHeader, HEADERBAR_APPEND,
                    HIB_CENTER | HIB_VCENTER | HIB_FIXEDPOS | HIB_FIXED );

    m_pCheckLB->SetStyle( m_pCheckLB->GetStyle()|WB_HSCROLL| WB_VSCROLL );
}

OfaMSFilterTabPage2::~OfaMSFilterTabPage2()
{
    delete pCheckButtonData;
    delete m_pCheckLB;
}

SfxTabPage* OfaMSFilterTabPage2::Create( Window* pParent,
                                const SfxItemSet& rAttrSet )
{
    return new OfaMSFilterTabPage2( pParent, rAttrSet );
}

bool OfaMSFilterTabPage2::FillItemSet( SfxItemSet& )
{
    SvtFilterOptions& rOpt = SvtFilterOptions::Get();

    static struct ChkCBoxEntries{
        MSFltrPg2_CheckBoxEntries eType;
        bool (SvtFilterOptions:: *FnIs)() const;
        void (SvtFilterOptions:: *FnSet)( bool bFlag );
    } aChkArr[] = {
        { Math,     &SvtFilterOptions::IsMathType2Math,
                        &SvtFilterOptions::SetMathType2Math },
        { Math,     &SvtFilterOptions::IsMath2MathType,
                        &SvtFilterOptions::SetMath2MathType },
        { Writer,   &SvtFilterOptions::IsWinWord2Writer,
                        &SvtFilterOptions::SetWinWord2Writer },
        { Writer,   &SvtFilterOptions::IsWriter2WinWord,
                        &SvtFilterOptions::SetWriter2WinWord },
        { Calc,     &SvtFilterOptions::IsExcel2Calc,
                        &SvtFilterOptions::SetExcel2Calc },
        { Calc,     &SvtFilterOptions::IsCalc2Excel,
                        &SvtFilterOptions::SetCalc2Excel },
        { Impress,  &SvtFilterOptions::IsPowerPoint2Impress,
                        &SvtFilterOptions::SetPowerPoint2Impress },
        { Impress,  &SvtFilterOptions::IsImpress2PowerPoint,
                        &SvtFilterOptions::SetImpress2PowerPoint },
        { SmartArt,  &SvtFilterOptions::IsSmartArt2Shape,
                        &SvtFilterOptions::SetSmartArt2Shape },
        { InvalidCBEntry, 0, 0 }
    };

    bool bCheck, bFirst = true;
    for( const ChkCBoxEntries* pArr = aChkArr;
            InvalidCBEntry != pArr->eType; ++pArr, bFirst = !bFirst )
    {
        sal_uInt16 nCol = bFirst ? 1 : 2;
        SvTreeListEntry* pEntry = GetEntry4Type( pArr->eType );
        if( pEntry )
        {
            SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetItem( nCol ));
            if (pItem && pItem->GetType() == SV_ITEM_ID_LBOXBUTTON)
            {
                sal_uInt16 nButtonFlags = pItem->GetButtonFlags();
                bCheck = SV_BUTTON_CHECKED ==
                        pCheckButtonData->ConvertToButtonState( nButtonFlags );

                if( bCheck != (rOpt.*pArr->FnIs)() )
                    (rOpt.*pArr->FnSet)( bCheck );
            }
        }
    }

    return true;
}

void OfaMSFilterTabPage2::Reset( const SfxItemSet& )
{
    SvtFilterOptions& rOpt = SvtFilterOptions::Get();

    m_pCheckLB->SetUpdateMode(false);
    m_pCheckLB->Clear();

    SvtModuleOptions aModuleOpt;

    // int the same sequence as the enums of MSFltrPg2_CheckBoxEntries
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SMATH ) )
        InsertEntry( sChgToFromMath, static_cast< sal_IntPtr >( Math ) );
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
        InsertEntry( sChgToFromWriter, static_cast< sal_IntPtr >( Writer ) );
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
        InsertEntry( sChgToFromCalc, static_cast< sal_IntPtr >( Calc ) );
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
        InsertEntry( sChgToFromImpress, static_cast< sal_IntPtr >( Impress ) );
    InsertEntry( sChgToFromSmartArt, static_cast< sal_IntPtr >( SmartArt ), true, false );

    static struct ChkCBoxEntries{
        MSFltrPg2_CheckBoxEntries eType;
        bool (SvtFilterOptions:: *FnIs)() const;
    } aChkArr[] = {
        { Math,     &SvtFilterOptions::IsMathType2Math },
        { Math,     &SvtFilterOptions::IsMath2MathType },
        { Writer,   &SvtFilterOptions::IsWinWord2Writer },
        { Writer,   &SvtFilterOptions::IsWriter2WinWord },
        { Calc,     &SvtFilterOptions::IsExcel2Calc },
        { Calc,     &SvtFilterOptions::IsCalc2Excel },
        { Impress,  &SvtFilterOptions::IsPowerPoint2Impress },
        { Impress,  &SvtFilterOptions::IsImpress2PowerPoint },
        { SmartArt, &SvtFilterOptions::IsSmartArt2Shape },
        { InvalidCBEntry, NULL }
    };

    sal_Bool bFirst = sal_True;
    for( const ChkCBoxEntries* pArr = aChkArr;
            InvalidCBEntry != pArr->eType; ++pArr, bFirst = !bFirst )
    {
        sal_uInt16 nCol = bFirst ? 1 : 2;
        SvTreeListEntry* pEntry = GetEntry4Type( static_cast< sal_IntPtr >( pArr->eType ) );
        if( pEntry )
        {
            SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetItem( nCol ));
            if (pItem && pItem->GetType() == SV_ITEM_ID_LBOXBUTTON)
            {
                if( (rOpt.*pArr->FnIs)() )
                    pItem->SetStateChecked();
                else
                    pItem->SetStateUnchecked();
                m_pCheckLB->InvalidateEntry( pEntry );
            }
        }
    }
    m_pCheckLB->SetUpdateMode( true );
}

void OfaMSFilterTabPage2::InsertEntry( const OUString& _rTxt, sal_IntPtr _nType )
{
    InsertEntry( _rTxt, _nType, true, true );
}

void OfaMSFilterTabPage2::InsertEntry( const OUString& _rTxt, sal_IntPtr _nType,
                                       sal_Bool loadEnabled, sal_Bool saveEnabled )
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;

    if( !pCheckButtonData )
        pCheckButtonData = new SvLBoxButtonData( m_pCheckLB );

    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), false));
    pEntry->AddItem( new SvLBoxButton( pEntry, loadEnabled? SvLBoxButtonKind_enabledCheckbox :
                                                            SvLBoxButtonKind_disabledCheckbox,
                                       0, pCheckButtonData ) );
    pEntry->AddItem( new SvLBoxButton( pEntry, saveEnabled? SvLBoxButtonKind_enabledCheckbox :
                                                            SvLBoxButtonKind_disabledCheckbox,
                                       0, pCheckButtonData ) );
    pEntry->AddItem( new SvLBoxString( pEntry, 0, _rTxt ) );

    pEntry->SetUserData( (void*)_nType );
    m_pCheckLB->Insert( pEntry );
}

SvTreeListEntry* OfaMSFilterTabPage2::GetEntry4Type( sal_IntPtr _nType ) const
{
    SvTreeListEntry* pEntry = m_pCheckLB->First();
    while ( pEntry )
    {
        if ( _nType == sal_IntPtr( pEntry->GetUserData() ) )
            return pEntry;
        pEntry = m_pCheckLB->Next( pEntry );
    }
    return NULL;
}

void OfaMSFilterTabPage2::MSFltrSimpleTable::SetTabs()
{
    SvSimpleTable::SetTabs();
    sal_uInt16 nAdjust = SV_LBOXTAB_ADJUST_RIGHT|SV_LBOXTAB_ADJUST_LEFT|SV_LBOXTAB_ADJUST_CENTER|SV_LBOXTAB_ADJUST_NUMERIC|SV_LBOXTAB_FORCE;

    if( aTabs.size() > 1 )
    {
        SvLBoxTab* pTab = aTabs[1];
        pTab->nFlags &= ~nAdjust;
        pTab->nFlags |= SV_LBOXTAB_PUSHABLE|SV_LBOXTAB_ADJUST_CENTER|SV_LBOXTAB_FORCE;
    }
    if( aTabs.size() > 2 )
    {
        SvLBoxTab* pTab = aTabs[2];
        pTab->nFlags &= ~nAdjust;
        pTab->nFlags |= SV_LBOXTAB_PUSHABLE|SV_LBOXTAB_ADJUST_CENTER|SV_LBOXTAB_FORCE;
    }
}

void OfaMSFilterTabPage2::MSFltrSimpleTable::HBarClick()
{
    // sorting is stopped by this overloading
}

void OfaMSFilterTabPage2::MSFltrSimpleTable::SetCheckButtonState(
                            SvTreeListEntry* pEntry, sal_uInt16 nCol, SvButtonState eState)
{
    SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetItem(nCol + 1));

    DBG_ASSERT(pItem,"SetCheckButton:Item not found");
    if (pItem->GetType() == SV_ITEM_ID_LBOXBUTTON)
    {
        switch( eState )
        {
            case SV_BUTTON_CHECKED:
                pItem->SetStateChecked();
                break;

            case SV_BUTTON_UNCHECKED:
                pItem->SetStateUnchecked();
                break;

            case SV_BUTTON_TRISTATE:
                pItem->SetStateTristate();
                break;
        }
        InvalidateEntry( pEntry );
    }
}

SvButtonState OfaMSFilterTabPage2::MSFltrSimpleTable::GetCheckButtonState(
                                    SvTreeListEntry* pEntry, sal_uInt16 nCol ) const
{
    SvButtonState eState = SV_BUTTON_UNCHECKED;
    SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetItem(nCol + 1));
    DBG_ASSERT(pItem,"GetChButnState:Item not found");

    if (pItem->GetType() == SV_ITEM_ID_LBOXBUTTON)
    {
        sal_uInt16 nButtonFlags = pItem->GetButtonFlags();
        eState = pCheckButtonData->ConvertToButtonState( nButtonFlags );
    }

    return eState;
}

void OfaMSFilterTabPage2::MSFltrSimpleTable::CheckEntryPos(sal_uLong nPos, sal_uInt16 nCol, sal_Bool bChecked)
{
    if ( nPos < GetEntryCount() )
        SetCheckButtonState(
            GetEntry(nPos),
            nCol,
            bChecked ? SvButtonState( SV_BUTTON_CHECKED ) :
                                       SvButtonState( SV_BUTTON_UNCHECKED ) );
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
            sal_Bool bIsChecked = ( GetCheckButtonState( pEntry, nCol ) == SV_BUTTON_CHECKED );
            CheckEntryPos( nSelPos, nCol, !bIsChecked );
            CallImplEventListeners( VCLEVENT_CHECKBOX_TOGGLE, (void*)pEntry );
        }
        else
        {
            sal_uInt16 nCheck = GetCheckButtonState( GetEntry(nSelPos), 1 ) == SV_BUTTON_CHECKED ? 1 : 0;
            if(GetCheckButtonState( GetEntry(nSelPos), 0 ))
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
