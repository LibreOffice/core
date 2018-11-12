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

#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>
#include <vcl/weld.hxx>
#include <unotools/charclass.hxx>
#include <stdlib.h>

#include <areasdlg.hxx>
#include <rangenam.hxx>
#include <reffact.hxx>
#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <compiler.hxx>
#include <markdata.hxx>

// List box positions for print range (PR)
enum {
    SC_AREASDLG_PR_ENTIRE  = 1,
    SC_AREASDLG_PR_USER    = 2,
    SC_AREASDLG_PR_SELECT  = 3
};

// List box positions for repeat ranges (RR)
enum {
    SC_AREASDLG_RR_NONE    = 0,
    SC_AREASDLG_RR_USER    = 1,
    SC_AREASDLG_RR_OFFSET  = 2
};

#define HDL(hdl)            LINK( this, ScPrintAreasDlg, hdl )
namespace
{
    void ERRORBOX(weld::Window* pParent, const char* rId)
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pParent,
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  ScResId(rId)));
        xBox->run();
    }
}

// global functions (->at the end of the file):

static bool lcl_CheckRepeatString( const OUString& rStr, const ScDocument* pDoc, bool bIsRow, ScRange* pRange );
static void lcl_GetRepeatRangeString( const ScRange* pRange, const ScDocument* pDoc, bool bIsRow, OUString& rStr );

#if 0
// this method is useful when debugging address flags.
static void printAddressFlags(ScRefFlags nFlag)
{
    if ((nFlag & ScRefFlags::COL_ABS      ) == ScRefFlags::COL_ABS      ) printf("ScRefFlags::COL_ABS      \n");
    if ((nFlag & ScRefFlags::ROW_ABS      ) == ScRefFlags::ROW_ABS      ) printf("ScRefFlags::ROW_ABS      \n");
    if ((nFlag & ScRefFlags::TAB_ABS      ) == ScRefFlags::TAB_ABS      ) printf("ScRefFlags::TAB_ABS      \n");
    if ((nFlag & ScRefFlags::TAB_3D       ) == ScRefFlags::TAB_3D       ) printf("ScRefFlags::TAB_3D       \n");
    if ((nFlag & ScRefFlags::COL2_ABS     ) == ScRefFlags::COL2_ABS     ) printf("ScRefFlags::COL2_ABS     \n");
    if ((nFlag & ScRefFlags::ROW2_ABS     ) == ScRefFlags::ROW2_ABS     ) printf("ScRefFlags::ROW2_ABS     \n");
    if ((nFlag & ScRefFlags::TAB2_ABS     ) == ScRefFlags::TAB2_ABS     ) printf("ScRefFlags::TAB2_ABS     \n");
    if ((nFlag & ScRefFlags::TAB2_3D      ) == ScRefFlags::TAB2_3D      ) printf("ScRefFlags::TAB2_3D      \n");
    if ((nFlag & ScRefFlags::ROW_VALID    ) == ScRefFlags::ROW_VALID    ) printf("ScRefFlags::ROW_VALID    \n");
    if ((nFlag & ScRefFlags::COL_VALID    ) == ScRefFlags::COL_VALID    ) printf("ScRefFlags::COL_VALID    \n");
    if ((nFlag & ScRefFlags::TAB_VALID    ) == ScRefFlags::TAB_VALID    ) printf("ScRefFlags::TAB_VALID    \n");
    if ((nFlag & ScRefFlags::FORCE_DOC    ) == ScRefFlags::FORCE_DOC    ) printf("ScRefFlags::FORCE_DOC    \n");
    if ((nFlag & ScRefFlags::ROW2_VALID   ) == ScRefFlags::ROW2_VALID   ) printf("ScRefFlags::ROW2_VALID   \n");
    if ((nFlag & ScRefFlags::COL2_VALID   ) == ScRefFlags::COL2_VALID   ) printf("ScRefFlags::COL2_VALID   \n");
    if ((nFlag & ScRefFlags::TAB2_VALID   ) == ScRefFlags::TAB2_VALID   ) printf("ScRefFlags::TAB2_VALID   \n");
    if ((nFlag & ScRefFlags::VALID        ) == ScRefFlags::VALID        ) printf("ScRefFlags::VALID        \n");
    if ((nFlag & ScRefFlags::ADDR_ABS     ) == ScRefFlags::ADDR_ABS     ) printf("ScRefFlags::ADDR_ABS     \n");
    if ((nFlag & ScRefFlags::RANGE_ABS    ) == ScRefFlags::RANGE_ABS    ) printf("ScRefFlags::RANGE_ABS    \n");
    if ((nFlag & ScRefFlags::ADDR_ABS_3D  ) == ScRefFlags::ADDR_ABS_3D  ) printf("ScRefFlags::ADDR_ABS_3D  \n");
    if ((nFlag & ScRefFlags::RANGE_ABS_3D ) == ScRefFlags::RANGE_ABS_3D ) printf("ScRefFlags::RANGE_ABS_3D \n");
}
#endif

