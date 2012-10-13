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

#include <rangelst.hxx>

#include <comphelper/string.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/charclass.hxx>
#include <stdlib.h>

#define _AREASDLG_CXX
#include "areasdlg.hxx"
#undef _AREASDLG_CXX

#include "scresid.hxx"
#include "rangenam.hxx"
#include "reffact.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "globstr.hrc"
#include "pagedlg.hrc"
#include "compiler.hxx"
#include "markdata.hxx"

// STATIC DATA ---------------------------------------------------------------

// List box positions for print range (PR)
const sal_uInt16 SC_AREASDLG_PR_NONE    = 0;
const sal_uInt16 SC_AREASDLG_PR_ENTIRE  = 1;
const sal_uInt16 SC_AREASDLG_PR_USER    = 2;
const sal_uInt16 SC_AREASDLG_PR_SELECT  = 3;
const sal_uInt16 SC_AREASDLG_PR_OFFSET  = 4;

// List box positions for repeat ranges (RR)
const sal_uInt16 SC_AREASDLG_RR_NONE    = 0;
const sal_uInt16 SC_AREASDLG_RR_USER    = 1;
const sal_uInt16 SC_AREASDLG_RR_OFFSET  = 2;

//============================================================================

using ::rtl::OUString;

#define HDL(hdl)            LINK( this, ScPrintAreasDlg, hdl )
#define ERRORBOX(nId)       ErrorBox( this, WinBits(WB_OK|WB_DEF_OK), \
                            ScGlobal::GetRscString( nId ) ).Execute()
#define SWAP(x1,x2)         { int n=x1; x1=x2; x2=n; }

// globale Funktionen (->am Ende der Datei):

static bool lcl_CheckRepeatString( const String& rStr, ScDocument* pDoc, bool bIsRow, ScRange* pRange );
static void lcl_GetRepeatRangeString( const ScRange* pRange, ScDocument* pDoc, bool bIsRow, String& rStr );
static void lcl_GetRepeatRangeString( const ScRange* pRange, ScDocument* pDoc, bool bIsRow, OUString& rStr )
{
    String aStr;
    lcl_GetRepeatRangeString(pRange, pDoc, bIsRow, aStr);
    rStr = aStr;
}

#if 0
// this method is useful when debugging address flags.
static void printAddressFlags(sal_uInt16 nFlag)
{
    if ((nFlag & SCA_COL_ABSOLUTE  ) == SCA_COL_ABSOLUTE  )  printf("SCA_COL_ABSOLUTE \n");
    if ((nFlag & SCA_ROW_ABSOLUTE  ) == SCA_ROW_ABSOLUTE  )  printf("SCA_ROW_ABSOLUTE \n");
    if ((nFlag & SCA_TAB_ABSOLUTE  ) == SCA_TAB_ABSOLUTE  )  printf("SCA_TAB_ABSOLUTE \n");
    if ((nFlag & SCA_TAB_3D        ) == SCA_TAB_3D        )  printf("SCA_TAB_3D       \n");
    if ((nFlag & SCA_COL2_ABSOLUTE ) == SCA_COL2_ABSOLUTE )  printf("SCA_COL2_ABSOLUTE\n");
    if ((nFlag & SCA_ROW2_ABSOLUTE ) == SCA_ROW2_ABSOLUTE )  printf("SCA_ROW2_ABSOLUTE\n");
    if ((nFlag & SCA_TAB2_ABSOLUTE ) == SCA_TAB2_ABSOLUTE )  printf("SCA_TAB2_ABSOLUTE\n");
    if ((nFlag & SCA_TAB2_3D       ) == SCA_TAB2_3D       )  printf("SCA_TAB2_3D      \n");
    if ((nFlag & SCA_VALID_ROW     ) == SCA_VALID_ROW     )  printf("SCA_VALID_ROW    \n");
    if ((nFlag & SCA_VALID_COL     ) == SCA_VALID_COL     )  printf("SCA_VALID_COL    \n");
    if ((nFlag & SCA_VALID_TAB     ) == SCA_VALID_TAB     )  printf("SCA_VALID_TAB    \n");
    if ((nFlag & SCA_FORCE_DOC     ) == SCA_FORCE_DOC     )  printf("SCA_FORCE_DOC    \n");
    if ((nFlag & SCA_VALID_ROW2    ) == SCA_VALID_ROW2    )  printf("SCA_VALID_ROW2   \n");
    if ((nFlag & SCA_VALID_COL2    ) == SCA_VALID_COL2    )  printf("SCA_VALID_COL2   \n");
    if ((nFlag & SCA_VALID_TAB2    ) == SCA_VALID_TAB2    )  printf("SCA_VALID_TAB2   \n");
    if ((nFlag & SCA_VALID         ) == SCA_VALID         )  printf("SCA_VALID        \n");
    if ((nFlag & SCA_ABS           ) == SCA_ABS           )  printf("SCA_ABS          \n");
    if ((nFlag & SCR_ABS           ) == SCR_ABS           )  printf("SCR_ABS          \n");
    if ((nFlag & SCA_ABS_3D        ) == SCA_ABS_3D        )  printf("SCA_ABS_3D       \n");
    if ((nFlag & SCR_ABS_3D        ) == SCR_ABS_3D        )  printf("SCR_ABS_3D       \n");
}
#endif

