/*************************************************************************
 *
 *  $RCSfile: tabvwsh4.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-22 18:31:22 $
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

#include <sfx2/topfrm.hxx>
#include "scitems.hxx"
#include <svx/boxitem.hxx>
#include <svx/fmshell.hxx>
#include <svx/sizeitem.hxx>
#include <svx/boxitem.hxx>
#include <svx/prtqry.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <svtools/printdlg.hxx>
#include <svtools/whiter.hxx>

#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif

#include "tabvwsh.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "docsh.hxx"
#include "scmod.hxx"
#include "appoptio.hxx"
#include "rangeutl.hxx"
#include "printfun.hxx"
#include "drawsh.hxx"
#include "drformsh.hxx"
#include "editsh.hxx"
#include "pivotsh.hxx"
#include "auditsh.hxx"
#include "drtxtob.hxx"
#include "inputhdl.hxx"
#include "editutil.hxx"
#include "inputopt.hxx"
#include "inputwin.hxx"
#include "scresid.hxx"
#include "dbcolect.hxx"     // fuer ReImport
#include "reffact.hxx"
#include "viewuno.hxx"
#include "anyrefdg.hxx"
#include "chgtrack.hxx"
#include "cellsh.hxx"
#include "oleobjsh.hxx"
#include "chartsh.hxx"
#include "graphsh.hxx"
#include "pgbrksh.hxx"
#include "dpobject.hxx"
#include "prevwsh.hxx"


void ActivateOlk( ScViewData* pViewData );
void DeActivateOlk( ScViewData* pViewData );

extern SfxViewShell* pScActiveViewShell;            // global.cxx

using namespace com::sun::star;

// STATIC DATA -----------------------------------------------------------

USHORT ScTabViewShell::nInsertCtrlState = SID_INSERT_GRAPHIC;
USHORT ScTabViewShell::nInsCellsCtrlState = 0;
USHORT ScTabViewShell::nInsObjCtrlState = SID_DRAW_CHART;

// -----------------------------------------------------------------------

void __EXPORT ScTabViewShell::Activate(BOOL bMDI)
{
    SfxViewShell::Activate(bMDI);

    //  hier kein GrabFocus, sonst gibt's Probleme wenn etwas inplace editiert wird!

    if ( bMDI )
    {
        //  fuer Eingabezeile (ClearCache)
        ScModule* pScMod = SC_MOD();
        pScMod->ViewShellChanged();

        ActivateView( TRUE, bFirstActivate );
        ActivateOlk( GetViewData() );

        //  #56870# AutoCorrect umsetzen, falls der Writer seins neu angelegt hat
        UpdateDrawTextOutliner();

        //  RegisterNewTargetNames gibts nicht mehr

        SfxViewFrame* pThisFrame  = GetViewFrame();
        if ( pInputHandler && pThisFrame->HasChildWindow(FID_INPUTLINE_STATUS) )
        {
            //  eigentlich nur beim Reload (letzte Version) noetig:
            //  Das InputWindow bleibt stehen, aber die View mitsamt InputHandler wird
            //  neu angelegt, darum muss der InputHandler am InputWindow gesetzt werden.
            SfxChildWindow* pChild = pThisFrame->GetChildWindow(FID_INPUTLINE_STATUS);
            if (pChild)
            {
                ScInputWindow* pWin = (ScInputWindow*)pChild->GetWindow();
                if (pWin && pWin->IsVisible())
                {

                    ScInputHandler* pOldHdl=pWin->GetInputHandler();

                    TypeId aScType = TYPE(ScTabViewShell);

                    SfxViewShell* pSh = SfxViewShell::GetFirst( &aScType );
                    while ( pSh!=NULL && pOldHdl!=NULL)
                    {
                        if (((ScTabViewShell*)pSh)->GetInputHandler() == pOldHdl)
                        {
                            pOldHdl->ResetDelayTimer();
                            break;
                        }
                        pSh = SfxViewShell::GetNext( *pSh, &aScType );
                    }

                    pWin->SetInputHandler( pInputHandler );
                }
            }
        }

        UpdateInputHandler( TRUE );

        if ( bFirstActivate )
        {
            SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_NAVIGATOR_UPDATEALL ) );
            bFirstActivate = FALSE;
        }

        pScActiveViewShell = this;

        ScInputHandler* pHdl = pScMod->GetInputHdl(this);
        if (pHdl)
        {
            pHdl->SetRefScale( GetViewData()->GetZoomX(), GetViewData()->GetZoomY() );
        }

        //  Aenderungs-Dialog aktualisieren

        if ( pThisFrame->HasChildWindow(FID_CHG_ACCEPT) )
        {
            SfxChildWindow* pChild = pThisFrame->GetChildWindow(FID_CHG_ACCEPT);
            if (pChild)
            {
                ((ScAcceptChgDlgWrapper*)pChild)->ReInitDlg();
            }
        }

        if(pScMod->IsRefDialogOpen())
        {
            USHORT nCurRefDlgId=pScMod->GetCurRefDlgId();
            SfxChildWindow* pChildWnd = pThisFrame->GetChildWindow( nCurRefDlgId );
            if ( pChildWnd )
            {
                ScAnyRefDlg* pRefDlg = (ScAnyRefDlg*)pChildWnd->GetWindow();
                pRefDlg->ViewShellChanged(this);
            }
        }
    }

    //  Wenn Referenzeingabe-Tip-Hilfe hier wieder angezeigt werden soll (ShowRefTip),
    //  muss sie beim Verschieben der View angepasst werden (gibt sonst Probleme unter OS/2
    //  beim Umschalten zwischen Dokumenten)
}

void __EXPORT ScTabViewShell::Deactivate(BOOL bMDI)
{
    HideTip();

    ScDocument* pDoc=GetViewData()->GetDocument();

    ScChangeTrack* pChanges=pDoc->GetChangeTrack();

    if(pChanges!=NULL)
    {
        Link aLink;
        pChanges->SetModifiedLink(aLink);
    }

    SfxViewShell::Deactivate(bMDI);

    ScInputHandler* pHdl = SC_MOD()->GetInputHdl(this);

    if( bMDI )
    {
        DeActivateOlk( GetViewData() );
        ActivateView( FALSE, FALSE );

        if ( GetViewFrame()->ISA(SfxInPlaceFrame) )                         // inplace
            GetViewData()->GetDocShell()->UpdateOle(GetViewData(),TRUE);

        if ( pHdl )
            pHdl->NotifyChange( NULL, TRUE ); // Timer-verzoegert wg. Dokumentwechsel

        if (pScActiveViewShell == this)
            pScActiveViewShell = NULL;
    }
    else
    {
        HideNoteMarker();           // Notiz-Anzeige

        if ( pHdl )
            pHdl->HideTip();        // Formel-AutoEingabe-Tip abschalten
    }
}

void ScTabViewShell::SetActive()
{
    // Die Sfx-View moechte sich gerne selbst aktivieren, weil dabei noch
    // magische Dinge geschehen (z.B. stuerzt sonst evtl. der Gestalter ab)
    ActiveGrabFocus();

#if 0
    SfxViewFrame* pFrame = GetViewFrame();
    if ( pFrame->ISA(SfxTopViewFrame) )
        pFrame->GetFrame()->Appear();

    SFX_APP()->SetViewFrame( pFrame );          // immer erst Appear, dann SetViewFrame (#29290#)
#endif
}

USHORT __EXPORT ScTabViewShell::PrepareClose(BOOL bUI, BOOL bForBrowsing)
{
    if ( pFormShell )
    {
        USHORT nRet = pFormShell->PrepareClose(bUI, bForBrowsing);
        if (nRet!=TRUE)
            return nRet;
    }
    return SfxViewShell::PrepareClose(bUI,bForBrowsing);
}

//------------------------------------------------------------------

Size __EXPORT ScTabViewShell::GetOptimalSizePixel() const
{
    Size aOptSize;

    USHORT              nCurTab     = GetViewData()->GetTabNo();
    ScDocument*         pDoc        = GetViewData()->GetDocument();
    ScStyleSheetPool*   pStylePool  = pDoc->GetStyleSheetPool();
    SfxStyleSheetBase*  pStyleSheet = pStylePool->Find(
                                        pDoc->GetPageStyle( nCurTab ),
                                        SFX_STYLE_FAMILY_PAGE );

    DBG_ASSERT( pStyleSheet, "PageStyle not found :-/" );

    if ( pStyleSheet )
    {
        const SfxItemSet&  rSet      = pStyleSheet->GetItemSet();
        const SvxSizeItem& rItem     = (const SvxSizeItem&)rSet.Get( ATTR_PAGE_SIZE );
        const Size&        rPageSize = rItem.GetSize();

        aOptSize.Width()  = (long) (rPageSize.Width()  * GetViewData()->GetPPTX());
        aOptSize.Height() = (long) (rPageSize.Height() * GetViewData()->GetPPTY());
    }

    return aOptSize;
}

//------------------------------------------------------------------

//  Zoom fuer In-Place berechnen
//  aus Verhaeltnis von VisArea und Fenstergroesse des GridWin

void ScTabViewShell::UpdateOleZoom()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    if ( pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
    {
        Size aObjSize = ((SfxInPlaceObject*)pDocSh)->GetVisArea().GetSize();
        Window* pWin = GetActiveWin();
        Size aWinHMM = pWin->PixelToLogic( pWin->GetOutputSizePixel(), MAP_100TH_MM );
        SetZoomFactor( Fraction( aWinHMM.Width(),aObjSize.Width() ),
                        Fraction( aWinHMM.Height(),aObjSize.Height() ) );
    }
}

void __EXPORT ScTabViewShell::AdjustPosSizePixel( const Point &rPos, const Size &rSize )
{
    OuterResizePixel( rPos, rSize );
}

void __EXPORT ScTabViewShell::InnerResizePixel( const Point &rOfs, const Size &rSize )
{
    SvBorder aBorder;
    GetBorderSize( aBorder, rSize );
    SetBorderPixel( aBorder );

    Size aNewSize( rSize );
    aNewSize.Width()  += aBorder.Left() + aBorder.Right();
    aNewSize.Height() += aBorder.Top() + aBorder.Bottom();

    DoResize( rOfs, aNewSize, TRUE );                   // rSize = Groesse von gridwin

    UpdateOleZoom();                                    //  Zoom fuer In-Place berechnen

//  GetViewData()->GetDocShell()->UpdateOle( GetViewData() );
    GetViewData()->GetDocShell()->SetDocumentModified();
}

void __EXPORT ScTabViewShell::OuterResizePixel( const Point &rOfs, const Size &rSize )
{
    SvBorder aBorder;
    GetBorderSize( aBorder, rSize );
    SetBorderPixel( aBorder );

    DoResize( rOfs, rSize );                    // Position und Groesse von tabview wie uebergeben

    // ForceMove als Ersatz fuer den Sfx-Move-Mechanismus
    // (aWinPos muss aktuell gehalten werden, damit ForceMove beim Ole-Deaktivieren klappt)

    ForceMove();
}

void __EXPORT ScTabViewShell::SetZoomFactor( const Fraction &rZoomX, const Fraction &rZoomY )
{
    //  fuer OLE...

    Fraction aFrac20( 1,5 );
    Fraction aFrac400( 4,1 );

    Fraction aNewX( rZoomX );
    if ( aNewX < aFrac20 )
        aNewX = aFrac20;
    if ( aNewX > aFrac400 )
        aNewX = aFrac400;
    Fraction aNewY( rZoomY );
    if ( aNewY < aFrac20 )
        aNewY = aFrac20;
    if ( aNewY > aFrac400 )
        aNewY = aFrac400;

    GetViewData()->UpdateScreenZoom( aNewX, aNewY );
    SetZoom( aNewX, aNewY );

    PaintGrid();
    PaintTop();
    PaintLeft();

    SfxViewShell::SetZoomFactor( rZoomX, rZoomY );
}

void __EXPORT ScTabViewShell::QueryObjAreaPixel( Rectangle& rRect ) const
{
    //  auf ganze Zellen anpassen (in 1/100 mm)

    Size aPixelSize = rRect.GetSize();
    Window* pWin = ((ScTabViewShell*)this)->GetActiveWin();
    Size aLogicSize = pWin->PixelToLogic( aPixelSize );

    const ScViewData* pViewData = GetViewData();
    ScDocument* pDoc = pViewData->GetDocument();
    ScSplitPos ePos = pViewData->GetActivePart();
    USHORT nCol = pViewData->GetPosX(WhichH(ePos));
    USHORT nRow = pViewData->GetPosY(WhichV(ePos));
    USHORT nTab = pViewData->GetTabNo();
    Rectangle aLogicRect = pDoc->GetMMRect( nCol, nRow, nCol, nRow, nTab );
    aLogicRect.SetSize( aLogicSize );

    pDoc->SnapVisArea( aLogicRect );

    rRect.SetSize( pWin->LogicToPixel( aLogicRect.GetSize() ) );

#if 0
    //  auf ganze Zellen anpassen (in Pixeln)

    ScViewData* pViewData = ((ScTabViewShell*)this)->GetViewData();
    Size aSize = rRect.GetSize();

    ScSplitPos ePos = pViewData->GetActivePart();
    Window* pWin = ((ScTabViewShell*)this)->GetActiveWin();

    Point aTest( aSize.Width(), aSize.Height() );
    short nPosX;
    short nPosY;
    pViewData->GetPosFromPixel( aTest.X(), aTest.Y(), ePos, nPosX, nPosY );
    BOOL bLeft;
    BOOL bTop;
    pViewData->GetMouseQuadrant( aTest, ePos, nPosX, nPosY, bLeft, bTop );
    if (!bLeft)
        ++nPosX;
    if (!bTop)
        ++nPosY;
    aTest = pViewData->GetScrPos( (USHORT)nPosX, (USHORT)nPosY, ePos, TRUE );

    rRect.SetSize(Size(aTest.X(),aTest.Y()));
#endif
}

//------------------------------------------------------------------

void __EXPORT ScTabViewShell::Move()
{
    Point aNewPos = GetViewFrame()->GetWindow().OutputToScreenPixel(Point());

    if (aNewPos != aWinPos)
    {
        StopMarking();
        aWinPos = aNewPos;
    }
}

//------------------------------------------------------------------

void __EXPORT ScTabViewShell::ShowCursor(FASTBOOL bOn)
{
/*!!!   ShowCursor wird nicht paarweise wie im gridwin gerufen.
        Der CursorLockCount am Gridwin muss hier direkt auf 0 gesetzt werden

    if (bOn)
        ShowAllCursors();
    else
        HideAllCursors();
*/
}

