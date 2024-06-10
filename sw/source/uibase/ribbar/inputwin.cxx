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

#include <sal/config.h>
#include <sal/log.hxx>

#include <osl/diagnose.h>
#include <comphelper/string.hxx>
#include <o3tl/safeint.hxx>
#include <officecfg/Office/Common.hxx>
#include <tools/gen.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/ruler.hxx>
#include <svl/stritem.hxx>
#include <vcl/event.hxx>
#include <vcl/weldutils.hxx>

#include <swtypes.hxx>
#include <cmdid.h>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <inputwin.hxx>
#include <fldbas.hxx>
#include <fldmgr.hxx>
#include <frmfmt.hxx>
#include <cellatr.hxx>
#include <edtwin.hxx>
#include <helpids.h>
#include <strings.hrc>
#include <bitmaps.hlst>

// Only for the UpdateRange: Delete the box in which the stacked cursor is positioned.
#include <pam.hxx>

#include <swundo.hxx>

#include <IDocumentContentOperations.hxx>

constexpr ToolBoxItemId ED_POS(2);
constexpr ToolBoxItemId ED_FORMULA(3);
constexpr ToolBoxItemId FN_FORMULA_CALC(FN_FORMAT + 156);  /* select formula */
constexpr ToolBoxItemId FN_FORMULA_CANCEL(FN_FORMAT + 157);  /* don't apply formula */
constexpr ToolBoxItemId FN_FORMULA_APPLY(FN_FORMAT + 158); /* apply formula */

SFX_IMPL_POS_CHILDWINDOW_WITHID( SwInputChild, FN_EDIT_FORMULA, SFX_OBJECTBAR_OBJECT )

IMPL_LINK(PosEdit, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return ChildKeyInput(rKEvt);
}

SwInputWindow::SwInputWindow(vcl::Window* pParent, SfxDispatcher const * pDispatcher)
    : ToolBox(pParent, WB_3DLOOK|WB_BORDER)
    , mxPos(VclPtr<PosEdit>::Create(this))
    , mxEdit(VclPtr<InputEdit>::Create(this))
    , m_pWrtShell(nullptr)
    , m_pView(nullptr)
    , m_bDoesUndo(true)
    , m_bResetUndo(false)
    , m_bCallUndo(false)
{
    m_bFirst = true;
    m_bIsTable = m_bDelSel = false;

    InsertItem(FN_FORMULA_CALC, Image(StockImage::Yes, RID_BMP_FORMULA_CALC),
               SwResId(STR_FORMULA_CALC));
    InsertItem(FN_FORMULA_CANCEL, Image(StockImage::Yes, RID_BMP_FORMULA_CANCEL),
               SwResId(STR_FORMULA_CANCEL));
    InsertItem(FN_FORMULA_APPLY, Image(StockImage::Yes, RID_BMP_FORMULA_APPLY),
               SwResId(STR_FORMULA_APPLY));

    SetHelpId(FN_FORMULA_CALC, HID_TBX_FORMULA_CALC);
    SetHelpId(FN_FORMULA_CANCEL, HID_TBX_FORMULA_CANCEL);
    SetHelpId(FN_FORMULA_APPLY, HID_TBX_FORMULA_APPLY);

    SwView *pDispatcherView = dynamic_cast<SwView*>(pDispatcher ? pDispatcher->GetFrame()->GetViewShell() : nullptr);
    SwView* pActiveView = ::GetActiveView();
    if (pDispatcherView == pActiveView)
        m_pView = pActiveView;
    m_pWrtShell = m_pView ? m_pView->GetWrtShellPtr() : nullptr;

    InsertWindow(ED_POS, mxPos.get(), ToolBoxItemBits::NONE, 0);
    SetItemText(ED_POS, SwResId(STR_ACCESS_FORMULA_TYPE));
    mxPos->set_accessible_name(SwResId(STR_ACCESS_FORMULA_TYPE));
    SetAccessibleName(SwResId(STR_ACCESS_FORMULA_TOOLBAR));
    InsertSeparator ( 1 );
    InsertSeparator ();
    InsertWindow(ED_FORMULA, mxEdit.get());
    SetItemText(ED_FORMULA, SwResId(STR_ACCESS_FORMULA_TEXT));
    mxEdit->set_accessible_name(SwResId(STR_ACCESS_FORMULA_TEXT));
    SetHelpId(ED_FORMULA, HID_EDIT_FORMULA);

    SetItemBits( FN_FORMULA_CALC, GetItemBits( FN_FORMULA_CALC ) | ToolBoxItemBits::DROPDOWNONLY );
    SetDropdownClickHdl( LINK( this, SwInputWindow, DropdownClickHdl ));

    Size    aSizeTbx = CalcWindowSizePixel();
    Size    aEditSize = mxEdit->GetSizePixel();
    tools::Rectangle aItemRect( GetItemRect(FN_FORMULA_CALC) );
    tools::Long nMaxHeight = std::max(aEditSize.Height(), aItemRect.GetHeight());
    if( nMaxHeight+2 > aSizeTbx.Height() )
        aSizeTbx.setHeight( nMaxHeight+2 );
    Size aSize = GetSizePixel();
    aSize.setHeight( aSizeTbx.Height() );
    SetSizePixel( aSize );

    // align edit and item vcentered
    Size    aPosSize = mxPos->GetSizePixel();
    aPosSize.setHeight( nMaxHeight );
    aEditSize.setHeight( nMaxHeight );
    Point aPosPos  = mxPos->GetPosPixel();
    Point aEditPos = mxEdit->GetPosPixel();
    aPosPos.setY( (aSize.Height() - nMaxHeight)/2 + 1 );
    aEditPos.setY( (aSize.Height() - nMaxHeight)/2 + 1 );
    mxPos->SetPosSizePixel( aPosPos, aPosSize );
    mxEdit->SetPosSizePixel( aEditPos, aEditSize );
}

