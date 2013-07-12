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


#define HDL(hdl)            LINK( this, ScPrintAreasDlg, hdl )
#define ERRORBOX(nId)       ErrorBox( this, WinBits(WB_OK|WB_DEF_OK), \
                            ScGlobal::GetRscString( nId ) ).Execute()

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
    : ScAnyRefDlg(pB, pCW, pParent, "PrintAreasDialog", "modules/scalc/ui/printareasdialog.ui")
    , bDlgLostFocus(false)
    , pDoc(NULL)
    , pViewData(NULL)
    , nCurTab(0)
{
    get(pLbPrintArea,"lbprintarea");
    get(pEdPrintArea,"edprintarea");
    pEdPrintArea->SetReferences(this, get<VclFrame>("printframe")->get_label_widget());
    pRefInputEdit = pEdPrintArea;
    get(pRbPrintArea,"rbprintarea");
    pRbPrintArea->SetReferences(this, pEdPrintArea);

    get(pLbRepeatRow,"lbrepeatrow");
    get(pEdRepeatRow,"edrepeatrow");
    pEdRepeatRow->SetReferences(this, get<VclFrame>("rowframe")->get_label_widget());
    get(pRbRepeatRow,"rbrepeatrow");
    pRbRepeatRow->SetReferences(this, pEdRepeatRow);

    get(pLbRepeatCol,"lbrepeatcol");
    get(pEdRepeatCol,"edrepeatcol");
    pEdRepeatCol->SetReferences(this, get<VclFrame>("colframe")->get_label_widget());
    get(pRbRepeatCol,"rbrepeatcol");
    pRbRepeatCol->SetReferences(this, pEdRepeatCol);

    get(pBtnOk,"ok");
    get(pBtnCancel,"cancel");

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
}


//----------------------------------------------------------------------------

ScPrintAreasDlg::~ScPrintAreasDlg()
{
    // Extra-Data an ListBox-Entries abraeumen
    ListBox* aLb[3] = { pLbPrintArea, pLbRepeatRow, pLbRepeatCol };

    for (sal_uInt16 i = 0; i < SAL_N_ELEMENTS(aLb); ++i)
    {
        sal_uInt16 nCount = aLb[i]->GetEntryCount();
        for ( sal_uInt16 j=0; j<nCount; j++ )
            delete (String*)aLb[i]->GetEntryData(j);
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

        if ( pEdPrintArea == pRefInputEdit )
        {
            rRef.Format( aStr, SCR_ABS, pDoc, eConv );
            String aVal = pEdPrintArea->GetText();
            Selection aSel = pEdPrintArea->GetSelection();
            aSel.Justify();
            aVal.Erase( (xub_StrLen)aSel.Min(), (xub_StrLen)aSel.Len() );
            aVal.Insert( aStr, (xub_StrLen)aSel.Min() );
            Selection aNewSel( aSel.Min(), aSel.Min()+aStr.Len() );
            pEdPrintArea->SetRefString( aVal );
            pEdPrintArea->SetSelection( aNewSel );
        }
        else
        {
            sal_Bool bRow = ( pEdRepeatRow == pRefInputEdit );
            lcl_GetRepeatRangeString(&rRef, pDoc, bRow, aStr);
            pRefInputEdit->SetRefString( aStr );
        }
    }

    Impl_ModifyHdl( pRefInputEdit );
}


//----------------------------------------------------------------------------

