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

#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
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
#include "clipparam.hxx"
#include "dragdata.hxx"
#include "markdata.hxx"

// STATIC DATA -----------------------------------------------------------

//==================================================================

ScTabControl::ScTabControl( Window* pParent, ScViewData* pData ) :
    TabBar( pParent,
            WinBits(WB_BORDER | WB_3DLOOK | WB_SCROLL | WB_RANGESELECT |
                    WB_MULTISELECT | WB_DRAG | WB_SIZEABLE) ),
            DropTargetHelper( this ),
            DragSourceHelper( this ),
            pViewData( pData ),
            nMouseClickPageId( TabBar::PAGE_NOT_FOUND ),
            nSelPageIdByMouse( TabBar::PAGE_NOT_FOUND ),
            bErrorShown( false )
{
    ScDocument* pDoc = pViewData->GetDocument();

    rtl::OUString aString;
    Color aTabBgColor;
    SCTAB nCount = pDoc->GetTableCount();
    for (SCTAB i=0; i<nCount; i++)
    {
        if (pDoc->IsVisible(i))
        {
            if (pDoc->GetName(i,aString))
            {
                if ( pDoc->IsScenario(i) )
                    InsertPage( static_cast<sal_uInt16>(i)+1, aString, TPB_SPECIAL );
                else
                    InsertPage( static_cast<sal_uInt16>(i)+1, aString );
                if ( !pDoc->IsDefaultTabBgColor(i) )
                {
                    aTabBgColor = pDoc->GetTabBgColor(i);
                    SetTabBgColor( static_cast<sal_uInt16>(i)+1, aTabBgColor );
                }
            }
        }
    }

    SetCurPageId( static_cast<sal_uInt16>(pViewData->GetTabNo()) + 1 );

    SetSizePixel( Size(SC_TABBAR_DEFWIDTH, 0) );

    SetSplitHdl( LINK( pViewData->GetView(), ScTabView, TabBarResize ) );

    EnableEditMode();
    UpdateInputContext();
}

ScTabControl::~ScTabControl()
{
}

sal_uInt16 ScTabControl::GetMaxId() const
{
    sal_uInt16 nVisCnt = GetPageCount();
    if (nVisCnt)
        return GetPageId(nVisCnt-1);

    return 0;
}

