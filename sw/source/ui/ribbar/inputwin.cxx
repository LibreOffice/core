/*************************************************************************
 *
 *  $RCSfile: inputwin.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _SFXIMGMGR_HXX //autogen
#include <sfx2/imgmgr.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVX_RULER_HXX //autogen
#include <svx/ruler.hxx>
#endif
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif


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
#include "doc.hxx"

#include "swundo.hxx"
#include "ribbar.hrc"
#include "inputwin.hrc"


SFX_IMPL_POS_CHILDWINDOW( SwInputChild, FN_EDIT_FORMULA, SFX_OBJECTBAR_OBJECT )

//==================================================================

SwInputWindow::SwInputWindow( Window* pParent )
    : ToolBox(  pParent ,   SW_RES( RID_TBX_FORMULA )),
    aEdit(      this, WB_3DLOOK|WB_TABSTOP|WB_BORDER|WB_NOHIDESELECTION),
    aPos(       this,       SW_RES(ED_POS)),
    aPopMenu(   SW_RES(MN_CALC_POPUP)),
    pMgr(0),
    pView(0),
    pWrtShell(0),
    aAktTableName(aEmptyStr)
{
    bFirst = bDoesUndo = TRUE;
    bActive = bIsTable = bDelSel = bResetUndo = bCallUndo = FALSE;

    FreeResource();
    SFX_APP()->GetImageManager()->RegisterToolBox(this);

    pView = ::GetActiveView();
    pWrtShell = pView ? pView->GetWrtShellPtr() : 0;

    InsertWindow( ED_POS, &aPos, 0, 0);
    InsertSeparator ( 1 );
    InsertSeparator ();
    InsertWindow( ED_FORMULA, &aEdit);
    SetHelpId(ED_FORMULA, HID_EDIT_FORMULA);

    SfxImageManager* pManager = SFX_IMAGEMANAGER();
    SetItemImage( FN_FORMULA_CALC, pManager->GetImage(FN_FORMULA_CALC ));
    SetItemImage( FN_FORMULA_CANCEL, pManager->GetImage(FN_FORMULA_CANCEL ));
    SetItemImage( FN_FORMULA_APPLY, pManager->GetImage(FN_FORMULA_APPLY ));

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
    SFX_APP()->GetImageManager()->ReleaseToolBox(this);
    //Lineale aufwecken
    if(pView)
    {
        pView->GetHLineal()->SetActive( TRUE );
        if( pView->GetVLineal())
            pView->GetVLineal()->SetActive( TRUE );
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

void SwInputWindow::Show()
{
    bIsTable = FALSE;
    //Lineale anhalten
    if(pView)
    {
        pView->GetHLineal()->SetActive( FALSE );
        if( pView->GetVLineal())
            pView->GetVLineal()->SetActive( FALSE );

        DBG_ASSERT(pWrtShell, "Keine WrtShell!")
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
            aPos.SetText(SW_RESSTR(STR_FORMULA));

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

//==================================================================


void __EXPORT SwInputWindow::Click( )
{
    USHORT nCurID = GetCurItemId();
    EndSelection(); // setzt CurItemId zurueck !
    switch ( nCurID )
    {
        case FN_FORMULA_CALC :
        {
            Point aBL = GetItemRect( FN_FORMULA_CALC ).BottomLeft();
            Point aPt(aBL.X(), aBL.Y());
#ifdef VCL
            aPopMenu.Execute( this, aPt );
#else
            aPt = OutputToScreenPixel( aPt );
            aPopMenu.Execute( aPt );
#endif
        }
        break;
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

    // JP 13.01.97: Formel soll immer mit einem "=" beginnen, hier
    //              also wieder entfernen
    BOOL bIsFormula = FALSE;
    String sEdit( aEdit.GetText() );
    sEdit.EraseLeadingChars().EraseTrailingChars();
    if( sEdit.Len() && '=' == sEdit.GetChar( 0 ) )
    {
        sEdit.Erase( 0, 1 );
        bIsFormula = TRUE;
    }

    pWrtShell->StartAllAction();
    if( bDelSel && pWrtShell->HasSelection() )
    {
        pWrtShell->StartUndo( UNDO_START );
        pWrtShell->DelRight();
    }
    else
    {
        pWrtShell->EnterStdMode();
        bDelSel = FALSE;
    }

    if( !bDelSel && pMgr->GetCurFld() && TYP_FORMELFLD == pMgr->GetCurTypeId() )
        pMgr->UpdateCurFld( pMgr->GetCurFld()->GetFormat(), aEmptyStr, sEdit );
    else if( sEdit.Len() )
    {
        if( bIsTable )
        {
            SfxItemSet aSet( pWrtShell->GetAttrPool(), RES_BOXATR_FORMULA, RES_BOXATR_FORMULA );
            aSet.Put( SwTblBoxFormula( sEdit ));
            pWrtShell->SetTblBoxFormulaAttrs( aSet );
            pWrtShell->UpdateTable();
        }
        else
        {
            SvNumberFormatter* pFormatter = pWrtShell->GetNumberFormatter();
            ULONG nSysNumFmt = pFormatter->GetFormatIndex( NF_NUMBER_STANDARD, LANGUAGE_SYSTEM);
            pMgr->InsertFld(TYP_FORMELFLD, GSE_FORMULA, aEmptyStr, sEdit, nSysNumFmt);
        }
     }

    if( bDelSel )
        pWrtShell->EndUndo( UNDO_END );
    pWrtShell->EndAllAction();

    pWrtShell->EndSelTblCells();
    pView->GetEditWin().GrabFocus();
    pView->GetViewFrame()->GetDispatcher()->Execute( FN_EDIT_FORMULA, SFX_CALLMODE_ASYNCHRON );
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

        String sNew( aEdit.GetText() );
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

            pWrtShell->GetDoc()->Delete( aPam );
            pWrtShell->GetDoc()->Insert( aPam, sNew );
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
        String sNew( aEdit.GetText() );
        pWrtShell->SwEditShell::Insert( sNew );
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
                cOpenBracket = '(', cCloseBracket = ')';
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
        ASSERT(nPos < aStr.Len(), Delimiter nicht gefunden.);
        ++nPos;
        SetSelection( Selection( nPos, nPos ));
    }
    else
    {
        BOOL bFound = FALSE;
        unsigned char cCh;
        USHORT nPos, nEndPos, nStartPos = (USHORT) aSelection.Min();
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
                if( cClose == (cCh = aActText.GetChar( nEndPos )) /*||
                    cCh == cCloseBracket*/ )
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
            nPos += aBoxes.Len();
        }
        else
        {
            String aTmp( (char)cOpen );
            aTmp += aBoxes;
            aTmp += (char)cClose;
            nPos = (USHORT)aSelection.Min();
            aActText.Insert( aTmp, nPos );
            nPos += aTmp.Len();
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


SwInputChild::SwInputChild(Window* pParent,
                                USHORT nId,
                                SfxBindings* ,
                                SfxChildWinInfo* ) :
                                SfxChildWindow( pParent, nId )
{
    SwView* pActiveView = GetActiveView();
    pDispatch = pActiveView ? pActiveView->GetViewFrame()->GetDispatcher() : 0;
    pWindow = new SwInputWindow( pParent );
    ((SwInputWindow*)pWindow)->Show();
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

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.88  2000/09/18 16:06:01  willem.vandorp
    OpenOffice header added.

    Revision 1.87  2000/09/07 15:59:27  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.86  2000/07/03 08:54:31  jp
    must changes for VCL

    Revision 1.85  2000/04/26 14:56:39  os
    GetName() returns const String&

    Revision 1.84  2000/04/18 14:54:44  os
    UNICODE

    Revision 1.83  2000/03/03 15:17:03  os
    StarView remainders removed

    Revision 1.82  1999/05/18 13:20:16  OS
    #66203# Undo im Dtor nur, wenn erforderlich


      Rev 1.81   18 May 1999 15:20:16   OS
   #66203# Undo im Dtor nur, wenn erforderlich

      Rev 1.80   24 Apr 1998 17:22:46   OS
   kein KeyInput waehrend die Rechenleiste aktiv ist #49301#

      Rev 1.79   12 Mar 1998 13:33:12   OM
   #48084# Formelfeld mit Standardformat einfuegen

      Rev 1.78   22 Jan 1998 20:07:40   JP
   CTOR des SwPaM umgestellt

      Rev 1.77   19 Jan 1998 20:08:32   JP
   Bug #46705#: Undo nur rufen, wenn etwas geloescht wurde

      Rev 1.76   24 Nov 1997 14:53:58   MA
   includes

      Rev 1.75   10 Nov 1997 11:43:12   OS
   HelpId fuer Edit #45436#

      Rev 1.74   30 Sep 1997 16:28:22   TJ

      Rev 1.73   19 Sep 1997 13:42:56   OS
   Edit mit WB_NOHIDESELECTION #43891#

      Rev 1.72   04 Sep 1997 17:14:02   MA
   includes

      Rev 1.71   01 Sep 1997 10:01:24   OS
   fuer VCL Window uebergeben

      Rev 1.70   29 Aug 1997 16:52:24   MH
   chg: Syntax

      Rev 1.69   29 Aug 1997 15:45:32   OS
   PopupMenu::Execute mit Window* fuer VCL

      Rev 1.68   29 Aug 1997 14:20:12   OS
   DLL-Umbau

      Rev 1.67   09 Jul 1997 17:36:10   HJS
   includes

      Rev 1.66   09 May 1997 09:28:42   JP
   Bug #39214#: Formel nur ersetzen, wenn die sich veraendert hat

      Rev 1.65   29 Apr 1997 16:18:50   OM
   Unsichtbare Benutzerfelder

      Rev 1.64   09 Apr 1997 16:18:42   MH
   chg: header

      Rev 1.63   07 Apr 1997 13:40:48   MH
   chg: header

      Rev 1.62   10 Feb 1997 16:26:40   OS
   Dispatcher im Childwindow merken

      Rev 1.61   07 Feb 1997 16:41:54   OS
   Dispatcher nur noch vom ViewFrame holen

      Rev 1.60   05 Feb 1997 14:32:00   JP
   FormatMenu bei Tabellenzellen entfernen

      Rev 1.59   29 Jan 1997 14:47:18   JP
   Umstellungen fuers neue Rechnen in Tabellen

      Rev 1.58   21 Jan 1997 13:48:40   JP
   Update der Eingabe in die TabellenBox

      Rev 1.57   14 Jan 1997 18:22:28   JP
   UpdateRange: Anfang des BoxNamens korrekt suchen

      Rev 1.56   14 Jan 1997 09:50:32   JP
   neu: SetFormula; UpdateRange: Crsr hinter die BoxNamen

      Rev 1.55   21 Nov 1996 17:56:44   MA
   chg: Kein ResMgr fuer GetImage()

      Rev 1.54   11 Nov 1996 11:11:30   MA
   ResMgr

      Rev 1.53   09 Nov 1996 13:51:40   OS
   PopupMenu immer im Click

      Rev 1.52   24 Oct 1996 13:36:38   JP
   String Umstellung: [] -> GetChar()

      Rev 1.51   10 Oct 1996 10:37:12   OS
   Registrierung der Toolbox am SfxImageManager

      Rev 1.50   28 Aug 1996 13:39:28   OS
   includes

      Rev 1.49   09 Jul 1996 17:52:42   OM
   Alternatives Formatmenue fuer Expressionfields

      Rev 1.48   26 Jun 1996 15:26:42   OS
   Aufruf von Dispatcher.Execute an 324 angepasst

      Rev 1.47   10 Jun 1996 18:47:04   HJS
   ; vergessen

      Rev 1.46   10 Jun 1996 11:38:08   OS
   Reihenfolge Apply und CancelFormula angepasst

      Rev 1.45   09 May 1996 15:34:12   OS
   HLineal an der View wird nicht mehr geloescht

      Rev 1.44   26 Apr 1996 10:00:32   SWG
   IMPL-Macro getauscht, GetInfo impl.

      Rev 1.43   10 Mar 1996 15:50:22   OS
   neu: Round

      Rev 1.42   29 Feb 1996 12:58:30   OS
   UpdateRange mit zusaetzlichem Parameter

      Rev 1.41   07 Feb 1996 07:29:20   OS
   Linkumstellung: return sollte ans Ende

      Rev 1.40   06 Feb 1996 15:21:16   JP
   Link Umstellung 305

------------------------------------------------------------------------*/






















