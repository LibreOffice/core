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

#include <comphelper/string.hxx>
#include <officecfg/Office/Common.hxx>
#include <tools/gen.hxx>
#include <sfx2/imgmgr.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/ruler.hxx>
#include <svl/zforlist.hxx>
#include <svl/stritem.hxx>
#include <vcl/settings.hxx>

#include "swtypes.hxx"
#include "cmdid.h"
#include "swmodule.hxx"
#include "wrtsh.hxx"
#include "view.hxx"
#include "calc.hxx"
#include "inputwin.hxx"
#include "fldbas.hxx"
#include "fldmgr.hxx"
#include "frmfmt.hxx"
#include "cellatr.hxx"
#include "edtwin.hxx"
#include "helpid.h"
#include "access.hrc"

// Only for the UpdateRange: Delete the box in which the stacked cursor is positioned.
#include "pam.hxx"

#include "swundo.hxx"
#include "ribbar.hrc"
#include "inputwin.hrc"

#include <IDocumentContentOperations.hxx>

SFX_IMPL_POS_CHILDWINDOW_WITHID( SwInputChild, FN_EDIT_FORMULA, SFX_OBJECTBAR_OBJECT )

SwInputWindow::SwInputWindow( vcl::Window* pParent )
    : ToolBox(  pParent ,   SW_RES( RID_TBX_FORMULA )),
    aPos(       VclPtr<Edit>::Create(this,       SW_RES(ED_POS))),
    aEdit(      VclPtr<InputEdit>::Create(this, WB_3DLOOK|WB_TABSTOP|WB_BORDER|WB_NOHIDESELECTION)),
    aPopMenu(   SW_RES(MN_CALC_POPUP)),
    pMgr(0),
    pWrtShell(0),
    pView(0),
    aAktTableName(aEmptyOUStr)
    , m_bDoesUndo(true)
    , m_bResetUndo(false)
    , m_bCallUndo(false)
{
    bFirst = true;
    bActive = bIsTable = bDelSel = false;

    FreeResource();

    aEdit->SetSizePixel( aEdit->CalcMinimumSize() );

    SfxImageManager* pManager = SfxImageManager::GetImageManager( *SW_MOD() );
    pManager->RegisterToolBox(this);

    pView = ::GetActiveView();
    pWrtShell = pView ? pView->GetWrtShellPtr() : 0;

    InsertWindow( ED_POS, aPos.get(), ToolBoxItemBits::NONE, 0);
    SetItemText(ED_POS, SW_RESSTR(STR_ACCESS_FORMULA_TYPE));
    aPos->SetAccessibleName(SW_RESSTR(STR_ACCESS_FORMULA_TYPE));
    SetAccessibleName(SW_RESSTR(STR_ACCESS_FORMULA_TOOLBAR));
    InsertSeparator ( 1 );
    InsertSeparator ();
    InsertWindow( ED_FORMULA, aEdit.get());
    SetItemText(ED_FORMULA, SW_RESSTR(STR_ACCESS_FORMULA_TEXT));
    aEdit->SetAccessibleName(SW_RESSTR(STR_ACCESS_FORMULA_TEXT));
    SetHelpId(ED_FORMULA, HID_EDIT_FORMULA);

    SetItemImage( FN_FORMULA_CALC,   pManager->GetImage(FN_FORMULA_CALC   ));
    SetItemImage( FN_FORMULA_CANCEL, pManager->GetImage(FN_FORMULA_CANCEL ));
    SetItemImage( FN_FORMULA_APPLY,  pManager->GetImage(FN_FORMULA_APPLY  ));

    SetItemBits( FN_FORMULA_CALC, GetItemBits( FN_FORMULA_CALC ) | ToolBoxItemBits::DROPDOWNONLY );
    SetDropdownClickHdl( LINK( this, SwInputWindow, DropdownClickHdl ));

    Size    aSizeTbx = CalcWindowSizePixel();
    Size    aEditSize = aEdit->GetSizePixel();
    Rectangle aItemRect( GetItemRect(FN_FORMULA_CALC) );
    long nMaxHeight = (aEditSize.Height() > aItemRect.GetHeight()) ? aEditSize.Height() : aItemRect.GetHeight();
    if( nMaxHeight+2 > aSizeTbx.Height() )
        aSizeTbx.Height() = nMaxHeight+2;
    Size aSize = GetSizePixel();
    aSize.Height() = aSizeTbx.Height();
    SetSizePixel( aSize );

    // align edit and item vcentered
    Size    aPosSize = aPos->GetSizePixel();
    aPosSize.Height()  = nMaxHeight;
    aEditSize.Height() = nMaxHeight;
    Point aPosPos  = aPos->GetPosPixel();
    Point aEditPos = aEdit->GetPosPixel();
    aPosPos.Y()    = (aSize.Height() - nMaxHeight)/2 + 1;
    aEditPos.Y()   = (aSize.Height() - nMaxHeight)/2 + 1;
    aPos->SetPosSizePixel( aPosPos, aPosSize );
    aEdit->SetPosSizePixel( aEditPos, aEditSize );

    aPopMenu.SetSelectHdl(LINK( this, SwInputWindow, MenuHdl ));
}

