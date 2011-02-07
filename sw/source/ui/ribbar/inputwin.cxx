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
#include <unotools/undoopt.hxx>

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
    , m_nActionCount(0)
    , m_bDoesUndo(true)
    , m_bResetUndo(false)
    , m_bCallUndo(false)
{
    bFirst = sal_True;
    bActive = bIsTable = bDelSel = sal_False;

    FreeResource();

    aEdit.SetSizePixel( aEdit.CalcMinimumSize() );

    SfxImageManager* pManager = SfxImageManager::GetImageManager( SW_MOD() );
    pManager->RegisterToolBox(this);

    pView = ::GetActiveView();
    pWrtShell = pView ? pView->GetWrtShellPtr() : 0;

    InsertWindow( ED_POS, &aPos, 0, 0);
    InsertSeparator ( 1 );
    InsertSeparator ();
    InsertWindow( ED_FORMULA, &aEdit);
    SetHelpId(ED_FORMULA, HID_EDIT_FORMULA);

    sal_Bool bHC = GetSettings().GetStyleSettings().GetHighContrastMode();
    SetItemImage( FN_FORMULA_CALC, pManager->GetImage(FN_FORMULA_CALC, bHC ));
    SetItemImage( FN_FORMULA_CANCEL, pManager->GetImage(FN_FORMULA_CANCEL, bHC  ));
    SetItemImage( FN_FORMULA_APPLY, pManager->GetImage(FN_FORMULA_APPLY, bHC  ));

    SetItemBits( FN_FORMULA_CALC, GetItemBits( FN_FORMULA_CALC ) | TIB_DROPDOWNONLY );
    SetDropdownClickHdl( LINK( this, SwInputWindow, DropdownClickHdl ));

    Size    aSizeTbx = CalcWindowSizePixel();
    Size    aEditSize = aEdit.GetSizePixel();
    Rectangle aItemRect( GetItemRect(FN_FORMULA_CALC) );
    long nMaxHeight = (aEditSize.Height() > aItemRect.GetHeight()) ? aEditSize.Height() : aItemRect.GetHeight();
    if( nMaxHeight+2 > aSizeTbx.Height() )
        aSizeTbx.Height() = nMaxHeight+2;
    Size aSize = GetSizePixel();
    aSize.Height() = aSizeTbx.Height();
    SetSizePixel( aSize );

    // align edit and item vcentered
    Size    aPosSize = aPos.GetSizePixel();
    aPosSize.Height()  = nMaxHeight;
    aEditSize.Height() = nMaxHeight;
    Point aPosPos  = aPos.GetPosPixel();
    Point aEditPos = aEdit.GetPosPixel();
    aPosPos.Y()    = (aSize.Height() - nMaxHeight)/2 + 1;
    aEditPos.Y()   = (aSize.Height() - nMaxHeight)/2 + 1;
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
        pView->GetHLineal().SetActive( sal_True );
        pView->GetVLineal().SetActive( sal_True );
    }
    if ( pMgr )
        delete pMgr;
    if(pWrtShell)
        pWrtShell->EndSelTblCells();

    CleanupUglyHackWithUndo();
}

