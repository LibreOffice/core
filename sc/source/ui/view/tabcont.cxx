/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabcont.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:51:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <vcl/sound.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include "tabcont.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "scmod.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "transobj.hxx"


// STATIC DATA -----------------------------------------------------------

//==================================================================

ScTabControl::ScTabControl( Window* pParent, ScViewData* pData ) :
            TabBar( pParent, WinBits( WB_BORDER | WB_3DLOOK | WB_SCROLL |
                                    WB_RANGESELECT | WB_MULTISELECT | WB_DRAG | WB_SIZEABLE ) ),
            DropTargetHelper( this ),
            DragSourceHelper( this ),
            pViewData( pData ),
            nMouseClickPageId( TAB_PAGE_NOTFOUND ),
            nSelPageIdByMouse( TAB_PAGE_NOTFOUND ),
            bErrorShown( FALSE )
{
    ScDocument* pDoc = pViewData->GetDocument();

    String aString;
    SCTAB nCount = pDoc->GetTableCount();
    for (SCTAB i=0; i<nCount; i++)
    {
        if (pDoc->IsVisible(i))
            if (pDoc->GetName(i,aString))
                if ( pDoc->IsScenario(i) )
                    InsertPage( static_cast<sal_uInt16>(i)+1, aString, TPB_SPECIAL );
                else
                    InsertPage( static_cast<sal_uInt16>(i)+1, aString );
    }

    SetCurPageId( static_cast<sal_uInt16>(pViewData->GetTabNo()) + 1 );

    SetSizePixel( Size(SC_TABBAR_DEFWIDTH, 0) );

    SetSplitHdl( LINK( pViewData->GetView(), ScTabView, TabBarResize ) );

    EnableEditMode();
}

ScTabControl::~ScTabControl()
{
}

USHORT ScTabControl::GetMaxId() const
{
    USHORT nVisCnt = GetPageCount();
    if (nVisCnt)
        return GetPageId(nVisCnt-1);

    return 0;
}

SCTAB ScTabControl::GetPrivatDropPos(const Point& rPos )
{
    USHORT nPos = ShowDropPos(rPos);

    SCTAB nRealPos = static_cast<SCTAB>(nPos);

    if(nPos !=0 )
    {
        ScDocument* pDoc = pViewData->GetDocument();

        SCTAB nCount = pDoc->GetTableCount();

        USHORT nViewPos=0;
        nRealPos = nCount;
        for (SCTAB i=0; i<nCount; i++)
        {
            if (pDoc->IsVisible(i))
            {
                nViewPos++;
                if(nViewPos==nPos)
                {
                    SCTAB j;
                    for (j=i+1; j<nCount; j++)
                    {
                        if (pDoc->IsVisible(j))
                        {
                            break;
                        }
                    }
                    nRealPos =j;
                    break;
                }
            }
        }
    }
    return nRealPos ;
}

void ScTabControl::MouseButtonDown( const MouseEvent& rMEvt )
{
    ScModule* pScMod = SC_MOD();
    if ( !pScMod->IsModalMode() && !pScMod->IsFormulaMode() && !IsInEditMode() )
    {
        //  View aktivieren
        pViewData->GetViewShell()->SetActive();         // Appear und SetViewFrame
        pViewData->GetView()->ActiveGrabFocus();
    }

    /*  #47745# Click into free area -> insert new sheet (like in Draw).
        Needing clean left click without modifiers (may be context menu).
        #106948# Remember clicks to all pages, to be able to move mouse pointer later. */
    if( rMEvt.IsLeft() && (rMEvt.GetModifier() == 0) )
        nMouseClickPageId = GetPageId( rMEvt.GetPosPixel() );
    else
        nMouseClickPageId = TAB_PAGE_NOTFOUND;

    TabBar::MouseButtonDown( rMEvt );
}