//------------------------------------------------------------------

void __EXPORT ScTabViewShell::WriteUserData(String& rData, BOOL bBrowse)
{
    GetViewData()->WriteUserData(rData);
}

void __EXPORT ScTabViewShell::ReadUserData(const String& rData, BOOL bBrowse)
{
    DoReadUserData( rData );
}

// DoReadUserData is also called from ctor when switching from print preview

void ScTabViewShell::DoReadUserData( const String& rData )
{
    Window* pOldWin = GetActiveWin();
    BOOL bFocus = pOldWin && pOldWin->HasFocus();

    GetViewData()->ReadUserData(rData);
    SetTabNo( GetViewData()->GetTabNo(), TRUE );

    if ( GetViewData()->IsPagebreakMode() )
        SetCurSubShell( GetCurObjectSelectionType(), TRUE );

    Window* pNewWin = GetActiveWin();
    if (pNewWin && pNewWin != pOldWin)
    {
        SetWindow( pNewWin );       //! ist diese ViewShell immer aktiv???
        if (bFocus)
            pNewWin->GrabFocus();
        WindowChanged();            // Drawing-Layer (z.B. #56771#)
    }

    if (GetViewData()->GetHSplitMode() == SC_SPLIT_FIX ||
        GetViewData()->GetVSplitMode() == SC_SPLIT_FIX)
    {
        InvalidateSplit();
    }

    ZoomChanged();

    TestHintWindow();

    //! if ViewData has more tables than document, remove tables in ViewData
}