//  class ScPrintAreasDlg

ScPrintAreasDlg::ScPrintAreasDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent )
    : ScAnyRefDlg(pB, pCW, pParent, "PrintAreasDialog", "modules/scalc/ui/printareasdialog.ui")
    , bDlgLostFocus(false)
    , pDoc(nullptr)
    , pViewData(nullptr)
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

    ScTabViewShell* pScViewSh = dynamic_cast<ScTabViewShell*>( SfxViewShell::Current()  );
    ScDocShell*     pScDocSh  = dynamic_cast<ScDocShell*>(     SfxObjectShell::Current()  );

    OSL_ENSURE( pScDocSh, "Current DocumentShell not found :-(" );

    pDoc = &pScDocSh->GetDocument();

    if ( pScViewSh )
    {
        pViewData = &pScViewSh->GetViewData();
        nCurTab   = pViewData->GetTabNo();
    }

    Impl_Reset();

    //@BugID 54702 Enable/Disable only in base class
    //SFX_APPWINDOW->Enable();
}

ScPrintAreasDlg::~ScPrintAreasDlg()
{
    disposeOnce();
}

void ScPrintAreasDlg::dispose()
{
    // clean extra data of ListBox entries
    ListBox* aLb[3] = { pLbPrintArea, pLbRepeatRow, pLbRepeatCol };

    for (ListBox* pBox : aLb)
    {
        const sal_Int32 nCount = pBox->GetEntryCount();
        for ( sal_Int32 j=0; j<nCount; j++ )
            delete static_cast<OUString*>(pBox->GetEntryData(j));
    }
    pLbPrintArea.clear();
    pEdPrintArea.clear();
    pRbPrintArea.clear();
    pLbRepeatRow.clear();
    pEdRepeatRow.clear();
    pRbRepeatRow.clear();
    pLbRepeatCol.clear();
    pEdRepeatCol.clear();
    pRbRepeatCol.clear();
    pBtnOk.clear();
    pBtnCancel.clear();
    pRefInputEdit.clear();
    ScAnyRefDlg::dispose();
}

bool ScPrintAreasDlg::Close()
{
    return DoClose( ScPrintAreasDlgWrapper::GetChildWindowId() );
}

bool ScPrintAreasDlg::IsTableLocked() const
{
    //  Printing areas are per table, therefore it makes no sense,
    //  to switch the table during input

    return true;
}