SwInputWindow::~SwInputWindow()
{
    disposeOnce();
}

void SwInputWindow::dispose()
{
    SfxImageManager::GetImageManager( *SW_MOD() )->ReleaseToolBox(this);

    // wake rulers
    if(pView)
    {
        pView->GetHRuler().SetActive();
        pView->GetVRuler().SetActive();
    }
    delete pMgr;
    if(pWrtShell)
        pWrtShell->EndSelTableCells();

    CleanupUglyHackWithUndo();

    aPos.disposeAndClear();
    aEdit.disposeAndClear();
    ToolBox::dispose();
}

void SwInputWindow::CleanupUglyHackWithUndo()
{
    if (m_bResetUndo)
    {
        if (pWrtShell)
        {
            DelBoxContent();
            pWrtShell->DoUndo(m_bDoesUndo);
            if (m_bCallUndo)
            {
                pWrtShell->Undo();
            }
        }
        m_bResetUndo = false; // #i117122# once is enough :)
    }
}

void SwInputWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( rDCEvt.GetType() == DataChangedEventType::SETTINGS && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        // update item images
        SwModule *pMod  = SW_MOD();
        SfxImageManager *pImgMgr = SfxImageManager::GetImageManager(*pMod);
        SetItemImage( FN_FORMULA_CALC,   pImgMgr->GetImage(FN_FORMULA_CALC   ));
        SetItemImage( FN_FORMULA_CANCEL, pImgMgr->GetImage(FN_FORMULA_CANCEL ));
        SetItemImage( FN_FORMULA_APPLY,  pImgMgr->GetImage(FN_FORMULA_APPLY  ));
    }

    ToolBox::DataChanged( rDCEvt );
}

void SwInputWindow::Resize()
{
    ToolBox::Resize();

    long    nWidth      = GetSizePixel().Width();
    long    nLeft       = aEdit->GetPosPixel().X();
    Size    aEditSize   = aEdit->GetSizePixel();

    aEditSize.Width() = std::max( ((long)(nWidth - nLeft - 5)), (long)0 );
    aEdit->SetSizePixel( aEditSize );
    aEdit->Invalidate();
}

