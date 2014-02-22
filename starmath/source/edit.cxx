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

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <toolkit/helper/vclunohelper.hxx>


#include "starmath.hrc"

#include <vcl/menu.hxx>
#include <vcl/settings.hxx>

#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editstat.hxx>
#include <editeng/eeitem.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/intitem.hxx>
#include <svl/itempool.hxx>
#include <svl/stritem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/lrspitem.hxx>
#include <svl/itemset.hxx>
#include <editeng/fontitem.hxx>
#include <sfx2/viewfrm.hxx>

#include "edit.hxx"
#include "view.hxx"
#include "document.hxx"
#include "config.hxx"
#include "accessibility.hxx"

#define SCROLL_LINE         24


using namespace com::sun::star::accessibility;
using namespace com::sun::star;
using namespace com::sun::star::uno;




void SmGetLeftSelectionPart(const ESelection &rSel,
                            sal_Int32 &nPara, sal_uInt16 &nPos)
    
{
    
    if (    rSel.nStartPara <  rSel.nEndPara
        ||  (rSel.nStartPara == rSel.nEndPara  &&  rSel.nStartPos < rSel.nEndPos) )
    {   nPara = rSel.nStartPara;
        nPos  = rSel.nStartPos;
    }
    else
    {   nPara = rSel.nEndPara;
        nPos  = rSel.nEndPos;
    }
}

bool SmEditWindow::IsInlineEditEnabled()
{
    SmViewShell *pView = GetView();
    return pView ? pView->IsInlineEditEnabled() : false;
}



SmEditWindow::SmEditWindow( SmCmdBoxWindow &rMyCmdBoxWin ) :
    Window              (&rMyCmdBoxWin),
    DropTargetHelper    ( this ),
    pAccessible         (0),
    rCmdBox             (rMyCmdBoxWin),
    pEditView           (0),
    pHScrollBar         (0),
    pVScrollBar         (0),
    pScrollBox          (0)
{
    SetHelpId(HID_SMA_COMMAND_WIN_EDIT);
    SetMapMode(MAP_PIXEL);

    
    rCmdBox.GetEditWindow()->EnableRTL( false );

    ApplyColorConfigValues( SM_MOD()->GetColorConfig() );

    
    SetBackground( GetSettings().GetStyleSettings().GetWindowColor() );

    aModifyTimer.SetTimeoutHdl(LINK(this, SmEditWindow, ModifyTimerHdl));
    aModifyTimer.SetTimeout(500);

    if (!IsInlineEditEnabled())
    {
        aCursorMoveTimer.SetTimeoutHdl(LINK(this, SmEditWindow, CursorMoveTimerHdl));
        aCursorMoveTimer.SetTimeout(500);
    }

    
    
    Show();
}


SmEditWindow::~SmEditWindow()
{
    aModifyTimer.Stop();

    StartCursorMove();

    
    
    
    if (pAccessible)
        pAccessible->ClearWin();    
    
    

    if (pEditView)
    {
        EditEngine *pEditEngine = pEditView->GetEditEngine();
        if (pEditEngine)
        {
            pEditEngine->SetStatusEventHdl( Link() );
            pEditEngine->RemoveView( pEditView );
        }
    }
    delete pEditView;
    delete pHScrollBar;
    delete pVScrollBar;
    delete pScrollBox;
}

void SmEditWindow::StartCursorMove()
{
    if (!IsInlineEditEnabled())
        aCursorMoveTimer.Stop();
}

void SmEditWindow::InvalidateSlots()
{
    SfxBindings& rBind = GetView()->GetViewFrame()->GetBindings();
    rBind.Invalidate(SID_COPY);
    rBind.Invalidate(SID_CUT);
    rBind.Invalidate(SID_DELETE);
}

SmViewShell * SmEditWindow::GetView()
{
    return rCmdBox.GetView();
}


SmDocShell * SmEditWindow::GetDoc()
{
    SmViewShell *pView = rCmdBox.GetView();
    return pView ? pView->GetDoc() : 0;
}