//------------------------------------------------------------------

void ScTabViewShell::TestFunction( USHORT nPar )
{
/*  switch (nPar)
    {
    }
*/
}

//------------------------------------------------------------------

void ScTabViewShell::ExecuteShowNIY( SfxRequest& rReq )
{
    ErrorMessage(STR_BOX_YNI);
}

//------------------------------------------------------------------

void ScTabViewShell::StateDisabled( SfxItemSet& rSet )
{
    SfxWhichIter aIter( rSet );
    USHORT       nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        rSet.DisableItem( nWhich );
        nWhich = aIter.NextWhich();
    }
}

void ScTabViewShell::SetDrawShellOrSub()
{
    bActiveDrawSh = TRUE;

    if(bActiveDrawFormSh)
    {
        SetCurSubShell(OST_DrawForm);
    }
    else if(bActiveGraphicSh)
    {
        SetCurSubShell(OST_Graphic);
    }
    else if(bActiveChartSh)
    {
        SetCurSubShell(OST_Chart);
    }
    else if(bActiveOleObjectSh)
    {
        SetCurSubShell(OST_OleObject);
    }
    else
    {
        SetCurSubShell(OST_Drawing);
    }
}

void ScTabViewShell::SetDrawShell( BOOL bActive )
{
    if(bActive)
    {
        SetCurSubShell(OST_Drawing);
    }
    else
    {
        if(bActiveDrawFormSh || bActiveDrawSh ||
            bActiveGraphicSh  || bActiveOleObjectSh||
            bActiveChartSh || bActiveDrawTextSh)
        {
            SetCurSubShell(OST_Cell);
        }
        bActiveDrawFormSh=FALSE;
        bActiveGraphicSh=FALSE;
        bActiveOleObjectSh=FALSE;
        bActiveChartSh=FALSE;
    }

    BOOL bWasDraw = bActiveDrawSh || bActiveDrawTextSh;

    bActiveDrawSh = bActive;
    bActiveDrawTextSh = FALSE;

    if ( !bActive )
    {
        ResetDrawDragMode();        //  Mirror / Rotate aus

        if (bWasDraw && (GetViewData()->GetHSplitMode() == SC_SPLIT_FIX ||
                         GetViewData()->GetVSplitMode() == SC_SPLIT_FIX))
        {
            //  Aktiven Teil an Cursor anpassen, etc.
            MoveCursorAbs( GetViewData()->GetCurX(), GetViewData()->GetCurY(),
                            SC_FOLLOW_NONE, FALSE, FALSE, TRUE );
        }
    }
}

void ScTabViewShell::SetDrawTextShell( BOOL bActive )
{
    bActiveDrawTextSh = bActive;
    if ( bActive )
    {
        bActiveDrawFormSh=FALSE;
        bActiveGraphicSh=FALSE;
        bActiveOleObjectSh=FALSE;
        bActiveChartSh=FALSE;
        bActiveDrawSh = FALSE;
        SetCurSubShell(OST_DrawText);
    }
    else
        SetCurSubShell(OST_Cell);

}

void ScTabViewShell::SetPivotShell( BOOL bActive )
{
    bActivePivotSh = bActive;

    //  #68771# #76198# SetPivotShell is called from CursorPosChanged every time
    //  -> don't change anything except switching between cell and pivot shell

    if ( eCurOST == OST_Pivot || eCurOST == OST_Cell )
    {
        if ( bActive )
        {
            bActiveDrawTextSh = bActiveDrawSh = FALSE;
            bActiveDrawFormSh=FALSE;
            bActiveGraphicSh=FALSE;
            bActiveOleObjectSh=FALSE;
            bActiveChartSh=FALSE;
            SetCurSubShell(OST_Pivot);
        }
        else
            SetCurSubShell(OST_Cell);
    }
}

void ScTabViewShell::SetAuditShell( BOOL bActive )
{
    bActiveAuditingSh = bActive;
    if ( bActive )
    {
        bActiveDrawTextSh = bActiveDrawSh = FALSE;
        bActiveDrawFormSh=FALSE;
        bActiveGraphicSh=FALSE;
        bActiveOleObjectSh=FALSE;
        bActiveChartSh=FALSE;
        SetCurSubShell(OST_Auditing);
    }
    else
        SetCurSubShell(OST_Cell);
}

void ScTabViewShell::SetDrawFormShell( BOOL bActive )
{
    bActiveDrawFormSh = bActive;

    if(bActiveDrawFormSh)
        SetCurSubShell(OST_DrawForm);
}
void ScTabViewShell::SetChartShell( BOOL bActive )
{
    bActiveChartSh = bActive;

    if(bActiveChartSh)
        SetCurSubShell(OST_Chart);
}

void ScTabViewShell::SetGraphicShell( BOOL bActive )
{
    bActiveGraphicSh = bActive;

    if(bActiveGraphicSh)
        SetCurSubShell(OST_Graphic);
}

