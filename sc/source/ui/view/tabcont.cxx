/*************************************************************************
 *
 *  $RCSfile: tabcont.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-22 18:37:38 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <sfx2/dispatch.hxx>
#include <vcl/sound.hxx>

#include "tabcont.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "scmod.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "dataobj.hxx"


// STATIC DATA -----------------------------------------------------------

//==================================================================

ScTabControl::ScTabControl( Window* pParent, ScViewData* pData ) :
            TabBar( pParent, WinBits( WB_BORDER | WB_3DLOOK | WB_SCROLL |
                                    WB_RANGESELECT | WB_MULTISELECT | WB_DRAG | WB_SIZEABLE ) ),
            pViewData( pData ),
            bDragging( FALSE ),
            bErrorShown( FALSE ),
            bAddDown( FALSE )
{
    ScDocument* pDoc = pViewData->GetDocument();

    String aString;
    USHORT nCount = pDoc->GetTableCount();
    for (USHORT i=0; i<nCount; i++)
    {
        if (pDoc->IsVisible(i))
            if (pDoc->GetName(i,aString))
                if ( pDoc->IsScenario(i) )
                    InsertPage( i+1, aString, TPB_SPECIAL );
                else
                    InsertPage( i+1, aString );
    }

    SetCurPageId( pViewData->GetTabNo() + 1 );

    SetSizePixel( Size(SC_TABBAR_DEFWIDTH, 0) );

    SetSplitHdl( LINK( pViewData->GetView(), ScTabView, TabBarResize ) );

    EnableEditMode();
}

__EXPORT ScTabControl::~ScTabControl()
{
}

USHORT ScTabControl::GetMaxId() const
{
    USHORT nVisCnt = GetPageCount();
    if (nVisCnt)
        return GetPageId(nVisCnt-1);

    return 0;
}

USHORT ScTabControl::GetPrivatDropPos(const Point& rPos )
{
    USHORT nPos = ShowDropPos(rPos);

    USHORT nRealPos = nPos;

    if(nPos !=0 )
    {
        ScDocument* pDoc = pViewData->GetDocument();

        USHORT nCount = pDoc->GetTableCount();

        USHORT nViewPos=0;
        nRealPos = nCount;
        for (USHORT i=0; i<nCount; i++)
        {
            if (pDoc->IsVisible(i))
            {
                nViewPos++;
                if(nViewPos==nPos)
                {
                    USHORT j;
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

void __EXPORT ScTabControl::MouseButtonDown( const MouseEvent& rMEvt )
{
    bAddDown = FALSE;

    ScModule* pScMod = SC_MOD();
    if ( !pScMod->IsModalMode() && !pScMod->IsFormulaMode() && !IsInEditMode() )
    {
        //  View aktivieren
        pViewData->GetViewShell()->SetActive();         // Appear und SetViewFrame
        pViewData->GetView()->ActiveGrabFocus();
    }

    //  #47745# Klick neben Tabellen -> neue Tabelle einfuegen (wie im Draw)
    //  eine saubere linke Maustaste ohne verwaessernde Modifier (damit koennte
    /// ja das Kontextmenue gemeint sein)
    if ( rMEvt.IsLeft() && !rMEvt.IsMod1() && !rMEvt.IsMod2() && !rMEvt.IsShift())
    {
        Point aPos = PixelToLogic( rMEvt.GetPosPixel() );
        if ( GetPageId(aPos) == 0 )
            bAddDown = TRUE;                // erst im ButtonUp, weil ein Dialog kommt
    }

    TabBar::MouseButtonDown( rMEvt );
}

void __EXPORT ScTabControl::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( bAddDown )
    {
        Point aPos = PixelToLogic( rMEvt.GetPosPixel() );
        if ( GetPageId(aPos) == 0 )
        {
            SfxDispatcher* pDispatcher = pViewData->GetViewShell()->GetViewFrame()->GetDispatcher();
            pDispatcher->Execute( FID_INS_TABLE, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD );
        }
    }

    bAddDown = FALSE;
    TabBar::MouseButtonUp( rMEvt );
}

void __EXPORT ScTabControl::Select()
{
    ScModule* pScMod = SC_MOD();
    ScDocument* pDoc = pViewData->GetDocument();
    ScMarkData& rMark = pViewData->GetMarkData();
    USHORT nCount = pDoc->GetTableCount();
    USHORT i;

    if ( pScMod->IsTableLocked() )      // darf jetzt nicht umgeschaltet werden ?
    {
        //  den alten Zustand des TabControls wiederherstellen:

        for (i=0; i<nCount; i++)
            SelectPage( i+1, rMark.GetTableSelect(i) );
        SetCurPageId( pViewData->GetTabNo() + 1 );

        Sound::Beep();
        return;
    }

    USHORT nCurId = GetCurPageId();
    if (!nCurId) return;            // kann vorkommen, wenn bei Excel-Import alles versteckt ist
    USHORT nPage = nCurId - 1;

    // OLE-inplace deaktivieren
    if ( nPage != pViewData->GetTabNo() )
        pViewData->GetView()->DrawMarkListHasChanged();

    //  InputEnterHandler nur wenn nicht Referenzeingabe

    BOOL bRefMode = pScMod->IsFormulaMode();
    if (!bRefMode)
        pScMod->InputEnterHandler();

    for (i=0; i<nCount; i++)
        rMark.SelectTable( i, IsPageSelected(i+1) );

/*      Markierungen werden per Default nicht pro Tabelle gehalten
    USHORT nSelCnt = GetSelectPageCount();
    if (nSelCnt>1)
        pDoc->ExtendMarksFromTable( nPage );
*/

    SfxDispatcher& rDisp = pViewData->GetDispatcher();
    if (rDisp.IsLocked())
        pViewData->GetView()->SetTabNo( nPage );
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

    USHORT nCount = pDoc->GetTableCount();
    USHORT i;
    String aString;
    USHORT nMaxCnt = Max( nCount, GetMaxId() );

    BOOL bModified = FALSE;                                     // Tabellen-Namen
    for (i=0; i<nMaxCnt && !bModified; i++)
    {
        if (pDoc->IsVisible(i))
            pDoc->GetName(i,aString);
        else
            aString.Erase();

        if (GetPageText(i+1) != aString)
            bModified = TRUE;
    }

    if (bModified)
    {
        Clear();
        for (i=0; i<nCount; i++)
            if (pDoc->IsVisible(i))
                if (pDoc->GetName(i,aString))
                if ( pDoc->IsScenario(i) )
                    InsertPage( i+1, aString, TPB_SPECIAL );
                else
                    InsertPage( i+1, aString );
    }
    SetCurPageId( pViewData->GetTabNo() + 1 );

    if (bActive)
    {
        bModified = FALSE;                                          // Selektion
        for (i=0; i<nMaxCnt && !bModified; i++)
            if ( rMark.GetTableSelect(i) != IsPageSelected(i+1) )
                bModified = TRUE;

        if ( bModified )
            for (i=0; i<nCount; i++)
                SelectPage( i+1, rMark.GetTableSelect(i) );
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
    USHORT nCount = GetMaxId();

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
        rMark.SelectTable( nPage, TRUE );
    }
    Invalidate();
}

