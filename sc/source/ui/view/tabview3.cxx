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

#include <rangelst.hxx>
#include "scitems.hxx"
#include <editeng/eeitem.hxx>


#include <editeng/brushitem.hxx>
#include <editeng/editview.hxx>
#include <svx/fmshell.hxx>
#include <svx/svdoole2.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/cursor.hxx>

#include "tabview.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "gridwin.hxx"
#include "olinewin.hxx"
#include "colrowba.hxx"
#include "tabcont.hxx"
#include "scmod.hxx"
#include "uiitems.hxx"
#include "sc.hrc"
#include "viewutil.hxx"
#include "editutil.hxx"
#include "inputhdl.hxx"
#include "inputwin.hxx"
#include "validat.hxx"
#include "hintwin.hxx"
#include "inputopt.hxx"
#include "rfindlst.hxx"
#include "hiranges.hxx"
#include "viewuno.hxx"
#include "chartarr.hxx"
#include "anyrefdg.hxx"
#include "dpobject.hxx"
#include "patattr.hxx"
#include "dociter.hxx"
#include "seltrans.hxx"
#include "fillinfo.hxx"
#include "AccessibilityHints.hxx"
#include "rangeutl.hxx"
#include "client.hxx"
#include "tabprotection.hxx"
#include "markdata.hxx"
#include "formula/FormulaCompiler.hxx"

#include <com/sun/star/chart2/data/HighlightedRange.hpp>

namespace
{

ScRange lcl_getSubRangeByIndex( const ScRange& rRange, sal_Int32 nIndex )
{
    ScAddress aResult( rRange.aStart );

    SCCOL nWidth = rRange.aEnd.Col() - rRange.aStart.Col() + 1;
    SCROW nHeight = rRange.aEnd.Row() - rRange.aStart.Row() + 1;
    SCTAB nDepth = rRange.aEnd.Tab() - rRange.aStart.Tab() + 1;
    if( (nWidth > 0) && (nHeight > 0) && (nDepth > 0) )
    {
        
        sal_Int32 nArea = nWidth * nHeight;
        aResult.IncCol( static_cast< SCsCOL >( nIndex % nWidth ) );
        aResult.IncRow( static_cast< SCsROW >( (nIndex % nArea) / nWidth ) );
        aResult.IncTab( static_cast< SCsTAB >( nIndex / nArea ) );
        if( !rRange.In( aResult ) )
            aResult = rRange.aStart;
    }

    return ScRange( aResult );
}

} 

using namespace com::sun::star;



//

//

void ScTabView::ClickCursor( SCCOL nPosX, SCROW nPosY, bool bControl )
{
    ScDocument* pDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();
    pDoc->SkipOverlapped(nPosX, nPosY, nTab);

    bool bRefMode = SC_MOD()->IsFormulaMode();

    if ( bRefMode )
    {
        DoneRefMode( false );

        if (bControl)
            SC_MOD()->AddRefEntry();

        InitRefMode( nPosX, nPosY, nTab, SC_REFTYPE_REF );
    }
    else
    {
        DoneBlockMode( bControl );
        aViewData.ResetOldCursor();
        SetCursor( (SCCOL) nPosX, (SCROW) nPosY );
    }
}

void ScTabView::UpdateAutoFillMark()
{
    
    ScRange aMarkRange;
    bool bMarked = (aViewData.GetSimpleArea( aMarkRange ) == SC_MARK_SIMPLE);

    sal_uInt16 i;
    for (i=0; i<4; i++)
        if (pGridWin[i] && pGridWin[i]->IsVisible())
            pGridWin[i]->UpdateAutoFillMark( bMarked, aMarkRange );

    for (i=0; i<2; i++)
    {
        if (pColBar[i] && pColBar[i]->IsVisible())
            pColBar[i]->SetMark( bMarked, aMarkRange.aStart.Col(), aMarkRange.aEnd.Col() );
        if (pRowBar[i] && pRowBar[i]->IsVisible())
            pRowBar[i]->SetMark( bMarked, aMarkRange.aStart.Row(), aMarkRange.aEnd.Row() );
    }

    
    
    CheckSelectionTransfer();   
}

void ScTabView::FakeButtonUp( ScSplitPos eWhich )
{
    if (pGridWin[eWhich])
        pGridWin[eWhich]->FakeButtonUp();
}

void ScTabView::HideAllCursors()
{
    for (sal_uInt16 i=0; i<4; i++)
        if (pGridWin[i])
            if (pGridWin[i]->IsVisible())
            {
                Cursor* pCur = pGridWin[i]->GetCursor();
                if (pCur)
                    if (pCur->IsVisible())
                        pCur->Hide();
                pGridWin[i]->HideCursor();
            }
}

void ScTabView::ShowAllCursors()
{
    for (sal_uInt16 i=0; i<4; i++)
        if (pGridWin[i])
            if (pGridWin[i]->IsVisible())
            {
                pGridWin[i]->ShowCursor();

                
                pGridWin[i]->CursorChanged();
            }
}

void ScTabView::ShowCursor()
{
    pGridWin[aViewData.GetActivePart()]->ShowCursor();

    
    pGridWin[aViewData.GetActivePart()]->CursorChanged();
}

void ScTabView::InvalidateAttribs()
{
    SfxBindings& rBindings = aViewData.GetBindings();

    rBindings.Invalidate( SID_STYLE_APPLY );
    rBindings.Invalidate( SID_STYLE_FAMILY2 );
    

    rBindings.Invalidate( SID_ATTR_CHAR_FONT );
    rBindings.Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
    rBindings.Invalidate( SID_ATTR_CHAR_COLOR );

    rBindings.Invalidate( SID_ATTR_CHAR_WEIGHT );
    rBindings.Invalidate( SID_ATTR_CHAR_POSTURE );
    rBindings.Invalidate( SID_ATTR_CHAR_UNDERLINE );
    rBindings.Invalidate( SID_ULINE_VAL_NONE );
    rBindings.Invalidate( SID_ULINE_VAL_SINGLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOUBLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOTTED );

    rBindings.Invalidate( SID_ATTR_CHAR_OVERLINE );

    rBindings.Invalidate( SID_ATTR_CHAR_KERNING );
    rBindings.Invalidate( SID_SET_SUPER_SCRIPT );
    rBindings.Invalidate( SID_SET_SUB_SCRIPT );
    rBindings.Invalidate( SID_ATTR_CHAR_STRIKEOUT );
    rBindings.Invalidate( SID_ATTR_CHAR_SHADOWED );

    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_LEFT );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_RIGHT );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_BLOCK );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_CENTER);
    rBindings.Invalidate( SID_NUMBER_TYPE_FORMAT);

    rBindings.Invalidate( SID_ALIGNLEFT );
    rBindings.Invalidate( SID_ALIGNRIGHT );
    rBindings.Invalidate( SID_ALIGNBLOCK );
    rBindings.Invalidate( SID_ALIGNCENTERHOR );

    rBindings.Invalidate( SID_ALIGNTOP );
    rBindings.Invalidate( SID_ALIGNBOTTOM );
    rBindings.Invalidate( SID_ALIGNCENTERVER );

    
    {
        rBindings.Invalidate( SID_H_ALIGNCELL );
        rBindings.Invalidate( SID_V_ALIGNCELL );
        rBindings.Invalidate( SID_ATTR_ALIGN_INDENT );
        rBindings.Invalidate( SID_FRAME_LINECOLOR );
        rBindings.Invalidate( SID_FRAME_LINESTYLE );
        rBindings.Invalidate( SID_ATTR_BORDER_OUTER );
        rBindings.Invalidate( SID_ATTR_BORDER_INNER );
        rBindings.Invalidate( SID_SCGRIDSHOW );
        rBindings.Invalidate( SID_ATTR_BORDER_DIAG_TLBR );
        rBindings.Invalidate( SID_ATTR_BORDER_DIAG_BLTR );
        rBindings.Invalidate( SID_NUMBER_TYPE_FORMAT );
    }

    rBindings.Invalidate( SID_BACKGROUND_COLOR );

    rBindings.Invalidate( SID_ATTR_ALIGN_LINEBREAK );
    rBindings.Invalidate( SID_NUMBER_FORMAT );

    rBindings.Invalidate( SID_TEXTDIRECTION_LEFT_TO_RIGHT );
    rBindings.Invalidate( SID_TEXTDIRECTION_TOP_TO_BOTTOM );
    rBindings.Invalidate( SID_ATTR_PARA_LEFT_TO_RIGHT );
    rBindings.Invalidate( SID_ATTR_PARA_RIGHT_TO_LEFT );

    
    rBindings.Invalidate( SID_ALIGN_ANY_HDEFAULT );
    rBindings.Invalidate( SID_ALIGN_ANY_LEFT );
    rBindings.Invalidate( SID_ALIGN_ANY_HCENTER );
    rBindings.Invalidate( SID_ALIGN_ANY_RIGHT );
    rBindings.Invalidate( SID_ALIGN_ANY_JUSTIFIED );
    rBindings.Invalidate( SID_ALIGN_ANY_VDEFAULT );
    rBindings.Invalidate( SID_ALIGN_ANY_TOP );
    rBindings.Invalidate( SID_ALIGN_ANY_VCENTER );
    rBindings.Invalidate( SID_ALIGN_ANY_BOTTOM );

    rBindings.Invalidate( SID_NUMBER_CURRENCY );
    rBindings.Invalidate( SID_NUMBER_SCIENTIFIC );
    rBindings.Invalidate( SID_NUMBER_DATE );
    rBindings.Invalidate( SID_NUMBER_CURRENCY );
    rBindings.Invalidate( SID_NUMBER_PERCENT );
    rBindings.Invalidate( SID_NUMBER_TIME );
}





void ScTabView::SetCursor( SCCOL nPosX, SCROW nPosY, bool bNew )
{
    SCCOL nOldX = aViewData.GetCurX();
    SCROW nOldY = aViewData.GetCurY();

    

    if ( nPosX != nOldX || nPosY != nOldY || bNew )
    {
        ScTabViewShell* pViewShell = aViewData.GetViewShell();
        bool bRefMode = ( pViewShell ? pViewShell->IsRefInputMode() : false );
        if ( aViewData.HasEditView( aViewData.GetActivePart() ) && !bRefMode ) 
        {
            UpdateInputLine();
        }

        HideAllCursors();

        aViewData.SetCurX( nPosX );
        aViewData.SetCurY( nPosY );

        ShowAllCursors();

        CursorPosChanged();
    }
}

void ScTabView::CheckSelectionTransfer()
{
    if ( aViewData.IsActive() )     
    {
        ScModule* pScMod = SC_MOD();
        ScSelectionTransferObj* pOld = pScMod->GetSelectionTransfer();
        if ( pOld && pOld->GetView() == this && pOld->StillValid() )
        {
            
        }
        else
        {
            ScSelectionTransferObj* pNew = ScSelectionTransferObj::CreateFromView( this );
            if ( pNew )
            {
                

                if (pOld)
                    pOld->ForgetView();

                uno::Reference<datatransfer::XTransferable> xRef( pNew );
                pScMod->SetSelectionTransfer( pNew );
                pNew->CopyToSelection( GetActiveWin() );                    
            }
            else if ( pOld && pOld->GetView() == this )
            {
                

                pOld->ForgetView();
                pScMod->SetSelectionTransfer( NULL );
                TransferableHelper::ClearSelection( GetActiveWin() );       
            }
            
        }
    }
}