void ScPrintAreasDlg::SetReference( const ScRange& rRef, ScDocument* /* pDoc */ )
{
    if ( pRefInputEdit )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart( pRefInputEdit );

        OUString  aStr;
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

        if ( pEdPrintArea == pRefInputEdit )
        {
            aStr = rRef.Format(ScRefFlags::RANGE_ABS, pDoc, eConv);
            OUString aVal = pEdPrintArea->GetText();
            Selection aSel = pEdPrintArea->GetSelection();
            aSel.Justify();
            aVal = aVal.replaceAt( aSel.Min(), aSel.Len(), aStr );
            Selection aNewSel( aSel.Min(), aSel.Min()+aStr.getLength() );
            pEdPrintArea->SetRefString( aVal );
            pEdPrintArea->SetSelection( aNewSel );
        }
        else
        {
            bool bRow = ( pEdRepeatRow == pRefInputEdit );
            lcl_GetRepeatRangeString(&rRef, pDoc, bRow, aStr);
            pRefInputEdit->SetRefString( aStr );
        }
        Impl_ModifyHdl( *pRefInputEdit );
    }
}

void ScPrintAreasDlg::AddRefEntry()
{
    if ( pRefInputEdit == pEdPrintArea )
    {
        const sal_Unicode sep = ScCompiler::GetNativeSymbolChar(ocSep);
        OUString aVal = pEdPrintArea->GetText() + OUStringLiteral1(sep);
        pEdPrintArea->SetText(aVal);

        sal_Int32 nLen = aVal.getLength();
        pEdPrintArea->SetSelection( Selection( nLen, nLen ) );

        Impl_ModifyHdl( *pEdPrintArea );
    }
}

void ScPrintAreasDlg::Deactivate()
{
    bDlgLostFocus = true;
}

void ScPrintAreasDlg::SetActive()
{
    if ( bDlgLostFocus )
    {
        bDlgLostFocus = false;

        if ( pRefInputEdit )
        {
            pRefInputEdit->GrabFocus();
            Impl_ModifyHdl( *pRefInputEdit );
        }
    }
    else
        GrabFocus();

    RefInputDone();
}

void ScPrintAreasDlg::Impl_Reset()
{
    OUString        aStrRange;
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

    // printing area

    aStrRange.clear();
    const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
    const sal_Unicode sep = ScCompiler::GetNativeSymbolChar(ocSep);
    sal_uInt16 nRangeCount = pDoc->GetPrintRangeCount( nCurTab );
    for (sal_uInt16 i=0; i<nRangeCount; i++)
    {
        const ScRange* pPrintRange = pDoc->GetPrintRange( nCurTab, i );
        if (pPrintRange)
        {
            if ( !aStrRange.isEmpty() )
                aStrRange += OUStringLiteral1(sep);
            aStrRange += pPrintRange->Format(ScRefFlags::RANGE_ABS, pDoc, eConv);
        }
    }
    pEdPrintArea->SetText( aStrRange );

    // repeat row

    lcl_GetRepeatRangeString(pRepeatRowRange, pDoc, true, aStrRange);
    pEdRepeatRow->SetText( aStrRange );

    // repeat column

    lcl_GetRepeatRangeString(pRepeatColRange, pDoc, false, aStrRange);
    pEdRepeatCol->SetText( aStrRange );

    Impl_ModifyHdl( *pEdPrintArea );
    Impl_ModifyHdl( *pEdRepeatRow );
    Impl_ModifyHdl( *pEdRepeatCol );
    if( pDoc->IsPrintEntireSheet( nCurTab ) )
        pLbPrintArea->SelectEntryPos( SC_AREASDLG_PR_ENTIRE );

    pEdPrintArea->SaveValue();   // save for FillItemSet():
    pEdRepeatRow->SaveValue();
    pEdRepeatCol->SaveValue();
}

bool ScPrintAreasDlg::Impl_GetItem( const Edit* pEd, SfxStringItem& rItem )
{
    OUString  aRangeStr = pEd->GetText();
    bool bDataChanged = pEd->IsValueChangedFromSaved();

    if ( !aRangeStr.isEmpty() && pEdPrintArea != pEd )
    {
        ScRange aRange;
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
        lcl_CheckRepeatString(aRangeStr, pDoc, pEdRepeatRow == pEd, &aRange);
        aRangeStr = aRange.Format(ScRefFlags::RANGE_ABS, pDoc, eConv);
    }

    rItem.SetValue( aRangeStr );

    return bDataChanged;
}

