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

#include <com/sun/star/document/XDocumentProperties.hpp>

void ActivateOlk( ScViewData* pViewData );
void DeActivateOlk( ScViewData* pViewData );

extern SfxViewShell* pScActiveViewShell;            

using namespace com::sun::star;



sal_uInt16 ScTabViewShell::nInsertCtrlState = SID_INSERT_GRAPHIC;
sal_uInt16 ScTabViewShell::nInsCellsCtrlState = 0;
sal_uInt16 ScTabViewShell::nInsObjCtrlState = SID_INSERT_DIAGRAM;



void ScTabViewShell::Activate(sal_Bool bMDI)
{
    SfxViewShell::Activate(bMDI);
    bIsActive = true;
    

    if ( bMDI )
    {
        
        ScModule* pScMod = SC_MOD();
        pScMod->ViewShellChanged();

        ActivateView( true, bFirstActivate );
        ActivateOlk( GetViewData() );

        
        UpdateDrawTextOutliner();

        

        SfxViewFrame* pThisFrame  = GetViewFrame();
        if ( pInputHandler && pThisFrame->HasChildWindow(FID_INPUTLINE_STATUS) )
        {
            
            
            
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

        UpdateInputHandler( true );

        if ( bFirstActivate )
        {
            SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_NAVIGATOR_UPDATEALL ) );
            bFirstActivate = false;

            
            
            
            
            
            
            ScExtDocOptions* pExtOpt = GetViewData()->GetDocument()->GetExtDocOptions();
            if ( pExtOpt && pExtOpt->IsChanged() )
            {
                GetViewData()->ReadExtOptions(*pExtOpt);        
                SetTabNo( GetViewData()->GetTabNo(), true );
                pExtOpt->SetChanged( false );
            }
        }

        pScActiveViewShell = this;

        ScInputHandler* pHdl = pScMod->GetInputHdl(this);
        if (pHdl)
        {
            pHdl->SetRefScale( GetViewData()->GetZoomX(), GetViewData()->GetZoomY() );
        }

        

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
            sal_uInt16 nModRefDlgId=pScMod->GetCurRefDlgId();
            SfxChildWindow* pChildWnd = pThisFrame->GetChildWindow( nModRefDlgId );
            if ( pChildWnd )
            {
                IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
                pRefDlg->ViewShellChanged();
            }
        }
    }

    
    

    
    
    

    ContextChangeEventMultiplexer::NotifyContextChange(
        GetController(),
        ::sfx2::sidebar::EnumContext::Context_Default);
}

void ScTabViewShell::Deactivate(sal_Bool bMDI)
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
    bIsActive = false;
    ScInputHandler* pHdl = SC_MOD()->GetInputHdl(this);

    if( bMDI )
    {
        
        
        bool bOldDontSwitch = bDontSwitch;
        bDontSwitch = true;

        DeActivateOlk( GetViewData() );
        ActivateView( false, false );

        if ( GetViewFrame()->GetFrame().IsInPlace() ) 
            GetViewData()->GetDocShell()->UpdateOle(GetViewData(), true);

        if ( pHdl )
            pHdl->NotifyChange( NULL, true ); 

        if (pScActiveViewShell == this)
            pScActiveViewShell = NULL;

        bDontSwitch = bOldDontSwitch;
    }
    else
    {
        HideNoteMarker();           

        if ( pHdl )
            pHdl->HideTip();        
    }
}

void ScTabViewShell::SetActive()
{
    
    
    ActiveGrabFocus();
}

bool ScTabViewShell::PrepareClose(sal_Bool bUI)
{
    
    
    
    ScInputHandler* pHdl = SC_MOD()->GetInputHdl( this );
    if ( pHdl && pHdl->IsInputMode() )
        pHdl->EnterHandler();

    
    FuPoor* pPoor = GetDrawFuncPtr();
    if ( pPoor && ( IsDrawTextShell() || pPoor->GetSlotID() == SID_DRAW_NOTEEDIT ) )
    {
        
        
        GetViewData()->GetDispatcher().Execute( pPoor->GetSlotID(), SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD );
    }
    ScDrawView* pDrView = GetScDrawView();
    if ( pDrView )
    {
        
        
        pDrView->ScEndTextEdit();
    }

    if ( pFormShell )
    {
        bool nRet = pFormShell->PrepareClose(bUI);
        if (!nRet)
            return nRet;
    }
    return SfxViewShell::PrepareClose(bUI);
}