SwInputWindow::~SwInputWindow()
{
    disposeOnce();
}

void SwInputWindow::dispose()
{
    // wake rulers
    if(m_pView)
    {
        m_pView->GetHRuler().SetActive();
        m_pView->GetVRuler().SetActive();
    }
    m_pMgr.reset();
    if(m_pWrtShell)
        m_pWrtShell->EndSelTableCells();

    CleanupUglyHackWithUndo();

    mxPos.disposeAndClear();
    mxEdit.disposeAndClear();
    ToolBox::dispose();
}

void SwInputWindow::CleanupUglyHackWithUndo()
{
    if (!m_bResetUndo)
        return;

    if (m_pWrtShell)
    {
        DelBoxContent();
        m_pWrtShell->DoUndo(m_bDoesUndo);
        if (m_bCallUndo)
        {
            m_pWrtShell->Undo();
        }
    }
    m_bResetUndo = false; // #i117122# once is enough :)
}

void SwInputWindow::Resize()
{
    ToolBox::Resize();

    tools::Long    nWidth      = GetSizePixel().Width();
    tools::Long    nLeft       = mxEdit->GetPosPixel().X();
    Size    aEditSize   = mxEdit->GetSizePixel();

    aEditSize.setWidth( std::max( static_cast<tools::Long>(nWidth - nLeft - 5), tools::Long(0) ) );
    mxEdit->SetSizePixel( aEditSize );
}