void ScTabView::CellContentChanged()
{
    SfxBindings& rBindings = aViewData.GetBindings();

    rBindings.Invalidate( SID_ATTR_SIZE );      
    rBindings.Invalidate( SID_THESAURUS );
    rBindings.Invalidate( SID_HYPERLINK_GETLINK );

    InvalidateAttribs();                    

    aViewData.GetViewShell()->UpdateInputHandler();
}

void ScTabView::SelectionChanged()
{
    SfxViewFrame* pViewFrame = aViewData.GetViewShell()->GetViewFrame();
    if (pViewFrame)
    {
        uno::Reference<frame::XController> xController = pViewFrame->GetFrame().GetController();
        if (xController.is())
        {
            ScTabViewObj* pImp = ScTabViewObj::getImplementation( xController );
            if (pImp)
                pImp->SelectionChanged();
        }
    }

    UpdateAutoFillMark();   

    SfxBindings& rBindings = aViewData.GetBindings();

    rBindings.Invalidate( SID_CURRENTCELL );    
    rBindings.Invalidate( SID_AUTO_FILTER );    
    rBindings.Invalidate( FID_NOTE_VISIBLE );
    rBindings.Invalidate( FID_SHOW_NOTE );
    rBindings.Invalidate( FID_HIDE_NOTE );
    rBindings.Invalidate( SID_DELETE_NOTE );

        

    rBindings.Invalidate( FID_INS_ROWBRK );
    rBindings.Invalidate( FID_INS_COLBRK );
    rBindings.Invalidate( FID_DEL_ROWBRK );
    rBindings.Invalidate( FID_DEL_COLBRK );
    rBindings.Invalidate( FID_MERGE_ON );
    rBindings.Invalidate( FID_MERGE_OFF );
    rBindings.Invalidate( FID_MERGE_TOGGLE );
    rBindings.Invalidate( SID_AUTOFILTER_HIDE );
    rBindings.Invalidate( SID_UNFILTER );
    rBindings.Invalidate( SID_REIMPORT_DATA );
    rBindings.Invalidate( SID_REFRESH_DBAREA );
    rBindings.Invalidate( SID_OUTLINE_SHOW );
    rBindings.Invalidate( SID_OUTLINE_HIDE );
    rBindings.Invalidate( SID_OUTLINE_REMOVE );
    rBindings.Invalidate( FID_FILL_TO_BOTTOM );
    rBindings.Invalidate( FID_FILL_TO_RIGHT );
    rBindings.Invalidate( FID_FILL_TO_TOP );
    rBindings.Invalidate( FID_FILL_TO_LEFT );
    rBindings.Invalidate( FID_FILL_SERIES );
    rBindings.Invalidate( SID_SCENARIOS );
    rBindings.Invalidate( SID_AUTOFORMAT );
    rBindings.Invalidate( SID_OPENDLG_TABOP );
    rBindings.Invalidate( SID_DATA_SELECT );

    rBindings.Invalidate( SID_CUT );
    rBindings.Invalidate( SID_COPY );
    rBindings.Invalidate( SID_PASTE );
    rBindings.Invalidate( SID_PASTE_SPECIAL );

    rBindings.Invalidate( FID_INS_ROW );
    rBindings.Invalidate( FID_INS_COLUMN );
    rBindings.Invalidate( FID_INS_CELL );
    rBindings.Invalidate( FID_INS_CELLSDOWN );
    rBindings.Invalidate( FID_INS_CELLSRIGHT );

    rBindings.Invalidate( FID_CHG_COMMENT );

        

    rBindings.Invalidate( SID_CELL_FORMAT_RESET );
    rBindings.Invalidate( SID_DELETE );
    rBindings.Invalidate( SID_DELETE_CONTENTS );
    rBindings.Invalidate( FID_DELETE_CELL );
    rBindings.Invalidate( FID_CELL_FORMAT );
    rBindings.Invalidate( SID_ENABLE_HYPHENATION );
    rBindings.Invalidate( SID_INSERT_POSTIT );
    rBindings.Invalidate( SID_CHARMAP );
    rBindings.Invalidate( SID_OPENDLG_FUNCTION );
    rBindings.Invalidate( FID_VALIDATION );
    rBindings.Invalidate( SID_EXTERNAL_SOURCE );
    rBindings.Invalidate( SID_TEXT_TO_COLUMNS );
    rBindings.Invalidate( SID_SORT_ASCENDING );
    rBindings.Invalidate( SID_SORT_DESCENDING );

    if (aViewData.GetViewShell()->HasAccessibilityObjects())
        aViewData.GetViewShell()->BroadcastAccessibility(SfxSimpleHint(SC_HINT_ACC_CURSORCHANGED));

    CellContentChanged();
}

void ScTabView::CursorPosChanged()
{
    bool bRefMode = SC_MOD()->IsFormulaMode();
    if ( !bRefMode ) 
        aViewData.GetDocShell()->Broadcast( SfxSimpleHint( FID_KILLEDITVIEW ) );

    

    ScDocument* pDoc = aViewData.GetDocument();
    bool bDP = NULL != pDoc->GetDPAtCursor(
        aViewData.GetCurX(), aViewData.GetCurY(), aViewData.GetTabNo() );
    aViewData.GetViewShell()->SetPivotShell(bDP);

    

    SelectionChanged();

    aViewData.SetTabStartCol( SC_TABSTART_NONE );
}

namespace {

Point calcHintWindowPosition(
    const Point& rCellPos, const Size& rCellSize, const Size& rFrameWndSize, const Size& rHintWndSize)
{
    const long nMargin = 20;

    long nMLeft = rCellPos.X();
    long nMRight = rFrameWndSize.Width() - rCellPos.X() - rCellSize.Width();
    long nMTop = rCellPos.Y();
    long nMBottom = rFrameWndSize.Height() - rCellPos.Y() - rCellSize.Height();

    

    if (nMRight - nMargin >= rHintWndSize.Width())
    {
        
        if (rFrameWndSize.Height() >= rHintWndSize.Height())
        {
            
            Point aPos = rCellPos;
            aPos.X() += rCellSize.Width() + nMargin;
            if (aPos.Y() + rHintWndSize.Height() > rFrameWndSize.Height())
            {
                
                aPos.Y() = rFrameWndSize.Height() - rHintWndSize.Height();
            }
            return aPos;
        }
    }

    if (nMBottom - nMargin >= rHintWndSize.Height())
    {
        
        if (rFrameWndSize.Width() >= rHintWndSize.Width())
        {
            
            Point aPos = rCellPos;
            aPos.Y() += rCellSize.Height() + nMargin;
            if (aPos.X() + rHintWndSize.Width() > rFrameWndSize.Width())
            {
                
                aPos.X() = rFrameWndSize.Width() - rHintWndSize.Width();
            }
            return aPos;
        }
    }

    if (nMLeft - nMargin >= rHintWndSize.Width())
    {
        
        if (rFrameWndSize.Height() >= rHintWndSize.Height())
        {
            
            Point aPos = rCellPos;
            aPos.X() -= rHintWndSize.Width() + nMargin;
            if (aPos.Y() + rHintWndSize.Height() > rFrameWndSize.Height())
            {
                
                aPos.Y() = rFrameWndSize.Height() - rHintWndSize.Height();
            }
            return aPos;
        }
    }

    if (nMTop - nMargin >= rHintWndSize.Height())
    {
        
        if (rFrameWndSize.Width() >= rHintWndSize.Width())
        {
            
            Point aPos = rCellPos;
            aPos.Y() -= rHintWndSize.Height() + nMargin;
            if (aPos.X() + rHintWndSize.Width() > rFrameWndSize.Width())
            {
                
                aPos.X() = rFrameWndSize.Width() - rHintWndSize.Width();
            }
            return aPos;
        }
    }

    

    if (nMRight - nMargin >= rHintWndSize.Width())
    {
        
        Point aPos = rCellPos;
        aPos.X() += nMargin + rCellSize.Width();
        aPos.Y() = 0;
        return aPos;
    }

    if (nMBottom - nMargin >= rHintWndSize.Height())
    {
        
        Point aPos = rCellPos;
        aPos.Y() += nMargin + rCellSize.Height();
        aPos.X() = 0;
        return aPos;
    }

    if (nMLeft - nMargin >= rHintWndSize.Width())
    {
        
        Point aPos = rCellPos;
        aPos.X() -= rHintWndSize.Width() + nMargin;
        aPos.Y() = 0;
        return aPos;
    }

    if (nMTop - nMargin >= rHintWndSize.Height())
    {
        
        Point aPos = rCellPos;
        aPos.Y() -= rHintWndSize.Height() + nMargin;
        aPos.X() = 0;
        return aPos;
    }

    
    
    Point aPos = rCellPos;
    aPos.X() += rCellSize.Width();
    return aPos;
}

}

void ScTabView::TestHintWindow()
{
    

    bool bListValButton = false;
    ScAddress aListValPos;

    ScDocument* pDoc = aViewData.GetDocument();
    const SfxUInt32Item* pItem = (const SfxUInt32Item*)
                                        pDoc->GetAttr( aViewData.GetCurX(),
                                                       aViewData.GetCurY(),
                                                       aViewData.GetTabNo(),
                                                       ATTR_VALIDDATA );
    if ( pItem->GetValue() )
    {
        const ScValidationData* pData = pDoc->GetValidationEntry( pItem->GetValue() );
        OSL_ENSURE(pData,"ValidationData nicht gefunden");
        OUString aTitle, aMessage;
        if ( pData && pData->GetInput( aTitle, aMessage ) && !aMessage.isEmpty() )
        {
            

            mpInputHintWindow.reset();

            ScSplitPos eWhich = aViewData.GetActivePart();
            ScGridWindow* pWin = pGridWin[eWhich];
            SCCOL nCol = aViewData.GetCurX();
            SCROW nRow = aViewData.GetCurY();
            Point aPos = aViewData.GetScrPos( nCol, nRow, eWhich );
            Size aWinSize = pWin->GetOutputSizePixel();
            
            if ( nCol >= aViewData.GetPosX(WhichH(eWhich)) &&
                 nRow >= aViewData.GetPosY(WhichV(eWhich)) &&
                 aPos.X() < aWinSize.Width() && aPos.Y() < aWinSize.Height() )
            {
                
                mpInputHintWindow.reset(new ScHintWindow(pWin, aTitle, aMessage));
                Size aHintWndSize = mpInputHintWindow->GetSizePixel();
                long nCellSizeX = 0;
                long nCellSizeY = 0;
                aViewData.GetMergeSizePixel(nCol, nRow, nCellSizeX, nCellSizeY);

                Point aHintPos = calcHintWindowPosition(
                    aPos, Size(nCellSizeX,nCellSizeY), aWinSize, aHintWndSize);

                mpInputHintWindow->SetPosPixel( aHintPos );
                mpInputHintWindow->ToTop();
                mpInputHintWindow->Show();
            }
        }
        else
            mpInputHintWindow.reset();

        
        if ( pData && pData->HasSelectionList() )
        {
            aListValPos.Set( aViewData.GetCurX(), aViewData.GetCurY(), aViewData.GetTabNo() );
            bListValButton = true;
        }
    }
    else
        mpInputHintWindow.reset();

    for ( sal_uInt16 i=0; i<4; i++ )
        if ( pGridWin[i] && pGridWin[i]->IsVisible() )
            pGridWin[i]->UpdateListValPos( bListValButton, aListValPos );
}

