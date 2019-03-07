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
#include <vcl/weld.hxx>

#include <uiitems.hxx>
#include <document.hxx>
#include <scresid.hxx>
#include <sc.hrc>
#include <strings.hrc>
#include <reffact.hxx>

#include <tabopdlg.hxx>

//  class ScTabOpDlg

ScTabOpDlg::ScTabOpDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                        ScDocument*         pDocument,
                        const ScRefAddress& rCursorPos )

    : ScAnyRefDlg(pB, pCW, pParent, "MultipleOperationsDialog",
        "modules/scalc/ui/multipleoperationsdialog.ui")
    , theFormulaCell(rCursorPos)
    , pDoc(pDocument)
    , nCurTab(theFormulaCell.Tab())
    , pEdActive(nullptr)
    , bDlgLostFocus(false)
    , errMsgNoFormula(ScResId(STR_NOFORMULASPECIFIED))
    , errMsgNoColRow(ScResId(STR_NOCOLROW))
    , errMsgWrongFormula(ScResId(STR_WRONGFORMULA))
    , errMsgWrongRowCol(ScResId(STR_WRONGROWCOL))
    , errMsgNoColFormula(ScResId(STR_NOCOLFORMULA))
    , errMsgNoRowFormula(ScResId(STR_NOROWFORMULA))
{
    get(m_pFtFormulaRange, "formulasft");
    get(m_pEdFormulaRange, "formulas");
    m_pEdFormulaRange->SetReferences(this, m_pFtFormulaRange);
    get(m_pRBFormulaRange, "formulasref");
    m_pRBFormulaRange->SetReferences(this, m_pEdFormulaRange);

    get(m_pFtRowCell, "rowft");
    get(m_pEdRowCell, "row");
    m_pEdRowCell->SetReferences(this, m_pFtRowCell);
    get(m_pRBRowCell, "rowref");
    m_pRBRowCell->SetReferences(this, m_pEdRowCell);

    get(m_pFtColCell, "colft");
    get(m_pEdColCell, "col");
    m_pEdColCell->SetReferences(this, m_pFtColCell);
    get(m_pRBColCell, "colref");
    m_pRBColCell->SetReferences(this, m_pEdColCell);

    get(m_pBtnOk, "ok");
    get(m_pBtnCancel, "cancel");

    Init();
}

ScTabOpDlg::~ScTabOpDlg()
{
    disposeOnce();
}

void ScTabOpDlg::dispose()
{
    Hide();
    m_pFtFormulaRange.clear();
    m_pEdFormulaRange.clear();
    m_pRBFormulaRange.clear();
    m_pFtRowCell.clear();
    m_pEdRowCell.clear();
    m_pRBRowCell.clear();
    m_pFtColCell.clear();
    m_pEdColCell.clear();
    m_pRBColCell.clear();
    m_pBtnOk.clear();
    m_pBtnCancel.clear();
    pEdActive.clear();
    ScAnyRefDlg::dispose();
}


void ScTabOpDlg::Init()
{
    m_pBtnOk->SetClickHdl     ( LINK( this, ScTabOpDlg, BtnHdl ) );
    m_pBtnCancel->SetClickHdl     ( LINK( this, ScTabOpDlg, BtnHdl ) );

    Link<Control&,void> aLink = LINK( this, ScTabOpDlg, GetFocusHdl );
    m_pEdFormulaRange->SetGetFocusHdl( aLink );
    m_pRBFormulaRange->SetGetFocusHdl( aLink );
    m_pEdRowCell->SetGetFocusHdl( aLink );
    m_pRBRowCell->SetGetFocusHdl( aLink );
    m_pEdColCell->SetGetFocusHdl( aLink );
    m_pRBColCell->SetGetFocusHdl( aLink );

    aLink = LINK( this, ScTabOpDlg, LoseFocusHdl );
    m_pEdFormulaRange->SetLoseFocusHdl( aLink );
    m_pRBFormulaRange->SetLoseFocusHdl( aLink );
    m_pEdRowCell->SetLoseFocusHdl( aLink );
    m_pRBRowCell->SetLoseFocusHdl( aLink );
    m_pEdColCell->SetLoseFocusHdl( aLink );
    m_pRBColCell->SetLoseFocusHdl( aLink );

    m_pEdFormulaRange->GrabFocus();
    pEdActive = m_pEdFormulaRange;

    //@BugID 54702 Enable/Disable only in the base class
    //SFX_APPWINDOW->Enable();
}

bool ScTabOpDlg::Close()
{
    return DoClose( ScTabOpDlgWrapper::GetChildWindowId() );
}

void ScTabOpDlg::SetActive()
{
    if ( bDlgLostFocus )
    {
        bDlgLostFocus = false;
        if( pEdActive )
            pEdActive->GrabFocus();
    }
    else
        GrabFocus();

    RefInputDone();
}

void ScTabOpDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( pEdActive )
    {
        ScAddress::Details aDetails(pDocP->GetAddressConvention(), 0, 0);

        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(pEdActive);

        OUString      aStr;
        ScRefFlags      nFmt = ( rRef.aStart.Tab() == nCurTab )
                                ? ScRefFlags::RANGE_ABS
                                : ScRefFlags::RANGE_ABS_3D;

        if (pEdActive == m_pEdFormulaRange)
        {
            theFormulaCell.Set( rRef.aStart, false, false, false);
            theFormulaEnd.Set( rRef.aEnd, false, false, false);
            aStr = rRef.Format(nFmt, pDocP, aDetails);
        }
        else if ( pEdActive == m_pEdRowCell )
        {
            theRowCell.Set( rRef.aStart, false, false, false);
            aStr = rRef.aStart.Format(nFmt, pDocP, aDetails);
        }
        else if ( pEdActive == m_pEdColCell )
        {
            theColCell.Set( rRef.aStart, false, false, false);
            aStr = rRef.aStart.Format(nFmt, pDocP, aDetails);
        }

        pEdActive->SetRefString( aStr );
    }
}