void ScTabViewShell::SetOleObjectShell( BOOL bActive )
{
    bActiveOleObjectSh = bActive;

    if(bActiveOleObjectSh)
        SetCurSubShell(OST_OleObject);
    else
        SetCurSubShell(OST_Cell);
}

void ScTabViewShell::SetEditShell(EditView* pView, BOOL bActive )
{
    if(bActive)
    {
        if (pEditShell)
            pEditShell->SetEditView( pView );
        else
            pEditShell = new ScEditShell( pView, GetViewData() );

        SetCurSubShell(OST_Editing);
    }
    else if(bActiveEditSh)
    {
        SetCurSubShell(OST_Cell);
    }
    bActiveEditSh = bActive;
}

void ScTabViewShell::SetCurSubShell(ObjectSelectionType eOST, BOOL bForce)
{
    ScViewData* pViewData   = GetViewData();
    ScDocShell* pDocSh      = pViewData->GetDocShell();
    SfxShell*   pCurSubSh   = NULL;

    if(bDontSwitch) return;

    if(!pCellShell) //Wird eh immer gebraucht.
    {
        pCellShell = new ScCellShell( GetViewData() );
        pCellShell->SetRepeatTarget( &aTarget );
    }

    BOOL bPgBrk=pViewData->IsPagebreakMode();

    if(bPgBrk && !pPageBreakShell)
    {
        pPageBreakShell = new ScPageBreakShell( this );
        pPageBreakShell->SetRepeatTarget( &aTarget );
    }


    if ( eOST!=eCurOST || bForce )
    {
        if(eCurOST!=OST_NONE) RemoveSubShell();

        if(pFormShell) AddSubShell(*pFormShell);


        switch(eOST)
        {
            case    OST_Cell:
                    {
                        AddSubShell(*pCellShell);
                        if(bPgBrk) AddSubShell(*pPageBreakShell);
                    }
                    break;
            case    OST_Editing:
                    {
                        AddSubShell(*pCellShell);
                        if(bPgBrk) AddSubShell(*pPageBreakShell);

                        if(pEditShell)
                        {
                            AddSubShell(*pEditShell);
                        }
                    }
                    break;
            case    OST_DrawText:
                    {
                        if ( !pDrawTextShell )
                        {
                            pDocSh->MakeDrawLayer();
                            pDrawTextShell = new ScDrawTextObjectBar( GetViewData() );
                        }
                        AddSubShell(*pDrawTextShell);
                    }
                    break;
            case    OST_Drawing:
                    {
                        if ( !pDrawShell )
                        {
                            pDocSh->MakeDrawLayer();
                            pDrawShell = new ScDrawShell( GetViewData() );
                            pDrawShell->SetRepeatTarget( &aTarget );
                        }
                        AddSubShell(*pDrawShell);
                    }
                    break;

            case    OST_DrawForm:
                    {
                        if ( !pDrawFormShell )
                        {
                            pDocSh->MakeDrawLayer();
                            pDrawFormShell = new ScDrawFormShell( GetViewData() );
                            pDrawFormShell->SetRepeatTarget( &aTarget );
                        }
                        AddSubShell(*pDrawFormShell);
                    }
                    break;

            case    OST_Chart:
                    {
                        if ( !pChartShell )
                        {
                            pDocSh->MakeDrawLayer();
                            pChartShell = new ScChartShell( GetViewData() );
                            pChartShell->SetRepeatTarget( &aTarget );
                        }
                        AddSubShell(*pChartShell);
                    }
                    break;

            case    OST_OleObject:
                    {
                        if ( !pOleObjectShell )
                        {
                            pDocSh->MakeDrawLayer();
                            pOleObjectShell = new ScOleObjectShell( GetViewData() );
                            pOleObjectShell->SetRepeatTarget( &aTarget );
                        }
                        AddSubShell(*pOleObjectShell);
                    }
                    break;

            case    OST_Graphic:
                    {
                        if ( !pGraphicShell)
                        {
                            pDocSh->MakeDrawLayer();
                            pGraphicShell = new ScGraphicShell( GetViewData() );
                            pGraphicShell->SetRepeatTarget( &aTarget );
                        }
                        AddSubShell(*pGraphicShell);
                    }
                    break;

            case    OST_Pivot:
                    {
                        AddSubShell(*pCellShell);
                        if(bPgBrk) AddSubShell(*pPageBreakShell);

                        if ( !pPivotShell )
                        {
                            pPivotShell = new ScPivotShell( this );
                            pPivotShell->SetRepeatTarget( &aTarget );
                        }
                        AddSubShell(*pPivotShell);
                    }
                    break;
            case    OST_Auditing:
                    {
                        AddSubShell(*pCellShell);
                        if(bPgBrk) AddSubShell(*pPageBreakShell);

                        if ( !pAuditingShell )
                        {
                            pDocSh->MakeDrawLayer();    // die Wartezeit lieber jetzt als beim Klick

                            pAuditingShell = new ScAuditingShell( GetViewData() );
                            pAuditingShell->SetRepeatTarget( &aTarget );
                        }
                        AddSubShell(*pAuditingShell);
                    }
                    break;
            default:
                    DBG_ERROR("Falsche Shell angefordert");
                    break;
        }
        eCurOST=eOST;
    }
}

ObjectSelectionType ScTabViewShell::GetCurObjectSelectionType()
{
    return eCurOST;
}

//  GetMySubShell / SetMySubShell: altes Verhalten simulieren,
//  dass es nur eine SubShell gibt (nur innerhalb der 5 eignenen SubShells)

SfxShell* ScTabViewShell::GetMySubShell() const
{
    //  GetSubShell() war frueher const, und GetSubShell(USHORT) sollte es auch sein...

    USHORT nPos = 0;
    SfxShell* pSub = ((ScTabViewShell*)this)->GetSubShell(nPos);
    while (pSub)
    {
        if ( pSub == pDrawShell  || pSub == pDrawTextShell || pSub == pEditShell ||
             pSub == pPivotShell || pSub == pAuditingShell || pSub == pDrawFormShell ||
             pSub == pCellShell  || pSub == pOleObjectShell|| pSub == pChartShell ||
             pSub == pGraphicShell || pSub == pPageBreakShell)
            return pSub;    // gefunden

        pSub = ((ScTabViewShell*)this)->GetSubShell(++nPos);
    }
    return NULL;        // keine von meinen dabei
}

void ScTabViewShell::SetMySubShell( SfxShell* pShell )
{
    SfxShell* pOld = GetMySubShell();
    if ( pOld != pShell )
    {
        if (pOld)
            RemoveSubShell(pOld);   // alte SubShell entfernen
        if (pShell)
            AddSubShell(*pShell);   // neue setzen
    }
}

BOOL ScTabViewShell::IsDrawTextShell() const
{
    return ( pDrawTextShell && ( GetMySubShell() == pDrawTextShell ) );
}

BOOL ScTabViewShell::IsAuditShell() const
{
    return ( pAuditingShell && ( GetMySubShell() == pAuditingShell ) );
}

