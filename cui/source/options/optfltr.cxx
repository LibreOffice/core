/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// include ---------------------------------------------------------------
#include <unotools/moduleoptions.hxx>
#include <unotools/fltrcfg.hxx>
#include "optfltr.hxx"
#include "optfltr.hrc"
#include <cuires.hrc>
#include "helpid.hrc"
#include <dialmgr.hxx>

enum MSFltrPg2_CheckBoxEntries {
    Math,
    Writer,
    Calc,
    Impress,
    InvalidCBEntry
};

#define CBCOL_FIRST     0
#define CBCOL_SECOND    1

// -----------------------------------------------------------------------

OfaMSFilterTabPage::OfaMSFilterTabPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage( pParent, CUI_RES( RID_OFAPAGE_MSFILTEROPT ), rSet ),
    aMSWordGB       ( this, CUI_RES( GB_WORD        ) ),
    aWBasicCodeCB   ( this, CUI_RES( CB_WBAS_CODE ) ),
    aWBasicWbctblCB ( this, CUI_RES( CB_WBAS_WBCTBL ) ),
    aWBasicStgCB    ( this, CUI_RES( CB_WBAS_STG  ) ),
    aMSExcelGB      ( this, CUI_RES( GB_EXCEL     ) ),
    aEBasicCodeCB   ( this, CUI_RES( CB_EBAS_CODE ) ),
    aEBasicExectblCB( this, CUI_RES( CB_EBAS_EXECTBL ) ),
    aEBasicStgCB    ( this, CUI_RES( CB_EBAS_STG  ) ),
    aMSPPointGB     ( this, CUI_RES( GB_PPOINT    ) ),
    aPBasicCodeCB   ( this, CUI_RES( CB_PBAS_CODE ) ),
    aPBasicStgCB    ( this, CUI_RES( CB_PBAS_STG  ) )
{
    FreeResource();

    aWBasicCodeCB.SetClickHdl( LINK( this, OfaMSFilterTabPage, LoadWordBasicCheckHdl_Impl ) );
    aEBasicCodeCB.SetClickHdl( LINK( this, OfaMSFilterTabPage, LoadExcelBasicCheckHdl_Impl ) );
}

OfaMSFilterTabPage::~OfaMSFilterTabPage()
{
}

IMPL_LINK( OfaMSFilterTabPage, LoadWordBasicCheckHdl_Impl, CheckBox*, EMPTYARG )
{
    aWBasicWbctblCB.Enable( aWBasicCodeCB.IsChecked() );
    return 0;
}

IMPL_LINK( OfaMSFilterTabPage, LoadExcelBasicCheckHdl_Impl, CheckBox*, EMPTYARG )
{
    aEBasicExectblCB.Enable( aEBasicCodeCB.IsChecked() );
    return 0;
}

SfxTabPage* OfaMSFilterTabPage::Create( Window* pParent,
                                        const SfxItemSet& rAttrSet )
{
    return new OfaMSFilterTabPage( pParent, rAttrSet );
}

BOOL OfaMSFilterTabPage::FillItemSet( SfxItemSet& )
{
    SvtFilterOptions* pOpt = SvtFilterOptions::Get();

    BOOL bFlag;
    if( aWBasicCodeCB.GetSavedValue() != (bFlag = aWBasicCodeCB.IsChecked()))
        pOpt->SetLoadWordBasicCode( bFlag );
    if( aWBasicWbctblCB.GetSavedValue() != (bFlag = aWBasicWbctblCB.IsChecked()))
        pOpt->SetLoadWordBasicExecutable( bFlag );
    if( aWBasicStgCB.GetSavedValue() != (bFlag = aWBasicStgCB.IsChecked()))
        pOpt->SetLoadWordBasicStorage( bFlag );

    if( aEBasicCodeCB.GetSavedValue() != (bFlag = aEBasicCodeCB.IsChecked()))
        pOpt->SetLoadExcelBasicCode( bFlag );
    if( aEBasicExectblCB.GetSavedValue() != (bFlag = aEBasicExectblCB.IsChecked()))
        pOpt->SetLoadExcelBasicExecutable( bFlag );
    if( aEBasicStgCB.GetSavedValue() != (bFlag = aEBasicStgCB.IsChecked()))
        pOpt->SetLoadExcelBasicStorage( bFlag );

    if( aPBasicCodeCB.GetSavedValue() != (bFlag = aPBasicCodeCB.IsChecked()))
        pOpt->SetLoadPPointBasicCode( bFlag );
    if( aPBasicStgCB.GetSavedValue() != (bFlag = aPBasicStgCB.IsChecked()))
        pOpt->SetLoadPPointBasicStorage( bFlag );

    return FALSE;
}

