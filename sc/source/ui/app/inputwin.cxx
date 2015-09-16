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

#include <algorithm>

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <sfx2/app.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/editview.hxx>
#include <editeng/editstat.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/lspcitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/event.hxx>
#include <sfx2/imgmgr.hxx>
#include <stdlib.h>
#include <editeng/scriptspaceitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <vcl/cursor.hxx>
#include <vcl/help.hxx>
#include <vcl/settings.hxx>
#include <svl/stritem.hxx>

#include "inputwin.hxx"
#include "scmod.hxx"
#include "uiitems.hxx"
#include "global.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "reffact.hxx"
#include "editutil.hxx"
#include "inputhdl.hxx"
#include "tabvwsh.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "appoptio.hxx"
#include "rangenam.hxx"
#include <formula/compiler.hrc>
#include "dbdata.hxx"
#include "rangeutl.hxx"
#include "docfunc.hxx"
#include "funcdesc.hxx"
#include "markdata.hxx"
#include <editeng/fontitem.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include "AccessibleEditObject.hxx"
#include "AccessibleText.hxx"
#include <svtools/miscopt.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XController.hpp>

#define THESIZE             1000000 // Should be more than enough!
#define TBX_WINDOW_HEIGHT   22 // in pixel - TODO: The same on all systems?
#define LEFT_OFFSET         5
#define INPUTWIN_MULTILINES 6
const long BUTTON_OFFSET = 2; ///< space between input line and the button to expand / collapse
const long ADDITIONAL_BORDER = 1; ///< height of the line at the bottom
const long ADDITIONAL_SPACE = 4; ///< additional vertical space when the multiline edit has more lines

using com::sun::star::uno::Reference;
using com::sun::star::uno::UNO_QUERY;

using com::sun::star::frame::XLayoutManager;
using com::sun::star::frame::XModel;
using com::sun::star::frame::XFrame;
using com::sun::star::frame::XController;
using com::sun::star::beans::XPropertySet;

enum ScNameInputType
{
    SC_NAME_INPUT_CELL,
    SC_NAME_INPUT_RANGE,
    SC_NAME_INPUT_NAMEDRANGE,
    SC_NAME_INPUT_DATABASE,
    SC_NAME_INPUT_ROW,
    SC_NAME_INPUT_SHEET,
    SC_NAME_INPUT_DEFINE,
    SC_NAME_INPUT_BAD_NAME,
    SC_NAME_INPUT_BAD_SELECTION,
    SC_MANAGE_NAMES
};

ScTextWndBase::ScTextWndBase( vcl::Window* pParent,  WinBits nStyle )
    : Window ( pParent, nStyle )
{
    if ( IsNativeControlSupported( CTRL_EDITBOX, PART_ENTIRE_CONTROL ) )
    {
        SetType( WINDOW_CALCINPUTLINE );
        SetBorderStyle( WindowBorderStyle::NWF );
    }
}

//  class ScInputWindowWrapper

SFX_IMPL_CHILDWINDOW_WITHID(ScInputWindowWrapper,FID_INPUTLINE_STATUS)

ScInputWindowWrapper::ScInputWindowWrapper( vcl::Window*          pParentP,
                                            sal_uInt16           nId,
                                            SfxBindings*     pBindings,
                                            SfxChildWinInfo* /* pInfo */ )
    :   SfxChildWindow( pParentP, nId )
{
    VclPtr<ScInputWindow> pWin = VclPtr<ScInputWindow>::Create( pParentP, pBindings );
    SetWindow( pWin );

    pWin->Show();

    pWin->SetSizePixel( pWin->CalcWindowSizePixel() );

    SetAlignment(SfxChildAlignment::LOWESTTOP);
    pBindings->Invalidate( FID_TOGGLEINPUTLINE );
}

/**
 * GetInfo is disposed of if there's a SFX_IMPL_TOOLBOX!
 */
SfxChildWinInfo ScInputWindowWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    return aInfo;
}

#define IMAGE(id) pImgMgr->SeekImage(id)
static inline bool lcl_isExperimentalMode()
{
    // make inputbar feature on by default, leave the switch for the
    // moment in case we need to back it out easily
    return true;
}

//  class ScInputWindow

static VclPtr<ScTextWndBase> lcl_chooseRuntimeImpl( vcl::Window* pParent, SfxBindings* pBind )
{
    ScTabViewShell* pViewSh = NULL;
    SfxDispatcher* pDisp = pBind->GetDispatcher();
    if ( pDisp )
    {
        SfxViewFrame* pViewFrm = pDisp->GetFrame();
        if ( pViewFrm )
            pViewSh = PTR_CAST( ScTabViewShell, pViewFrm->GetViewShell() );
    }

    if ( !lcl_isExperimentalMode() )
        return VclPtr<ScTextWnd>::Create( pParent, pViewSh );
    return VclPtr<ScInputBarGroup>::Create( pParent, pViewSh );
}

ScInputWindow::ScInputWindow( vcl::Window* pParent, SfxBindings* pBind ) :
        // With WB_CLIPCHILDREN otherwise we get flickering
        ToolBox         ( pParent, WinBits(WB_CLIPCHILDREN) ),
        aWndPos         ( VclPtr<ScPosWnd>::Create(this) ),
        pRuntimeWindow  ( lcl_chooseRuntimeImpl( this, pBind ) ),
        aTextWindow     ( *pRuntimeWindow ),
        pInputHdl       ( NULL ),
        aTextOk         ( ScResId( SCSTR_QHELP_BTNOK ) ),       // Not always new as a Resource
        aTextCancel     ( ScResId( SCSTR_QHELP_BTNCANCEL ) ),
        aTextSum        ( ScResId( SCSTR_QHELP_BTNSUM ) ),
        aTextEqual      ( ScResId( SCSTR_QHELP_BTNEQUAL ) ),
        mnMaxY          (0),
        bIsOkCancelMode ( false ),
        bInResize       ( false ),
        mbIsMultiLine   ( lcl_isExperimentalMode() )
{
    ScModule*        pScMod  = SC_MOD();
    SfxImageManager* pImgMgr = SfxImageManager::GetImageManager(*pScMod);

    // #i73615# don't rely on SfxViewShell::Current while constructing the input line
    // (also for GetInputHdl below)
    ScTabViewShell* pViewSh = NULL;
    SfxDispatcher* pDisp = pBind->GetDispatcher();
    if ( pDisp )
    {
        SfxViewFrame* pViewFrm = pDisp->GetFrame();
        if ( pViewFrm )
            pViewSh = PTR_CAST( ScTabViewShell, pViewFrm->GetViewShell() );
    }
    OSL_ENSURE( pViewSh, "no view shell for input window" );

    // Position window, 3 buttons, input window
    InsertWindow    (1, aWndPos.get(), ToolBoxItemBits::NONE, 0);
    InsertSeparator (1);
    InsertItem      (SID_INPUT_FUNCTION, IMAGE(SID_INPUT_FUNCTION), ToolBoxItemBits::NONE, 2);
    InsertItem      (SID_INPUT_SUM,      IMAGE(SID_INPUT_SUM), ToolBoxItemBits::NONE, 3);
    InsertItem      (SID_INPUT_EQUAL,    IMAGE(SID_INPUT_EQUAL), ToolBoxItemBits::NONE, 4);
    InsertSeparator (5);
    InsertWindow    (7, &aTextWindow, ToolBoxItemBits::NONE, 6);

    aWndPos   ->SetQuickHelpText(ScResId(SCSTR_QHELP_POSWND));
    aWndPos   ->SetHelpId       (HID_INSWIN_POS);
    aTextWindow.SetQuickHelpText(ScResId(SCSTR_QHELP_INPUTWND));
    aTextWindow.SetHelpId       (HID_INSWIN_INPUT);

    // No SetHelpText: the helptexts come from the Help
    SetItemText (SID_INPUT_FUNCTION, ScResId(SCSTR_QHELP_BTNCALC));
    SetHelpId   (SID_INPUT_FUNCTION, HID_INSWIN_CALC);

    SetItemText (SID_INPUT_SUM, aTextSum);
    SetHelpId   (SID_INPUT_SUM, HID_INSWIN_SUMME);

    SetItemText (SID_INPUT_EQUAL, aTextEqual);
    SetHelpId   (SID_INPUT_EQUAL, HID_INSWIN_FUNC);

    SetHelpId( HID_SC_INPUTWIN ); // For the whole input row

    aWndPos   ->Show();
    aTextWindow.Show();

    pInputHdl = SC_MOD()->GetInputHdl( pViewSh, false ); // use own handler even if ref-handler is set
    if (pInputHdl)
        pInputHdl->SetInputWindow( this );

    if (pInputHdl && !pInputHdl->GetFormString().isEmpty())
    {
        // Switch over while the Function AutoPilot is active
        // -> show content of the Function AutoPilot again
        // Also show selection (remember at the InputHdl)
        aTextWindow.SetTextString( pInputHdl->GetFormString() );
    }
    else if (pInputHdl && pInputHdl->IsInputMode())
    {
        // If the input row was hidden while editing (e.g. when editing a formula
        // and then switching to another document or the help), display the text
        // we just edited from the InputHandler
        aTextWindow.SetTextString( pInputHdl->GetEditString() ); // Dispaly text
        if ( pInputHdl->IsTopMode() )
            pInputHdl->SetMode( SC_INPUT_TABLE ); // Focus ends up at the bottom anyways
    }
    else if (pViewSh)
        pViewSh->UpdateInputHandler(true); // Absolutely necessary update

    pImgMgr->RegisterToolBox(this);
    SetAccessibleName(ScResId(STR_ACC_TOOLBAR_FORMULA));
}

ScInputWindow::~ScInputWindow()
{
    disposeOnce();
}

void ScInputWindow::dispose()
{
    bool bDown = ( ScGlobal::pSysLocale == NULL ); // after Clear?

    //  if any view's input handler has a pointer to this input window, reset it
    //  (may be several ones, #74522#)
    //  member pInputHdl is not used here

    if ( !bDown )
    {
        TypeId aScType = TYPE(ScTabViewShell);
        SfxViewShell* pSh = SfxViewShell::GetFirst( &aScType );
        while ( pSh )
        {
            ScInputHandler* pHdl = static_cast<ScTabViewShell*>(pSh)->GetInputHandler();
            if ( pHdl && pHdl->GetInputWindow() == this )
            {
                pHdl->SetInputWindow( NULL );
                pHdl->StopInputWinEngine( false );  // reset pTopView pointer
            }
            pSh = SfxViewShell::GetNext( *pSh, &aScType );
        }
    }

    SfxImageManager::GetImageManager( *SC_MOD() )->ReleaseToolBox( this );

    pRuntimeWindow.disposeAndClear();
    aWndPos.disposeAndClear();

    ToolBox::dispose();
}

void ScInputWindow::SetInputHandler( ScInputHandler* pNew )
{
    //  Is called in the Activate of the View ...
    if ( pNew != pInputHdl )
    {
        // On Reload (last version) the pInputHdl is the InputHandler of the old, deleted
        // ViewShell: so don't touch it here!
        pInputHdl = pNew;
        if (pInputHdl)
            pInputHdl->SetInputWindow( this );
    }
}

