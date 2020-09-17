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
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <unotools/charclass.hxx>

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

static bool lcl_CheckRepeatString( const OUString& rStr, const ScDocument& rDoc, bool bIsRow, ScRange* pRange );
static void lcl_GetRepeatRangeString( const ScRange* pRange, const ScDocument& rDoc, bool bIsRow, OUString& rStr );

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


ScPrintAreasDlg::ScPrintAreasDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent)
    : ScAnyRefDlgController(pB, pCW, pParent, "modules/scalc/ui/printareasdialog.ui", "PrintAreasDialog")
    , bDlgLostFocus(false)
    , pDoc(nullptr)
    , pViewData(nullptr)
    , nCurTab(0)
    , m_xLbPrintArea(m_xBuilder->weld_combo_box("lbprintarea"))
    , m_xEdPrintArea(new formula::RefEdit(m_xBuilder->weld_entry("edprintarea")))
    , m_xRbPrintArea(new formula::RefButton(m_xBuilder->weld_button("rbprintarea")))
    , m_xLbRepeatRow(m_xBuilder->weld_combo_box("lbrepeatrow"))
    , m_xEdRepeatRow(new formula::RefEdit(m_xBuilder->weld_entry("edrepeatrow")))
    , m_xRbRepeatRow(new formula::RefButton(m_xBuilder->weld_button("rbrepeatrow")))
    , m_xLbRepeatCol(m_xBuilder->weld_combo_box("lbrepeatcol"))
    , m_xEdRepeatCol(new formula::RefEdit(m_xBuilder->weld_entry("edrepeatcol")))
    , m_xRbRepeatCol(new formula::RefButton(m_xBuilder->weld_button("rbrepeatcol")))
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
    , m_xBtnCancel(m_xBuilder->weld_button("cancel"))
    , m_xPrintFrame(m_xBuilder->weld_frame("printframe"))
    , m_xRowFrame(m_xBuilder->weld_frame("rowframe"))
    , m_xColFrame(m_xBuilder->weld_frame("colframe"))
    , m_xPrintFrameFT(m_xPrintFrame->weld_label_widget())
    , m_xRowFrameFT(m_xRowFrame->weld_label_widget())
    , m_xColFrameFT(m_xColFrame->weld_label_widget())
{
    m_xEdPrintArea->SetReferences(this, m_xPrintFrameFT.get());
    m_pRefInputEdit = m_xEdPrintArea.get();
    m_xRbPrintArea->SetReferences(this, m_xEdPrintArea.get());

    m_xEdRepeatRow->SetReferences(this, m_xRowFrameFT.get());
    m_xRbRepeatRow->SetReferences(this, m_xEdRepeatRow.get());

    m_xEdRepeatCol->SetReferences(this, m_xColFrameFT.get());
    m_xRbRepeatCol->SetReferences(this, m_xEdRepeatCol.get());

    ScTabViewShell* pScViewSh = dynamic_cast<ScTabViewShell*>( SfxViewShell::Current()  );
    ScDocShell* pScDocSh = dynamic_cast<ScDocShell*>(SfxObjectShell::Current());
    assert(pScDocSh && "Current DocumentShell not found :-(");

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
}

void ScPrintAreasDlg::Close()
{
    DoClose( ScPrintAreasDlgWrapper::GetChildWindowId() );
}

bool ScPrintAreasDlg::IsTableLocked() const
{
    //  Printing areas are per table, therefore it makes no sense,
    //  to switch the table during input

    return true;
}

void ScPrintAreasDlg::SetReference( const ScRange& rRef, ScDocument& /* rDoc */ )
{
    if ( !m_pRefInputEdit )
        return;

    if ( rRef.aStart != rRef.aEnd )
        RefInputStart( m_pRefInputEdit );

    OUString  aStr;
    const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

    if (m_xEdPrintArea.get() == m_pRefInputEdit)
    {
        aStr = rRef.Format(*pDoc, ScRefFlags::RANGE_ABS, eConv);
        OUString aVal = m_xEdPrintArea->GetText();
        Selection aSel = m_xEdPrintArea->GetSelection();
        aSel.Justify();
        aVal = aVal.replaceAt( aSel.Min(), aSel.Len(), aStr );
        Selection aNewSel( aSel.Min(), aSel.Min()+aStr.getLength() );
        m_xEdPrintArea->SetRefString( aVal );
        m_xEdPrintArea->SetSelection( aNewSel );
    }
    else
    {
        bool bRow = ( m_xEdRepeatRow.get() == m_pRefInputEdit );
        lcl_GetRepeatRangeString(&rRef, *pDoc, bRow, aStr);
        m_pRefInputEdit->SetRefString( aStr );
    }
    Impl_ModifyHdl( *m_pRefInputEdit );
}