//============================================================================
//  class ScPrintAreasDlg

//----------------------------------------------------------------------------

ScPrintAreasDlg::ScPrintAreasDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent )
    :   ScAnyRefDlg     ( pB, pCW, pParent, RID_SCDLG_AREAS),
        //
        aFlPrintArea    ( this, ScResId( FL_PRINTAREA ) ),
        aLbPrintArea    ( this, ScResId( LB_PRINTAREA ) ),
        aEdPrintArea    ( this, this, ScResId( ED_PRINTAREA ) ),
        aRbPrintArea    ( this, ScResId( RB_PRINTAREA ), &aEdPrintArea, this ),
        //
        aFlRepeatRow    ( this, ScResId( FL_REPEATROW ) ),
        aLbRepeatRow    ( this, ScResId( LB_REPEATROW ) ),
        aEdRepeatRow    ( this, this, ScResId( ED_REPEATROW ) ),
        aRbRepeatRow    ( this, ScResId( RB_REPEATROW ), &aEdRepeatRow, this ),
        //
        aFlRepeatCol    ( this, ScResId( FL_REPEATCOL ) ),
        aLbRepeatCol    ( this, ScResId( LB_REPEATCOL ) ),
        aEdRepeatCol    ( this, this, ScResId( ED_REPEATCOL ) ),
        aRbRepeatCol    ( this, ScResId( RB_REPEATCOL ), &aEdRepeatCol, this ),
        //
        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        //
        bDlgLostFocus   ( false ),
        pRefInputEdit   ( &aEdPrintArea ),
        pDoc            ( NULL ),
        pViewData       ( NULL ),
        nCurTab         ( 0 )
{
    ScTabViewShell* pScViewSh = PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
    ScDocShell*     pScDocSh  = PTR_CAST( ScDocShell,     SfxObjectShell::Current() );

    OSL_ENSURE( pScDocSh, "Current DocumentShell not found :-(" );

    pDoc = pScDocSh->GetDocument();

    if ( pScViewSh )
    {
        pViewData = pScViewSh->GetViewData();
        nCurTab   = pViewData->GetTabNo();
    }

    Impl_Reset();

    //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
    //SFX_APPWINDOW->Enable();

    FreeResource();
}


//----------------------------------------------------------------------------

ScPrintAreasDlg::~ScPrintAreasDlg()
{
    // Extra-Data an ListBox-Entries abraeumen
    ListBox* pLb[3] = { &aLbPrintArea, &aLbRepeatRow, &aLbRepeatCol };

    for ( sal_uInt16 i=0; i<3; i++ )
    {
        sal_uInt16 nCount = pLb[i]->GetEntryCount();
        for ( sal_uInt16 j=0; j<nCount; j++ )
            delete (String*)pLb[i]->GetEntryData(j);
    }
}


//----------------------------------------------------------------------------

sal_Bool ScPrintAreasDlg::Close()
{
    return DoClose( ScPrintAreasDlgWrapper::GetChildWindowId() );
}


//----------------------------------------------------------------------------