bool ScInputWindow::UseSubTotal(ScRangeList* pRangeList)
{
    bool bSubTotal = false;
    ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
    if ( pViewSh )
    {
        ScDocument* pDoc = pViewSh->GetViewData().GetDocument();
        size_t nRangeCount (pRangeList->size());
        size_t nRangeIndex (0);
        while (!bSubTotal && nRangeIndex < nRangeCount)
        {
            const ScRange* pRange = (*pRangeList)[nRangeIndex];
            if( pRange )
            {
                SCTAB nTabEnd(pRange->aEnd.Tab());
                SCTAB nTab(pRange->aStart.Tab());
                while (!bSubTotal && nTab <= nTabEnd)
                {
                    SCROW nRowEnd(pRange->aEnd.Row());
                    SCROW nRow(pRange->aStart.Row());
                    while (!bSubTotal && nRow <= nRowEnd)
                    {
                        if (pDoc->RowFiltered(nRow, nTab))
                            bSubTotal = true;
                        else
                            ++nRow;
                    }
                    ++nTab;
                }
            }
            ++nRangeIndex;
        }

        const ScDBCollection::NamedDBs& rDBs = pDoc->GetDBCollection()->getNamedDBs();
        ScDBCollection::NamedDBs::const_iterator itr = rDBs.begin(), itrEnd = rDBs.end();
        for (; !bSubTotal && itr != itrEnd; ++itr)
        {
            const ScDBData& rDB = **itr;
            if (!rDB.HasAutoFilter())
                continue;

            nRangeIndex = 0;
            while (!bSubTotal && nRangeIndex < nRangeCount)
            {
                const ScRange* pRange = (*pRangeList)[nRangeIndex];
                if( pRange )
                {
                    ScRange aDBArea;
                    rDB.GetArea(aDBArea);
                    if (aDBArea.Intersects(*pRange))
                        bSubTotal = true;
                }
                ++nRangeIndex;
            }
        }
    }
    return bSubTotal;
}

void ScInputWindow::Select()
{
    ScModule* pScMod = SC_MOD();
    ToolBox::Select();

    switch ( GetCurItemId() )
    {
        case SID_INPUT_FUNCTION:
            {
                //! new method at ScModule to query if function autopilot is open
                SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                if ( pViewFrm && !pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) )
                {
                    pViewFrm->GetDispatcher()->Execute( SID_OPENDLG_FUNCTION,
                                              SfxCallMode::SYNCHRON | SfxCallMode::RECORD );

                    // The Toolbox will be disabled anyways, so we don't need to switch here,
                    // regardless whether it succeeded or not!
//                  SetOkCancelMode();
                }
            }
            break;

        case SID_INPUT_CANCEL:
            pScMod->InputCancelHandler();
            SetSumAssignMode();
            break;

        case SID_INPUT_OK:
            pScMod->InputEnterHandler();
            SetSumAssignMode();
            aTextWindow.Invalidate(); // Or else the Selection remains
            break;

        case SID_INPUT_SUM:
            {
                ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
                if ( pViewSh )
                {
                    const ScMarkData& rMark = pViewSh->GetViewData().GetMarkData();
                    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
                    {
                        ScRangeList aMarkRangeList;
                        rMark.FillRangeListWithMarks( &aMarkRangeList, false );
                        ScDocument* pDoc = pViewSh->GetViewData().GetDocument();

                        // check if one of the marked ranges is empty
                        bool bEmpty = false;
                        const size_t nCount = aMarkRangeList.size();
                        for ( size_t i = 0; i < nCount; ++i )
                        {
                            const ScRange aRange( *aMarkRangeList[i] );
                            if ( pDoc->IsBlockEmpty( aRange.aStart.Tab(),
                                    aRange.aStart.Col(), aRange.aStart.Row(),
                                    aRange.aEnd.Col(), aRange.aEnd.Row() ) )
                            {
                                bEmpty = true;
                                break;
                            }
                        }

                        if ( bEmpty )
                        {
                            ScRangeList aRangeList;
                            const bool bDataFound = pViewSh->GetAutoSumArea( aRangeList );
                            if ( bDataFound )
                            {
                                ScAddress aAddr = aRangeList.back()->aEnd;
                                aAddr.IncRow();
                                const bool bSubTotal( UseSubTotal( &aRangeList ) );
                                pViewSh->EnterAutoSum( aRangeList, bSubTotal, aAddr );
                            }
                        }
                        else
                        {
                            const bool bSubTotal( UseSubTotal( &aMarkRangeList ) );
                            for ( size_t i = 0; i < nCount; ++i )
                            {
                                const ScRange aRange( *aMarkRangeList[i] );
                                const bool bSetCursor = ( i == nCount - 1 );
                                const bool bContinue = ( i != 0 );
                                if ( !pViewSh->AutoSum( aRange, bSubTotal, bSetCursor, bContinue ) )
                                {
                                    pViewSh->MarkRange( aRange, false );
                                    pViewSh->SetCursor( aRange.aEnd.Col(), aRange.aEnd.Row() );
                                    const ScRangeList aRangeList;
                                    ScAddress aAddr = aRange.aEnd;
                                    aAddr.IncRow();
                                    const OUString aFormula = pViewSh->GetAutoSumFormula(
                                        aRangeList, bSubTotal, aAddr );
                                    SetFuncString( aFormula );
                                    break;
                                }
                            }
                        }
                    }
                    else // Only insert into input row
                    {
                        ScRangeList aRangeList;
                        const bool bDataFound = pViewSh->GetAutoSumArea( aRangeList );
                        const bool bSubTotal( UseSubTotal( &aRangeList ) );
                        ScAddress aAddr = pViewSh->GetViewData().GetCurPos();
                        const OUString aFormula = pViewSh->GetAutoSumFormula( aRangeList, bSubTotal, aAddr );
                        SetFuncString( aFormula );

                        if ( bDataFound && pScMod->IsEditMode() )
                        {
                            ScInputHandler* pHdl = pScMod->GetInputHdl( pViewSh );
                            if ( pHdl )
                            {
                                pHdl->InitRangeFinder( aFormula );

                                //! SetSelection at the InputHandler?
                                //! Set bSelIsRef?
                                const sal_Int32 nOpen = aFormula.indexOf('(');
                                const sal_Int32 nLen = aFormula.getLength();
                                if ( nOpen != -1 && nLen > nOpen )
                                {
                                    sal_uInt8 nAdd(1);
                                    if (bSubTotal)
                                        nAdd = 3;
                                    ESelection aSel(0,nOpen+nAdd,0,nLen-1);
                                    EditView* pTableView = pHdl->GetTableView();
                                    if (pTableView)
                                        pTableView->SetSelection(aSel);
                                    EditView* pTopView = pHdl->GetTopView();
                                    if (pTopView)
                                        pTopView->SetSelection(aSel);
                                }
                            }
                        }
                    }
                }
            }
            break;

        case SID_INPUT_EQUAL:
        {
            aTextWindow.StartEditEngine();
            if ( pScMod->IsEditMode() ) // Isn't if e.g. protected
            {
                aTextWindow.StartEditEngine();

                sal_Int32 nStartPos = 1;
                sal_Int32 nEndPos = 1;

                ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
                if ( pViewSh )
                {
                    const OUString& rString = aTextWindow.GetTextString();
                    const sal_Int32 nLen = rString.getLength();

                    ScDocument* pDoc = pViewSh->GetViewData().GetDocument();
                    CellType eCellType = pDoc->GetCellType( pViewSh->GetViewData().GetCurPos() );
                    switch ( eCellType )
                    {
                        case CELLTYPE_VALUE:
                        {
                            nEndPos = nLen + 1;
                            aTextWindow.SetTextString("=" +  rString);
                            break;
                        }
                        case CELLTYPE_STRING:
                        case CELLTYPE_EDIT:
                            nStartPos = 0;
                            nEndPos = nLen;
                            break;
                        case CELLTYPE_FORMULA:
                            nEndPos = nLen;
                            break;
                        default:
                            aTextWindow.SetTextString("=");
                            break;
                    }
                }

                EditView* pView = aTextWindow.GetEditView();
                if (pView)
                {
                    pView->SetSelection( ESelection(0, nStartPos, 0, nEndPos) );
                    pScMod->InputChanged(pView);
                    SetOkCancelMode();
                    pView->SetEditEngineUpdateMode(true);
                }
            }
            break;
        }
    }
}

void ScInputWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    ToolBox::Paint(rRenderContext, rRect);

    // draw a line at the bottom to distinguish that from the grid
    // (we have space for that thanks to ADDITIONAL_BORDER)
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());

    Size aSize = GetSizePixel();
    rRenderContext.DrawLine(Point(0, aSize.Height() - 1),
                            Point(aSize.Width() - 1, aSize.Height() - 1));
}

void ScInputWindow::Resize()
{
    ToolBox::Resize();
    if ( mbIsMultiLine )
    {
        aTextWindow.Resize();
        Size aSize = GetSizePixel();
        aSize.Height() = CalcWindowSizePixel().Height() + ADDITIONAL_BORDER;
        ScInputBarGroup* pGroupBar = dynamic_cast< ScInputBarGroup* > ( pRuntimeWindow.get() );
        if ( pGroupBar )
        {
            // To ensure smooth display and prevent the items in the toolbar being
            // repositioned ( vertically ) we lock the vertical positioning of the toolbox
            // items when we are displaying > 1 line.
            // So, we need to adjust the height of the toolbox accordingly. If we don't
            // then the largest item ( e.g. the GroupBar window ) will actually be
            // positioned such that the toolbar will cut off the bottom of that item
            if ( pGroupBar->GetNumLines() > 1 )
                aSize.Height() += pGroupBar->GetVertOffset() + ADDITIONAL_SPACE;
        }
        SetSizePixel(aSize);
        Invalidate();
    }
    else
    {
        long nWidth = GetSizePixel().Width();
        long nLeft  = aTextWindow.GetPosPixel().X();
        Size aSize  = aTextWindow.GetSizePixel();

        aSize.Width() = std::max( ((long)(nWidth - nLeft - 5)), (long)0 );

        aTextWindow.SetSizePixel( aSize );
        aTextWindow.Invalidate();
    }
}

void ScInputWindow::SetFuncString( const OUString& rString, bool bDoEdit )
{
    //! new method at ScModule to query if function autopilot is open
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    EnableButtons( pViewFrm && !pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) );
    aTextWindow.StartEditEngine();

    ScModule* pScMod = SC_MOD();
    if ( pScMod->IsEditMode() )
    {
        if ( bDoEdit )
            aTextWindow.GrabFocus();
        aTextWindow.SetTextString( rString );
        EditView* pView = aTextWindow.GetEditView();
        if (pView)
        {
            sal_Int32 nLen = rString.getLength();

            if ( nLen > 0 )
            {
                nLen--;
                pView->SetSelection( ESelection( 0, nLen, 0, nLen ) );
            }

            pScMod->InputChanged(pView);
            if ( bDoEdit )
                SetOkCancelMode(); // Not the case if immediately followed by Enter/Cancel

            pView->SetEditEngineUpdateMode(true);
        }
    }
}