void SwInputWindow::ShowWin()
{
    m_bIsTable = false;
    // stop rulers
    if (m_pView && m_pWrtShell)
    {
        m_pView->GetHRuler().SetActive( false );
        m_pView->GetVRuler().SetActive( false );

        // Cursor in table
        m_bIsTable = m_pWrtShell->IsCursorInTable();

        if( m_bFirst )
            m_pWrtShell->SelTableCells( LINK( this, SwInputWindow,
                                                SelTableCellsNotify) );
        if( m_bIsTable )
        {
            const OUString& rPos = m_pWrtShell->GetBoxNms();
            sal_Int32 nPos = 0;
            short nSrch = -1;
            while( (nPos = rPos.indexOf( ':',nPos + 1 ) ) != -1 )
                nSrch = static_cast<short>(nPos);
            mxPos->set_text( rPos.copy( ++nSrch ) );
            m_aCurrentTableName = m_pWrtShell->GetTableFormat()->GetName();
        }
        else
            mxPos->set_text(SwResId(STR_TBL_FORMULA));

        // Edit current field
        OSL_ENSURE(m_pMgr == nullptr, "FieldManager not deleted");
        m_pMgr.reset(new SwFieldMgr);

        // Form should always begin with "=" , so set here
        OUString sEdit('=');
        if( m_pMgr->GetCurField() && SwFieldTypesEnum::Formel == m_pMgr->GetCurTypeId() )
        {
            sEdit += m_pMgr->GetCurFieldPar2();
        }
        else if( m_bFirst && m_bIsTable )
        {
            m_bResetUndo = true;
            SAL_WARN_IF(
                officecfg::Office::Common::Undo::Steps::get() <= 0,
                "sw", "/org.openoffice.Office.Common/Undo/Steps <= 0");

            m_bDoesUndo = m_pWrtShell->DoesUndo();
            if( !m_bDoesUndo )
            {
                m_pWrtShell->DoUndo();
            }

            if( !m_pWrtShell->SwCursorShell::HasSelection() )
            {
                m_pWrtShell->MoveSection( GoCurrSection, fnSectionStart );
                m_pWrtShell->SetMark();
                m_pWrtShell->MoveSection( GoCurrSection, fnSectionEnd );
            }
            if( m_pWrtShell->SwCursorShell::HasSelection() )
            {
                m_pWrtShell->StartUndo( SwUndoId::DELETE );
                m_pWrtShell->Delete(false);
                if( SwUndoId::EMPTY != m_pWrtShell->EndUndo( SwUndoId::DELETE ))
                {
                    m_bCallUndo = true;
                }
            }
            m_pWrtShell->DoUndo(false);

            SfxItemSetFixed<RES_BOXATR_FORMULA, RES_BOXATR_FORMULA> aSet( m_pWrtShell->GetAttrPool() );
            if( m_pWrtShell->GetTableBoxFormulaAttrs( aSet ))
            {
                SwTableBoxFormula& rFormula
                    = const_cast<SwTableBoxFormula&>(aSet.Get(RES_BOXATR_FORMULA));
                // rFormula could be ANY of the table's formulas.
                // GetFormula returns the "current" formula - which is basically undefined,
                // so do something that encourages the current position's formula to become current.
                if (m_pWrtShell->GetCursor())
                {
                    const SwNode* pNd = m_pWrtShell->GetCursor()->GetPointNode().FindTableNode();
                    if (pNd)
                    {
                        const SwTable& rTable = static_cast<const SwTableNode*>(pNd)->GetTable();
                        // get cell's external formula (for UI) by waving the magic wand.
                        rFormula.PtrToBoxNm(&rTable);
                    }
                }

                sEdit += rFormula.GetFormula();
            }
        }

        if( m_bFirst )
        {
            // Set WrtShell flags correctly
            m_pWrtShell->SttSelect();
            m_pWrtShell->EndSelect();
        }

        m_bFirst = false;

        mxEdit->connect_changed( LINK( this, SwInputWindow, ModifyHdl ));

        mxEdit->set_text( sEdit );
        m_sOldFormula = sEdit;

        // For input cut the UserInterface

        m_pView->GetEditWin().LockKeyInput(true);
        m_pView->GetViewFrame().GetDispatcher()->Lock(true);
        m_pWrtShell->Push();
    }

    ToolBox::Show();

    // grab focus after ToolBox is shown so focus isn't potentially lost elsewhere
    if (m_pView)
    {
        int nPos = mxEdit->get_text().getLength();
        mxEdit->select_region(nPos, nPos);
        mxEdit->GrabFocus();
    }
}