bool ScPrintAreasDlg::Impl_CheckRefStrings()
{
    bool        bOk = false;
    OUString      aStrPrintArea   = pEdPrintArea->GetText();
    OUString      aStrRepeatRow   = pEdRepeatRow->GetText();
    OUString      aStrRepeatCol   = pEdRepeatCol->GetText();

    bool bPrintAreaOk = true;
    if ( !aStrPrintArea.isEmpty() )
    {
        const ScRefFlags nValidAddr  = ScRefFlags::VALID | ScRefFlags::ROW_VALID | ScRefFlags::COL_VALID;
        const ScRefFlags nValidRange = nValidAddr | ScRefFlags::ROW2_VALID | ScRefFlags::COL2_VALID;
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
        const sal_Unicode sep  = ScCompiler::GetNativeSymbolChar(ocSep);

        ScAddress aAddr;
        ScRange aRange;
        for ( sal_Int32 nIdx = 0; nIdx >= 0; )
        {
            const OUString aOne = aStrPrintArea.getToken(0, sep, nIdx);
            ScRefFlags nResult = aRange.Parse( aOne, pDoc, eConv );
            if ((nResult & nValidRange) != nValidRange)
            {
                ScRefFlags nAddrResult = aAddr.Parse( aOne, pDoc, eConv );
                if ((nAddrResult & nValidAddr) != nValidAddr)
                {
                    bPrintAreaOk = false;
                    break;
                }
            }
        }
    }

    bool bRepeatRowOk = aStrRepeatRow.isEmpty();
    if ( !bRepeatRowOk )
        bRepeatRowOk = lcl_CheckRepeatString(aStrRepeatRow, pDoc, true, nullptr);

    bool bRepeatColOk = aStrRepeatCol.isEmpty();
    if ( !bRepeatColOk )
        bRepeatColOk = lcl_CheckRepeatString(aStrRepeatCol, pDoc, false, nullptr);

    // error messages

    bOk = (bPrintAreaOk && bRepeatRowOk && bRepeatColOk);

    if ( !bOk )
    {
        Edit* pEd = nullptr;

        if ( !bPrintAreaOk ) pEd = pEdPrintArea;
        else if ( !bRepeatRowOk ) pEd = pEdRepeatRow;
        else if ( !bRepeatColOk ) pEd = pEdRepeatCol;

        ERRORBOX(GetFrameWeld(), STR_INVALID_TABREF);

        OSL_ASSERT(pEd);

        if (pEd)
            pEd->GrabFocus();
    }

    return bOk;
}