void SwInputWindow::ShowWin()
{
    bIsTable = false;
    // stop rulers
    if(pView)
    {
        pView->GetHRuler().SetActive( false );
        pView->GetVRuler().SetActive( false );

        OSL_ENSURE(pWrtShell, "no WrtShell!");
        // Cursor in table
        bIsTable = pWrtShell->IsCrsrInTable();

        if( bFirst )
            pWrtShell->SelTableCells( LINK( this, SwInputWindow,
                                                SelTableCellsNotify) );
        if( bIsTable )
        {
            const OUString& rPos = pWrtShell->GetBoxNms();
            sal_Int32 nPos = 0;
            short nSrch = -1;
            while( (nPos = rPos.indexOf( ':',nPos + 1 ) ) != -1 )
                nSrch = (short) nPos;
            aPos->SetText( rPos.copy( ++nSrch ) );
            aAktTableName = pWrtShell->GetTableFormat()->GetName();
        }
        else
            aPos->SetText(SW_RESSTR(STR_TBL_FORMULA));

        // Edit current field
        OSL_ENSURE(pMgr == 0, "FieldManager not deleted");
        pMgr = new SwFieldMgr;

        // Formular should always begin with "=" , so set here
        OUString sEdit('=');
        if( pMgr->GetCurField() && TYP_FORMELFLD == pMgr->GetCurTypeId() )
        {
            sEdit += pMgr->GetCurFieldPar2();
        }
        else if( bFirst )
        {
            if( bIsTable )
            {
                m_bResetUndo = true;
                SAL_WARN_IF(
                    officecfg::Office::Common::Undo::Steps::get() <= 0,
                    "sw", "/org.openoffice.Office.Common/Undo/Steps <= 0");

                m_bDoesUndo = pWrtShell->DoesUndo();
                if( !m_bDoesUndo )
                {
                    pWrtShell->DoUndo();
                }

                if( !pWrtShell->SwCrsrShell::HasSelection() )
                {
                    pWrtShell->MoveSection( fnSectionCurr, fnSectionStart );
                    pWrtShell->SetMark();
                    pWrtShell->MoveSection( fnSectionCurr, fnSectionEnd );
                }
                if( pWrtShell->SwCrsrShell::HasSelection() )
                {
                    pWrtShell->StartUndo( UNDO_DELETE );
                    pWrtShell->Delete();
                    if( 0 != pWrtShell->EndUndo( UNDO_DELETE ))
                    {
                        m_bCallUndo = true;
                    }
                }
                pWrtShell->DoUndo(false);

                SfxItemSet aSet( pWrtShell->GetAttrPool(), RES_BOXATR_FORMULA, RES_BOXATR_FORMULA );
                if( pWrtShell->GetTableBoxFormulaAttrs( aSet ))
                    sEdit += static_cast<const SwTableBoxFormula&>(aSet.Get( RES_BOXATR_FORMULA )).GetFormula();
            }
        }

        if( bFirst )
        {
            // Set WrtShell flags correctly
            pWrtShell->SttSelect();
            pWrtShell->EndSelect();
        }

        bFirst = false;

        aEdit->SetModifyHdl( LINK( this, SwInputWindow, ModifyHdl ));

        aEdit->SetText( sEdit );
        aEdit->SetSelection( Selection( sEdit.getLength(), sEdit.getLength() ) );
        sOldFormula = sEdit;

        aEdit->Invalidate();
        aEdit->Update();
        aEdit->GrabFocus();
        // For input cut the UserInterface

        pView->GetEditWin().LockKeyInput(true);
        pView->GetViewFrame()->GetDispatcher()->Lock(true);
        pWrtShell->Push();
    }
    ToolBox::Show();
}

IMPL_LINK_TYPED( SwInputWindow, MenuHdl, Menu *, pMenu, bool )
{
static const char * const aStrArr[] = {
    sCalc_Phd,
    sCalc_Sqrt,
    sCalc_Or,
    sCalc_Xor,
    sCalc_And,
    sCalc_Not,
    sCalc_Eq,
    sCalc_Neq,
    sCalc_Leq,
    sCalc_Geq,
    sCalc_L,
    sCalc_G,
    sCalc_Sum,
    sCalc_Mean,
    sCalc_Min,
    sCalc_Max,
    sCalc_Sin,
    sCalc_Cos,
    sCalc_Tan,
    sCalc_Asin,
    sCalc_Acos,
    sCalc_Atan,
    sCalc_Pow,
    "|",
    sCalc_Round
};

    sal_uInt16 nId = pMenu->GetCurItemId();
    if ( nId <= MN_CALC_ROUND )
    {
        OUString aTmp( OUString::createFromAscii(aStrArr[nId - 1]) );
        aTmp += " ";
        aEdit->ReplaceSelected( aTmp );
    }
    return false;
}

IMPL_LINK_NOARG_TYPED(SwInputWindow, DropdownClickHdl, ToolBox *, void)
{
    sal_uInt16 nCurID = GetCurItemId();
    EndSelection(); // reset back CurItemId !
    switch ( nCurID )
    {
        case FN_FORMULA_CALC :
        {
            aPopMenu.Execute( this, GetItemRect( FN_FORMULA_CALC ), PopupMenuFlags::NoMouseUpClose );
            break;
        default:
            break;
        }
    }
}

