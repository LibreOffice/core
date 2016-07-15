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

#include "olkact.hxx"
#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <sfx2/app.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <editeng/boxitem.hxx>
#include <svx/fmpage.hxx>
#include <svx/fmshell.hxx>
#include <editeng/sizeitem.hxx>
#include <svx/prtqry.hxx>
#include <svx/sidebar/ContextChangeEventMultiplexer.hxx>
#include <sfx2/request.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/whiter.hxx>
#include <unotools/moduleoptions.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/docfile.hxx>
#include <vcl/openglwin.hxx>

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
#include "dbdata.hxx"
#include "reffact.hxx"
#include "viewuno.hxx"
#include "dispuno.hxx"
#include "anyrefdg.hxx"
#include "chgtrack.hxx"
#include "cellsh.hxx"
#include "oleobjsh.hxx"
#include "chartsh.hxx"
#include "graphsh.hxx"
#include "mediash.hxx"
#include "pgbrksh.hxx"
#include "dpobject.hxx"
#include "prevwsh.hxx"
#include "tpprint.hxx"
#include "scextopt.hxx"
#include "printopt.hxx"
#include "drawview.hxx"
#include "fupoor.hxx"
#include "navsett.hxx"
#include "scabstdlg.hxx"
#include "externalrefmgr.hxx"
#include "defaultsoptions.hxx"
#include "markdata.hxx"
#include "preview.hxx"
#include "docoptio.hxx"
#include <documentlinkmgr.hxx>
#include <gridwin.hxx>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/chart2/X3DChartWindowProvider.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XChartType.hpp>
#include <sfx2/lokhelper.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

extern SfxViewShell* pScActiveViewShell;            // global.cxx

using namespace com::sun::star;

void ScTabViewShell::Activate(bool bMDI)
{
    SfxViewShell::Activate(bMDI);
    bIsActive = true;
    // here no GrabFocus, otherwise there will be problems when something is edited inplace!

    if ( bMDI )
    {
        // for input row (ClearCache)
        ScModule* pScMod = SC_MOD();
        pScMod->ViewShellChanged();

        ActivateView( true, bFirstActivate );

        // update AutoCorrect, if Writer has newly created this
        UpdateDrawTextOutliner();

        // RegisterNewTargetNames does not exist anymore

        SfxViewFrame* pThisFrame  = GetViewFrame();
        if ( pInputHandler && pThisFrame->HasChildWindow(FID_INPUTLINE_STATUS) )
        {
            // actually only required for Reload (last version):
            // The InputWindow remains, but the View along with the InputHandler is newly created,
            // that is why the InputHandler must be set at the InputWindow.
            SfxChildWindow* pChild = pThisFrame->GetChildWindow(FID_INPUTLINE_STATUS);
            if (pChild)
            {
                ScInputWindow* pWin = static_cast<ScInputWindow*>(pChild->GetWindow());
                if (pWin && pWin->IsVisible())
                {

                    ScInputHandler* pOldHdl=pWin->GetInputHandler();

                    SfxViewShell* pSh = SfxViewShell::GetFirst( true, checkSfxViewShell<ScTabViewShell> );
                    while ( pSh!=nullptr && pOldHdl!=nullptr)
                    {
                        if (static_cast<ScTabViewShell*>(pSh)->GetInputHandler() == pOldHdl)
                        {
                            pOldHdl->ResetDelayTimer();
                            break;
                        }
                        pSh = SfxViewShell::GetNext( *pSh, true, checkSfxViewShell<ScTabViewShell> );
                    }

                    pWin->SetInputHandler( pInputHandler );
                }
            }
        }

        UpdateInputHandler( true );

        if ( bFirstActivate )
        {
            SfxGetpApp()->Broadcast( SfxSimpleHint( SC_HINT_NAVIGATOR_UPDATEALL ) );
            bFirstActivate = false;

            // ReadExtOptions (view settings from Excel import) must also be done
            // after the ctor, because of the potential calls to Window::Show.
            // Even after a bugfix (Window::Show no longer notifies the access
            // bridge, it's done in ImplSetReallyVisible), there are problems if Window::Show
            // is called during the ViewShell ctor and reschedules asynchronous calls
            // (for example from the FmFormShell ctor).
            ScExtDocOptions* pExtOpt = GetViewData().GetDocument()->GetExtDocOptions();
            if ( pExtOpt && pExtOpt->IsChanged() )
            {
                GetViewData().ReadExtOptions(*pExtOpt);        // Excel view settings
                SetTabNo( GetViewData().GetTabNo(), true );
                pExtOpt->SetChanged( false );
            }
        }

        pScActiveViewShell = this;

        ScInputHandler* pHdl = pScMod->GetInputHdl(this);
        if (pHdl)
        {
            pHdl->SetRefScale( GetViewData().GetZoomX(), GetViewData().GetZoomY() );
        }

        // update change dialog

        if ( pThisFrame->HasChildWindow(FID_CHG_ACCEPT) )
        {
            SfxChildWindow* pChild = pThisFrame->GetChildWindow(FID_CHG_ACCEPT);
            if (pChild)
            {
                static_cast<ScAcceptChgDlgWrapper*>(pChild)->ReInitDlg();
            }
        }

        if(pScMod->IsRefDialogOpen())
        {
            sal_uInt16 nModRefDlgId=pScMod->GetCurRefDlgId();
            SfxChildWindow* pChildWnd = pThisFrame->GetChildWindow( nModRefDlgId );
            if ( pChildWnd )
            {
                IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
                assert(pRefDlg);
                if(pRefDlg)
                {
                    pRefDlg->ViewShellChanged();
                }
            }
        }
    }

    //  don't call CheckSelectionTransfer here - activating a view should not change the
    //  primary selection (may be happening just because the mouse was moved over the window)

    ContextChangeEventMultiplexer::NotifyContextChange(
        GetController(),
        ::sfx2::sidebar::EnumContext::Context_Default);
}

void ScTabViewShell::Deactivate(bool bMDI)
{
    HideTip();

    ScDocument* pDoc=GetViewData().GetDocument();

    ScChangeTrack* pChanges=pDoc->GetChangeTrack();

    if(pChanges!=nullptr)
    {
        Link<ScChangeTrack&,void> aLink;
        pChanges->SetModifiedLink(aLink);
    }

    SfxViewShell::Deactivate(bMDI);
    bIsActive = false;
    ScInputHandler* pHdl = SC_MOD()->GetInputHdl(this);

    if( bMDI )
    {
        //  during shell deactivation, shells must not be switched, or the loop
        //  through the shell stack (in SfxDispatcher::DoDeactivate_Impl) will not work
        bool bOldDontSwitch = bDontSwitch;
        bDontSwitch = true;

        ActivateView( false, false );

        if ( GetViewFrame()->GetFrame().IsInPlace() ) // inplace
            GetViewData().GetDocShell()->UpdateOle(&GetViewData(), true);

        if ( pHdl )
            pHdl->NotifyChange( nullptr, true ); // timer-delayed due to document switching

        if (pScActiveViewShell == this)
            pScActiveViewShell = nullptr;

        bDontSwitch = bOldDontSwitch;
    }
    else
    {
        HideNoteMarker();           // note marker

        if ( pHdl )
            pHdl->HideTip();        // Hide formula auto input tip
    }
}

void ScTabViewShell::SetActive()
{
    // SFX-View would like to activate itself, since then magical things would happen
    // (eg else the designer may crash)
    ActiveGrabFocus();
}