EditEngine * SmEditWindow::GetEditEngine()
{
    EditEngine *pEditEng = 0;
    if (pEditView)
        pEditEng = pEditView->GetEditEngine();
    else
    {
        SmDocShell *pDoc = GetDoc();
        if (pDoc)
            pEditEng = &pDoc->GetEditEngine();
    }
    return pEditEng;
}


SfxItemPool * SmEditWindow::GetEditEngineItemPool()
{
    SmDocShell *pDoc = GetDoc();
    return pDoc ? &pDoc->GetEditEngineItemPool() : 0;
}

void SmEditWindow::ApplyColorConfigValues( const svtools::ColorConfig &rColorCfg )
{
    
#if OSL_DEBUG_LEVEL > 1

#endif
    SetTextColor( rColorCfg.GetColorValue(svtools::FONTCOLOR).nColor );
    Invalidate();
}

void SmEditWindow::DataChanged( const DataChangedEvent& )
{
    const StyleSettings aSettings( GetSettings().GetStyleSettings() );

    ApplyColorConfigValues( SM_MOD()->GetColorConfig() );
    SetBackground( aSettings.GetWindowColor() );

    
    
    SetPointFont( aSettings.GetFieldFont() /*aSettings.GetAppFont()*/ );

    EditEngine  *pEditEngine = GetEditEngine();
    SfxItemPool *pEditEngineItemPool = GetEditEngineItemPool();

    if (pEditEngine && pEditEngineItemPool)
    {
        
        
        

        pEditEngine->SetDefTab(sal_uInt16(GetTextWidth(OUString("XXXX"))));

        SetEditEngineDefaultFonts(*pEditEngineItemPool);

        
        
        
        OUString aTxt( pEditEngine->GetText( LINEEND_LF ) );
        pEditEngine->Clear();   
        pEditEngine->SetText( aTxt );
    }

    AdjustScrollBars();
    Resize();
}

IMPL_LINK( SmEditWindow, ModifyTimerHdl, Timer *, EMPTYARG /*pTimer*/ )
{
    UpdateStatus();
    aModifyTimer.Stop();
    return 0;
}

IMPL_LINK(SmEditWindow, CursorMoveTimerHdl, Timer *, EMPTYARG /*pTimer*/)
    
    
    
{
    if (IsInlineEditEnabled())
        return 0;

    ESelection aNewSelection(GetSelection());

    if (!aNewSelection.IsEqual(aOldSelection))
    {
        SmViewShell *pView = rCmdBox.GetView();
        if (pView)
        {
            
            sal_Int32  nRow;
            sal_uInt16 nCol;
            SmGetLeftSelectionPart(aNewSelection, nRow, nCol);
            nRow++;
            nCol++;
            pView->GetGraphicWindow().SetCursorPos(static_cast<sal_uInt16>(nRow), nCol);
            aOldSelection = aNewSelection;
        }
    }
    aCursorMoveTimer.Stop();

    return 0;
}

void SmEditWindow::Resize()
{
    if (!pEditView)
        CreateEditView();

    if (pEditView)
    {
        pEditView->SetOutputArea(AdjustScrollBars());
        pEditView->ShowCursor();

        OSL_ENSURE( pEditView->GetEditEngine(), "EditEngine missing" );
        const long nMaxVisAreaStart = pEditView->GetEditEngine()->GetTextHeight() -
                                      pEditView->GetOutputArea().GetHeight();
        if (pEditView->GetVisArea().Top() > nMaxVisAreaStart)
        {
            Rectangle aVisArea(pEditView->GetVisArea() );
            aVisArea.Top() = (nMaxVisAreaStart > 0 ) ? nMaxVisAreaStart : 0;
            aVisArea.SetSize(pEditView->GetOutputArea().GetSize());
            pEditView->SetVisArea(aVisArea);
            pEditView->ShowCursor();
        }
        InitScrollBars();
    }
    Invalidate();
}

void SmEditWindow::MouseButtonUp(const MouseEvent &rEvt)
{
    if (pEditView)
        pEditView->MouseButtonUp(rEvt);
    else
        Window::MouseButtonUp (rEvt);

    if (!IsInlineEditEnabled())
        CursorMoveTimerHdl(&aCursorMoveTimer);
    InvalidateSlots();
}