bool ScTabView::HasHintWindow() const
{
    return mpInputHintWindow.get() != NULL;
}

void ScTabView::RemoveHintWindow()
{
    mpInputHintWindow.reset();
}



static Window* lcl_GetCareWin(SfxViewFrame* pViewFrm)
{
    

    
    if ( pViewFrm->HasChildWindow(SID_SEARCH_DLG) )
    {
        SfxChildWindow* pChild = pViewFrm->GetChildWindow(SID_SEARCH_DLG);
        if (pChild)
        {
            Window* pWin = pChild->GetWindow();
            if (pWin && pWin->IsVisible())
                return pWin;
        }
    }

    
    if ( pViewFrm->HasChildWindow(FID_CHG_ACCEPT) )
    {
        SfxChildWindow* pChild = pViewFrm->GetChildWindow(FID_CHG_ACCEPT);
        if (pChild)
        {
            Window* pWin = pChild->GetWindow();
            if (pWin && pWin->IsVisible())
                return pWin;
        }
    }

    return NULL;
}

    //
    
    //

void ScTabView::AlignToCursor( SCsCOL nCurX, SCsROW nCurY, ScFollowMode eMode,
                                const ScSplitPos* pWhich )
{
    //
    
    //

    ScSplitPos eActive = aViewData.GetActivePart();
    ScHSplitPos eActiveX = WhichH(eActive);
    ScVSplitPos eActiveY = WhichV(eActive);
    bool bHFix = (aViewData.GetHSplitMode() == SC_SPLIT_FIX);
    bool bVFix = (aViewData.GetVSplitMode() == SC_SPLIT_FIX);
    if (bHFix)
        if (eActiveX == SC_SPLIT_LEFT && nCurX >= (SCsCOL)aViewData.GetFixPosX())
        {
            ActivatePart( (eActiveY==SC_SPLIT_TOP) ? SC_SPLIT_TOPRIGHT : SC_SPLIT_BOTTOMRIGHT );
            eActiveX = SC_SPLIT_RIGHT;
        }
    if (bVFix)
        if (eActiveY == SC_SPLIT_TOP && nCurY >= (SCsROW)aViewData.GetFixPosY())
        {
            ActivatePart( (eActiveX==SC_SPLIT_LEFT) ? SC_SPLIT_BOTTOMLEFT : SC_SPLIT_BOTTOMRIGHT );
            eActiveY = SC_SPLIT_BOTTOM;
        }

    //
    
    //

    if ( eMode != SC_FOLLOW_NONE )
    {
        ScSplitPos eAlign;
        if (pWhich)
            eAlign = *pWhich;
        else
            eAlign = aViewData.GetActivePart();
        ScHSplitPos eAlignX = WhichH(eAlign);
        ScVSplitPos eAlignY = WhichV(eAlign);

        SCsCOL nDeltaX = (SCsCOL) aViewData.GetPosX(eAlignX);
        SCsROW nDeltaY = (SCsROW) aViewData.GetPosY(eAlignY);
        SCsCOL nSizeX = (SCsCOL) aViewData.VisibleCellsX(eAlignX);
        SCsROW nSizeY = (SCsROW) aViewData.VisibleCellsY(eAlignY);

        long nCellSizeX;
        long nCellSizeY;
        if ( nCurX >= 0 && nCurY >= 0 )
            aViewData.GetMergeSizePixel( (SCCOL)nCurX, (SCROW)nCurY, nCellSizeX, nCellSizeY );
        else
            nCellSizeX = nCellSizeY = 0;
        Size aScrSize = aViewData.GetScrSize();
        long nSpaceX = ( aScrSize.Width()  - nCellSizeX ) / 2;
        long nSpaceY = ( aScrSize.Height() - nCellSizeY ) / 2;
        

        bool bForceNew = false;     

        

        
        
        

        

        if ( eMode == SC_FOLLOW_JUMP )
        {
            Window* pCare = lcl_GetCareWin( aViewData.GetViewShell()->GetViewFrame() );
            if (pCare)
            {
                bool bLimit = false;
                Rectangle aDlgPixel;
                Size aWinSize;
                Window* pWin = GetActiveWin();
                if (pWin)
                {
                    aDlgPixel = pCare->GetWindowExtentsRelative( pWin );
                    aWinSize = pWin->GetOutputSizePixel();
                    
                    if ( aDlgPixel.Right() >= 0 && aDlgPixel.Left() < aWinSize.Width() )
                    {
                        if ( nCurX < nDeltaX || nCurX >= nDeltaX+nSizeX ||
                             nCurY < nDeltaY || nCurY >= nDeltaY+nSizeY )
                            bLimit = true;          
                        else
                        {
                            
                            Point aStart = aViewData.GetScrPos( nCurX, nCurY, eAlign );
                            long nCSX, nCSY;
                            aViewData.GetMergeSizePixel( nCurX, nCurY, nCSX, nCSY );
                            Rectangle aCursor( aStart, Size( nCSX, nCSY ) );
                            if ( aCursor.IsOver( aDlgPixel ) )
                                bLimit = true;      
                        }
                    }
                }

                if (bLimit)
                {
                    bool bBottom = false;
                    long nTopSpace = aDlgPixel.Top();
                    long nBotSpace = aWinSize.Height() - aDlgPixel.Bottom();
                    if ( nBotSpace > 0 && nBotSpace > nTopSpace )
                    {
                        long nDlgBot = aDlgPixel.Bottom();
                        SCsCOL nWPosX;
                        SCsROW nWPosY;
                        aViewData.GetPosFromPixel( 0,nDlgBot, eAlign, nWPosX, nWPosY );
                        ++nWPosY;   

                        SCsROW nDiff = nWPosY - nDeltaY;
                        if ( nCurY >= nDiff )           
                        {
                            nSpaceY = nDlgBot + ( nBotSpace - nCellSizeY ) / 2;
                            bBottom = true;
                            bForceNew = true;
                        }
                    }
                    if ( !bBottom && nTopSpace > 0 )
                    {
                        nSpaceY = ( nTopSpace - nCellSizeY ) / 2;
                        bForceNew = true;
                    }
                }
            }
        }
        

        SCsCOL nNewDeltaX = nDeltaX;
        SCsROW nNewDeltaY = nDeltaY;
        bool bDoLine = false;

        switch (eMode)
        {
            case SC_FOLLOW_JUMP:
                if ( nCurX < nDeltaX || nCurX >= nDeltaX+nSizeX )
                {
                    nNewDeltaX = nCurX - static_cast<SCsCOL>(aViewData.CellsAtX( nCurX, -1, eAlignX, static_cast<sal_uInt16>(nSpaceX) ));
                    if (nNewDeltaX < 0) nNewDeltaX = 0;
                    nSizeX = (SCsCOL) aViewData.CellsAtX( nNewDeltaX, 1, eAlignX );
                }
                if ( nCurY < nDeltaY || nCurY >= nDeltaY+nSizeY || bForceNew )
                {
                    nNewDeltaY = nCurY - static_cast<SCsROW>(aViewData.CellsAtY( nCurY, -1, eAlignY, static_cast<sal_uInt16>(nSpaceY) ));
                    if (nNewDeltaY < 0) nNewDeltaY = 0;
                    nSizeY = (SCsROW) aViewData.CellsAtY( nNewDeltaY, 1, eAlignY );
                }
                bDoLine = true;
                break;

            case SC_FOLLOW_LINE:
                bDoLine = true;
                break;

            case SC_FOLLOW_FIX:
                if ( nCurX < nDeltaX || nCurX >= nDeltaX+nSizeX )
                {
                    nNewDeltaX = nDeltaX + nCurX - aViewData.GetCurX();
                    if (nNewDeltaX < 0) nNewDeltaX = 0;
                    nSizeX = (SCsCOL) aViewData.CellsAtX( nNewDeltaX, 1, eAlignX );
                }
                if ( nCurY < nDeltaY || nCurY >= nDeltaY+nSizeY )
                {
                    nNewDeltaY = nDeltaY + nCurY - aViewData.GetCurY();
                    if (nNewDeltaY < 0) nNewDeltaY = 0;
                    nSizeY = (SCsROW) aViewData.CellsAtY( nNewDeltaY, 1, eAlignY );
                }

                

                if ( nCurX < nNewDeltaX || nCurX >= nNewDeltaX+nSizeX )
                {
                    nNewDeltaX = nCurX - (nSizeX / 2);
                    if (nNewDeltaX < 0) nNewDeltaX = 0;
                    nSizeX = (SCsCOL) aViewData.CellsAtX( nNewDeltaX, 1, eAlignX );
                }
                if ( nCurY < nNewDeltaY || nCurY >= nNewDeltaY+nSizeY )
                {
                    nNewDeltaY = nCurY - (nSizeY / 2);
                    if (nNewDeltaY < 0) nNewDeltaY = 0;
                    nSizeY = (SCsROW) aViewData.CellsAtY( nNewDeltaY, 1, eAlignY );
                }

                bDoLine = true;
                break;

            case SC_FOLLOW_NONE:
                break;
            default:
                OSL_FAIL("Falscher Cursormodus");
                break;
        }

        if (bDoLine)
        {
            while ( nCurX >= nNewDeltaX+nSizeX )
            {
                nNewDeltaX = nCurX-nSizeX+1;
                ScDocument* pDoc = aViewData.GetDocument();
                SCTAB nTab = aViewData.GetTabNo();
                while ( nNewDeltaX < MAXCOL && !pDoc->GetColWidth( nNewDeltaX, nTab ) )
                    ++nNewDeltaX;
                nSizeX = (SCsCOL) aViewData.CellsAtX( nNewDeltaX, 1, eAlignX );
            }
            while ( nCurY >= nNewDeltaY+nSizeY )
            {
                nNewDeltaY = nCurY-nSizeY+1;
                ScDocument* pDoc = aViewData.GetDocument();
                SCTAB nTab = aViewData.GetTabNo();
                while ( nNewDeltaY < MAXROW && !pDoc->GetRowHeight( nNewDeltaY, nTab ) )
                    ++nNewDeltaY;
                nSizeY = (SCsROW) aViewData.CellsAtY( nNewDeltaY, 1, eAlignY );
            }
            if ( nCurX < nNewDeltaX ) nNewDeltaX = nCurX;
            if ( nCurY < nNewDeltaY ) nNewDeltaY = nCurY;
        }

        if ( nNewDeltaX != nDeltaX )
            nSizeX = (SCsCOL) aViewData.CellsAtX( nNewDeltaX, 1, eAlignX );
        if (nNewDeltaX+nSizeX-1 > MAXCOL) nNewDeltaX = MAXCOL-nSizeX+1;
        if (nNewDeltaX < 0) nNewDeltaX = 0;

        if ( nNewDeltaY != nDeltaY )
            nSizeY = (SCsROW) aViewData.CellsAtY( nNewDeltaY, 1, eAlignY );
        if (nNewDeltaY+nSizeY-1 > MAXROW) nNewDeltaY = MAXROW-nSizeY+1;
        if (nNewDeltaY < 0) nNewDeltaY = 0;

        if ( nNewDeltaX != nDeltaX ) ScrollX( nNewDeltaX - nDeltaX, eAlignX );
        if ( nNewDeltaY != nDeltaY ) ScrollY( nNewDeltaY - nDeltaY, eAlignY );
    }

    //
    
    //

    if (bHFix)
        if (eActiveX == SC_SPLIT_RIGHT && nCurX < (SCsCOL)aViewData.GetFixPosX())
        {
            ActivatePart( (eActiveY==SC_SPLIT_TOP) ? SC_SPLIT_TOPLEFT : SC_SPLIT_BOTTOMLEFT );
            eActiveX = SC_SPLIT_LEFT;
        }
    if (bVFix)
        if (eActiveY == SC_SPLIT_BOTTOM && nCurY < (SCsROW)aViewData.GetFixPosY())
        {
            ActivatePart( (eActiveX==SC_SPLIT_LEFT) ? SC_SPLIT_TOPLEFT : SC_SPLIT_TOPRIGHT );
            eActiveY = SC_SPLIT_TOP;
        }
}