void OfaMSFilterTabPage::Reset( const SfxItemSet& )
{
    SvtFilterOptions* pOpt = SvtFilterOptions::Get();

    aWBasicCodeCB.Check( pOpt->IsLoadWordBasicCode() );
    aWBasicCodeCB.SaveValue();
    aWBasicWbctblCB.Check( pOpt->IsLoadWordBasicExecutable() );
    aWBasicWbctblCB.SaveValue();
    aWBasicStgCB.Check( pOpt->IsLoadWordBasicStorage() );
    aWBasicStgCB.SaveValue();
    LoadWordBasicCheckHdl_Impl( &aWBasicCodeCB );

    aEBasicCodeCB.Check( pOpt->IsLoadExcelBasicCode() );
    aEBasicCodeCB.SaveValue();
    aEBasicExectblCB.Check( pOpt->IsLoadExcelBasicExecutable() );
    aEBasicExectblCB.SaveValue();
    aEBasicStgCB.Check( pOpt->IsLoadExcelBasicStorage() );
    aEBasicStgCB.SaveValue();
    LoadExcelBasicCheckHdl_Impl( &aEBasicCodeCB );

    aPBasicCodeCB.Check( pOpt->IsLoadPPointBasicCode() );
    aPBasicCodeCB.SaveValue();
    aPBasicStgCB.Check( pOpt->IsLoadPPointBasicStorage() );
    aPBasicStgCB.SaveValue();

}

OfaMSFilterTabPage2::OfaMSFilterTabPage2( Window* pParent,
                                        const SfxItemSet& rSet )
    : SfxTabPage( pParent, CUI_RES( RID_OFAPAGE_MSFILTEROPT2 ), rSet ),
    aCheckLB            ( this, CUI_RES( CLB_SETTINGS   )),
    aHeader1FT          ( this, CUI_RES( FT_HEADER1_EXPLANATION )),
    aHeader2FT          ( this, CUI_RES( FT_HEADER2_EXPLANATION )),
    sHeader1            ( CUI_RES( ST_HEADER1 )),
    sHeader2            ( CUI_RES( ST_HEADER2 )),
    sChgToFromMath      ( CUI_RES( ST_CHG_MATH  )),
    sChgToFromWriter    ( CUI_RES( ST_CHG_WRITER )),
    sChgToFromCalc      ( CUI_RES( ST_CHG_CALC )),
    sChgToFromImpress   ( CUI_RES( ST_CHG_IMPRESS )),
    pCheckButtonData(0)
{
    FreeResource();

    static long aStaticTabs[] = { 3, 0, 20, 40 };
    aCheckLB.SvxSimpleTable::SetTabs( aStaticTabs );

    String sHeader( sHeader1 );
    (( sHeader += '\t' ) += sHeader2 ) += '\t';
    aCheckLB.InsertHeaderEntry( sHeader, HEADERBAR_APPEND,
                    HIB_CENTER | HIB_VCENTER | HIB_FIXEDPOS | HIB_FIXED );

    aCheckLB.SetHelpId( HID_OFAPAGE_MSFLTR2_CLB );
    aCheckLB.SetWindowBits( WB_HSCROLL| WB_VSCROLL );
}

OfaMSFilterTabPage2::~OfaMSFilterTabPage2()
{
    delete pCheckButtonData;
}

SfxTabPage* OfaMSFilterTabPage2::Create( Window* pParent,
                                const SfxItemSet& rAttrSet )
{
    return new OfaMSFilterTabPage2( pParent, rAttrSet );
}