void SmEditWindow::MouseButtonDown(const MouseEvent &rEvt)
{
    if (pEditView)
        pEditView->MouseButtonDown(rEvt);
    else
        Window::MouseButtonDown (rEvt);

    GrabFocus();
}

void SmEditWindow::Command(const CommandEvent& rCEvt)
{
    bool bForwardEvt = true;
    if (rCEvt.GetCommand() == COMMAND_CONTEXTMENU)
    {
        GetParent()->ToTop();

        Point aPoint = rCEvt.GetMousePosPixel();
        PopupMenu* pPopupMenu = new PopupMenu(SmResId(RID_COMMANDMENU));

        
        Menu* pMenu = NULL;
        ::com::sun::star::ui::ContextMenuExecuteEvent aEvent;
        aEvent.SourceWindow = VCLUnoHelper::GetInterface( this );
        aEvent.ExecutePosition.X = aPoint.X();
        aEvent.ExecutePosition.Y = aPoint.Y();
        OUString sDummy;
        if ( GetView()->TryContextMenuInterception( *pPopupMenu, sDummy, pMenu, aEvent ) )
        {
            if ( pMenu )
            {
                delete pPopupMenu;
                pPopupMenu = (PopupMenu*) pMenu;
            }
        }

        pPopupMenu->SetSelectHdl(LINK(this, SmEditWindow, MenuSelectHdl));

        pPopupMenu->Execute( this, aPoint );
        delete pPopupMenu;
        bForwardEvt = false;
    }
    else if (rCEvt.GetCommand() == COMMAND_WHEEL)
        bForwardEvt = !HandleWheelCommands( rCEvt );

    if (bForwardEvt)
    {
        if (pEditView)
            pEditView->Command( rCEvt );
        else
            Window::Command (rCEvt);
    }
}


bool SmEditWindow::HandleWheelCommands( const CommandEvent &rCEvt )
{
    bool bCommandHandled = false;    
                                    
                                    

    const CommandWheelData* pWData = rCEvt.GetWheelData();
    if (pWData)
    {
        if (COMMAND_WHEEL_ZOOM == pWData->GetMode())
            bCommandHandled = true;     
        else
            bCommandHandled = HandleScrollCommand( rCEvt, pHScrollBar, pVScrollBar);
    }

    return bCommandHandled;
}


IMPL_LINK_INLINE_START( SmEditWindow, MenuSelectHdl, Menu *, pMenu )
{
    SmViewShell *pViewSh = rCmdBox.GetView();
    if (pViewSh)
        pViewSh->GetViewFrame()->GetDispatcher()->Execute(
                SID_INSERTCOMMAND, SFX_CALLMODE_STANDARD,
                new SfxInt16Item(SID_INSERTCOMMAND, pMenu->GetCurItemId()), 0L);
    return 0;
}
IMPL_LINK_INLINE_END( SmEditWindow, MenuSelectHdl, Menu *, pMenu )