bool ScTabView::SelMouseButtonDown( const MouseEvent& rMEvt )
{
    bool bRet = false;

    
    bool bMod1Locked = aViewData.GetViewShell()->GetLockedModifiers() & KEY_MOD1 ? true : false;
    aViewData.SetSelCtrlMouseClick( rMEvt.IsMod1() || bMod1Locked );

    if ( pSelEngine )
    {
        bMoveIsShift = rMEvt.IsShift();
        bRet = pSelEngine->SelMouseButtonDown( rMEvt );
        bMoveIsShift = false;
    }

    aViewData.SetSelCtrlMouseClick( false ); 

    return bRet;
}

    //
    
    //

void ScTabView::MoveCursorAbs( SCsCOL nCurX, SCsROW nCurY, ScFollowMode eMode,
                               bool bShift, bool bControl, bool bKeepOld, bool bKeepSel )
{
    if (!bKeepOld)
        aViewData.ResetOldCursor();

    
    if( aViewData.GetViewShell()->GetForceFocusOnCurCell() )
        aViewData.GetViewShell()->SetForceFocusOnCurCell( !ValidColRow(nCurX, nCurY) );

    if (nCurX < 0) nCurX = 0;
    if (nCurY < 0) nCurY = 0;
    if (nCurX > MAXCOL) nCurX = MAXCOL;
    if (nCurY > MAXROW) nCurY = MAXROW;

    HideAllCursors();

        

    AlignToCursor( nCurX, nCurY, eMode );
    

    if (bKeepSel)
    {
        SetCursor( nCurX, nCurY );      

        
        
        

        const ScMarkData& rMark = aViewData.GetMarkData();
        ScRangeList aSelList;
        rMark.FillRangeListWithMarks(&aSelList, false);
        if (!aSelList.In(ScRange(nCurX, nCurY, aViewData.GetTabNo())))
            
            DoneBlockMode(true);
    }
    else
    {
        if (!bShift)
        {
            
            ScMarkData& rMark = aViewData.GetMarkData();
            bool bMarked = rMark.IsMarked() || rMark.IsMultiMarked();
            if (bMarked)
            {
                rMark.ResetMark();
                DoneBlockMode();
                InitOwnBlockMode();
                MarkDataChanged();
            }
        }

        bool bSame = ( nCurX == aViewData.GetCurX() && nCurY == aViewData.GetCurY() );
        bMoveIsShift = bShift;
        pSelEngine->CursorPosChanging( bShift, bControl );
        bMoveIsShift = false;
        aFunctionSet.SetCursorAtCell( nCurX, nCurY, false );

        
        
        if (bSame)
            SelectionChanged();
    }

    ShowAllCursors();
    TestHintWindow();
}

void ScTabView::MoveCursorRel( SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode,
                               bool bShift, bool bKeepSel )
{
    ScDocument* pDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();

    bool bSkipProtected = false, bSkipUnprotected = false;
    ScTableProtection* pProtect = pDoc->GetTabProtection(nTab);
    if ( pProtect && pProtect->isProtected() )
    {
        bSkipProtected   = !pProtect->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS);
        bSkipUnprotected = !pProtect->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS);
    }

    if ( bSkipProtected && bSkipUnprotected )
        return;

    SCsCOL nOldX;
    SCsROW nOldY;
    SCsCOL nCurX;
    SCsROW nCurY;
    if ( aViewData.IsRefMode() )
    {
        nOldX = (SCsCOL) aViewData.GetRefEndX();
        nOldY = (SCsROW) aViewData.GetRefEndY();
        nCurX = nOldX + nMovX;
        nCurY = nOldY + nMovY;
    }
    else
    {
        nOldX = (SCsCOL) aViewData.GetCurX();
        nOldY = (SCsROW) aViewData.GetCurY();
        nCurX = (nMovX != 0) ? nOldX+nMovX : (SCsCOL) aViewData.GetOldCurX();
        nCurY = (nMovY != 0) ? nOldY+nMovY : (SCsROW) aViewData.GetOldCurY();
    }

    aViewData.ResetOldCursor();

    if (nMovX != 0 && ValidColRow(nCurX,nCurY))
        SkipCursorHorizontal(nCurX, nCurY, nOldX, nMovX);

    if (nMovY != 0 && ValidColRow(nCurX,nCurY))
        SkipCursorVertical(nCurX, nCurY, nOldY, nMovY);

    MoveCursorAbs( nCurX, nCurY, eMode, bShift, false, true, bKeepSel );
}

void ScTabView::MoveCursorPage( SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode, bool bShift, bool bKeepSel )
{
    SCsCOL nPageX;
    SCsROW nPageY;
    GetPageMoveEndPosition(nMovX, nMovY, nPageX, nPageY);
    MoveCursorRel( nPageX, nPageY, eMode, bShift, bKeepSel );
}

void ScTabView::MoveCursorArea( SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode, bool bShift, bool bKeepSel )
{
    SCsCOL nNewX;
    SCsROW nNewY;
    GetAreaMoveEndPosition(nMovX, nMovY, eMode, nNewX, nNewY, eMode);
    MoveCursorRel(nNewX, nNewY, eMode, bShift, bKeepSel);
}

void ScTabView::MoveCursorEnd( SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode, bool bShift, bool bKeepSel )
{
    ScDocument* pDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();

    SCCOL nCurX;
    SCROW nCurY;
    aViewData.GetMoveCursor( nCurX,nCurY );
    SCCOL nNewX = nCurX;
    SCROW nNewY = nCurY;

    SCCOL nUsedX = 0;
    SCROW nUsedY = 0;
    if ( nMovX > 0 || nMovY > 0 )
        pDoc->GetPrintArea( nTab, nUsedX, nUsedY );     

    if (nMovX<0)
        nNewX=0;
    else if (nMovX>0)
        nNewX=nUsedX;                                   

    if (nMovY<0)
        nNewY=0;
    else if (nMovY>0)
        nNewY=nUsedY;

    aViewData.ResetOldCursor();
    MoveCursorRel( ((SCsCOL)nNewX)-(SCsCOL)nCurX, ((SCsROW)nNewY)-(SCsROW)nCurY, eMode, bShift, bKeepSel );
}

void ScTabView::MoveCursorScreen( SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode, bool bShift )
{
    ScDocument* pDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();

    SCCOL nCurX;
    SCROW nCurY;
    aViewData.GetMoveCursor( nCurX,nCurY );
    SCCOL nNewX = nCurX;
    SCROW nNewY = nCurY;

    ScSplitPos eWhich = aViewData.GetActivePart();
    SCCOL nPosX = aViewData.GetPosX( WhichH(eWhich) );
    SCROW nPosY = aViewData.GetPosY( WhichV(eWhich) );

    SCCOL nAddX = aViewData.VisibleCellsX( WhichH(eWhich) );
    if (nAddX != 0)
        --nAddX;
    SCROW nAddY = aViewData.VisibleCellsY( WhichV(eWhich) );
    if (nAddY != 0)
        --nAddY;

    if (nMovX<0)
        nNewX=nPosX;
    else if (nMovX>0)
        nNewX=nPosX+nAddX;

    if (nMovY<0)
        nNewY=nPosY;
    else if (nMovY>0)
        nNewY=nPosY+nAddY;

    aViewData.SetOldCursor( nNewX,nNewY );
    pDoc->SkipOverlapped(nNewX, nNewY, nTab);
    MoveCursorAbs( nNewX, nNewY, eMode, bShift, false, true );
}

void ScTabView::MoveCursorEnter( bool bShift )          
{
    const ScInputOptions& rOpt = SC_MOD()->GetInputOptions();
    if (!rOpt.GetMoveSelection())
    {
        aViewData.UpdateInputHandler(true);
        return;
    }

    SCsCOL nMoveX = 0;
    SCsROW nMoveY = 0;
    switch ((ScDirection)rOpt.GetMoveDir())
    {
        case DIR_BOTTOM:
            nMoveY = bShift ? -1 : 1;
            break;
        case DIR_RIGHT:
            nMoveX = bShift ? -1 : 1;
            break;
        case DIR_TOP:
            nMoveY = bShift ? 1 : -1;
            break;
        case DIR_LEFT:
            nMoveX = bShift ? 1 : -1;
            break;
    }

    ScMarkData& rMark = aViewData.GetMarkData();
    if (rMark.IsMarked() || rMark.IsMultiMarked())
    {
        SCCOL nCurX;
        SCROW nCurY;
        aViewData.GetMoveCursor( nCurX,nCurY );
        SCCOL nNewX = nCurX;
        SCROW nNewY = nCurY;
        SCTAB nTab = aViewData.GetTabNo();

        ScDocument* pDoc = aViewData.GetDocument();
        pDoc->GetNextPos( nNewX,nNewY, nTab, nMoveX,nMoveY, true, false, rMark );

        MoveCursorRel( ((SCsCOL)nNewX)-(SCsCOL)nCurX, ((SCsROW)nNewY)-(SCsROW)nCurY,
                            SC_FOLLOW_LINE, false, true );

        
        if ( nNewX == nCurX && nNewY == nCurY )
            aViewData.UpdateInputHandler(true);
    }
    else
    {
        if ( nMoveY != 0 && !nMoveX )
        {
            
            SCCOL nTabCol = aViewData.GetTabStartCol();
            if (nTabCol != SC_TABSTART_NONE)
            {
                SCCOL nCurX;
                SCROW nCurY;
                aViewData.GetMoveCursor( nCurX,nCurY );
                nMoveX = ((SCsCOL)nTabCol)-(SCsCOL)nCurX;
            }
        }

        MoveCursorRel( nMoveX,nMoveY, SC_FOLLOW_LINE, false );
    }
}