void ScPrintAreasDlg::AddRefEntry()
{
    if ( pRefInputEdit == pEdPrintArea )
    {
        const sal_Unicode sep = ScCompiler::GetNativeSymbol(ocSep).GetChar(0);
        String aVal = pEdPrintArea->GetText();
        aVal += sep;
        pEdPrintArea->SetText(aVal);

        xub_StrLen nLen = aVal.Len();
        pEdPrintArea->SetSelection( Selection( nLen, nLen ) );

        Impl_ModifyHdl( pEdPrintArea );
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

    pEdPrintArea->SetModifyHdl   ( HDL(Impl_ModifyHdl) );
    pEdRepeatRow->SetModifyHdl   ( HDL(Impl_ModifyHdl) );
    pEdRepeatCol->SetModifyHdl   ( HDL(Impl_ModifyHdl) );
    pEdPrintArea->SetGetFocusHdl( HDL(Impl_GetFocusHdl) );
    pEdRepeatRow->SetGetFocusHdl( HDL(Impl_GetFocusHdl) );
    pEdRepeatCol->SetGetFocusHdl( HDL(Impl_GetFocusHdl) );
    pLbPrintArea->SetGetFocusHdl( HDL(Impl_GetFocusHdl) );
    pLbRepeatRow->SetGetFocusHdl( HDL(Impl_GetFocusHdl) );
    pLbRepeatCol->SetGetFocusHdl( HDL(Impl_GetFocusHdl) );
    pLbPrintArea->SetSelectHdl   ( HDL(Impl_SelectHdl) );
    pLbRepeatRow->SetSelectHdl   ( HDL(Impl_SelectHdl) );
    pLbRepeatCol->SetSelectHdl   ( HDL(Impl_SelectHdl) );
    pBtnOk->      SetClickHdl    ( HDL(Impl_BtnHdl)    );
    pBtnCancel->  SetClickHdl    ( HDL(Impl_BtnHdl)    );

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
    pEdPrintArea->SetText( aStrRange );

    //-------------------------------
    // Wiederholungszeile
    //-------------------------------
    lcl_GetRepeatRangeString(pRepeatRowRange, pDoc, true, aStrRange);
    pEdRepeatRow->SetText( aStrRange );

    //--------------------------------
    // Wiederholungsspalte
    //--------------------------------
    lcl_GetRepeatRangeString(pRepeatColRange, pDoc, false, aStrRange);
    pEdRepeatCol->SetText( aStrRange );

    Impl_ModifyHdl( pEdPrintArea );
    Impl_ModifyHdl( pEdRepeatRow );
    Impl_ModifyHdl( pEdRepeatCol );
    if( pDoc->IsPrintEntireSheet( nCurTab ) )
        pLbPrintArea->SelectEntryPos( SC_AREASDLG_PR_ENTIRE );

    pEdPrintArea->SaveValue();   // fuer FillItemSet() merken:
    pEdRepeatRow->SaveValue();
    pEdRepeatCol->SaveValue();
}


//----------------------------------------------------------------------------

bool ScPrintAreasDlg::Impl_GetItem( Edit* pEd, SfxStringItem& rItem )
{
    String  aRangeStr = pEd->GetText();
    bool bDataChanged = (pEd->GetSavedValue() != aRangeStr);

    if ( (aRangeStr.Len() > 0) && pEdPrintArea != pEd )
    {
        ScRange aRange;
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
        lcl_CheckRepeatString(aRangeStr, pDoc, pEdRepeatRow == pEd, &aRange);
        aRange.Format(aRangeStr, SCR_ABS, pDoc, eConv);
    }

    rItem.SetValue( aRangeStr );

    return bDataChanged;
}


//----------------------------------------------------------------------------

sal_Bool ScPrintAreasDlg::Impl_CheckRefStrings()
{
    sal_Bool        bOk = false;
    String      aStrPrintArea   = pEdPrintArea->GetText();
    String      aStrRepeatRow   = pEdRepeatRow->GetText();
    String      aStrRepeatCol   = pEdRepeatCol->GetText();

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

             if ( !bPrintAreaOk ) pEd = pEdPrintArea;
        else if ( !bRepeatRowOk ) pEd = pEdRepeatRow;
        else if ( !bRepeatColOk ) pEd = pEdRepeatCol;

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

    pLbPrintArea->SetEntryData( SC_AREASDLG_PR_SELECT, new String( aStrRange ) );

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
                pLbPrintArea->SetEntryData(
                    pLbPrintArea->InsertEntry(aName),
                    new String(aSymbol) );
            }

            if (itr->second->HasType(RT_ROWHEADER))
            {
                lcl_GetRepeatRangeString(&aRange, pDoc, true, aSymbol);
                pLbRepeatRow->SetEntryData(
                    pLbRepeatRow->InsertEntry(aName),
                    new String(aSymbol) );
            }

            if (itr->second->HasType(RT_COLHEADER))
            {
                lcl_GetRepeatRangeString(&aRange, pDoc, false, aSymbol);
                pLbRepeatCol->SetEntryData(
                    pLbRepeatCol->InsertEntry(aName),
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
    if ( pBtnOk == pBtn )
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
            bool bEntireSheet = (pLbPrintArea->GetSelectEntryPos() == SC_AREASDLG_PR_ENTIRE);
            SfxBoolItem aEntireSheet( FN_PARAM_4, bEntireSheet );

            bDataChanged = bEntireSheet != pDoc->IsPrintEntireSheet( nCurTab );
            if( !bEntireSheet )
            {
                // if new list box selection is not "Entire sheet", get the edit field contents
                bDataChanged |= Impl_GetItem( pEdPrintArea, aPrintArea );
            }

            //-------------------------------
            // Wiederholungszeile veraendert?
            //-------------------------------
            bDataChanged |= Impl_GetItem( pEdRepeatRow, aRepeatRow );

            //--------------------------------
            // Wiederholungsspalte veraendert?
            //--------------------------------
            bDataChanged |= Impl_GetItem( pEdRepeatCol, aRepeatCol );

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
    else if ( pBtnCancel == pBtn )
        Close();

    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScPrintAreasDlg, Impl_GetFocusHdl, Control*, pCtr )
{
    if ( pCtr ==(Control *) pEdPrintArea ||
         pCtr ==(Control *) pEdRepeatRow ||
         pCtr ==(Control *) pEdRepeatCol)
    {
         pRefInputEdit = (formula::RefEdit*) pCtr;
    }
    else if ( pCtr ==(Control *) pLbPrintArea)
    {
        pRefInputEdit = pEdPrintArea;
    }
    else if ( pCtr ==(Control *) pLbRepeatRow)
    {
        pRefInputEdit = pEdRepeatRow;
    }
    else if ( pCtr ==(Control *) pLbRepeatCol)
    {
        pRefInputEdit = pEdRepeatCol;
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
    if( pLb == pLbPrintArea )
    {
        pEd = pEdPrintArea;
        nAllSheetPos = SC_AREASDLG_PR_ENTIRE;
        nUserDefPos = SC_AREASDLG_PR_USER;
        nFirstCustomPos = SC_AREASDLG_PR_SELECT;    // "Selection" and following
    }
    else if( pLb == pLbRepeatCol )
        pEd = pEdRepeatCol;
    else if( pLb == pLbRepeatRow )
        pEd = pEdRepeatRow;
    else
        return 0;

    // fill edit field according to list box selection
    if( (nSelPos == 0) || (nSelPos == nAllSheetPos) )
        pEd->SetText( EMPTY_STRING );
    else if( nSelPos == nUserDefPos && !pLb->IsTravelSelect() && pEd->GetText().isEmpty())
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

    if( pEd == pEdPrintArea )
    {
        pLb = pLbPrintArea;
        nUserDefPos = SC_AREASDLG_PR_USER;
        nFirstCustomPos = SC_AREASDLG_PR_SELECT;    // "Selection" and following
    }
    else if( pEd == pEdRepeatCol )
        pLb = pLbRepeatCol;
    else if( pEd == pEdRepeatRow )
        pLb = pLbRepeatRow;
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
