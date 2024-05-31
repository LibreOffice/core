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

#include <sfx2/dispatch.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <uiitems.hxx>
#include <document.hxx>
#include <scresid.hxx>
#include <sc.hrc>
#include <strings.hrc>
#include <reffact.hxx>

#include <tabopdlg.hxx>


ScTabOpDlg::ScTabOpDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                       ScDocument* pDocument,
                       const ScRefAddress& rCursorPos )
    : ScAnyRefDlgController(pB, pCW, pParent, u"modules/scalc/ui/multipleoperationsdialog.ui"_ustr,
                            u"MultipleOperationsDialog"_ustr)
    , theFormulaCell(rCursorPos)
    , pDoc(pDocument)
    , nCurTab(theFormulaCell.Tab())
    , bDlgLostFocus(false)
    , errMsgNoFormula(ScResId(STR_NOFORMULASPECIFIED))
    , errMsgNoColRow(ScResId(STR_NOCOLROW))
    , errMsgWrongFormula(ScResId(STR_WRONGFORMULA))
    , errMsgWrongRowCol(ScResId(STR_WRONGROWCOL))
    , errMsgNoColFormula(ScResId(STR_NOCOLFORMULA))
    , errMsgNoRowFormula(ScResId(STR_NOROWFORMULA))
    , m_pEdActive(nullptr)
    , m_xFtFormulaRange(m_xBuilder->weld_label(u"formulasft"_ustr))
    , m_xEdFormulaRange(new formula::RefEdit(m_xBuilder->weld_entry(u"formulas"_ustr)))
    , m_xRBFormulaRange(new formula::RefButton(m_xBuilder->weld_button(u"formulasref"_ustr)))
    , m_xFtRowCell(m_xBuilder->weld_label(u"rowft"_ustr))
    , m_xEdRowCell(new formula::RefEdit(m_xBuilder->weld_entry(u"row"_ustr)))
    , m_xRBRowCell(new formula::RefButton(m_xBuilder->weld_button(u"rowref"_ustr)))
    , m_xFtColCell(m_xBuilder->weld_label(u"colft"_ustr))
    , m_xEdColCell(new formula::RefEdit(m_xBuilder->weld_entry(u"col"_ustr)))
    , m_xRBColCell(new formula::RefButton(m_xBuilder->weld_button(u"colref"_ustr)))
    , m_xBtnOk(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xBtnCancel(m_xBuilder->weld_button(u"cancel"_ustr))
{
    m_xEdFormulaRange->SetReferences(this, m_xFtFormulaRange.get());
    m_xRBFormulaRange->SetReferences(this, m_xEdFormulaRange.get());

    m_xEdRowCell->SetReferences(this, m_xFtRowCell.get());
    m_xRBRowCell->SetReferences(this, m_xEdRowCell.get());

    m_xEdColCell->SetReferences(this, m_xFtColCell.get());
    m_xRBColCell->SetReferences(this, m_xEdColCell.get());

    Init();
}

ScTabOpDlg::~ScTabOpDlg()
{
}

void ScTabOpDlg::Init()
{
    m_xBtnOk->connect_clicked( LINK( this, ScTabOpDlg, BtnHdl ) );
    m_xBtnCancel->connect_clicked( LINK( this, ScTabOpDlg, BtnHdl ) );

    Link<formula::RefEdit&,void> aEditLink = LINK( this, ScTabOpDlg, GetEditFocusHdl );
    m_xEdFormulaRange->SetGetFocusHdl( aEditLink );
    m_xEdRowCell->SetGetFocusHdl( aEditLink );
    m_xEdColCell->SetGetFocusHdl( aEditLink );

    Link<formula::RefButton&,void> aButtonLink = LINK( this, ScTabOpDlg, GetButtonFocusHdl );
    m_xRBFormulaRange->SetGetFocusHdl( aButtonLink );
    m_xRBRowCell->SetGetFocusHdl( aButtonLink );
    m_xRBColCell->SetGetFocusHdl( aButtonLink );

    aEditLink = LINK( this, ScTabOpDlg, LoseEditFocusHdl );
    m_xEdFormulaRange->SetLoseFocusHdl( aEditLink );
    m_xEdRowCell->SetLoseFocusHdl( aEditLink );
    m_xEdColCell->SetLoseFocusHdl( aEditLink );

    aButtonLink = LINK( this, ScTabOpDlg, LoseButtonFocusHdl );
    m_xRBFormulaRange->SetLoseFocusHdl( aButtonLink );
    m_xRBRowCell->SetLoseFocusHdl( aButtonLink );
    m_xRBColCell->SetLoseFocusHdl( aButtonLink );

    m_xEdFormulaRange->GrabFocus();
    m_pEdActive = m_xEdFormulaRange.get();
}

void ScTabOpDlg::Close()
{
    DoClose( ScTabOpDlgWrapper::GetChildWindowId() );
}

void ScTabOpDlg::SetActive()
{
    if ( bDlgLostFocus )
    {
        bDlgLostFocus = false;
        if (m_pEdActive)
            m_pEdActive->GrabFocus();
    }
    else
        m_xDialog->grab_focus();

    RefInputDone();
}

void ScTabOpDlg::SetReference( const ScRange& rRef, ScDocument& rDocP )
{
    if (!m_pEdActive)
        return;

    ScAddress::Details aDetails(rDocP.GetAddressConvention(), 0, 0);

    if ( rRef.aStart != rRef.aEnd )
        RefInputStart(m_pEdActive);

    OUString      aStr;
    ScRefFlags      nFmt = ( rRef.aStart.Tab() == nCurTab )
                            ? ScRefFlags::RANGE_ABS
                            : ScRefFlags::RANGE_ABS_3D;

    if (m_pEdActive == m_xEdFormulaRange.get())
    {
        theFormulaCell.Set( rRef.aStart, false, false, false);
        theFormulaEnd.Set( rRef.aEnd, false, false, false);
        aStr = rRef.Format(rDocP, nFmt, aDetails);
    }
    else if (m_pEdActive == m_xEdRowCell.get())
    {
        theRowCell.Set( rRef.aStart, false, false, false);
        aStr = rRef.aStart.Format(nFmt, &rDocP, aDetails);
    }
    else if (m_pEdActive == m_xEdColCell.get())
    {
        theColCell.Set( rRef.aStart, false, false, false);
        aStr = rRef.aStart.Format(nFmt, &rDocP, aDetails);
    }

    m_pEdActive->SetRefString( aStr );
}

void ScTabOpDlg::RaiseError( ScTabOpErr eError )
{
    const OUString* pMsg = &errMsgNoFormula;
    formula::RefEdit* pEd  = m_xEdFormulaRange.get();

    switch ( eError )
    {
        case TABOPERR_NOFORMULA:
            pMsg = &errMsgNoFormula;
            pEd  = m_xEdFormulaRange.get();
            break;

        case TABOPERR_NOCOLROW:
            pMsg = &errMsgNoColRow;
            pEd  = m_xEdRowCell.get();
            break;

        case TABOPERR_WRONGFORMULA:
            pMsg = &errMsgWrongFormula;
            pEd  = m_xEdFormulaRange.get();
            break;

        case TABOPERR_WRONGROW:
            pMsg = &errMsgWrongRowCol;
            pEd  = m_xEdRowCell.get();
            break;

        case TABOPERR_NOCOLFORMULA:
            pMsg = &errMsgNoColFormula;
            pEd  = m_xEdFormulaRange.get();
            break;

        case TABOPERR_WRONGCOL:
            pMsg = &errMsgWrongRowCol;
            pEd  = m_xEdColCell.get();
            break;

        case TABOPERR_NOROWFORMULA:
            pMsg = &errMsgNoRowFormula;
            pEd  = m_xEdFormulaRange.get();
            break;
    }

    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                              VclMessageType::Error, VclButtonsType::OkCancel, *pMsg));
    xBox->run();
    pEd->GrabFocus();
}