bool ScTabViewShell::PrepareClose(bool bUI)
{
    // Call EnterHandler even in formula mode here,
    // so a formula change in an embedded object isn't lost
    // (ScDocShell::PrepareClose isn't called then).
    ScInputHandler* pHdl = SC_MOD()->GetInputHdl( this );
    if ( pHdl && pHdl->IsInputMode() )
        pHdl->EnterHandler();

    // draw text edit mode must be closed
    FuPoor* pPoor = GetDrawFuncPtr();
    if ( pPoor && ( IsDrawTextShell() || pPoor->GetSlotID() == SID_DRAW_NOTEEDIT ) )
    {
        // "clean" end of text edit, including note handling, subshells and draw func switching,
        // as in FuDraw and ScTabView::DrawDeselectAll
        GetViewData().GetDispatcher().Execute( pPoor->GetSlotID(), SfxCallMode::SLOT | SfxCallMode::RECORD );
    }
    ScDrawView* pDrView = GetScDrawView();
    if ( pDrView )
    {
        // force end of text edit, to be safe
        // ScEndTextEdit must always be used, to ensure correct UndoManager
        pDrView->ScEndTextEdit();
    }

    if ( pFormShell )
    {
        bool bRet = pFormShell->PrepareClose(bUI);
        if (!bRet)
            return bRet;
    }
    return SfxViewShell::PrepareClose(bUI);
}

// calculate zoom for in-place
// from the ratio of VisArea and window size of GridWin

void ScTabViewShell::UpdateOleZoom()
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    if ( pDocSh->GetCreateMode() == SfxObjectCreateMode::EMBEDDED )
    {
        //TODO/LATER: is there a difference between the two GetVisArea methods?
        Size aObjSize = static_cast<const SfxObjectShell*>(pDocSh)->GetVisArea().GetSize();
        if ( aObjSize.Width() > 0 && aObjSize.Height() > 0 )
        {
            vcl::Window* pWin = GetActiveWin();
            Size aWinHMM = pWin->PixelToLogic( pWin->GetOutputSizePixel(), MAP_100TH_MM );
            SetZoomFactor( Fraction( aWinHMM.Width(),aObjSize.Width() ),
                            Fraction( aWinHMM.Height(),aObjSize.Height() ) );
        }
    }
}

void ScTabViewShell::AdjustPosSizePixel( const Point &rPos, const Size &rSize )
{
    OuterResizePixel( rPos, rSize );
}

void ScTabViewShell::InnerResizePixel( const Point &rOfs, const Size &rSize )
{
    Size aNewSize( rSize );
    if ( GetViewFrame()->GetFrame().IsInPlace() )
    {
        SvBorder aBorder;
           GetBorderSize( aBorder, rSize );
        SetBorderPixel( aBorder );

        Size aObjSize = GetObjectShell()->GetVisArea().GetSize();

          Size aSize( rSize );
        aSize.Width() -= (aBorder.Left() + aBorder.Right());
        aSize.Height() -= (aBorder.Top() + aBorder.Bottom());

        if ( aObjSize.Width() > 0 && aObjSize.Height() > 0 )
        {
            Size aLogicSize = GetWindow()->PixelToLogic( aSize, MAP_100TH_MM );
            SfxViewShell::SetZoomFactor( Fraction( aLogicSize.Width(),aObjSize.Width() ),
                            Fraction( aLogicSize.Height(),aObjSize.Height() ) );
        }

        Point aPos( rOfs );
        aPos.X() += aBorder.Left();
        aPos.Y() += aBorder.Top();
        GetWindow()->SetPosSizePixel( aPos, aSize );
    }
    else
    {
        SvBorder aBorder;
        GetBorderSize( aBorder, rSize );
        SetBorderPixel( aBorder );
        aNewSize.Width()  += aBorder.Left() + aBorder.Right();
        aNewSize.Height() += aBorder.Top() + aBorder.Bottom();
    }

    DoResize( rOfs, aNewSize, true );                   // rSize = size of gridwin

    UpdateOleZoom();                                    // calculate zoom for in-place

    GetViewData().GetDocShell()->SetDocumentModified();
}

void ScTabViewShell::OuterResizePixel( const Point &rOfs, const Size &rSize )
{
    SvBorder aBorder;
    GetBorderSize( aBorder, rSize );
    SetBorderPixel( aBorder );

    DoResize( rOfs, rSize );                    // position and size of tabview as passed

    // ForceMove as replacement for Sfx-Move mechanism
    // (aWinPos must be kept current, so that ForceMove works for Ole deactivation)

    ForceMove();
}

void ScTabViewShell::SetZoomFactor( const Fraction &rZoomX, const Fraction &rZoomY )
{
    // for OLE...

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

    GetViewData().UpdateScreenZoom( aNewX, aNewY );
    SetZoom( aNewX, aNewY, true );

    PaintGrid();
    PaintTop();
    PaintLeft();

    SfxViewShell::SetZoomFactor( rZoomX, rZoomY );
}

void ScTabViewShell::QueryObjAreaPixel( Rectangle& rRect ) const
{
    // adjust to entire cells (in 1/100 mm)

    Size aPixelSize = rRect.GetSize();
    vcl::Window* pWin = const_cast<ScTabViewShell*>(this)->GetActiveWin();
    Size aLogicSize = pWin->PixelToLogic( aPixelSize );

    const ScViewData& rViewData = GetViewData();
    ScDocument* pDoc = rViewData.GetDocument();
    ScSplitPos ePos = rViewData.GetActivePart();
    SCCOL nCol = rViewData.GetPosX(WhichH(ePos));
    SCROW nRow = rViewData.GetPosY(WhichV(ePos));
    SCTAB nTab = rViewData.GetTabNo();
    bool bNegativePage = pDoc->IsNegativePage( nTab );

    Rectangle aLogicRect = pDoc->GetMMRect( nCol, nRow, nCol, nRow, nTab );
    if ( bNegativePage )
    {
        // use right edge of aLogicRect, and aLogicSize
        aLogicRect.Left() = aLogicRect.Right() - aLogicSize.Width() + 1;    // Right() is set below
    }
    aLogicRect.SetSize( aLogicSize );

    rViewData.GetDocShell()->SnapVisArea( aLogicRect );

    rRect.SetSize( pWin->LogicToPixel( aLogicRect.GetSize() ) );
}

void ScTabViewShell::Move()
{
    Point aNewPos = GetViewFrame()->GetWindow().OutputToScreenPixel(Point());

    if (aNewPos != aWinPos)
    {
        StopMarking();
        aWinPos = aNewPos;
    }
}

void ScTabViewShell::ShowCursor(bool /* bOn */)
{
/*!!!   ShowCursor is not called as a pair as in gridwin.
        here the CursorLockCount for Gridwin must be set directly to 0

    if (bOn)
        ShowAllCursors();
    else
        HideAllCursors();
*/
}

void ScTabViewShell::WriteUserData(OUString& rData, bool /* bBrowse */)
{
    GetViewData().WriteUserData(rData);
}

void ScTabViewShell::WriteUserDataSequence (uno::Sequence < beans::PropertyValue >& rSettings, bool /* bBrowse */ )
{
    GetViewData().WriteUserDataSequence (rSettings);
}

void ScTabViewShell::ReadUserData(const OUString& rData, bool /* bBrowse */)
{
    if ( !GetViewData().GetDocShell()->IsPreview() )
        DoReadUserData( rData );
}

void ScTabViewShell::ReadUserDataSequence (const uno::Sequence < beans::PropertyValue >& rSettings, bool /* bBrowse */ )
{
    if ( !GetViewData().GetDocShell()->IsPreview() )
        DoReadUserDataSequence( rSettings );
}