void SmEditWindow::KeyInput(const KeyEvent& rKEvt)
{
    if (rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE)
    {
        bool bCallBase = true;
        SfxViewShell* pViewShell = GetView();
        if ( pViewShell && pViewShell->ISA(SmViewShell) )
        {
            
            bCallBase = !pViewShell->Escape();
        }
        if ( bCallBase )
            Window::KeyInput( rKEvt );
    }
    else
    {
        StartCursorMove();

        bool autoClose = false;
        ESelection aSelection = pEditView->GetSelection();
        
        
        aSelection.Adjust();
        OUString selected = pEditView->GetEditEngine()->GetText(aSelection);

        if (selected.trim() == "<?>")
            autoClose = true;
        else if (selected.isEmpty() && !aSelection.HasRange())
        {
            selected = pEditView->GetEditEngine()->GetText(aSelection.nEndPara);
            if (!selected.isEmpty())
            {
                sal_Int32 index = selected.indexOf("\n", aSelection.nEndPos);
                if (index != -1)
                {
                    selected = selected.copy(index, sal_Int32(aSelection.nEndPos-index));
                    if (selected.trim().isEmpty())
                        autoClose = true;
                }
                else
                {
                    sal_Int32 length = selected.getLength();
                    if (aSelection.nEndPos == length)
                        autoClose = true;
                    else
                    {
                        selected = selected.copy(aSelection.nEndPos);
                        if (selected.trim().isEmpty())
                            autoClose = true;
                    }
                }
            }
            else
                autoClose = true;
        }

        if (!pEditView)
            CreateEditView();
        if ( !pEditView->PostKeyEvent(rKEvt) )
        {
            SmViewShell *pView = GetView();
            if ( pView && !pView->KeyInput(rKEvt) )
            {
                
                Flush();
                if ( aModifyTimer.IsActive() )
                    aModifyTimer.Stop();
                Window::KeyInput(rKEvt);
            }
            else
            {
                
                
                SfxViewShell* pVShell = GetView();
                if ( pVShell && pVShell->ISA(SmViewShell) &&
                     ((SmViewShell*)pVShell)->GetGraphicWindow().HasFocus() )
                {
                    GrabFocus();
                }
            }
        }
        else
        {
            
            
            SmDocShell *pDocShell = GetDoc();
            if (pDocShell)
                pDocShell->SetModified( GetEditEngine()->IsModified() );

            aModifyTimer.Start();
        }

        
        sal_Unicode charCode = rKEvt.GetCharCode();
        OUString close;

        if (charCode == '{')
            close = "  }";
        else if (charCode == '[')
            close = "  ]";
        else if (charCode == '(')
            close = "  )";

        
        if (!close.isEmpty() && autoClose)
        {
            pEditView->InsertText(close);
            
            aSelection.nStartPos += 2;
            aSelection.nEndPos = aSelection.nStartPos;
            pEditView->SetSelection(aSelection);
        }

        InvalidateSlots();
    }
}

void SmEditWindow::Paint(const Rectangle& rRect)
{
    if (!pEditView)
        CreateEditView();
    pEditView->Paint(rRect);
}

void SmEditWindow::CreateEditView()
{
    EditEngine *pEditEngine = GetEditEngine();

    
    
    if (!pEditView && pEditEngine)
    {
        pEditView = new EditView( pEditEngine, this );
        pEditEngine->InsertView( pEditView );

        if (!pVScrollBar)
            pVScrollBar = new ScrollBar(this, WinBits(WB_VSCROLL));
        if (!pHScrollBar)
            pHScrollBar = new ScrollBar(this, WinBits(WB_HSCROLL));
        if (!pScrollBox)
            pScrollBox  = new ScrollBarBox(this);
        pVScrollBar->SetScrollHdl(LINK(this, SmEditWindow, ScrollHdl));
        pHScrollBar->SetScrollHdl(LINK(this, SmEditWindow, ScrollHdl));
        pVScrollBar->EnableDrag( true );
        pHScrollBar->EnableDrag( true );

        pEditView->SetOutputArea(AdjustScrollBars());

        ESelection eSelection;

        pEditView->SetSelection(eSelection);
        Update();
        pEditView->ShowCursor(true, true);

        pEditEngine->SetStatusEventHdl( LINK(this, SmEditWindow, EditStatusHdl) );
        SetPointer(pEditView->GetPointer());

        SetScrollBarRanges();
    }
}


IMPL_LINK( SmEditWindow, EditStatusHdl, EditStatus *, EMPTYARG /*pStat*/ )
{
    if (!pEditView)
        return 1;
    else
    {
        Resize();
        return 0;
    }
}

IMPL_LINK_INLINE_START( SmEditWindow, ScrollHdl, ScrollBar *, EMPTYARG /*pScrollBar*/ )
{
    OSL_ENSURE(pEditView, "EditView missing");
    if (pEditView)
    {
        pEditView->SetVisArea(Rectangle(Point(pHScrollBar->GetThumbPos(),
                                            pVScrollBar->GetThumbPos()),
                                        pEditView->GetVisArea().GetSize()));
        pEditView->Invalidate();
    }
    return 0;
}
IMPL_LINK_INLINE_END( SmEditWindow, ScrollHdl, ScrollBar *, pScrollBar )

