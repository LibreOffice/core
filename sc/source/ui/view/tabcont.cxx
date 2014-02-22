/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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

    OUString aString;
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

    SetScrollAlwaysEnabled(true);

    SetScrollAreaContextHdl( LINK( this, ScTabControl, ShowPageList ) );
}

IMPL_LINK(ScTabControl, ShowPageList, const CommandEvent *, pEvent)
{
    PopupMenu aPopup;

    sal_uInt16 nCurPageId = GetCurPageId();

    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nCount = pDoc->GetTableCount();
    for (SCTAB i=0; i<nCount; ++i)
    {
        if (pDoc->IsVisible(i))
        {
            OUString aString;
            if (pDoc->GetName(i, aString))
            {
                sal_uInt16 nId = static_cast<sal_uInt16>(i)+1;
                aPopup.InsertItem(nId, aString, MIB_CHECKABLE);
                if (nId == nCurPageId)
                    aPopup.CheckItem(nId);
            }
        }
    }

    sal_uInt16 nItemId = aPopup.Execute( this, pEvent->GetMousePosPixel() );
    SwitchToPageId(nItemId);

    return 0;
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
        
        pViewData->GetViewShell()->SetActive();         
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

    
    if( nMouseClickPageId != GetPageId( aPos, true ) )
        nMouseClickPageId = TabBar::PAGE_NOT_FOUND;

    if (nMouseClickPageId == TabBar::INSERT_TAB_POS)
    {
        
        ScDocument* pDoc = pViewData->GetDocument();
        ScModule* pScMod = SC_MOD();
        if (!pDoc->IsDocEditable() || pScMod->IsTableLocked())
            return;
        OUString aName;
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
        
        
        
        sal_uInt16 nSlot = ( GetSelectPageCount() > 1 ) ? FID_TAB_DESELECTALL : FID_INS_TABLE;
        SfxDispatcher* pDispatcher = pViewData->GetViewShell()->GetViewFrame()->GetDispatcher();
        pDispatcher->Execute( nSlot, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD );
        
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

    if ( pScMod->IsTableLocked() )      
    {
        

        for (i=0; i<nCount; i++)
            SelectPage( static_cast<sal_uInt16>(i)+1, rMark.GetTableSelect(i) );
        SetCurPageId( static_cast<sal_uInt16>(pViewData->GetTabNo()) + 1 );

        return;
    }

    sal_uInt16 nCurId = GetCurPageId();
    if (!nCurId) return;            
    sal_uInt16 nPage = nCurId - 1;

    
    if ( nPage != static_cast<sal_uInt16>(pViewData->GetTabNo()) )
        pViewData->GetView()->DrawMarkListHasChanged();

    

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

        
        
        

    if ( bRefMode && pViewData->GetRefType() == SC_REFTYPE_REF )
        if ( pViewData->GetViewShell()->GetViewFrame()->HasChildWindow(SID_OPENDLG_CONSOLIDATE) )
        {
            ScRange aRange(
                    pViewData->GetRefStartX(), pViewData->GetRefStartY(), pViewData->GetRefStartZ(),
                    pViewData->GetRefEndX(), pViewData->GetRefEndY(), pViewData->GetRefEndZ() );
            pScMod->SetReference( aRange, pDoc, &rMark );
            pScMod->EndReference();                     
        }
}

void ScTabControl::UpdateInputContext()
{
    ScDocument* pDoc = pViewData->GetDocument();
    WinBits nStyle = GetStyle();
    if (pDoc->GetDocumentShell()->IsReadOnly())
        
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
    OUString aString;
    SCTAB nMaxCnt = std::max( nCount, static_cast<SCTAB>(GetMaxId()) );
    Color aTabBgColor;

    sal_Bool bModified = false;                                     
    for (i=0; i<nMaxCnt && !bModified; i++)
    {
        if (pDoc->IsVisible(i))
        {
            pDoc->GetName(i,aString);
            aTabBgColor = pDoc->GetTabBgColor(i);
        }
        else
        {
            aString = OUString();
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
        bModified = false;                                          
        for (i=0; i<nMaxCnt && !bModified; i++)
            if ( rMark.GetTableSelect(i) != (bool) IsPageSelected(static_cast<sal_uInt16>(i)+1) )
                bModified = sal_True;

        
        
        if ( bModified )
            for (i=0; i<nCount; i++)
                SelectPage( static_cast<sal_uInt16>(i)+1, rMark.GetTableSelect(i) );
    }
    else
    {
    }
}

void ScTabControl::SetSheetLayoutRTL( bool bSheetRTL )
{
    SetEffectiveRTL( bSheetRTL );
    nSelPageIdByMouse = TabBar::PAGE_NOT_FOUND;
}

void ScTabControl::SwitchToPageId(sal_uInt16 nId)
{
    if (nId)
    {
        sal_Bool bAlreadySelected = IsPageSelected( nId );
        
        SetCurPageId( nId );
        
        
        if(!bAlreadySelected)
        {
            sal_uInt16 nCount = GetMaxId();

            for (sal_uInt16 i=1; i<=nCount; i++)
                SelectPage( i, i==nId );
            Select();
        }
    }
}

void ScTabControl::Command( const CommandEvent& rCEvt )
{
    ScModule*       pScMod   = SC_MOD();
    ScTabViewShell* pViewSh  = pViewData->GetViewShell();
    sal_Bool            bDisable = pScMod->IsFormulaMode() || pScMod->IsModalMode();

    
    pViewSh->SetActive();

    sal_uInt16 nCmd = rCEvt.GetCommand();
    if ( nCmd == COMMAND_CONTEXTMENU )
    {
        if (!bDisable)
        {
            
            
            
            sal_uInt16 nId = GetPageId( rCEvt.GetMousePosPixel() );
            SwitchToPageId(nId);

            
            pViewSh->DeactivateOle();

            
            
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
        CommandEvent aCEvt( rPosPixel, COMMAND_STARTDRAG, true );   
        if (TabBar::StartDrag( aCEvt, aRegion ))
            DoDrag( aRegion );
    }
}

void ScTabControl::DoDrag( const Region& /* rRegion */ )
{
    ScDocShell* pDocSh = pViewData->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();

    SCTAB nTab = pViewData->GetTabNo();
    ScRange aTabRange( 0, 0, nTab, MAXCOL, MAXROW, nTab );
    ScMarkData aTabMark = pViewData->GetMarkData();
    aTabMark.ResetMark();   
    aTabMark.SetMarkArea( aTabRange );

    ScDocument* pClipDoc = new ScDocument( SCDOCMODE_CLIP );
    ScClipParam aClipParam(aTabRange, false);
    pDoc->CopyToClip(aClipParam, pClipDoc, &aTabMark, false);

    TransferableObjectDescriptor aObjDesc;
    pDocSh->FillTransferableObjectDescriptor( aObjDesc );
    aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
    

    ScTransferObj* pTransferObj = new ScTransferObj( pClipDoc, aObjDesc );
    com::sun::star::uno::Reference<com::sun::star::datatransfer::XTransferable> xTransferable( pTransferObj );

    pTransferObj->SetDragSourceFlags( SC_DROP_TABLE );

    pTransferObj->SetDragSource( pDocSh, aTabMark );

    Window* pWindow = pViewData->GetActiveWin();
    SC_MOD()->SetDragObject( pTransferObj, NULL );      
    pTransferObj->StartDrag( pWindow, DND_ACTION_COPYMOVE | DND_ACTION_LINK );
}

static sal_uInt16 lcl_DocShellNr( ScDocument* pDoc )
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
        
        SCTAB nPos = GetPrivatDropPos( rEvt.maPosPixel );
        HideDropPos();

        if ( nPos == rData.pCellTransfer->GetVisibleTab() && rEvt.mnAction == DND_ACTION_MOVE )
        {
            
            
        }
        else
        {
            if ( !pDoc->GetChangeTrack() && pDoc->IsDocEditable() )
            {
                
                pViewData->GetView()->MoveTable( lcl_DocShellNr(pDoc), nPos, rEvt.mnAction != DND_ACTION_MOVE );

                rData.pCellTransfer->SetDragWasInternal();          
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
        
        if ( !pDoc->GetChangeTrack() && pDoc->IsDocEditable() )
        {
            ShowDropPos( rEvt.maPosPixel );
            return rEvt.mnAction;
        }
    }
    else                    
    {
        SwitchPage( rEvt.maPosPixel );      
        return 0;                           
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
        OUString aNewName = GetEditText();
        sal_Bool bDone = pViewSh->RenameTable( aNewName, nTab );
        if ( bDone )
            nRet = TABBAR_RENAMING_YES;
        else if ( bErrorShown )
        {
            
            
            
            OSL_FAIL("ScTabControl::AllowRenaming: nested calls");
            nRet = TABBAR_RENAMING_NO;
        }
        else if ( Application::IsInModalMode() )
        {
            
            
            nRet = TABBAR_RENAMING_CANCEL;
        }
        else
        {
            bErrorShown = true;
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
        nSelPageIdByMouse = TabBar::PAGE_NOT_FOUND;  
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