void ScTabViewShell::DoReadUserDataSequence( const uno::Sequence < beans::PropertyValue >& rSettings )
{
    vcl::Window* pOldWin = GetActiveWin();
    bool bFocus = pOldWin && pOldWin->HasFocus();

    GetViewData().ReadUserDataSequence(rSettings);
    SetTabNo( GetViewData().GetTabNo(), true );

    if ( GetViewData().IsPagebreakMode() )
        SetCurSubShell( GetCurObjectSelectionType(), true );

    vcl::Window* pNewWin = GetActiveWin();
    if (pNewWin && pNewWin != pOldWin)
    {
        SetWindow( pNewWin );       //! is this ViewShell always active???
        if (bFocus)
            pNewWin->GrabFocus();
        WindowChanged();            // drawing layer (for instance #56771#)
    }

    if (GetViewData().GetHSplitMode() == SC_SPLIT_FIX ||
        GetViewData().GetVSplitMode() == SC_SPLIT_FIX)
    {
        InvalidateSplit();
    }

    ZoomChanged();

    TestHintWindow();

    //! if ViewData has more tables than document, remove tables in ViewData
}

// DoReadUserData is also called from ctor when switching from print preview

void ScTabViewShell::DoReadUserData( const OUString& rData )
{
    vcl::Window* pOldWin = GetActiveWin();
    bool bFocus = pOldWin && pOldWin->HasFocus();

    GetViewData().ReadUserData(rData);
    SetTabNo( GetViewData().GetTabNo(), true );

    if ( GetViewData().IsPagebreakMode() )
        SetCurSubShell( GetCurObjectSelectionType(), true );

    vcl::Window* pNewWin = GetActiveWin();
    if (pNewWin && pNewWin != pOldWin)
    {
        SetWindow( pNewWin );       //! is this ViewShell always active???
        if (bFocus)
            pNewWin->GrabFocus();
        WindowChanged();            // drawing layer (for instance #56771#)
    }

    if (GetViewData().GetHSplitMode() == SC_SPLIT_FIX ||
        GetViewData().GetVSplitMode() == SC_SPLIT_FIX)
    {
        InvalidateSplit();
    }

    ZoomChanged();

    TestHintWindow();

    //! if ViewData has more tables than document, remove tables in ViewData
}

void ScTabViewShell::UpdateDrawShell()
{
    // Called after user interaction that may delete the selected drawing object.
    // Remove DrawShell if nothing is selected.

    SdrView* pDrView = GetSdrView();
    if ( pDrView && !pDrView->AreObjectsMarked() && !IsDrawSelMode() )
        SetDrawShell( false );
}

void ScTabViewShell::SetDrawShellOrSub()
{
    bActiveDrawSh = true;

    if(bActiveDrawFormSh)
    {
        SetCurSubShell(OST_DrawForm);
    }
    else if(bActiveGraphicSh)
    {
        SetCurSubShell(OST_Graphic);
    }
    else if(bActiveMediaSh)
    {
        SetCurSubShell(OST_Media);
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
        SetCurSubShell(OST_Drawing, true /* force: different toolbars are
                                            visible concerning shape type
                                            and shape state */);
    }
}

void ScTabViewShell::SetDrawShell( bool bActive )
{
    if(bActive)
    {
        SetCurSubShell(OST_Drawing, true /* force: different toolbars are
                                            visible concerning shape type
                                            and shape state */);
    }
    else
    {
        if(bActiveDrawFormSh || bActiveDrawSh ||
            bActiveGraphicSh || bActiveMediaSh || bActiveOleObjectSh||
            bActiveChartSh || bActiveDrawTextSh)
        {
            SetCurSubShell(OST_Cell);
        }
        bActiveDrawFormSh=false;
        bActiveGraphicSh=false;
        bActiveMediaSh=false;
        bActiveOleObjectSh=false;
        bActiveChartSh=false;
    }

    bool bWasDraw = bActiveDrawSh || bActiveDrawTextSh;

    bActiveDrawSh = bActive;
    bActiveDrawTextSh = false;

    if ( !bActive )
    {
        ResetDrawDragMode();        // switch off Mirror / Rotate

        if (bWasDraw && (GetViewData().GetHSplitMode() == SC_SPLIT_FIX ||
                         GetViewData().GetVSplitMode() == SC_SPLIT_FIX))
        {
            // adjust active part to cursor, etc.
            MoveCursorAbs( GetViewData().GetCurX(), GetViewData().GetCurY(),
                            SC_FOLLOW_NONE, false, false, true );
        }
    }
}

void ScTabViewShell::SetDrawTextShell( bool bActive )
{
    bActiveDrawTextSh = bActive;
    if ( bActive )
    {
        bActiveDrawFormSh=false;
        bActiveGraphicSh=false;
        bActiveMediaSh=false;
        bActiveOleObjectSh=false;
        bActiveChartSh=false;
        bActiveDrawSh = false;
        SetCurSubShell(OST_DrawText);
    }
    else
        SetCurSubShell(OST_Cell);

}

void ScTabViewShell::SetPivotShell( bool bActive )
{
    bActivePivotSh = bActive;

    //  SetPivotShell is called from CursorPosChanged every time
    //  -> don't change anything except switching between cell and pivot shell

    if ( eCurOST == OST_Pivot || eCurOST == OST_Cell )
    {
        if ( bActive )
        {
            bActiveDrawTextSh = bActiveDrawSh = false;
            bActiveDrawFormSh=false;
            bActiveGraphicSh=false;
            bActiveMediaSh=false;
            bActiveOleObjectSh=false;
            bActiveChartSh=false;
            SetCurSubShell(OST_Pivot);
        }
        else
            SetCurSubShell(OST_Cell);
    }
}

void ScTabViewShell::SetAuditShell( bool bActive )
{
    bActiveAuditingSh = bActive;
    if ( bActive )
    {
        bActiveDrawTextSh = bActiveDrawSh = false;
        bActiveDrawFormSh=false;
        bActiveGraphicSh=false;
        bActiveMediaSh=false;
        bActiveOleObjectSh=false;
        bActiveChartSh=false;
        SetCurSubShell(OST_Auditing);
    }
    else
        SetCurSubShell(OST_Cell);
}

void ScTabViewShell::SetDrawFormShell( bool bActive )
{
    bActiveDrawFormSh = bActive;

    if(bActiveDrawFormSh)
        SetCurSubShell(OST_DrawForm);
}
void ScTabViewShell::SetChartShell( bool bActive )
{
    bActiveChartSh = bActive;

    if(bActiveChartSh)
        SetCurSubShell(OST_Chart);
}

void ScTabViewShell::SetGraphicShell( bool bActive )
{
    bActiveGraphicSh = bActive;

    if(bActiveGraphicSh)
        SetCurSubShell(OST_Graphic);
}

void ScTabViewShell::SetMediaShell( bool bActive )
{
    bActiveMediaSh = bActive;

    if(bActiveMediaSh)
        SetCurSubShell(OST_Media);
}

void ScTabViewShell::SetOleObjectShell( bool bActive )
{
    bActiveOleObjectSh = bActive;

    if(bActiveOleObjectSh)
        SetCurSubShell(OST_OleObject);
    else
        SetCurSubShell(OST_Cell);
}