void ScInputWindow::SetPosString( const OUString& rStr )
{
    aWndPos->SetPos( rStr );
}

void ScInputWindow::SetTextString( const OUString& rString )
{
    if (rString.getLength() <= 32767)
        aTextWindow.SetTextString(rString);
    else
        aTextWindow.SetTextString(rString.copy(0, 32767));
}

void ScInputWindow::SetOkCancelMode()
{
    //! new method at ScModule to query if function autopilot is open
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    EnableButtons( pViewFrm && !pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) );

    ScModule* pScMod = SC_MOD();
    SfxImageManager* pImgMgr = SfxImageManager::GetImageManager(*pScMod);
    if (!bIsOkCancelMode)
    {
        RemoveItem( 3 ); // Remove SID_INPUT_SUM and SID_INPUT_EQUAL
        RemoveItem( 3 );
        InsertItem( SID_INPUT_CANCEL, IMAGE( SID_INPUT_CANCEL ), ToolBoxItemBits::NONE, 3 );
        InsertItem( SID_INPUT_OK,     IMAGE( SID_INPUT_OK ),     ToolBoxItemBits::NONE, 4 );
        SetItemText ( SID_INPUT_CANCEL, aTextCancel );
        SetHelpId   ( SID_INPUT_CANCEL, HID_INSWIN_CANCEL );
        SetItemText ( SID_INPUT_OK,     aTextOk );
        SetHelpId   ( SID_INPUT_OK,     HID_INSWIN_OK );
        bIsOkCancelMode = true;
    }
}

void ScInputWindow::SetSumAssignMode()
{
    //! new method at ScModule to query if function autopilot is open
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    EnableButtons( pViewFrm && !pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) );

    ScModule* pScMod = SC_MOD();
    SfxImageManager* pImgMgr = SfxImageManager::GetImageManager(*pScMod);
    if (bIsOkCancelMode)
    {
        // Remove SID_INPUT_CANCEL, and SID_INPUT_OK
        RemoveItem( 3 );
        RemoveItem( 3 );
        InsertItem( SID_INPUT_SUM,   IMAGE( SID_INPUT_SUM ),   ToolBoxItemBits::NONE, 3 );
        InsertItem( SID_INPUT_EQUAL, IMAGE( SID_INPUT_EQUAL ), ToolBoxItemBits::NONE, 4 );
        SetItemText ( SID_INPUT_SUM,   aTextSum );
        SetHelpId   ( SID_INPUT_SUM,   HID_INSWIN_SUMME );
        SetItemText ( SID_INPUT_EQUAL, aTextEqual );
        SetHelpId   ( SID_INPUT_EQUAL, HID_INSWIN_FUNC );
        bIsOkCancelMode = false;

        SetFormulaMode(false); // No editing -> no formula
    }
}

void ScInputWindow::SetFormulaMode( bool bSet )
{
    aWndPos->SetFormulaMode(bSet);
    aTextWindow.SetFormulaMode(bSet);
}

void ScInputWindow::SetText( const OUString& rString )
{
    ToolBox::SetText(rString);
}

OUString ScInputWindow::GetText() const
{
    return ToolBox::GetText();
}

bool ScInputWindow::IsInputActive()
{
    return aTextWindow.IsInputActive();
}

EditView* ScInputWindow::GetEditView()
{
    return aTextWindow.GetEditView();
}

void ScInputWindow::MakeDialogEditView()
{
    aTextWindow.MakeDialogEditView();
}

void ScInputWindow::StopEditEngine( bool bAll )
{
    aTextWindow.StopEditEngine( bAll );
}

void ScInputWindow::TextGrabFocus()
{
    aTextWindow.TextGrabFocus();
}

void ScInputWindow::TextInvalidate()
{
    aTextWindow.Invalidate();
}

void ScInputWindow::SwitchToTextWin()
{
    // used for shift-ctrl-F2

    aTextWindow.StartEditEngine();
    if ( SC_MOD()->IsEditMode() )
    {
        aTextWindow.TextGrabFocus();
        EditView* pView = aTextWindow.GetEditView();
        if (pView)
        {
            sal_Int32 nPara =  pView->GetEditEngine()->GetParagraphCount() ? ( pView->GetEditEngine()->GetParagraphCount() - 1 ) : 0;
            sal_Int32 nLen = pView->GetEditEngine()->GetTextLen( nPara );
            ESelection aSel( nPara, nLen, nPara, nLen );
            pView->SetSelection( aSel ); // set cursor to end of text
        }
    }
}

void ScInputWindow::PosGrabFocus()
{
    aWndPos->GrabFocus();
}

void ScInputWindow::EnableButtons( bool bEnable )
{
    //  when enabling buttons, always also enable the input window itself
    if ( bEnable && !IsEnabled() )
        Enable();

    EnableItem( SID_INPUT_FUNCTION,                                   bEnable );
    EnableItem( bIsOkCancelMode ? SID_INPUT_CANCEL : SID_INPUT_SUM,   bEnable );
    EnableItem( bIsOkCancelMode ? SID_INPUT_OK     : SID_INPUT_EQUAL, bEnable );
//  Invalidate();
}

void ScInputWindow::StateChanged( StateChangedType nType )
{
    ToolBox::StateChanged( nType );

    if ( nType == StateChangedType::InitShow ) Resize();
}

void ScInputWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( rDCEvt.GetType() == DataChangedEventType::SETTINGS && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        //  update item images
        ScModule*        pScMod  = SC_MOD();
        SfxImageManager* pImgMgr = SfxImageManager::GetImageManager(*pScMod);

        // IMAGE macro uses pScMod, pImgMg
        SetItemImage( SID_INPUT_FUNCTION, IMAGE( SID_INPUT_FUNCTION ) );
        if ( bIsOkCancelMode )
        {
            SetItemImage( SID_INPUT_CANCEL, IMAGE( SID_INPUT_CANCEL ) );
            SetItemImage( SID_INPUT_OK,     IMAGE( SID_INPUT_OK ) );
        }
        else
        {
            SetItemImage( SID_INPUT_SUM,   IMAGE( SID_INPUT_SUM ) );
            SetItemImage( SID_INPUT_EQUAL, IMAGE( SID_INPUT_EQUAL ) );
        }
    }

    ToolBox::DataChanged( rDCEvt );
}

bool ScInputWindow::IsPointerAtResizePos()
{
    if ( GetOutputSizePixel().Height() - GetPointerPosPixel().Y() <= 4  )
        return true;
    else
        return false;
}

void ScInputWindow::MouseMove( const MouseEvent& rMEvt )
{
    if ( mbIsMultiLine )
    {
        Point aPosPixel = GetPointerPosPixel();

        ScInputBarGroup* pGroupBar = dynamic_cast< ScInputBarGroup* > ( pRuntimeWindow.get() );

        if ( bInResize || IsPointerAtResizePos() )
            SetPointer( Pointer( PointerStyle::WindowSSize ) );
        else
            SetPointer( Pointer( PointerStyle::Arrow ) );

        if ( bInResize )
        {
            // detect direction
            long nResizeThreshold = ( (long)TBX_WINDOW_HEIGHT * 0.7 );
            bool bResetPointerPos = false;

            // Detect attempt to expand toolbar too much
            if ( aPosPixel.Y() >= mnMaxY )
            {
                bResetPointerPos = true;
                aPosPixel.Y() = mnMaxY;
            } // or expanding down
            else if ( GetOutputSizePixel().Height() - aPosPixel.Y() < -nResizeThreshold  )
            {
                pGroupBar->IncrementVerticalSize();
                bResetPointerPos = true;
            } // or shrinking up
            else if ( ( GetOutputSizePixel().Height() - aPosPixel.Y()  ) > nResizeThreshold )
            {
                bResetPointerPos = true;
                pGroupBar->DecrementVerticalSize();
            }

            if ( bResetPointerPos )
            {
                aPosPixel.Y() =  GetOutputSizePixel().Height();
                SetPointerPosPixel( aPosPixel );
            }
        }
    }
    ToolBox::MouseMove( rMEvt );
}

void ScInputWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( mbIsMultiLine )
    {
        if ( rMEvt.IsLeft() )
        {
            if ( IsPointerAtResizePos() )
            {
                // Don't leave the mouse pointer leave *this* window
                CaptureMouse();
                bInResize = true;

                // find the height of the gridwin, we don't wan't to be
                // able to expand the toolbar too far so we need to
                // calculate an upper limit
                // I'd prefer to leave at least a single column header and a
                // row but I don't know how to get that value in pixels.
                // Use TBX_WINDOW_HEIGHT for the moment
                ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
                mnMaxY =  GetOutputSizePixel().Height() + ( pViewSh->GetGridHeight(SC_SPLIT_TOP) + pViewSh->GetGridHeight(SC_SPLIT_BOTTOM) ) - TBX_WINDOW_HEIGHT;
            }
        }
    }
    ToolBox::MouseButtonDown( rMEvt );
}
void ScInputWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( mbIsMultiLine )
    {
        ReleaseMouse();
        if ( rMEvt.IsLeft() )
        {
            bInResize = false;
            mnMaxY = 0;
        }
    }
    ToolBox::MouseButtonUp( rMEvt );
}

ScInputBarGroup::ScInputBarGroup(vcl::Window* pParent, ScTabViewShell* pViewSh)
    :   ScTextWndBase        ( pParent, WinBits(WB_HIDE |  WB_TABSTOP ) ),
        aMultiTextWnd        ( VclPtr<ScMultiTextWnd>::Create(this, pViewSh) ),
        aButton              ( VclPtr<ImageButton>::Create(this, WB_TABSTOP | WB_RECTSTYLE | WB_SMALLSTYLE) ),
        aScrollBar           ( VclPtr<ScrollBar>::Create(this, WB_TABSTOP | WB_VERT | WB_DRAG) ),
        nVertOffset          ( 0 )
{
      aMultiTextWnd->Show();
      aMultiTextWnd->SetQuickHelpText( ScResId( SCSTR_QHELP_INPUTWND ) );
      aMultiTextWnd->SetHelpId( HID_INSWIN_INPUT );

      Size aSize( GetSettings().GetStyleSettings().GetScrollBarSize(), aMultiTextWnd->GetPixelHeightForLines(1) );

      aButton->SetClickHdl( LINK( this, ScInputBarGroup, ClickHdl ) );
      aButton->SetSizePixel( aSize );
      aButton->Enable();
      aButton->SetSymbol( SymbolType::SPIN_DOWN  );
      aButton->SetQuickHelpText( ScResId( SCSTR_QHELP_EXPAND_FORMULA ) );
      aButton->Show();

      aScrollBar->SetSizePixel( aSize );
      aScrollBar->SetScrollHdl( LINK( this, ScInputBarGroup, Impl_ScrollHdl ) );
}

ScInputBarGroup::~ScInputBarGroup()
{
    disposeOnce();
}

void ScInputBarGroup::dispose()
{
    aMultiTextWnd.disposeAndClear();
    aButton.disposeAndClear();
    aScrollBar.disposeAndClear();
    ScTextWndBase::dispose();
}