void SwInputWindow::Click( )
{
    sal_uInt16 nCurID = GetCurItemId();
    EndSelection(); // reset back CurItemId !
    switch ( nCurID )
    {
        case FN_FORMULA_CANCEL:
        {
            CancelFormula();
        }
        break;
        case FN_FORMULA_APPLY:
        {
            ApplyFormula();
        }
        break;
   }
}

void  SwInputWindow::ApplyFormula()
{
    pView->GetViewFrame()->GetDispatcher()->Lock(false);
    pView->GetEditWin().LockKeyInput(false);
    CleanupUglyHackWithUndo();
    pWrtShell->Pop( false );

    // Formular should always begin with "=", so remove it here again
    OUString sEdit(comphelper::string::strip(aEdit->GetText(), ' '));
    if( !sEdit.isEmpty() && '=' == sEdit[0] )
        sEdit = sEdit.copy( 1 );
    SfxStringItem aParam(FN_EDIT_FORMULA, sEdit);

    pWrtShell->EndSelTableCells();
    pView->GetEditWin().GrabFocus();
    const SfxPoolItem* aArgs[2];
    aArgs[0] = &aParam;
    aArgs[1] = 0;
    pView->GetViewFrame()->GetBindings().Execute( FN_EDIT_FORMULA, aArgs, 0, SfxCallMode::ASYNCHRON );
}

void  SwInputWindow::CancelFormula()
{
    if(pView)
    {
        pView->GetViewFrame()->GetDispatcher()->Lock( false );
        pView->GetEditWin().LockKeyInput(false);
        CleanupUglyHackWithUndo();
        pWrtShell->Pop( false );

        if( bDelSel )
            pWrtShell->EnterStdMode();

        pWrtShell->EndSelTableCells();

        pView->GetEditWin().GrabFocus();

        pView->GetViewFrame()->GetDispatcher()->Execute( FN_EDIT_FORMULA, SfxCallMode::ASYNCHRON);
    }
}

const sal_Unicode CH_LRE = 0x202a;
const sal_Unicode CH_PDF = 0x202c;

IMPL_LINK_TYPED( SwInputWindow, SelTableCellsNotify, SwWrtShell&, rCaller, void )
{
    if(bIsTable)
    {
        SwFrameFormat* pTableFormat = rCaller.GetTableFormat();
        OUString sBoxNms( rCaller.GetBoxNms() );
        OUString sTableNm;
        if( pTableFormat && aAktTableName != pTableFormat->GetName() )
            sTableNm = pTableFormat->GetName();

        aEdit->UpdateRange( sBoxNms, sTableNm );

        OUString sNew;
        sNew += OUString(CH_LRE);
        sNew += aEdit->GetText();
        sNew += OUString(CH_PDF);

        if( sNew != sOldFormula )
        {
            // The WrtShell is in the table selection,
            // then cancel the table selection otherwise, the cursor is
            // positioned "in the forest" and the live update does not work!
            pWrtShell->StartAllAction();

            SwPaM aPam( *pWrtShell->GetStkCrsr()->GetPoint() );
            aPam.Move( fnMoveBackward, fnGoSection );
            aPam.SetMark();
            aPam.Move( fnMoveForward, fnGoSection );

            IDocumentContentOperations& rIDCO = pWrtShell->getIDocumentContentOperations();
            rIDCO.DeleteRange( aPam );
            rIDCO.InsertString( aPam, sNew );
            pWrtShell->EndAllAction();
            sOldFormula = sNew;
        }
    }
    else
        aEdit->GrabFocus();
}

void SwInputWindow::SetFormula( const OUString& rFormula, bool bDelFlag )
{
    OUString sEdit('=');
    if( !rFormula.isEmpty() )
    {
        if( '=' == rFormula[0] )
            sEdit = rFormula;
        else
            sEdit += rFormula;
    }
    aEdit->SetText( sEdit );
    aEdit->SetSelection( Selection( sEdit.getLength(), sEdit.getLength() ) );
    aEdit->Invalidate();
    bDelSel = bDelFlag;
}

IMPL_LINK_NOARG(SwInputWindow, ModifyHdl)
{
    if (bIsTable && m_bResetUndo)
    {
        pWrtShell->StartAllAction();
        DelBoxContent();
        OUString sNew;
        sNew += OUString(CH_LRE);
        sNew += aEdit->GetText();
        sNew += OUString(CH_PDF);
        pWrtShell->SwEditShell::Insert2( sNew );
        pWrtShell->EndAllAction();
        sOldFormula = sNew;
    }
    return 0;
}

