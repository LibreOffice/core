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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <tools/gen.hxx>
#include <sfx2/imgmgr.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/ruler.hxx>
#include <svl/zforlist.hxx>
#include <svl/stritem.hxx>

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

// nur fuers UpdateRange - Box in dem der gestackte Cursor sthet loeschen
#include "pam.hxx"

#include "swundo.hxx"
#include "ribbar.hrc"
#include "inputwin.hrc"

#include <IDocumentContentOperations.hxx>

SFX_IMPL_POS_CHILDWINDOW( SwInputChild, FN_EDIT_FORMULA, SFX_OBJECTBAR_OBJECT )

//==================================================================

SwInputWindow::SwInputWindow( Window* pParent, SfxBindings* pBind )
    : ToolBox(  pParent ,   SW_RES( RID_TBX_FORMULA )),
    aPos(       this,       SW_RES(ED_POS)),
    aEdit(      this, WB_3DLOOK|WB_TABSTOP|WB_BORDER|WB_NOHIDESELECTION),
    aPopMenu(   SW_RES(MN_CALC_POPUP)),
    pMgr(0),
    pWrtShell(0),
    pView(0),
    pBindings(pBind),
    aAktTableName(aEmptyStr)
{
    bFirst = bDoesUndo = TRUE;
    bActive = bIsTable = bDelSel = bResetUndo = bCallUndo = FALSE;

    FreeResource();

    SfxImageManager* pManager = SfxImageManager::GetImageManager( SW_MOD() );
    pManager->RegisterToolBox(this);

    pView = ::GetActiveView();
    pWrtShell = pView ? pView->GetWrtShellPtr() : 0;

    InsertWindow( ED_POS, &aPos, 0, 0);
    InsertSeparator ( 1 );
    InsertSeparator ();
    InsertWindow( ED_FORMULA, &aEdit);
    SetHelpId(ED_FORMULA, HID_EDIT_FORMULA);

    SetItemImage( FN_FORMULA_CALC,   pManager->GetImage(FN_FORMULA_CALC   ));
    SetItemImage( FN_FORMULA_CANCEL, pManager->GetImage(FN_FORMULA_CANCEL ));
    SetItemImage( FN_FORMULA_APPLY,  pManager->GetImage(FN_FORMULA_APPLY  ));

    SetItemBits( FN_FORMULA_CALC, GetItemBits( FN_FORMULA_CALC ) | TIB_DROPDOWNONLY );
    SetDropdownClickHdl( LINK( this, SwInputWindow, DropdownClickHdl ));

    Size    aSizeTbx = CalcWindowSizePixel();
    Size aSize = GetSizePixel();
    aSize.Height() = aSizeTbx.Height();
    SetSizePixel( aSize );
    Size    aPosSize = aPos.GetSizePixel();
    Size    aEditSize = aEdit.GetSizePixel();
    aPosSize.Height() = aEditSize.Height() = GetItemRect(FN_FORMULA_CALC).GetHeight() - 2;

    Point aPosPos = aPos.GetPosPixel();
    Point aEditPos= aEdit.GetPosPixel();
    aPosPos.Y() = aEditPos.Y() = GetItemRect( FN_FORMULA_CALC ).TopLeft().Y() + 1;
    aPos.SetPosSizePixel( aPosPos, aPosSize );
    aEdit.SetPosSizePixel( aEditPos, aEditSize );

    aPopMenu.SetSelectHdl(LINK( this, SwInputWindow, MenuHdl ));
}

//==================================================================

__EXPORT SwInputWindow::~SwInputWindow()
{
    SfxImageManager::GetImageManager( SW_MOD() )->ReleaseToolBox(this);

    //Lineale aufwecken
    if(pView)
    {
        pView->GetHLineal().SetActive( TRUE );
        pView->GetVLineal().SetActive( TRUE );
    }
    if ( pMgr )
        delete pMgr;
    if(pWrtShell)
        pWrtShell->EndSelTblCells();

    if( bResetUndo )
    {
        DelBoxCntnt();
        pWrtShell->DoUndo( bDoesUndo );
        if(bCallUndo)
            pWrtShell->Undo();
        SwEditShell::SetUndoActionCount( nActionCnt );
    }
}