sal_Bool ScPrintAreasDlg::IsTableLocked() const
{
    //  Druckbereiche gelten pro Tabelle, darum macht es keinen Sinn,
    //  bei der Eingabe die Tabelle umzuschalten

    return sal_True;
}


//----------------------------------------------------------------------------

void ScPrintAreasDlg::SetReference( const ScRange& rRef, ScDocument* /* pDoc */ )
{
    if ( pRefInputEdit )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart( pRefInputEdit );

        String  aStr;
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

        if ( &aEdPrintArea == pRefInputEdit )
        {
            rRef.Format( aStr, SCR_ABS, pDoc, eConv );
            String aVal = aEdPrintArea.GetText();
            Selection aSel = aEdPrintArea.GetSelection();
            aSel.Justify();
            aVal.Erase( (xub_StrLen)aSel.Min(), (xub_StrLen)aSel.Len() );
            aVal.Insert( aStr, (xub_StrLen)aSel.Min() );
            Selection aNewSel( aSel.Min(), aSel.Min()+aStr.Len() );
            aEdPrintArea.SetRefString( aVal );
            aEdPrintArea.SetSelection( aNewSel );
        }
        else
        {
            sal_Bool bRow = ( &aEdRepeatRow == pRefInputEdit );
            lcl_GetRepeatRangeString(&rRef, pDoc, bRow, aStr);
            pRefInputEdit->SetRefString( aStr );
        }
    }

    Impl_ModifyHdl( pRefInputEdit );
}


//----------------------------------------------------------------------------

void ScPrintAreasDlg::AddRefEntry()
{
    if ( pRefInputEdit == &aEdPrintArea )
    {
        const sal_Unicode sep = ScCompiler::GetNativeSymbol(ocSep).GetChar(0);
        String aVal = aEdPrintArea.GetText();
        aVal += sep;
        aEdPrintArea.SetText(aVal);

        xub_StrLen nLen = aVal.Len();
        aEdPrintArea.SetSelection( Selection( nLen, nLen ) );

        Impl_ModifyHdl( &aEdPrintArea );
    }
}


//----------------------------------------------------------------------------

void ScPrintAreasDlg::Deactivate()
{
    bDlgLostFocus = sal_True;
}


//----------------------------------------------------------------------------

void ScPrintAreasDlg::SetActive()
{
    if ( bDlgLostFocus )
    {
        bDlgLostFocus = false;

        if ( pRefInputEdit )
        {
            pRefInputEdit->GrabFocus();
            Impl_ModifyHdl( pRefInputEdit );
        }
    }
    else
        GrabFocus();

    RefInputDone();
}


//----------------------------------------------------------------------------