static bool lcl_Parse( const OUString& rString, const ScDocument& rDoc, SCTAB nCurTab,
                ScRefAddress& rStart, ScRefAddress& rEnd )
{
    bool bRet = false;
    const formula::FormulaGrammar::AddressConvention eConv = rDoc.GetAddressConvention();
    if ( rString.indexOf(':') != -1 )
        bRet = ConvertDoubleRef( rDoc, rString, nCurTab, rStart, rEnd, eConv );
    else
    {
        bRet = ConvertSingleRef( rDoc, rString, nCurTab, rStart, eConv );
        rEnd = rStart;
    }
    return bRet;
}

// Handler:

IMPL_LINK(ScTabOpDlg, BtnHdl, weld::Button&, rBtn, void)
{
    if (&rBtn == m_xBtnOk.get())
    {
        ScTabOpParam::Mode eMode = ScTabOpParam::Column;
        sal_uInt16 nError = 0;

        // The following code checks:
        // 1. do the strings contain correct cell references / defined names?
        // 2. is formula range row if row is empty or column if column is empty
        //    or single reference if both?
        // 3. is at least one of row or column non-empty?

        if (m_xEdFormulaRange->GetText().isEmpty())
            nError = TABOPERR_NOFORMULA;
        else if (m_xEdRowCell->GetText().isEmpty() &&
                 m_xEdColCell->GetText().isEmpty())
            nError = TABOPERR_NOCOLROW;
        else if ( !lcl_Parse( m_xEdFormulaRange->GetText(), *pDoc, nCurTab,
                                theFormulaCell, theFormulaEnd ) )
            nError = TABOPERR_WRONGFORMULA;
        else
        {
            const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
            if (!m_xEdRowCell->GetText().isEmpty())
            {
                if (!ConvertSingleRef( *pDoc, m_xEdRowCell->GetText(), nCurTab,
                                       theRowCell, eConv ))
                    nError = TABOPERR_WRONGROW;
                else
                {
                    if (m_xEdColCell->GetText().isEmpty() &&
                        theFormulaCell.Col() != theFormulaEnd.Col())
                        nError = TABOPERR_NOCOLFORMULA;
                    else
                        eMode = ScTabOpParam::Row;
                }
            }
            if (!m_xEdColCell->GetText().isEmpty())
            {
                if (!ConvertSingleRef( *pDoc, m_xEdColCell->GetText(), nCurTab,
                                       theColCell, eConv ))
                    nError = TABOPERR_WRONGCOL;
                else
                {
                    if (eMode == ScTabOpParam::Row)                         // both
                    {
                        eMode = ScTabOpParam::Both;
                        ConvertSingleRef( *pDoc, m_xEdFormulaRange->GetText(), nCurTab,
                                          theFormulaCell, eConv );
                    }
                    else if (theFormulaCell.Row() != theFormulaEnd.Row())
                        nError = TABOPERR_NOROWFORMULA;
                    else
                        eMode = ScTabOpParam::Column;
                }
            }
        }

        if (nError)
            RaiseError( static_cast<ScTabOpErr>(nError) );
        else
        {
            ScTabOpParam aOutParam(theFormulaCell, theFormulaEnd, theRowCell, theColCell, eMode);
            ScTabOpItem  aOutItem( SID_TABOP, &aOutParam );

            SetDispatcherLock( false );
            SwitchToDocument();
            GetBindings().GetDispatcher()->ExecuteList(SID_TABOP,
                                      SfxCallMode::SLOT | SfxCallMode::RECORD,
                                      { &aOutItem });
            response(RET_OK);
        }
    }
    else if (&rBtn == m_xBtnCancel.get())
        response(RET_CANCEL);
}