void ScTabViewShell::SetDrawTextUndo( SfxUndoManager* pUndoMgr )
{
    // Default: Undo-Manager der DocShell
    if (!pUndoMgr)
        pUndoMgr = GetViewData()->GetDocShell()->GetUndoManager();

    if (pDrawTextShell)
        pDrawTextShell->SetUndoManager(pUndoMgr);
    else
        DBG_ERROR("SetDrawTextUndo ohne DrawTextShell");
}

//------------------------------------------------------------------

ScTabViewShell* ScTabViewShell::GetActiveViewShell()
{
    return PTR_CAST(ScTabViewShell,Current());
}

//------------------------------------------------------------------

SfxPrinter* __EXPORT ScTabViewShell::GetPrinter( BOOL bCreate )
{
    //  Drucker ist immer da (wird fuer die FontListe schon beim Starten angelegt)
    return GetViewData()->GetDocShell()->GetPrinter();
}

USHORT __EXPORT ScTabViewShell::SetPrinter( SfxPrinter *pNewPrinter, USHORT nDiffFlags )
{
    return GetViewData()->GetDocShell()->SetPrinter( pNewPrinter, nDiffFlags );
}

PrintDialog* __EXPORT ScTabViewShell::CreatePrintDialog( Window *pParent )
{
    ScDocShell* pDocShell   = GetViewData()->GetDocShell();
    ScDocument* pDoc        = pDocShell->GetDocument();

    pDoc->SetPrintOptions();                // Optionen aus OFA am Printer setzen
    SfxPrinter* pPrinter = GetPrinter();

    String          aStrRange;
#ifndef VCL
    PrintDialog*    pDlg        = new PrintDialog( pParent,
                                                   WinBits(WB_SVLOOK|WB_STDMODAL) );
#else
    PrintDialog*    pDlg        = new PrintDialog( pParent);
#endif
    USHORT          i;
    USHORT          nTabCount   = pDoc->GetTableCount();
    long            nDocPageMax = 0;

    for ( i=0; i<nTabCount; i++ )
    {
        ScPrintFunc aPrintFunc( pDocShell, pPrinter, i );
        nDocPageMax += aPrintFunc.GetTotalPages();
    }

    if ( nDocPageMax > 0 )
    {
        aStrRange = '1';
        if ( nDocPageMax > 1 )
        {
            aStrRange += '-';
            aStrRange += String::CreateFromInt32( nDocPageMax );
        }
    }

    pDlg->SetRangeText  ( aStrRange );
    pDlg->EnableRange   ( PRINTDIALOG_ALL );
    pDlg->EnableRange   ( PRINTDIALOG_SELECTION );
    pDlg->EnableRange   ( PRINTDIALOG_RANGE );
    pDlg->SetFirstPage  ( 1 );
    pDlg->SetMinPage    ( 1 );
    pDlg->SetLastPage   ( (USHORT)nDocPageMax );
    pDlg->SetMaxPage    ( (USHORT)nDocPageMax );
    pDlg->EnableCollate ();

    return pDlg;
}

void __EXPORT ScTabViewShell::PreparePrint( PrintDialog* pPrintDialog )
{
    ScDocShell* pDocShell = GetViewData()->GetDocShell();

    SfxViewShell::PreparePrint( pPrintDialog );
    pDocShell->PreparePrint( pPrintDialog, &GetViewData()->GetMarkData() );
}

ErrCode ScTabViewShell::DoPrint( SfxPrinter *pPrinter,
                                 PrintDialog *pPrintDialog, BOOL bSilent )
{
    //  #72527# if SID_PRINTDOCDIRECT is executed and there's a selection,
    //  ask if only the selection should be printed

    const ScMarkData& rMarkData = GetViewData()->GetMarkData();
    if ( !pPrintDialog && !bSilent && ( rMarkData.IsMarked() || rMarkData.IsMultiMarked() ) )
    {
        SvxPrtQryBox aQuery( GetDialogParent() );
        short nBtn = aQuery.Execute();

        if ( nBtn == RET_CANCEL )
            return ERRCODE_IO_ABORT;

        if ( nBtn == RET_OK )
            bPrintSelected = TRUE;
    }

    //  SfxViewShell::DoPrint calls Print (after StartJob etc.)
    ErrCode nRet = SfxViewShell::DoPrint( pPrinter, pPrintDialog, bSilent );

    bPrintSelected = FALSE;

    return nRet;
}

USHORT __EXPORT ScTabViewShell::Print( SfxProgress& rProgress,
                                       PrintDialog* pPrintDialog )
{
    ScDocShell* pDocShell = GetViewData()->GetDocShell();
    pDocShell->GetDocument()->SetPrintOptions();    // Optionen aus OFA am Printer setzen

    SfxViewShell::Print( rProgress, pPrintDialog );
    pDocShell->Print( rProgress, pPrintDialog, &GetViewData()->GetMarkData(), bPrintSelected );

    return 0;
}

void ScTabViewShell::StopEditShell()
{
    if ( pEditShell != NULL && !bDontSwitch )
        SetEditShell(NULL, FALSE );
}

//------------------------------------------------------------------