void ScPrintAreasDlg::Impl_Reset()
{
    String          aStrRange;
    const ScRange*  pRepeatColRange = pDoc->GetRepeatColRange( nCurTab );
    const ScRange*  pRepeatRowRange = pDoc->GetRepeatRowRange( nCurTab );

    aEdPrintArea.SetModifyHdl   ( HDL(Impl_ModifyHdl) );
    aEdRepeatRow.SetModifyHdl   ( HDL(Impl_ModifyHdl) );
    aEdRepeatCol.SetModifyHdl   ( HDL(Impl_ModifyHdl) );
    aEdPrintArea.SetGetFocusHdl( HDL(Impl_GetFocusHdl) );
    aEdRepeatRow.SetGetFocusHdl( HDL(Impl_GetFocusHdl) );
    aEdRepeatCol.SetGetFocusHdl( HDL(Impl_GetFocusHdl) );
    aLbPrintArea.SetGetFocusHdl( HDL(Impl_GetFocusHdl) );
    aLbRepeatRow.SetGetFocusHdl( HDL(Impl_GetFocusHdl) );
    aLbRepeatCol.SetGetFocusHdl( HDL(Impl_GetFocusHdl) );
    aLbPrintArea.SetSelectHdl   ( HDL(Impl_SelectHdl) );
    aLbRepeatRow.SetSelectHdl   ( HDL(Impl_SelectHdl) );
    aLbRepeatCol.SetSelectHdl   ( HDL(Impl_SelectHdl) );
    aBtnOk      .SetClickHdl    ( HDL(Impl_BtnHdl)    );
    aBtnCancel  .SetClickHdl    ( HDL(Impl_BtnHdl)    );

    Impl_FillLists();

    //-------------------------
    // Druckbereich
    //-------------------------
    aStrRange.Erase();
    String aOne;
    const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
    const sal_Unicode sep = ScCompiler::GetNativeSymbol(ocSep).GetChar(0);
    sal_uInt16 nRangeCount = pDoc->GetPrintRangeCount( nCurTab );
    for (sal_uInt16 i=0; i<nRangeCount; i++)
    {
        const ScRange* pPrintRange = pDoc->GetPrintRange( nCurTab, i );
        if (pPrintRange)
        {
            if ( aStrRange.Len() )
                aStrRange += sep;
            pPrintRange->Format( aOne, SCR_ABS, pDoc, eConv );
            aStrRange += aOne;
        }
    }
    aEdPrintArea.SetText( aStrRange );

    //-------------------------------
    // Wiederholungszeile
    //-------------------------------
    lcl_GetRepeatRangeString(pRepeatRowRange, pDoc, true, aStrRange);
    aEdRepeatRow.SetText( aStrRange );

    //--------------------------------
    // Wiederholungsspalte
    //--------------------------------
    lcl_GetRepeatRangeString(pRepeatColRange, pDoc, false, aStrRange);
    aEdRepeatCol.SetText( aStrRange );

    Impl_ModifyHdl( &aEdPrintArea );
    Impl_ModifyHdl( &aEdRepeatRow );
    Impl_ModifyHdl( &aEdRepeatCol );
    if( pDoc->IsPrintEntireSheet( nCurTab ) )
        aLbPrintArea.SelectEntryPos( SC_AREASDLG_PR_ENTIRE );

    aEdPrintArea.SaveValue();   // fuer FillItemSet() merken:
    aEdRepeatRow.SaveValue();
    aEdRepeatCol.SaveValue();
}


//----------------------------------------------------------------------------

bool ScPrintAreasDlg::Impl_GetItem( Edit* pEd, SfxStringItem& rItem )
{
    String  aRangeStr = pEd->GetText();
    bool bDataChanged = (pEd->GetSavedValue() != aRangeStr);

    if ( (aRangeStr.Len() > 0) && &aEdPrintArea != pEd )
    {
        ScRange aRange;
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
        lcl_CheckRepeatString(aRangeStr, pDoc, &aEdRepeatRow == pEd, &aRange);
        aRange.Format(aRangeStr, SCR_ABS, pDoc, eConv);
    }

    rItem.SetValue( aRangeStr );

    return bDataChanged;
}


//----------------------------------------------------------------------------

sal_Bool ScPrintAreasDlg::Impl_CheckRefStrings()
{
    sal_Bool        bOk = false;
    String      aStrPrintArea   = aEdPrintArea.GetText();
    String      aStrRepeatRow   = aEdRepeatRow.GetText();
    String      aStrRepeatCol   = aEdRepeatCol.GetText();

    sal_Bool bPrintAreaOk = sal_True;
    if ( aStrPrintArea.Len() )
    {
        const sal_uInt16 nValidAddr  = SCA_VALID | SCA_VALID_ROW | SCA_VALID_COL;
        const sal_uInt16 nValidRange = nValidAddr | SCA_VALID_ROW2 | SCA_VALID_COL2;
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
        const sal_Unicode sep  = ScCompiler::GetNativeSymbol(ocSep).GetChar(0);

        ScAddress aAddr;
        ScRange aRange;
        xub_StrLen nSepCount = comphelper::string::getTokenCount(aStrPrintArea, sep);
        for ( xub_StrLen i = 0; i < nSepCount && bPrintAreaOk; ++i )
        {
            String aOne = aStrPrintArea.GetToken(i, sep);
            sal_uInt16 nResult = aRange.Parse( aOne, pDoc, eConv );
            if ((nResult & nValidRange) != nValidRange)
            {
                sal_uInt16 nAddrResult = aAddr.Parse( aOne, pDoc, eConv );
                if ((nAddrResult & nValidAddr) != nValidAddr)
                    bPrintAreaOk = false;
            }
        }
    }

    sal_Bool bRepeatRowOk = (aStrRepeatRow.Len() == 0);
    if ( !bRepeatRowOk )
        bRepeatRowOk = lcl_CheckRepeatString(aStrRepeatRow, pDoc, true, NULL);

    sal_Bool bRepeatColOk = (aStrRepeatCol.Len() == 0);
    if ( !bRepeatColOk )
        bRepeatColOk = lcl_CheckRepeatString(aStrRepeatCol, pDoc, false, NULL);

    // Fehlermeldungen

    bOk = (bPrintAreaOk && bRepeatRowOk && bRepeatColOk);

    if ( !bOk )
    {
        Edit* pEd = NULL;

             if ( !bPrintAreaOk ) pEd = &aEdPrintArea;
        else if ( !bRepeatRowOk ) pEd = &aEdRepeatRow;
        else if ( !bRepeatColOk ) pEd = &aEdRepeatCol;

        ERRORBOX( STR_INVALID_TABREF );

        OSL_ASSERT(pEd);

        if (pEd)
            pEd->GrabFocus();
    }

    return bOk;
}