//==================================================================

void SwInputWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( rDCEvt.GetType() == DATACHANGED_SETTINGS && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        //      update item images
        SwModule *pMod  = SW_MOD();
        SfxImageManager *pImgMgr = SfxImageManager::GetImageManager( pMod );
        SetItemImage( FN_FORMULA_CALC,   pImgMgr->GetImage(FN_FORMULA_CALC   ));
        SetItemImage( FN_FORMULA_CANCEL, pImgMgr->GetImage(FN_FORMULA_CANCEL ));
        SetItemImage( FN_FORMULA_APPLY,  pImgMgr->GetImage(FN_FORMULA_APPLY  ));
    }

    ToolBox::DataChanged( rDCEvt );
}

//==================================================================

void __EXPORT SwInputWindow::Resize()
{
    ToolBox::Resize();

    long    nWidth      = GetSizePixel().Width();
    long    nLeft       = aEdit.GetPosPixel().X();
    Size    aEditSize   = aEdit.GetSizePixel();

    aEditSize.Width() = Max( ((long)(nWidth - nLeft - 5)), (long)0 );
    aEdit.SetSizePixel( aEditSize );
    aEdit.Invalidate();
}

//==================================================================

void SwInputWindow::ShowWin()
{
    bIsTable = FALSE;
    //Lineale anhalten
    if(pView)
    {
        pView->GetHLineal().SetActive( FALSE );
        pView->GetVLineal().SetActive( FALSE );

        OSL_ENSURE(pWrtShell, "no WrtShell!");
        // Cursor in Tabelle
        bIsTable = pWrtShell->IsCrsrInTbl() ? TRUE : FALSE;

        if( bFirst )
            pWrtShell->SelTblCells( LINK( this, SwInputWindow,
                                                SelTblCellsNotify) );
        if( bIsTable )
        {
            const String& rPos = pWrtShell->GetBoxNms();
            USHORT nPos = 0;
            short nSrch = -1;
            while( (nPos = rPos.Search( ':',nPos + 1 ) ) != STRING_NOTFOUND )
                nSrch = (short) nPos;
            aPos.SetText( rPos.Copy( ++nSrch ) );
            aAktTableName = pWrtShell->GetTableFmt()->GetName();
        }
        else
            aPos.SetText(SW_RESSTR(STR_TBL_FORMULA));

        // Aktuelles Feld bearbeiten
        OSL_ENSURE(pMgr == 0, "FieldManager not deleted");
        pMgr = new SwFldMgr;

        // Formel soll immer mit einem "=" beginnen, hier
        // also setzen
        String sEdit( '=' );
        if( pMgr->GetCurFld() && TYP_FORMELFLD == pMgr->GetCurTypeId() )
        {
            sEdit += pMgr->GetCurFldPar2();
        }
        else if( bFirst )
        {
            if( bIsTable )
            {
                bResetUndo = TRUE;
                nActionCnt = SwEditShell::GetUndoActionCount();
                SwEditShell::SetUndoActionCount( nActionCnt + 1 );

                bDoesUndo = pWrtShell->DoesUndo();
                if( !bDoesUndo )
                    pWrtShell->DoUndo( TRUE );

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
                        bCallUndo = TRUE;
                }
                pWrtShell->DoUndo( FALSE );

                SfxItemSet aSet( pWrtShell->GetAttrPool(), RES_BOXATR_FORMULA, RES_BOXATR_FORMULA );
                if( pWrtShell->GetTblBoxFormulaAttrs( aSet ))
                    sEdit += ((SwTblBoxFormula&)aSet.Get( RES_BOXATR_FORMULA )).GetFormula();
            }
        }

        if( bFirst )
        {
            // WrtShell Flags richtig setzen
            pWrtShell->SttSelect();
            pWrtShell->EndSelect();
        }

        bFirst = FALSE;

        aEdit.SetModifyHdl( LINK( this, SwInputWindow, ModifyHdl ));

        aEdit.SetText( sEdit );
        aEdit.SetSelection( Selection( sEdit.Len(), sEdit.Len() ) );
        sOldFml = sEdit;

        aEdit.Invalidate();
        aEdit.Update();
        aEdit.GrabFocus();
        // UserInterface fuer die Eingabe abklemmen

        pView->GetEditWin().LockKeyInput(TRUE);
        pView->GetViewFrame()->GetDispatcher()->Lock(TRUE);
        pWrtShell->Push();
    }
    ToolBox::Show();
}
//==================================================================