bool ScTabView::MoveCursorKeyInput( const KeyEvent& rKeyEvent )
{
    const KeyCode& rKCode = rKeyEvent.GetKeyCode();

    enum { MOD_NONE, MOD_CTRL, MOD_ALT, MOD_BOTH } eModifier =
        rKCode.IsMod1() ?
            (rKCode.IsMod2() ? MOD_BOTH : MOD_CTRL) :
            (rKCode.IsMod2() ? MOD_ALT : MOD_NONE);

    bool bSel = rKCode.IsShift();
    sal_uInt16 nCode = rKCode.GetCode();

    
    SCsCOL nDX = 0;
    SCsROW nDY = 0;
    switch( nCode )
    {
        case KEY_LEFT:  nDX = -1;   break;
        case KEY_RIGHT: nDX = 1;    break;
        case KEY_UP:    nDY = -1;   break;
        case KEY_DOWN:  nDY = 1;    break;
    }
    if( nDX != 0 || nDY != 0 )
    {
        switch( eModifier )
        {
            case MOD_NONE:  MoveCursorRel( nDX, nDY, SC_FOLLOW_LINE, bSel );    break;
            case MOD_CTRL:  MoveCursorArea( nDX, nDY, SC_FOLLOW_JUMP, bSel );   break;
            default:
            {
                
            }
        }
        
        return true;
    }

    
    if( (nCode == KEY_PAGEUP) || (nCode == KEY_PAGEDOWN) )
    {
        nDX = (nCode == KEY_PAGEUP) ? -1 : 1;
        switch( eModifier )
        {
            case MOD_NONE:  MoveCursorPage( 0, static_cast<SCsCOLROW>(nDX), SC_FOLLOW_FIX, bSel );  break;
            case MOD_ALT:   MoveCursorPage( nDX, 0, SC_FOLLOW_FIX, bSel );  break;
            case MOD_CTRL:  SelectNextTab( nDX );                           break;
            default:
            {
                
            }
        }
        return true;
    }

    
    if( (nCode == KEY_HOME) || (nCode == KEY_END) )
    {
        nDX = (nCode == KEY_HOME) ? -1 : 1;
        ScFollowMode eMode = (nCode == KEY_HOME) ? SC_FOLLOW_LINE : SC_FOLLOW_JUMP;
        switch( eModifier )
        {
            case MOD_NONE:  MoveCursorEnd( nDX, 0, eMode, bSel );   break;
            case MOD_CTRL:  MoveCursorEnd( nDX, static_cast<SCsCOLROW>(nDX), eMode, bSel ); break;
            default:
            {
                
            }
        }
        return true;
    }

    return false;
}


        
void ScTabView::FindNextUnprot( bool bShift, bool bInSelection )
{
    short nMove = bShift ? -1 : 1;

    ScMarkData& rMark = aViewData.GetMarkData();
    bool bMarked = bInSelection && (rMark.IsMarked() || rMark.IsMultiMarked());

    SCCOL nCurX;
    SCROW nCurY;
    aViewData.GetMoveCursor( nCurX,nCurY );
    SCCOL nNewX = nCurX;
    SCROW nNewY = nCurY;
    SCTAB nTab = aViewData.GetTabNo();

    ScDocument* pDoc = aViewData.GetDocument();
    pDoc->GetNextPos( nNewX,nNewY, nTab, nMove,0, bMarked, true, rMark );

    SCCOL nTabCol = aViewData.GetTabStartCol();
    if ( nTabCol == SC_TABSTART_NONE )
        nTabCol = nCurX;                    

    MoveCursorRel( ((SCsCOL)nNewX)-(SCsCOL)nCurX, ((SCsROW)nNewY)-(SCsROW)nCurY,
                   SC_FOLLOW_LINE, false, true );

    
    aViewData.SetTabStartCol( nTabCol );
}

void ScTabView::MarkColumns()
{
    SCCOL nStartCol;
    SCCOL nEndCol;

    ScMarkData& rMark = aViewData.GetMarkData();
    if (rMark.IsMarked())
    {
        ScRange aMarkRange;
        rMark.GetMarkArea( aMarkRange );
        nStartCol = aMarkRange.aStart.Col();
        nEndCol = aMarkRange.aEnd.Col();
    }
    else
    {
        SCROW nDummy;
        aViewData.GetMoveCursor( nStartCol, nDummy );
        nEndCol=nStartCol;
    }

    SCTAB nTab = aViewData.GetTabNo();
    DoneBlockMode();
    InitBlockMode( nStartCol,0, nTab );
    MarkCursor( nEndCol,MAXROW, nTab );
    SelectionChanged();
}

void ScTabView::MarkRows()
{
    SCROW nStartRow;
    SCROW nEndRow;

    ScMarkData& rMark = aViewData.GetMarkData();
    if (rMark.IsMarked())
    {
        ScRange aMarkRange;
        rMark.GetMarkArea( aMarkRange );
        nStartRow = aMarkRange.aStart.Row();
        nEndRow = aMarkRange.aEnd.Row();
    }
    else
    {
        SCCOL nDummy;
        aViewData.GetMoveCursor( nDummy, nStartRow );
        nEndRow=nStartRow;
    }

    SCTAB nTab = aViewData.GetTabNo();
    DoneBlockMode();
    InitBlockMode( 0,nStartRow, nTab );
    MarkCursor( MAXCOL,nEndRow, nTab );
    SelectionChanged();
}

void ScTabView::MarkDataArea( bool bIncludeCursor )
{
    ScDocument* pDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();
    SCCOL nStartCol = aViewData.GetCurX();
    SCROW nStartRow = aViewData.GetCurY();
    SCCOL nEndCol = nStartCol;
    SCROW nEndRow = nStartRow;

    pDoc->GetDataArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow, bIncludeCursor, false );

    HideAllCursors();
    DoneBlockMode();
    InitBlockMode( nStartCol, nStartRow, nTab );
    MarkCursor( nEndCol, nEndRow, nTab );
    ShowAllCursors();

    SelectionChanged();
}

void ScTabView::MarkMatrixFormula()
{
    ScDocument* pDoc = aViewData.GetDocument();
    ScAddress aCursor( aViewData.GetCurX(), aViewData.GetCurY(), aViewData.GetTabNo() );
    ScRange aMatrix;
    if ( pDoc->GetMatrixFormulaRange( aCursor, aMatrix ) )
    {
        MarkRange( aMatrix, false );        
    }
}

void ScTabView::MarkRange( const ScRange& rRange, bool bSetCursor, bool bContinue )
{
    SCTAB nTab = rRange.aStart.Tab();
    SetTabNo( nTab );

    HideAllCursors();
    DoneBlockMode( bContinue ); 
    if (bSetCursor)             
    {
        SCCOL nAlignX = rRange.aStart.Col();
        SCROW nAlignY = rRange.aStart.Row();
        bool bCol = ( rRange.aStart.Col() == 0 && rRange.aEnd.Col() == MAXCOL ) && !aViewData.GetDocument()->IsInVBAMode();
        bool bRow = ( rRange.aStart.Row() == 0 && rRange.aEnd.Row() == MAXROW );
        if ( bCol )
            nAlignX = aViewData.GetPosX(WhichH(aViewData.GetActivePart()));
        if ( bRow )
            nAlignY = aViewData.GetPosY(WhichV(aViewData.GetActivePart()));
        AlignToCursor( nAlignX, nAlignY, SC_FOLLOW_JUMP );
    }
    InitBlockMode( rRange.aStart.Col(), rRange.aStart.Row(), nTab );
    MarkCursor( rRange.aEnd.Col(), rRange.aEnd.Row(), nTab );
    if (bSetCursor)
    {
        SCCOL nPosX = rRange.aStart.Col();
        SCROW nPosY = rRange.aStart.Row();
        ScDocument* pDoc = aViewData.GetDocument();
        pDoc->SkipOverlapped(nPosX, nPosY, nTab);

        aViewData.ResetOldCursor();
        SetCursor( nPosX, nPosY );
    }
    ShowAllCursors();

    SelectionChanged();
}

void ScTabView::Unmark()
{
    ScMarkData& rMark = aViewData.GetMarkData();
    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
    {
        SCCOL nCurX;
        SCROW nCurY;
        aViewData.GetMoveCursor( nCurX,nCurY );
        MoveCursorAbs( nCurX, nCurY, SC_FOLLOW_NONE, false, false );

        SelectionChanged();
    }
}

void ScTabView::SetMarkData( const ScMarkData& rNew )
{
    DoneBlockMode();
    InitOwnBlockMode();
    aViewData.GetMarkData() = rNew;

    MarkDataChanged();
}

void ScTabView::MarkDataChanged()
{
    

    UpdateSelectionOverlay();
}

void ScTabView::SelectNextTab( short nDir, bool bExtendSelection )
{
    if (!nDir) return;
    OSL_ENSURE( nDir==-1 || nDir==1, "SelectNextTab: falscher Wert");

    ScDocument* pDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();
    if (nDir<0)
    {
        if (!nTab) return;
        --nTab;
        while (!pDoc->IsVisible(nTab))
        {
            if (!nTab) return;
            --nTab;
        }
    }
    else
    {
        SCTAB nCount = pDoc->GetTableCount();
        ++nTab;
        if (nTab >= nCount) return;
        while (!pDoc->IsVisible(nTab))
        {
            ++nTab;
            if (nTab >= nCount) return;
        }
    }

    SetTabNo( nTab, false, bExtendSelection );
    PaintExtras();
}