void ScTabControl::MouseButtonUp( const MouseEvent& rMEvt )
{
    Point aPos = PixelToLogic( rMEvt.GetPosPixel() );

    // mouse button down and up on same page?
    if( nMouseClickPageId != GetPageId( aPos ) )
        nMouseClickPageId = TAB_PAGE_NOTFOUND;

    if( nMouseClickPageId == 0 )
    {
        // free area clicked -> add new sheet
        SfxDispatcher* pDispatcher = pViewData->GetViewShell()->GetViewFrame()->GetDispatcher();
        pDispatcher->Execute( FID_INS_TABLE, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD );
        // forget page ID, to be really sure that the dialog is not called twice
        nMouseClickPageId = TAB_PAGE_NOTFOUND;
    }

    TabBar::MouseButtonUp( rMEvt );
}

void ScTabControl::Select()
{
    /*  Remember last clicked page ID. */
    nSelPageIdByMouse = nMouseClickPageId;
    /*  Reset nMouseClickPageId, so that next Select() call may invalidate
        nSelPageIdByMouse (i.e. if called from keyboard). */
    nMouseClickPageId = TAB_PAGE_NOTFOUND;

    ScModule* pScMod = SC_MOD();
    ScDocument* pDoc = pViewData->GetDocument();
    ScMarkData& rMark = pViewData->GetMarkData();
    SCTAB nCount = pDoc->GetTableCount();
    SCTAB i;

    if ( pScMod->IsTableLocked() )      // darf jetzt nicht umgeschaltet werden ?
    {
        //  den alten Zustand des TabControls wiederherstellen:

        for (i=0; i<nCount; i++)
            SelectPage( static_cast<sal_uInt16>(i)+1, rMark.GetTableSelect(i) );
        SetCurPageId( static_cast<sal_uInt16>(pViewData->GetTabNo()) + 1 );

        Sound::Beep();
        return;
    }

    USHORT nCurId = GetCurPageId();
    if (!nCurId) return;            // kann vorkommen, wenn bei Excel-Import alles versteckt ist
    USHORT nPage = nCurId - 1;

    // OLE-inplace deaktivieren
    if ( nPage != static_cast<sal_uInt16>(pViewData->GetTabNo()) )
        pViewData->GetView()->DrawMarkListHasChanged();

    //  InputEnterHandler nur wenn nicht Referenzeingabe

    BOOL bRefMode = pScMod->IsFormulaMode();
    if (!bRefMode)
        pScMod->InputEnterHandler();

    for (i=0; i<nCount; i++)
        rMark.SelectTable( i, IsPageSelected(static_cast<sal_uInt16>(i)+1) );

/*      Markierungen werden per Default nicht pro Tabelle gehalten
    USHORT nSelCnt = GetSelectPageCount();
    if (nSelCnt>1)
        pDoc->ExtendMarksFromTable( nPage );
*/

    SfxDispatcher& rDisp = pViewData->GetDispatcher();
    if (rDisp.IsLocked())
        pViewData->GetView()->SetTabNo( static_cast<SCTAB>(nPage) );
    else
    {
        //  Tabelle fuer Basic ist 1-basiert
        SfxUInt16Item aItem( SID_CURRENTTAB, nPage + 1 );
        rDisp.Execute( SID_CURRENTTAB, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                                &aItem, (void*) NULL );
    }

    SfxBindings& rBind = pViewData->GetBindings();
    rBind.Invalidate( FID_FILL_TAB );

    rBind.Invalidate( FID_INS_TABLE );
    rBind.Invalidate( FID_TAB_APPEND );
    rBind.Invalidate( FID_TAB_MOVE );
    rBind.Invalidate( FID_TAB_RENAME );
    rBind.Invalidate( FID_DELETE_TABLE );
    rBind.Invalidate( FID_TABLE_SHOW );
    rBind.Invalidate( FID_TABLE_HIDE );

        //  SetReference nur wenn der Konsolidieren-Dialog offen ist
        //  (fuer Referenzen ueber mehrere Tabellen)
        //  bei anderen gibt das nur unnoetiges Gezappel

    if ( bRefMode && pViewData->GetRefType() == SC_REFTYPE_REF )
        if ( pViewData->GetViewShell()->GetViewFrame()->HasChildWindow(SID_OPENDLG_CONSOLIDATE) )
        {
            ScRange aRange(
                    pViewData->GetRefStartX(), pViewData->GetRefStartY(), pViewData->GetRefStartZ(),
                    pViewData->GetRefEndX(), pViewData->GetRefEndY(), pViewData->GetRefEndZ() );
            pScMod->SetReference( aRange, pDoc, &rMark );
            pScMod->EndReference();                     // wegen Auto-Hide
        }
}