void ScPrintAreasDlg::Impl_FillLists()
{

    // Get selection and remember String in PrintArea-ListBox

    ScRange  aRange;
    OUString aStrRange;
    bool bSimple = true;

    if ( pViewData )
        bSimple = (pViewData->GetSimpleArea( aRange ) == SC_MARK_SIMPLE);

    formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

    if ( bSimple )
        aStrRange = aRange.Format(ScRefFlags::RANGE_ABS, pDoc, eConv);
    else
    {
        ScRangeListRef aList( new ScRangeList );
        pViewData->GetMarkData().FillRangeListWithMarks( aList.get(), false );
        aList->Format(aStrRange, ScRefFlags::RANGE_ABS, pDoc, eConv);
    }

    pLbPrintArea->SetEntryData( SC_AREASDLG_PR_SELECT, new OUString( aStrRange ) );

    // Get ranges and remember in ListBoxen

    ScRangeName* pRangeNames = pDoc->GetRangeName();

    if (!pRangeNames || pRangeNames->empty())
        // No range names to process.
        return;

    for (const auto& rEntry : *pRangeNames)
    {
        if (!rEntry.second->HasType(ScRangeData::Type::AbsArea   )
            && !rEntry.second->HasType(ScRangeData::Type::RefArea)
            && !rEntry.second->HasType(ScRangeData::Type::AbsPos ))
            continue;

        OUString aName = rEntry.second->GetName();
        OUString aSymbol;
        rEntry.second->GetSymbol(aSymbol);
        if (aRange.ParseAny(aSymbol, pDoc, eConv) & ScRefFlags::VALID)
        {
            if (rEntry.second->HasType(ScRangeData::Type::PrintArea))
            {
                aSymbol = aRange.Format(ScRefFlags::RANGE_ABS, pDoc, eConv);
                pLbPrintArea->SetEntryData(
                    pLbPrintArea->InsertEntry(aName),
                    new OUString(aSymbol) );
            }

            if (rEntry.second->HasType(ScRangeData::Type::RowHeader))
            {
                lcl_GetRepeatRangeString(&aRange, pDoc, true, aSymbol);
                pLbRepeatRow->SetEntryData(
                    pLbRepeatRow->InsertEntry(aName),
                    new OUString(aSymbol) );
            }

            if (rEntry.second->HasType(ScRangeData::Type::ColHeader))
            {
                lcl_GetRepeatRangeString(&aRange, pDoc, false, aSymbol);
                pLbRepeatCol->SetEntryData(
                    pLbRepeatCol->InsertEntry(aName),
                    new OUString(aSymbol));
            }
        }
    }
}

// Handler:

IMPL_LINK( ScPrintAreasDlg, Impl_BtnHdl, Button*, pBtn, void )
{
    if ( pBtnOk == pBtn )
    {
        if ( Impl_CheckRefStrings() )
        {
            OUString        aStr;
            SfxStringItem   aPrintArea( SID_CHANGE_PRINTAREA, aStr );
            SfxStringItem   aRepeatRow( FN_PARAM_2, aStr );
            SfxStringItem   aRepeatCol( FN_PARAM_3, aStr );

            // Printing area changed?

            // first try the list box, if "Entire sheet" is selected
            bool bEntireSheet = (pLbPrintArea->GetSelectedEntryPos() == SC_AREASDLG_PR_ENTIRE);
            SfxBoolItem aEntireSheet( FN_PARAM_4, bEntireSheet );

            bool bDataChanged = bEntireSheet != pDoc->IsPrintEntireSheet( nCurTab );
            if( !bEntireSheet )
            {
                // if new list box selection is not "Entire sheet", get the edit field contents
                bDataChanged |= Impl_GetItem( pEdPrintArea, aPrintArea );
            }

            // Repeat row changed?

            bDataChanged |= Impl_GetItem( pEdRepeatRow, aRepeatRow );

            // Repeat column changed?

            bDataChanged |= Impl_GetItem( pEdRepeatCol, aRepeatCol );

            if ( bDataChanged )
            {
                SetDispatcherLock( false );
                SwitchToDocument();
                GetBindings().GetDispatcher()->ExecuteList(SID_CHANGE_PRINTAREA,
                      SfxCallMode::SLOT | SfxCallMode::RECORD,
                      { &aPrintArea, &aRepeatRow, &aRepeatCol, &aEntireSheet });
            }

            Close();
        }
    }
    else if ( pBtnCancel == pBtn )
        Close();
}

IMPL_LINK( ScPrintAreasDlg, Impl_GetFocusHdl, Control&, rCtrl, void )
{
    if ( &rCtrl ==static_cast<Control *>(pEdPrintArea) ||
         &rCtrl ==static_cast<Control *>(pEdRepeatRow) ||
         &rCtrl ==static_cast<Control *>(pEdRepeatCol))
    {
         pRefInputEdit = static_cast<formula::RefEdit*>(&rCtrl);
    }
    else if ( &rCtrl ==static_cast<Control *>(pLbPrintArea))
    {
        pRefInputEdit = pEdPrintArea;
    }
    else if ( &rCtrl ==static_cast<Control *>(pLbRepeatRow))
    {
        pRefInputEdit = pEdRepeatRow;
    }
    else if ( &rCtrl ==static_cast<Control *>(pLbRepeatCol))
    {
        pRefInputEdit = pEdRepeatCol;
    }
}