void ScTabView::SetTabNo( SCTAB nTab, bool bNew, bool bExtendSelection, bool bSameTabButMoved )
{
    if ( !ValidTab(nTab) )
    {
        OSL_FAIL("SetTabNo: falsche Tabelle");
        return;
    }

    if ( nTab != aViewData.GetTabNo() || bNew )
    {
        
        FmFormShell* pFormSh = aViewData.GetViewShell()->GetFormShell();
        if (pFormSh)
        {
            bool bAllowed = pFormSh->PrepareClose(true);
            if (!bAllowed)
            {
                
                

                return;     
            }
        }

                                        

        ScDocument* pDoc = aViewData.GetDocument();

        pDoc->MakeTable( nTab );

        
        
        aViewData.GetDocShell()->UpdatePendingRowHeights( nTab );

        SCTAB nTabCount = pDoc->GetTableCount();
        SCTAB nOldPos = nTab;
        while (!pDoc->IsVisible(nTab))              
        {
            bool bUp = (nTab>=nOldPos);
            if (bUp)
            {
                ++nTab;
                if (nTab>=nTabCount)
                {
                    nTab = nOldPos;
                    bUp = false;
                }
            }

            if (!bUp)
            {
                if (nTab != 0)
                    --nTab;
                else
                {
                    OSL_FAIL("keine sichtbare Tabelle");
                    pDoc->SetVisible( 0, true );
                }
            }
        }

        
        
        DrawDeselectAll();

        ScModule* pScMod = SC_MOD();
        bool bRefMode = pScMod->IsFormulaMode();
        if ( !bRefMode ) 
        {
            DoneBlockMode();
            pSelEngine->Reset();                
            aViewData.SetRefTabNo( nTab );
        }

        ScSplitPos eOldActive = aViewData.GetActivePart();      
        bool bFocus = pGridWin[eOldActive]->HasFocus();

        aViewData.SetTabNo( nTab );
        
        
        UpdateShow();
        aViewData.ResetOldCursor();

        SfxBindings& rBindings = aViewData.GetBindings();
        ScMarkData& rMark = aViewData.GetMarkData();

        bool bAllSelected = true;
        for (SCTAB nSelTab = 0; nSelTab < nTabCount; ++nSelTab)
        {
            if (!pDoc->IsVisible(nSelTab) || rMark.GetTableSelect(nSelTab))
            {
                if (nTab == nSelTab)
                    
                    
                    bExtendSelection = true;
            }
            else
            {
                bAllSelected = false;
                if (bExtendSelection)
                    
                    break;
            }
        }
        if (bAllSelected && !bNew)
            
            
            bExtendSelection = false;

        if (bExtendSelection)
            rMark.SelectTable( nTab, true );
        else
        {
            rMark.SelectOneTable( nTab );
            rBindings.Invalidate( FID_FILL_TAB );
            rBindings.Invalidate( FID_TAB_DESELECTALL );
        }

        SetCursor( aViewData.GetCurX(), aViewData.GetCurY(), true );
        bool bUnoRefDialog = pScMod->IsRefDialogOpen() && pScMod->GetCurRefDlgId() == WID_SIMPLE_REF;

        
        RefreshZoom();
        UpdateVarZoom();

        if ( bRefMode )     
        {
            for (sal_uInt16 i = 0; i < 4; ++i)
                if (pGridWin[i] && pGridWin[i]->IsVisible())
                    pGridWin[i]->UpdateEditViewPos();
        }

        TabChanged(bSameTabButMoved);                                       
        UpdateVisibleRange();

        aViewData.GetViewShell()->WindowChanged();          
        if ( !bUnoRefDialog )
            aViewData.GetViewShell()->DisconnectAllClients();   
        else
        {
            

            ScClient* pClient = static_cast<ScClient*>(aViewData.GetViewShell()->GetIPClient());
            if ( pClient && pClient->IsObjectInPlaceActive() )
            {
                Rectangle aObjArea = pClient->GetObjArea();
                if ( nTab == aViewData.GetRefTabNo() )
                {
                    

                    SdrOle2Obj* pDrawObj = pClient->GetDrawObj();
                    if ( pDrawObj )
                    {
                        Rectangle aRect = pDrawObj->GetLogicRect();
                        MapMode aMapMode( MAP_100TH_MM );
                        Size aOleSize = pDrawObj->GetOrigObjSize( &aMapMode );
                        aRect.SetSize( aOleSize );
                        aObjArea = aRect;
                    }
                }
                else
                {
                    

                    aObjArea.SetPos( Point( 0, -2*aObjArea.GetHeight() ) );
                }
                pClient->SetObjArea( aObjArea );
            }
        }

        if ( bFocus && aViewData.GetActivePart() != eOldActive && !bRefMode )
            ActiveGrabFocus();      

            

        bool bResize = false;
        if ( aViewData.GetHSplitMode() == SC_SPLIT_FIX )
            if (aViewData.UpdateFixX())
                bResize = true;
        if ( aViewData.GetVSplitMode() == SC_SPLIT_FIX )
            if (aViewData.UpdateFixY())
                bResize = true;
        if (bResize)
            RepeatResize();
        InvalidateSplit();

        if ( aViewData.IsPagebreakMode() )
            UpdatePageBreakData();              

        
        
        for (sal_uInt16 i=0; i<4; i++)
            if (pGridWin[i])
                pGridWin[i]->SetMapMode( pGridWin[i]->GetDrawMapMode() );
        SetNewVisArea();

        PaintGrid();
        PaintTop();
        PaintLeft();
        PaintExtras();

        DoResize( aBorderPos, aFrameSize );
        rBindings.Invalidate( SID_DELETE_PRINTAREA );   
        rBindings.Invalidate( FID_DEL_MANUALBREAKS );
        rBindings.Invalidate( FID_RESET_PRINTZOOM );
        rBindings.Invalidate( SID_STATUS_DOCPOS );      
        rBindings.Invalidate( SID_STATUS_PAGESTYLE );   
        rBindings.Invalidate( SID_CURRENTTAB );         
        rBindings.Invalidate( SID_STYLE_FAMILY2 );  
        rBindings.Invalidate( SID_STYLE_FAMILY4 );  
        rBindings.Invalidate( SID_TABLES_COUNT );

        if(pScMod->IsRefDialogOpen())
        {
            sal_uInt16 nCurRefDlgId=pScMod->GetCurRefDlgId();
            SfxViewFrame* pViewFrm = aViewData.GetViewShell()->GetViewFrame();
            SfxChildWindow* pChildWnd = pViewFrm->GetChildWindow( nCurRefDlgId );
            if ( pChildWnd )
            {
                IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
                pRefDlg->ViewShellChanged();
            }
        }
    }
}

//

//

void ScTabView::MakeEditView( ScEditEngineDefaulter* pEngine, SCCOL nCol, SCROW nRow )
{
    DrawDeselectAll();

    if (pDrawView)
        DrawEnableAnim( false );

    EditView* pSpellingView = aViewData.GetSpellingView();

    for (sal_uInt16 i=0; i<4; i++)
        if (pGridWin[i])
            if ( pGridWin[i]->IsVisible() && !aViewData.HasEditView((ScSplitPos)i) )
            {
                ScHSplitPos eHWhich = WhichH( (ScSplitPos) i );
                ScVSplitPos eVWhich = WhichV( (ScSplitPos) i );
                SCCOL nScrX = aViewData.GetPosX( eHWhich );
                SCROW nScrY = aViewData.GetPosY( eVWhich );

                bool bPosVisible =
                     ( nCol >= nScrX && nCol <= nScrX + aViewData.VisibleCellsX(eHWhich) + 1 &&
                       nRow >= nScrY && nRow <= nScrY + aViewData.VisibleCellsY(eVWhich) + 1 );

                
                

                
                if ( bPosVisible || aViewData.GetActivePart() == (ScSplitPos) i ||
                     ( pSpellingView && aViewData.GetEditView((ScSplitPos) i) == pSpellingView ) )
                {
                    pGridWin[i]->HideCursor();

                    pGridWin[i]->DeleteCursorOverlay();
                    pGridWin[i]->DeleteAutoFillOverlay();
                    pGridWin[i]->DeleteCopySourceOverlay();

                    
                    pGridWin[i]->flushOverlayManager();

                    
                    pGridWin[i]->SetMapMode(aViewData.GetLogicMode());

                    aViewData.SetEditEngine( (ScSplitPos) i, pEngine, pGridWin[i], nCol, nRow );

                    if ( !bPosVisible )
                    {
                        
                        
                        pGridWin[i]->UpdateEditViewPos();
                    }
                }
            }

    if (aViewData.GetViewShell()->HasAccessibilityObjects())
        aViewData.GetViewShell()->BroadcastAccessibility(SfxSimpleHint(SC_HINT_ACC_ENTEREDITMODE));
}

void ScTabView::UpdateEditView()
{
    ScSplitPos eActive = aViewData.GetActivePart();
    for (sal_uInt16 i=0; i<4; i++)
        if (aViewData.HasEditView( (ScSplitPos) i ))
        {
            EditView* pEditView = aViewData.GetEditView( (ScSplitPos) i );
            aViewData.SetEditEngine( (ScSplitPos) i,
                static_cast<ScEditEngineDefaulter*>(pEditView->GetEditEngine()),
                pGridWin[i], GetViewData()->GetCurX(), GetViewData()->GetCurY() );
            if ( (ScSplitPos)i == eActive )
                pEditView->ShowCursor( false );
        }
}

void ScTabView::KillEditView( bool bNoPaint )
{
    sal_uInt16 i;
    SCCOL nCol1 = aViewData.GetEditStartCol();
    SCROW nRow1 = aViewData.GetEditStartRow();
    SCCOL nCol2 = aViewData.GetEditEndCol();
    SCROW nRow2 = aViewData.GetEditEndRow();
    bool bPaint[4];
    bool bNotifyAcc = false;

    bool bExtended = nRow1 != nRow2;                    
    bool bAtCursor = nCol1 <= aViewData.GetCurX() &&
                     nCol2 >= aViewData.GetCurX() &&
                     nRow1 == aViewData.GetCurY();
    for (i=0; i<4; i++)
    {
        bPaint[i] = aViewData.HasEditView( (ScSplitPos) i );
        if (bPaint[i])
            bNotifyAcc = true;
    }

    
    if ((bNotifyAcc) && (aViewData.GetViewShell()->HasAccessibilityObjects()))
        aViewData.GetViewShell()->BroadcastAccessibility(SfxSimpleHint(SC_HINT_ACC_LEAVEEDITMODE));

    aViewData.ResetEditView();
    for (i=0; i<4; i++)
        if (pGridWin[i] && bPaint[i])
            if (pGridWin[i]->IsVisible())
            {
                pGridWin[i]->ShowCursor();

                pGridWin[i]->SetMapMode(pGridWin[i]->GetDrawMapMode());

                
                if (bExtended || ( bAtCursor && !bNoPaint ))
                {
                    pGridWin[i]->Draw( nCol1, nRow1, nCol2, nRow2 );
                    pGridWin[i]->UpdateSelectionOverlay();
                }
            }

    if (pDrawView)
        DrawEnableAnim( true );

        
        

    bool bGrabFocus = false;
    if (aViewData.IsActive())
    {
        ScInputHandler* pInputHdl = SC_MOD()->GetInputHdl();
        if ( pInputHdl )
        {
            ScInputWindow* pInputWin = pInputHdl->GetInputWindow();
            if (pInputWin && pInputWin->IsInputActive())
                bGrabFocus = true;
        }
    }

    if (bGrabFocus)
    {



        GetActiveWin()->GrabFocus();
    }

    

    for (i=0; i<4; i++)
        if (pGridWin[i] && pGridWin[i]->IsVisible())
        {
            Cursor* pCur = pGridWin[i]->GetCursor();
            if (pCur && pCur->IsVisible())
                pCur->Hide();

            if(bPaint[i])
            {
                pGridWin[i]->UpdateCursorOverlay();
                pGridWin[i]->UpdateAutoFillOverlay();
            }
        }
}

void ScTabView::UpdateFormulas()
{
    if ( aViewData.GetDocument()->IsAutoCalcShellDisabled() )
        return ;

    sal_uInt16 i;
    for (i=0; i<4; i++)
        if (pGridWin[i])
            if (pGridWin[i]->IsVisible())
                pGridWin[i]->UpdateFormulas();

    if ( aViewData.IsPagebreakMode() )
        UpdatePageBreakData();              

    UpdateHeaderWidth();

    
    if ( aViewData.HasEditView( aViewData.GetActivePart() ) )
        UpdateEditView();
}