//----------------------------------------------------------------------------

void ScPrintAreasDlg::Impl_FillLists()
{
    //------------------------------------------------------
    // Selektion holen und String in PrintArea-ListBox merken
    //------------------------------------------------------
    ScRange aRange;
    String  aStrRange;
    sal_Bool bSimple = sal_True;

    if ( pViewData )
        bSimple = (pViewData->GetSimpleArea( aRange ) == SC_MARK_SIMPLE);

    formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

    if ( bSimple )
        aRange.Format( aStrRange, SCR_ABS, pDoc, eConv );
    else
    {
        ScRangeListRef aList( new ScRangeList );
        pViewData->GetMarkData().FillRangeListWithMarks( aList, false );
        aList->Format( aStrRange, SCR_ABS, pDoc, eConv );
    }

    aLbPrintArea.SetEntryData( SC_AREASDLG_PR_SELECT, new String( aStrRange ) );

    //------------------------------------------------------
    // Ranges holen und in ListBoxen merken
    //------------------------------------------------------
    ScRangeName* pRangeNames = pDoc->GetRangeName();

    if (!pRangeNames || pRangeNames->empty())
        // No range names to process.
        return;

    ScRangeName::const_iterator itr = pRangeNames->begin(), itrEnd = pRangeNames->end();
    for (; itr != itrEnd; ++itr)
    {
        if (!itr->second->HasType(RT_ABSAREA) && !itr->second->HasType(RT_REFAREA) && !itr->second->HasType(RT_ABSPOS))
            continue;

        OUString aName = itr->second->GetName();
        OUString aSymbol;
        itr->second->GetSymbol(aSymbol);
        if (aRange.ParseAny(aSymbol, pDoc, eConv) & SCA_VALID)
        {
            if (itr->second->HasType(RT_PRINTAREA))
            {
                aRange.Format(aSymbol, SCR_ABS, pDoc, eConv);
                aLbPrintArea.SetEntryData(
                    aLbPrintArea.InsertEntry(aName),
                    new String(aSymbol) );
            }

            if (itr->second->HasType(RT_ROWHEADER))
            {
                lcl_GetRepeatRangeString(&aRange, pDoc, true, aSymbol);
                aLbRepeatRow.SetEntryData(
                    aLbRepeatRow.InsertEntry(aName),
                    new String(aSymbol) );
            }

            if (itr->second->HasType(RT_COLHEADER))
            {
                lcl_GetRepeatRangeString(&aRange, pDoc, false, aSymbol);
                aLbRepeatCol.SetEntryData(
                    aLbRepeatCol.InsertEntry(aName),
                    new String(aSymbol));
            }
        }
    }
}


//----------------------------------------------------------------------------
// Handler:
//----------------------------------------------------------------------------