void ScTabControl::UpdateStatus()
{
    ScDocument* pDoc = pViewData->GetDocument();
    ScMarkData& rMark = pViewData->GetMarkData();
    BOOL bActive = pViewData->IsActive();

    SCTAB nCount = pDoc->GetTableCount();
    SCTAB i;
    String aString;
    SCTAB nMaxCnt = Max( nCount, static_cast<SCTAB>(GetMaxId()) );

    BOOL bModified = FALSE;                                     // Tabellen-Namen
    for (i=0; i<nMaxCnt && !bModified; i++)
    {
        if (pDoc->IsVisible(i))
            pDoc->GetName(i,aString);
        else
            aString.Erase();

        if (GetPageText(static_cast<sal_uInt16>(i)+1) != aString)
            bModified = TRUE;
    }

    if (bModified)
    {
        Clear();
        for (i=0; i<nCount; i++)
            if (pDoc->IsVisible(i))
                if (pDoc->GetName(i,aString))
                if ( pDoc->IsScenario(i) )
                    InsertPage( static_cast<sal_uInt16>(i)+1, aString, TPB_SPECIAL );
                else
                    InsertPage( static_cast<sal_uInt16>(i)+1, aString );
    }
    SetCurPageId( static_cast<sal_uInt16>(pViewData->GetTabNo()) + 1 );

    if (bActive)
    {
        bModified = FALSE;                                          // Selektion
        for (i=0; i<nMaxCnt && !bModified; i++)
            if ( rMark.GetTableSelect(i) != IsPageSelected(static_cast<sal_uInt16>(i)+1) )
                bModified = TRUE;

        if ( bModified )
            for (i=0; i<nCount; i++)
                SelectPage( static_cast<sal_uInt16>(i)+1, rMark.GetTableSelect(i) );
    }
    else
    {
    }
}

void ScTabControl::ActivateView(BOOL bActivate)
{
//  ScDocument* pDoc = pViewData->GetDocument();
    ScMarkData& rMark = pViewData->GetMarkData();

//  ResetMark direkt in TabView
//  pDoc->ResetMark();

    USHORT nCurId = GetCurPageId();
    if (!nCurId) return;            // kann vorkommen, wenn bei Excel-Import alles versteckt ist
    USHORT nPage = nCurId - 1;
//    USHORT nCount = GetMaxId();

    /*
    USHORT i;
    for (i=0; i<nCount; i++)
    {
        SelectPage( i+1, FALSE );
        if (bActivate)
            rMark.SelectTable( i, FALSE );
    }
    */
    if (bActivate)
    {
        SelectPage( nPage+1, TRUE );
        rMark.SelectTable( static_cast<SCTAB>(nPage), TRUE );
    }
    Invalidate();
}

void ScTabControl::SetSheetLayoutRTL( BOOL bSheetRTL )
{
    /*  #106948# mirror the tabbar control, if sheet RTL mode differs from UI RTL mode
        - In LTR Office the tabbar is mirrored for RTL sheets.
        - In RTL Office the tabbar is mirrored anyway, mirror it again for LTR sheets. */
    SetMirrored( bSheetRTL != GetSettings().GetLayoutRTL() );
    // forget last selected sheet also if not mirrored (Mirror() is not called then)
    nSelPageIdByMouse = TAB_PAGE_NOTFOUND;
}


