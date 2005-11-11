/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optfltr.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-11 13:54:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

// include ---------------------------------------------------------------

#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif

#include <svtools/fltrcfg.hxx>
#include "optfltr.hxx"
#include "optfltr.hrc"
#include "dialogs.hrc"
#include "helpid.hrc"
#include "dialmgr.hxx"

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
    : SfxTabPage( pParent, ResId( RID_OFAPAGE_MSFILTEROPT, DIALOG_MGR() ), rSet ),
    aMSWordGB       ( this, ResId( GB_WORD      ) ),
    aWBasicCodeCB   ( this, ResId( CB_WBAS_CODE ) ),
    aWBasicStgCB    ( this, ResId( CB_WBAS_STG  ) ),
    aMSExcelGB      ( this, ResId( GB_EXCEL     ) ),
    aEBasicCodeCB   ( this, ResId( CB_EBAS_CODE ) ),
    aEBasicStgCB    ( this, ResId( CB_EBAS_STG  ) ),
    aMSPPointGB     ( this, ResId( GB_PPOINT    ) ),
    aPBasicCodeCB   ( this, ResId( CB_PBAS_CODE ) ),
    aPBasicStgCB    ( this, ResId( CB_PBAS_STG  ) )
{
    FreeResource();
}

OfaMSFilterTabPage::~OfaMSFilterTabPage()
{
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
    if( aWBasicStgCB.GetSavedValue() != (bFlag = aWBasicStgCB.IsChecked()))
        pOpt->SetLoadWordBasicStorage( bFlag );

    if( aEBasicCodeCB.GetSavedValue() != (bFlag = aEBasicCodeCB.IsChecked()))
        pOpt->SetLoadExcelBasicCode( bFlag );
    if( aEBasicStgCB.GetSavedValue() != (bFlag = aEBasicStgCB.IsChecked()))
        pOpt->SetLoadExcelBasicStorage( bFlag );

    if( aPBasicCodeCB.GetSavedValue() != (bFlag = aPBasicCodeCB.IsChecked()))
        pOpt->SetLoadPPointBasicCode( bFlag );
    if( aPBasicStgCB.GetSavedValue() != (bFlag = aPBasicStgCB.IsChecked()))
        pOpt->SetLoadPPointBasicStorage( bFlag );

    return FALSE;
}

/*-----------------02.09.96 13.47-------------------

--------------------------------------------------*/

void OfaMSFilterTabPage::Reset( const SfxItemSet& )
{
    SvtFilterOptions* pOpt = SvtFilterOptions::Get();

    aWBasicCodeCB.Check( pOpt->IsLoadWordBasicCode() );
    aWBasicCodeCB.SaveValue();
    aWBasicStgCB.Check( pOpt->IsLoadWordBasicStorage() );
    aWBasicStgCB.SaveValue();

    aEBasicCodeCB.Check( pOpt->IsLoadExcelBasicCode() );
    aEBasicCodeCB.SaveValue();
    aEBasicStgCB.Check( pOpt->IsLoadExcelBasicStorage() );
    aEBasicStgCB.SaveValue();

    aPBasicCodeCB.Check( pOpt->IsLoadPPointBasicCode() );
    aPBasicCodeCB.SaveValue();
    aPBasicStgCB.Check( pOpt->IsLoadPPointBasicStorage() );
    aPBasicStgCB.SaveValue();

}

/*-----------------29.06.00 13:22-------------------
 *
 * --------------------------------------------------*/
OfaMSFilterTabPage2::OfaMSFilterTabPage2( Window* pParent,
                                        const SfxItemSet& rSet )
    : SfxTabPage( pParent, ResId( RID_OFAPAGE_MSFILTEROPT2, DIALOG_MGR() ), rSet ),
    aCheckLB            ( this, ResId( CLB_SETTINGS )),
    aHeader1FT          ( this, ResId( FT_HEADER1_EXPLANATION )),
    aHeader2FT          ( this, ResId( FT_HEADER2_EXPLANATION )),
    sHeader1            ( ResId( ST_HEADER1 )),
    sHeader2            ( ResId( ST_HEADER2 )),
    sChgToFromMath      ( ResId( ST_CHG_MATH    )),
    sChgToFromWriter    ( ResId( ST_CHG_WRITER )),
    sChgToFromCalc      ( ResId( ST_CHG_CALC )),
    sChgToFromImpress   ( ResId( ST_CHG_IMPRESS )),
    aChkunBmp           ( ResId( CHKBUT_UNCHECKED )),
    aChkchBmp           ( ResId( CHKBUT_CHECKED )),
    aChkchhiBmp         ( ResId( CHKBUT_HICHECKED )),
    aChkunhiBmp         ( ResId( CHKBUT_HIUNCHECKED )),
    aChktriBmp          ( ResId( CHKBUT_TRISTATE    )),
    aChktrihiBmp        ( ResId( CHKBUT_HITRISTATE )),
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