IMPL_LINK( ScPrintAreasDlg, Impl_BtnHdl, PushButton*, pBtn )
{
    if ( &aBtnOk == pBtn )
    {
        if ( Impl_CheckRefStrings() )
        {
            bool            bDataChanged = false;
            String          aStr;
            SfxStringItem   aPrintArea( SID_CHANGE_PRINTAREA, aStr );
            SfxStringItem   aRepeatRow( FN_PARAM_2, aStr );
            SfxStringItem   aRepeatCol( FN_PARAM_3, aStr );

            //-------------------------
            // Druckbereich veraendert?
            //-------------------------

            // first try the list box, if "Entite sheet" is selected
            bool bEntireSheet = (aLbPrintArea.GetSelectEntryPos() == SC_AREASDLG_PR_ENTIRE);
            SfxBoolItem aEntireSheet( FN_PARAM_4, bEntireSheet );

            bDataChanged = bEntireSheet != pDoc->IsPrintEntireSheet( nCurTab );
            if( !bEntireSheet )
            {
                // if new list box selection is not "Entire sheet", get the edit field contents
                bDataChanged |= Impl_GetItem( &aEdPrintArea, aPrintArea );
            }

            //-------------------------------
            // Wiederholungszeile veraendert?
            //-------------------------------
            bDataChanged |= Impl_GetItem( &aEdRepeatRow, aRepeatRow );

            //--------------------------------
            // Wiederholungsspalte veraendert?
            //--------------------------------
            bDataChanged |= Impl_GetItem( &aEdRepeatCol, aRepeatCol );

            if ( bDataChanged )
            {
                SetDispatcherLock( false );
                SwitchToDocument();
                GetBindings().GetDispatcher()->Execute( SID_CHANGE_PRINTAREA,
                                          SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                                          &aPrintArea, &aRepeatRow, &aRepeatCol, &aEntireSheet, 0L );
            }

            Close();
        }
    }
    else if ( &aBtnCancel == pBtn )
        Close();

    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScPrintAreasDlg, Impl_GetFocusHdl, Control*, pCtr )
{
    if ( pCtr ==(Control *) &aEdPrintArea ||
         pCtr ==(Control *) &aEdRepeatRow ||
         pCtr ==(Control *) &aEdRepeatCol)
    {
         pRefInputEdit = (formula::RefEdit*) pCtr;
    }
    else if ( pCtr ==(Control *) &aLbPrintArea)
    {
        pRefInputEdit = &aEdPrintArea;
    }
    else if ( pCtr ==(Control *) &aLbRepeatRow)
    {
        pRefInputEdit = &aEdRepeatRow;
    }
    else if ( pCtr ==(Control *) &aLbRepeatCol)
    {
        pRefInputEdit = &aEdRepeatCol;
    }

    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScPrintAreasDlg, Impl_SelectHdl, ListBox*, pLb )
{
    sal_uInt16 nSelPos = pLb->GetSelectEntryPos();
    Edit* pEd = NULL;

    // list box positions of specific entries, default to "repeat row/column" list boxes
    sal_uInt16 nAllSheetPos = SC_AREASDLG_RR_NONE;
    sal_uInt16 nUserDefPos = SC_AREASDLG_RR_USER;
    sal_uInt16 nFirstCustomPos = SC_AREASDLG_RR_OFFSET;

    // find edit field for list box, and list box positions
    if( pLb == &aLbPrintArea )
    {
        pEd = &aEdPrintArea;
        nAllSheetPos = SC_AREASDLG_PR_ENTIRE;
        nUserDefPos = SC_AREASDLG_PR_USER;
        nFirstCustomPos = SC_AREASDLG_PR_SELECT;    // "Selection" and following
    }
    else if( pLb == &aLbRepeatCol )
        pEd = &aEdRepeatCol;
    else if( pLb == &aLbRepeatRow )
        pEd = &aEdRepeatRow;
    else
        return 0;

    // fill edit field according to list box selection
    if( (nSelPos == 0) || (nSelPos == nAllSheetPos) )
        pEd->SetText( EMPTY_STRING );
    else if( nSelPos == nUserDefPos && !pLb->IsTravelSelect() && pEd->GetText().Len() == 0 )
        pLb->SelectEntryPos( 0 );
    else if( nSelPos >= nFirstCustomPos )
        pEd->SetText( *static_cast< String* >( pLb->GetEntryData( nSelPos ) ) );

    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScPrintAreasDlg, Impl_ModifyHdl, formula::RefEdit*, pEd )
{
    ListBox* pLb = NULL;

    // list box positions of specific entries, default to "repeat row/column" list boxes
    sal_uInt16 nUserDefPos = SC_AREASDLG_RR_USER;
    sal_uInt16 nFirstCustomPos = SC_AREASDLG_RR_OFFSET;

    if( pEd == &aEdPrintArea )
    {
        pLb = &aLbPrintArea;
        nUserDefPos = SC_AREASDLG_PR_USER;
        nFirstCustomPos = SC_AREASDLG_PR_SELECT;    // "Selection" and following
    }
    else if( pEd == &aEdRepeatCol )
        pLb = &aLbRepeatCol;
    else if( pEd == &aEdRepeatRow )
        pLb = &aLbRepeatRow;
    else
        return 0;

    // set list box selection according to edit field
    sal_uInt16  nEntryCount = pLb->GetEntryCount();
    String  aStrEd( pEd->GetText() );
    String  aEdUpper = aStrEd;
    aEdUpper.ToUpperAscii();

    if ( (nEntryCount > nFirstCustomPos) && aStrEd.Len() > 0 )
    {
        sal_Bool    bFound  = false;
        String* pSymbol = NULL;
        sal_uInt16 i;

        for ( i=nFirstCustomPos; i<nEntryCount && !bFound; i++ )
        {
            pSymbol = (String*)pLb->GetEntryData( i );
            bFound  = ( (*pSymbol == aStrEd) || (*pSymbol == aEdUpper) );
        }

        pLb->SelectEntryPos( bFound ? i-1 : nUserDefPos );
    }
    else
        pLb->SelectEntryPos( aStrEd.Len() ? nUserDefPos : 0 );

    return 0;
}