void ScTabViewShell::SetEditShell(EditView* pView, bool bActive )
{
    if(bActive)
    {
        if (pEditShell)
            pEditShell->SetEditView( pView );
        else
            pEditShell = new ScEditShell( pView, &GetViewData() );

        SetCurSubShell(OST_Editing);
    }
    else if(bActiveEditSh)
    {
        SetCurSubShell(OST_Cell);
    }
    bActiveEditSh = bActive;
}

void ScTabViewShell::SetCurSubShell(ObjectSelectionType eOST, bool bForce)
{
    ScViewData& rViewData   = GetViewData();
    ScDocShell* pDocSh      = rViewData.GetDocShell();

    if(bDontSwitch) return;

    if(!pCellShell) // is anyway always used
    {
        pCellShell = new ScCellShell( &GetViewData() );
        pCellShell->SetRepeatTarget( &aTarget );
    }

    bool bPgBrk = rViewData.IsPagebreakMode();

    if(bPgBrk && !pPageBreakShell)
    {
        pPageBreakShell = new ScPageBreakShell( this );
        pPageBreakShell->SetRepeatTarget( &aTarget );
    }

    if ( eOST!=eCurOST || bForce )
    {
        bool bCellBrush = false;    // "format paint brush" allowed for cells
        bool bDrawBrush = false;    // "format paint brush" allowed for drawing objects

        if(eCurOST!=OST_NONE) RemoveSubShell();

        if (pFormShell && !bFormShellAtTop)
            AddSubShell(*pFormShell);               // add below own subshells

        switch(eOST)
        {
            case    OST_Cell:
            {
                AddSubShell(*pCellShell);
                if(bPgBrk) AddSubShell(*pPageBreakShell);
                bCellBrush = true;
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
                    pDrawTextShell = new ScDrawTextObjectBar( &GetViewData() );
                }
                AddSubShell(*pDrawTextShell);
            }
            break;
            case    OST_Drawing:
            {
                if (svx::checkForSelectedCustomShapes(
                            GetScDrawView(), true /* bOnlyExtruded */ )) {
                    if (pExtrusionBarShell == nullptr)
                        pExtrusionBarShell = new svx::ExtrusionBar(this);
                    AddSubShell( *pExtrusionBarShell );
                }
                sal_uInt32 nCheckStatus = 0;
                if (svx::checkForSelectedFontWork(
                            GetScDrawView(), nCheckStatus )) {
                    if (pFontworkBarShell == nullptr)
                        pFontworkBarShell = new svx::FontworkBar(this);
                    AddSubShell( *pFontworkBarShell );
                }

                if ( !pDrawShell )
                {
                    pDocSh->MakeDrawLayer();
                    pDrawShell = new ScDrawShell( &GetViewData() );
                    pDrawShell->SetRepeatTarget( &aTarget );
                }
                AddSubShell(*pDrawShell);
                bDrawBrush = true;
            }
            break;

            case    OST_DrawForm:
            {
                if ( !pDrawFormShell )
                {
                    pDocSh->MakeDrawLayer();
                    pDrawFormShell = new ScDrawFormShell( &GetViewData() );
                    pDrawFormShell->SetRepeatTarget( &aTarget );
                }
                AddSubShell(*pDrawFormShell);
                bDrawBrush = true;
            }
            break;

            case    OST_Chart:
            {
                if ( !pChartShell )
                {
                    pDocSh->MakeDrawLayer();
                    pChartShell = new ScChartShell( &GetViewData() );
                    pChartShell->SetRepeatTarget( &aTarget );
                }
                AddSubShell(*pChartShell);
                bDrawBrush = true;
            }
            break;

            case    OST_OleObject:
            {
                if ( !pOleObjectShell )
                {
                    pDocSh->MakeDrawLayer();
                    pOleObjectShell = new ScOleObjectShell( &GetViewData() );
                    pOleObjectShell->SetRepeatTarget( &aTarget );
                }
                AddSubShell(*pOleObjectShell);
                bDrawBrush = true;
            }
            break;

            case    OST_Graphic:
            {
                if ( !pGraphicShell)
                {
                    pDocSh->MakeDrawLayer();
                    pGraphicShell = new ScGraphicShell( &GetViewData() );
                    pGraphicShell->SetRepeatTarget( &aTarget );
                }
                AddSubShell(*pGraphicShell);
                bDrawBrush = true;
            }
            break;

            case    OST_Media:
            {
                if ( !pMediaShell)
                {
                    pDocSh->MakeDrawLayer();
                    pMediaShell = new ScMediaShell( &GetViewData() );
                    pMediaShell->SetRepeatTarget( &aTarget );
                }
                AddSubShell(*pMediaShell);
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
                bCellBrush = true;
            }
            break;
            case    OST_Auditing:
            {
                AddSubShell(*pCellShell);
                if(bPgBrk) AddSubShell(*pPageBreakShell);

                if ( !pAuditingShell )
                {
                    pDocSh->MakeDrawLayer();    // the waiting time rather now as on the click

                    pAuditingShell = new ScAuditingShell( &GetViewData() );
                    pAuditingShell->SetRepeatTarget( &aTarget );
                }
                AddSubShell(*pAuditingShell);
                bCellBrush = true;
            }
            break;
            default:
            OSL_FAIL("wrong shell requested");
            break;
        }

        if (pFormShell && bFormShellAtTop)
            AddSubShell(*pFormShell);               // add on top of own subshells

        eCurOST=eOST;

        // abort "format paint brush" when switching to an incompatible shell
        if ( ( GetBrushDocument() && !bCellBrush ) || ( GetDrawBrushSet() && !bDrawBrush ) )
            ResetBrushDocument();
    }
}

void ScTabViewShell::SetFormShellAtTop( bool bSet )
{
    if ( pFormShell && !bSet )
        pFormShell->ForgetActiveControl();      // let the FormShell know it no longer has the focus

    if ( bFormShellAtTop != bSet )
    {
        bFormShellAtTop = bSet;
        SetCurSubShell( GetCurObjectSelectionType(), true );
    }
}

IMPL_LINK_NOARG_TYPED(ScTabViewShell, FormControlActivated, LinkParamNone*, void)
{
    // a form control got the focus, so the form shell has to be on top
    SetFormShellAtTop( true );
}

// GetMySubShell / SetMySubShell: simulate old behavior,
// so that there is only one SubShell (only whithin the 5 own SubShells)

SfxShell* ScTabViewShell::GetMySubShell() const
{
    //  GetSubShell() was const before, and GetSubShell(sal_uInt16) should also be const...

    sal_uInt16 nPos = 0;
    SfxShell* pSub = const_cast<ScTabViewShell*>(this)->GetSubShell(nPos);
    while (pSub)
    {
        if ( pSub == pDrawShell  || pSub == pDrawTextShell || pSub == pEditShell ||
             pSub == pPivotShell || pSub == pAuditingShell || pSub == pDrawFormShell ||
             pSub == pCellShell  || pSub == pOleObjectShell|| pSub == pChartShell ||
             pSub == pGraphicShell || pSub == pMediaShell || pSub == pPageBreakShell)
            return pSub;    // found

        pSub = const_cast<ScTabViewShell*>(this)->GetSubShell(++nPos);
    }
    return nullptr;        // none from mine present
}

bool ScTabViewShell::IsDrawTextShell() const
{
    return ( pDrawTextShell && ( GetMySubShell() == pDrawTextShell ) );
}

bool ScTabViewShell::IsAuditShell() const
{
    return ( pAuditingShell && ( GetMySubShell() == pAuditingShell ) );
}