void SwInputWindow::MenuHdl(std::u16string_view command)
{
    if (!command.empty())
        mxEdit->replace_selection(OUString::Concat(command) + " ");
}

IMPL_LINK_NOARG(SwInputWindow, DropdownClickHdl, ToolBox *, void)
{
    ToolBoxItemId nCurID = GetCurItemId();
    EndSelection(); // reset back CurItemId !
    if (nCurID == FN_FORMULA_CALC)
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(nullptr, u"modules/swriter/ui/inputwinmenu.ui"_ustr));
        std::unique_ptr<weld::Menu> xPopMenu(xBuilder->weld_menu(u"menu"_ustr));
        tools::Rectangle aRect(GetItemRect(FN_FORMULA_CALC));
        weld::Window* pParent = weld::GetPopupParent(*this, aRect);
        MenuHdl(xPopMenu->popup_at_rect(pParent, aRect));
    }
}

void SwInputWindow::Click( )
{
    ToolBoxItemId nCurID = GetCurItemId();
    EndSelection(); // reset back CurItemId !
    if ( nCurID == FN_FORMULA_CANCEL )
    {
        CancelFormula();
    }
    else if (nCurID == FN_FORMULA_APPLY)
    {
        ApplyFormula();
    }
}

void  SwInputWindow::ApplyFormula()
{
    // in case it was created while loading the document, the active view
    // wasn't initialised at that time, so ShowWin() didn't initialise anything
    // either - nothing to do
    if (!m_pView || !m_pWrtShell)
    {
        // presumably there must be an active view now since the event arrived
        if (SwView* pView = GetActiveView())
        {
            // this just makes the input window go away, so that the next time it works
            pView->GetViewFrame().GetDispatcher()->Execute(FN_EDIT_FORMULA, SfxCallMode::ASYNCHRON);
        }
        return;
    }

    m_pView->GetViewFrame().GetDispatcher()->Lock(false);
    m_pView->GetEditWin().LockKeyInput(false);
    CleanupUglyHackWithUndo();
    m_pWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);

    // Form should always begin with "=", so remove it here again
    OUString sEdit(comphelper::string::strip(mxEdit->get_text(), ' '));
    if( !sEdit.isEmpty() && '=' == sEdit[0] )
        sEdit = sEdit.copy( 1 );
    SfxStringItem aParam(FN_EDIT_FORMULA, sEdit);

    m_pWrtShell->EndSelTableCells();
    m_pView->GetEditWin().GrabFocus();
    const SfxPoolItem* aArgs[2];
    aArgs[0] = &aParam;
    aArgs[1] = nullptr;
    m_pView->GetViewFrame().GetBindings().Execute( FN_EDIT_FORMULA, aArgs, SfxCallMode::ASYNCHRON );
}

void  SwInputWindow::CancelFormula()
{
    // in case it was created while loading the document, the active view
    // wasn't initialised at that time, so ShowWin() didn't initialise anything
    // either - nothing to do
    if (!m_pView || !m_pWrtShell)
    {
        // presumably there must be an active view now since the event arrived
        if (SwView* pView = GetActiveView())
        {
            // this just makes the input window go away, so that the next time it works
            pView->GetViewFrame().GetDispatcher()->Execute(FN_EDIT_FORMULA, SfxCallMode::ASYNCHRON);
        }
        return;
    }

    m_pView->GetViewFrame().GetDispatcher()->Lock( false );
    m_pView->GetEditWin().LockKeyInput(false);
    CleanupUglyHackWithUndo();
    m_pWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);

    if( m_bDelSel )
        m_pWrtShell->EnterStdMode();

    m_pWrtShell->EndSelTableCells();

    m_pView->GetEditWin().GrabFocus();

    m_pView->GetViewFrame().GetDispatcher()->Execute( FN_EDIT_FORMULA, SfxCallMode::ASYNCHRON);
}

const sal_Unicode CH_LRE = 0x202a;
const sal_Unicode CH_PDF = 0x202c;