void ScPrintAreasDlg::AddRefEntry()
{
    if (m_pRefInputEdit == m_xEdPrintArea.get())
    {
        const sal_Unicode sep = ScCompiler::GetNativeSymbolChar(ocSep);
        OUString aVal = m_xEdPrintArea->GetText() + OUStringChar(sep);
        m_xEdPrintArea->SetText(aVal);

        sal_Int32 nLen = aVal.getLength();
        m_xEdPrintArea->SetSelection( Selection( nLen, nLen ) );

        Impl_ModifyHdl( *m_xEdPrintArea );
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

        if ( m_pRefInputEdit )
        {
            m_pRefInputEdit->GrabFocus();
            Impl_ModifyHdl( *m_pRefInputEdit );
        }
    }
    else
        m_xDialog->grab_focus();

    RefInputDone();
}

void ScPrintAreasDlg::Impl_Reset()
{
    OUString        aStrRange;
    const ScRange*  pRepeatColRange = pDoc->GetRepeatColRange( nCurTab );
    const ScRange*  pRepeatRowRange = pDoc->GetRepeatRowRange( nCurTab );

    m_xEdPrintArea->SetModifyHdl   ( HDL(Impl_ModifyHdl) );
    m_xEdRepeatRow->SetModifyHdl   ( HDL(Impl_ModifyHdl) );
    m_xEdRepeatCol->SetModifyHdl   ( HDL(Impl_ModifyHdl) );
    m_xEdPrintArea->SetGetFocusHdl( HDL(Impl_GetEditFocusHdl) );
    m_xEdRepeatRow->SetGetFocusHdl( HDL(Impl_GetEditFocusHdl) );
    m_xEdRepeatCol->SetGetFocusHdl( HDL(Impl_GetEditFocusHdl) );
    m_xLbPrintArea->connect_focus_in( HDL(Impl_GetFocusHdl) );
    m_xLbRepeatRow->connect_focus_in( HDL(Impl_GetFocusHdl) );
    m_xLbRepeatCol->connect_focus_in( HDL(Impl_GetFocusHdl) );
    m_xLbPrintArea->connect_changed( HDL(Impl_SelectHdl) );
    m_xLbRepeatRow->connect_changed( HDL(Impl_SelectHdl) );
    m_xLbRepeatCol->connect_changed( HDL(Impl_SelectHdl) );
    m_xBtnOk->connect_clicked( HDL(Impl_BtnHdl)    );
    m_xBtnCancel->connect_clicked( HDL(Impl_BtnHdl)    );

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
                aStrRange += OUStringChar(sep);
            aStrRange += pPrintRange->Format(*pDoc, ScRefFlags::RANGE_ABS, eConv);
        }
    }
    m_xEdPrintArea->SetText( aStrRange );

    // repeat row

    lcl_GetRepeatRangeString(pRepeatRowRange, *pDoc, true, aStrRange);
    m_xEdRepeatRow->SetText( aStrRange );

    // repeat column

    lcl_GetRepeatRangeString(pRepeatColRange, *pDoc, false, aStrRange);
    m_xEdRepeatCol->SetText( aStrRange );

    Impl_ModifyHdl( *m_xEdPrintArea );
    Impl_ModifyHdl( *m_xEdRepeatRow );
    Impl_ModifyHdl( *m_xEdRepeatCol );
    if( pDoc->IsPrintEntireSheet( nCurTab ) )
        m_xLbPrintArea->set_active(SC_AREASDLG_PR_ENTIRE);

    m_xEdPrintArea->SaveValue();   // save for FillItemSet():
    m_xEdRepeatRow->SaveValue();
    m_xEdRepeatCol->SaveValue();
}