IMPL_LINK( SwInputWindow, MenuHdl, Menu *, pMenu )
{
static const char * __READONLY_DATA aStrArr[] = {
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

    USHORT nId = pMenu->GetCurItemId();
    if ( nId <= MN_CALC_ROUND )
    {
        String aTmp( String::CreateFromAscii(aStrArr[nId - 1]) );
        aTmp += ' ';
        aEdit.ReplaceSelected( aTmp );
    }
    return 0;
}

IMPL_LINK( SwInputWindow, DropdownClickHdl, ToolBox*, EMPTYARG )
{
    USHORT nCurID = GetCurItemId();
    EndSelection(); // setzt CurItemId zurueck !
    switch ( nCurID )
    {
        case FN_FORMULA_CALC :
        {
            aPopMenu.Execute( this, GetItemRect( FN_FORMULA_CALC ), POPUPMENU_NOMOUSEUPCLOSE );
            break;
        default:
            break;
        }
    }

    return TRUE;
}

//==================================================================


void __EXPORT SwInputWindow::Click( )
{
    USHORT nCurID = GetCurItemId();
    EndSelection(); // setzt CurItemId zurueck !
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

//==================================================================

void  SwInputWindow::ApplyFormula()
{
    pView->GetViewFrame()->GetDispatcher()->Lock(FALSE);
    pView->GetEditWin().LockKeyInput(FALSE);
    if( bResetUndo )
    {
        DelBoxCntnt();
        pWrtShell->DoUndo( bDoesUndo );
        SwEditShell::SetUndoActionCount( nActionCnt );
        if( bCallUndo )
            pWrtShell->Undo();
        bResetUndo = FALSE;
    }
    pWrtShell->Pop( FALSE );

    // Formel soll immer mit einem "=" beginnen, hier
    // also wieder entfernen
    String sEdit( aEdit.GetText() );
    sEdit.EraseLeadingChars().EraseTrailingChars();
    if( sEdit.Len() && '=' == sEdit.GetChar( 0 ) )
        sEdit.Erase( 0, 1 );
    SfxStringItem aParam(FN_EDIT_FORMULA, sEdit);

    pWrtShell->EndSelTblCells();
    pView->GetEditWin().GrabFocus();
    const SfxPoolItem* aArgs[2];
    aArgs[0] = &aParam;
    aArgs[1] = 0;
    pView->GetViewFrame()->GetBindings().Execute( FN_EDIT_FORMULA, aArgs, 0, SFX_CALLMODE_ASYNCHRON );
}

//==================================================================

void  SwInputWindow::CancelFormula()
{
    if(pView)
    {
        pView->GetViewFrame()->GetDispatcher()->Lock( FALSE );
        pView->GetEditWin().LockKeyInput(FALSE);
        if( bResetUndo )
        {
            DelBoxCntnt();
            pWrtShell->DoUndo( bDoesUndo );
            SwEditShell::SetUndoActionCount( nActionCnt );
            if( bCallUndo )
                pWrtShell->Undo();
            bResetUndo = FALSE;
        }
        pWrtShell->Pop( FALSE );

        if( bDelSel )
            pWrtShell->EnterStdMode();

        pWrtShell->EndSelTblCells();

        pView->GetEditWin().GrabFocus();
    }
    pView->GetViewFrame()->GetDispatcher()->Execute( FN_EDIT_FORMULA, SFX_CALLMODE_ASYNCHRON);
}
//==================================================================

const xub_Unicode CH_LRE = 0x202a;
const xub_Unicode CH_PDF = 0x202c;

IMPL_LINK( SwInputWindow, SelTblCellsNotify, SwWrtShell *, pCaller )
{
    if(bIsTable)
    {
        SwFrmFmt* pTblFmt = pCaller->GetTableFmt();
        String sBoxNms( pCaller->GetBoxNms() );
        String sTblNm;
        if( pTblFmt && aAktTableName != pTblFmt->GetName() )
            sTblNm = pTblFmt->GetName();

        aEdit.UpdateRange( sBoxNms, sTblNm );

        String sNew;
        sNew += CH_LRE;
        sNew += aEdit.GetText();
        sNew += CH_PDF;

        if( sNew != sOldFml )
        {
            // Die WrtShell ist in der Tabellen Selektion
            // dann die Tabellen Selektion wieder aufheben, sonst steht der
            // Cursor "im Wald" und das LiveUpdate funktioniert nicht!
            pWrtShell->StartAllAction();

            SwPaM aPam( *pWrtShell->GetStkCrsr()->GetPoint() );
            aPam.Move( fnMoveBackward, fnGoSection );
            aPam.SetMark();
            aPam.Move( fnMoveForward, fnGoSection );

            IDocumentContentOperations* pIDCO = pWrtShell->getIDocumentContentOperations();
            pIDCO->DeleteRange( aPam );
            pIDCO->InsertString( aPam, sNew );
            pWrtShell->EndAllAction();
            sOldFml = sNew;
        }
    }
    else
        aEdit.GrabFocus();
    return 0;
}


void SwInputWindow::SetFormula( const String& rFormula, BOOL bDelFlag )
{
    String sEdit( '=' );
    if( rFormula.Len() )
    {
        if( '=' == rFormula.GetChar( 0 ) )
            sEdit = rFormula;
        else
            sEdit += rFormula;
    }
    aEdit.SetText( sEdit );
    aEdit.SetSelection( Selection( sEdit.Len(), sEdit.Len() ) );
    aEdit.Invalidate();
    bDelSel = bDelFlag;
}

IMPL_LINK( SwInputWindow, ModifyHdl, InputEdit*, EMPTYARG )
{
    if( bIsTable && bResetUndo )
    {
        pWrtShell->StartAllAction();
        DelBoxCntnt();
        String sNew;
        sNew += CH_LRE;
        sNew += aEdit.GetText();
        sNew += CH_PDF;
        pWrtShell->SwEditShell::Insert2( sNew );
        pWrtShell->EndAllAction();
        sOldFml = sNew;
    }
    return 0;
}


void SwInputWindow::DelBoxCntnt()
{
    if( bIsTable )
    {
        pWrtShell->StartAllAction();
        pWrtShell->ClearMark();
        pWrtShell->Pop( FALSE );
        pWrtShell->Push();
        pWrtShell->MoveSection( fnSectionCurr, fnSectionStart );
        pWrtShell->SetMark();
        pWrtShell->MoveSection( fnSectionCurr, fnSectionEnd );
        pWrtShell->SwEditShell::Delete();
        pWrtShell->EndAllAction();
    }
}

//==================================================================

void __EXPORT InputEdit::KeyInput(const KeyEvent& rEvent)
{
    const KeyCode aCode = rEvent.GetKeyCode();
    if(aCode == KEY_RETURN || aCode == KEY_F2 )
        ((SwInputWindow*)GetParent())->ApplyFormula();
    else if(aCode == KEY_ESCAPE )
        ((SwInputWindow*)GetParent())->CancelFormula();
    else
        Edit::KeyInput(rEvent);
}

//==================================================================

void __EXPORT InputEdit::UpdateRange(const String& rBoxes,
                                    const String& rName )
{
    if( !rBoxes.Len() )
    {
        GrabFocus();
        return;
    }
    const sal_Unicode   cOpen = '<', cClose = '>',
                cOpenBracket = '(';
    String aPrefix = rName;
    if(rName.Len())
        aPrefix += '.';
    String aBoxes = aPrefix;
    aBoxes += rBoxes;
    Selection aSelection(GetSelection());
    USHORT nSel = (USHORT) aSelection.Len();
    //OS: mit dem folgenden Ausdruck wird sichergestellt, dass im overwrite-Modus
    //die selektierte schliessende Klammer nicht geloescht wird
    if( nSel && ( nSel > 1 ||
        GetText().GetChar( (USHORT)aSelection.Min() ) != cClose ) )
        Cut();
    else
        aSelection.Max() = aSelection.Min();
    String aActText(GetText());
    const USHORT nLen = aActText.Len();
    if( !nLen )
    {
        String aStr(cOpen);
        aStr += aBoxes;
        aStr += cClose;
        SetText(aStr);
        USHORT nPos = aStr.Search( cClose );
        OSL_ENSURE(nPos < aStr.Len(), "delimiter not found");
        ++nPos;
        SetSelection( Selection( nPos, nPos ));
    }
    else
    {
        BOOL bFound = FALSE;
        sal_Unicode cCh;
        USHORT nPos, nEndPos = 0, nStartPos = (USHORT) aSelection.Min();
        if( nStartPos-- )
        {
            do {
                if( cOpen  == (cCh = aActText.GetChar( nStartPos ) ) ||
                    cOpenBracket == cCh )
                {
                    bFound = cCh == cOpen;
                    break;
                }
            } while( nStartPos-- > 0 );
        }
        if( bFound )
        {
            bFound = FALSE;
            nEndPos = nStartPos;
            while( nEndPos < nLen )
            {
                if( cClose == (cCh = aActText.GetChar( nEndPos )))
                {
                    bFound = TRUE;
                    break;
                }
                ++nEndPos;
            }
            // nur wenn akt. Pos im Breich oder direkt dahinter liegt
            if( bFound && !( nStartPos < (USHORT)aSelection.Max() &&
                             (USHORT)aSelection.Max() <= nEndPos + 1 ))
                bFound = FALSE;
        }
        if( bFound )
        {
            nPos = ++nStartPos + 1; // wir wollen dahinter
            aActText.Erase( nStartPos, nEndPos - nStartPos );
            aActText.Insert( aBoxes, nStartPos );
            nPos = nPos + aBoxes.Len();
        }
        else
        {
            String aTmp( (char)cOpen );
            aTmp += aBoxes;
            aTmp += (char)cClose;
            nPos = (USHORT)aSelection.Min();
            aActText.Insert( aTmp, nPos );
            nPos = nPos + aTmp.Len();
        }
        if( GetText() != aActText )
        {
            SetText( aActText );
            SetSelection( Selection( nPos, nPos ) );
        }
    }
    GrabFocus();

}
//==================================================================

SwInputChild::SwInputChild(Window* _pParent,
                                USHORT nId,
                                SfxBindings* pBindings,
                                SfxChildWinInfo* ) :
                                SfxChildWindow( _pParent, nId )
{
    pDispatch = pBindings->GetDispatcher();
    pWindow = new SwInputWindow( _pParent, pBindings );
    ((SwInputWindow*)pWindow)->ShowWin();
    eChildAlignment = SFX_ALIGN_LOWESTTOP;
}


__EXPORT SwInputChild::~SwInputChild()
{
    if(pDispatch)
        pDispatch->Lock(FALSE);
}


SfxChildWinInfo __EXPORT SwInputChild::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();     \
    return aInfo;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