void ScTabViewShell::SetDrawTextUndo( ::svl::IUndoManager* pNewUndoMgr )
{
    // Default: undo manager for DocShell
    if (!pNewUndoMgr)
        pNewUndoMgr = GetViewData().GetDocShell()->GetUndoManager();

    if (pDrawTextShell)
    {
        pDrawTextShell->SetUndoManager(pNewUndoMgr);
        ScDocShell* pDocSh = GetViewData().GetDocShell();
        if ( pNewUndoMgr == pDocSh->GetUndoManager() &&
             !pDocSh->GetDocument().IsUndoEnabled() )
        {
            pNewUndoMgr->SetMaxUndoActionCount( 0 );
        }
    }
    else
    {
        OSL_FAIL("SetDrawTextUndo without DrawTextShell");
    }
}

ScTabViewShell* ScTabViewShell::GetActiveViewShell()
{
    return dynamic_cast< ScTabViewShell *>( Current() );
}

SfxPrinter* ScTabViewShell::GetPrinter( bool bCreate )
{
    // printer is always present (is created for the FontList already on start-up)
    return GetViewData().GetDocShell()->GetPrinter(bCreate);
}

sal_uInt16 ScTabViewShell::SetPrinter( SfxPrinter *pNewPrinter, SfxPrinterChangeFlags nDiffFlags, bool )
{
    return GetViewData().GetDocShell()->SetPrinter( pNewPrinter, nDiffFlags );
}

bool ScTabViewShell::HasPrintOptionsPage() const
{
    return true;
}

VclPtr<SfxTabPage> ScTabViewShell::CreatePrintOptionsPage( vcl::Window *pParent, const SfxItemSet &rOptions )
{
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");
    ::CreateTabPage ScTpPrintOptionsCreate = pFact->GetTabPageCreatorFunc( RID_SCPAGE_PRINT );
    if ( ScTpPrintOptionsCreate )
        return ScTpPrintOptionsCreate( pParent, &rOptions );
    return VclPtr<SfxTabPage>();
}

void ScTabViewShell::StopEditShell()
{
    if ( pEditShell != nullptr && !bDontSwitch )
        SetEditShell(nullptr, false );
}

// close handler to ensure function of dialog:

IMPL_LINK_NOARG_TYPED(ScTabViewShell, SimpleRefClose, const OUString*, void)
{
    SfxInPlaceClient* pClient = GetIPClient();
    if ( pClient && pClient->IsObjectInPlaceActive() )
    {
        // If range selection was started with an active embedded object,
        // switch back to original sheet (while the dialog is still open).

        SetTabNo( GetViewData().GetRefTabNo() );
    }

    ScSimpleRefDlgWrapper::SetAutoReOpen( true );
}

// handlers to call UNO listeners:

static ScTabViewObj* lcl_GetViewObj( ScTabViewShell& rShell )
{
    ScTabViewObj* pRet = nullptr;
    SfxViewFrame* pViewFrame = rShell.GetViewFrame();
    if (pViewFrame)
    {
        SfxFrame& rFrame = pViewFrame->GetFrame();
        uno::Reference<frame::XController> xController = rFrame.GetController();
        if (xController.is())
            pRet = ScTabViewObj::getImplementation( xController );
    }
    return pRet;
}

IMPL_LINK_TYPED( ScTabViewShell, SimpleRefDone, const OUString&, aResult, void )
{
    ScTabViewObj* pImpObj = lcl_GetViewObj( *this );
    if ( pImpObj )
        pImpObj->RangeSelDone( aResult );
}

IMPL_LINK_TYPED( ScTabViewShell, SimpleRefAborted, const OUString&, rResult, void )
{
    ScTabViewObj* pImpObj = lcl_GetViewObj( *this );
    if ( pImpObj )
        pImpObj->RangeSelAborted( rResult );
}

IMPL_LINK_TYPED( ScTabViewShell, SimpleRefChange, const OUString&, rResult, void )
{
    ScTabViewObj* pImpObj = lcl_GetViewObj( *this );
    if ( pImpObj )
        pImpObj->RangeSelChanged( rResult );
}

void ScTabViewShell::StartSimpleRefDialog(
            const OUString& rTitle, const OUString& rInitVal,
            bool bCloseOnButtonUp, bool bSingleCell, bool bMultiSelection )
{
    SfxViewFrame* pViewFrm = GetViewFrame();

    if ( GetActiveViewShell() != this )
    {
        // #i18833# / #i34499# The API method can be called for a view that's not active.
        // Then the view has to be activated first, the same way as in Execute for SID_CURRENTDOC.
        // Can't use GrabFocus here, because it needs to take effect immediately.

        pViewFrm->GetFrame().Appear();
    }

    sal_uInt16 nId = ScSimpleRefDlgWrapper::GetChildWindowId();

    SC_MOD()->SetRefDialog( nId, true, pViewFrm );

    ScSimpleRefDlgWrapper* pWnd = static_cast<ScSimpleRefDlgWrapper*>(pViewFrm->GetChildWindow( nId ));
    if (pWnd)
    {
        pWnd->SetCloseHdl( LINK( this, ScTabViewShell, SimpleRefClose ) );
        pWnd->SetUnoLinks( LINK( this, ScTabViewShell, SimpleRefDone ),
                           LINK( this, ScTabViewShell, SimpleRefAborted ),
                           LINK( this, ScTabViewShell, SimpleRefChange ) );
        pWnd->SetRefString( rInitVal );
        pWnd->SetFlags( bCloseOnButtonUp, bSingleCell, bMultiSelection );
        ScSimpleRefDlgWrapper::SetAutoReOpen( false );
        vcl::Window* pWin = pWnd->GetWindow();
        pWin->SetText( rTitle );
        pWnd->StartRefInput();
    }
}

void ScTabViewShell::StopSimpleRefDialog()
{
    SfxViewFrame* pViewFrm = GetViewFrame();
    sal_uInt16 nId = ScSimpleRefDlgWrapper::GetChildWindowId();

    ScSimpleRefDlgWrapper* pWnd = static_cast<ScSimpleRefDlgWrapper*>(pViewFrm->GetChildWindow( nId ));
    if (pWnd)
    {
        vcl::Window* pWin = pWnd->GetWindow();
        if (pWin && pWin->IsSystemWindow())
            static_cast<SystemWindow*>(pWin)->Close();     // calls abort handler
    }
}