SCTAB ScTabControl::GetPrivatDropPos(const Point& rPos )
{
    sal_uInt16 nPos = ShowDropPos(rPos);

    SCTAB nRealPos = static_cast<SCTAB>(nPos);

    if(nPos !=0 )
    {
        ScDocument* pDoc = pViewData->GetDocument();

        SCTAB nCount = pDoc->GetTableCount();

        sal_uInt16 nViewPos=0;
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

    /*  Click into free area -> insert new sheet (like in Draw).
        Needing clean left click without modifiers (may be context menu).
        Remember clicks to all pages, to be able to move mouse pointer later. */
    if( rMEvt.IsLeft() && (rMEvt.GetModifier() == 0) )
        nMouseClickPageId = GetPageId( rMEvt.GetPosPixel(), true );
    else
        nMouseClickPageId = TabBar::PAGE_NOT_FOUND;

    TabBar::MouseButtonDown( rMEvt );
}

void ScTabControl::MouseButtonUp( const MouseEvent& rMEvt )
{
    Point aPos = PixelToLogic( rMEvt.GetPosPixel() );

    // mouse button down and up on same page?
    if( nMouseClickPageId != GetPageId( aPos, true ) )
        nMouseClickPageId = TabBar::PAGE_NOT_FOUND;

    if (nMouseClickPageId == TabBar::INSERT_TAB_POS)
    {
        // Insert a new sheet at the right end, with default name.
        ScDocument* pDoc = pViewData->GetDocument();
        if (!pDoc->IsDocEditable())
            return;
        rtl::OUString aName;
        pDoc->CreateValidTabName(aName);
        SCTAB nTabCount = pDoc->GetTableCount();
        pViewData->GetViewShell()->InsertTable(aName, nTabCount);
        return;
    }

    if ( rMEvt.GetClicks() == 2 && rMEvt.IsLeft() && nMouseClickPageId != 0 && nMouseClickPageId != TAB_PAGE_NOTFOUND )
    {
        SfxDispatcher* pDispatcher = pViewData->GetViewShell()->GetViewFrame()->GetDispatcher();
        pDispatcher->Execute( FID_TAB_MENU_RENAME, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD );
        return;
    }

    if( nMouseClickPageId == 0 )
    {
        // Click in the area next to the existing tabs:
        // #i70320# if several sheets are selected, deselect all ecxept the current sheet,
        // otherwise add new sheet
        sal_uInt16 nSlot = ( GetSelectPageCount() > 1 ) ? FID_TAB_DESELECTALL : FID_INS_TABLE;
        SfxDispatcher* pDispatcher = pViewData->GetViewShell()->GetViewFrame()->GetDispatcher();
        pDispatcher->Execute( nSlot, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD );
        // forget page ID, to be really sure that the dialog is not called twice
        nMouseClickPageId = TabBar::PAGE_NOT_FOUND;
    }

    TabBar::MouseButtonUp( rMEvt );
}

void ScTabControl::Select()
{
    /*  Remember last clicked page ID. */
    nSelPageIdByMouse = nMouseClickPageId;
    /*  Reset nMouseClickPageId, so that next Select() call may invalidate
        nSelPageIdByMouse (i.e. if called from keyboard). */
    nMouseClickPageId = TabBar::PAGE_NOT_FOUND;

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

        return;
    }

    sal_uInt16 nCurId = GetCurPageId();
    if (!nCurId) return;            // kann vorkommen, wenn bei Excel-Import alles versteckt ist
    sal_uInt16 nPage = nCurId - 1;

    // OLE-inplace deaktivieren
    if ( nPage != static_cast<sal_uInt16>(pViewData->GetTabNo()) )
        pViewData->GetView()->DrawMarkListHasChanged();

    //  InputEnterHandler nur wenn nicht Referenzeingabe

    sal_Bool bRefMode = pScMod->IsFormulaMode();
    if (!bRefMode)
        pScMod->InputEnterHandler();

    for (i=0; i<nCount; i++)
        rMark.SelectTable( i, IsPageSelected(static_cast<sal_uInt16>(i)+1) );

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
    rBind.Invalidate( FID_TAB_DESELECTALL );

    rBind.Invalidate( FID_INS_TABLE );
    rBind.Invalidate( FID_TAB_APPEND );
    rBind.Invalidate( FID_TAB_MOVE );
    rBind.Invalidate( FID_TAB_RENAME );
    rBind.Invalidate( FID_DELETE_TABLE );
    rBind.Invalidate( FID_TABLE_SHOW );
    rBind.Invalidate( FID_TABLE_HIDE );
    rBind.Invalidate( FID_TAB_SET_TAB_BG_COLOR );

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

void ScTabControl::UpdateInputContext()
{
    ScDocument* pDoc = pViewData->GetDocument();
    WinBits nStyle = GetStyle();
    if (pDoc->GetDocumentShell()->IsReadOnly())
        // no insert sheet tab for readonly doc.
        SetStyle((nStyle & ~WB_INSERTTAB));
    else
        SetStyle((nStyle | WB_INSERTTAB));
}

void ScTabControl::UpdateStatus()
{
    ScDocument* pDoc = pViewData->GetDocument();
    ScMarkData& rMark = pViewData->GetMarkData();
    sal_Bool bActive = pViewData->IsActive();

    SCTAB nCount = pDoc->GetTableCount();
    SCTAB i;
    rtl::OUString aString;
    SCTAB nMaxCnt = Max( nCount, static_cast<SCTAB>(GetMaxId()) );
    Color aTabBgColor;

    sal_Bool bModified = false;                                     // Tabellen-Namen
    for (i=0; i<nMaxCnt && !bModified; i++)
    {
        if (pDoc->IsVisible(i))
        {
            pDoc->GetName(i,aString);
            aTabBgColor = pDoc->GetTabBgColor(i);
        }
        else
        {
            aString = rtl::OUString();
        }

        if ( !aString.equals(GetPageText(static_cast<sal_uInt16>(i)+1)) || (GetTabBgColor(static_cast<sal_uInt16>(i)+1) != aTabBgColor) )
            bModified = sal_True;
    }

    if (bModified)
    {
        Clear();
        for (i=0; i<nCount; i++)
        {
            if (pDoc->IsVisible(i))
            {
                if (pDoc->GetName(i,aString))
                {
                    if ( pDoc->IsScenario(i) )
                        InsertPage( static_cast<sal_uInt16>(i)+1, aString, TPB_SPECIAL );
                    else
                        InsertPage( static_cast<sal_uInt16>(i)+1, aString );
                    if ( !pDoc->IsDefaultTabBgColor(i) )
                    {
                        aTabBgColor = pDoc->GetTabBgColor(i);
                        SetTabBgColor( static_cast<sal_uInt16>(i)+1, aTabBgColor );
                    }
                }
            }
        }
    }
    SetCurPageId( static_cast<sal_uInt16>(pViewData->GetTabNo()) + 1 );

    if (bActive)
    {
        bModified = false;                                          // Selektion
        for (i=0; i<nMaxCnt && !bModified; i++)
            if ( rMark.GetTableSelect(i) != (bool) IsPageSelected(static_cast<sal_uInt16>(i)+1) )
                bModified = sal_True;

        // #i99576# the following loop is mis-optimized on unxsoli4 and the reason
        // why this file is in NOOPTFILES.
        if ( bModified )
            for (i=0; i<nCount; i++)
                SelectPage( static_cast<sal_uInt16>(i)+1, rMark.GetTableSelect(i) );
    }
    else
    {
    }
}

void ScTabControl::SetSheetLayoutRTL( sal_Bool bSheetRTL )
{
    SetEffectiveRTL( bSheetRTL );
    nSelPageIdByMouse = TabBar::PAGE_NOT_FOUND;
}


void ScTabControl::Command( const CommandEvent& rCEvt )
{
    ScModule*       pScMod   = SC_MOD();
    ScTabViewShell* pViewSh  = pViewData->GetViewShell();
    sal_Bool            bDisable = pScMod->IsFormulaMode() || pScMod->IsModalMode();

    // ViewFrame erstmal aktivieren (Bug 19493):
    pViewSh->SetActive();

    sal_uInt16 nCmd = rCEvt.GetCommand();
    if ( nCmd == COMMAND_CONTEXTMENU )
    {
        if (!bDisable)
        {
            // #i18735# select the page that is under the mouse cursor
            // if multiple tables are selected and the one under the cursor
            // is not part of them then unselect them
            sal_uInt16 nId = GetPageId( rCEvt.GetMousePosPixel() );
            if (nId)
            {
                sal_Bool bAlreadySelected = IsPageSelected( nId );
                //make the clicked page the current one
                SetCurPageId( nId );
                //change the selection when the current one is not already
                //selected or part of a multi selection
                if(!bAlreadySelected)
                {
                    sal_uInt16 nCount = GetMaxId();

                    for (sal_uInt16 i=1; i<=nCount; i++)
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
    sal_Bool bDisable = pScMod->IsFormulaMode() || pScMod->IsModalMode();

    if (!bDisable)
    {
        Region aRegion( Rectangle(0,0,0,0) );
        CommandEvent aCEvt( rPosPixel, COMMAND_STARTDRAG, sal_True );   // needed for StartDrag
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
    ScClipParam aClipParam(ScRange(0, 0, 0, MAXCOL, MAXROW, 0), false);
    pDoc->CopyToClip(aClipParam, pClipDoc, &aTabMark, false);

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

sal_uInt16 lcl_DocShellNr( ScDocument* pDoc )
{
    sal_uInt16 nShellCnt = 0;
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

    OSL_FAIL("Dokument nicht gefunden");
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
                return sal_True;
            }
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
        return TABBAR_RENAMING_YES;
    else
        return TABBAR_RENAMING_NO;
}

long ScTabControl::AllowRenaming()
{
    ScTabViewShell* pViewSh = pViewData->GetViewShell();
    OSL_ENSURE( pViewSh, "pViewData->GetViewShell()" );

    long nRet = TABBAR_RENAMING_CANCEL;
    sal_uInt16 nId = GetEditPageId();
    if ( nId )
    {
        SCTAB nTab = nId - 1;
        String aNewName = GetEditText();
        sal_Bool bDone = pViewSh->RenameTable( aNewName, nTab );
        if ( bDone )
            nRet = TABBAR_RENAMING_YES;
        else if ( bErrorShown )
        {
            //  if the error message from this TabControl is currently visible,
            //  don't end edit mode now, to avoid problems when returning to
            //  the other call (showing the error) - this should not happen
            OSL_FAIL("ScTabControl::AllowRenaming: nested calls");
            nRet = TABBAR_RENAMING_NO;
        }
        else if ( Application::IsInModalMode() )
        {
            //  don't show error message above any modal dialog
            //  instead cancel renaming without error message
            nRet = TABBAR_RENAMING_CANCEL;
        }
        else
        {
            bErrorShown = sal_True;
            pViewSh->ErrorMessage( STR_INVALIDTABNAME );
            bErrorShown = false;
            nRet = TABBAR_RENAMING_NO;
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
    if( nSelPageIdByMouse != TabBar::PAGE_NOT_FOUND )
    {
        Rectangle aRect( GetPageRect( GetCurPageId() ) );
        if( !aRect.IsEmpty() )
            SetPointerPosPixel( aRect.Center() );
        nSelPageIdByMouse = TabBar::PAGE_NOT_FOUND;  // only once after a Select()
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