void
ScInputBarGroup::InsertAccessibleTextData( ScAccessibleEditLineTextData& rTextData )
{
    aMultiTextWnd->InsertAccessibleTextData( rTextData );
}

void
ScInputBarGroup::RemoveAccessibleTextData( ScAccessibleEditLineTextData& rTextData )
{
    aMultiTextWnd->RemoveAccessibleTextData( rTextData );
}

const OUString&
ScInputBarGroup::GetTextString() const
{
    return aMultiTextWnd->GetTextString();
}

void ScInputBarGroup::SetTextString( const OUString& rString )
{
    aMultiTextWnd->SetTextString(rString);
}

void ScInputBarGroup::Resize()
{
    vcl::Window*w = GetParent();
    ScInputWindow *pParent;
    pParent = dynamic_cast<ScInputWindow*>(w);

    if (pParent == NULL)
    {
        OSL_FAIL("The parent window pointer pParent is null");
        return;
    }

    long nWidth = pParent->GetSizePixel().Width();
    long nLeft  = GetPosPixel().X();

    Size aSize  = GetSizePixel();
    aSize.Width() = std::max(long(nWidth - nLeft - LEFT_OFFSET), long(0));

    aScrollBar->SetPosPixel(Point( aSize.Width() - aButton->GetSizePixel().Width(), aButton->GetSizePixel().Height() ) );

    Size aTmpSize( aSize );
    aTmpSize.Width() = aTmpSize.Width() - aButton->GetSizePixel().Width() - BUTTON_OFFSET;
    aMultiTextWnd->SetSizePixel(aTmpSize);

    aMultiTextWnd->Resize();

    aSize.Height() = aMultiTextWnd->GetSizePixel().Height();

    SetSizePixel(aSize);

    if (aMultiTextWnd->GetNumLines() > 1)
    {
        aButton->SetSymbol( SymbolType::SPIN_UP  );
        aButton->SetQuickHelpText( ScResId( SCSTR_QHELP_COLLAPSE_FORMULA ) );
        Size scrollSize = aButton->GetSizePixel();
        scrollSize.Height() = aMultiTextWnd->GetSizePixel().Height() - aButton->GetSizePixel().Height();
        aScrollBar->SetSizePixel( scrollSize );

        Size aOutSz = aMultiTextWnd->GetOutputSize();

        aScrollBar->SetVisibleSize( aOutSz.Height() );
        aScrollBar->SetPageSize( aOutSz.Height() );
        aScrollBar->SetLineSize( aMultiTextWnd->GetTextHeight() );
        aScrollBar->SetRange( Range( 0, aMultiTextWnd->GetEditEngTxtHeight() ) );

        aScrollBar->Resize();
        aScrollBar->Show();
    }
    else
    {
        aButton->SetSymbol( SymbolType::SPIN_DOWN  );
        aButton->SetQuickHelpText( ScResId( SCSTR_QHELP_EXPAND_FORMULA ) );
        aScrollBar->Hide();
    }

    aButton->SetPosPixel(Point(aSize.Width() - aButton->GetSizePixel().Width(), 0));

    Invalidate();
}

void ScInputBarGroup::StopEditEngine( bool bAll )
{
    aMultiTextWnd->StopEditEngine( bAll );
}

void ScInputBarGroup::StartEditEngine()
{
    aMultiTextWnd->StartEditEngine();
}

void ScInputBarGroup::MakeDialogEditView()
{
    aMultiTextWnd->MakeDialogEditView();
}

EditView* ScInputBarGroup::GetEditView()
{
    return aMultiTextWnd->GetEditView();
}

bool ScInputBarGroup::IsInputActive()
{
    return aMultiTextWnd->IsInputActive();
}

void ScInputBarGroup::SetFormulaMode(bool bSet)
{
    aMultiTextWnd->SetFormulaMode(bSet);
}

void ScInputBarGroup::IncrementVerticalSize()
{
    aMultiTextWnd->SetNumLines( aMultiTextWnd->GetNumLines() + 1 );
    TriggerToolboxLayout();
}

void ScInputBarGroup::DecrementVerticalSize()
{
    if ( aMultiTextWnd->GetNumLines() > 1 )
    {
        aMultiTextWnd->SetNumLines( aMultiTextWnd->GetNumLines() - 1 );
        TriggerToolboxLayout();
    }
}

IMPL_LINK_NOARG_TYPED(ScInputBarGroup, ClickHdl, Button*, void)
{
    vcl::Window* w = GetParent();
    ScInputWindow* pParent;
    pParent = dynamic_cast<ScInputWindow*>(w);

    if (pParent == NULL)
    {
        OSL_FAIL("The parent window pointer pParent is null");
        return;
    }
    if (aMultiTextWnd->GetNumLines() > 1)
    {
        aMultiTextWnd->SetNumLines(1);
    }
    else
    {
        aMultiTextWnd->SetNumLines(aMultiTextWnd->GetLastNumExpandedLines());
    }
    TriggerToolboxLayout();

    // Restore focus to input line(s) if necessary
    if (  SC_MOD()->GetInputHdl()->IsTopMode() )
        aMultiTextWnd->GrabFocus();
}

void ScInputBarGroup::TriggerToolboxLayout()
{
    vcl::Window *w=GetParent();
    ScInputWindow &rParent = dynamic_cast<ScInputWindow&>(*w);
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();

    // Capture the vertical position of this window in the toolbar, when we increase
    // the size of the toolbar to accommodate expanded line input we need to take this
    // into account
    if ( !nVertOffset )
        nVertOffset = rParent.GetItemPosRect( rParent.GetItemCount() - 1 ).Top();

    if ( pViewFrm )
    {
        Reference< com::sun::star::beans::XPropertySet > xPropSet( pViewFrm->GetFrame().GetFrameInterface(), UNO_QUERY );
        Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;

        if ( xPropSet.is() )
        {
            com::sun::star::uno::Any aValue = xPropSet->getPropertyValue("LayoutManager");
            aValue >>= xLayoutManager;
        }

        if ( xLayoutManager.is() )
        {
            if ( aMultiTextWnd->GetNumLines() > 1)
                rParent.SetToolbarLayoutMode( TBX_LAYOUT_LOCKVERT );
            else
                rParent.SetToolbarLayoutMode( TBX_LAYOUT_NORMAL );
            xLayoutManager->lock();
            DataChangedEvent aFakeUpdate( DataChangedEventType::SETTINGS, NULL,  AllSettingsFlags::STYLE );

            // this basically will trigger the reposititioning of the
            // items in the toolbar from ImplFormat ( which is controlled by
            // mnWinHeight ) which in turn is updated in ImplCalcItem which is
            // controlled by mbCalc. Additionally the ImplFormat above is
            // controlled via mbFormat. It seems the easiest way to get these
            // booleans set is to send in the fake event below.
            rParent.DataChanged( aFakeUpdate);

            // highest item in toolbar will have been calculated via the
            // event above. Call resize on InputBar to pick up the height
            // change
            rParent.Resize();

            // unlock relayouts the toolbars in the 4 quadrants
            xLayoutManager->unlock();
        }
    }
}

IMPL_LINK_NOARG_TYPED(ScInputBarGroup, Impl_ScrollHdl, ScrollBar*, void)
{
    aMultiTextWnd->DoScroll();
}

void ScInputBarGroup::TextGrabFocus()
{
    aMultiTextWnd->TextGrabFocus();
}

ScMultiTextWnd::ScMultiTextWnd( ScInputBarGroup* pParen, ScTabViewShell* pViewSh )
    : ScTextWnd( pParen, pViewSh ),
      mrGroupBar(* pParen ),
      mnLines( 1 ),
      mnLastExpandedLines( INPUTWIN_MULTILINES ),
      mbInvalidate( false )
{
    Size aBorder;
    aBorder = CalcWindowSize(aBorder);
    mnBorderHeight = aBorder.Height();
}

ScMultiTextWnd::~ScMultiTextWnd()
{
}

void ScMultiTextWnd::Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect )
{
    EditView* pView = GetEditView();
    if (pView)
    {
        if (mbInvalidate)
        {
            pView->Invalidate();
            mbInvalidate = false;
        }
        pEditView->Paint(rRect, &rRenderContext);
    }
}

EditView* ScMultiTextWnd::GetEditView()
{
    if ( !pEditView )
        InitEditEngine();
    return pEditView;
}

long ScMultiTextWnd::GetPixelHeightForLines( long nLines )
{
    // add padding ( for the borders of the window )
    return ( nLines * LogicToPixel( Size( 0, GetTextHeight() ) ).Height() ) + mnBorderHeight;
}

void ScMultiTextWnd::SetNumLines( long nLines )
{
    mnLines = nLines;
    if ( nLines > 1 )
    {
        mnLastExpandedLines = nLines;
        Resize();
    }
}

void ScMultiTextWnd::Resize()
{
    // Only Height is recalculated here, Width is applied from
    // parent/container window
    Size aTextBoxSize = GetSizePixel();

    aTextBoxSize.Height() = GetPixelHeightForLines( mnLines );
    SetSizePixel( aTextBoxSize );

    if(pEditView)
    {
        Size aOutputSize = GetOutputSizePixel();
        Rectangle aOutputArea = PixelToLogic( Rectangle( Point(), aOutputSize ));
        pEditView->SetOutputArea( aOutputArea );

        // Don't leave an empty area at the bottom if we can move the text down.
        long nMaxVisAreaTop = pEditEngine->GetTextHeight() - aOutputArea.GetHeight();
        if (pEditView->GetVisArea().Top() > nMaxVisAreaTop)
        {
            pEditView->Scroll(0, pEditView->GetVisArea().Top() - nMaxVisAreaTop);
        }

        pEditEngine->SetPaperSize( PixelToLogic( Size( aOutputSize.Width(), 10000 ) ) );
    }

    SetScrollBarRange();
}

IMPL_LINK_NOARG_TYPED(ScMultiTextWnd, ModifyHdl, LinkParamNone*, void)
{
    ScTextWnd::NotifyHdl(nullptr);
}

IMPL_LINK_TYPED(ScMultiTextWnd, NotifyHdl, EENotify&, rNotify, void)
{
    // need to process EE_NOTIFY_TEXTVIEWSCROLLED here
    // sometimes we don't seem to get EE_NOTIFY_TEXTVIEWSCROLLED e.g. when
    // we insert text at the beginning of the text so the cursor never moves
    // down to generate a scroll event

    if ( rNotify.eNotificationType == EE_NOTIFY_TEXTVIEWSCROLLED
         || rNotify.eNotificationType == EE_NOTIFY_TEXTHEIGHTCHANGED )
        SetScrollBarRange();
}

long ScMultiTextWnd::GetEditEngTxtHeight()
{
    return pEditView ? pEditView->GetEditEngine()->GetTextHeight() : 0;
}

void ScMultiTextWnd::SetScrollBarRange()
{
    if ( pEditView )
    {
        ScrollBar& rVBar = mrGroupBar.GetScrollBar();
        rVBar.SetRange( Range( 0, GetEditEngTxtHeight() ) );
        long currentDocPos = pEditView->GetVisArea().TopLeft().Y();
        rVBar.SetThumbPos( currentDocPos );
    }
}