void SwInputWindow::DelBoxContent()
{
    if( bIsTable )
    {
        pWrtShell->StartAllAction();
        pWrtShell->ClearMark();
        pWrtShell->Pop( false );
        pWrtShell->Push();
        pWrtShell->MoveSection( fnSectionCurr, fnSectionStart );
        pWrtShell->SetMark();
        pWrtShell->MoveSection( fnSectionCurr, fnSectionEnd );
        pWrtShell->SwEditShell::Delete();
        pWrtShell->EndAllAction();
    }
}

void InputEdit::KeyInput(const KeyEvent& rEvent)
{
    const vcl::KeyCode aCode = rEvent.GetKeyCode();
    if(aCode == KEY_RETURN || aCode == KEY_F2 )
        static_cast<SwInputWindow*>(GetParent())->ApplyFormula();
    else if(aCode == KEY_ESCAPE )
        static_cast<SwInputWindow*>(GetParent())->CancelFormula();
    else
        Edit::KeyInput(rEvent);
}

void InputEdit::UpdateRange(const OUString& rBoxes,
                                    const OUString& rName )
{
    if( rBoxes.isEmpty() )
    {
        GrabFocus();
        return;
    }
    const sal_Unicode   cOpen = '<', cClose = '>',
                cOpenBracket = '(';
    OUString aPrefix = rName;
    if(!rName.isEmpty())
        aPrefix += ".";
    OUString aBoxes = aPrefix;
    aBoxes += rBoxes;
    Selection aSelection(GetSelection());
    sal_uInt16 nSel = (sal_uInt16) aSelection.Len();
    // OS: The following expression ensures that in the overwrite mode,
    // the selected closing parenthesis will be not deleted.
    if( nSel && ( nSel > 1 ||
                  GetText()[ (sal_uInt16)aSelection.Min() ] != cClose ))
        Cut();
    else
        aSelection.Max() = aSelection.Min();
    OUString aActText(GetText());
    const sal_uInt16 nLen = aActText.getLength();
    if( !nLen )
    {
        OUString aStr = OUStringBuffer().
            append(cOpen).append(aBoxes).append(cClose).
            makeStringAndClear();
        SetText(aStr);
        sal_Int32 nPos = aStr.indexOf( cClose );
        OSL_ENSURE(nPos != -1, "delimiter not found");
        ++nPos;
        SetSelection( Selection( nPos, nPos ));
    }
    else
    {
        bool bFound = false;
        sal_Unicode cCh;
        sal_uInt16 nPos, nEndPos = 0, nStartPos = (sal_uInt16) aSelection.Min();
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
                if( cClose == (cCh = aActText[ nEndPos ]))
                {
                    bFound = true;
                    break;
                }
                ++nEndPos;
            }
            // Only if the current position lies in the range or right behind.
            if( bFound && !( nStartPos < (sal_uInt16)aSelection.Max() &&
                             (sal_uInt16)aSelection.Max() <= nEndPos + 1 ))
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
            OUString aTmp = OUStringBuffer().
                append(cOpen).append(aBoxes).append(cClose).
                makeStringAndClear();
            nPos = (sal_uInt16)aSelection.Min();
            aActText = aActText.replaceAt( nPos, 0, aTmp );
            nPos = nPos + aTmp.getLength();
        }
        if( GetText() != OUString(aActText) )
        {
            SetText( aActText );
            SetSelection( Selection( nPos, nPos ) );
        }
    }
    GrabFocus();

}

SwInputChild::SwInputChild(vcl::Window* _pParent,
                                sal_uInt16 nId,
                                SfxBindings* pBindings,
                                SfxChildWinInfo* ) :
                                SfxChildWindow( _pParent, nId )
{
    pDispatch = pBindings->GetDispatcher();
    SetWindow( VclPtr<SwInputWindow>::Create( _pParent ) );
    static_cast<SwInputWindow*>(GetWindow())->ShowWin();
    SetAlignment(SfxChildAlignment::LOWESTTOP);
}

SwInputChild::~SwInputChild()
{
    if(pDispatch)
        pDispatch->Lock(false);
}

SfxChildWinInfo SwInputChild::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();     \
    return aInfo;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