bool ScTabViewShell::TabKeyInput(const KeyEvent& rKEvt)
{
    ScModule* pScMod = SC_MOD();

    SfxViewFrame* pThisFrame = GetViewFrame();
    if ( pThisFrame->GetChildWindow( SID_OPENDLG_FUNCTION ) )
        return false;

    vcl::KeyCode aCode = rKEvt.GetKeyCode();
    bool bShift     = aCode.IsShift();
    bool bControl   = aCode.IsMod1();
    bool bAlt       = aCode.IsMod2();
    sal_uInt16 nCode    = aCode.GetCode();
    bool bUsed      = false;
    bool bInPlace   = pScMod->IsEditMode();     // Editengine gets all
    bool bAnyEdit   = pScMod->IsInputMode();    // only characters & backspace
    bool bDraw      = IsDrawTextEdit();

    HideNoteMarker();   // note marker

    // don't do extra HideCursor/ShowCursor calls if EnterHandler will switch to a different sheet
    bool bOnRefSheet = ( GetViewData().GetRefTabNo() == GetViewData().GetTabNo() );
    bool bHideCursor = ( ( nCode == KEY_RETURN && bInPlace ) || nCode == KEY_TAB ) && bOnRefSheet;

    if (bHideCursor)
        HideAllCursors();

    ScDocument* pDoc = GetViewData().GetDocument();
    if ( pDoc )
        pDoc->KeyInput( rKEvt );    // TimerDelays etc.

    if( bInPlace )
    {
        bUsed = pScMod->InputKeyEvent( rKEvt );         // input
        if( !bUsed )
            bUsed = SfxViewShell::KeyInput( rKEvt );    // accelerators
    }
    else if( bAnyEdit )
    {
        bool bIsType = false;
        sal_uInt16 nModi = aCode.GetModifier();
        sal_uInt16 nGroup = aCode.GetGroup();

        if ( nGroup == KEYGROUP_NUM || nGroup == KEYGROUP_ALPHA || nGroup == 0 )
            if ( !bControl && !bAlt )
                bIsType = true;

        if ( nGroup == KEYGROUP_MISC )
            switch ( nCode )
            {
                case KEY_RETURN:
                    bIsType = bControl && !bAlt;        // Control, Shift-Control-Return
                    if ( !bIsType && nModi == 0 )
                    {
                        // Does the Input Handler also want a simple Return?

                        ScInputHandler* pHdl = pScMod->GetInputHdl(this);
                        bIsType = pHdl && pHdl->TakesReturn();
                    }
                    break;
                case KEY_SPACE:
                    bIsType = !bControl && !bAlt;       // without modifier or Shift-Space
                    break;
                case KEY_ESCAPE:
                    bIsType = (nModi == 0); // only without modifier
                    break;
                default:
                    bIsType = true;
            }

        if( bIsType )
            bUsed = pScMod->InputKeyEvent( rKEvt );     // input

        if( !bUsed )
            bUsed = SfxViewShell::KeyInput( rKEvt );    // accelerators

        if ( !bUsed && !bIsType && nCode != KEY_RETURN )    // input once again afterwards
            bUsed = pScMod->InputKeyEvent( rKEvt );
    }
    else
    {
        // special case: copy/cut for multiselect  -> error message
        //  (Slot is disabled, so SfxViewShell::KeyInput would be swallowed without a comment)
        KeyFuncType eFunc = aCode.GetFunction();
        if ( eFunc == KeyFuncType::CUT )
        {
            ScRange aDummy;
            ScMarkType eMarkType = GetViewData().GetSimpleArea( aDummy );
            if (eMarkType != SC_MARK_SIMPLE)
            {
                ErrorMessage(STR_NOMULTISELECT);
                bUsed = true;
            }
        }
        if (!bUsed)
            bUsed = SfxViewShell::KeyInput( rKEvt );    // accelerators

        //  during inplace editing, some slots are handled by the
        //  container app and are executed during Window::KeyInput.
        //  -> don't pass keys to input handler that would be used there
        //  but should call slots instead.
        bool bParent = ( GetViewFrame()->GetFrame().IsInPlace() && eFunc != KeyFuncType::DONTKNOW );

        if( !bUsed && !bDraw && nCode != KEY_RETURN && !bParent )
            bUsed = pScMod->InputKeyEvent( rKEvt, true );       // input
    }

    if (!bInPlace && !bUsed && !bDraw)
    {
        switch (nCode)
        {
            case KEY_RETURN:
                {
                    bool bNormal = !bControl && !bAlt;
                    if ( !bAnyEdit && bNormal )
                    {
                        // Depending on options, Enter switches to edit mode.
                        const ScInputOptions& rOpt = pScMod->GetInputOptions();
                        if ( rOpt.GetEnterEdit() )
                        {
                            pScMod->SetInputMode( SC_INPUT_TABLE );
                            bUsed = true;
                        }
                    }

                    bool bEditReturn = bControl && !bShift;         // pass on to edit engine
                    if ( !bUsed && !bEditReturn )
                    {
                        if ( bOnRefSheet )
                            HideAllCursors();

                        ScEnterMode nMode = ScEnterMode::NORMAL;
                        if ( bShift && bControl )
                            nMode = ScEnterMode::MATRIX;
                        else if ( bAlt )
                            nMode = ScEnterMode::BLOCK;
                        pScMod->InputEnterHandler(nMode);

                        if (nMode == ScEnterMode::NORMAL)
                        {
                            if( bShift )
                                GetViewData().GetDispatcher().Execute( SID_CURSORENTERUP,
                                            SfxCallMode::SLOT | SfxCallMode::RECORD );
                            else
                                GetViewData().GetDispatcher().Execute( SID_CURSORENTERDOWN,
                                            SfxCallMode::SLOT | SfxCallMode::RECORD );
                        }
                        else
                            UpdateInputHandler(true);

                        if ( bOnRefSheet )
                            ShowAllCursors();

                        // here no UpdateInputHandler, since during reference input on another
                        // document this ViewShell is not the one that is used for input.

                        bUsed = true;
                    }
                }
                break;
        }
    }

    // hard-code Alt-Cursor key, since Alt is not configurable

    if ( !bUsed && bAlt && !bControl )
    {
        sal_uInt16 nSlotId = 0;
        switch (nCode)
        {
            case KEY_UP:
                ModifyCellSize( DIR_TOP, bShift );
                bUsed = true;
                break;
            case KEY_DOWN:
                ModifyCellSize( DIR_BOTTOM, bShift );
                bUsed = true;
                break;
            case KEY_LEFT:
                ModifyCellSize( DIR_LEFT, bShift );
                bUsed = true;
                break;
            case KEY_RIGHT:
                ModifyCellSize( DIR_RIGHT, bShift );
                bUsed = true;
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
            GetViewData().GetDispatcher().Execute( nSlotId, SfxCallMode::SLOT | SfxCallMode::RECORD );
            bUsed = true;
        }
    }

    // use Ctrl+Alt+Shift+arrow keys to move the cursor in cells
    // while keeping the last selection
    if ( !bUsed && bAlt && bControl && bShift)
    {
        sal_uInt16 nSlotId = 0;
        switch (nCode)
        {
            case KEY_UP:
                nSlotId = SID_CURSORUP;
                break;
            case KEY_DOWN:
                nSlotId = SID_CURSORDOWN;
                break;
            case KEY_LEFT:
                nSlotId = SID_CURSORLEFT;
                break;
            case KEY_RIGHT:
                nSlotId = SID_CURSORRIGHT;
                break;
            case KEY_PAGEUP:
                nSlotId = SID_CURSORPAGEUP;
                break;
            case KEY_PAGEDOWN:
                nSlotId = SID_CURSORPAGEDOWN;
                break;
            case KEY_HOME:
                nSlotId = SID_CURSORHOME;
                break;
            case KEY_END:
                nSlotId = SID_CURSOREND;
                break;
            default:
                nSlotId = 0;
                break;
        }
        if ( nSlotId )
        {
            sal_uInt16 nMode = GetLockedModifiers();
            LockModifiers(KEY_MOD1);
            GetViewData().GetDispatcher().Execute( nSlotId, SfxCallMode::SLOT | SfxCallMode::RECORD );
            LockModifiers(nMode);
            bUsed = true;
        }
    }
    if (bHideCursor)
        ShowAllCursors();

    return bUsed;
}

bool ScTabViewShell::SfxKeyInput(const KeyEvent& rKeyEvent)
{
    return SfxViewShell::KeyInput( rKeyEvent );
}

bool ScTabViewShell::KeyInput( const KeyEvent &rKeyEvent )
{
    return TabKeyInput( rKeyEvent );
}