BOOL OfaMSFilterTabPage2::FillItemSet( SfxItemSet& )
{
    SvtFilterOptions* pOpt = SvtFilterOptions::Get();

    static struct ChkCBoxEntries{
        MSFltrPg2_CheckBoxEntries eType;
        BOOL (SvtFilterOptions:: *FnIs)() const;
        void (SvtFilterOptions:: *FnSet)( BOOL bFlag );
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
        { InvalidCBEntry, 0, 0 }
    };

    BOOL bCheck, bFirst = TRUE;
    for( const ChkCBoxEntries* pArr = aChkArr;
            InvalidCBEntry != pArr->eType; ++pArr, bFirst = !bFirst )
    {
        USHORT nCol = bFirst ? 1 : 2;
        SvLBoxEntry* pEntry = GetEntry4Type( pArr->eType );
        if( pEntry )
        {
            SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetItem( nCol ));
            if( pItem && ((SvLBoxItem*)pItem)->IsA() == SV_ITEM_ID_LBOXBUTTON )
            {
                USHORT nButtonFlags = pItem->GetButtonFlags();
                bCheck = SV_BUTTON_CHECKED ==
                        pCheckButtonData->ConvertToButtonState( nButtonFlags );

                if( bCheck != (pOpt->*pArr->FnIs)() )
                    (pOpt->*pArr->FnSet)( bCheck );
            }
        }
    }

    return TRUE;
}

void OfaMSFilterTabPage2::Reset( const SfxItemSet& )
{
    SvtFilterOptions* pOpt = SvtFilterOptions::Get();

    aCheckLB.SetUpdateMode(FALSE);
    aCheckLB.Clear();

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

    static struct ChkCBoxEntries{
        MSFltrPg2_CheckBoxEntries eType;
        BOOL (SvtFilterOptions:: *FnIs)() const;
    } aChkArr[] = {
        { Math,     &SvtFilterOptions::IsMathType2Math },
        { Math,     &SvtFilterOptions::IsMath2MathType },
        { Writer,   &SvtFilterOptions::IsWinWord2Writer },
        { Writer,   &SvtFilterOptions::IsWriter2WinWord },
        { Calc,     &SvtFilterOptions::IsExcel2Calc },
        { Calc,     &SvtFilterOptions::IsCalc2Excel },
        { Impress,  &SvtFilterOptions::IsPowerPoint2Impress },
        { Impress,  &SvtFilterOptions::IsImpress2PowerPoint },
        { InvalidCBEntry, NULL }
    };

    BOOL bFirst = TRUE;
    for( const ChkCBoxEntries* pArr = aChkArr;
            InvalidCBEntry != pArr->eType; ++pArr, bFirst = !bFirst )
    {
        USHORT nCol = bFirst ? 1 : 2;
        SvLBoxEntry* pEntry = GetEntry4Type( static_cast< sal_IntPtr >( pArr->eType ) );
        if( pEntry )
        {
            SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetItem( nCol ));
            if( pItem && ((SvLBoxItem*)pItem)->IsA() == SV_ITEM_ID_LBOXBUTTON )
            {
                if( (pOpt->*pArr->FnIs)() )
                    pItem->SetStateChecked();
                else
                    pItem->SetStateUnchecked();
                aCheckLB.InvalidateEntry( pEntry );
            }
        }
    }
    aCheckLB.SetUpdateMode( TRUE );
}

void OfaMSFilterTabPage2::InsertEntry( const String& _rTxt, sal_IntPtr _nType )
{
    SvLBoxEntry* pEntry = new SvLBoxEntry;

    if( !pCheckButtonData )
        pCheckButtonData = new SvLBoxButtonData( &aCheckLB );

    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));
    pEntry->AddItem( new SvLBoxButton( pEntry, SvLBoxButtonKind_enabledCheckbox,
                                       0, pCheckButtonData ) );
    pEntry->AddItem( new SvLBoxButton( pEntry, SvLBoxButtonKind_enabledCheckbox,
                                       0, pCheckButtonData ) );
    pEntry->AddItem( new SvLBoxString( pEntry, 0, _rTxt ) );

    pEntry->SetUserData( (void*)_nType );
    aCheckLB.Insert( pEntry );
}