void ScTabOpDlg::RaiseError( ScTabOpErr eError )
{
    const OUString* pMsg = &errMsgNoFormula;
    Edit*           pEd  = m_pEdFormulaRange;

    switch ( eError )
    {
        case TABOPERR_NOFORMULA:
            pMsg = &errMsgNoFormula;
            pEd  = m_pEdFormulaRange;
            break;

        case TABOPERR_NOCOLROW:
            pMsg = &errMsgNoColRow;
            pEd  = m_pEdRowCell;
            break;

        case TABOPERR_WRONGFORMULA:
            pMsg = &errMsgWrongFormula;
            pEd  = m_pEdFormulaRange;
            break;

        case TABOPERR_WRONGROW:
            pMsg = &errMsgWrongRowCol;
            pEd  = m_pEdRowCell;
            break;

        case TABOPERR_NOCOLFORMULA:
            pMsg = &errMsgNoColFormula;
            pEd  = m_pEdFormulaRange;
            break;

        case TABOPERR_WRONGCOL:
            pMsg = &errMsgWrongRowCol;
            pEd  = m_pEdColCell;
            break;

        case TABOPERR_NOROWFORMULA:
            pMsg = &errMsgNoRowFormula;
            pEd  = m_pEdFormulaRange;
            break;
    }

    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
                                              VclMessageType::Error, VclButtonsType::OkCancel, *pMsg));
    xBox->run();
    pEd->GrabFocus();
}

static bool lcl_Parse( const OUString& rString, const ScDocument* pDoc, SCTAB nCurTab,
                ScRefAddress& rStart, ScRefAddress& rEnd )
{
    bool bRet = false;
    const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
    if ( rString.indexOf(':') != -1 )
        bRet = ConvertDoubleRef( pDoc, rString, nCurTab, rStart, rEnd, eConv );
    else
    {
        bRet = ConvertSingleRef( pDoc, rString, nCurTab, rStart, eConv );
        rEnd = rStart;
    }
    return bRet;
}

// Handler:

IMPL_LINK( ScTabOpDlg, BtnHdl, Button*, pBtn, void )
{
    if (pBtn == m_pBtnOk)
    {
        ScTabOpParam::Mode eMode = ScTabOpParam::Column;
        sal_uInt16 nError = 0;

        // The following code checks:
        // 1. do the strings contain correct cell references / defined names?
        // 2. is formula range row if row is empty or column if column is empty
        //    or single reference if both?
        // 3. is at least one of row or column non-empty?

        if (m_pEdFormulaRange->GetText().isEmpty())
            nError = TABOPERR_NOFORMULA;
        else if (m_pEdRowCell->GetText().isEmpty() &&
                 m_pEdColCell->GetText().isEmpty())
            nError = TABOPERR_NOCOLROW;
        else if ( !lcl_Parse( m_pEdFormulaRange->GetText(), pDoc, nCurTab,
                                theFormulaCell, theFormulaEnd ) )
            nError = TABOPERR_WRONGFORMULA;
        else
        {
            const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
            if (!m_pEdRowCell->GetText().isEmpty())
            {
                if (!ConvertSingleRef( pDoc, m_pEdRowCell->GetText(), nCurTab,
                                       theRowCell, eConv ))
                    nError = TABOPERR_WRONGROW;
                else
                {
                    if (m_pEdColCell->GetText().isEmpty() &&
                        theFormulaCell.Col() != theFormulaEnd.Col())
                        nError = TABOPERR_NOCOLFORMULA;
                    else
                        eMode = ScTabOpParam::Row;
                }
            }
            if (!m_pEdColCell->GetText().isEmpty())
            {
                if (!ConvertSingleRef( pDoc, m_pEdColCell->GetText(), nCurTab,
                                       theColCell, eConv ))
                    nError = TABOPERR_WRONGCOL;
                else
                {
                    if (eMode == ScTabOpParam::Row)                         // both
                    {
                        eMode = ScTabOpParam::Both;
                        ConvertSingleRef( pDoc, m_pEdFormulaRange->GetText(), nCurTab,
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
            Close();
        }
    }
    else if (pBtn == m_pBtnCancel)
        Close();
}

IMPL_LINK( ScTabOpDlg, GetFocusHdl, Control&, rCtrl, void )
{
    if( (&rCtrl == static_cast<Control*>(m_pEdFormulaRange)) || (&rCtrl == static_cast<Control*>(m_pRBFormulaRange)) )
        pEdActive = m_pEdFormulaRange;
    else if( (&rCtrl == static_cast<Control*>(m_pEdRowCell)) || (&rCtrl == static_cast<Control*>(m_pRBRowCell)) )
        pEdActive = m_pEdRowCell;
    else if( (&rCtrl == static_cast<Control*>(m_pEdColCell)) || (&rCtrl == static_cast<Control*>(m_pRBColCell)) )
        pEdActive = m_pEdColCell;
    else
        pEdActive = nullptr;

    if( pEdActive )
        pEdActive->SetSelection( Selection( 0, SELECTION_MAX ) );
}

IMPL_LINK_NOARG(ScTabOpDlg, LoseFocusHdl, Control&, void)
{
    bDlgLostFocus = !IsActive();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