void ScTabViewShell::UpdateOleZoom()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    if ( pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
    {
        
        Size aObjSize = ((const SfxObjectShell*)pDocSh)->GetVisArea().GetSize();
        if ( aObjSize.Width() > 0 && aObjSize.Height() > 0 )
        {
            Window* pWin = GetActiveWin();
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

    DoResize( rOfs, aNewSize, true );                   

    UpdateOleZoom();                                    

    GetViewData()->GetDocShell()->SetDocumentModified();
}

void ScTabViewShell::OuterResizePixel( const Point &rOfs, const Size &rSize )
{
    SvBorder aBorder;
    GetBorderSize( aBorder, rSize );
    SetBorderPixel( aBorder );

    DoResize( rOfs, rSize );                    

    
    

    ForceMove();
}

void ScTabViewShell::SetZoomFactor( const Fraction &rZoomX, const Fraction &rZoomY )
{
    

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
    SetZoom( aNewX, aNewY, true );

    PaintGrid();
    PaintTop();
    PaintLeft();

    SfxViewShell::SetZoomFactor( rZoomX, rZoomY );
}

void ScTabViewShell::QueryObjAreaPixel( Rectangle& rRect ) const
{
    

    Size aPixelSize = rRect.GetSize();
    Window* pWin = ((ScTabViewShell*)this)->GetActiveWin();
    Size aLogicSize = pWin->PixelToLogic( aPixelSize );

    const ScViewData* pViewData = GetViewData();
    ScDocument* pDoc = pViewData->GetDocument();
    ScSplitPos ePos = pViewData->GetActivePart();
    SCCOL nCol = pViewData->GetPosX(WhichH(ePos));
    SCROW nRow = pViewData->GetPosY(WhichV(ePos));
    SCTAB nTab = pViewData->GetTabNo();
    bool bNegativePage = pDoc->IsNegativePage( nTab );

    Rectangle aLogicRect = pDoc->GetMMRect( nCol, nRow, nCol, nRow, nTab );
    if ( bNegativePage )
    {
        
        aLogicRect.Left() = aLogicRect.Right() - aLogicSize.Width() + 1;    
    }
    aLogicRect.SetSize( aLogicSize );

    pViewData->GetDocShell()->SnapVisArea( aLogicRect );

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
/*!!!   ShowCursor wird nicht paarweise wie im gridwin gerufen.
        Der CursorLockCount am Gridwin muss hier direkt auf 0 gesetzt werden

    if (bOn)
        ShowAllCursors();
    else
        HideAllCursors();
*/
}



void ScTabViewShell::WriteUserData(OUString& rData, bool /* bBrowse */)
{
    GetViewData()->WriteUserData(rData);
}

void ScTabViewShell::WriteUserDataSequence (uno::Sequence < beans::PropertyValue >& rSettings, sal_Bool /* bBrowse */ )
{
    GetViewData()->WriteUserDataSequence (rSettings);
}

void ScTabViewShell::ReadUserData(const OUString& rData, bool /* bBrowse */)
{
    if ( !GetViewData()->GetDocShell()->IsPreview() )
        DoReadUserData( rData );
}

void ScTabViewShell::ReadUserDataSequence (const uno::Sequence < beans::PropertyValue >& rSettings, sal_Bool /* bBrowse */ )
{
    if ( !GetViewData()->GetDocShell()->IsPreview() )
        DoReadUserDataSequence( rSettings );
}

void ScTabViewShell::DoReadUserDataSequence( const uno::Sequence < beans::PropertyValue >& rSettings )
{
    Window* pOldWin = GetActiveWin();
    bool bFocus = pOldWin && pOldWin->HasFocus();

    GetViewData()->ReadUserDataSequence(rSettings);
    SetTabNo( GetViewData()->GetTabNo(), true );

    if ( GetViewData()->IsPagebreakMode() )
        SetCurSubShell( GetCurObjectSelectionType(), true );

    Window* pNewWin = GetActiveWin();
    if (pNewWin && pNewWin != pOldWin)
    {
        SetWindow( pNewWin );       
        if (bFocus)
            pNewWin->GrabFocus();
        WindowChanged();            
    }

    if (GetViewData()->GetHSplitMode() == SC_SPLIT_FIX ||
        GetViewData()->GetVSplitMode() == SC_SPLIT_FIX)
    {
        InvalidateSplit();
    }

    ZoomChanged();

    TestHintWindow();

    
}



void ScTabViewShell::DoReadUserData( const OUString& rData )
{
    Window* pOldWin = GetActiveWin();
    bool bFocus = pOldWin && pOldWin->HasFocus();

    GetViewData()->ReadUserData(rData);
    SetTabNo( GetViewData()->GetTabNo(), true );

    if ( GetViewData()->IsPagebreakMode() )
        SetCurSubShell( GetCurObjectSelectionType(), true );

    Window* pNewWin = GetActiveWin();
    if (pNewWin && pNewWin != pOldWin)
    {
        SetWindow( pNewWin );       
        if (bFocus)
            pNewWin->GrabFocus();
        WindowChanged();            
    }

    if (GetViewData()->GetHSplitMode() == SC_SPLIT_FIX ||
        GetViewData()->GetVSplitMode() == SC_SPLIT_FIX)
    {
        InvalidateSplit();
    }

    ZoomChanged();

    TestHintWindow();

    
}



void ScTabViewShell::UpdateDrawShell()
{
    
    

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
        ResetDrawDragMode();        

        if (bWasDraw && (GetViewData()->GetHSplitMode() == SC_SPLIT_FIX ||
                         GetViewData()->GetVSplitMode() == SC_SPLIT_FIX))
        {
            
            MoveCursorAbs( GetViewData()->GetCurX(), GetViewData()->GetCurY(),
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
            pEditShell = new ScEditShell( pView, GetViewData() );

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
    ScViewData* pViewData   = GetViewData();
    ScDocShell* pDocSh      = pViewData->GetDocShell();

    if(bDontSwitch) return;

    if(!pCellShell) 
    {
        pCellShell = new ScCellShell( GetViewData() );
        pCellShell->SetRepeatTarget( &aTarget );
    }

    bool bPgBrk=pViewData->IsPagebreakMode();

    if(bPgBrk && !pPageBreakShell)
    {
        pPageBreakShell = new ScPageBreakShell( this );
        pPageBreakShell->SetRepeatTarget( &aTarget );
    }


    if ( eOST!=eCurOST || bForce )
    {
        bool bCellBrush = false;    
        bool bDrawBrush = false;    

        if(eCurOST!=OST_NONE) RemoveSubShell();

        if (pFormShell && !bFormShellAtTop)
            AddSubShell(*pFormShell);               

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
                            pDrawTextShell = new ScDrawTextObjectBar( GetViewData() );
                        }
                        AddSubShell(*pDrawTextShell);
                    }
                    break;
            case    OST_Drawing:
                    {
                        if (svx::checkForSelectedCustomShapes(
                                GetScDrawView(), true /* bOnlyExtruded */ )) {
                            if (pExtrusionBarShell == 0)
                                pExtrusionBarShell = new svx::ExtrusionBar(this);
                            AddSubShell( *pExtrusionBarShell );
                        }
                        sal_uInt32 nCheckStatus = 0;
                        if (svx::checkForSelectedFontWork(
                                GetScDrawView(), nCheckStatus )) {
                            if (pFontworkBarShell == 0)
                                pFontworkBarShell = new svx::FontworkBar(this);
                            AddSubShell( *pFontworkBarShell );
                        }

                        if ( !pDrawShell )
                        {
                            pDocSh->MakeDrawLayer();
                            pDrawShell = new ScDrawShell( GetViewData() );
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
                            pDrawFormShell = new ScDrawFormShell( GetViewData() );
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
                            pChartShell = new ScChartShell( GetViewData() );
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
                            pOleObjectShell = new ScOleObjectShell( GetViewData() );
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
                            pGraphicShell = new ScGraphicShell( GetViewData() );
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
                            pMediaShell = new ScMediaShell( GetViewData() );
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
                            pDocSh->MakeDrawLayer();    

                            pAuditingShell = new ScAuditingShell( GetViewData() );
                            pAuditingShell->SetRepeatTarget( &aTarget );
                        }
                        AddSubShell(*pAuditingShell);
                        bCellBrush = true;
                    }
                    break;
            default:
                    OSL_FAIL("Falsche Shell angefordert");
                    break;
        }

        if (pFormShell && bFormShellAtTop)
            AddSubShell(*pFormShell);               

        eCurOST=eOST;

        
        if ( ( GetBrushDocument() && !bCellBrush ) || ( GetDrawBrushSet() && !bDrawBrush ) )
            ResetBrushDocument();
    }
}

void ScTabViewShell::SetFormShellAtTop( bool bSet )
{
    if ( pFormShell && !bSet )
        pFormShell->ForgetActiveControl();      

    if ( bFormShellAtTop != bSet )
    {
        bFormShellAtTop = bSet;
        SetCurSubShell( GetCurObjectSelectionType(), true );
    }
}

IMPL_LINK_NOARG(ScTabViewShell, FormControlActivated)
{
    
    SetFormShellAtTop( true );
    return 0;
}

ObjectSelectionType ScTabViewShell::GetCurObjectSelectionType()
{
    return eCurOST;
}




SfxShell* ScTabViewShell::GetMySubShell() const
{
    

    sal_uInt16 nPos = 0;
    SfxShell* pSub = ((ScTabViewShell*)this)->GetSubShell(nPos);
    while (pSub)
    {
        if ( pSub == pDrawShell  || pSub == pDrawTextShell || pSub == pEditShell ||
             pSub == pPivotShell || pSub == pAuditingShell || pSub == pDrawFormShell ||
             pSub == pCellShell  || pSub == pOleObjectShell|| pSub == pChartShell ||
             pSub == pGraphicShell || pSub == pMediaShell || pSub == pPageBreakShell)
            return pSub;    

        pSub = ((ScTabViewShell*)this)->GetSubShell(++nPos);
    }
    return NULL;        
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
    
    if (!pNewUndoMgr)
        pNewUndoMgr = GetViewData()->GetDocShell()->GetUndoManager();

    if (pDrawTextShell)
    {
        pDrawTextShell->SetUndoManager(pNewUndoMgr);
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        if ( pNewUndoMgr == pDocSh->GetUndoManager() &&
             !pDocSh->GetDocument()->IsUndoEnabled() )
        {
            pNewUndoMgr->SetMaxUndoActionCount( 0 );
        }
    }
    else
    {
        OSL_FAIL("SetDrawTextUndo ohne DrawTextShell");
    }
}



ScTabViewShell* ScTabViewShell::GetActiveViewShell()
{
    return PTR_CAST(ScTabViewShell,Current());
}



SfxPrinter* ScTabViewShell::GetPrinter( sal_Bool bCreate )
{
    
    return GetViewData()->GetDocShell()->GetPrinter(bCreate);
}

sal_uInt16 ScTabViewShell::SetPrinter( SfxPrinter *pNewPrinter, sal_uInt16 nDiffFlags, bool )
{
    return GetViewData()->GetDocShell()->SetPrinter( pNewPrinter, nDiffFlags );
}

bool ScTabViewShell::HasPrintOptionsPage() const
{
    return true;
}

SfxTabPage* ScTabViewShell::CreatePrintOptionsPage( Window *pParent, const SfxItemSet &rOptions )
{
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");
    ::CreateTabPage ScTpPrintOptionsCreate =    pFact->GetTabPageCreatorFunc( RID_SCPAGE_PRINT );
    if ( ScTpPrintOptionsCreate )
        return  (*ScTpPrintOptionsCreate)( pParent, rOptions);
    return 0;
}

void ScTabViewShell::StopEditShell()
{
    if ( pEditShell != NULL && !bDontSwitch )
        SetEditShell(NULL, false );
}





IMPL_LINK_NOARG(ScTabViewShell, SimpleRefClose)
{
    SfxInPlaceClient* pClient = GetIPClient();
    if ( pClient && pClient->IsObjectInPlaceActive() )
    {
        
        

        SetTabNo( GetViewData()->GetRefTabNo() );
    }

    ScSimpleRefDlgWrapper::SetAutoReOpen( true );
    return 0;
}



static ScTabViewObj* lcl_GetViewObj( ScTabViewShell& rShell )
{
    ScTabViewObj* pRet = NULL;
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

IMPL_LINK( ScTabViewShell, SimpleRefDone, OUString*, pResult )
{
    ScTabViewObj* pImpObj = lcl_GetViewObj( *this );
    if ( pImpObj && pResult )
        pImpObj->RangeSelDone( *pResult );
    return 0;
}

IMPL_LINK( ScTabViewShell, SimpleRefAborted, OUString*, pResult )
{
    ScTabViewObj* pImpObj = lcl_GetViewObj( *this );
    if ( pImpObj && pResult )
        pImpObj->RangeSelAborted( *pResult );
    return 0;
}

IMPL_LINK( ScTabViewShell, SimpleRefChange, OUString*, pResult )
{
    ScTabViewObj* pImpObj = lcl_GetViewObj( *this );
    if ( pImpObj && pResult )
        pImpObj->RangeSelChanged( *pResult );
    return 0;
}

void ScTabViewShell::StartSimpleRefDialog(
            const OUString& rTitle, const OUString& rInitVal,
            bool bCloseOnButtonUp, bool bSingleCell, bool bMultiSelection )
{
    SfxViewFrame* pViewFrm = GetViewFrame();

    if ( GetActiveViewShell() != this )
    {
        
        
        

        pViewFrm->GetFrame().Appear();
    }

    sal_uInt16 nId = ScSimpleRefDlgWrapper::GetChildWindowId();

    SC_MOD()->SetRefDialog( nId, true, pViewFrm );

    ScSimpleRefDlgWrapper* pWnd = (ScSimpleRefDlgWrapper*)pViewFrm->GetChildWindow( nId );
    if (pWnd)
    {
        pWnd->SetCloseHdl( LINK( this, ScTabViewShell, SimpleRefClose ) );
        pWnd->SetUnoLinks( LINK( this, ScTabViewShell, SimpleRefDone ),
                           LINK( this, ScTabViewShell, SimpleRefAborted ),
                           LINK( this, ScTabViewShell, SimpleRefChange ) );
        pWnd->SetRefString( rInitVal );
        pWnd->SetFlags( bCloseOnButtonUp, bSingleCell, bMultiSelection );
        pWnd->SetAutoReOpen( false );
        Window* pWin = pWnd->GetWindow();
        pWin->SetText( rTitle );
        pWnd->StartRefInput();
    }
}

void ScTabViewShell::StopSimpleRefDialog()
{
    SfxViewFrame* pViewFrm = GetViewFrame();
    sal_uInt16 nId = ScSimpleRefDlgWrapper::GetChildWindowId();

    ScSimpleRefDlgWrapper* pWnd = (ScSimpleRefDlgWrapper*)pViewFrm->GetChildWindow( nId );
    if (pWnd)
    {
        Window* pWin = pWnd->GetWindow();
        if (pWin && pWin->IsSystemWindow())
            ((SystemWindow*)pWin)->Close();     
    }
}



bool ScTabViewShell::TabKeyInput(const KeyEvent& rKEvt)
{
    ScModule* pScMod = SC_MOD();

    SfxViewFrame* pThisFrame = GetViewFrame();
    if ( pThisFrame->GetChildWindow( SID_OPENDLG_FUNCTION ) )
        return false;

    KeyCode aCode   = rKEvt.GetKeyCode();
    bool bShift     = aCode.IsShift();
    bool bControl   = aCode.IsMod1();
    bool bAlt       = aCode.IsMod2();
    sal_uInt16 nCode    = aCode.GetCode();
    bool bUsed      = false;
    bool bInPlace   = pScMod->IsEditMode();     
    bool bAnyEdit   = pScMod->IsInputMode();    
    bool bDraw      = IsDrawTextEdit();

    HideNoteMarker();   

    
    bool bOnRefSheet = ( GetViewData()->GetRefTabNo() == GetViewData()->GetTabNo() );
    bool bHideCursor = ( ( nCode == KEY_RETURN && bInPlace ) || nCode == KEY_TAB ) && bOnRefSheet;

    if (bHideCursor)
        HideAllCursors();

    ScDocument* pDoc = GetViewData()->GetDocument();
    if ( pDoc )
        pDoc->KeyInput( rKEvt );    

    if( bInPlace )
    {
        bUsed = pScMod->InputKeyEvent( rKEvt );         
        if( !bUsed )
            bUsed = SfxViewShell::KeyInput( rKEvt );    
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
                    bIsType = bControl && !bAlt;        
                    if ( !bIsType && nModi == 0 )
                    {
                        

                        ScInputHandler* pHdl = pScMod->GetInputHdl(this);
                        bIsType = pHdl && pHdl->TakesReturn();
                    }
                    break;
                case KEY_SPACE:
                    bIsType = !bControl && !bAlt;       
                    break;
                case KEY_ESCAPE:
                    bIsType = (nModi == 0); 
                    break;
                default:
                    bIsType = true;
            }

        if( bIsType )
            bUsed = pScMod->InputKeyEvent( rKEvt );     

        if( !bUsed )
            bUsed = SfxViewShell::KeyInput( rKEvt );    

        if ( !bUsed && !bIsType && nCode != KEY_RETURN )    
            bUsed = pScMod->InputKeyEvent( rKEvt );
    }
    else
    {
        
        
        KeyFuncType eFunc = aCode.GetFunction();
        if ( eFunc == KEYFUNC_CUT )
        {
            ScRange aDummy;
            ScMarkType eMarkType = GetViewData()->GetSimpleArea( aDummy );
            if ( eMarkType != SC_MARK_SIMPLE &&
                    !(eFunc == KEYFUNC_COPY && eMarkType == SC_MARK_SIMPLE_FILTERED) )
            {
                ErrorMessage(STR_NOMULTISELECT);
                bUsed = true;
            }
        }
        if (!bUsed)
            bUsed = SfxViewShell::KeyInput( rKEvt );    

        
        
        
        
        bool bParent = ( GetViewFrame()->GetFrame().IsInPlace() && eFunc != KEYFUNC_DONTKNOW );

        if( !bUsed && !bDraw && nCode != KEY_RETURN && !bParent )
            bUsed = pScMod->InputKeyEvent( rKEvt, true );       
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
                        

                        const ScInputOptions& rOpt = pScMod->GetInputOptions();
                        if ( rOpt.GetEnterEdit() )
                        {
                            pScMod->SetInputMode( SC_INPUT_TABLE );
                            bUsed = true;
                        }
                    }

                    bool bEditReturn = bControl && !bShift;         
                    if ( !bUsed && !bEditReturn )
                    {
                        if ( bOnRefSheet )
                            HideAllCursors();

                        sal_uInt8 nMode = SC_ENTER_NORMAL;
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
                            UpdateInputHandler(true);

                        if ( bOnRefSheet )
                            ShowAllCursors();

                        
                        
                        

                        bUsed = true;
                    }
                }
                break;
        }
    }

    

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
            GetViewData()->GetDispatcher().Execute( nSlotId, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD );
            bUsed = true;
        }
    }

    
    
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
            GetViewData()->GetDispatcher().Execute( nSlotId, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD );
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