bool ScPrintAreasDlg::Impl_GetItem( const formula::RefEdit* pEd, SfxStringItem& rItem )
{
    OUString  aRangeStr = pEd->GetText();
    bool bDataChanged = pEd->IsValueChangedFromSaved();

    if ( !aRangeStr.isEmpty() && m_xEdPrintArea.get() != pEd )
    {
        ScRange aRange;
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
        lcl_CheckRepeatString(aRangeStr, *pDoc, m_xEdRepeatRow.get() == pEd, &aRange);
        aRangeStr = aRange.Format(*pDoc, ScRefFlags::RANGE_ABS, eConv);
    }

    rItem.SetValue( aRangeStr );

    return bDataChanged;
}

bool ScPrintAreasDlg::Impl_CheckRefStrings()
{
    bool        bOk = false;
    OUString      aStrPrintArea   = m_xEdPrintArea->GetText();
    OUString      aStrRepeatRow   = m_xEdRepeatRow->GetText();
    OUString      aStrRepeatCol   = m_xEdRepeatCol->GetText();

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
            ScRefFlags nResult = aRange.Parse( aOne, *pDoc, eConv );
            if ((nResult & nValidRange) != nValidRange)
            {
                ScRefFlags nAddrResult = aAddr.Parse( aOne, *pDoc, eConv );
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
        bRepeatRowOk = lcl_CheckRepeatString(aStrRepeatRow, *pDoc, true, nullptr);

    bool bRepeatColOk = aStrRepeatCol.isEmpty();
    if ( !bRepeatColOk )
        bRepeatColOk = lcl_CheckRepeatString(aStrRepeatCol, *pDoc, false, nullptr);

    // error messages

    bOk = (bPrintAreaOk && bRepeatRowOk && bRepeatColOk);

    if ( !bOk )
    {
        formula::RefEdit* pEd = nullptr;

        if ( !bPrintAreaOk ) pEd = m_xEdPrintArea.get();
        else if ( !bRepeatRowOk ) pEd = m_xEdRepeatRow.get();
        else if ( !bRepeatColOk ) pEd = m_xEdRepeatCol.get();

        ERRORBOX(m_xDialog.get(), STR_INVALID_TABREF);

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
        aStrRange = aRange.Format(*pDoc, ScRefFlags::RANGE_ABS, eConv);
    else
    {
        ScRangeListRef aList( new ScRangeList );
        pViewData->GetMarkData().FillRangeListWithMarks( aList.get(), false );
        aList->Format(aStrRange, ScRefFlags::RANGE_ABS, *pDoc, eConv);
    }

    m_xLbPrintArea->set_id(SC_AREASDLG_PR_SELECT, aStrRange);

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
        if (aRange.ParseAny(aSymbol, *pDoc, eConv) & ScRefFlags::VALID)
        {
            if (rEntry.second->HasType(ScRangeData::Type::PrintArea))
            {
                aSymbol = aRange.Format(*pDoc, ScRefFlags::RANGE_ABS, eConv);
                m_xLbPrintArea->append(aSymbol, aName);
            }

            if (rEntry.second->HasType(ScRangeData::Type::RowHeader))
            {
                lcl_GetRepeatRangeString(&aRange, *pDoc, true, aSymbol);
                m_xLbRepeatRow->append(aSymbol, aName);
            }

            if (rEntry.second->HasType(ScRangeData::Type::ColHeader))
            {
                lcl_GetRepeatRangeString(&aRange, *pDoc, false, aSymbol);
                m_xLbRepeatCol->append(aSymbol, aName);
            }
        }
    }
}

// Handler:

IMPL_LINK(ScPrintAreasDlg, Impl_BtnHdl, weld::Button&, rBtn, void)
{
    if (m_xBtnOk.get() == &rBtn)
    {
        if ( Impl_CheckRefStrings() )
        {
            SfxStringItem   aPrintArea( SID_CHANGE_PRINTAREA, "" );
            SfxStringItem   aRepeatRow( FN_PARAM_2, "" );
            SfxStringItem   aRepeatCol( FN_PARAM_3, "" );

            // Printing area changed?

            // first try the list box, if "Entire sheet" is selected
            bool bEntireSheet = (m_xLbPrintArea->get_active() == SC_AREASDLG_PR_ENTIRE);
            SfxBoolItem aEntireSheet( FN_PARAM_4, bEntireSheet );

            bool bDataChanged = bEntireSheet != pDoc->IsPrintEntireSheet( nCurTab );
            if( !bEntireSheet )
            {
                // if new list box selection is not "Entire sheet", get the edit field contents
                bDataChanged |= Impl_GetItem( m_xEdPrintArea.get(), aPrintArea );
            }

            // Repeat row changed?

            bDataChanged |= Impl_GetItem( m_xEdRepeatRow.get(), aRepeatRow );

            // Repeat column changed?

            bDataChanged |= Impl_GetItem( m_xEdRepeatCol.get(), aRepeatCol );

            if ( bDataChanged )
            {
                SetDispatcherLock( false );
                SwitchToDocument();
                GetBindings().GetDispatcher()->ExecuteList(SID_CHANGE_PRINTAREA,
                      SfxCallMode::SLOT | SfxCallMode::RECORD,
                      { &aPrintArea, &aRepeatRow, &aRepeatCol, &aEntireSheet });
            }

            response(RET_OK);
        }
    }
    else if (m_xBtnCancel.get() == &rBtn)
        response(RET_CANCEL);
}

IMPL_LINK(ScPrintAreasDlg, Impl_GetEditFocusHdl, formula::RefEdit&, rCtrl, void)
{
    m_pRefInputEdit = &rCtrl;
}

IMPL_LINK(ScPrintAreasDlg, Impl_GetFocusHdl, weld::Widget&, rCtrl, void)
{
    if (&rCtrl == m_xLbPrintArea.get())
        m_pRefInputEdit = m_xEdPrintArea.get();
    else if (&rCtrl == m_xLbRepeatRow.get())
        m_pRefInputEdit = m_xEdRepeatRow.get();
    else if (&rCtrl == m_xLbRepeatCol.get())
        m_pRefInputEdit = m_xEdRepeatCol.get();
}

IMPL_LINK( ScPrintAreasDlg, Impl_SelectHdl, weld::ComboBox&, rLb, void )
{
    const sal_Int32 nSelPos = rLb.get_active();
    formula::RefEdit* pEd = nullptr;

    // list box positions of specific entries, default to "repeat row/column" list boxes
    sal_Int32 nAllSheetPos = SC_AREASDLG_RR_NONE;
    sal_Int32 nFirstCustomPos = SC_AREASDLG_RR_OFFSET;

    // find edit field for list box, and list box positions
    if (&rLb == m_xLbPrintArea.get())
    {
        pEd = m_xEdPrintArea.get();
        nAllSheetPos = SC_AREASDLG_PR_ENTIRE;
        nFirstCustomPos = SC_AREASDLG_PR_SELECT;    // "Selection" and following
    }
    else if (&rLb == m_xLbRepeatCol.get())
        pEd = m_xEdRepeatCol.get();
    else if (&rLb == m_xLbRepeatRow.get())
        pEd = m_xEdRepeatRow.get();
    else
        return;

    // fill edit field according to list box selection
    if( (nSelPos == 0) || (nSelPos == nAllSheetPos) )
        pEd->SetText( EMPTY_OUSTRING );
    else if( nSelPos >= nFirstCustomPos )
        pEd->SetText(rLb.get_id(nSelPos));
}

IMPL_LINK( ScPrintAreasDlg, Impl_ModifyHdl, formula::RefEdit&, rEd, void )
{
    weld::ComboBox* pLb = nullptr;

    // list box positions of specific entries, default to "repeat row/column" list boxes
    sal_Int32 nUserDefPos = SC_AREASDLG_RR_USER;
    sal_Int32 nFirstCustomPos = SC_AREASDLG_RR_OFFSET;

    if( &rEd == m_xEdPrintArea.get() )
    {
        pLb = m_xLbPrintArea.get();
        nUserDefPos = SC_AREASDLG_PR_USER;
        nFirstCustomPos = SC_AREASDLG_PR_SELECT;    // "Selection" and following
    }
    else if( &rEd == m_xEdRepeatCol.get() )
        pLb = m_xLbRepeatCol.get();
    else if( &rEd == m_xEdRepeatRow.get() )
        pLb = m_xLbRepeatRow.get();
    else
        return;

    // set list box selection according to edit field
    const sal_Int32 nEntryCount = pLb->get_count();
    OUString aStrEd( rEd.GetText() );
    OUString aEdUpper = aStrEd.toAsciiUpperCase();

    if ( (nEntryCount > nFirstCustomPos) && !aStrEd.isEmpty() )
    {
        bool    bFound  = false;
        sal_Int32 i;

        for ( i=nFirstCustomPos; i<nEntryCount && !bFound; i++ )
        {
            const OUString& rSymbol = pLb->get_id(i);
            bFound = (rSymbol == aStrEd || rSymbol == aEdUpper);
        }

        pLb->set_active( bFound ? i-1 : nUserDefPos );
    }
    else
        pLb->set_active( !aStrEd.isEmpty() ? nUserDefPos : 0 );
}

// global functions:

// TODO: It might make sense to move these functions to address.?xx. -kohei

static bool lcl_CheckOne_OOO( const ScDocument& rDoc, const OUString& rStr, bool bIsRow, SCCOLROW& rVal )
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

                bStrOk = (n > 0) && ( n <= rDoc.GetSheetLimits().GetMaxRowCount() );
                if ( bStrOk )
                    nNum = static_cast<SCCOLROW>(n - 1);
            }
        }
        else
        {
            SCCOL nCol = 0;
            bStrOk = ::AlphaToCol(rDoc, nCol, aStr);
            nNum = nCol;
        }
    }

    if ( bStrOk )
        rVal = nNum;

    return bStrOk;
}