IMPL_LINK( ScTabOpDlg, GetEditFocusHdl, formula::RefEdit&, rCtrl, void )
{
    if (&rCtrl == m_xEdFormulaRange.get())
        m_pEdActive = m_xEdFormulaRange.get();
    else if (&rCtrl == m_xEdRowCell.get())
        m_pEdActive = m_xEdRowCell.get();
    else if (&rCtrl == m_xEdColCell.get())
        m_pEdActive = m_xEdColCell.get();
    else
        m_pEdActive = nullptr;

    if( m_pEdActive )
        m_pEdActive->SelectAll();
}

IMPL_LINK( ScTabOpDlg, GetButtonFocusHdl, formula::RefButton&, rCtrl, void )
{
    if (&rCtrl == m_xRBFormulaRange.get())
        m_pEdActive = m_xEdFormulaRange.get();
    else if (&rCtrl == m_xRBRowCell.get())
        m_pEdActive = m_xEdRowCell.get();
    else if (&rCtrl == m_xRBColCell.get())
        m_pEdActive = m_xEdColCell.get();
    else
        m_pEdActive = nullptr;

    if( m_pEdActive )
        m_pEdActive->SelectAll();
}

IMPL_LINK_NOARG(ScTabOpDlg, LoseEditFocusHdl, formula::RefEdit&, void)
{
    bDlgLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG(ScTabOpDlg, LoseButtonFocusHdl, formula::RefButton&, void)
{
    bDlgLostFocus = !m_xDialog->has_toplevel_focus();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