BOOL ScTabViewShell::TabKeyInput(const KeyEvent& rKEvt)
{
    ScModule* pScMod = SC_MOD();

    SfxViewFrame* pThisFrame = GetViewFrame();
    if ( pThisFrame->GetChildWindow( SID_OPENDLG_FUNCTION ) )
        return FALSE;

    KeyCode aCode   = rKEvt.GetKeyCode();
    BOOL bShift     = aCode.IsShift();
    BOOL bControl   = aCode.IsMod1();
    BOOL bAlt       = aCode.IsMod2();
    USHORT nCode    = aCode.GetCode();
    BOOL bUsed      = FALSE;
    BOOL bInPlace   = pScMod->IsEditMode();     // Editengine bekommt alles
    BOOL bAnyEdit   = pScMod->IsInputMode();    // nur Zeichen & Backspace
    BOOL bDraw      = IsDrawTextEdit();

    HideNoteMarker();   // Notiz-Anzeige

    BOOL bHideCursor = ( nCode == KEY_RETURN && bInPlace ) ||
                            nCode == KEY_TAB;

    if (bHideCursor)
        HideAllCursors();

    ScDocument* pDoc = GetViewData()->GetDocument();
    if ( pDoc )
        pDoc->KeyInput( rKEvt );    // TimerDelays etc.

    if( bInPlace )
    {
        bUsed = pScMod->InputKeyEvent( rKEvt );         // Eingabe
        if( !bUsed )
            bUsed = SfxViewShell::KeyInput( rKEvt );    // Acceleratoren
    }
    else if( bAnyEdit )
    {
        BOOL bIsType = FALSE;
        USHORT nModi = aCode.GetModifier();
        USHORT nGroup = aCode.GetGroup();

        if ( nGroup == KEYGROUP_NUM || nGroup == KEYGROUP_ALPHA || nGroup == 0 )
            if ( !bControl && !bAlt )
                bIsType = TRUE;

        if ( nGroup == KEYGROUP_MISC )
            switch ( nCode )
            {
                case KEY_RETURN:
                    bIsType = bControl && !bAlt;        // Control, Shift-Control-Return
                    if ( !bIsType && nModi == 0 )
                    {
                        //  Will der InputHandler auch ein einfaches Return?

                        ScInputHandler* pHdl = pScMod->GetInputHdl(this);
                        bIsType = pHdl && pHdl->TakesReturn();
                    }
                    break;
                case KEY_SPACE:
                    bIsType = !bControl && !bAlt;       // ohne Modifier oder Shift-Space
                    break;
                case KEY_ESCAPE:
                case KEY_BACKSPACE:
                    bIsType = (nModi == 0); // nur ohne Modifier
                    break;
                default:
                    bIsType = TRUE;
            }

        if( bIsType )
            bUsed = pScMod->InputKeyEvent( rKEvt );     // Eingabe

        if( !bUsed )
            bUsed = SfxViewShell::KeyInput( rKEvt );    // Acceleratoren

        if ( !bUsed && !bIsType && nCode != KEY_RETURN )    // Eingabe nochmal hinterher
            bUsed = pScMod->InputKeyEvent( rKEvt );
    }
    else
    {
        //  #51889# Spezialfall: Copy/Cut bei Mehrfachselektion -> Fehlermeldung
        //  (Slot ist disabled, SfxViewShell::KeyInput wuerde also kommentarlos verschluckt)
        KeyFuncType eFunc = aCode.GetFunction();
        if ( eFunc == KEYFUNC_COPY || eFunc == KEYFUNC_CUT )
        {
            ScRange aDummy;
            if ( !GetViewData()->GetSimpleArea( aDummy, TRUE ) )    //! macht TRUE aerger ?
            {
                ErrorMessage(STR_NOMULTISELECT);
                bUsed = TRUE;
            }
        }
        if (!bUsed)
            bUsed = SfxViewShell::KeyInput( rKEvt );            // Acceleratoren

        //  #74696# during inplace editing, some slots are handled by the
        //  container app and are executed during Window::KeyInput.
        //  -> don't pass keys to input handler that would be used there
        //  but should call slots instead.
        BOOL bParent = ( GetViewFrame()->ISA(SfxInPlaceFrame) && eFunc != KEYFUNC_DONTKNOW );

        if( !bUsed && !bDraw && nCode != KEY_RETURN && !bParent )
            bUsed = pScMod->InputKeyEvent( rKEvt, TRUE );       // Eingabe
    }

    if (!bInPlace && !bUsed && !bDraw)
    {
        switch (nCode)
        {
            case KEY_RETURN:
                {
                    BOOL bNormal = !bControl && !bAlt;
                    if ( !bAnyEdit && bNormal )
                    {
                        //  je nach Optionen mit Enter in den Edit-Modus schalten

                        const ScInputOptions& rOpt = pScMod->GetInputOptions();
                        if ( rOpt.GetEnterEdit() )
                        {
                            pScMod->SetInputMode( SC_INPUT_TABLE );
                            bUsed = TRUE;
                        }
                    }

                    BOOL bEditReturn = bControl && !bShift;         // An Edit-Engine weiter
                    if ( !bUsed && !bEditReturn )
                    {
                        HideAllCursors();

                        BYTE nMode = SC_ENTER_NORMAL;
                        if ( bShift && bControl )
                            nMode = SC_ENTER_MATRIX;
                        else if ( bAlt )
                            nMode = SC_ENTER_BLOCK;
                        pScMod->InputEnterHandler(nMode);

                        if (nMode == SC_ENTER_NORMAL)
                        {
                            if( bShift )
                                GetViewData()->GetDispatcher().Execute( SID_CURSORENTERUP,
                                            SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD );
                            else
                                GetViewData()->GetDispatcher().Execute( SID_CURSORENTERDOWN,
                                            SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD );
                        }
                        else
                            UpdateInputHandler(TRUE);

                        ShowAllCursors();

                        //  hier kein UpdateInputHandler, weil bei Referenzeingabe auf ein
                        //  anderes Dokument diese ViewShell nicht die ist, auf der eingegeben
                        //  wird!

                        bUsed = TRUE;
                    }
                }
                break;
        }
    }

    //  Alt-Cursortasten hart codiert, weil Alt nicht konfigurierbar ist

    if ( !bUsed && bAlt && !bControl )
    {
        USHORT nSlotId = 0;
        switch (nCode)
        {
            case KEY_UP:
                ModifyCellSize( DIR_TOP, bShift );
                bUsed = TRUE;
                break;
            case KEY_DOWN:
                ModifyCellSize( DIR_BOTTOM, bShift );
                bUsed = TRUE;
                break;
            case KEY_LEFT:
                ModifyCellSize( DIR_LEFT, bShift );
                bUsed = TRUE;
                break;
            case KEY_RIGHT:
                ModifyCellSize( DIR_RIGHT, bShift );
                bUsed = TRUE;
                break;
            case KEY_PAGEUP:
                nSlotId = bShift ? SID_CURSORPAGELEFT_SEL : SID_CURSORPAGELEFT_;
                break;
            case KEY_PAGEDOWN:
                nSlotId = bShift ? SID_CURSORPAGERIGHT_SEL : SID_CURSORPAGERIGHT_;
                break;
        }
        if ( nSlotId )
        {
            GetViewData()->GetDispatcher().Execute( nSlotId, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD );
            bUsed = TRUE;
        }
    }

    if (bHideCursor)
        ShowAllCursors();

    return bUsed;
}

BOOL ScTabViewShell::SfxKeyInput(const KeyEvent& rKeyEvent)
{
    return SfxViewShell::KeyInput( rKeyEvent );
}

FASTBOOL __EXPORT ScTabViewShell::KeyInput( const KeyEvent &rKeyEvent )
{
//  return SfxViewShell::KeyInput( rKeyEvent );
    return TabKeyInput( rKeyEvent );
}

//------------------------------------------------------------------

//  SfxViewShell( pViewFrame, SFX_VIEW_MAXIMIZE_FIRST | SFX_VIEW_DISABLE_ACCELS ),

#define __INIT_ScTabViewShell \
    SfxViewShell( pViewFrame, SFX_VIEW_MAXIMIZE_FIRST | SFX_VIEW_CAN_PRINT ), \
    nDrawSfxId(0),              \
    nCtrlSfxId(USHRT_MAX),      \
    eCurOST(OST_NONE),          \
    nFormSfxId(USHRT_MAX),      \
    pCellShell(NULL),           \
    pPageBreakShell(NULL),      \
    pDrawShell(NULL),           \
    pDrawFormShell(NULL),       \
    pOleObjectShell(NULL),      \
    pChartShell(NULL),          \
    pGraphicShell(NULL),        \
    pDrawTextShell(NULL),       \
    pEditShell(NULL),           \
    pPivotShell(NULL),          \
    pAuditingShell(NULL),       \
    pFormShell(NULL),           \
    pInputHandler(NULL),        \
    pCurFrameLine(NULL),        \
    bActiveChartSh(FALSE),      \
    bActiveDrawSh(FALSE),       \
    bActiveDrawTextSh(FALSE),   \
    bActiveGraphicSh(FALSE),    \
    bActivePivotSh(FALSE),      \
    bActiveAuditingSh(FALSE),   \
    bActiveDrawFormSh(FALSE),   \
    bActiveOleObjectSh(FALSE),  \
    bDontSwitch(FALSE),         \
    bInFormatDialog(FALSE),     \
    bPrintSelected(FALSE),      \
    bReadOnly(FALSE),           \
    pScSbxObject(NULL),         \
    bChartAreaValid(FALSE),     \
    bChartDlgIsEdit(FALSE),     \
    pDialogDPObject(NULL),      \
    aTarget( this )