Rectangle SmEditWindow::AdjustScrollBars()
{
    const Size aOut( GetOutputSizePixel() );
    Point aPoint;
    Rectangle aRect( aPoint, aOut );

    if (pVScrollBar && pHScrollBar && pScrollBox)
    {
        const long nTmp = GetSettings().GetStyleSettings().GetScrollBarSize();
        Point aPt( aRect.TopRight() ); aPt.X() -= nTmp -1L;
        pVScrollBar->SetPosSizePixel( aPt, Size(nTmp, aOut.Height() - nTmp));

        aPt = aRect.BottomLeft(); aPt.Y() -= nTmp - 1L;
        pHScrollBar->SetPosSizePixel( aPt, Size(aOut.Width() - nTmp, nTmp));

        aPt.X() = pHScrollBar->GetSizePixel().Width();
        aPt.Y() = pVScrollBar->GetSizePixel().Height();
        pScrollBox->SetPosSizePixel(aPt, Size(nTmp, nTmp ));

        aRect.Right()  = aPt.X() - 2;
        aRect.Bottom() = aPt.Y() - 2;
    }
    return aRect;
}

void SmEditWindow::SetScrollBarRanges()
{
    
    EditEngine *pEditEngine = GetEditEngine();
    if (pVScrollBar && pHScrollBar && pEditEngine && pEditView)
    {
        long nTmp = pEditEngine->GetTextHeight();
        pVScrollBar->SetRange(Range(0, nTmp));
        pVScrollBar->SetThumbPos(pEditView->GetVisArea().Top());

        nTmp = pEditEngine->GetPaperSize().Width();
        pHScrollBar->SetRange(Range(0,nTmp));
        pHScrollBar->SetThumbPos(pEditView->GetVisArea().Left());
    }
}

void SmEditWindow::InitScrollBars()
{
    if (pVScrollBar && pHScrollBar && pScrollBox && pEditView)
    {
        const Size aOut( pEditView->GetOutputArea().GetSize() );
        pVScrollBar->SetVisibleSize(aOut.Height());
        pVScrollBar->SetPageSize(aOut.Height() * 8 / 10);
        pVScrollBar->SetLineSize(aOut.Height() * 2 / 10);

        pHScrollBar->SetVisibleSize(aOut.Width());
        pHScrollBar->SetPageSize(aOut.Width() * 8 / 10);
        pHScrollBar->SetLineSize(SCROLL_LINE );

        SetScrollBarRanges();

        pVScrollBar->Show();
        pHScrollBar->Show();
        pScrollBox->Show();
    }
}


OUString SmEditWindow::GetText() const
{
    OUString aText;
    EditEngine *pEditEngine = const_cast< SmEditWindow* >(this)->GetEditEngine();
    OSL_ENSURE( pEditEngine, "EditEngine missing" );
    if (pEditEngine)
        aText = pEditEngine->GetText( LINEEND_LF );
    return aText;
}


void SmEditWindow::SetText(const OUString& rText)
{
    EditEngine *pEditEngine = GetEditEngine();
    OSL_ENSURE( pEditEngine, "EditEngine missing" );
    if (pEditEngine  &&  !pEditEngine->IsModified())
    {
        if (!pEditView)
            CreateEditView();

        ESelection eSelection = pEditView->GetSelection();

        pEditEngine->SetText(rText);
        pEditEngine->ClearModifyFlag();

        
        
        aModifyTimer.Start();

        pEditView->SetSelection(eSelection);
    }
}


void SmEditWindow::GetFocus()
{
    Window::GetFocus();

    if (xAccessible.is())
    {
        
        ::accessibility::AccessibleTextHelper *pHelper = pAccessible->GetTextHelper();
        if (pHelper)
            pHelper->SetFocus(true);
    }

    if (!pEditView)
         CreateEditView();
    EditEngine *pEditEngine = GetEditEngine();
    if (pEditEngine)
        pEditEngine->SetStatusEventHdl( LINK(this, SmEditWindow, EditStatusHdl) );

    
    if(GetView() && IsInlineEditEnabled())
        GetView()->SetInsertIntoEditWindow(true);
}