#define __INIT_ScTabViewShell \
    eCurOST(OST_NONE),          \
    nDrawSfxId(0),              \
    nCtrlSfxId(USHRT_MAX),      \
    nFormSfxId(USHRT_MAX),      \
    pDrawShell(NULL),           \
    pDrawTextShell(NULL),       \
    pEditShell(NULL),           \
    pPivotShell(NULL),          \
    pAuditingShell(NULL),       \
    pDrawFormShell(NULL),       \
    pCellShell(NULL),           \
    pOleObjectShell(NULL),      \
    pChartShell(NULL),          \
    pGraphicShell(NULL),        \
    pMediaShell(NULL),          \
    pPageBreakShell(NULL),      \
    pExtrusionBarShell(NULL),   \
    pFontworkBarShell(NULL),    \
    pFormShell(NULL),           \
    pInputHandler(NULL),        \
    pCurFrameLine(NULL),        \
    aTarget( this ),            \
    pDialogDPObject(NULL),      \
    pNavSettings(NULL),         \
    bActiveDrawSh(false),       \
    bActiveDrawTextSh(false),   \
    bActivePivotSh(false),      \
    bActiveAuditingSh(false),   \
    bActiveDrawFormSh(false),   \
    bActiveOleObjectSh(false),  \
    bActiveChartSh(false),      \
    bActiveGraphicSh(false),    \
    bActiveMediaSh(false),      \
    bActiveEditSh(false),       \
    bFormShellAtTop(false),     \
    bDontSwitch(false),         \
    bInFormatDialog(false),     \
    bPrintSelected(false),      \
    bReadOnly(false),           \
    pScSbxObject(NULL),         \
    bChartAreaValid(false),     \
    bForceFocusOnCurCell(false),\
    nCurRefDlgId(0),            \
    pAccessibilityBroadcaster(NULL), \
    mbInSwitch(false)