void ScTabControl::Command( const CommandEvent& rCEvt )
{
    ScModule*       pScMod   = SC_MOD();
    ScTabViewShell* pViewSh  = pViewData->GetViewShell();
    BOOL            bDisable = pScMod->IsFormulaMode() || pScMod->IsModalMode();

    // ViewFrame erstmal aktivieren (Bug 19493):
    pViewSh->SetActive();

    USHORT nCmd = rCEvt.GetCommand();
    if ( nCmd == COMMAND_CONTEXTMENU )
    {
        if (!bDisable)
        {
            // #i18735# select the page that is under the mouse cursor
            // if multiple tables are selected and the one under the cursor
            // is not part of them then unselect them
            USHORT nId = GetPageId( rCEvt.GetMousePosPixel() );
            if (nId)
            {
                BOOL bAlreadySelected = IsPageSelected( nId );
                //make the clicked page the current one
                SetCurPageId( nId );
                //change the selection when the current one is not already
                //selected or part of a multi selection
                if(!bAlreadySelected)
                {
                    USHORT nCount = GetMaxId();

                    for (USHORT i=1; i<=nCount; i++)
                        SelectPage( i, i==nId );
                    Select();
                }
            }

            // #i52073# OLE inplace editing has to be stopped before showing the sheet tab context menu
            pViewSh->DeactivateOle();

            //  Popup-Menu:
            //  get Dispatcher from ViewData (ViewFrame) instead of Shell (Frame), so it can't be null
            pViewData->GetDispatcher().ExecutePopup( ScResId(RID_POPUP_TAB) );
        }
    }
}

void ScTabControl::StartDrag( sal_Int8 /* nAction */, const Point& rPosPixel )
{
    ScModule* pScMod = SC_MOD();
    BOOL bDisable = pScMod->IsFormulaMode() || pScMod->IsModalMode();

    if (!bDisable)
    {
        Region aRegion( Rectangle(0,0,0,0) );
        CommandEvent aCEvt( rPosPixel, COMMAND_STARTDRAG, TRUE );   // needed for StartDrag
        if (TabBar::StartDrag( aCEvt, aRegion ))
            DoDrag( aRegion );
    }
}

void ScTabControl::DoDrag( const Region& /* rRegion */ )
{
    ScDocShell* pDocSh = pViewData->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();

    SCTAB nTab = pViewData->GetTabNo();
    ScMarkData aTabMark = pViewData->GetMarkData();
    aTabMark.ResetMark();   // doesn't change marked table information
    aTabMark.SetMarkArea( ScRange(0,0,nTab,MAXCOL,MAXROW,nTab) );

    ScDocument* pClipDoc = new ScDocument( SCDOCMODE_CLIP );
    pDoc->CopyToClip( 0,0, MAXCOL,MAXROW, FALSE, pClipDoc, FALSE, &aTabMark );

    TransferableObjectDescriptor aObjDesc;
    pDocSh->FillTransferableObjectDescriptor( aObjDesc );
    aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
    // maSize is set in ScTransferObj ctor

    ScTransferObj* pTransferObj = new ScTransferObj( pClipDoc, aObjDesc );
    com::sun::star::uno::Reference<com::sun::star::datatransfer::XTransferable> xTransferable( pTransferObj );

    pTransferObj->SetDragSourceFlags( SC_DROP_TABLE );

    pTransferObj->SetDragSource( pDocSh, aTabMark );

    Window* pWindow = pViewData->GetActiveWin();
    SC_MOD()->SetDragObject( pTransferObj, NULL );      // for internal D&D
    pTransferObj->StartDrag( pWindow, DND_ACTION_COPYMOVE | DND_ACTION_LINK );
}

USHORT lcl_DocShellNr( ScDocument* pDoc )
{
    USHORT nShellCnt = 0;
    SfxObjectShell* pShell = SfxObjectShell::GetFirst();
    while ( pShell )
    {
        if ( pShell->Type() == TYPE(ScDocShell) )
        {
            if ( ((ScDocShell*)pShell)->GetDocument() == pDoc )
                return nShellCnt;

            ++nShellCnt;
        }
        pShell = SfxObjectShell::GetNext( *pShell );
    }

    DBG_ERROR("Dokument nicht gefunden");
    return 0;
}