void SmEditWindow::LoseFocus()
{
    EditEngine *pEditEngine = GetEditEngine();
    if (pEditEngine)
        pEditEngine->SetStatusEventHdl( Link() );

    Window::LoseFocus();

    if (xAccessible.is())
    {
        
        ::accessibility::AccessibleTextHelper *pHelper = pAccessible->GetTextHelper();
        if (pHelper)
            pHelper->SetFocus(false);
    }
}


bool SmEditWindow::IsAllSelected() const
{
    bool bRes = false;
    EditEngine *pEditEngine = ((SmEditWindow *) this)->GetEditEngine();
    OSL_ENSURE( pEditView, "NULL pointer" );
    OSL_ENSURE( pEditEngine, "NULL pointer" );
    if (pEditEngine  &&  pEditView)
    {
        ESelection eSelection( pEditView->GetSelection() );
        sal_Int32 nParaCnt = pEditEngine->GetParagraphCount();
        if (!(nParaCnt - 1))
        {
            sal_Int32 nTextLen = pEditEngine->GetText( LINEEND_LF ).getLength();
            bRes = !eSelection.nStartPos && (eSelection.nEndPos == nTextLen - 1);
        }
        else
        {
            bRes = !eSelection.nStartPara && (eSelection.nEndPara == nParaCnt - 1);
        }
    }
    return bRes;
}

void SmEditWindow::SelectAll()
{
    OSL_ENSURE( pEditView, "NULL pointer" );
    if (pEditView)
    {
        
        pEditView->SetSelection( ESelection( 0, 0, EE_PARA_ALL, EE_TEXTPOS_ALL ) );
    }
}

void SmEditWindow::InsertCommand(sal_uInt16 nCommand)
{
    OSL_ENSURE( pEditView, "EditView missing" );
    if (pEditView)
    {
        ESelection aSelection = pEditView->GetSelection();

        OSL_ENSURE( pEditView, "NULL pointer" );
        OUString aText = SM_RESSTR(nCommand);

        OUString aCurrentFormula = pEditView->GetEditEngine()->GetText();
        sal_Int32 nStartIndex = 0;
        sal_Int32 nEndIndex = 0;

        
        for (sal_Int32 nParaPos = 0; nParaPos < aSelection.nStartPara; nParaPos++)
             nStartIndex = aCurrentFormula.indexOf("\n", nStartIndex) + 1;

        nStartIndex += aSelection.nStartPos;

        
        for (sal_Int32 nParaPos = 0; nParaPos < aSelection.nEndPara; nParaPos++)
             nEndIndex = aCurrentFormula.indexOf("\n", nEndIndex) + 1;

        nEndIndex += aSelection.nEndPos;

        
        if (nEndIndex < aCurrentFormula.getLength() &&
            aCurrentFormula[nEndIndex] == ' ')
            aText = aText.trim();

        
        if (aSelection.nStartPos > 0 && aCurrentFormula[nStartIndex - 1] != ' ')
            aText = " " + aText;

        pEditView->InsertText(aText);

        
        aSelection.nEndPara = aSelection.nStartPara;
        if (HasMark(aText))
        {
            aSelection.nEndPos = aSelection.nStartPos;
            pEditView->SetSelection(aSelection);
            SelNextMark();
        }
        else
        {   
            aSelection.nEndPos = aSelection.nStartPos + aText.getLength();
            aSelection.nStartPos = aSelection.nEndPos;
            pEditView->SetSelection(aSelection);
        }

        aModifyTimer.Start();
        StartCursorMove();
        GrabFocus();
    }
}

void SmEditWindow::MarkError(const Point &rPos)
{
    OSL_ENSURE( pEditView, "EditView missing" );
    if (pEditView)
    {
        const sal_uInt16        nCol = sal::static_int_cast< sal_uInt16 >(rPos.X());
        const sal_uInt16        nRow = sal::static_int_cast< sal_uInt16 >(rPos.Y() - 1);

        pEditView->SetSelection(ESelection(nRow, nCol - 1, nRow, nCol));
        GrabFocus();
    }
}