void SwInputWindow::CleanupUglyHackWithUndo()
{
    if (m_bResetUndo)
    {
        DelBoxCntnt();
        pWrtShell->DoUndo(m_bDoesUndo);
        if (m_bCallUndo)
        {
            pWrtShell->Undo();
        }
        if (0 == m_nActionCount)
        {
            SW_MOD()->GetUndoOptions().SetUndoCount(0);
        }
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
        sal_Bool bHC = GetSettings().GetStyleSettings().GetHighContrastMode();
        //
        SetItemImage( FN_FORMULA_CALC,   pImgMgr->GetImage(FN_FORMULA_CALC,   bHC ));
        SetItemImage( FN_FORMULA_CANCEL, pImgMgr->GetImage(FN_FORMULA_CANCEL, bHC ));
        SetItemImage( FN_FORMULA_APPLY,  pImgMgr->GetImage(FN_FORMULA_APPLY,  bHC ));
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
    bIsTable = sal_False;
    //Lineale anhalten
    if(pView)
    {
        pView->GetHLineal().SetActive( sal_False );
        pView->GetVLineal().SetActive( sal_False );

        DBG_ASSERT(pWrtShell, "Keine WrtShell!");
        // Cursor in Tabelle
        bIsTable = pWrtShell->IsCrsrInTbl() ? sal_True : sal_False;

        if( bFirst )
            pWrtShell->SelTblCells( LINK( this, SwInputWindow,
                                                SelTblCellsNotify) );
        if( bIsTable )
        {
            const String& rPos = pWrtShell->GetBoxNms();
            sal_uInt16 nPos = 0;
            short nSrch = -1;
            while( (nPos = rPos.Search( ':',nPos + 1 ) ) != STRING_NOTFOUND )
                nSrch = (short) nPos;
            aPos.SetText( rPos.Copy( ++nSrch ) );
            aAktTableName = pWrtShell->GetTableFmt()->GetName();
        }
        else
            aPos.SetText(SW_RESSTR(STR_TBL_FORMULA));

        // Aktuelles Feld bearbeiten
        ASSERT(pMgr == 0, FieldManager nicht geloescht.);
        pMgr = new SwFldMgr;

        // JP 13.01.97: Formel soll immer mit einem "=" beginnen, hier
        //              also setzen
        String sEdit( '=' );
        if( pMgr->GetCurFld() && TYP_FORMELFLD == pMgr->GetCurTypeId() )
        {
            sEdit += pMgr->GetCurFldPar2();
        }
        else if( bFirst )
        {
            if( bIsTable )
            {
                m_bResetUndo = true;
                m_nActionCount = SW_MOD()->GetUndoOptions().GetUndoCount();
                if (0 == m_nActionCount) { // deactivated? turn it on...
                    SW_MOD()->GetUndoOptions().SetUndoCount(1);
                }

                bDoesUndo = pWrtShell->DoesUndo();
                if( !bDoesUndo )
                    pWrtShell->DoUndo( sal_True );

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

        bFirst = sal_False;

        aEdit.SetModifyHdl( LINK( this, SwInputWindow, ModifyHdl ));

        aEdit.SetText( sEdit );
        aEdit.SetSelection( Selection( sEdit.Len(), sEdit.Len() ) );
        sOldFml = sEdit;

        aEdit.Invalidate();
        aEdit.Update();
        aEdit.GrabFocus();
        // UserInterface fuer die Eingabe abklemmen

        pView->GetEditWin().LockKeyInput(sal_True);
        pView->GetViewFrame()->GetDispatcher()->Lock(sal_True);
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

    sal_uInt16 nId = pMenu->GetCurItemId();
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
    sal_uInt16 nCurID = GetCurItemId();
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

    return sal_True;
}

//==================================================================


void __EXPORT SwInputWindow::Click( )
{
    sal_uInt16 nCurID = GetCurItemId();
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
    pView->GetViewFrame()->GetDispatcher()->Lock(sal_False);
    pView->GetEditWin().LockKeyInput(sal_False);
    CleanupUglyHackWithUndo();
    pWrtShell->Pop( sal_False );

    // JP 13.01.97: Formel soll immer mit einem "=" beginnen, hier
    //              also wieder entfernen
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
        pView->GetViewFrame()->GetDispatcher()->Lock( sal_False );
        pView->GetEditWin().LockKeyInput(sal_False);
        CleanupUglyHackWithUndo();
        pWrtShell->Pop( sal_False );

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


void SwInputWindow::SetFormula( const String& rFormula, sal_Bool bDelFlag )
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
    if (bIsTable && m_bResetUndo)
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
        pWrtShell->Pop( sal_False );
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
    sal_uInt16 nSel = (sal_uInt16) aSelection.Len();
    //OS: mit dem folgenden Ausdruck wird sichergestellt, dass im overwrite-Modus
    //die selektierte schliessende Klammer nicht geloescht wird
    if( nSel && ( nSel > 1 ||
        GetText().GetChar( (sal_uInt16)aSelection.Min() ) != cClose ) )
        Cut();
    else
        aSelection.Max() = aSelection.Min();
    String aActText(GetText());
    const sal_uInt16 nLen = aActText.Len();
    if( !nLen )
    {
        String aStr(cOpen);
        aStr += aBoxes;
        aStr += cClose;
        SetText(aStr);
        sal_uInt16 nPos = aStr.Search( cClose );
        ASSERT(nPos < aStr.Len(), Delimiter nicht gefunden.);
        ++nPos;
        SetSelection( Selection( nPos, nPos ));
    }
    else
    {
        sal_Bool bFound = sal_False;
        sal_Unicode cCh;
        sal_uInt16 nPos, nEndPos = 0, nStartPos = (sal_uInt16) aSelection.Min();
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
            bFound = sal_False;
            nEndPos = nStartPos;
            while( nEndPos < nLen )
            {
                if( cClose == (cCh = aActText.GetChar( nEndPos )) /*||
                    cCh == cCloseBracket*/ )
                {
                    bFound = sal_True;
                    break;
                }
                ++nEndPos;
            }
            // nur wenn akt. Pos im Breich oder direkt dahinter liegt
            if( bFound && !( nStartPos < (sal_uInt16)aSelection.Max() &&
                             (sal_uInt16)aSelection.Max() <= nEndPos + 1 ))
                bFound = sal_False;
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
            nPos = (sal_uInt16)aSelection.Min();
            aActText.Insert( aTmp, nPos );
            nPos = nPos + aTmp.Len();
        }
        if( GetText() != aActText )
        {
            SetText( aActText );
            SetSelection( Selection( nPos, nPos ) );
//          GetModifyHdl().Call( this );
        }
    }
    GrabFocus();

}
//==================================================================


SwInputChild::SwInputChild(Window* _pParent,
                                sal_uInt16 nId,
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
        pDispatch->Lock(sal_False);
}


SfxChildWinInfo __EXPORT SwInputChild::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();     \
    return aInfo;
}