void ScTabView::PaintArea( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                            ScUpdateMode eMode )
{
    SCCOL nCol1;
    SCROW nRow1;
    SCCOL nCol2;
    SCROW nRow2;

    PutInOrder( nStartCol, nEndCol );
    PutInOrder( nStartRow, nEndRow );

    for (size_t i = 0; i < 4; ++i)
    {
        if (!pGridWin[i] || !pGridWin[i]->IsVisible())
            continue;

        ScHSplitPos eHWhich = WhichH( (ScSplitPos) i );
        ScVSplitPos eVWhich = WhichV( (ScSplitPos) i );
        bool bOut = false;

        nCol1 = nStartCol;
        nRow1 = nStartRow;
        nCol2 = nEndCol;
        nRow2 = nEndRow;

        SCCOL nScrX = aViewData.GetPosX( eHWhich );
        SCROW nScrY = aViewData.GetPosY( eVWhich );
        if (nCol1 < nScrX) nCol1 = nScrX;
        if (nCol2 < nScrX)
        {
            if ( eMode == SC_UPDATE_ALL )   
                nCol2 = nScrX;              
            else
                bOut = true;                
        }
        if (nRow1 < nScrY) nRow1 = nScrY;
        if (nRow2 < nScrY) bOut = true;

        SCCOL nLastX = nScrX + aViewData.VisibleCellsX( eHWhich ) + 1;
        SCROW nLastY = nScrY + aViewData.VisibleCellsY( eVWhich ) + 1;
        if (nCol1 > nLastX) bOut = true;
        if (nCol2 > nLastX) nCol2 = nLastX;
        if (nRow1 > nLastY) bOut = true;
        if (nRow2 > nLastY) nRow2 = nLastY;

        if (bOut)
            continue;

        if ( eMode == SC_UPDATE_CHANGED )
            pGridWin[i]->Draw( nCol1, nRow1, nCol2, nRow2, eMode );
        else    
        {
            bool bLayoutRTL = aViewData.GetDocument()->IsLayoutRTL( aViewData.GetTabNo() );
            long nLayoutSign = bLayoutRTL ? -1 : 1;

            Point aStart = aViewData.GetScrPos( nCol1, nRow1, (ScSplitPos) i );
            Point aEnd   = aViewData.GetScrPos( nCol2+1, nRow2+1, (ScSplitPos) i );
            if ( eMode == SC_UPDATE_ALL )
                aEnd.X() = bLayoutRTL ? 0 : (pGridWin[i]->GetOutputSizePixel().Width());
            aEnd.X() -= nLayoutSign;
            aEnd.Y() -= 1;

            
            if ( eMode == SC_UPDATE_ALL && nRow2 >= MAXROW )
                aEnd.Y() = pGridWin[i]->GetOutputSizePixel().Height();

            bool bShowChanges = true;           
            if (bShowChanges)
            {
                aStart.X() -= nLayoutSign;      
                aStart.Y() -= 1;
            }

            bool bMarkClipped = aViewData.GetOptions().GetOption( VOPT_CLIPMARKS );
            if (bMarkClipped)
            {
                
                
                
                
                
                {
                    long nMarkPixel = (long)( SC_CLIPMARK_SIZE * aViewData.GetPPTX() );
                    aStart.X() -= nMarkPixel * nLayoutSign;
                    if (!bShowChanges)
                        aStart.X() -= nLayoutSign;      
                }
            }

            pGridWin[i]->Invalidate( pGridWin[i]->PixelToLogic( Rectangle( aStart,aEnd ) ) );
        }
    }

    
    
    
    
    
    
    
}

void ScTabView::PaintRangeFinderEntry (ScRangeFindData* pData, const SCTAB nTab)
{
    ScRange aRef = pData->aRef;
    aRef.Justify();                 

    if ( aRef.aStart == aRef.aEnd )     
        aViewData.GetDocument()->ExtendMerge(aRef);

    if ( aRef.aStart.Tab() >= nTab && aRef.aEnd.Tab() <= nTab )
    {
        SCCOL nCol1 = aRef.aStart.Col();
        SCROW nRow1 = aRef.aStart.Row();
        SCCOL nCol2 = aRef.aEnd.Col();
        SCROW nRow2 = aRef.aEnd.Row();

        
        

        bool bHiddenEdge = false;
        SCROW nTmp;
        ScDocument* pDoc = aViewData.GetDocument();
        while ( nCol1 > 0 && pDoc->ColHidden(nCol1, nTab) )
        {
            --nCol1;
            bHiddenEdge = true;
        }
        while ( nCol2 < MAXCOL && pDoc->ColHidden(nCol2, nTab) )
        {
            ++nCol2;
            bHiddenEdge = true;
        }
        nTmp = pDoc->LastVisibleRow(0, nRow1, nTab);
        if (!ValidRow(nTmp))
            nTmp = 0;
        if (nTmp < nRow1)
        {
            nRow1 = nTmp;
            bHiddenEdge = true;
        }
        nTmp = pDoc->FirstVisibleRow(nRow2, MAXROW, nTab);
        if (!ValidRow(nTmp))
            nTmp = MAXROW;
        if (nTmp > nRow2)
        {
            nRow2 = nTmp;
            bHiddenEdge = true;
        }

        if ( nCol2 - nCol1 > 1 && nRow2 - nRow1 > 1 && !bHiddenEdge )
        {
            
            PaintArea( nCol1, nRow1, nCol2, nRow1, SC_UPDATE_MARKS );
            PaintArea( nCol1, nRow1+1, nCol1, nRow2-1, SC_UPDATE_MARKS );
            PaintArea( nCol2, nRow1+1, nCol2, nRow2-1, SC_UPDATE_MARKS );
            PaintArea( nCol1, nRow2, nCol2, nRow2, SC_UPDATE_MARKS );
        }
        else    
            PaintArea( nCol1, nRow1, nCol2, nRow2, SC_UPDATE_MARKS );
    }
}

void ScTabView::UpdateGrid()
{
    if (!aViewData.IsActive())
        return;

    if (!UpdateVisibleRange())
        
        return;

    SC_MOD()->AnythingChanged();                
    PaintGrid();
}

void ScTabView::PaintRangeFinder( long nNumber )
{
    ScInputHandler* pHdl = SC_MOD()->GetInputHdl( aViewData.GetViewShell() );
    if (pHdl)
    {
        ScRangeFindList* pRangeFinder = pHdl->GetRangeFindList();
        if ( pRangeFinder && pRangeFinder->GetDocName() == aViewData.GetDocShell()->GetTitle() )
        {
            SCTAB nTab = aViewData.GetTabNo();
            sal_uInt16 nCount = (sal_uInt16)pRangeFinder->Count();

            if (nNumber < 0)
            {
                for (sal_uInt16 i=0; i<nCount; i++)
                    PaintRangeFinderEntry(pRangeFinder->GetObject(i),nTab);
            }
            else
            {
                sal_uInt16 idx = nNumber;
                if (idx < nCount)
                    PaintRangeFinderEntry(pRangeFinder->GetObject(idx),nTab);
            }
        }
    }
}



void ScTabView::AddHighlightRange( const ScRange& rRange, const Color& rColor )
{
    maHighlightRanges.push_back( ScHighlightEntry( rRange, rColor ) );

    SCTAB nTab = aViewData.GetTabNo();
    if ( nTab >= rRange.aStart.Tab() && nTab <= rRange.aEnd.Tab() )
        PaintArea( rRange.aStart.Col(), rRange.aStart.Row(),
                    rRange.aEnd.Col(), rRange.aEnd.Row(), SC_UPDATE_MARKS );
}

void ScTabView::ClearHighlightRanges()
{
    SCTAB nTab = aViewData.GetTabNo();
    std::vector<ScHighlightEntry>::const_iterator pIter;
    for ( pIter = maHighlightRanges.begin(); pIter != maHighlightRanges.end(); ++pIter)
    {
        ScRange aRange = pIter->aRef;
        if ( nTab >= aRange.aStart.Tab() && nTab <= aRange.aEnd.Tab() )
            PaintArea( aRange.aStart.Col(), aRange.aStart.Row(),
                       aRange.aEnd.Col(), aRange.aEnd.Row(), SC_UPDATE_MARKS );
    }

    maHighlightRanges.clear();
}

void ScTabView::DoChartSelection(
    const uno::Sequence< chart2::data::HighlightedRange > & rHilightRanges )
{
    ClearHighlightRanges();
    const sal_Unicode sep = ::formula::FormulaCompiler::GetNativeSymbolChar(ocSep);

    for( sal_Int32 i=0; i<rHilightRanges.getLength(); ++i )
    {
        Color aSelColor( rHilightRanges[i].PreferredColor );
        ScRangeList aRangeList;
        ScDocument* pDoc = aViewData.GetDocShell()->GetDocument();
        if( ScRangeStringConverter::GetRangeListFromString(
                aRangeList, rHilightRanges[i].RangeRepresentation, pDoc, pDoc->GetAddressConvention(), sep ))
        {
            size_t nListSize = aRangeList.size();
            for ( size_t j = 0; j < nListSize; ++j )
            {
                ScRange* p = aRangeList[j];
                if( rHilightRanges[i].Index == - 1 )
                    AddHighlightRange( *p, aSelColor );
                else
                    AddHighlightRange( lcl_getSubRangeByIndex( *p, rHilightRanges[i].Index ), aSelColor );
            }
        }
    }
}



void ScTabView::PaintGrid()
{
    sal_uInt16 i;
    for (i=0; i<4; i++)
        if (pGridWin[i])
            if (pGridWin[i]->IsVisible())
                pGridWin[i]->Invalidate();
}



void ScTabView::PaintTop()
{
    sal_uInt16 i;
    for (i=0; i<2; i++)
    {
        if (pColBar[i])
            pColBar[i]->Invalidate();
        if (pColOutline[i])
            pColOutline[i]->Invalidate();
    }
}

void ScTabView::CreateAnchorHandles(SdrHdlList& rHdl, const ScAddress& rAddress)
{
    sal_uInt16 i;

    for(i=0; i<4; i++)
    {
        if(pGridWin[i])
        {
            if(pGridWin[i]->IsVisible())
            {
                pGridWin[i]->CreateAnchorHandle(rHdl, rAddress);
            }
        }
    }
}

void ScTabView::PaintTopArea( SCCOL nStartCol, SCCOL nEndCol )
{
        

    if ( nStartCol < aViewData.GetPosX(SC_SPLIT_LEFT) ||
         nStartCol < aViewData.GetPosX(SC_SPLIT_RIGHT) )
        aViewData.RecalcPixPos();

        

    if ( aViewData.GetHSplitMode() == SC_SPLIT_FIX && nStartCol < aViewData.GetFixPosX() )
        if (aViewData.UpdateFixX())
            RepeatResize();

        

    if (nStartCol>0)
        --nStartCol;                

    bool bLayoutRTL = aViewData.GetDocument()->IsLayoutRTL( aViewData.GetTabNo() );
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    for (sal_uInt16 i=0; i<2; i++)
    {
        ScHSplitPos eWhich = (ScHSplitPos) i;
        if (pColBar[eWhich])
        {
            Size aWinSize = pColBar[eWhich]->GetSizePixel();
            long nStartX = aViewData.GetScrPos( nStartCol, 0, eWhich ).X();
            long nEndX;
            if (nEndCol >= MAXCOL)
                nEndX = bLayoutRTL ? 0 : ( aWinSize.Width()-1 );
            else
                nEndX = aViewData.GetScrPos( nEndCol+1, 0, eWhich ).X() - nLayoutSign;
            pColBar[eWhich]->Invalidate(
                    Rectangle( nStartX, 0, nEndX, aWinSize.Height()-1 ) );
        }
        if (pColOutline[eWhich])
            pColOutline[eWhich]->Invalidate();
    }
}