IMPL_LINK( SwInputWindow, SelTableCellsNotify, SwWrtShell&, rCaller, void )
{
    if(m_pWrtShell && m_bIsTable)
    {
        SwFrameFormat* pTableFormat = rCaller.GetTableFormat();
        OUString sBoxNms( rCaller.GetBoxNms() );
        OUString sTableNm;
        if( pTableFormat && m_aCurrentTableName != pTableFormat->GetName() )
            sTableNm = pTableFormat->GetName();

        mxEdit->UpdateRange( sBoxNms, sTableNm );

        OUString sNew = OUStringChar(CH_LRE) + mxEdit->get_text()
            + OUStringChar(CH_PDF);

        if( sNew != m_sOldFormula )
        {
            // The WrtShell is in the table selection,
            // then cancel the table selection otherwise, the cursor is
            // positioned "in the forest" and the live update does not work!
            m_pWrtShell->StartAllAction();

            SwPaM aPam( *m_pWrtShell->GetStackCursor()->GetPoint() );
            aPam.Move( fnMoveBackward, GoInSection );
            aPam.SetMark();
            aPam.Move( fnMoveForward, GoInSection );

            IDocumentContentOperations& rIDCO = m_pWrtShell->getIDocumentContentOperations();
            rIDCO.DeleteRange( aPam );
            rIDCO.InsertString( aPam, sNew );
            m_pWrtShell->EndAllAction();
            m_sOldFormula = sNew;
        }
    }
    else
        mxEdit->GrabFocus();
}

void SwInputWindow::SetFormula( const OUString& rFormula )
{
    OUString sEdit('=');
    if( !rFormula.isEmpty() )
    {
        if( '=' == rFormula[0] )
            sEdit = rFormula;
        else
            sEdit += rFormula;
    }
    mxEdit->set_text( sEdit );
    mxEdit->select_region(sEdit.getLength(), sEdit.getLength());
    m_bDelSel = true;
}

IMPL_LINK_NOARG(SwInputWindow, ModifyHdl, weld::Entry&, void)
{
    if (m_pWrtShell && m_bIsTable && m_bResetUndo)
    {
        m_pWrtShell->StartAllAction();
        DelBoxContent();
        OUString sNew = OUStringChar(CH_LRE) + mxEdit->get_text()
            + OUStringChar(CH_PDF);
        m_pWrtShell->SwEditShell::Insert2( sNew );
        m_pWrtShell->EndAllAction();
        m_sOldFormula = sNew;
    }
}

void SwInputWindow::DelBoxContent()
{
    if( m_pWrtShell && m_bIsTable )
    {
        m_pWrtShell->StartAllAction();
        m_pWrtShell->ClearMark();
        m_pWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
        m_pWrtShell->Push();
        m_pWrtShell->MoveSection( GoCurrSection, fnSectionStart );
        m_pWrtShell->SetMark();
        m_pWrtShell->MoveSection( GoCurrSection, fnSectionEnd );
        m_pWrtShell->SwEditShell::Delete(false);
        m_pWrtShell->EndAllAction();
    }
}

IMPL_LINK(InputEdit, KeyInputHdl, const KeyEvent&, rEvent, bool)
{
    bool bHandled = false;
    const vcl::KeyCode aCode = rEvent.GetKeyCode();
    if (aCode == KEY_RETURN || aCode == KEY_F2)
    {
        bHandled = ActivateHdl(*m_xWidget);
    }
    else if(aCode == KEY_ESCAPE )
    {
        static_cast<SwInputWindow*>(GetParent())->CancelFormula();
        bHandled = true;
    }
    return bHandled || ChildKeyInput(rEvent);
}

IMPL_LINK_NOARG(InputEdit, ActivateHdl, weld::Entry&, bool)
{
    static_cast<SwInputWindow*>(GetParent())->ApplyFormula();
    return true;
}