IMPL_LINK( ScPrintAreasDlg, Impl_SelectHdl, ListBox&, rLb, void )
{
    ListBox* pLb = &rLb;
    const sal_Int32 nSelPos = pLb->GetSelectedEntryPos();
    Edit* pEd = nullptr;

    // list box positions of specific entries, default to "repeat row/column" list boxes
    sal_Int32 nAllSheetPos = SC_AREASDLG_RR_NONE;
    sal_Int32 nUserDefPos = SC_AREASDLG_RR_USER;
    sal_Int32 nFirstCustomPos = SC_AREASDLG_RR_OFFSET;

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
        return;

    // fill edit field according to list box selection
    if( (nSelPos == 0) || (nSelPos == nAllSheetPos) )
        pEd->SetText( EMPTY_OUSTRING );
    else if( nSelPos == nUserDefPos && !pLb->IsTravelSelect() && pEd->GetText().isEmpty())
        pLb->SelectEntryPos( 0 );
    else if( nSelPos >= nFirstCustomPos )
        pEd->SetText( *static_cast< OUString* >( pLb->GetEntryData( nSelPos ) ) );
}

IMPL_LINK( ScPrintAreasDlg, Impl_ModifyHdl, Edit&, rEd, void )
{
    ListBox* pLb = nullptr;

    // list box positions of specific entries, default to "repeat row/column" list boxes
    sal_Int32 nUserDefPos = SC_AREASDLG_RR_USER;
    sal_Int32 nFirstCustomPos = SC_AREASDLG_RR_OFFSET;

    if( &rEd == pEdPrintArea )
    {
        pLb = pLbPrintArea;
        nUserDefPos = SC_AREASDLG_PR_USER;
        nFirstCustomPos = SC_AREASDLG_PR_SELECT;    // "Selection" and following
    }
    else if( &rEd == pEdRepeatCol )
        pLb = pLbRepeatCol;
    else if( &rEd == pEdRepeatRow )
        pLb = pLbRepeatRow;
    else
        return;

    // set list box selection according to edit field
    const sal_Int32 nEntryCount = pLb->GetEntryCount();
    OUString aStrEd( rEd.GetText() );
    OUString aEdUpper = aStrEd.toAsciiUpperCase();

    if ( (nEntryCount > nFirstCustomPos) && !aStrEd.isEmpty() )
    {
        bool    bFound  = false;
        sal_Int32 i;

        for ( i=nFirstCustomPos; i<nEntryCount && !bFound; i++ )
        {
            OUString* pSymbol = static_cast<OUString*>(pLb->GetEntryData( i ));
            bFound  = ( (*pSymbol)  ==aStrEd || (*pSymbol) == aEdUpper );
        }

        pLb->SelectEntryPos( bFound ? i-1 : nUserDefPos );
    }
    else
        pLb->SelectEntryPos( !aStrEd.isEmpty() ? nUserDefPos : 0 );
}

// global functions:

// TODO: It might make sense to move these functions to address.?xx. -kohei