//------------------------------------------------------------------

void ScTabViewShell::Construct()
{
    SfxApplication* pSfxApp  = SFX_APP();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();

    bReadOnly = pDocSh->IsReadOnly();

    SetName( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("View")) ); // fuer SBX
    Color aColBlack( COL_BLACK );
//  SetPool( &pSfxApp->GetPool() );
    SetPool( &SC_MOD()->GetPool() );
    SetWindow( GetActiveWin() );

    pCurFrameLine   = new SvxBorderLine( &aColBlack, 20, 0, 0 );
    pPivotSource    = new ScArea;
    StartListening(*GetViewData()->GetDocShell());
    StartListening(*GetViewFrame());
    StartListening(*pSfxApp,TRUE);      // SfxViewShell hoert da schon zu?

    SfxViewFrame* pFirst = SfxViewFrame::GetFirst(pDocSh);
    BOOL bFirstView = !pFirst
          || (pFirst == GetViewFrame() && !SfxViewFrame::GetNext(*pFirst,pDocSh));

    if ( pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
    {
        Rectangle aVisArea = ((SfxInPlaceObject*)pDocSh)->GetVisArea();

        USHORT nVisTab = pDoc->GetVisibleTab();
        if (!pDoc->HasTable(nVisTab))
        {
            nVisTab = 0;
            pDoc->SetVisibleTab(nVisTab);
        }
        SetTabNo( nVisTab );
        GetViewData()->SetScreenPos( aVisArea.TopLeft() );      // richtige Stelle zeigen

        if ( GetViewFrame()->ISA(SfxInPlaceFrame) )             // inplace
        {
            pDocSh->SetInplace( TRUE );             // schon so initialisiert
            if (pDoc->IsEmbedded())
                pDoc->ResetEmbedded();              // keine blaue Markierung
        }
        else if ( bFirstView )
        {
            pDocSh->SetInplace( FALSE );
            GetViewData()->SetZoom(                             // PPT neu berechnen
                            GetViewData()->GetZoomX(),
                            GetViewData()->GetZoomY());
            if (!pDoc->IsEmbedded())
                pDoc->SetEmbedded( aVisArea );                  // VisArea markieren
        }
    }

    // ViewInputHandler
    //  #48721# jeder Task hat neuerdings sein eigenes InputWindow,
    //  darum muesste eigentlich entweder jeder Task seinen InputHandler bekommen,
    //  oder das InputWindow muesste sich beim App-InputHandler anmelden, wenn der
    //  Task aktiv wird, oder das InputWindow muesste sich den InputHandler selbst
    //  anlegen (dann immer ueber das InputWindow suchen, und nur wenn das nicht da
    //  ist, den InputHandler von der App nehmen).
    //  Als Sofortloesung bekommt erstmal jede View ihren Inputhandler, das gibt
    //  nur noch Probleme, wenn zwei Views in einem Task-Fenster sind.

    pInputHandler = new ScInputHandler;

    // Alte Version:
    //  if ( !GetViewFrame()->ISA(SfxTopViewFrame) )        // OLE oder Plug-In
    //      pInputHandler = new ScInputHandler;

            //  FormShell vor MakeDrawView anlegen, damit die DrawView auf jeden Fall
            //  an der FormShell angemeldet werden kann
            //  Gepusht wird die FormShell im ersten Activate
    pFormShell = new FmFormShell(this);

            //  DrawView darf nicht im TabView - ctor angelegt werden,
            //  wenn die ViewShell noch nicht kostruiert ist...
    if (pDoc->GetDrawLayer())
        MakeDrawView();
    ViewOptionsHasChanged(FALSE);   // legt auch evtl. DrawView an

    SetUndoManager( pDocSh->GetUndoManager() );
    pFormShell->SetUndoManager( pDocSh->GetUndoManager() );
    SetRepeatTarget( &aTarget );
    pFormShell->SetRepeatTarget( &aTarget );
    SetHelpId( HID_SCSHELL_TABVWSH );

    if ( bFirstView )
    {   // erste View?

        //  Tabellen anhaengen? (nicht bei OLE)
        if ( pDocSh->IsEmpty() &&
                pDocSh->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
        {
            USHORT nInitTabCount = 3;                           //! konfigurierbar !!!
            for (USHORT i=1; i<nInitTabCount; i++)
                pDoc->MakeTable(i);
        }

        const ScExtDocOptions* pExtOpt = pDoc->GetExtDocOptions();
        if (pExtOpt)
        {
            GetViewData()->ReadExtOptions(*pExtOpt);        //  Excel-View Optionen
            SetTabNo( GetViewData()->GetTabNo(), TRUE );
            //! alles von ReadUserData auch hier
        }

        //  Link-Update nicht verschachteln
        if ( pDocSh->GetCreateMode() != SFX_CREATE_MODE_INTERNAL )
        {
            BOOL bLink = FALSE;                                 // Links updaten
            USHORT nTabCount = pDoc->GetTableCount();
            for (USHORT i=0; i<nTabCount && !bLink; i++)
                if (pDoc->IsLinked(i))
                    bLink = TRUE;
            if (!bLink)
                if (pDoc->HasDdeLinks() || pDoc->HasAreaLinks())
                    bLink = TRUE;
            if (bLink)
            {
                if ( !pFirst )
                    pFirst = GetViewFrame();

                if(SC_MOD()->GetCurRefDlgId()==0)
                {
                        pFirst->GetDispatcher()->Execute( SID_UPDATETABLINKS,
                                            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
                }
            }

            BOOL bReImport = FALSE;                             // importierte Daten aktualisieren
            ScDBCollection* pDBColl = pDoc->GetDBCollection();
            if ( pDBColl )
            {
                USHORT nCount = pDBColl->GetCount();
                for (USHORT i=0; i<nCount && !bReImport; i++)
                {
                    ScDBData* pData = (*pDBColl)[i];
                    if ( pData->IsStripData() &&
                            pData->HasImportParam() && !pData->HasImportSelection() )
                        bReImport = TRUE;
                }
            }
            if (bReImport)
            {
                if ( !pFirst )
                    pFirst = GetViewFrame();
                if(SC_MOD()->GetCurRefDlgId()==0)
                {
                    pFirst->GetDispatcher()->Execute( SID_REIMPORT_AFTER_LOAD,
                                            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
                }
            }
        }
    }

    UpdateAutoFillMark();

    bFirstActivate = TRUE; // NavigatorUpdate aufschieben bis Activate()
}

//------------------------------------------------------------------

ScTabViewShell::ScTabViewShell( SfxViewFrame* pViewFrame,
                                const ScTabViewShell& rWin ) :
    __INIT_ScTabViewShell,
    ScDBFunc( &pViewFrame->GetWindow(), rWin, this )
{
    Construct();

    UpdatePageBreakData();

    uno::Reference<frame::XFrame> xFrame = pViewFrame->GetFrame()->GetFrameInterface();
    if (xFrame.is())
        xFrame->setComponent( uno::Reference<awt::XWindow>(), new ScTabViewObj( this ) );

    SetCurSubShell(OST_Cell);
}

//------------------------------------------------------------------

ScTabViewShell::ScTabViewShell( SfxViewFrame* pViewFrame,
                                SfxViewShell* pOldSh ) :
    __INIT_ScTabViewShell,
    ScDBFunc( &pViewFrame->GetWindow(), (ScDocShell&)*pViewFrame->GetObjectShell(), this )
{
    const ScAppOptions& rAppOpt = SC_MOD()->GetAppOptions();

    Construct();

    Fraction aFract( rAppOpt.GetZoom(), 100 );
    SetZoom( aFract, aFract );
    SetZoomType( rAppOpt.GetZoomType() );

    uno::Reference<frame::XFrame> xFrame = pViewFrame->GetFrame()->GetFrameInterface();
    if (xFrame.is())
        xFrame->setComponent( uno::Reference<awt::XWindow>(), new ScTabViewObj( this ) );

    SetCurSubShell(OST_Cell);

    //  if switching back from print preview,
    //  restore the view settings that were active when creating the preview

    if ( pOldSh && pOldSh->ISA( ScPreviewShell ) )
    {
        String aOldData = ((ScPreviewShell*)pOldSh)->GetSourceData();
        if ( aOldData.Len() )
        {
            //  restore old view settings
            //  (DoReadUserData also sets SubShell, current Window etc.)
            DoReadUserData( aOldData );
        }
    }
}

#undef __INIT_ScTabViewShell

//------------------------------------------------------------------

__EXPORT ScTabViewShell::~ScTabViewShell()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    EndListening(*pDocSh);
    EndListening(*GetViewFrame());

    SC_MOD()->ViewShellGone(this);

    RemoveSubShell();           // alle
    SetWindow(0);

    //  #54104# alles auf NULL, falls aus dem TabView-dtor noch darauf zugegriffen wird
    //! (soll eigentlich nicht !??!?!)

    DELETEZ(pCellShell);
    DELETEZ(pPageBreakShell);
    DELETEZ(pDrawShell);
    DELETEZ(pDrawFormShell);
    DELETEZ(pOleObjectShell);
    DELETEZ(pChartShell);
    DELETEZ(pGraphicShell);
    DELETEZ(pDrawTextShell);
    DELETEZ(pEditShell);
    DELETEZ(pPivotShell);
    DELETEZ(pAuditingShell);
    DELETEZ(pCurFrameLine);
    DELETEZ(pInputHandler);
    DELETEZ(pPivotSource);
    DELETEZ(pDialogDPObject);

    DELETEZ(pFormShell);
}

//------------------------------------------------------------------

void ScTabViewShell::SetDialogDPObject( const ScDPObject* pObj )
{
    delete pDialogDPObject;
    if (pObj)
        pDialogDPObject = new ScDPObject( *pObj );
    else
        pDialogDPObject = NULL;
}

//------------------------------------------------------------------

void ScTabViewShell::FillFieldData( ScHeaderFieldData& rData )
{
    ScDocShell* pDocShell = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocShell->GetDocument();
    USHORT nTab = GetViewData()->GetTabNo();
    pDoc->GetName( nTab, rData.aTabName );

    rData.aTitle        = pDocShell->GetTitle();
    rData.aLongDocName  = pDocShell->GetMedium()->GetName();
    if ( !rData.aLongDocName.Len() )
        rData.aLongDocName = rData.aTitle;
    rData.aShortDocName = INetURLObject( rData.aLongDocName ).GetName();
    rData.nPageNo       = 1;
    rData.nTotalPages   = 99;

    //  eNumType kennt der Dialog selber
}

//------------------------------------------------------------------

void ScTabViewShell::SetChartArea( const ScRangeListRef& rSource, const Rectangle& rDest )
{
    bChartAreaValid = TRUE;
    aChartSource    = rSource;
    aChartPos       = rDest;
    nChartDestTab   = GetViewData()->GetTabNo();
}

void ScTabViewShell::ResetChartArea()
{
    bChartAreaValid = FALSE;
}

BOOL ScTabViewShell::GetChartArea( ScRangeListRef& rSource, Rectangle& rDest, USHORT& rTab ) const
{
    rSource = aChartSource;
    rDest   = aChartPos;
    rTab    = nChartDestTab;
    return bChartAreaValid;
}

BOOL ScTabViewShell::IsChartDlgEdit() const
{
    return bChartDlgIsEdit;
}

const String& ScTabViewShell::GetEditChartName() const
{
    return aEditChartName;
}

//------------------------------------------------------------------

void ScTabViewShell::ExecTbx( SfxRequest& rReq )
{
    const SfxItemSet* pReqArgs = rReq.GetArgs();
    USHORT nSlot = rReq.GetSlot();
    const SfxPoolItem* pItem = NULL;
    if ( pReqArgs )
        pReqArgs->GetItemState( nSlot, TRUE, &pItem );

    switch ( nSlot )
    {
        case SID_TBXCTL_INSERT:
            if ( pItem )
                nInsertCtrlState = ((const SfxUInt16Item*)pItem)->GetValue();
            break;
        case SID_TBXCTL_INSCELLS:
            if ( pItem )
                nInsCellsCtrlState = ((const SfxUInt16Item*)pItem)->GetValue();
            break;
        case SID_TBXCTL_INSOBJ:
            if ( pItem )
                nInsObjCtrlState = ((const SfxUInt16Item*)pItem)->GetValue();
            break;
        default:
            DBG_ERROR("Slot im Wald");
    }
    GetViewFrame()->GetBindings().Invalidate( nSlot );
}

void ScTabViewShell::GetTbxState( SfxItemSet& rSet )
{
    rSet.Put( SfxUInt16Item( SID_TBXCTL_INSERT,   nInsertCtrlState ) );
    rSet.Put( SfxUInt16Item( SID_TBXCTL_INSCELLS, nInsCellsCtrlState ) );

    //  ohne installiertes Chart darf Chart nicht Default sein...
    if ( nInsObjCtrlState == SID_DRAW_CHART && !SFX_APP()->HasFeature(SFX_FEATURE_SCHART) )
        nInsObjCtrlState = SID_INSERT_OBJECT;

    rSet.Put( SfxUInt16Item( SID_TBXCTL_INSOBJ,   nInsObjCtrlState ) );
}