void ScTabView::PaintLeft()
{
    sal_uInt16 i;
    for (i=0; i<2; i++)
    {
        if (pRowBar[i])
            pRowBar[i]->Invalidate();
        if (pRowOutline[i])
            pRowOutline[i]->Invalidate();
    }
}

void ScTabView::PaintLeftArea( SCROW nStartRow, SCROW nEndRow )
{
        

    if ( nStartRow < aViewData.GetPosY(SC_SPLIT_TOP) ||
         nStartRow < aViewData.GetPosY(SC_SPLIT_BOTTOM) )
        aViewData.RecalcPixPos();

        

    if ( aViewData.GetVSplitMode() == SC_SPLIT_FIX && nStartRow < aViewData.GetFixPosY() )
        if (aViewData.UpdateFixY())
            RepeatResize();

        

    if (nStartRow>0)
        --nStartRow;

    for (sal_uInt16 i=0; i<2; i++)
    {
        ScVSplitPos eWhich = (ScVSplitPos) i;
        if (pRowBar[eWhich])
        {
            Size aWinSize = pRowBar[eWhich]->GetSizePixel();
            long nStartY = aViewData.GetScrPos( 0, nStartRow, eWhich ).Y();
            long nEndY;
            if (nEndRow >= MAXROW)
                nEndY = aWinSize.Height()-1;
            else
                nEndY = aViewData.GetScrPos( 0, nEndRow+1, eWhich ).Y() - 1;
            pRowBar[eWhich]->Invalidate(
                    Rectangle( 0, nStartY, aWinSize.Width()-1, nEndY ) );
        }
        if (pRowOutline[eWhich])
            pRowOutline[eWhich]->Invalidate();
    }
}

bool ScTabView::PaintExtras()
{
    bool bRet = false;
    ScDocument* pDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();
    if (!pDoc->HasTable(nTab))                  
    {
        SCTAB nCount = pDoc->GetTableCount();
        aViewData.SetTabNo(nCount-1);
        bRet = true;
    }
    pTabControl->UpdateStatus();                        
    return bRet;
}

void ScTabView::RecalcPPT()
{
    
    

    double nOldX = aViewData.GetPPTX();
    double nOldY = aViewData.GetPPTY();

    aViewData.RefreshZoom();                            

    bool bChangedX = ( aViewData.GetPPTX() != nOldX );
    bool bChangedY = ( aViewData.GetPPTY() != nOldY );
    if ( bChangedX || bChangedY )
    {
        
        

        Fraction aZoomX = aViewData.GetZoomX();
        Fraction aZoomY = aViewData.GetZoomY();
        SetZoom( aZoomX, aZoomY, false );

        PaintGrid();
        if (bChangedX)
            PaintTop();
        if (bChangedY)
            PaintLeft();
    }
}

void ScTabView::ActivateView( bool bActivate, bool bFirst )
{
    if ( bActivate == aViewData.IsActive() && !bFirst )
    {
        
        
        return;
    }

    
    
    
    

    if (!bActivate)
    {
        ScModule* pScMod = SC_MOD();
        bool bRefMode = pScMod->IsFormulaMode();

            
            

        if (!bRefMode)
        {
            
            ScInputHandler* pHdl = SC_MOD()->GetInputHdl(aViewData.GetViewShell());
            if (pHdl)
                pHdl->EnterHandler();
        }
    }

    PaintExtras();

    aViewData.Activate(bActivate);

    PaintBlock(false);                  

    if (!bActivate)
        HideAllCursors();               
    else if (!bFirst)
        ShowAllCursors();

    if (bActivate)
    {
        if ( bFirst )
        {
            ScSplitPos eWin = aViewData.GetActivePart();
            OSL_ENSURE( pGridWin[eWin], "rottes Dokument, nicht alle SplitPos in GridWin" );
            if ( !pGridWin[eWin] )
            {
                eWin = SC_SPLIT_BOTTOMLEFT;
                if ( !pGridWin[eWin] )
                {
                    short i;
                    for ( i=0; i<4; i++ )
                    {
                        if ( pGridWin[i] )
                        {
                            eWin = (ScSplitPos) i;
                            break;  
                        }
                    }
                    OSL_ENSURE( i<4, "und BUMM" );
                }
                aViewData.SetActivePart( eWin );
            }
        }
        
        
        

        UpdateInputContext();
    }
    else
        pGridWin[aViewData.GetActivePart()]->ClickExtern();
}

void ScTabView::ActivatePart( ScSplitPos eWhich )
{
    ScSplitPos eOld = aViewData.GetActivePart();
    if ( eOld != eWhich )
    {
        bInActivatePart = true;

        bool bRefMode = SC_MOD()->IsFormulaMode();

        
        if ( aViewData.HasEditView(eOld) && !bRefMode )
            UpdateInputLine();

        ScHSplitPos eOldH = WhichH(eOld);
        ScVSplitPos eOldV = WhichV(eOld);
        ScHSplitPos eNewH = WhichH(eWhich);
        ScVSplitPos eNewV = WhichV(eWhich);
        bool bTopCap  = pColBar[eOldH] && pColBar[eOldH]->IsMouseCaptured();
        bool bLeftCap = pRowBar[eOldV] && pRowBar[eOldV]->IsMouseCaptured();

        bool bFocus = pGridWin[eOld]->HasFocus();
        bool bCapture = pGridWin[eOld]->IsMouseCaptured();
        if (bCapture)
            pGridWin[eOld]->ReleaseMouse();
        pGridWin[eOld]->ClickExtern();
        pGridWin[eOld]->HideCursor();
        pGridWin[eWhich]->HideCursor();
        aViewData.SetActivePart( eWhich );

        ScTabViewShell* pShell = aViewData.GetViewShell();
        pShell->WindowChanged();

        pSelEngine->SetWindow(pGridWin[eWhich]);
        pSelEngine->SetWhich(eWhich);
        pSelEngine->SetVisibleArea( Rectangle(Point(), pGridWin[eWhich]->GetOutputSizePixel()) );

        pGridWin[eOld]->MoveMouseStatus(*pGridWin[eWhich]);

        if ( bCapture || pGridWin[eWhich]->IsMouseCaptured() )
        {
            
            
            
            pGridWin[eWhich]->ReleaseMouse();
            pGridWin[eWhich]->StartTracking();
        }

        if ( bTopCap && pColBar[eNewH] )
        {
            pColBar[eOldH]->SetIgnoreMove(true);
            pColBar[eNewH]->SetIgnoreMove(false);
            pHdrSelEng->SetWindow( pColBar[eNewH] );
            long nWidth = pColBar[eNewH]->GetOutputSizePixel().Width();
            pHdrSelEng->SetVisibleArea( Rectangle( 0, LONG_MIN, nWidth-1, LONG_MAX ) );
            pColBar[eNewH]->CaptureMouse();
        }
        if ( bLeftCap && pRowBar[eNewV] )
        {
            pRowBar[eOldV]->SetIgnoreMove(true);
            pRowBar[eNewV]->SetIgnoreMove(false);
            pHdrSelEng->SetWindow( pRowBar[eNewV] );
            long nHeight = pRowBar[eNewV]->GetOutputSizePixel().Height();
            pHdrSelEng->SetVisibleArea( Rectangle( LONG_MIN, 0, LONG_MAX, nHeight-1 ) );
            pRowBar[eNewV]->CaptureMouse();
        }
        aHdrFunc.SetWhich(eWhich);

        pGridWin[eOld]->ShowCursor();
        pGridWin[eWhich]->ShowCursor();

        SfxInPlaceClient* pClient = aViewData.GetViewShell()->GetIPClient();
        bool bOleActive = ( pClient && pClient->IsObjectInPlaceActive() );

        
        
        if ( !bRefMode && !bOleActive )
            aViewData.GetViewShell()->SetWindow( pGridWin[eWhich] );

        if ( bFocus && !aViewData.IsAnyFillMode() && !bRefMode )
        {
            
            
            pGridWin[eWhich]->GrabFocus();
        }

        bInActivatePart = false;
    }
}

void ScTabView::HideListBox()
{
    for (sal_uInt16 i=0; i<4; i++)
        if (pGridWin[i])
            pGridWin[i]->ClickExtern();
}

void ScTabView::UpdateInputContext()
{
    ScGridWindow* pWin = pGridWin[aViewData.GetActivePart()];
    if (pWin)
        pWin->UpdateInputContext();

    if (pTabControl)
        pTabControl->UpdateInputContext();
}



long ScTabView::GetGridWidth( ScHSplitPos eWhich )
{
    ScSplitPos eGridWhich = ( eWhich == SC_SPLIT_LEFT ) ? SC_SPLIT_BOTTOMLEFT : SC_SPLIT_BOTTOMRIGHT;
    if (pGridWin[eGridWhich])
        return pGridWin[eGridWhich]->GetSizePixel().Width();
    else
        return 0;
}



long ScTabView::GetGridHeight( ScVSplitPos eWhich )
{
    ScSplitPos eGridWhich = ( eWhich == SC_SPLIT_TOP ) ? SC_SPLIT_TOPLEFT : SC_SPLIT_BOTTOMLEFT;
    if (pGridWin[eGridWhich])
        return pGridWin[eGridWhich]->GetSizePixel().Height();
    else
        return 0;
}

void ScTabView::UpdateInputLine()
{
    SC_MOD()->InputEnterHandler();
}

void ScTabView::ZoomChanged()
{
    ScInputHandler* pHdl = SC_MOD()->GetInputHdl(aViewData.GetViewShell());
    if (pHdl)
        pHdl->SetRefScale( aViewData.GetZoomX(), aViewData.GetZoomY() );

    UpdateFixPos();

    UpdateScrollBars();

    
    
    
    
    
    sal_uInt32 a;

    for(a = 0L; a < 4L; a++)
    {
        if(pGridWin[a])
        {
            pGridWin[a]->SetMapMode(pGridWin[a]->GetDrawMapMode());
        }
    }

    SetNewVisArea();

    InterpretVisible();     

    SfxBindings& rBindings = aViewData.GetBindings();
    rBindings.Invalidate( SID_ATTR_ZOOM );
    rBindings.Invalidate( SID_ATTR_ZOOMSLIDER );

    HideNoteMarker();

    
    ScGridWindow* pWin = pGridWin[aViewData.GetActivePart()];

    if ( pWin && aViewData.HasEditView( aViewData.GetActivePart() ) )
    {
        
        pWin->flushOverlayManager();

        
        
        pWin->SetMapMode( aViewData.GetLogicMode() );
        UpdateEditView();
    }
}

void ScTabView::CheckNeedsRepaint()
{
    sal_uInt16 i;
    for (i=0; i<4; i++)
        if ( pGridWin[i] && pGridWin[i]->IsVisible() )
            pGridWin[i]->CheckNeedsRepaint();
}

bool ScTabView::NeedsRepaint()
{
    for (size_t i = 0; i < 4; i++)
        if (pGridWin[i] && pGridWin[i]->IsVisible() && pGridWin[i]->NeedsRepaint())
            return true;
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