BOOL OfaMSFilterTabPage2::FillItemSet( SfxItemSet& rSet )
{
    BOOL bModified = FALSE;
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
                {
                    bModified = TRUE;
                    (pOpt->*pArr->FnSet)( bCheck );
                }
            }
        }
    }

    return TRUE;
}

void OfaMSFilterTabPage2::Reset( const SfxItemSet& rSet )
{
    SvtFilterOptions* pOpt = SvtFilterOptions::Get();

    aCheckLB.SetUpdateMode(FALSE);
    aCheckLB.Clear();

    SvtModuleOptions aModuleOpt;

    // int the same sequence as the enums of MSFltrPg2_CheckBoxEntries
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SMATH ) )
        InsertEntry( sChgToFromMath, static_cast< sal_Int32 >( Math ) );
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
        InsertEntry( sChgToFromWriter, static_cast< sal_Int32 >( Writer ) );
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
        InsertEntry( sChgToFromCalc, static_cast< sal_Int32 >( Calc ) );
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
        InsertEntry( sChgToFromImpress, static_cast< sal_Int32 >( Impress ) );

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
        SvLBoxEntry* pEntry = GetEntry4Type( static_cast< sal_Int32 >( pArr->eType ) );
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

void OfaMSFilterTabPage2::InsertEntry( const String& _rTxt, sal_Int32 _nType )
{
    SvLBoxEntry* pEntry = new SvLBoxEntry;

    if( !pCheckButtonData )
        pCheckButtonData = new SvLBoxButtonData( &aCheckLB );

    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));
    pEntry->AddItem( new SvLBoxButton( pEntry, 0, pCheckButtonData ) );
    pEntry->AddItem( new SvLBoxButton( pEntry, 0, pCheckButtonData ) );
    pEntry->AddItem( new SvLBoxString( pEntry, 0, _rTxt ) );

    pEntry->SetUserData( (void*)_nType );

    aCheckLB.GetModel()->Insert( pEntry );
}

SvLBoxEntry* OfaMSFilterTabPage2::GetEntry4Type( sal_Int32 _nType ) const
{
    SvLBoxEntry* pEntry = aCheckLB.First();
    while ( pEntry )
    {
        if ( _nType == (sal_Int32)pEntry->GetUserData() )
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
/* -----------------------------2002/06/20 11:51------------------------------

 ---------------------------------------------------------------------------*/
void OfaMSFilterTabPage2::MSFltrSimpleTable::SetCheckButtonState(
                            SvLBoxEntry* pEntry, USHORT nCol, SvButtonState eState)
{
    SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetItem(nCol + 1));

    DBG_ASSERT(pItem,"SetCheckButton:Item not found")
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
/* -----------------------------2002/06/20 11:56------------------------------

 ---------------------------------------------------------------------------*/
SvButtonState OfaMSFilterTabPage2::MSFltrSimpleTable::GetCheckButtonState(
                                    SvLBoxEntry* pEntry, USHORT nCol ) const
{
    SvButtonState eState = SV_BUTTON_UNCHECKED;
    SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetItem(nCol + 1));
    DBG_ASSERT(pItem,"GetChButnState:Item not found")

    if (((SvLBoxItem*)pItem)->IsA() == SV_ITEM_ID_LBOXBUTTON)
    {
        USHORT nButtonFlags = pItem->GetButtonFlags();
        eState = pCheckButtonData->ConvertToButtonState( nButtonFlags );
    }

    return eState;
}
/* -----------------------------2002/06/20 11:57------------------------------

 ---------------------------------------------------------------------------*/
void OfaMSFilterTabPage2::MSFltrSimpleTable::CheckEntryPos(ULONG nPos, USHORT nCol, BOOL bChecked)
{
    if ( nPos < GetEntryCount() )
        SetCheckButtonState(
            GetEntry(nPos),
            nCol,
            bChecked ? SvButtonState( SV_BUTTON_CHECKED ) :
                                       SvButtonState( SV_BUTTON_UNCHECKED ) );
}
/* -----------------------------2002/06/20 11:51------------------------------

 ---------------------------------------------------------------------------*/
void OfaMSFilterTabPage2::MSFltrSimpleTable::KeyInput( const KeyEvent& rKEvt )
{
    if(!rKEvt.GetKeyCode().GetModifier() &&
        KEY_SPACE == rKEvt.GetKeyCode().GetCode())
    {
        ULONG nSelPos = GetModel()->GetAbsPos(GetCurEntry());
        USHORT nCheck = GetCheckButtonState( GetEntry(nSelPos), 1 ) == SV_BUTTON_CHECKED ? 1 : 0;
        if(GetCheckButtonState( GetEntry(nSelPos), 0 ))
            nCheck += 2;
        nCheck--;
        nCheck &= 3;
        CheckEntryPos(nSelPos, 1, 0 != (nCheck & 1));
        CheckEntryPos(nSelPos, 0, 0 != (nCheck & 2));
    }
    else
        SvxSimpleTable::KeyInput(rKEvt);
}