void
ScMultiTextWnd::DoScroll()
{
    if ( pEditView )
    {
        ScrollBar& rVBar = mrGroupBar.GetScrollBar();
        long currentDocPos = pEditView->GetVisArea().TopLeft().Y();
        long nDiff = currentDocPos - rVBar.GetThumbPos();
        pEditView->Scroll( 0, nDiff );
        currentDocPos = pEditView->GetVisArea().TopLeft().Y();
        rVBar.SetThumbPos( currentDocPos );
    }
}

void ScMultiTextWnd::StartEditEngine()
{
    // Don't activate if we're a modal dialog ourselves (Doc-modal dialog)
    SfxObjectShell* pObjSh = SfxObjectShell::Current();
    if ( pObjSh && pObjSh->IsInModalMode() )
        return;

    if ( !pEditView || !pEditEngine )
    {
        InitEditEngine();
    }

    SC_MOD()->SetInputMode( SC_INPUT_TOP );

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (pViewFrm)
        pViewFrm->GetBindings().Invalidate( SID_ATTR_INSERT );
}

static void lcl_ExtendEditFontAttribs( SfxItemSet& rSet )
{
    const SfxPoolItem& rFontItem = rSet.Get( EE_CHAR_FONTINFO );
    rSet.Put( rFontItem, EE_CHAR_FONTINFO_CJK );
    rSet.Put( rFontItem, EE_CHAR_FONTINFO_CTL );
    const SfxPoolItem& rHeightItem = rSet.Get( EE_CHAR_FONTHEIGHT );
    rSet.Put( rHeightItem, EE_CHAR_FONTHEIGHT_CJK );
    rSet.Put( rHeightItem, EE_CHAR_FONTHEIGHT_CTL );
    const SfxPoolItem& rWeightItem = rSet.Get( EE_CHAR_WEIGHT );
    rSet.Put( rWeightItem, EE_CHAR_WEIGHT_CJK );
    rSet.Put( rWeightItem, EE_CHAR_WEIGHT_CTL );
    const SfxPoolItem& rItalicItem = rSet.Get( EE_CHAR_ITALIC );
    rSet.Put( rItalicItem, EE_CHAR_ITALIC_CJK );
    rSet.Put( rItalicItem, EE_CHAR_ITALIC_CTL );
    const SfxPoolItem& rLangItem = rSet.Get( EE_CHAR_LANGUAGE );
    rSet.Put( rLangItem, EE_CHAR_LANGUAGE_CJK );
    rSet.Put( rLangItem, EE_CHAR_LANGUAGE_CTL );
}

static void lcl_ModifyRTLDefaults( SfxItemSet& rSet )
{
    rSet.Put( SvxAdjustItem( SVX_ADJUST_RIGHT, EE_PARA_JUST ) );

    // always using rtl writing direction would break formulas
    //rSet.Put( SvxFrameDirectionItem( FRMDIR_HORI_RIGHT_TOP, EE_PARA_WRITINGDIR ) );

    // PaperSize width is limited to USHRT_MAX in RTL mode (because of EditEngine's
    // sal_uInt16 values in EditLine), so the text may be wrapped and line spacing must be
    // increased to not see the beginning of the next line.
    SvxLineSpacingItem aItem( SVX_LINESPACE_TWO_LINES, EE_PARA_SBL );
    aItem.SetPropLineSpace( 200 );
    rSet.Put( aItem );
}

static void lcl_ModifyRTLVisArea( EditView* pEditView )
{
    Rectangle aVisArea = pEditView->GetVisArea();
    Size aPaper = pEditView->GetEditEngine()->GetPaperSize();
    long nDiff = aPaper.Width() - aVisArea.Right();
    aVisArea.Left()  += nDiff;
    aVisArea.Right() += nDiff;
    pEditView->SetVisArea(aVisArea);
}

void ScMultiTextWnd::InitEditEngine()
{
    ScFieldEditEngine* pNew;
    ScTabViewShell* pViewSh = GetViewShell();
    ScDocShell* pDocSh = NULL;
    if ( pViewSh )
    {
        pDocSh = pViewSh->GetViewData().GetDocShell();
        ScDocument* pDoc = pViewSh->GetViewData().GetDocument();
        pNew = new ScFieldEditEngine(pDoc, pDoc->GetEnginePool(), pDoc->GetEditPool());
    }
    else
        pNew = new ScFieldEditEngine(NULL, EditEngine::CreatePool(), NULL, true);
    pNew->SetExecuteURL( false );
    pEditEngine = pNew;

    Size barSize=GetSizePixel();
    pEditEngine->SetUpdateMode( false );
    pEditEngine->SetPaperSize( PixelToLogic(Size(barSize.Width(),10000)) );
    pEditEngine->SetWordDelimiters(
                    ScEditUtil::ModifyDelimiters( pEditEngine->GetWordDelimiters() ) );

    UpdateAutoCorrFlag();

    {
        SfxItemSet* pSet = new SfxItemSet( pEditEngine->GetEmptyItemSet() );
        EditEngine::SetFontInfoInItemSet( *pSet, aTextFont );
        lcl_ExtendEditFontAttribs( *pSet );
        // turn off script spacing to match DrawText output
        pSet->Put( SvxScriptSpaceItem( false, EE_PARA_ASIANCJKSPACING ) );
        if ( bIsRTL )
            lcl_ModifyRTLDefaults( *pSet );
        pEditEngine->SetDefaults( pSet );
    }

    // If the Cell contains URLFields, they need to be taken over into the entry row,
    // or else the position is not correct anymore
    bool bFilled = false;
    ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
    if ( pHdl ) //! Test if it's the right InputHdl?
        bFilled = pHdl->GetTextAndFields( *pEditEngine );

    pEditEngine->SetUpdateMode( true );

    // aString is the truth ...
    if (bFilled && pEditEngine->GetText() == aString)
        Invalidate(); // Repaint for (filled) Field
    else
        pEditEngine->SetText(aString); // At least the right text then

    pEditView = new EditView( pEditEngine, this );
    pEditView->SetInsertMode(bIsInsertMode);

    // Text from Clipboard is taken over as ASCII in a single row
    EVControlBits n = pEditView->GetControlWord();
    pEditView->SetControlWord( n | EVControlBits::SINGLELINEPASTE );

    pEditEngine->InsertView( pEditView, EE_APPEND );

    Resize();

    if ( bIsRTL )
        lcl_ModifyRTLVisArea( pEditView );

    pEditEngine->SetModifyHdl(LINK(this, ScMultiTextWnd, ModifyHdl));
    pEditEngine->SetNotifyHdl(LINK(this, ScMultiTextWnd, NotifyHdl));

    if (!maAccTextDatas.empty())
        maAccTextDatas.back()->StartEdit();

    // as long as EditEngine and DrawText sometimes differ for CTL text,
    // repaint now to have the EditEngine's version visible
    if (pDocSh)
    {
        ScDocument& rDoc = pDocSh->GetDocument(); // any document
        SvtScriptType nScript = rDoc.GetStringScriptType( aString );
        if ( nScript & SvtScriptType::COMPLEX )
            Invalidate();
    }
}

void ScMultiTextWnd::StopEditEngine( bool bAll )
{
    if ( pEditEngine )
        pEditEngine->SetNotifyHdl(Link<EENotify&,void>());
    ScTextWnd::StopEditEngine( bAll );
}

void ScMultiTextWnd::SetTextString( const OUString& rNewString )
{
    // Ideally it would be best to create on demand the EditEngine/EditView here, but... for
    // the initialisation scenario where a cell is first clicked on we end up with the text in the
    // inputbar window scrolled to the bottom if we do that here ( because the tableview and topview
    // are synced I guess ).
    // should fix that I suppose :-/ need to look a bit further into that
    mbInvalidate = true; // ensure next Paint ( that uses editengine ) call will call Invalidate first
    ScTextWnd::SetTextString( rNewString );
    SetScrollBarRange();
    DoScroll();
}

ScTextWnd::ScTextWnd( vcl::Window* pParent, ScTabViewShell* pViewSh )
    :   ScTextWndBase(pParent, WinBits(WB_HIDE | WB_BORDER)),
        DragSourceHelper(this),
        pEditEngine  (NULL),
        pEditView    (NULL),
        bIsInsertMode(true),
        bFormulaMode (false),
        bInputMode   (false),
        mpViewShell(pViewSh)
{
    EnableRTL(false); // EditEngine can't be used with VCL EnableRTL

    bIsRTL = AllSettings::GetLayoutRTL();

    //  always use application font, so a font with cjk chars can be installed
    vcl::Font aAppFont = GetFont();
    aTextFont = aAppFont;
    aTextFont.SetSize(PixelToLogic(aAppFont.GetSize(), MAP_TWIP));  // AppFont is in pixels

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    Color aBgColor = rStyleSettings.GetWindowColor();
    Color aTxtColor = rStyleSettings.GetWindowTextColor();

    aTextFont.SetTransparent(true);
    aTextFont.SetFillColor(aBgColor);
    aTextFont.SetColor(aTxtColor);
    aTextFont.SetWeight(WEIGHT_NORMAL);

    Size aSize(1,TBX_WINDOW_HEIGHT);
    Size aMinEditSize( Edit::GetMinimumEditSize() );
    if( aMinEditSize.Height() > aSize.Height() )
        aSize.Height() = aMinEditSize.Height();

    SetSizePixel(aSize);
    SetBackground(aBgColor);
    SetLineColor(COL_BLACK);
    SetMapMode(MAP_TWIP);
    SetPointer(PointerStyle::Text);
    SetFont(aTextFont);
}

ScTextWnd::~ScTextWnd()
{
    disposeOnce();
}

void ScTextWnd::dispose()
{
    while (!maAccTextDatas.empty()) {
        maAccTextDatas.back()->Dispose();
    }
    delete pEditView;
    pEditView = NULL;
    delete pEditEngine;
    pEditEngine = NULL;

    ScTextWndBase::dispose();
}

void ScTextWnd::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    if (pEditView)
    {
        Size aSize(GetOutputSizePixel());
        long nDiff = aSize.Height() - rRenderContext.LogicToPixel(Size(0, rRenderContext.GetTextHeight())).Height();
        pEditView->SetOutputArea(rRenderContext.PixelToLogic(Rectangle(Point(0, (nDiff > 0) ? nDiff / 2 : 1), aSize)));

        pEditView->Paint(rRect, &rRenderContext);
    }
    else
    {
        Size aSize = GetOutputSizePixel();
        SetFont(aTextFont);

        long nDiff = aSize.Height() - rRenderContext.LogicToPixel(Size(0, rRenderContext.GetTextHeight())).Height();

//      if (nDiff<2) nDiff=2;       // At least 1 pixel

        long nStartPos = 0;
        if (bIsRTL)
        {
            //  right-align
            nStartPos += aSize.Width() - rRenderContext.LogicToPixel(Size(rRenderContext.GetTextWidth(aString), 0)).Width();

            //  LayoutMode isn't changed as long as ModifyRTLDefaults doesn't include SvxFrameDirectionItem
        }
        rRenderContext.DrawText(rRenderContext.PixelToLogic(Point(nStartPos, nDiff / 2)), aString);
    }
    rRenderContext.Pop();
}