void SmEditWindow::SelNextMark()
{
    EditEngine *pEditEngine = GetEditEngine();
    OSL_ENSURE( pEditView, "NULL pointer" );
    OSL_ENSURE( pEditEngine, "NULL pointer" );
    if (pEditEngine  &&  pEditView)
    {
        ESelection eSelection = pEditView->GetSelection();
        sal_Int32 nPos = eSelection.nEndPos;
        sal_Int32 nCounts = pEditEngine->GetParagraphCount();

        while (eSelection.nEndPara < nCounts)
        {
            OUString aText = pEditEngine->GetText(eSelection.nEndPara);
            nPos = aText.indexOf("<?>", nPos);
            if (nPos != -1)
            {
                pEditView->SetSelection(ESelection(
                    eSelection.nEndPara, nPos, eSelection.nEndPara, nPos + 3));
                break;
            }

            nPos = 0;
            eSelection.nEndPara++;
        }
    }
}

void SmEditWindow::SelPrevMark()
{
    EditEngine *pEditEngine = GetEditEngine();
    OSL_ENSURE( pEditEngine, "NULL pointer" );
    OSL_ENSURE( pEditView, "NULL pointer" );
    if (pEditEngine  &&  pEditView)
    {
        ESelection eSelection = pEditView->GetSelection();
        sal_Int32 nPos = -1;
        sal_Int32 nMax = eSelection.nStartPos;
        OUString aText(pEditEngine->GetText(eSelection.nStartPara));
        OUString aMark("<?>");
        sal_Int32 nCounts = pEditEngine->GetParagraphCount();

        do
        {
            sal_Int32 nMarkIndex = aText.indexOf(aMark);
            while ((nMarkIndex < nMax) && (nMarkIndex != -1))
            {
                nPos = nMarkIndex;
                nMarkIndex = aText.indexOf(aMark, nMarkIndex + 1);
            }

            if (nPos == -1)
            {
                eSelection.nStartPara--;
                aText = pEditEngine->GetText(eSelection.nStartPara);
                nMax = aText.getLength();
            }
        }
        while ((eSelection.nStartPara < nCounts) &&
            (nPos == -1));

        if (nPos != -1)
        {
            pEditView->SetSelection(ESelection(
                eSelection.nStartPara, nPos, eSelection.nStartPara, nPos + 3));
        }
    }
}

bool SmEditWindow::HasMark(const OUString& rText) const
    
{
    return rText.indexOf("<?>") != -1;
}

void SmEditWindow::MouseMove(const MouseEvent &rEvt)
{
    if (pEditView)
        pEditView->MouseMove(rEvt);
}

sal_Int8 SmEditWindow::AcceptDrop( const AcceptDropEvent& /*rEvt*/ )
{
    return pEditView ? /*pEditView->QueryDrop( rEvt )*/DND_ACTION_NONE: DND_ACTION_NONE;
}

sal_Int8 SmEditWindow::ExecuteDrop( const ExecuteDropEvent& /*rEvt*/ )
{
    return pEditView ? /*pEditView->Drop( rEvt )*/DND_ACTION_NONE : DND_ACTION_NONE;
}

ESelection SmEditWindow::GetSelection() const
{
    
    
    
    ESelection eSel;
    if (pEditView)
        eSel = pEditView->GetSelection();
    return eSel;
}

void SmEditWindow::SetSelection(const ESelection &rSel)
{
    OSL_ENSURE( pEditView, "NULL pointer" );
    if (pEditView)
        pEditView->SetSelection(rSel);
    InvalidateSlots();
}

bool SmEditWindow::IsEmpty() const
{
    EditEngine *pEditEngine = ((SmEditWindow *) this)->GetEditEngine();
    bool bEmpty = ( pEditEngine ? pEditEngine->GetTextLen() == 0 : false);
    return bEmpty;
}

bool SmEditWindow::IsSelected() const
{
    return pEditView ? pEditView->HasSelection() : false;
}


void SmEditWindow::UpdateStatus( bool bSetDocModified )
{
    SmModule *pMod = SM_MOD();
    if (pMod && pMod->GetConfig()->IsAutoRedraw())
        Flush();
    if ( bSetDocModified )
        GetDoc()->SetModified(true);
}