static bool lcl_CheckOne_OOO( const OUString& rStr, bool bIsRow, SCCOLROW& rVal )
{
    // Allowed syntax for rStr:
    // Row: [$]1-MAXTAB
    // Col: [$]A-IV

    OUString    aStr    = rStr;
    sal_Int32   nLen    = aStr.getLength();
    SCCOLROW    nNum    = 0;
    bool    bStrOk  = ( nLen > 0 ) && ( bIsRow ? ( nLen < 6 ) : ( nLen < 4 ) );

    if ( bStrOk )
    {
        if ( '$' == aStr[0] )
            aStr = aStr.copy( 1 );

        if ( bIsRow )
        {
            bStrOk = CharClass::isAsciiNumeric(aStr);

            if ( bStrOk )
            {
                sal_Int32 n = aStr.toInt32();

                bStrOk = (n > 0) && ( n <= MAXROWCOUNT );
                if ( bStrOk )
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

static bool lcl_CheckOne_XL_A1( const OUString& rStr, bool bIsRow, SCCOLROW& rVal )
{
    // XL A1 style is identical to OOO one for print range formats.
    return lcl_CheckOne_OOO(rStr, bIsRow, rVal);
}

static bool lcl_CheckOne_XL_R1C1( const OUString& rStr, bool bIsRow, SCCOLROW& rVal )
{
    sal_Int32 nLen = rStr.getLength();
    if (nLen <= 1)
        // There must be at least two characters.
        return false;

    const sal_Unicode preUpper = bIsRow ? 'R' : 'C';
    const sal_Unicode preLower = bIsRow ? 'r' : 'c';
    if (rStr[0] != preUpper && rStr[0] != preLower)
        return false;

    OUString aNumStr = rStr.copy(1);
    if (!CharClass::isAsciiNumeric(aNumStr))
        return false;

    sal_Int32 nNum = aNumStr.toInt32();

    if (nNum <= 0)
        return false;

    if ((bIsRow && nNum > MAXROWCOUNT) || (!bIsRow && nNum > MAXCOLCOUNT))
        return false;

    rVal = static_cast<SCCOLROW>(nNum-1);
    return true;
}

static bool lcl_CheckRepeatOne( const OUString& rStr, formula::FormulaGrammar::AddressConvention eConv, bool bIsRow, SCCOLROW& rVal )
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

static bool lcl_CheckRepeatString( const OUString& rStr, const ScDocument* pDoc, bool bIsRow, ScRange* pRange )
{
    // Row: [valid row] rsep [valid row]
    // Col: [valid col] rsep [valid col]

    const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
    const sal_Unicode rsep = ScCompiler::GetNativeSymbolChar(ocRange);

    if (pRange)
    {
        // initialize the range value.
        pRange->aStart.SetCol(0);
        pRange->aStart.SetRow(0);
        pRange->aEnd.SetCol(0);
        pRange->aEnd.SetRow(0);
    }

    OUString aBuf;
    SCCOLROW nVal = 0;
    sal_Int32 nLen = rStr.getLength();
    bool bEndPos = false;
    for( sal_Int32 i = 0; i < nLen; ++i )
    {
        const sal_Unicode c = rStr[i];
        if (c == rsep)
        {
            if (bEndPos)
                // We aren't supposed to have more than one range separator.
                return false;

            // range separator
            if (aBuf.isEmpty())
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

            aBuf.clear();
            bEndPos = true;
        }
        else
            aBuf += OUStringLiteral1(c);
    }

    if (!aBuf.isEmpty())
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

static void lcl_GetRepeatRangeString( const ScRange* pRange, const ScDocument* pDoc, bool bIsRow, OUString& rStr )
{
    rStr.clear();
    if (!pRange)
        return;

    const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
    const ScAddress& rStart = pRange->aStart;
    const ScAddress& rEnd   = pRange->aEnd;

    const ScRefFlags nFmt = bIsRow
                            ? (ScRefFlags::ROW_VALID | ScRefFlags::ROW_ABS)
                            : (ScRefFlags::COL_VALID | ScRefFlags::COL_ABS);
    rStr += rStart.Format(nFmt, pDoc, eConv);
    if ((bIsRow && rStart.Row() != rEnd.Row()) || (!bIsRow && rStart.Col() != rEnd.Col()))
    {
        rStr += ScCompiler::GetNativeSymbol(ocRange);
        rStr += rEnd.Format(nFmt, pDoc, eConv);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