//============================================================================
// globale Funktionen:

// ----------------------------------------------------------------------------

// TODO: It might make sense to move these functions to address.?xx. -kohei

static bool lcl_CheckOne_OOO( const String& rStr, bool bIsRow, SCCOLROW& rVal )
{
    // Zulaessige Syntax fuer rStr:
    // Row: [$]1-MAXTAB
    // Col: [$]A-IV

    String  aStr    = rStr;
    xub_StrLen nLen = aStr.Len();
    SCCOLROW    nNum    = 0;
    sal_Bool    bStrOk  = ( nLen > 0 ) && ( bIsRow ? ( nLen < 6 ) : ( nLen < 4 ) );

    if ( bStrOk )
    {
        if ( '$' == aStr.GetChar(0) )
            aStr.Erase( 0, 1 );

        if ( bIsRow )
        {
            bStrOk = CharClass::isAsciiNumeric(aStr);

            if ( bStrOk )
            {
                sal_Int32 n = aStr.ToInt32();

                if ( ( bStrOk = (n > 0) && ( n <= MAXROWCOUNT ) ) != false )
                    nNum = static_cast<SCCOLROW>(n - 1);
            }
        }
        else
        {
            SCCOL nCol = 0;
            bStrOk = ::AlphaToCol( nCol, aStr);
            nNum = nCol;
        }
    }

    if ( bStrOk )
        rVal = nNum;

    return bStrOk;
}

static bool lcl_CheckOne_XL_A1( const String& rStr, bool bIsRow, SCCOLROW& rVal )
{
    // XL A1 style is identical to OOO one for print range formats.
    return lcl_CheckOne_OOO(rStr, bIsRow, rVal);
}

static bool lcl_CheckOne_XL_R1C1( const String& rStr, bool bIsRow, SCCOLROW& rVal )
{
    xub_StrLen nLen = rStr.Len();
    if (nLen <= 1)
        // There must be at least two characters.
        return false;

    const sal_Unicode preUpper = bIsRow ? 'R' : 'C';
    const sal_Unicode preLower = bIsRow ? 'r' : 'c';
    if (rStr.GetChar(0) != preUpper && rStr.GetChar(0) != preLower)
        return false;

    String aNumStr = rStr.Copy(1);
    if (!CharClass::isAsciiNumeric(aNumStr))
        return false;

    sal_Int32 nNum = aNumStr.ToInt32();

    if (nNum <= 0)
        return false;

    if ((bIsRow && nNum > MAXROWCOUNT) || (!bIsRow && nNum > MAXCOLCOUNT))
        return false;

    rVal = static_cast<SCCOLROW>(nNum-1);
    return true;
}