static bool lcl_CheckOne_XL_A1( const ScDocument& rDoc, const OUString& rStr, bool bIsRow, SCCOLROW& rVal )
{
    // XL A1 style is identical to OOO one for print range formats.
    return lcl_CheckOne_OOO(rDoc, rStr, bIsRow, rVal);
}

static bool lcl_CheckOne_XL_R1C1( const ScDocument& rDoc, const OUString& rStr, bool bIsRow, SCCOLROW& rVal )
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

    if ((bIsRow && nNum > rDoc.GetSheetLimits().GetMaxRowCount()) || (!bIsRow && nNum > MAXCOLCOUNT))
        return false;

    rVal = static_cast<SCCOLROW>(nNum-1);
    return true;
}

static bool lcl_CheckRepeatOne( const ScDocument& rDoc, const OUString& rStr, formula::FormulaGrammar::AddressConvention eConv, bool bIsRow, SCCOLROW& rVal )
{
    switch (eConv)
    {
        case formula::FormulaGrammar::CONV_OOO:
            return lcl_CheckOne_OOO(rDoc, rStr, bIsRow, rVal);
        case formula::FormulaGrammar::CONV_XL_A1:
            return lcl_CheckOne_XL_A1(rDoc, rStr, bIsRow, rVal);
        case formula::FormulaGrammar::CONV_XL_R1C1:
            return lcl_CheckOne_XL_R1C1(rDoc, rStr, bIsRow, rVal);
        default:
        {
            // added to avoid warnings
        }
    }
    return false;
}