void SmEditWindow::Cut()
{
    OSL_ENSURE( pEditView, "EditView missing" );
    if (pEditView)
    {
        pEditView->Cut();
        UpdateStatus(true);
    }
}

void SmEditWindow::Copy()
{
    OSL_ENSURE( pEditView, "EditView missing" );
    if (pEditView)
        pEditView->Copy();
}

void SmEditWindow::Paste()
{
    OSL_ENSURE( pEditView, "EditView missing" );
    if (pEditView)
    {
        pEditView->Paste();
        UpdateStatus(true);
    }
}

void SmEditWindow::Delete()
{
    OSL_ENSURE( pEditView, "EditView missing" );
    if (pEditView)
    {
        pEditView->DeleteSelected();
        UpdateStatus(true);
    }
}

void SmEditWindow::InsertText(const OUString& rText)
{
    OSL_ENSURE( pEditView, "EditView missing" );
    if (pEditView)
    {
        
        
        ESelection aSelection = pEditView->GetSelection();
        OUString aCurrentFormula = pEditView->GetEditEngine()->GetText();
        sal_Int32 nStartIndex = 0;
        sal_Int32 nEndIndex = 0;

        
        for (sal_Int32 nParaPos = 0; nParaPos < aSelection.nStartPara; nParaPos++)
             nStartIndex = aCurrentFormula.indexOf("\n", nStartIndex) + 1;

        nStartIndex += aSelection.nStartPos;

        
        for (sal_Int32 nParaPos = 0; nParaPos < aSelection.nEndPara; nParaPos++)
             nEndIndex = aCurrentFormula.indexOf("\n", nEndIndex) + 1;

        nEndIndex += aSelection.nEndPos;

        
        
        OUString string(rText);

        
        if (aSelection.nStartPos > 0 && aCurrentFormula[nStartIndex - 1] != ' ')
            string = " " + string;

        /*
          fdo#65588 -  Elements Dock: Scrollbar moves into input window
          This change "solves" the visual problem. But I don't think so
          this is the best solution.
        */
        pVScrollBar->Hide();
        pHScrollBar->Hide();
        pEditView->InsertText(string);
        AdjustScrollBars();
        pVScrollBar->Show();
        pHScrollBar->Show();

        
        aSelection.nEndPara = aSelection.nStartPara;
        if (HasMark(string))
        {
            aSelection.nEndPos = aSelection.nStartPos;
            pEditView->SetSelection(aSelection);
            SelNextMark();
        }
        else
        {   
            aSelection.nEndPos = aSelection.nStartPos + string.getLength();
            aSelection.nStartPos = aSelection.nEndPos;
            pEditView->SetSelection(aSelection);
        }

        aModifyTimer.Start();
        StartCursorMove();
        GrabFocus();
    }
}

void SmEditWindow::Flush()
{
    EditEngine *pEditEngine = GetEditEngine();
    if (pEditEngine  &&  pEditEngine->IsModified())
    {
        pEditEngine->ClearModifyFlag();
        SmViewShell *pViewSh = rCmdBox.GetView();
        if (pViewSh)
        {
            pViewSh->GetViewFrame()->GetDispatcher()->Execute(
                    SID_TEXT, SFX_CALLMODE_STANDARD,
                    new SfxStringItem(SID_TEXT, GetText()), 0L);
        }
    }
    if (aCursorMoveTimer.IsActive())
    {
        aCursorMoveTimer.Stop();
        CursorMoveTimerHdl(&aCursorMoveTimer);
    }
}


void SmEditWindow::DeleteEditView( SmViewShell & /*rView*/ )
{
    if (pEditView)
    {
        EditEngine *pEditEngine = pEditView->GetEditEngine();
        if (pEditEngine)
        {
            pEditEngine->SetStatusEventHdl( Link() );
            pEditEngine->RemoveView( pEditView );
        }
        delete pEditView;
        pEditView = 0;
    }
}


uno::Reference< XAccessible > SmEditWindow::CreateAccessible()
{
    if (!pAccessible)
    {
        pAccessible = new SmEditAccessible( this );
        xAccessible = pAccessible;
        pAccessible->Init();
    }
    return xAccessible;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