void ScTextWnd::Resize()
{
    Invalidate();
}

void ScTextWnd::MouseMove( const MouseEvent& rMEvt )
{
    if (pEditView)
        pEditView->MouseMove( rMEvt );
}

void ScTextWnd::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (!HasFocus())
    {
        StartEditEngine();
        if ( SC_MOD()->IsEditMode() )
            GrabFocus();
    }

    if (pEditView)
    {
        pEditView->SetEditEngineUpdateMode( true );
        pEditView->MouseButtonDown( rMEvt );
    }
}

void ScTextWnd::MouseButtonUp( const MouseEvent& rMEvt )
{
    if (pEditView)
        if (pEditView->MouseButtonUp( rMEvt ))
        {
            if ( rMEvt.IsMiddle() &&
                     GetSettings().GetMouseSettings().GetMiddleButtonAction() == MouseMiddleButtonAction::PasteSelection )
            {
                //  EditView may have pasted from selection
                SC_MOD()->InputChanged( pEditView );
            }
            else
                SC_MOD()->InputSelection( pEditView );
        }
}

void ScTextWnd::Command( const CommandEvent& rCEvt )
{
    bInputMode = true;
    CommandEventId nCommand = rCEvt.GetCommand();
    if ( pEditView /* && nCommand == CommandEventId::StartDrag */ )
    {
        ScModule* pScMod = SC_MOD();
        ScTabViewShell* pStartViewSh = ScTabViewShell::GetActiveViewShell();

        // don't modify the font defaults here - the right defaults are
        // already set in StartEditEngine when the EditEngine is created

        // Prevent that the EditView is lost when switching between Views
        pScMod->SetInEditCommand( true );
        pEditView->Command( rCEvt );
        pScMod->SetInEditCommand( false );

        //  CommandEventId::StartDrag does not mean by far that the content was actually changed,
        //  so don't trigger an InputChanged.
        //! Detect if dragged with Move or forbid Drag&Move somehow

        if ( nCommand == CommandEventId::StartDrag )
        {
            // Is dragged onto another View?
            ScTabViewShell* pEndViewSh = ScTabViewShell::GetActiveViewShell();
            if ( pEndViewSh != pStartViewSh && pStartViewSh != NULL )
            {
                ScViewData& rViewData = pStartViewSh->GetViewData();
                ScInputHandler* pHdl = pScMod->GetInputHdl( pStartViewSh );
                if ( pHdl && rViewData.HasEditView( rViewData.GetActivePart() ) )
                {
                    pHdl->CancelHandler();
                    rViewData.GetView()->ShowCursor(); // Missing for KillEditView, due to being inactive
                }
            }
        }
        else if ( nCommand == CommandEventId::CursorPos )
        {
            //  don't call InputChanged for CommandEventId::CursorPos
        }
        else if ( nCommand == CommandEventId::InputLanguageChange )
        {
            // #i55929# Font and font size state depends on input language if nothing is selected,
            // so the slots have to be invalidated when the input language is changed.

            SfxViewFrame* pViewFrm = SfxViewFrame::Current();
            if (pViewFrm)
            {
                SfxBindings& rBindings = pViewFrm->GetBindings();
                rBindings.Invalidate( SID_ATTR_CHAR_FONT );
                rBindings.Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
            }
        }
        else if ( nCommand == CommandEventId::Wheel )
        {
            //don't call InputChanged for CommandEventId::Wheel
        }
        else
            SC_MOD()->InputChanged( pEditView );
    }
    else
        Window::Command(rCEvt); // Or else let the base class handle it...

    bInputMode = false;
}

void ScTextWnd::StartDrag( sal_Int8 /* nAction */, const Point& rPosPixel )
{
    if ( pEditView )
    {
        CommandEvent aDragEvent( rPosPixel, CommandEventId::StartDrag, true );
        pEditView->Command( aDragEvent );

        //  handling of d&d to different view (CancelHandler) can't be done here,
        //  because the call returns before d&d is complete.
    }
}

void ScTextWnd::KeyInput(const KeyEvent& rKEvt)
{
    bInputMode = true;
    if (!SC_MOD()->InputKeyEvent( rKEvt ))
    {
        bool bUsed = false;
        ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
        if ( pViewSh )
            bUsed = pViewSh->SfxKeyInput(rKEvt); // Only accelerators, no input
        if (!bUsed)
            Window::KeyInput( rKEvt );
    }
    bInputMode = false;
}

void ScTextWnd::GetFocus()
{
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if ( pViewSh )
        pViewSh->SetFormShellAtTop( false ); // focus in input line -> FormShell no longer on top
}

void ScTextWnd::LoseFocus()
{
}

OUString ScTextWnd::GetText() const
{
    //  Override to get the text via the testtool
    if ( pEditEngine )
        return pEditEngine->GetText();
    else
        return GetTextString();
}

void ScTextWnd::SetFormulaMode( bool bSet )
{
    if ( bSet != bFormulaMode )
    {
        bFormulaMode = bSet;
        UpdateAutoCorrFlag();
    }
}

void ScTextWnd::UpdateAutoCorrFlag()
{
    if ( pEditEngine )
    {
        EEControlBits nControl = pEditEngine->GetControlWord();
        EEControlBits nOld = nControl;
        if ( bFormulaMode )
            nControl &= ~EEControlBits::AUTOCORRECT; // No AutoCorrect in Formulas
        else
            nControl |= EEControlBits::AUTOCORRECT; // Else do enable it

        if ( nControl != nOld )
            pEditEngine->SetControlWord( nControl );
    }
}

void ScTextWnd::StartEditEngine()
{
    // Don't activate if we're a modal ourselves (Doc modal Dialog)
    SfxObjectShell* pObjSh = SfxObjectShell::Current();
    if ( pObjSh && pObjSh->IsInModalMode() )
        return;

    if ( !pEditView || !pEditEngine )
    {
        ScFieldEditEngine* pNew;
        ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
        if ( pViewSh )
        {
            ScDocument* pDoc = pViewSh->GetViewData().GetDocument();
            pNew = new ScFieldEditEngine(pDoc, pDoc->GetEnginePool(), pDoc->GetEditPool());
        }
        else
            pNew = new ScFieldEditEngine(NULL, EditEngine::CreatePool(), NULL, true);
        pNew->SetExecuteURL( false );
        pEditEngine = pNew;

        pEditEngine->SetUpdateMode( false );
        pEditEngine->SetPaperSize( Size( bIsRTL ? USHRT_MAX : THESIZE, 300 ) );
        pEditEngine->SetWordDelimiters(
                        ScEditUtil::ModifyDelimiters( pEditEngine->GetWordDelimiters() ) );

        UpdateAutoCorrFlag();

        {
            SfxItemSet* pSet = new SfxItemSet( pEditEngine->GetEmptyItemSet() );
            EditEngine::SetFontInfoInItemSet( *pSet, aTextFont );
            lcl_ExtendEditFontAttribs( *pSet );

            // turn off script spacing to match DrawText output
            pSet->Put( SvxScriptSpaceItem( false, EE_PARA_ASIANCJKSPACING ) );
            if ( bIsRTL )
                lcl_ModifyRTLDefaults( *pSet );
            pEditEngine->SetDefaults( pSet );
        }

        // If the Cell contains URLFields, they need to be taken over into the entry row,
        // or else the position is not correct anymore
        bool bFilled = false;
        ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
        if ( pHdl ) //! Check if it's the right InputHdl?
            bFilled = pHdl->GetTextAndFields( *pEditEngine );

        pEditEngine->SetUpdateMode( true );

        //  aString is the truth...
        if (bFilled && pEditEngine->GetText() == aString)
            Invalidate(); // Repaint for (filled) Fields
        else
            pEditEngine->SetText(aString); // At least the right text then

        pEditView = new EditView( pEditEngine, this );
        pEditView->SetInsertMode(bIsInsertMode);

        // The text from the Clipboard is taken over as ASCII in a single row
        EVControlBits n = pEditView->GetControlWord();
        pEditView->SetControlWord( n | EVControlBits::SINGLELINEPASTE );

        pEditEngine->InsertView( pEditView, EE_APPEND );

        Resize();

        if ( bIsRTL )
            lcl_ModifyRTLVisArea( pEditView );

        pEditEngine->SetModifyHdl(LINK(this, ScTextWnd, NotifyHdl));

        if (!maAccTextDatas.empty())
            maAccTextDatas.back()->StartEdit();

        //  as long as EditEngine and DrawText sometimes differ for CTL text,
        //  repaint now to have the EditEngine's version visible
//        SfxObjectShell* pObjSh = SfxObjectShell::Current();
        if ( pObjSh && pObjSh->ISA(ScDocShell) )
        {
            ScDocument& rDoc = static_cast<ScDocShell*>(pObjSh)->GetDocument();    // any document
            SvtScriptType nScript = rDoc.GetStringScriptType( aString );
            if ( nScript & SvtScriptType::COMPLEX )
                Invalidate();
        }
    }

    SC_MOD()->SetInputMode( SC_INPUT_TOP );

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (pViewFrm)
        pViewFrm->GetBindings().Invalidate( SID_ATTR_INSERT );
}

IMPL_LINK_NOARG_TYPED(ScTextWnd, NotifyHdl, LinkParamNone*, void)
{
    if (pEditView && !bInputMode)
    {
        ScInputHandler* pHdl = SC_MOD()->GetInputHdl();

        //  Use the InputHandler's InOwnChange flag to prevent calling InputChanged
        //  while an InputHandler method is modifying the EditEngine content

        if ( pHdl && !pHdl->IsInOwnChange() )
            pHdl->InputChanged( pEditView, true );  // #i20282# InputChanged must know if called from modify handler
    }
}

void ScTextWnd::StopEditEngine( bool bAll )
{
    if (pEditView)
    {
        if (!maAccTextDatas.empty())
            maAccTextDatas.back()->EndEdit();

        ScModule* pScMod = SC_MOD();

        if (!bAll)
            pScMod->InputSelection( pEditView );
        aString = pEditEngine->GetText();
        bIsInsertMode = pEditView->IsInsertMode();
        bool bSelection = pEditView->HasSelection();
        pEditEngine->SetModifyHdl(Link<LinkParamNone*,void>());
        DELETEZ(pEditView);
        DELETEZ(pEditEngine);

        if ( pScMod->IsEditMode() && !bAll )
            pScMod->SetInputMode(SC_INPUT_TABLE);

        SfxViewFrame* pViewFrm = SfxViewFrame::Current();
        if (pViewFrm)
            pViewFrm->GetBindings().Invalidate( SID_ATTR_INSERT );

        if (bSelection)
            Invalidate(); // So that the Selection is not left there
    }
}

static sal_Int32 findFirstNonMatchingChar(const OUString& rStr1, const OUString& rStr2)
{
    // Search the string for unmatching chars
    const sal_Unicode*  pStr1 = rStr1.getStr();
    const sal_Unicode*  pStr2 = rStr2.getStr();
    sal_Int32      i = 0;
    while ( i < rStr1.getLength() )
    {
        // Abort on the first unmatching char
        if ( *pStr1 != *pStr2 )
            return i;
        ++pStr1,
        ++pStr2,
        ++i;
    }

    return i;
}