void ScTabViewShell::Construct( TriState nForceDesignMode )
{
    SfxApplication* pSfxApp  = SfxGetpApp();
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    bReadOnly = pDocSh->IsReadOnly();
    bIsActive = false;

    EnableAutoSpell(rDoc.GetDocOptions().IsAutoSpell());

    SetName("View"); // for SBX
    Color aColBlack( COL_BLACK );
    SetPool( &SC_MOD()->GetPool() );
    SetWindow( GetActiveWin() );

    pCurFrameLine   = new ::editeng::SvxBorderLine(&aColBlack, 20,
                            table::BorderLineStyle::SOLID);
    pPivotSource    = new ScArea;
    StartListening(*GetViewData().GetDocShell(),true);
    StartListening(*GetViewFrame(),true);
    StartListening(*pSfxApp,true);              // #i62045# #i62046# application is needed for Calc's own hints

    SfxViewFrame* pFirst = SfxViewFrame::GetFirst(pDocSh);
    bool bFirstView = !pFirst
          || (pFirst == GetViewFrame() && !SfxViewFrame::GetNext(*pFirst,pDocSh));

    if ( pDocSh->GetCreateMode() == SfxObjectCreateMode::EMBEDDED )
    {
        //TODO/LATER: is there a difference between the two GetVisArea methods?
        Rectangle aVisArea = static_cast<const SfxObjectShell*>(pDocSh)->GetVisArea();

        SCTAB nVisTab = rDoc.GetVisibleTab();
        if (!rDoc.HasTable(nVisTab))
        {
            nVisTab = 0;
            rDoc.SetVisibleTab(nVisTab);
        }
        SetTabNo( nVisTab );
        bool bNegativePage = rDoc.IsNegativePage( nVisTab );
        // show the right cells
        GetViewData().SetScreenPos( bNegativePage ? aVisArea.TopRight() : aVisArea.TopLeft() );

        if ( GetViewFrame()->GetFrame().IsInPlace() )                         // inplace
        {
            pDocSh->SetInplace( true );             // already initiated like this
            if (rDoc.IsEmbedded())
                rDoc.ResetEmbedded();              // no blue mark
        }
        else if ( bFirstView )
        {
            pDocSh->SetInplace( false );
            GetViewData().RefreshZoom();           // recalculate PPT
            if (!rDoc.IsEmbedded())
                rDoc.SetEmbedded( rDoc.GetVisibleTab(), aVisArea );                  // mark VisArea
        }
    }

    // ViewInputHandler
    // Each task now has its own InputWindow,
    // therefore either should each task get its own InputHandler,
    // or the InputWindow should create its own InputHandler
    // (then always search via InputWindow and only if not found
    // use the InputHandler of the App).
    // As an intermediate solution each View gets its own InputHandler,
    // which only yields problems if two Views are in one task window.

    pInputHandler = new ScInputHandler;

    // old version:
    //  if ( !GetViewFrame()->ISA(SfxTopViewFrame) )        // OLE or Plug-In
    //      pInputHandler = new ScInputHandler;

            // create FormShell before MakeDrawView, so that DrawView can be registered at the
            // FormShell in every case
            // the FormShell is pushed in the first activate
    pFormShell = new FmFormShell(this);
    pFormShell->SetControlActivationHandler( LINK( this, ScTabViewShell, FormControlActivated ) );

            // DrawView must not be created in TabView - ctor,
            // if the ViewShell is not yet constructed...
    if (rDoc.GetDrawLayer())
        MakeDrawView( nForceDesignMode );
    ViewOptionsHasChanged(false);   // possibly also creates DrawView

    ::svl::IUndoManager* pMgr = pDocSh->GetUndoManager();
    SetUndoManager( pMgr );
    pFormShell->SetUndoManager( pMgr );
    if ( !rDoc.IsUndoEnabled() )
    {
        pMgr->SetMaxUndoActionCount( 0 );
    }
    SetRepeatTarget( &aTarget );
    pFormShell->SetRepeatTarget( &aTarget );
    SetHelpId( HID_SCSHELL_TABVWSH );

    if ( bFirstView )   // first view?
    {
        rDoc.SetDocVisible( true );        // used when creating new sheets
        if ( pDocSh->IsEmpty() )
        {
            // set first sheet's RTL flag (following will already be initialized because of SetDocVisible)
            rDoc.SetLayoutRTL( 0, ScGlobal::IsSystemRTL() );

            // append additional sheets (not for OLE object)
            if ( pDocSh->GetCreateMode() != SfxObjectCreateMode::EMBEDDED )
            {
                // Get the customized initial tab count
                const ScDefaultsOptions& rOpt = SC_MOD()->GetDefaultsOptions();
                SCTAB nInitTabCount = rOpt.GetInitTabCount();

                for (SCTAB i=1; i<nInitTabCount; i++)
                    rDoc.MakeTable(i,false);
            }

            pDocSh->SetEmpty( false );          // #i6232# make sure this is done only once
        }

        // ReadExtOptions is now in Activate

        // link update no nesting
        if ( pDocSh->GetCreateMode() != SfxObjectCreateMode::INTERNAL &&
             pDocSh->IsUpdateEnabled() )  // #105575#; update only in the first creation of the ViewShell
        {
            // Check if there are any external data.
            bool bLink = rDoc.GetExternalRefManager()->hasExternalData();
            if (!bLink)
            {
                // #i100042# sheet links can still exist independently from external formula references
                SCTAB nTabCount = rDoc.GetTableCount();
                for (SCTAB i=0; i<nTabCount && !bLink; i++)
                    if (rDoc.IsLinked(i))
                        bLink = true;
            }
            if (!bLink)
            {
                const sc::DocumentLinkManager& rMgr = rDoc.GetDocLinkManager();
                if (rMgr.hasDdeLinks() || rDoc.HasAreaLinks())
                    bLink = true;
            }
            if (bLink)
            {
                if ( !pFirst )
                    pFirst = GetViewFrame();

                if(SC_MOD()->GetCurRefDlgId()==0)
                {
                        pFirst->GetDispatcher()->Execute( SID_UPDATETABLINKS,
                                            SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
                }
            }

            bool bReImport = false;                             // update imported data
            ScDBCollection* pDBColl = rDoc.GetDBCollection();
            if ( pDBColl )
            {
                const ScDBCollection::NamedDBs& rDBs = pDBColl->getNamedDBs();
                ScDBCollection::NamedDBs::const_iterator itr = rDBs.begin(), itrEnd = rDBs.end();
                for (; itr != itrEnd; ++itr)
                {
                    if ((*itr)->IsStripData() && (*itr)->HasImportParam()
                        && !(*itr)->HasImportSelection())
                    {
                        bReImport = true;
                        break;
                    }
                }
            }
            if (bReImport)
            {
                if ( !pFirst )
                    pFirst = GetViewFrame();
                if(SC_MOD()->GetCurRefDlgId()==0)
                {
                    pFirst->GetDispatcher()->Execute( SID_REIMPORT_AFTER_LOAD,
                                            SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
                }
            }
        }
    }

    UpdateAutoFillMark();

    // ScDispatchProviderInterceptor registers itself in ctor
    xDisProvInterceptor = new ScDispatchProviderInterceptor( this );

    bFirstActivate = true; // NavigatorUpdate aufschieben bis Activate()

    // #105575#; update only in the first creation of the ViewShell
    pDocSh->SetUpdateEnabled(false);

    if ( GetViewFrame()->GetFrame().IsInPlace() )
        UpdateHeaderWidth(); // The inplace activation requires headers to be calculated

    SvBorder aBorder;
    GetBorderSize( aBorder, Size() );
    SetBorderPixel( aBorder );
}

ScTabViewShell::ScTabViewShell( SfxViewFrame* pViewFrame,
                                SfxViewShell* pOldSh ) :
    SfxViewShell( pViewFrame, SfxViewShellFlags::CAN_PRINT | SfxViewShellFlags::HAS_PRINTOPTIONS ),
    ScDBFunc( &pViewFrame->GetWindow(), static_cast<ScDocShell&>(*pViewFrame->GetObjectShell()), this ),
    eCurOST(OST_NONE),
    nDrawSfxId(0),
    pDrawShell(nullptr),
    pDrawTextShell(nullptr),
    pEditShell(nullptr),
    pPivotShell(nullptr),
    pAuditingShell(nullptr),
    pDrawFormShell(nullptr),
    pCellShell(nullptr),
    pOleObjectShell(nullptr),
    pChartShell(nullptr),
    pGraphicShell(nullptr),
    pMediaShell(nullptr),
    pPageBreakShell(nullptr),
    pExtrusionBarShell(nullptr),
    pFontworkBarShell(nullptr),
    pFormShell(nullptr),
    pInputHandler(nullptr),
    pCurFrameLine(nullptr),
    aTarget(this),
    pDialogDPObject(nullptr),
    pNavSettings(nullptr),
    bActiveDrawSh(false),
    bActiveDrawTextSh(false),
    bActivePivotSh(false),
    bActiveAuditingSh(false),
    bActiveDrawFormSh(false),
    bActiveOleObjectSh(false),
    bActiveChartSh(false),
    bActiveGraphicSh(false),
    bActiveMediaSh(false),
    bActiveEditSh(false),
    bFormShellAtTop(false),
    bDontSwitch(false),
    bInFormatDialog(false),
    bReadOnly(false),
    bChartAreaValid(false),
    bForceFocusOnCurCell(false),
    nCurRefDlgId(0),
    pAccessibilityBroadcaster(nullptr),
    mbInSwitch(false)
{
    const ScAppOptions& rAppOpt = SC_MOD()->GetAppOptions();

    //  if switching back from print preview,
    //  restore the view settings that were active when creating the preview
    //  ReadUserData must not happen from ctor, because the view's edit window
    //  has to be shown by the sfx. ReadUserData is deferred until the first Activate call.
    //  old DesignMode state from form layer must be restored, too

    TriState nForceDesignMode = TRISTATE_INDET;
    if ( pOldSh && dynamic_cast<const ScPreviewShell*>( pOldSh) !=  nullptr )
    {
        ScPreviewShell* pPreviewShell = static_cast<ScPreviewShell*>(pOldSh);
        nForceDesignMode = pPreviewShell->GetSourceDesignMode();
        ScPreview* p = pPreviewShell->GetPreview();
        if (p)
            GetViewData().GetMarkData().SetSelectedTabs(p->GetSelectedTabs());
    }

    Construct( nForceDesignMode );

    if ( GetViewData().GetDocShell()->IsPreview() )
    {
        //  preview for template dialog: always show whole page
        SetZoomType( SvxZoomType::WHOLEPAGE, true );    // zoom value is recalculated at next Resize
    }
    else
    {
        Fraction aFract( rAppOpt.GetZoom(), 100 );
        SetZoom( aFract, aFract, true );
        SetZoomType( rAppOpt.GetZoomType(), true );
    }

    // make Controller known to SFX
    new ScTabViewObj( this );

    SetCurSubShell(OST_Cell);
    SvBorder aBorder;
    GetBorderSize( aBorder, Size() );
    SetBorderPixel( aBorder );

    MakeDrawLayer();
}

ScTabViewShell::~ScTabViewShell()
{
    // Notify other LOK views that we are going away.
    SfxLokHelper::notifyOtherViews(this, LOK_CALLBACK_VIEW_CURSOR_VISIBLE, "visible", "false");
    SfxLokHelper::notifyOtherViews(this, LOK_CALLBACK_TEXT_VIEW_SELECTION, "selection", "");
    SfxLokHelper::notifyOtherViews(this, LOK_CALLBACK_GRAPHIC_VIEW_SELECTION, "selection", "EMPTY");
    SfxLokHelper::notifyOtherViews(this, LOK_CALLBACK_CELL_VIEW_CURSOR, "rectangle", "EMPTY");

    ScDocShell* pDocSh = GetViewData().GetDocShell();
    EndListening(*pDocSh);
    EndListening(*GetViewFrame());
    EndListening(*SfxGetpApp());           // #i62045# #i62046# needed now - SfxViewShell no longer does it

    SC_MOD()->ViewShellGone(this);

    RemoveSubShell();           // all
    SetWindow(nullptr);

    // all to NULL, in case the TabView-dtor tries to access them
    //! (should not really! ??!?!)
    if (pInputHandler)
        pInputHandler->SetDocumentDisposing(true);

    DELETEZ(pFontworkBarShell);
    DELETEZ(pExtrusionBarShell);
    DELETEZ(pCellShell);
    DELETEZ(pPageBreakShell);
    DELETEZ(pDrawShell);
    DELETEZ(pDrawFormShell);
    DELETEZ(pOleObjectShell);
    DELETEZ(pChartShell);
    DELETEZ(pGraphicShell);
    DELETEZ(pMediaShell);
    DELETEZ(pDrawTextShell);
    DELETEZ(pEditShell);
    DELETEZ(pPivotShell);
    DELETEZ(pAuditingShell);
    DELETEZ(pCurFrameLine);
    DELETEZ(pInputHandler);
    DELETEZ(pPivotSource);
    DELETEZ(pDialogDPObject);
    DELETEZ(pNavSettings);

    DELETEZ(pFormShell);
    DELETEZ(pAccessibilityBroadcaster);
}

void ScTabViewShell::SetDialogDPObject( const ScDPObject* pObj )
{
    delete pDialogDPObject;
    if (pObj)
        pDialogDPObject = new ScDPObject( *pObj );
    else
        pDialogDPObject = nullptr;
}

void ScTabViewShell::FillFieldData( ScHeaderFieldData& rData )
{
    ScDocShell* pDocShell = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocShell->GetDocument();
    SCTAB nTab = GetViewData().GetTabNo();
    OUString aTmp;
    rDoc.GetName(nTab, aTmp);
    rData.aTabName = aTmp;

    if( pDocShell->getDocProperties()->getTitle().getLength() != 0 )
        rData.aTitle = pDocShell->getDocProperties()->getTitle();
    else
        rData.aTitle = pDocShell->GetTitle();

    const INetURLObject& rURLObj = pDocShell->GetMedium()->GetURLObject();
    rData.aLongDocName  = rURLObj.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS );
    if ( !rData.aLongDocName.isEmpty() )
        rData.aShortDocName = rURLObj.GetName( INetURLObject::DECODE_UNAMBIGUOUS );
    else
        rData.aShortDocName = rData.aLongDocName = rData.aTitle;
    rData.nPageNo       = 1;
    rData.nTotalPages   = 99;

    // eNumType is known by the dialog
}

bool ScTabViewShell::GetChartArea( ScRangeListRef& rSource, Rectangle& rDest, SCTAB& rTab ) const
{
    rSource = aChartSource;
    rDest   = aChartPos;
    rTab    = nChartDestTab;
    return bChartAreaValid;
}

ScNavigatorSettings* ScTabViewShell::GetNavigatorSettings()
{
    if( !pNavSettings )
        pNavSettings = new ScNavigatorSettings;
    return pNavSettings;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