sal_Int8 ScTabControl::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    EndSwitchPage();

    ScDocument* pDoc = pViewData->GetDocument();
    const ScDragData& rData = SC_MOD()->GetDragData();
    if ( rData.pCellTransfer && ( rData.pCellTransfer->GetDragSourceFlags() & SC_DROP_TABLE ) &&
            rData.pCellTransfer->GetSourceDocument() == pDoc )
    {
        // moving of tables within the document
        SCTAB nPos = GetPrivatDropPos( rEvt.maPosPixel );
        HideDropPos();

        if ( nPos == rData.pCellTransfer->GetVisibleTab() && rEvt.mnAction == DND_ACTION_MOVE )
        {
            // #i83005# do nothing - don't move to the same position
            // (too easily triggered unintentionally, and might take a long time in large documents)
        }
        else
        {
            if ( !pDoc->GetChangeTrack() && pDoc->IsDocEditable() )
            {
                //! use table selection from the tab control where dragging was started?
                pViewData->GetView()->MoveTable( lcl_DocShellNr(pDoc), nPos, rEvt.mnAction != DND_ACTION_MOVE );

                rData.pCellTransfer->SetDragWasInternal();          // don't delete
                return TRUE;
            }
            else
                Sound::Beep();
        }
    }

    return 0;
}

sal_Int8 ScTabControl::AcceptDrop( const AcceptDropEvent& rEvt )
{
    if ( rEvt.mbLeaving )
    {
        EndSwitchPage();
        HideDropPos();
        return rEvt.mnAction;
    }

    const ScDocument* pDoc = pViewData->GetDocument();
    const ScDragData& rData = SC_MOD()->GetDragData();
    if ( rData.pCellTransfer && ( rData.pCellTransfer->GetDragSourceFlags() & SC_DROP_TABLE ) &&
            rData.pCellTransfer->GetSourceDocument() == pDoc )
    {
        // moving of tables within the document
        if ( !pDoc->GetChangeTrack() && pDoc->IsDocEditable() )
        {
            ShowDropPos( rEvt.maPosPixel );
            return rEvt.mnAction;
        }
    }
    else                    // switch sheets for all formats
    {
        SwitchPage( rEvt.maPosPixel );      // switch sheet after timeout
        return 0;                           // nothing can be dropped here
    }

    return 0;
}

long ScTabControl::StartRenaming()
{
    if ( pViewData->GetDocument()->IsDocEditable() )
        return TAB_RENAMING_YES;
    else
        return TAB_RENAMING_NO;
}

long ScTabControl::AllowRenaming()
{
    ScTabViewShell* pViewSh = pViewData->GetViewShell();
    DBG_ASSERT( pViewSh, "pViewData->GetViewShell()" );

    long nRet = TAB_RENAMING_CANCEL;
    USHORT nId = GetEditPageId();
    if ( nId )
    {
        SCTAB nTab = nId - 1;
        String aNewName = GetEditText();
        BOOL bDone = pViewSh->RenameTable( aNewName, nTab );
        if ( bDone )
            nRet = TAB_RENAMING_YES;
        else if ( bErrorShown )
        {
            //  if the error message from this TabControl is currently visible,
            //  don't end edit mode now, to avoid problems when returning to
            //  the other call (showing the error) - this should not happen
            DBG_ERROR("ScTabControl::AllowRenaming: nested calls");
            nRet = TAB_RENAMING_NO;
        }
        else if ( Application::IsInModalMode() )
        {
            //  #73472# don't show error message above any modal dialog
            //  instead cancel renaming without error message
            nRet = TAB_RENAMING_CANCEL;
        }
        else
        {
            bErrorShown = TRUE;
            pViewSh->ErrorMessage( STR_INVALIDTABNAME );
            bErrorShown = FALSE;
            nRet = TAB_RENAMING_NO;
        }
    }
    return nRet;
}

void ScTabControl::EndRenaming()
{
    if ( HasFocus() )
        pViewData->GetView()->ActiveGrabFocus();
}

void ScTabControl::Mirror()
{
    TabBar::Mirror();
    if( nSelPageIdByMouse != TAB_PAGE_NOTFOUND )
    {
        Rectangle aRect( GetPageRect( GetCurPageId() ) );
        if( !aRect.IsEmpty() )
            SetPointerPosPixel( aRect.Center() );
        nSelPageIdByMouse = TAB_PAGE_NOTFOUND;  // only once after a Select()
    }
}