static bool lcl_CheckRepeatString( const OUString& rStr, const ScDocument& rDoc, bool bIsRow, ScRange* pRange )
{
    // Row: [valid row] rsep [valid row]
    // Col: [valid col] rsep [valid col]

    const formula::FormulaGrammar::AddressConvention eConv = rDoc.GetAddressConvention();
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

            bool bRes = lcl_CheckRepeatOne(rDoc, aBuf, eConv, bIsRow, nVal);
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
            aBuf += OUStringChar(c);
    }

    if (!aBuf.isEmpty())
    {
        bool bRes = lcl_CheckRepeatOne(rDoc, aBuf, eConv, bIsRow, nVal);
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

static void lcl_GetRepeatRangeString( const ScRange* pRange, const ScDocument& rDoc, bool bIsRow, OUString& rStr )
{
    rStr.clear();
    if (!pRange)
        return;

    const formula::FormulaGrammar::AddressConvention eConv = rDoc.GetAddressConvention();
    const ScAddress& rStart = pRange->aStart;
    const ScAddress& rEnd   = pRange->aEnd;

    const ScRefFlags nFmt = bIsRow
                            ? (ScRefFlags::ROW_VALID | ScRefFlags::ROW_ABS)
                            : (ScRefFlags::COL_VALID | ScRefFlags::COL_ABS);
    rStr += rStart.Format(nFmt, &rDoc, eConv);
    if ((bIsRow && rStart.Row() != rEnd.Row()) || (!bIsRow && rStart.Col() != rEnd.Col()))
    {
        rStr += ScCompiler::GetNativeSymbol(ocRange);
        rStr += rEnd.Format(nFmt, &rDoc, eConv);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