SvLBoxEntry* OfaMSFilterTabPage2::GetEntry4Type( sal_IntPtr _nType ) const
{
    SvLBoxEntry* pEntry = aCheckLB.First();
    while ( pEntry )
    {
        if ( _nType == sal_IntPtr( pEntry->GetUserData() ) )
            return pEntry;
        pEntry = aCheckLB.Next( pEntry );
    }
    return NULL;
}

void OfaMSFilterTabPage2::MSFltrSimpleTable::SetTabs()
{
    SvxSimpleTable::SetTabs();
    USHORT nAdjust = SV_LBOXTAB_ADJUST_RIGHT|SV_LBOXTAB_ADJUST_LEFT|SV_LBOXTAB_ADJUST_CENTER|SV_LBOXTAB_ADJUST_NUMERIC|SV_LBOXTAB_FORCE;

    if( aTabs.Count() > 1 )
    {
        SvLBoxTab* pTab = (SvLBoxTab*)aTabs.GetObject(1);
        pTab->nFlags &= ~nAdjust;
        pTab->nFlags |= SV_LBOXTAB_PUSHABLE|SV_LBOXTAB_ADJUST_CENTER|SV_LBOXTAB_FORCE;
    }
    if( aTabs.Count() > 2 )
    {
        SvLBoxTab* pTab = (SvLBoxTab*)aTabs.GetObject(2);
        pTab->nFlags &= ~nAdjust;
        pTab->nFlags |= SV_LBOXTAB_PUSHABLE|SV_LBOXTAB_ADJUST_CENTER|SV_LBOXTAB_FORCE;
    }
}

void OfaMSFilterTabPage2::MSFltrSimpleTable::HBarClick()
{
    // Sortierung durch diese Ueberladung abgeklemmt
}

void OfaMSFilterTabPage2::MSFltrSimpleTable::SetCheckButtonState(
                            SvLBoxEntry* pEntry, USHORT nCol, SvButtonState eState)
{
    SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetItem(nCol + 1));

    DBG_ASSERT(pItem,"SetCheckButton:Item not found");
    if (((SvLBoxItem*)pItem)->IsA() == SV_ITEM_ID_LBOXBUTTON)
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
                                    SvLBoxEntry* pEntry, USHORT nCol ) const
{
    SvButtonState eState = SV_BUTTON_UNCHECKED;
    SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetItem(nCol + 1));
    DBG_ASSERT(pItem,"GetChButnState:Item not found");

    if (((SvLBoxItem*)pItem)->IsA() == SV_ITEM_ID_LBOXBUTTON)
    {
        USHORT nButtonFlags = pItem->GetButtonFlags();
        eState = pCheckButtonData->ConvertToButtonState( nButtonFlags );
    }

    return eState;
}

void OfaMSFilterTabPage2::MSFltrSimpleTable::CheckEntryPos(ULONG nPos, USHORT nCol, BOOL bChecked)
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
        ULONG nSelPos = GetModel()->GetAbsPos(GetCurEntry());
        USHORT nCol = GetCurrentTabPos() - 1;
        if ( nCol < 2 )
        {
            SvLBoxEntry* pEntry = GetEntry( nSelPos );
            sal_Bool bIsChecked = ( GetCheckButtonState( pEntry, nCol ) == SV_BUTTON_CHECKED );
            CheckEntryPos( nSelPos, nCol, !bIsChecked );
            CallImplEventListeners( VCLEVENT_CHECKBOX_TOGGLE, (void*)pEntry );
        }
        else
        {
            USHORT nCheck = GetCheckButtonState( GetEntry(nSelPos), 1 ) == SV_BUTTON_CHECKED ? 1 : 0;
            if(GetCheckButtonState( GetEntry(nSelPos), 0 ))
                nCheck += 2;
            nCheck--;
            nCheck &= 3;
            CheckEntryPos(nSelPos, 1, 0 != (nCheck & 1));
            CheckEntryPos(nSelPos, 0, 0 != (nCheck & 2));
        }
    }
    else
        SvxSimpleTable::KeyInput(rKEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