void ScTextWnd::SetTextString( const OUString& rNewString )
{
    if ( rNewString != aString )
    {
        bInputMode = true;

        // Find position of the change, only paint the rest
        if (!pEditEngine)
        {
            bool bPaintAll;
            if ( bIsRTL )
                bPaintAll = true;
            else
            {
                //  test if CTL script type is involved
                SvtScriptType nOldScript = SvtScriptType::NONE;
                SvtScriptType nNewScript = SvtScriptType::NONE;
                SfxObjectShell* pObjSh = SfxObjectShell::Current();
                if ( pObjSh && pObjSh->ISA(ScDocShell) )
                {
                    //  any document can be used (used only for its break iterator)
                    ScDocument& rDoc = static_cast<ScDocShell*>(pObjSh)->GetDocument();
                    nOldScript = rDoc.GetStringScriptType( aString );
                    nNewScript = rDoc.GetStringScriptType( rNewString );
                }
                bPaintAll = ( nOldScript & SvtScriptType::COMPLEX ) || ( nNewScript & SvtScriptType::COMPLEX );
            }

            if ( bPaintAll )
            {
                // if CTL is involved, the whole text has to be redrawn
                Invalidate();
            }
            else
            {
                long nTextSize = 0;
                sal_Int32 nDifPos;
                if (rNewString.getLength() > aString.getLength())
                    nDifPos = findFirstNonMatchingChar(rNewString, aString);
                else
                    nDifPos = findFirstNonMatchingChar(aString, rNewString);

                long nSize1 = GetTextWidth(aString);
                long nSize2 = GetTextWidth(rNewString);
                if ( nSize1>0 && nSize2>0 )
                    nTextSize = std::max( nSize1, nSize2 );
                else
                    nTextSize = GetOutputSize().Width(); // Overflow

                Point aLogicStart = PixelToLogic(Point(0,0));
                long nStartPos = aLogicStart.X();
                long nInvPos = nStartPos;
                if (nDifPos)
                    nInvPos += GetTextWidth(aString,0,nDifPos);

                InvalidateFlags nFlags = InvalidateFlags::NONE;
                if ( nDifPos == aString.getLength() ) // only new characters appended
                    nFlags = InvalidateFlags::NoErase;      // then background is already clear

                Invalidate( Rectangle( nInvPos, 0, nStartPos+nTextSize, GetOutputSize().Height()-1 ), nFlags );
            }
        }
        else
        {
            pEditEngine->SetText(rNewString);
        }

        aString = rNewString;

        if (!maAccTextDatas.empty())
            maAccTextDatas.back()->TextChanged();

        bInputMode = false;
    }
}

const OUString& ScTextWnd::GetTextString() const
{
    return aString;
}

bool ScTextWnd::IsInputActive()
{
    return HasFocus();
}

EditView* ScTextWnd::GetEditView()
{
    return pEditView;
}

void ScTextWnd::MakeDialogEditView()
{
    if ( pEditView ) return;

    ScFieldEditEngine* pNew;
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if ( pViewSh )
    {
        ScDocument* pDoc = pViewSh->GetViewData().GetDocument();
        pNew = new ScFieldEditEngine(pDoc, pDoc->GetEnginePool(), pDoc->GetEditPool());
    }
    else
        pNew = new ScFieldEditEngine(NULL, EditEngine::CreatePool(), NULL, true);
    pNew->SetExecuteURL( false );
    pEditEngine = pNew;

    pEditEngine->SetUpdateMode( false );
    pEditEngine->SetWordDelimiters( pEditEngine->GetWordDelimiters() + "=" );
    pEditEngine->SetPaperSize( Size( bIsRTL ? USHRT_MAX : THESIZE, 300 ) );

    SfxItemSet* pSet = new SfxItemSet( pEditEngine->GetEmptyItemSet() );
    EditEngine::SetFontInfoInItemSet( *pSet, aTextFont );
    lcl_ExtendEditFontAttribs( *pSet );
    if ( bIsRTL )
        lcl_ModifyRTLDefaults( *pSet );
    pEditEngine->SetDefaults( pSet );
    pEditEngine->SetUpdateMode( true );

    pEditView   = new EditView( pEditEngine, this );
    pEditEngine->InsertView( pEditView, EE_APPEND );

    Resize();

    if ( bIsRTL )
        lcl_ModifyRTLVisArea( pEditView );

    if (!maAccTextDatas.empty())
        maAccTextDatas.back()->StartEdit();
}

void ScTextWnd::ImplInitSettings()
{
    bIsRTL = AllSettings::GetLayoutRTL();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    Color aBgColor= rStyleSettings.GetWindowColor();
    Color aTxtColor= rStyleSettings.GetWindowTextColor();

    aTextFont.SetFillColor   ( aBgColor );
    aTextFont.SetColor       (aTxtColor);
    SetBackground           ( aBgColor );
    Invalidate();
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > ScTextWnd::CreateAccessible()
{
    return new ScAccessibleEditObject(GetAccessibleParentWindow()->GetAccessible(), NULL, this,
        OUString(ScResId(STR_ACC_EDITLINE_NAME)),
        OUString(ScResId(STR_ACC_EDITLINE_DESCR)), ScAccessibleEditObject::EditLine);
}

void ScTextWnd::InsertAccessibleTextData( ScAccessibleEditLineTextData& rTextData )
{
    OSL_ENSURE( ::std::find( maAccTextDatas.begin(), maAccTextDatas.end(), &rTextData ) == maAccTextDatas.end(),
        "ScTextWnd::InsertAccessibleTextData - passed object already registered" );
    maAccTextDatas.push_back( &rTextData );
}

void ScTextWnd::RemoveAccessibleTextData( ScAccessibleEditLineTextData& rTextData )
{
    AccTextDataVector::iterator aEnd = maAccTextDatas.end();
    AccTextDataVector::iterator aIt = ::std::find( maAccTextDatas.begin(), aEnd, &rTextData );
    OSL_ENSURE( aIt != aEnd, "ScTextWnd::RemoveAccessibleTextData - passed object not registered" );
    if( aIt != aEnd )
        maAccTextDatas.erase( aIt );
}

void ScTextWnd::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
    else
        Window::DataChanged( rDCEvt );
}

void ScTextWnd::TextGrabFocus()
{
    GrabFocus();
}

// Position window

ScPosWnd::ScPosWnd( vcl::Window* pParent ) :
    ComboBox    ( pParent, WinBits(WB_HIDE | WB_DROPDOWN) ),
    pAccel      ( NULL ),
    nTipVisible ( 0 ),
    bFormulaMode( false )
{
    Size aSize( GetTextWidth( OUString("GW99999:GW99999") ),
                GetTextHeight() );
    aSize.Width() += 25;    // FIXME: ??
    aSize.Height() = CalcWindowSizePixel(11); // Functions: 10 MRU + "others..."
    SetSizePixel( aSize );

    FillRangeNames();

    StartListening( *SfxGetpApp() ); // For Navigator rangename updates
}

ScPosWnd::~ScPosWnd()
{
    disposeOnce();
}

void ScPosWnd::dispose()
{
    EndListening( *SfxGetpApp() );

    HideTip();

    delete pAccel;
    ComboBox::dispose();
}

void ScPosWnd::SetFormulaMode( bool bSet )
{
    if ( bSet != bFormulaMode )
    {
        bFormulaMode = bSet;

        if ( bSet )
            FillFunctions();
        else
            FillRangeNames();

        HideTip();
    }
}

void ScPosWnd::SetPos( const OUString& rPosStr )
{
    if ( aPosStr != rPosStr )
    {
        aPosStr = rPosStr;
        SetText(aPosStr);
    }
}

namespace {

OUString createLocalRangeName(const OUString& rName, const OUString& rTableName)
{
    OUStringBuffer aString (rName);
    aString.append(" (");
    aString.append(rTableName);
    aString.append(")");
    return aString.makeStringAndClear();
}

}

void ScPosWnd::FillRangeNames()
{
    Clear();

    SfxObjectShell* pObjSh = SfxObjectShell::Current();
    if ( pObjSh && pObjSh->ISA(ScDocShell) )
    {
        ScDocument& rDoc = static_cast<ScDocShell*>(pObjSh)->GetDocument();

        InsertEntry(ScGlobal::GetRscString( STR_MANAGE_NAMES ));
        SetSeparatorPos(0);

        ScRange aDummy;
        std::set<OUString> aSet;
        ScRangeName* pRangeNames = rDoc.GetRangeName();
        if (!pRangeNames->empty())
        {
            ScRangeName::const_iterator itrBeg = pRangeNames->begin(), itrEnd = pRangeNames->end();
            for (ScRangeName::const_iterator itr = itrBeg; itr != itrEnd; ++itr)
            {
                if (itr->second->IsValidReference(aDummy))
                    aSet.insert(itr->second->GetName());
            }
        }
        for (SCTAB i = 0; i < rDoc.GetTableCount(); ++i)
        {
            ScRangeName* pLocalRangeName = rDoc.GetRangeName(i);
            if (pLocalRangeName && !pLocalRangeName->empty())
            {
                OUString aTableName;
                rDoc.GetName(i, aTableName);
                for (ScRangeName::const_iterator itr = pLocalRangeName->begin(); itr != pLocalRangeName->end(); ++itr)
                {
                    if (itr->second->IsValidReference(aDummy))
                        aSet.insert(createLocalRangeName(itr->second->GetName(), aTableName));
                }
            }
        }

        if (!aSet.empty())
        {
            for (std::set<OUString>::iterator itr = aSet.begin();
                    itr != aSet.end(); ++itr)
            {
                InsertEntry(*itr);
            }
        }
    }
    SetText(aPosStr);
}

void ScPosWnd::FillFunctions()
{
    Clear();

    OUString aFirstName;
    const ScAppOptions& rOpt = SC_MOD()->GetAppOptions();
    sal_uInt16 nMRUCount = rOpt.GetLRUFuncListCount();
    const sal_uInt16* pMRUList = rOpt.GetLRUFuncList();
    if (pMRUList)
    {
        const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
        sal_uLong nListCount = pFuncList->GetCount();
        for (sal_uInt16 i=0; i<nMRUCount; i++)
        {
            sal_uInt16 nId = pMRUList[i];
            for (sal_uLong j=0; j<nListCount; j++)
            {
                const ScFuncDesc* pDesc = pFuncList->GetFunction( j );
                if ( pDesc->nFIndex == nId && pDesc->pFuncName )
                {
                    InsertEntry( *pDesc->pFuncName );
                    if (aFirstName.isEmpty())
                        aFirstName = *pDesc->pFuncName;
                    break; // Stop searching
                }
            }
        }
    }

    //! Re-add entry "Other..." for Function AutoPilot if it can work with text that
    // has been entered so far

    //  InsertEntry( ScGlobal::GetRscString(STR_FUNCTIONLIST_MORE) );

    SetText(aFirstName);
}