void __EXPORT ScTabControl::Command( const CommandEvent& rCEvt )
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
            //  nur eine Tabelle selektieren:
            /*
            USHORT nId = GetPageId( rCEvt.GetMousePosPixel() );
            if (nId)
            {
                SetCurPageId( nId );
                USHORT nCount = GetMaxId();

                for (USHORT i=1; i<=nCount; i++)
                    SelectPage( i, i==nId );
                Select();
            }
            */
            //  Popup-Menu:
            pViewSh->GetDispatcher()->ExecutePopup( ScResId(RID_POPUP_TAB) );
        }
    }
    else if ( nCmd == COMMAND_STARTDRAG )
    {
        if (!bDisable)
        {
            Region aRegion( Rectangle(0,0,0,0) );
            if (StartDrag( rCEvt, aRegion ))
                DoDrag( aRegion );
        }
    }
}


void ScTabControl::DoDrag( const Region& rRegion )
{
    ScModule* pScMod = SC_MOD();

    ScDocument* pDoc = pViewData->GetDocument();
    ScMarkData& rMark= pViewData->GetMarkData();

    USHORT nTab = pViewData->GetTabNo();
    pScMod->SetDragObject( rMark, ScRange(0,0,nTab,MAXCOL,MAXROW,nTab), 0,0, pDoc, SC_DROP_TABLE );

    bDragging = TRUE;           // um Tabellen innerhalb dieses TabBars zu verschieben

    SvDataObjectRef pDragServer = new ScDataObject(pDoc,FALSE,pViewData->GetDocShell());
    DropAction eAction = pDragServer->ExecuteDrag(pViewData->GetActiveWin(),
                                            Pointer(POINTER_MOVEDATA),
                                            Pointer(POINTER_COPYDATA),
                                            Pointer(POINTER_LINKDATA) );

    bDragging = FALSE;

    pScMod->ResetDragObject();

    //! eAction auswerten
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

BOOL __EXPORT ScTabControl::Drop( const DropEvent& rEvt )
{
    EndSwitchPage();

    ScModule* pScMod = SC_MOD();
    ScDocument* pSourceDoc = pScMod->GetDragData().pDoc;
    if (pSourceDoc && bDragging)
    {
        USHORT nPos = GetPrivatDropPos( rEvt.GetPosPixel() );
        HideDropPos();
        ScDocument* pDestDoc = pViewData->GetDocument();
        if ( pSourceDoc == pDestDoc && pSourceDoc->IsDocEditable() )
        {
            pViewData->GetView()->MoveTable( lcl_DocShellNr(pDestDoc), nPos, rEvt.GetAction()==DROP_COPY );

            pScMod->SetDragIntern();                // nicht loeschen
            return TRUE;
        }
        else
            Sound::Beep();
    }

    return FALSE;
}

BOOL __EXPORT ScTabControl::QueryDrop( DropEvent& rEvt )
{
    if (rEvt.IsLeaveWindow())
    {
        EndSwitchPage();
        HideDropPos();
        return TRUE;
    }

    if (bDragging)          // innerhalb eines TabControls
    {
        const ScDocument* pDoc = pViewData->GetDocument();
        if ( !pDoc->GetChangeTrack() && pDoc->IsDocEditable() )
        {
            ShowDropPos( rEvt.GetPosPixel() );
            return TRUE;
        }
    }
    else        // umschalten fuer alle Formate
    {
        SwitchPage( rEvt.GetPosPixel() );   // switch sheet after timeout
        return FALSE;                       // nothing can be dropped here
    }

    return FALSE;
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
        USHORT nTab = nId - 1;
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