void ScTabViewShell::Construct( sal_uInt8 nForceDesignMode )
{
    SfxApplication* pSfxApp  = SFX_APP();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    bReadOnly = pDocSh->IsReadOnly();
    bIsActive = false;

    EnableAutoSpell(pDoc->GetDocOptions().IsAutoSpell());

    SetName(OUString("View")); 
    Color aColBlack( COL_BLACK );
    SetPool( &SC_MOD()->GetPool() );
    SetWindow( GetActiveWin() );

    pCurFrameLine   = new ::editeng::SvxBorderLine(&aColBlack, 20,
                            table::BorderLineStyle::SOLID);
    pPivotSource    = new ScArea;
    StartListening(*GetViewData()->GetDocShell(),true);
    StartListening(*GetViewFrame(),true);
    StartListening(*pSfxApp,true);              

    SfxViewFrame* pFirst = SfxViewFrame::GetFirst(pDocSh);
    bool bFirstView = !pFirst
          || (pFirst == GetViewFrame() && !SfxViewFrame::GetNext(*pFirst,pDocSh));

    if ( pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
    {
        
        Rectangle aVisArea = ((const SfxObjectShell*)pDocSh)->GetVisArea();

        SCTAB nVisTab = pDoc->GetVisibleTab();
        if (!pDoc->HasTable(nVisTab))
        {
            nVisTab = 0;
            pDoc->SetVisibleTab(nVisTab);
        }
        SetTabNo( nVisTab );
        bool bNegativePage = pDoc->IsNegativePage( nVisTab );
        
        GetViewData()->SetScreenPos( bNegativePage ? aVisArea.TopRight() : aVisArea.TopLeft() );

        if ( GetViewFrame()->GetFrame().IsInPlace() )                         
        {
            pDocSh->SetInplace( true );             
            if (pDoc->IsEmbedded())
                pDoc->ResetEmbedded();              
        }
        else if ( bFirstView )
        {
            pDocSh->SetInplace( false );
            GetViewData()->RefreshZoom();           
            if (!pDoc->IsEmbedded())
                pDoc->SetEmbedded( pDoc->GetVisibleTab(), aVisArea );                  
        }
    }

    
    
    
    
    
    
    
    
    

    pInputHandler = new ScInputHandler;

    
    
    

            
            
            
    pFormShell = new FmFormShell(this);
    pFormShell->SetControlActivationHandler( LINK( this, ScTabViewShell, FormControlActivated ) );

            
            
    if (pDoc->GetDrawLayer())
        MakeDrawView( nForceDesignMode );
    ViewOptionsHasChanged(false);   

    ::svl::IUndoManager* pMgr = pDocSh->GetUndoManager();
    SetUndoManager( pMgr );
    pFormShell->SetUndoManager( pMgr );
    if ( !pDoc->IsUndoEnabled() )
    {
        pMgr->SetMaxUndoActionCount( 0 );
    }
    SetRepeatTarget( &aTarget );
    pFormShell->SetRepeatTarget( &aTarget );
    SetHelpId( HID_SCSHELL_TABVWSH );

    if ( bFirstView )   
    {
        pDoc->SetDocVisible( true );        
        if ( pDocSh->IsEmpty() )
        {
            
            pDoc->SetLayoutRTL( 0, ScGlobal::IsSystemRTL() );

            
            if ( pDocSh->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
            {
                
                const ScDefaultsOptions& rOpt = SC_MOD()->GetDefaultsOptions();
                SCTAB nInitTabCount = rOpt.GetInitTabCount();

                for (SCTAB i=1; i<nInitTabCount; i++)
                    pDoc->MakeTable(i,false);
            }

            pDocSh->SetEmpty( false );          
        }

        

        
        if ( pDocSh->GetCreateMode() != SFX_CREATE_MODE_INTERNAL &&
             pDocSh->IsUpdateEnabled() )  
        {
            
            bool bLink = pDoc->GetExternalRefManager()->hasExternalData();
            if (!bLink)
            {
                
                SCTAB nTabCount = pDoc->GetTableCount();
                for (SCTAB i=0; i<nTabCount && !bLink; i++)
                    if (pDoc->IsLinked(i))
                        bLink = true;
            }
            if (!bLink)
            {
                const sc::DocumentLinkManager& rMgr = pDoc->GetDocLinkManager();
                if (rMgr.hasDdeLinks() || pDoc->HasAreaLinks())
                    bLink = true;
            }
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

            bool bReImport = false;                             
            ScDBCollection* pDBColl = pDoc->GetDBCollection();
            if ( pDBColl )
            {
                const ScDBCollection::NamedDBs& rDBs = pDBColl->getNamedDBs();
                ScDBCollection::NamedDBs::const_iterator itr = rDBs.begin(), itrEnd = rDBs.end();
                for (; itr != itrEnd; ++itr)
                {
                    if (itr->IsStripData() && itr->HasImportParam() && !itr->HasImportSelection())
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
                                            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
                }
            }
        }
    }

    UpdateAutoFillMark();

    
    xDisProvInterceptor = new ScDispatchProviderInterceptor( this );

    bFirstActivate = true; 

    
    pDocSh->SetUpdateEnabled(false);

    if ( GetViewFrame()->GetFrame().IsInPlace() )
        UpdateHeaderWidth(); 

    SvBorder aBorder;
    GetBorderSize( aBorder, Size() );
    SetBorderPixel( aBorder );
}






ScTabViewShell::ScTabViewShell( SfxViewFrame* pViewFrame,
                                SfxViewShell* pOldSh ) :
    SfxViewShell( pViewFrame, SFX_VIEW_CAN_PRINT | SFX_VIEW_HAS_PRINTOPTIONS ),
    ScDBFunc( &pViewFrame->GetWindow(), (ScDocShell&)*pViewFrame->GetObjectShell(), this ),
    __INIT_ScTabViewShell
{
    const ScAppOptions& rAppOpt = SC_MOD()->GetAppOptions();

    
    
    
    
    

    sal_uInt8 nForceDesignMode = SC_FORCEMODE_NONE;
    if ( pOldSh && pOldSh->ISA( ScPreviewShell ) )
    {
        ScPreviewShell* pPreviewShell = ((ScPreviewShell*)pOldSh);
        nForceDesignMode = pPreviewShell->GetSourceDesignMode();
        ScPreview* p = pPreviewShell->GetPreview();
        if (p)
            GetViewData()->GetMarkData().SetSelectedTabs(p->GetSelectedTabs());
    }

    Construct( nForceDesignMode );

    if ( GetViewData()->GetDocShell()->IsPreview() )
    {
        
        SetZoomType( SVX_ZOOM_WHOLEPAGE, true );    
    }
    else
    {
        Fraction aFract( rAppOpt.GetZoom(), 100 );
        SetZoom( aFract, aFract, true );
        SetZoomType( rAppOpt.GetZoomType(), true );
    }

    
    new ScTabViewObj( this );

    SetCurSubShell(OST_Cell);
    SvBorder aBorder;
    GetBorderSize( aBorder, Size() );
    SetBorderPixel( aBorder );

    
    MakeDrawLayer();
}

#undef __INIT_ScTabViewShell



ScTabViewShell::~ScTabViewShell()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    EndListening(*pDocSh);
    EndListening(*GetViewFrame());
    EndListening(*SFX_APP());           

    SC_MOD()->ViewShellGone(this);

    RemoveSubShell();           
    SetWindow(0);

    
    

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
        pDialogDPObject = NULL;
}



void ScTabViewShell::FillFieldData( ScHeaderFieldData& rData )
{
    ScDocShell* pDocShell = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocShell->GetDocument();
    SCTAB nTab = GetViewData()->GetTabNo();
    OUString aTmp;
    pDoc->GetName(nTab, aTmp);
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

    
}



void ScTabViewShell::SetChartArea( const ScRangeListRef& rSource, const Rectangle& rDest )
{
    bChartAreaValid = true;
    aChartSource    = rSource;
    aChartPos       = rDest;
    nChartDestTab   = GetViewData()->GetTabNo();
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




void ScTabViewShell::ExecTbx( SfxRequest& rReq )
{
    const SfxItemSet* pReqArgs = rReq.GetArgs();
    sal_uInt16 nSlot = rReq.GetSlot();
    const SfxPoolItem* pItem = NULL;
    if ( pReqArgs )
        pReqArgs->GetItemState( nSlot, true, &pItem );

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
            OSL_FAIL("Slot im Wald");
    }
    GetViewFrame()->GetBindings().Invalidate( nSlot );
}

void ScTabViewShell::GetTbxState( SfxItemSet& rSet )
{
    rSet.Put( SfxUInt16Item( SID_TBXCTL_INSERT,   nInsertCtrlState ) );
    rSet.Put( SfxUInt16Item( SID_TBXCTL_INSCELLS, nInsCellsCtrlState ) );

    
    if ( nInsObjCtrlState == SID_DRAW_CHART && !SvtModuleOptions().IsChart() )
        nInsObjCtrlState = SID_INSERT_OBJECT;

    rSet.Put( SfxUInt16Item( SID_TBXCTL_INSOBJ,   nInsObjCtrlState ) );
}

const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & ScTabViewShell::GetForms() const
{
    if( !pFormShell || !pFormShell->GetCurPage() )
    {
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > aRef;
        return aRef;
    }
    return pFormShell->GetCurPage()->GetForms();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