void InputEdit::UpdateRange(std::u16string_view rBoxes,
                                    const OUString& rName )
{
    if( rBoxes.empty() )
    {
        GrabFocus();
        return;
    }
    const sal_Unicode   cOpen = '<', cClose = '>',
                cOpenBracket = '(';
    OUString aPrefix = rName;
    if(!rName.isEmpty())
        aPrefix += ".";
    OUString aBoxes = aPrefix + rBoxes;

    int nSelStartPos, nSelEndPos;
    m_xWidget->get_selection_bounds(nSelStartPos, nSelEndPos);

    Selection aSelection(nSelStartPos, nSelEndPos);
    sal_uInt16 nSel = o3tl::narrowing<sal_uInt16>(aSelection.Len());
    // OS: The following expression ensures that in the overwrite mode,
    // the selected closing parenthesis will be not deleted.
    if( nSel && ( nSel > 1 ||
                  m_xWidget->get_text()[ o3tl::narrowing<sal_uInt16>(aSelection.Min()) ] != cClose ))
        m_xWidget->cut_clipboard();
    else
        aSelection.Max() = aSelection.Min();
    OUString aActText(m_xWidget->get_text());
    const sal_uInt16 nLen = aActText.getLength();
    if( !nLen )
    {
        OUString aStr = OUStringChar(cOpen) + aBoxes + OUStringChar(cClose);
        m_xWidget->set_text(aStr);
        sal_Int32 nPos = aStr.indexOf( cClose );
        OSL_ENSURE(nPos != -1, "delimiter not found");
        ++nPos;
        m_xWidget->select_region(nPos, nPos);
    }
    else
    {
        bool bFound = false;
        sal_Unicode cCh;
        sal_uInt16 nPos, nEndPos = 0, nStartPos = o3tl::narrowing<sal_uInt16>(aSelection.Min());
        if( nStartPos-- )
        {
            do {
                if( cOpen  == (cCh = aActText[ nStartPos ] ) ||
                    cOpenBracket == cCh )
                {
                    bFound = cCh == cOpen;
                    break;
                }
            } while( nStartPos-- > 0 );
        }
        if( bFound )
        {
            bFound = false;
            nEndPos = nStartPos;
            while( nEndPos < nLen )
            {
                if( cClose == aActText[ nEndPos ] )
                {
                    bFound = true;
                    break;
                }
                ++nEndPos;
            }
            // Only if the current position lies in the range or right behind.
            if( bFound && ( nStartPos >= o3tl::make_unsigned(aSelection.Max()) ||
                             o3tl::narrowing<sal_uInt16>(aSelection.Max()) > nEndPos + 1 ))
                bFound = false;
        }
        if( bFound )
        {
            nPos = ++nStartPos + 1; // We want behind
            aActText = aActText.replaceAt( nStartPos, nEndPos - nStartPos, aBoxes );
            nPos = nPos + aBoxes.getLength();
        }
        else
        {
            OUString aTmp = OUStringChar(cOpen) + aBoxes + OUStringChar(cClose);
            nPos = o3tl::narrowing<sal_uInt16>(aSelection.Min());
            aActText = aActText.replaceAt( nPos, 0, aTmp );
            nPos = nPos + aTmp.getLength();
        }
        if( m_xWidget->get_text() != aActText )
        {
            m_xWidget->set_text(aActText);
            m_xWidget->select_region(nPos, nPos);
        }
    }
    GrabFocus();

}

SwInputChild::SwInputChild(vcl::Window* _pParent,
                                sal_uInt16 nId,
                                SfxBindings const * pBindings,
                                SfxChildWinInfo* ) :
                                SfxChildWindow( _pParent, nId )
{
    m_pDispatch = pBindings->GetDispatcher();
    SetWindow(VclPtr<SwInputWindow>::Create(_pParent, m_pDispatch));
    static_cast<SwInputWindow*>(GetWindow())->ShowWin();
    SetAlignment(SfxChildAlignment::LOWESTTOP);
}

SwInputChild::~SwInputChild()
{
    if(m_pDispatch)
        m_pDispatch->Lock(false);
}

SfxChildWinInfo SwInputChild::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    return aInfo;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