void ScPosWnd::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( !bFormulaMode )
    {
        // Does the list of range names need updating?
        const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
        if ( pSimpleHint )
        {
            sal_uLong nHintId = pSimpleHint->GetId();
            if ( nHintId == SC_HINT_AREAS_CHANGED || nHintId == SC_HINT_NAVIGATOR_UPDATEALL)
                FillRangeNames();
        }
        else if ( dynamic_cast<const SfxEventHint*>(&rHint) )
        {
            sal_uLong nEventId = static_cast<const SfxEventHint*>(&rHint)->GetEventId();
            if ( nEventId == SFX_EVENT_ACTIVATEDOC )
                FillRangeNames();
        }
    }
}

void ScPosWnd::HideTip()
{
    if ( nTipVisible )
    {
        Help::HideTip( nTipVisible );
        nTipVisible = 0;
    }
}

static ScNameInputType lcl_GetInputType( const OUString& rText )
{
    ScNameInputType eRet = SC_NAME_INPUT_BAD_NAME;      // the more general error

    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if ( pViewSh )
    {
        ScViewData& rViewData = pViewSh->GetViewData();
        ScDocument* pDoc = rViewData.GetDocument();
        SCTAB nTab = rViewData.GetTabNo();
        formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

        // test in same order as in SID_CURRENTCELL execute

        ScRange aRange;
        ScAddress aAddress;
        ScRangeUtil aRangeUtil;
        SCTAB nNameTab;
        sal_Int32 nNumeric;

        if (rText == ScGlobal::GetRscString(STR_MANAGE_NAMES))
            eRet = SC_MANAGE_NAMES;
        else if ( aRange.Parse( rText, pDoc, eConv ) & SCA_VALID )
            eRet = SC_NAME_INPUT_RANGE;
        else if ( aAddress.Parse( rText, pDoc, eConv ) & SCA_VALID )
            eRet = SC_NAME_INPUT_CELL;
        else if ( ScRangeUtil::MakeRangeFromName( rText, pDoc, nTab, aRange, RUTL_NAMES, eConv ) )
            eRet = SC_NAME_INPUT_NAMEDRANGE;
        else if ( ScRangeUtil::MakeRangeFromName( rText, pDoc, nTab, aRange, RUTL_DBASE, eConv ) )
            eRet = SC_NAME_INPUT_DATABASE;
        else if ( comphelper::string::isdigitAsciiString( rText ) &&
                  ( nNumeric = rText.toInt32() ) > 0 && nNumeric <= MAXROW+1 )
            eRet = SC_NAME_INPUT_ROW;
        else if ( pDoc->GetTable( rText, nNameTab ) )
            eRet = SC_NAME_INPUT_SHEET;
        else if ( ScRangeData::IsNameValid( rText, pDoc ) )     // nothing found, create new range?
        {
            if ( rViewData.GetSimpleArea( aRange ) == SC_MARK_SIMPLE )
                eRet = SC_NAME_INPUT_DEFINE;
            else
                eRet = SC_NAME_INPUT_BAD_SELECTION;
        }
        else
            eRet = SC_NAME_INPUT_BAD_NAME;
    }

    return eRet;
}

void ScPosWnd::Modify()
{
    ComboBox::Modify();

    HideTip();

    if ( !IsTravelSelect() && !bFormulaMode )
    {
        // determine the action that would be taken for the current input

        ScNameInputType eType = lcl_GetInputType( GetText() );      // uses current view
        sal_uInt16 nStrId = 0;
        switch ( eType )
        {
            case SC_NAME_INPUT_CELL:
                nStrId = STR_NAME_INPUT_CELL;
                break;
            case SC_NAME_INPUT_RANGE:
            case SC_NAME_INPUT_NAMEDRANGE:
                nStrId = STR_NAME_INPUT_RANGE;      // named range or range reference
                break;
            case SC_NAME_INPUT_DATABASE:
                nStrId = STR_NAME_INPUT_DBRANGE;
                break;
            case SC_NAME_INPUT_ROW:
                nStrId = STR_NAME_INPUT_ROW;
                break;
            case SC_NAME_INPUT_SHEET:
                nStrId = STR_NAME_INPUT_SHEET;
                break;
            case SC_NAME_INPUT_DEFINE:
                nStrId = STR_NAME_INPUT_DEFINE;
                break;
            default:
                // other cases (error): no tip help
                break;
        }

        if ( nStrId )
        {
            // show the help tip at the text cursor position

            vcl::Window* pWin = GetSubEdit();
            if (!pWin)
                pWin = this;
            Point aPos;
            vcl::Cursor* pCur = pWin->GetCursor();
            if (pCur)
                aPos = pWin->LogicToPixel( pCur->GetPos() );
            aPos = pWin->OutputToScreenPixel( aPos );
            Rectangle aRect( aPos, aPos );

            OUString aText = ScGlobal::GetRscString( nStrId );
            QuickHelpFlags nAlign = QuickHelpFlags::Left|QuickHelpFlags::Bottom;
            nTipVisible = Help::ShowTip(pWin, aRect, aText, nAlign);
        }
    }
}

void ScPosWnd::Select()
{
    ComboBox::Select(); //  In VCL GetText() only return the selected entry afterwards

    HideTip();

    if (!IsTravelSelect())
        DoEnter();
}

void ScPosWnd::DoEnter()
{
    OUString aText = GetText();
    if ( !aText.isEmpty() )
    {
        if ( bFormulaMode )
        {
            ScModule* pScMod = SC_MOD();
            if ( aText == ScGlobal::GetRscString(STR_FUNCTIONLIST_MORE) )
            {
                // Function AutoPilot
                //! Continue working with the text entered so far

                //! new method at ScModule to query if function autopilot is open
                SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                if ( pViewFrm && !pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) )
                    pViewFrm->GetDispatcher()->Execute( SID_OPENDLG_FUNCTION,
                                              SfxCallMode::SYNCHRON | SfxCallMode::RECORD );
            }
            else
            {
                ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
                ScInputHandler* pHdl = pScMod->GetInputHdl( pViewSh );
                if (pHdl)
                    pHdl->InsertFunction( aText );
            }
        }
        else
        {
            // depending on the input, select something or create a new named range

            ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
            if ( pViewSh )
            {
                ScViewData& rViewData = pViewSh->GetViewData();
                ScDocShell* pDocShell = rViewData.GetDocShell();
                ScDocument& rDoc = pDocShell->GetDocument();

                ScNameInputType eType = lcl_GetInputType( aText );
                if ( eType == SC_NAME_INPUT_BAD_NAME || eType == SC_NAME_INPUT_BAD_SELECTION )
                {
                    sal_uInt16 nId = ( eType == SC_NAME_INPUT_BAD_NAME ) ? STR_NAME_ERROR_NAME : STR_NAME_ERROR_SELECTION;
                    pViewSh->ErrorMessage( nId );
                }
                else if ( eType == SC_NAME_INPUT_DEFINE )
                {
                    ScRangeName* pNames = rDoc.GetRangeName();
                    ScRange aSelection;
                    if ( pNames && !pNames->findByUpperName(ScGlobal::pCharClass->uppercase(aText)) &&
                            (rViewData.GetSimpleArea( aSelection ) == SC_MARK_SIMPLE) )
                    {
                        ScRangeName aNewRanges( *pNames );
                        ScAddress aCursor( rViewData.GetCurX(), rViewData.GetCurY(), rViewData.GetTabNo() );
                        OUString aContent(aSelection.Format(SCR_ABS_3D, &rDoc, rDoc.GetAddressConvention()));
                        ScRangeData* pNew = new ScRangeData( &rDoc, aText, aContent, aCursor );
                        if ( aNewRanges.insert(pNew) )
                        {
                            pDocShell->GetDocFunc().ModifyRangeNames( aNewRanges );
                            pViewSh->UpdateInputHandler(true);
                        }
                    }
                }
                else if (eType == SC_MANAGE_NAMES)
                {
                    sal_uInt16          nId  = ScNameDlgWrapper::GetChildWindowId();
                    SfxViewFrame* pViewFrm = pViewSh->GetViewFrame();
                    SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                    SC_MOD()->SetRefDialog( nId, pWnd == nullptr );
                }
                else
                {
                    // for all selection types, execute the SID_CURRENTCELL slot.
                    if (eType == SC_NAME_INPUT_CELL || eType == SC_NAME_INPUT_RANGE)
                    {
                        // Note that SID_CURRENTCELL always expects address to
                        // be in Calc A1 format.  Convert the text.
                        ScRange aRange(0,0, rViewData.GetTabNo());
                        aRange.ParseAny(aText, &rDoc, rDoc.GetAddressConvention());
                        aText = aRange.Format(SCR_ABS_3D, &rDoc, ::formula::FormulaGrammar::CONV_OOO);
                    }

                    SfxStringItem aPosItem( SID_CURRENTCELL, aText );
                    SfxBoolItem aUnmarkItem( FN_PARAM_1, true );        // remove existing selection

                    pViewSh->GetViewData().GetDispatcher().Execute( SID_CURRENTCELL,
                                        SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
                                        &aPosItem, &aUnmarkItem, 0L );
                }
            }
        }
    }
    else
        SetText( aPosStr );

    ReleaseFocus_Impl();
}

bool ScPosWnd::Notify( NotifyEvent& rNEvt )
{
    bool bHandled = true;

    switch (rNEvt.GetType())
    {
        case MouseNotifyEvent::KEYINPUT:
        {
            const KeyEvent* pKEvt = rNEvt.GetKeyEvent();

            switch ( pKEvt->GetKeyCode().GetCode() )
            {
                case KEY_RETURN:
                    DoEnter();
                    break;

                case KEY_ESCAPE:
                    if (nTipVisible)
                    {
                        // escape when the tip help is shown: only hide the tip
                        HideTip();
                    }
                    else
                    {
                        if (!bFormulaMode)
                            SetText( aPosStr );
                        ReleaseFocus_Impl();
                    }
                    break;

                default:
                    bHandled = false;
                    break;
            }
        }
        break;
        case MouseNotifyEvent::GETFOCUS:
        {
            // Select the whole text upon focus.
            OUString aStr = GetText();
            SetSelection(Selection(0, aStr.getLength()));
        }
        break;
        case MouseNotifyEvent::LOSEFOCUS:
            HideTip();
            bHandled = false;
        break;
        default:
            bHandled = false;
    }

    if (!bHandled)
        bHandled = ComboBox::Notify(rNEvt);

    return bHandled;
}

void ScPosWnd::ReleaseFocus_Impl()
{
    HideTip();

    SfxViewShell* pCurSh = SfxViewShell::Current();
    ScInputHandler* pHdl = SC_MOD()->GetInputHdl( PTR_CAST( ScTabViewShell, pCurSh ) );
    if ( pHdl && pHdl->IsTopMode() )
    {
        // Focus back in input row?
        ScInputWindow* pInputWin = pHdl->GetInputWindow();
        if (pInputWin)
        {
            pInputWin->TextGrabFocus();
            return;
        }
    }

    // Set focus to active View
    if ( pCurSh )
    {
        vcl::Window* pShellWnd = pCurSh->GetWindow();

        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