static bool lcl_CheckRepeatOne( const String& rStr, formula::FormulaGrammar::AddressConvention eConv, bool bIsRow, SCCOLROW& rVal )
{
    switch (eConv)
    {
        case formula::FormulaGrammar::CONV_OOO:
            return lcl_CheckOne_OOO(rStr, bIsRow, rVal);
        case formula::FormulaGrammar::CONV_XL_A1:
            return lcl_CheckOne_XL_A1(rStr, bIsRow, rVal);
        case formula::FormulaGrammar::CONV_XL_R1C1:
            return lcl_CheckOne_XL_R1C1(rStr, bIsRow, rVal);
        default:
        {
            // added to avoid warnings
        }
    }
    return false;
}

static bool lcl_CheckRepeatString( const String& rStr, ScDocument* pDoc, bool bIsRow, ScRange* pRange )
{
    // Row: [valid row] rsep [valid row]
    // Col: [valid col] rsep [valid col]

    const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
    const sal_Unicode rsep = ScCompiler::GetNativeSymbol(ocRange).GetChar(0);

    if (pRange)
    {
        // initialize the range value.
        pRange->aStart.SetCol(0);
        pRange->aStart.SetRow(0);
        pRange->aEnd.SetCol(0);
        pRange->aEnd.SetRow(0);
    }

    String aBuf;
    SCCOLROW nVal = 0;
    xub_StrLen nLen = rStr.Len();
    bool bEndPos = false;
    for (xub_StrLen i = 0; i < nLen; ++i)
    {
        const sal_Unicode c = rStr.GetChar(i);
        if (c == rsep)
        {
            if (bEndPos)
                // We aren't supposed to have more than one range separator.
                return false;

            // range separator
            if (aBuf.Len() == 0)
                return false;

            bool bRes = lcl_CheckRepeatOne(aBuf, eConv, bIsRow, nVal);
            if (!bRes)
                return false;

            if (pRange)
            {
                if (bIsRow)
                {
                    pRange->aStart.SetRow(static_cast<SCROW>(nVal));
                    pRange->aEnd.SetRow(static_cast<SCROW>(nVal));
                }
                else
                {
                    pRange->aStart.SetCol(static_cast<SCCOL>(nVal));
                    pRange->aEnd.SetCol(static_cast<SCCOL>(nVal));
                }
            }

            aBuf.Erase();
            bEndPos = true;
        }
        else
            aBuf.Append(c);
    }

    if (aBuf.Len() > 0)
    {
        bool bRes = lcl_CheckRepeatOne(aBuf, eConv, bIsRow, nVal);
        if (!bRes)
            return false;

        if (pRange)
        {
            if (bIsRow)
            {
                if (!bEndPos)
                    pRange->aStart.SetRow(static_cast<SCROW>(nVal));
                pRange->aEnd.SetRow(static_cast<SCROW>(nVal));
            }
            else
            {
                if (!bEndPos)
                    pRange->aStart.SetCol(static_cast<SCCOL>(nVal));
                pRange->aEnd.SetCol(static_cast<SCCOL>(nVal));
            }
        }
    }

    return true;
}

// ----------------------------------------------------------------------------

static void lcl_GetRepeatRangeString( const ScRange* pRange, ScDocument* pDoc, bool bIsRow, String& rStr )
{
    rStr.Erase();
    if (!pRange)
        return;

    const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
    const ScAddress& rStart = pRange->aStart;
    const ScAddress& rEnd   = pRange->aEnd;

    const sal_uInt16 nFmt = bIsRow ? (SCA_VALID_ROW | SCA_ROW_ABSOLUTE) : (SCA_VALID_COL | SCA_COL_ABSOLUTE);
    String aTmpStr;
    rStart.Format(aTmpStr, nFmt, pDoc, eConv);
    rStr += aTmpStr;
    if ((bIsRow && rStart.Row() != rEnd.Row()) || (!bIsRow && rStart.Col() != rEnd.Col()))
    {
        rStr += ScCompiler::GetNativeSymbol(ocRange);
        rEnd.Format(aTmpStr, nFmt, pDoc, eConv);
        rStr += aTmpStr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
