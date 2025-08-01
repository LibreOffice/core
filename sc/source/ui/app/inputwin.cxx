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

#include <memory>
#include <algorithm>
#include <string_view>

#include <editeng/eeitem.hxx>

#include <sfx2/app.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/editview.hxx>
#include <editeng/editstat.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/langitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/event.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/cursor.hxx>
#include <vcl/help.hxx>
#include <vcl/settings.hxx>
#include <svl/stritem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weldutils.hxx>
#include <unotools/charclass.hxx>

#include <inputwin.hxx>
#include <scmod.hxx>
#include <global.hxx>
#include <scresid.hxx>
#include <strings.hrc>
#include <globstr.hrc>
#include <bitmaps.hlst>
#include <reffact.hxx>
#include <editutil.hxx>
#include <inputhdl.hxx>
#include <tabvwsh.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <appoptio.hxx>
#include <rangenam.hxx>
#include <rangeutl.hxx>
#include <docfunc.hxx>
#include <funcdesc.hxx>
#include <editeng/fontitem.hxx>
#include <AccessibleEditObject.hxx>
#include <AccessibleText.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <helpids.h>
#include <output.hxx>

namespace com::sun::star::accessibility { class XAccessible; }

const tools::Long THESIZE = 1000000;            // Should be more than enough!
const tools::Long INPUTLINE_INSET_MARGIN = 2;   // Space between border and interior widgets of input line
const tools::Long LEFT_OFFSET = 5;              // Left offset of input line
//TODO const long BUTTON_OFFSET = 2;            // Space between input line and button to expand/collapse
const tools::Long INPUTWIN_MULTILINES = 6;      // Initial number of lines within multiline dropdown
const tools::Long TOOLBOX_WINDOW_HEIGHT = 22;   // Height of toolbox window in pixels - TODO: The same on all systems?
const tools::Long POSITION_COMBOBOX_WIDTH = 18; // Width of position combobox in characters
const int RESIZE_HOTSPOT_HEIGHT = 4;

using com::sun::star::uno::Reference;
using com::sun::star::uno::UNO_QUERY;

using com::sun::star::frame::XLayoutManager;
using com::sun::star::beans::XPropertySet;

namespace {

constexpr ToolBoxItemId SID_INPUT_FUNCTION (SC_VIEW_START + 47);
constexpr ToolBoxItemId SID_INPUT_SUM     (SC_VIEW_START + 48);
constexpr ToolBoxItemId SID_INPUT_EQUAL   (SC_VIEW_START + 49);
constexpr ToolBoxItemId SID_INPUT_CANCEL  (SC_VIEW_START + 50);
constexpr ToolBoxItemId SID_INPUT_OK      (SC_VIEW_START + 51);

enum ScNameInputType
{
    SC_NAME_INPUT_CELL,
    SC_NAME_INPUT_RANGE,
    SC_NAME_INPUT_NAMEDRANGE_LOCAL,
    SC_NAME_INPUT_NAMEDRANGE_GLOBAL,
    SC_NAME_INPUT_DATABASE,
    SC_NAME_INPUT_ROW,
    SC_NAME_INPUT_SHEET,
    SC_NAME_INPUT_DEFINE,
    SC_NAME_INPUT_BAD_NAME,
    SC_NAME_INPUT_BAD_SELECTION,
    SC_MANAGE_NAMES
};

}

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


static ScTabViewShell* lcl_chooseRuntimeImpl( const SfxBindings* pBind )
{
    ScTabViewShell* pViewSh = nullptr;
    SfxDispatcher* pDisp = pBind->GetDispatcher();
    if ( pDisp )
    {
        SfxViewFrame* pViewFrm = pDisp->GetFrame();
        if ( pViewFrm )
            pViewSh = dynamic_cast<ScTabViewShell*>( pViewFrm->GetViewShell()  );
    }
    return pViewSh;
}

ScInputWindow::ScInputWindow( vcl::Window* pParent, const SfxBindings* pBind ) :
        // With WB_CLIPCHILDREN otherwise we get flickering
        ToolBox         ( pParent, WinBits(WB_CLIPCHILDREN | WB_BORDER | WB_NOSHADOW) ),
        aWndPos         ( VclPtr<ScPosWnd>::Create( this, lcl_chooseRuntimeImpl(pBind)) ),
        mxTextWindow    ( VclPtr<ScInputBarGroup>::Create( this, lcl_chooseRuntimeImpl(pBind)) ),
        pInputHdl       ( nullptr ),
        mpViewShell     ( nullptr ),
        mnMaxY          (0),
        mnStandardItemHeight(0),
        bIsOkCancelMode ( false ),
        bInResize       ( false )
{
    // #i73615# don't rely on SfxViewShell::Current while constructing the input line
    // (also for GetInputHdl below)
    ScTabViewShell* pViewSh = nullptr;
    SfxDispatcher* pDisp = pBind->GetDispatcher();
    if ( pDisp )
    {
        SfxViewFrame* pViewFrm = pDisp->GetFrame();
        if ( pViewFrm )
            pViewSh = dynamic_cast<ScTabViewShell*>( pViewFrm->GetViewShell()  );
    }
    OSL_ENSURE( pViewSh, "no view shell for input window" );

    mpViewShell = pViewSh;

    // Position window, 3 buttons, input window
    if (!comphelper::LibreOfficeKit::isActive())
    {
        InsertWindow    (ToolBoxItemId(1), aWndPos.get(), ToolBoxItemBits::NONE, 0);
        InsertSeparator (1);
        InsertItem      (SID_INPUT_FUNCTION, Image(StockImage::Yes, RID_BMP_INPUT_FUNCTION), ToolBoxItemBits::NONE, 2);
    }

    const bool bIsLOKMobilePhone = mpViewShell && mpViewShell->isLOKMobilePhone();

    // sigma and equal buttons
    if (!bIsLOKMobilePhone)
    {
        InsertItem      (SID_INPUT_SUM,      Image(StockImage::Yes, RID_BMP_INPUT_SUM), ToolBoxItemBits::DROPDOWN, 3);
        InsertItem      (SID_INPUT_EQUAL,    Image(StockImage::Yes, RID_BMP_INPUT_EQUAL), ToolBoxItemBits::NONE, 4);
        InsertItem      (SID_INPUT_CANCEL,   Image(StockImage::Yes, RID_BMP_INPUT_CANCEL), ToolBoxItemBits::NONE, 5);
        InsertItem      (SID_INPUT_OK,       Image(StockImage::Yes, RID_BMP_INPUT_OK), ToolBoxItemBits::NONE, 6);
    }

    InsertWindow    (ToolBoxItemId(7), mxTextWindow.get(), ToolBoxItemBits::NONE, 7);
    SetDropdownClickHdl( LINK( this, ScInputWindow, DropdownClickHdl ));

    if (!comphelper::LibreOfficeKit::isActive())
    {
        aWndPos   ->SetQuickHelpText(ScResId(SCSTR_QHELP_POSWND));
        aWndPos   ->SetHelpId       (HID_INSWIN_POS);

        mxTextWindow->SetQuickHelpText(ScResId(SCSTR_QHELP_INPUTWND));
        mxTextWindow->SetHelpId       (HID_INSWIN_INPUT);

        // No SetHelpText: the helptexts come from the Help
        SetItemText (SID_INPUT_FUNCTION, ScResId(SCSTR_QHELP_BTNCALC));
        SetHelpId   (SID_INPUT_FUNCTION, HID_INSWIN_CALC);
    }

    // sigma and equal buttons
    if (!bIsLOKMobilePhone)
    {
        SetHelpId   (SID_INPUT_SUM, HID_INSWIN_SUMME);
        SetHelpId   (SID_INPUT_EQUAL, HID_INSWIN_FUNC);
        SetHelpId   (SID_INPUT_CANCEL, HID_INSWIN_CANCEL);
        SetHelpId   (SID_INPUT_OK, HID_INSWIN_OK);

        if (!comphelper::LibreOfficeKit::isActive())
        {
            SetItemText ( SID_INPUT_SUM, ScResId( SCSTR_QHELP_BTNSUM ) );
            SetItemText ( SID_INPUT_EQUAL, ScResId( SCSTR_QHELP_BTNEQUAL ) );
            SetItemText ( SID_INPUT_CANCEL, ScResId( SCSTR_QHELP_BTNCANCEL ) );
            SetItemText ( SID_INPUT_OK, ScResId( SCSTR_QHELP_BTNOK ) );
        }

        EnableItem( SID_INPUT_CANCEL, false );
        EnableItem( SID_INPUT_OK, false );

        HideItem( SID_INPUT_CANCEL );
        HideItem( SID_INPUT_OK );

        mnStandardItemHeight = GetItemRect(SID_INPUT_SUM).GetHeight();
    }

    SetHelpId( HID_SC_INPUTWIN ); // For the whole input row

    if (!comphelper::LibreOfficeKit::isActive())
        aWndPos   ->Show();
    mxTextWindow->Show();

    pInputHdl = ScModule::get()->GetInputHdl( pViewSh, false ); // use own handler even if ref-handler is set
    if (pInputHdl)
        pInputHdl->SetInputWindow( this );

    if (pInputHdl && !pInputHdl->GetFormString().isEmpty())
    {
        // Switch over while the Function AutoPilot is active
        // -> show content of the Function AutoPilot again
        // Also show selection (remember at the InputHdl)
        mxTextWindow->SetTextString(pInputHdl->GetFormString(), true);
    }
    else if (pInputHdl && pInputHdl->IsInputMode())
    {
        // If the input row was hidden while editing (e.g. when editing a formula
        // and then switching to another document or the help), display the text
        // we just edited from the InputHandler
        mxTextWindow->SetTextString(pInputHdl->GetEditString(), true); // Display text
        if ( pInputHdl->IsTopMode() )
            pInputHdl->SetMode( SC_INPUT_TABLE ); // Focus ends up at the bottom anyways
    }
    else if (pViewSh)
    {
        // Don't stop editing in LOK a remote user might be editing.
        const bool bStopEditing = !comphelper::LibreOfficeKit::isActive();
        pViewSh->UpdateInputHandler(true, bStopEditing); // Absolutely necessary update
    }

    SetToolbarLayoutMode( ToolBoxLayoutMode::Locked );

    SetAccessibleName(ScResId(STR_ACC_TOOLBAR_FORMULA));
}

ScInputWindow::~ScInputWindow()
{
    disposeOnce();
}

void ScInputWindow::dispose()
{
    bool bDown = !ScGlobal::oSysLocale; // after Clear?

    //  if any view's input handler has a pointer to this input window, reset it
    //  (may be several ones, #74522#)
    //  member pInputHdl is not used here

    if ( !bDown )
    {
        SfxViewShell* pSh = SfxViewShell::GetFirst( true, checkSfxViewShell<ScTabViewShell> );
        while ( pSh )
        {
            ScInputHandler* pHdl = static_cast<ScTabViewShell*>(pSh)->GetInputHandler();
            if ( pHdl && pHdl->GetInputWindow() == this )
            {
                pHdl->SetInputWindow( nullptr );
                pHdl->StopInputWinEngine( false );  // reset pTopView pointer
            }
            pSh = SfxViewShell::GetNext( *pSh, true, checkSfxViewShell<ScTabViewShell> );
        }
    }

    if (comphelper::LibreOfficeKit::isActive())
    {
        if (GetLOKNotifier())
            ReleaseLOKNotifier();
    }

    mxTextWindow.disposeAndClear();
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

void ScInputWindow::Select()
{
    ScModule* pScMod = ScModule::get();
    ToolBox::Select();

    ToolBoxItemId curItemId = GetCurItemId();
    if (curItemId == SID_INPUT_FUNCTION)
    {
        //! new method at ScModule to query if function autopilot is open
        SfxViewFrame* pViewFrm = SfxViewFrame::Current();
        if ( pViewFrm && ( comphelper::LibreOfficeKit::isActive() || !pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) ) )
        {
            pViewFrm->GetDispatcher()->Execute( SID_OPENDLG_FUNCTION,
                                        SfxCallMode::SYNCHRON | SfxCallMode::RECORD );

            // The Toolbox will be disabled anyways, so we don't need to switch here,
            // regardless whether it succeeded or not!
//                  SetOkCancelMode();
        }
    }
    else if (curItemId == SID_INPUT_CANCEL)
    {
        pScMod->InputCancelHandler();
        SetSumAssignMode();
    }
    else if (curItemId == SID_INPUT_OK)
    {
        pScMod->InputEnterHandler();
        SetSumAssignMode();
        mxTextWindow->Invalidate(); // Or else the Selection remains
    }
    else if (curItemId == SID_INPUT_SUM)
    {
        bool bRangeFinder = false;
        bool bSubTotal = false;
        AutoSum(bRangeFinder, bSubTotal, ocSum);
    }
    else if (curItemId == SID_INPUT_EQUAL)
    {
        StartFormula();
    }
}

void ScInputWindow::StartFormula()
{
    ScModule* pScMod = ScModule::get();
    mxTextWindow->StartEditEngine();
    if ( pScMod->IsEditMode() ) // Isn't if e.g. protected
    {
        mxTextWindow->StartEditEngine();

        sal_Int32 nStartPos = 1;
        sal_Int32 nEndPos = 1;

        ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>( SfxViewShell::Current()  );
        if ( pViewSh )
        {
            const OUString& rString = mxTextWindow->GetTextString();
            const sal_Int32 nLen = rString.getLength();

            ScDocument& rDoc = pViewSh->GetViewData().GetDocument();
            CellType eCellType = rDoc.GetCellType( pViewSh->GetViewData().GetCurPos() );
            switch ( eCellType )
            {
                case CELLTYPE_VALUE:
                {
                    nEndPos = nLen + 1;
                    mxTextWindow->SetTextString("=" +  rString, true);
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
                    mxTextWindow->SetTextString(u"="_ustr, true);
                    break;
            }
        }

        EditView* pView = mxTextWindow->GetEditView();
        if (pView)
        {
            sal_Int32 nStartPara = 0, nEndPara = 0;
            if (comphelper::LibreOfficeKit::isActive())
            {
                TextGrabFocus();
                if (pViewSh && !pViewSh->isLOKDesktop())
                {
                    nStartPara = nEndPara = pView->getEditEngine().GetParagraphCount() ?
                        (pView->getEditEngine().GetParagraphCount() - 1) : 0;
                    nStartPos = nEndPos = pView->getEditEngine().GetTextLen(nStartPara);
                }
            }
            pView->SetSelection(ESelection(nStartPara, nStartPos, nEndPara, nEndPos));
            pScMod->InputChanged(pView);
            SetOkCancelMode();
            pView->SetEditEngineUpdateLayout(true);
        }
    }
}

void ScInputWindow::PixelInvalidate(const tools::Rectangle* pRectangle)
{
    if (comphelper::LibreOfficeKit::isDialogPainting() || !comphelper::LibreOfficeKit::isActive())
        return;

    if (pRectangle)
    {
        tools::Rectangle aRect(*pRectangle);
        aRect.Move(-GetOutOffXPixel(), -GetOutOffYPixel());
        Window::PixelInvalidate(&aRect);
    }
    else
    {
        Window::PixelInvalidate(nullptr);
    }
}

void ScInputWindow::SetSizePixel( const Size& rNewSize )
{
    const vcl::ILibreOfficeKitNotifier* pNotifier = GetLOKNotifier();
    if (pNotifier)
    {
        if (vcl::Window* pFrameWindowImpl = GetParent())
        {
            if (vcl::Window* pWorkWindow = pFrameWindowImpl->GetParent())
            {
                if (vcl::Window* pImplBorderWindow = pWorkWindow->GetParent())
                {
                    Size aSize = pImplBorderWindow->GetSizePixel();
                    aSize.setWidth(rNewSize.getWidth());
                    pImplBorderWindow->SetSizePixel(aSize);
                }
            }
        }
    }

    ToolBox::SetSizePixel(rNewSize);
}

void ScInputWindow::Resize()
{
    ToolBox::Resize();

    Size aStartSize = GetSizePixel();
    Size aSize = aStartSize;

    auto nLines = mxTextWindow->GetNumLines();
    //(-10) to allow margin between sidebar and formulabar
    tools::Long margin = (comphelper::LibreOfficeKit::isActive()) ? 10 : 0;
    Size aTextWindowSize(aSize.Width() - mxTextWindow->GetPosPixel().X() - LEFT_OFFSET - margin,
                         mxTextWindow->GetPixelHeightForLines(nLines));
    mxTextWindow->SetSizePixel(aTextWindowSize);

    int nTopOffset = 0;
    if (nLines > 1)
    {
        // Initially there is 1 line and the edit is vertically centered in the toolbar
        // Later, if expanded then the vertical position of the edit will remain at
        // that initial position, so when calculating the overall size of the expanded
        // toolbar we have to include that initial offset in order to not make
        // the edit overlap the RESIZE_HOTSPOT_HEIGHT area so that dragging to resize
        // is still possible.
        int nNormalHeight = mxTextWindow->GetPixelHeightForLines(1);
        int nInitialTopMargin = (mnStandardItemHeight - nNormalHeight) / 2;
        if (nInitialTopMargin > 0)
            nTopOffset = nInitialTopMargin;
    }

    // add empty space of RESIZE_HOTSPOT_HEIGHT so resize is possible when hovering there
    aSize.setHeight(CalcWindowSizePixel().Height() + RESIZE_HOTSPOT_HEIGHT + nTopOffset);

    if (aStartSize != aSize)
        SetSizePixel(aSize);

    Invalidate();
}

void ScInputWindow::NotifyLOKClient()
{
    if (comphelper::LibreOfficeKit::isActive() && !GetLOKNotifier() && mpViewShell)
        SetLOKNotifier(mpViewShell);
}

void ScInputWindow::SetFuncString( const OUString& rString, bool bDoEdit )
{
    //! new method at ScModule to query if function autopilot is open
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    EnableButtons( pViewFrm && !pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) );
    mxTextWindow->StartEditEngine();

    ScModule* pScMod = ScModule::get();
    if ( !pScMod->IsEditMode() )
        return;

    if ( bDoEdit )
        mxTextWindow->TextGrabFocus();
    mxTextWindow->SetTextString(rString, true);
    EditView* pView = mxTextWindow->GetEditView();
    if (!pView)
        return;

    sal_Int32 nLen = rString.getLength();

    if ( nLen > 0 )
    {
        nLen--;
        pView->SetSelection(ESelection(0, nLen));
    }

    pScMod->InputChanged(pView);
    if ( bDoEdit )
        SetOkCancelMode(); // Not the case if immediately followed by Enter/Cancel

    pView->SetEditEngineUpdateLayout(true);
}

void ScInputWindow::SetPosString( const OUString& rStr )
{
    aWndPos->SetPos( rStr );
}

void ScInputWindow::SetTextString( const OUString& rString, bool bKitUpdate )
{
    if (rString.getLength() <= 32767)
        mxTextWindow->SetTextString(rString, bKitUpdate);
    else
        mxTextWindow->SetTextString(rString.copy(0, 32767), bKitUpdate);
}

void ScInputWindow::SetOkCancelMode()
{
    //! new method at ScModule to query if function autopilot is open
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    EnableButtons( pViewFrm && !pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) );

    if (bIsOkCancelMode)
        return;

    EnableItem  ( SID_INPUT_SUM,   false );
    EnableItem  ( SID_INPUT_EQUAL, false );
    HideItem    ( SID_INPUT_SUM );
    HideItem    ( SID_INPUT_EQUAL );

    ShowItem    ( SID_INPUT_CANCEL, true );
    ShowItem    ( SID_INPUT_OK,     true );
    EnableItem  ( SID_INPUT_CANCEL, true );
    EnableItem  ( SID_INPUT_OK,     true );

    bIsOkCancelMode = true;
}

void ScInputWindow::SetSumAssignMode()
{
    //! new method at ScModule to query if function autopilot is open
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    EnableButtons( pViewFrm && !pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) );

    if (!bIsOkCancelMode)
        return;

    EnableItem  ( SID_INPUT_CANCEL, false );
    EnableItem  ( SID_INPUT_OK,     false );
    HideItem    ( SID_INPUT_CANCEL );
    HideItem    ( SID_INPUT_OK );

    ShowItem    ( SID_INPUT_SUM,    true );
    ShowItem    ( SID_INPUT_EQUAL,  true );
    EnableItem  ( SID_INPUT_SUM,    true );
    EnableItem  ( SID_INPUT_EQUAL,  true );

    bIsOkCancelMode = false;

    SetFormulaMode(false); // No editing -> no formula
}

void ScInputWindow::SetFormulaMode( bool bSet )
{
    if (!comphelper::LibreOfficeKit::isActive())
        aWndPos->SetFormulaMode(bSet);
    mxTextWindow->SetFormulaMode(bSet);
}

bool ScInputWindow::IsInputActive()
{
    return mxTextWindow->IsInputActive();
}

EditView* ScInputWindow::GetEditView()
{
    return mxTextWindow->GetEditView();
}

vcl::Window* ScInputWindow::GetEditWindow()
{
    return mxTextWindow;
}

Point ScInputWindow::GetCursorScreenPixelPos(bool bBelow)
{
    return mxTextWindow->GetCursorScreenPixelPos(bBelow);
}

void ScInputWindow::MakeDialogEditView()
{
    mxTextWindow->MakeDialogEditView();
}

void ScInputWindow::StopEditEngine( bool bAll )
{
    mxTextWindow->StopEditEngine( bAll );
}

void ScInputWindow::TextGrabFocus()
{
    mxTextWindow->TextGrabFocus();
}

void ScInputWindow::TextInvalidate()
{
    mxTextWindow->Invalidate();
}

void ScInputWindow::SwitchToTextWin()
{
    // used for shift-ctrl-F2

    mxTextWindow->StartEditEngine();
    if (ScModule::get()->IsEditMode())
    {
        mxTextWindow->TextGrabFocus();
        EditView* pView = mxTextWindow->GetEditView();
        if (pView)
        {
            pView->SetSelection(ESelection::AtEnd()); // set cursor to end of text
        }
    }
}

void ScInputWindow::PosGrabFocus()
{
    if (!comphelper::LibreOfficeKit::isActive())
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

void ScInputWindow::NumLinesChanged()
{
    mxTextWindow->NumLinesChanged();
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
        SetItemImage(SID_INPUT_FUNCTION, Image(StockImage::Yes, RID_BMP_INPUT_FUNCTION));
        if ( bIsOkCancelMode )
        {
            SetItemImage(SID_INPUT_CANCEL, Image(StockImage::Yes, RID_BMP_INPUT_CANCEL));
            SetItemImage(SID_INPUT_OK,     Image(StockImage::Yes, RID_BMP_INPUT_OK));
        }
        else
        {
            SetItemImage(SID_INPUT_SUM,   Image(StockImage::Yes, RID_BMP_INPUT_SUM));
            SetItemImage(SID_INPUT_EQUAL, Image(StockImage::Yes, RID_BMP_INPUT_EQUAL));
        }
    }

    ToolBox::DataChanged( rDCEvt );
}

bool ScInputWindow::IsPointerAtResizePos()
{
    return GetOutputSizePixel().Height() - GetPointerPosPixel().Y() <= RESIZE_HOTSPOT_HEIGHT;
}

void ScInputWindow::MouseMove( const MouseEvent& rMEvt )
{
    Point aPosPixel = GetPointerPosPixel();

    ScInputBarGroup* pGroupBar = mxTextWindow.get();

    if (bInResize || IsPointerAtResizePos())
        SetPointer(PointerStyle::WindowSSize);
    else
        SetPointer(PointerStyle::Arrow);

    if (bInResize)
    {
        // detect direction
        tools::Long nResizeThreshold = tools::Long(TOOLBOX_WINDOW_HEIGHT * 0.7);
        bool bResetPointerPos = false;

        // Detect attempt to expand toolbar too much
        if (aPosPixel.Y() >= mnMaxY)
        {
            bResetPointerPos = true;
            aPosPixel.setY( mnMaxY );
        } // or expanding down
        else if (GetOutputSizePixel().Height() - aPosPixel.Y() < -nResizeThreshold)
        {
            pGroupBar->IncrementVerticalSize();
            bResetPointerPos = true;
        } // or shrinking up
        else if ((GetOutputSizePixel().Height() - aPosPixel.Y()) > nResizeThreshold)
        {
            bResetPointerPos = true;
            pGroupBar->DecrementVerticalSize();
        }

        if (bResetPointerPos)
        {
            aPosPixel.setY(  GetOutputSizePixel().Height() );
            SetPointerPosPixel(aPosPixel);
        }
    }

    ToolBox::MouseMove(rMEvt);
}

void ScInputWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (rMEvt.IsLeft())
    {
        if (IsPointerAtResizePos())
        {
            // Don't leave the mouse pointer leave *this* window
            CaptureMouse();
            bInResize = true;

            // find the height of the gridwin, we don't want to be
            // able to expand the toolbar too far so we need to
            // calculate an upper limit
            // I'd prefer to leave at least a single column header and a
            // row but I don't know how to get that value in pixels.
            // Use TOOLBOX_WINDOW_HEIGHT for the moment
            if (ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell())
            {
                mnMaxY = GetOutputSizePixel().Height() + (pViewSh->GetGridHeight(SC_SPLIT_TOP)
                       + pViewSh->GetGridHeight(SC_SPLIT_BOTTOM)) - TOOLBOX_WINDOW_HEIGHT;
            }
        }
    }

    ToolBox::MouseButtonDown( rMEvt );
}
void ScInputWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    ReleaseMouse();
    if ( rMEvt.IsLeft() )
    {
        bInResize = false;
        mnMaxY = 0;
    }

    ToolBox::MouseButtonUp( rMEvt );
}

void ScInputWindow::AutoSum( bool& bRangeFinder, bool& bSubTotal, OpCode eCode )
{
    ScModule* pScMod = ScModule::get();
    ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>( SfxViewShell::Current()  );
    if ( !pViewSh )
        return;

    const OUString aFormula = pViewSh->DoAutoSum(bRangeFinder, bSubTotal, eCode);
    if ( aFormula.isEmpty() )
        return;

    SetFuncString( aFormula );
    const sal_Int32 aOpen = aFormula.indexOf('(');
    const sal_Int32 aLen  = aFormula.getLength();
    if (!(bRangeFinder && pScMod->IsEditMode()))
        return;

    ScInputHandler* pHdl = pScMod->GetInputHdl( pViewSh );
    if ( !pHdl )
        return;

    pHdl->InitRangeFinder( aFormula );

    //! SetSelection at the InputHandler?
    //! Set bSelIsRef?
    if ( aOpen != -1 && aLen > aOpen )
    {
        ESelection aSel( 0, aOpen + (bSubTotal ? 3 : 1), 0, aLen-1 );
        EditView* pTableView = pHdl->GetTableView();
        if ( pTableView )
            pTableView->SetSelection( aSel );
        EditView* pTopView = pHdl->GetTopView();
        if ( pTopView )
            pTopView->SetSelection( aSel );
    }
}

ScInputBarGroup::ScInputBarGroup(vcl::Window* pParent, ScTabViewShell* pViewSh)
    : InterimItemWindow(pParent, u"modules/scalc/ui/inputbar.ui"_ustr, u"InputBar"_ustr, true, reinterpret_cast<sal_uInt64>(pViewSh))
    , mxBackground(m_xBuilder->weld_container(u"background"_ustr))
    , mxTextWndGroup(new ScTextWndGroup(*this, pViewSh))
    , mxButtonUp(m_xBuilder->weld_button(u"up"_ustr))
    , mxButtonDown(m_xBuilder->weld_button(u"down"_ustr))
{
    InitControlBase(m_xContainer.get());

    SetPaintTransparent(false);
    SetBackgrounds();

    mxButtonUp->connect_clicked(LINK(this, ScInputBarGroup, ClickHdl));
    mxButtonDown->connect_clicked(LINK(this, ScInputBarGroup, ClickHdl));

    if (!comphelper::LibreOfficeKit::isActive())
    {
        mxButtonUp->set_tooltip_text(ScResId( SCSTR_QHELP_COLLAPSE_FORMULA));
        mxButtonDown->set_tooltip_text(ScResId(SCSTR_QHELP_EXPAND_FORMULA));
    }

    int nHeight = mxTextWndGroup->GetPixelHeightForLines(1);
    mxButtonUp->set_size_request(-1, nHeight);
    mxButtonDown->set_size_request(-1, nHeight);

    // disable the multiline toggle on the mobile phones
    const SfxViewShell* pViewShell = SfxViewShell::Current();
    if (!comphelper::LibreOfficeKit::isActive() || !(pViewShell && pViewShell->isLOKMobilePhone()))
        mxButtonDown->show();

    // tdf#154042 Use an initial height of one row so the Toolbar positions
    // this in the same place regardless of how many rows it eventually shows
    Size aSize(GetSizePixel().Width(), nHeight);
    SetSizePixel(aSize);
}

void ScInputBarGroup::SetBackgrounds()
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    SetBackground(rStyleSettings.GetFaceColor());
    // match to bg used in ScTextWnd::SetDrawingArea to the margin area is drawn with the
    // same desired bg
    mxBackground->set_background(rStyleSettings.GetFieldColor());
}

void ScInputBarGroup::DataChanged(const DataChangedEvent& rDCEvt)
{
    InterimItemWindow::DataChanged(rDCEvt);
    if ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
    {
        SetBackgrounds();
        Invalidate();
    }
}

Point ScInputBarGroup::GetCursorScreenPixelPos(bool bBelow)
{
    return mxTextWndGroup->GetCursorScreenPixelPos(bBelow);
}

ScInputBarGroup::~ScInputBarGroup()
{
    disposeOnce();
}

void ScInputBarGroup::dispose()
{
    mxTextWndGroup.reset();
    mxButtonUp.reset();
    mxButtonDown.reset();
    mxBackground.reset();
    InterimItemWindow::dispose();
}

void ScInputBarGroup::InsertAccessibleTextData( ScAccessibleEditLineTextData& rTextData )
{
    mxTextWndGroup->InsertAccessibleTextData(rTextData);
}

void ScInputBarGroup::RemoveAccessibleTextData( ScAccessibleEditLineTextData& rTextData )
{
    mxTextWndGroup->RemoveAccessibleTextData(rTextData);
}

const OUString& ScInputBarGroup::GetTextString() const
{
    return mxTextWndGroup->GetTextString();
}

void ScInputBarGroup::SetTextString(const OUString& rString, bool bKitUpdate)
{
    mxTextWndGroup->SetTextString(rString, bKitUpdate);
}

void ScInputBarGroup::Resize()
{
    mxTextWndGroup->SetScrollPolicy();
    InterimItemWindow::Resize();
}

void ScInputBarGroup::StopEditEngine(bool bAll)
{
    mxTextWndGroup->StopEditEngine(bAll);
}

void ScInputBarGroup::StartEditEngine()
{
    mxTextWndGroup->StartEditEngine();
}

void ScInputBarGroup::MakeDialogEditView()
{
    mxTextWndGroup->MakeDialogEditView();
}

EditView* ScInputBarGroup::GetEditView() const
{
    return mxTextWndGroup->GetEditView();
}

bool ScInputBarGroup::HasEditView() const
{
    return mxTextWndGroup->HasEditView();
}

bool ScInputBarGroup::IsInputActive()
{
    return mxTextWndGroup->IsInputActive();
}

void ScInputBarGroup::SetFormulaMode(bool bSet)
{
    mxTextWndGroup->SetFormulaMode(bSet);
}

void ScInputBarGroup::IncrementVerticalSize()
{
    mxTextWndGroup->SetNumLines(mxTextWndGroup->GetNumLines() + 1);
    TriggerToolboxLayout();
}

void ScInputBarGroup::DecrementVerticalSize()
{
    if (mxTextWndGroup->GetNumLines() > 1)
    {
        mxTextWndGroup->SetNumLines(mxTextWndGroup->GetNumLines() - 1);
        TriggerToolboxLayout();
    }
}

void ScInputWindow::MenuHdl(std::u16string_view command)
{
    if (command.empty())
        return;

    bool bSubTotal = false;
    bool bRangeFinder = false;
    OpCode eCode = ocSum;
    if ( command ==  u"sum" )
    {
        eCode = ocSum;
    }
    else if ( command == u"average" )
    {
        eCode = ocAverage;
    }
    else if ( command == u"max" )
    {
        eCode = ocMax;
    }
    else if ( command == u"min" )
    {
        eCode = ocMin;
    }
    else if ( command == u"count" )
    {
        eCode = ocCount;
    }
    else if ( command == u"counta" )
    {
        eCode = ocCount2;
    }
    else if ( command == u"product" )
    {
        eCode = ocProduct;
    }
    else if (command == u"stdev")
    {
        eCode = ocStDev;
    }
    else if (command == u"stdevp")
    {
        eCode = ocStDevP;
    }
    else if (command == u"var")
    {
        eCode = ocVar;
    }
    else if (command == u"varp")
    {
        eCode = ocVarP;
    }

    AutoSum( bRangeFinder, bSubTotal, eCode );
}

IMPL_LINK_NOARG(ScInputWindow, DropdownClickHdl, ToolBox *, void)
{
    ToolBoxItemId nCurID = GetCurItemId();
    EndSelection();

    if (nCurID == SID_INPUT_SUM)
    {
        tools::Rectangle aRect(GetItemRect(SID_INPUT_SUM));
        weld::Window* pPopupParent = weld::GetPopupParent(*this, aRect);
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(pPopupParent, u"modules/scalc/ui/autosum.ui"_ustr));
        std::unique_ptr<weld::Menu> xPopMenu(xBuilder->weld_menu(u"menu"_ustr));
        MenuHdl(xPopMenu->popup_at_rect(pPopupParent, aRect));
    }
}

IMPL_LINK_NOARG(ScInputBarGroup, ClickHdl, weld::Button&, void)
{
    if (mxTextWndGroup->GetNumLines() > 1)
        mxTextWndGroup->SetNumLines(1);
    else
        mxTextWndGroup->SetNumLines(mxTextWndGroup->GetLastNumExpandedLines());

    NumLinesChanged();
}

void ScInputBarGroup::NumLinesChanged()
{
    if (mxTextWndGroup->GetNumLines() > 1)
    {
        mxButtonDown->hide();
        mxButtonUp->show();
        mxTextWndGroup->SetLastNumExpandedLines(mxTextWndGroup->GetNumLines());
    }
    else
    {
        mxButtonUp->hide();
        mxButtonDown->show();
    }
    TriggerToolboxLayout();

    // Restore focus to input line(s) if necessary
    ScInputHandler* pHdl = ScModule::get()->GetInputHdl();
    if ( pHdl && pHdl->IsTopMode() )
        mxTextWndGroup->TextGrabFocus();
}

void ScInputBarGroup::TriggerToolboxLayout()
{
    // layout changes are expensive and un-necessary.
    if (comphelper::LibreOfficeKit::isActive())
        return;

    vcl::Window *w=GetParent();
    ScInputWindow &rParent = dynamic_cast<ScInputWindow&>(*w);
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();

    if ( !pViewFrm )
        return;

    Reference< css::beans::XPropertySet > xPropSet( pViewFrm->GetFrame().GetFrameInterface(), UNO_QUERY );
    Reference< css::frame::XLayoutManager > xLayoutManager;

    if ( xPropSet.is() )
    {
        css::uno::Any aValue = xPropSet->getPropertyValue(u"LayoutManager"_ustr);
        aValue >>= xLayoutManager;
    }

    if ( !xLayoutManager.is() )
        return;

    xLayoutManager->lock();
    DataChangedEvent aFakeUpdate( DataChangedEventType::SETTINGS, nullptr,  AllSettingsFlags::STYLE );

    // this basically will trigger the repositioning of the
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

void ScInputBarGroup::TextGrabFocus()
{
    mxTextWndGroup->TextGrabFocus();
}

constexpr tools::Long gnBorderWidth = (INPUTLINE_INSET_MARGIN + 1) * 2;
constexpr tools::Long gnBorderHeight = INPUTLINE_INSET_MARGIN + 1;

ScTextWndGroup::ScTextWndGroup(ScInputBarGroup& rParent, ScTabViewShell* pViewSh)
    : mxTextWnd(new ScTextWnd(*this, pViewSh))
    , mxScrollWin(rParent.GetBuilder().weld_scrolled_window(u"scrolledwindow"_ustr, true))
    , mxTextWndWin(new weld::CustomWeld(rParent.GetBuilder(), u"sc_input_window"_ustr, *mxTextWnd))
    , mrParent(rParent)
{
    mxScrollWin->connect_vadjustment_value_changed(LINK(this, ScTextWndGroup, Impl_ScrollHdl));
    if (ScTabViewShell* pActiveViewShell = comphelper::LibreOfficeKit::isActive() ?
            dynamic_cast<ScTabViewShell*>(SfxViewShell::Current()) : nullptr)
    {
        pActiveViewShell->LOKSendFormulabarUpdate(nullptr, u""_ustr, ESelection());
    }
}

Point ScTextWndGroup::GetCursorScreenPixelPos(bool bBelow)
{
    Point aPos;
    if (!HasEditView())
        return aPos;
    EditView* pEditView = GetEditView();
    vcl::Cursor* pCur = pEditView->GetCursor();
    if (!pCur)
        return aPos;
    Point aLogicPos = pCur->GetPos();
    if (bBelow)
        aLogicPos.AdjustY(pCur->GetHeight());
    aPos = GetEditViewDevice().LogicToPixel(aLogicPos);
    bool bRTL = mrParent.IsRTLEnabled();
    if (bRTL)
        aPos.setX(mxTextWnd->GetOutputSizePixel().Width() - aPos.X() + gnBorderWidth);
    else
        aPos.AdjustX(gnBorderWidth + 1);

    return mrParent.OutputToScreenPixel(aPos);
}

ScTextWndGroup::~ScTextWndGroup()
{
}

void ScTextWndGroup::InsertAccessibleTextData(ScAccessibleEditLineTextData& rTextData)
{
    mxTextWnd->InsertAccessibleTextData(rTextData);
}

EditView* ScTextWndGroup::GetEditView() const
{
    return mxTextWnd->GetEditView();
}

const OutputDevice& ScTextWndGroup::GetEditViewDevice() const
{
    return mxTextWnd->GetEditViewDevice();
}

tools::Long ScTextWndGroup::GetLastNumExpandedLines() const
{
    return mxTextWnd->GetLastNumExpandedLines();
}

void ScTextWndGroup::SetLastNumExpandedLines(tools::Long nLastExpandedLines)
{
    mxTextWnd->SetLastNumExpandedLines(nLastExpandedLines);
}

tools::Long ScTextWndGroup::GetNumLines() const
{
    return mxTextWnd->GetNumLines();
}

int ScTextWndGroup::GetPixelHeightForLines(tools::Long nLines)
{
    return mxTextWnd->GetPixelHeightForLines(nLines) + 2 * gnBorderHeight;
}

weld::ScrolledWindow& ScTextWndGroup::GetScrollWin()
{
    return *mxScrollWin;
}

const OUString& ScTextWndGroup::GetTextString() const
{
    return mxTextWnd->GetTextString();
}

bool ScTextWndGroup::HasEditView() const
{
    return mxTextWnd->HasEditView();
}

bool ScTextWndGroup::IsInputActive()
{
    return mxTextWnd->IsInputActive();
}

void ScTextWndGroup::MakeDialogEditView()
{
    mxTextWnd->MakeDialogEditView();
}

void ScTextWndGroup::RemoveAccessibleTextData(ScAccessibleEditLineTextData& rTextData)
{
    mxTextWnd->RemoveAccessibleTextData(rTextData);
}

void ScTextWndGroup::SetScrollPolicy()
{
    if (mxTextWnd->GetNumLines() > 2)
        mxScrollWin->set_vpolicy(VclPolicyType::ALWAYS);
    else
        mxScrollWin->set_vpolicy(VclPolicyType::NEVER);
}

void ScTextWndGroup::SetNumLines(tools::Long nLines)
{
    mxTextWnd->SetNumLines(nLines);
}

void ScTextWndGroup::SetFormulaMode(bool bSet)
{
    mxTextWnd->SetFormulaMode(bSet);
}

void ScTextWndGroup::SetTextString(const OUString& rString, bool bKitUpdate)
{
    mxTextWnd->SetTextString(rString, bKitUpdate);
}

void ScTextWndGroup::StartEditEngine()
{
    mxTextWnd->StartEditEngine();
}

void ScTextWndGroup::StopEditEngine(bool bAll)
{
    mxTextWnd->StopEditEngine( bAll );
}

void ScTextWndGroup::TextGrabFocus()
{
    mxTextWnd->TextGrabFocus();
}

IMPL_LINK_NOARG(ScTextWndGroup, Impl_ScrollHdl, weld::ScrolledWindow&, void)
{
    mxTextWnd->DoScroll();
}

void ScTextWnd::Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect )
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetFieldColor();
    rRenderContext.SetBackground(aBgColor);

    // tdf#137713 we rely on GetEditView creating it if it doesn't already exist so
    // GetEditView() must be called unconditionally
    if (EditView* pView = GetEditView())
    {
        if (mbInvalidate)
        {
            pView->Invalidate();
            mbInvalidate = false;
        }
    }

    if (comphelper::LibreOfficeKit::isActive() && m_xEditEngine)
    {
        // EditEngine/EditView works in twips logical coordinates, so set the device map-mode to twips before painting
        // and use twips version of the painting area 'rRect'.
        // Document zoom should not be included in this conversion.
        tools::Rectangle aLogicRect = OutputDevice::LogicToLogic(rRect, MapMode(MapUnit::MapPixel), MapMode(MapUnit::MapTwip));
        MapMode aOriginalMode = rRenderContext.GetMapMode();
        rRenderContext.SetMapMode(MapMode(MapUnit::MapTwip));
        WeldEditView::Paint(rRenderContext, aLogicRect);
        rRenderContext.SetMapMode(aOriginalMode);
    }
    else
        WeldEditView::Paint(rRenderContext, rRect);
}

EditView* ScTextWnd::GetEditView() const
{
    if ( !m_xEditView )
        const_cast<ScTextWnd&>(*this).InitEditEngine();
    return m_xEditView.get();
}

bool ScTextWnd::HasEditView() const { return m_xEditView != nullptr; }

const OutputDevice& ScTextWnd::GetEditViewDevice() const
{
    return EditViewOutputDevice();
}

int ScTextWnd::GetPixelHeightForLines(tools::Long nLines)
{
    OutputDevice& rDevice = GetDrawingArea()->get_ref_device();
    return rDevice.LogicToPixel(Size(0, nLines * rDevice.GetTextHeight())).Height() + 1;
}

tools::Long ScTextWnd::GetNumLines() const
{
    ScViewData& rViewData = mpViewShell->GetViewData();
    return rViewData.GetFormulaBarLines();
}

void ScTextWnd::SetNumLines(tools::Long nLines)
{
    ScViewData& rViewData = mpViewShell->GetViewData();
    rViewData.SetFormulaBarLines(nLines);
    if ( nLines > 1 )
    {
        // SetFormulaBarLines sanitizes the height, so get the sanitized value
        mnLastExpandedLines = rViewData.GetFormulaBarLines();
        Resize();
    }
}

void ScTextWnd::Resize()
{
    if (m_xEditView)
    {
        Size aOutputSize = GetOutputSizePixel();
        OutputDevice& rDevice = GetDrawingArea()->get_ref_device();
        tools::Rectangle aOutputArea = rDevice.PixelToLogic( tools::Rectangle( Point(), aOutputSize ));
        m_xEditView->SetOutputArea( aOutputArea );

        // Don't leave an empty area at the bottom if we can move the text down.
        tools::Long nMaxVisAreaTop = m_xEditEngine->GetTextHeight() - aOutputArea.GetHeight();
        if (m_xEditView->GetVisArea().Top() > nMaxVisAreaTop)
        {
            m_xEditView->Scroll(0, m_xEditView->GetVisArea().Top() - nMaxVisAreaTop);
        }

        m_xEditEngine->SetPaperSize( rDevice.PixelToLogic( Size( aOutputSize.Width(), 10000 ) ) );
    }

    // skip WeldEditView's Resize();
    weld::CustomWidgetController::Resize();

    SetScrollBarRange();
}

int ScTextWnd::GetEditEngTxtHeight() const
{
    return m_xEditView ? m_xEditView->getEditEngine().GetTextHeight() : 0;
}

void ScTextWnd::SetScrollBarRange()
{
    if (!m_xEditView)
        return;

    OutputDevice& rDevice = GetDrawingArea()->get_ref_device();
    Size aOutputSize = rDevice.GetOutputSize();

    int nUpper = GetEditEngTxtHeight();
    int nCurrentDocPos = m_xEditView->GetVisArea().Top();
    int nStepIncrement = GetTextHeight();
    int nPageIncrement = aOutputSize.Height();
    int nPageSize = aOutputSize.Height();

    /* limit the page size to below nUpper because gtk's gtk_scrolled_window_start_deceleration has
       effectively...

       lower = gtk_adjustment_get_lower
       upper = gtk_adjustment_get_upper - gtk_adjustment_get_page_size

       and requires that upper > lower or the deceleration animation never ends
    */
    nPageSize = std::min(nPageSize, nUpper);

    weld::ScrolledWindow& rVBar = mrGroupBar.GetScrollWin();
    rVBar.vadjustment_configure(nCurrentDocPos, nUpper, nStepIncrement, nPageIncrement, nPageSize);
}

void ScTextWnd::DoScroll()
{
    if (m_xEditView)
    {
        weld::ScrolledWindow& rVBar = mrGroupBar.GetScrollWin();
        auto currentDocPos = m_xEditView->GetVisArea().Top();
        auto nDiff = currentDocPos - rVBar.vadjustment_get_value();
        // we expect SetScrollBarRange callback to be triggered by Scroll
        // to set where we ended up
        m_xEditView->Scroll(0, nDiff);
    }
}

void ScTextWnd::StartEditEngine()
{
    // Don't activate if we're a modal dialog ourselves (Doc-modal dialog)
    SfxObjectShell* pObjSh = SfxObjectShell::Current();
    if ( pObjSh && pObjSh->IsInModalMode() )
        return;

    if ( !m_xEditView || !m_xEditEngine )
    {
        InitEditEngine();
    }

    ScInputHandler* pHdl = mpViewShell->GetInputHandler();
    if (pHdl)
        pHdl->SetMode(SC_INPUT_TOP, nullptr, static_cast<ScEditEngineDefaulter*>(m_xEditEngine.get()));

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (pViewFrm)
        pViewFrm->GetBindings().Invalidate( SID_ATTR_INSERT );
}

static void lcl_ExtendEditFontAttribs( SfxItemSet& rSet )
{
    const SfxPoolItem& rFontItem = rSet.Get( EE_CHAR_FONTINFO );
    std::unique_ptr<SfxPoolItem> pNewItem(rFontItem.Clone());
    pNewItem->SetWhich(EE_CHAR_FONTINFO_CJK);
    rSet.Put( *pNewItem );
    pNewItem->SetWhich(EE_CHAR_FONTINFO_CTL);
    rSet.Put( *pNewItem );
    const SfxPoolItem& rHeightItem = rSet.Get( EE_CHAR_FONTHEIGHT );
    pNewItem.reset(rHeightItem.Clone());
    pNewItem->SetWhich(EE_CHAR_FONTHEIGHT_CJK);
    rSet.Put( *pNewItem );
    pNewItem->SetWhich(EE_CHAR_FONTHEIGHT_CTL);
    rSet.Put( *pNewItem );
    const SfxPoolItem& rWeightItem = rSet.Get( EE_CHAR_WEIGHT );
    pNewItem.reset(rWeightItem.Clone());
    pNewItem->SetWhich(EE_CHAR_WEIGHT_CJK);
    rSet.Put( *pNewItem );
    pNewItem->SetWhich(EE_CHAR_WEIGHT_CTL);
    rSet.Put( *pNewItem );
    const SfxPoolItem& rItalicItem = rSet.Get( EE_CHAR_ITALIC );
    pNewItem.reset(rItalicItem.Clone());
    pNewItem->SetWhich(EE_CHAR_ITALIC_CJK);
    rSet.Put( *pNewItem );
    pNewItem->SetWhich(EE_CHAR_ITALIC_CTL);
    rSet.Put( *pNewItem );
    const SfxPoolItem& rLangItem = rSet.Get( EE_CHAR_LANGUAGE );
    pNewItem.reset(rLangItem.Clone());
    pNewItem->SetWhich(EE_CHAR_LANGUAGE_CJK);
    rSet.Put( *pNewItem );
    pNewItem->SetWhich(EE_CHAR_LANGUAGE_CTL);
    rSet.Put( *pNewItem );
}

static void lcl_ModifyRTLDefaults( SfxItemSet& rSet )
{
    rSet.Put( SvxAdjustItem( SvxAdjust::Right, EE_PARA_JUST ) );

    // always using rtl writing direction would break formulas
    //rSet.Put( SvxFrameDirectionItem( SvxFrameDirection::Horizontal_RL_TB, EE_PARA_WRITINGDIR ) );

    // PaperSize width is limited to USHRT_MAX in RTL mode (because of EditEngine's
    // sal_uInt16 values in EditLine), so the text may be wrapped and line spacing must be
    // increased to not see the beginning of the next line.
    SvxLineSpacingItem aItem( LINE_SPACE_DEFAULT_HEIGHT, EE_PARA_SBL );
    aItem.SetPropLineSpace( 200 );
    rSet.Put( aItem );
}

static void lcl_ModifyRTLVisArea( EditView* pEditView )
{
    tools::Rectangle aVisArea = pEditView->GetVisArea();
    Size aPaper = pEditView->getEditEngine().GetPaperSize();
    tools::Long nDiff = aPaper.Width() - aVisArea.Right();
    aVisArea.AdjustLeft(nDiff );
    aVisArea.AdjustRight(nDiff );
    pEditView->SetVisArea(aVisArea);
}

void ScTextWnd::InitEditEngine()
{
    std::unique_ptr<ScFieldEditEngine> pNew;
    ScDocShell* pDocSh = nullptr;
    if ( mpViewShell )
    {
        pDocSh = &mpViewShell->GetViewData().GetDocShell();
        ScDocument& rDoc = mpViewShell->GetViewData().GetDocument();
        pNew = std::make_unique<ScFieldEditEngine>(&rDoc, rDoc.GetEditEnginePool());
    }
    else
        pNew = std::make_unique<ScFieldEditEngine>(nullptr, EditEngine::CreatePool().get(), true);
    pNew->SetExecuteURL( false );
    m_xEditEngine = std::move(pNew);

    Size barSize = GetOutputSizePixel();
    m_xEditEngine->SetUpdateLayout( false );
    m_xEditEngine->SetPaperSize( GetDrawingArea()->get_ref_device().PixelToLogic(Size(barSize.Width(),10000)) );
    m_xEditEngine->SetWordDelimiters(
                    ScEditUtil::ModifyDelimiters( m_xEditEngine->GetWordDelimiters() ) );
    m_xEditEngine->SetReplaceLeadingSingleQuotationMark( false );

    UpdateAutoCorrFlag();

    {
        SfxItemSet aSet( m_xEditEngine->GetEmptyItemSet() );
        EditEngine::SetFontInfoInItemSet( aSet, aTextFont );
        lcl_ExtendEditFontAttribs( aSet );
        // turn off script spacing to match DrawText output
        aSet.Put( SvxScriptSpaceItem( false, EE_PARA_ASIANCJKSPACING ) );
        if ( bIsRTL )
            lcl_ModifyRTLDefaults( aSet );
        static_cast<ScEditEngineDefaulter*>(m_xEditEngine.get())->SetDefaults( std::move(aSet) );
    }

    // If the Cell contains URLFields, they need to be taken over into the entry row,
    // or else the position is not correct anymore
    bool bFilled = false;
    ScInputHandler* pHdl = ScModule::get()->GetInputHdl();
    if ( pHdl ) //! Test if it's the right InputHdl?
        bFilled = pHdl->GetTextAndFields(static_cast<ScEditEngineDefaulter&>(*m_xEditEngine));

    m_xEditEngine->SetUpdateLayout( true );

    // aString is the truth ...
    if (bFilled && m_xEditEngine->GetText() == aString)
        Invalidate(); // Repaint for (filled) Field
    else
        static_cast<ScEditEngineDefaulter*>(m_xEditEngine.get())->SetTextCurrentDefaults(aString); // At least the right text then

    m_xEditView = std::make_unique<EditView>(*m_xEditEngine, nullptr);

    // we get cursor, selection etc. messages from the VCL/window layer
    // otherwise these are injected into the document causing confusion.
    m_xEditView->SuppressLOKMessages(true);

    m_xEditView->setEditViewCallbacks(this);
    m_xEditView->SetInsertMode(bIsInsertMode);

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetFieldColor();
    m_xEditView->SetBackgroundColor(aBgColor);

    if (pAcc)
    {
        pAcc->InitAcc(nullptr, m_xEditView.get(),
                      ScResId(STR_ACC_EDITLINE_NAME),
                      ScResId(STR_ACC_EDITLINE_DESCR));
    }

    if (comphelper::LibreOfficeKit::isActive())
        m_xEditView->RegisterViewShell(mpViewShell);

    // Text from Clipboard is taken over as ASCII in a single row
    EVControlBits n = m_xEditView->GetControlWord();
    m_xEditView->SetControlWord( n | EVControlBits::SINGLELINEPASTE );

    m_xEditEngine->InsertView( m_xEditView.get(), EE_APPEND );

    Resize();

    if ( bIsRTL )
        lcl_ModifyRTLVisArea( m_xEditView.get() );

    m_xEditEngine->SetModifyHdl(LINK(this, ScTextWnd, ModifyHdl));
    m_xEditEngine->SetStatusEventHdl(LINK(this, ScTextWnd, EditStatusHdl));

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

ScTextWnd::ScTextWnd(ScTextWndGroup& rParent, ScTabViewShell* pViewSh) :
        bIsRTL(AllSettings::GetLayoutRTL()),
        bIsInsertMode(true),
        bFormulaMode (false),
        bInputMode   (false),
        mpViewShell(pViewSh),
        mrGroupBar(rParent),
        mnLastExpandedLines(INPUTWIN_MULTILINES),
        mbInvalidate(false)
{
}

ScTextWnd::~ScTextWnd()
{
    while (!maAccTextDatas.empty()) {
        maAccTextDatas.back()->Dispose();
    }
}

bool ScTextWnd::MouseMove( const MouseEvent& rMEvt )
{
    return m_xEditView && m_xEditView->MouseMove(rMEvt);
}

bool ScTextWnd::CanFocus() const
{
    return ScModule::get()->IsEditMode();
}

void ScTextWnd::UpdateFocus()
{
    if (!HasFocus())
    {
        StartEditEngine();
        if (CanFocus())
            TextGrabFocus();
    }
}

bool ScTextWnd::MouseButtonDown( const MouseEvent& rMEvt )
{
    UpdateFocus();

    bool bClickOnSelection = false;
    if (m_xEditView)
    {
        m_xEditView->SetEditEngineUpdateLayout( true );
        bClickOnSelection = m_xEditView->IsSelectionAtPoint(rMEvt.GetPosPixel());
    }
    if (!bClickOnSelection)
    {
        rtl::Reference<TransferDataContainer> xTransferable(new TransferDataContainer);
        GetDrawingArea()->enable_drag_source(xTransferable, DND_ACTION_NONE);
    }
    else
    {
        rtl::Reference<TransferDataContainer> xTransferable(m_xHelper);
        GetDrawingArea()->enable_drag_source(xTransferable, DND_ACTION_COPY);
    }
    return WeldEditView::MouseButtonDown(rMEvt);
}

bool ScTextWnd::MouseButtonUp( const MouseEvent& rMEvt )
{
    bool bRet = WeldEditView::MouseButtonUp(rMEvt);
    if (bRet)
    {
        if ( rMEvt.IsMiddle() &&
                 Application::GetSettings().GetMouseSettings().GetMiddleButtonAction() == MouseMiddleButtonAction::PasteSelection )
        {
            //  EditView may have pasted from selection
            ScModule::get()->InputChanged(m_xEditView.get());
        }
        else
            ScModule::get()->InputSelection(m_xEditView.get());
    }
    return bRet;
}

bool ScTextWnd::Command( const CommandEvent& rCEvt )
{
    bool bConsumed = false;

    bInputMode = true;
    CommandEventId nCommand = rCEvt.GetCommand();
    if (m_xEditView)
    {
        ScModule* pScMod = ScModule::get();
        ScTabViewShell* pStartViewSh = ScTabViewShell::GetActiveViewShell();

        // don't modify the font defaults here - the right defaults are
        // already set in StartEditEngine when the EditEngine is created

        // Prevent that the EditView is lost when switching between Views
        pScMod->SetInEditCommand( true );
        m_xEditView->Command( rCEvt );
        pScMod->SetInEditCommand( false );

        //  CommandEventId::StartDrag does not mean by far that the content was actually changed,
        //  so don't trigger an InputChanged.
        //! Detect if dragged with Move or forbid Drag&Move somehow

        if ( nCommand == CommandEventId::StartDrag )
        {
            // Is dragged onto another View?
            ScTabViewShell* pEndViewSh = ScTabViewShell::GetActiveViewShell();
            if ( pEndViewSh != pStartViewSh && pStartViewSh != nullptr )
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
        else if ( nCommand == CommandEventId::EndExtTextInput )
        {
            ScModule* mod = ScModule::get();
            if (bFormulaMode)
            {
                ScInputHandler* pHdl = mod->GetInputHdl();
                if (pHdl)
                    pHdl->InputCommand(rCEvt);
            }
            mod->InputChanged(m_xEditView.get());
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
        else if ( nCommand == CommandEventId::ContextMenu )
        {
            bConsumed = true;
            SfxViewFrame* pViewFrm = SfxViewFrame::Current();
            if (pViewFrm)
            {
                Point aPos = rCEvt.GetMousePosPixel();
                if (!rCEvt.IsMouseEvent())
                {
                    Size aSize = GetOutputSizePixel();
                    aPos = Point(aSize.Width() / 2, aSize.Height() / 2);
                }
                if (IsMouseCaptured())
                    ReleaseMouse();
                UpdateFocus();
                pViewFrm->GetDispatcher()->ExecutePopup(u"formulabar"_ustr, &mrGroupBar.GetVclParent(), &aPos);
            }
        }
        else if ( nCommand == CommandEventId::Wheel )
        {
            //don't call InputChanged for CommandEventId::Wheel
        }
        else if ( nCommand == CommandEventId::GestureSwipe )
        {
            //don't call InputChanged for CommandEventId::GestureSwipe
        }
        else if ( nCommand == CommandEventId::GestureLongPress )
        {
            //don't call InputChanged for CommandEventId::GestureLongPress
        }
        else if ( nCommand == CommandEventId::ModKeyChange )
        {
            //pass alt press/release to parent impl
        }
        else
            ScModule::get()->InputChanged(m_xEditView.get());
    }

    if ( comphelper::LibreOfficeKit::isActive() && nCommand == CommandEventId::CursorPos )
    {
        // LOK uses this to setup caret position because drawingarea is replaced
        // with text input field, it sends logical caret position (start, end) not pixels

        StartEditEngine();
        TextGrabFocus();

        if (!m_xEditView)
            return true;

        ScModule* mod = ScModule::get();
        // if we focus input after "Accept Formula" command, we need to notify to get it working
        mod->InputChanged(m_xEditView.get());

        // information about paragraph is in additional data
        // information about position in a paragraph in a Mouse Pos
        // see vcl/jsdialog/executor.cxx "textselection" event
        const Point* pParaPoint = static_cast<const Point*>(rCEvt.GetEventData());
        Point aSelectionStartEnd = rCEvt.GetMousePosPixel();

        sal_Int32 nParaStart, nParaEnd, nPosStart, nPosEnd;

        nParaStart = pParaPoint ? pParaPoint->X() : 0;
        nParaEnd = pParaPoint ? pParaPoint->Y() : 0;
        nPosStart = m_xEditView->GetPosNoField(nParaStart, aSelectionStartEnd.X());
        nPosEnd = m_xEditView->GetPosNoField(nParaEnd, aSelectionStartEnd.Y());

        m_xEditView->SetSelection(ESelection(nParaStart, nPosStart, nParaEnd, nPosEnd));
        mod->InputSelection(m_xEditView.get());

        bConsumed = true;
    }

    bInputMode = false;

    return bConsumed;
}

bool ScTextWnd::StartDrag()
{
    // tdf#145248 don't start a drag if actively selecting
    if (m_xEditView && !m_xEditEngine->IsInSelectionMode())
    {
        OUString sSelection = m_xEditView->GetSelected();
        m_xHelper->SetData(sSelection);
        return sSelection.isEmpty();
    }
    return true;
}

bool ScTextWnd::KeyInput(const KeyEvent& rKEvt)
{
    bool bUsed = true;
    bInputMode = true;
    if (!ScModule::get()->InputKeyEvent(rKEvt))
    {
        bUsed = false;
        ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
        if ( pViewSh )
            bUsed = pViewSh->SfxKeyInput(rKEvt); // Only accelerators, no input
    }
    bInputMode = false;
    return bUsed;
}

void ScTextWnd::GetFocus()
{
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if ( pViewSh )
        pViewSh->SetFormShellAtTop( false ); // focus in input line -> FormShell no longer on top
    WeldEditView::GetFocus();
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
    if (m_xEditEngine)
    {
        EEControlBits nControl = m_xEditEngine->GetControlWord();
        EEControlBits nOld = nControl;
        if ( bFormulaMode )
            nControl &= ~EEControlBits::AUTOCORRECT; // No AutoCorrect in Formulas
        else
            nControl |= EEControlBits::AUTOCORRECT; // Else do enable it

        if ( nControl != nOld )
            m_xEditEngine->SetControlWord( nControl );
    }
}

void ScTextWnd::EditViewScrollStateChange()
{
    // editengine height has changed or editview scroll pos has changed
    SetScrollBarRange();
}

IMPL_LINK_NOARG(ScTextWnd, ModifyHdl, LinkParamNone*, void)
{
    if (m_xEditView && !bInputMode)
    {
        ScInputHandler* pHdl = ScModule::get()->GetInputHdl();

        //  Use the InputHandler's InOwnChange flag to prevent calling InputChanged
        //  while an InputHandler method is modifying the EditEngine content

        if ( pHdl && !pHdl->IsInOwnChange() )
            pHdl->InputChanged( m_xEditView.get(), true );  // #i20282# InputChanged must know if called from modify handler
    }
}

IMPL_LINK_NOARG(ScTextWnd, EditStatusHdl, EditStatus&, void)
{
    SetScrollBarRange();
    DoScroll();
    Invalidate();
}

void ScTextWnd::StopEditEngine( bool bAll )
{
    if (!m_xEditEngine)
        return;

    if (m_xEditView)
    {
        if (!maAccTextDatas.empty())
            maAccTextDatas.back()->EndEdit();

        ScModule* pScMod = ScModule::get();

        if (!bAll)
            pScMod->InputSelection( m_xEditView.get() );
        aString = m_xEditEngine->GetText();
        bIsInsertMode = m_xEditView->IsInsertMode();
        bool bSelection = m_xEditView->HasSelection();
        m_xEditEngine->SetStatusEventHdl(Link<EditStatus&, void>());
        m_xEditEngine->SetModifyHdl(Link<LinkParamNone*,void>());
        m_xEditView.reset();
        m_xEditEngine.reset();

        ScInputHandler* pHdl = mpViewShell->GetInputHandler();

        if (pHdl && pHdl->IsEditMode() && !bAll)
            pHdl->SetMode(SC_INPUT_TABLE);

        SfxViewFrame* pViewFrm = SfxViewFrame::Current();
        if (pViewFrm)
            pViewFrm->GetBindings().Invalidate( SID_ATTR_INSERT );

        if (bSelection)
            Invalidate(); // So that the Selection is not left there
    }

    if (comphelper::LibreOfficeKit::isActive())
    {
        // Clear
        std::vector<ReferenceMark> aReferenceMarks;
        ScInputHandler::SendReferenceMarks( mpViewShell, aReferenceMarks );
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
        ++pStr1;
        ++pStr2;
        ++i;
    }

    return i;
}

void ScTextWnd::SetTextString( const OUString& rNewString, bool bKitUpdate )
{
    // Ideally it would be best to create on demand the EditEngine/EditView here, but... for
    // the initialisation scenario where a cell is first clicked on we end up with the text in the
    // inputbar window scrolled to the bottom if we do that here ( because the tableview and topview
    // are synced I guess ).
    // should fix that I suppose :-/ need to look a bit further into that
    mbInvalidate = true; // ensure next Paint ( that uses editengine ) call will call Invalidate first

    if ( rNewString != aString )
    {
        bInputMode = true;

        // Find position of the change, only paint the rest
        if (!m_xEditEngine)
        {
            bool bPaintAll = GetNumLines() > 1 || bIsRTL;
            if (!bPaintAll)
            {
                //  test if CTL script type is involved
                SvtScriptType nOldScript = SvtScriptType::NONE;
                SvtScriptType nNewScript = SvtScriptType::NONE;
                SfxObjectShell* pObjSh = SfxObjectShell::Current();
                if ( auto pDocShell = dynamic_cast<ScDocShell*>( pObjSh) )
                {
                    //  any document can be used (used only for its break iterator)
                    ScDocument& rDoc = pDocShell->GetDocument();
                    nOldScript = rDoc.GetStringScriptType( aString );
                    nNewScript = rDoc.GetStringScriptType( rNewString );
                }
                bPaintAll = ( nOldScript & SvtScriptType::COMPLEX ) || ( nNewScript & SvtScriptType::COMPLEX );
            }

            if ( bPaintAll )
            {
                // In multiline mode, or if CTL is involved, the whole text has to be redrawn
                Invalidate();
            }
            else
            {
                tools::Long nTextSize = 0;
                sal_Int32 nDifPos;
                if (rNewString.getLength() > aString.getLength())
                    nDifPos = findFirstNonMatchingChar(rNewString, aString);
                else
                    nDifPos = findFirstNonMatchingChar(aString, rNewString);

                tools::Long nSize1 = GetTextWidth(aString);
                tools::Long nSize2 = GetTextWidth(rNewString);
                if ( nSize1>0 && nSize2>0 )
                    nTextSize = std::max( nSize1, nSize2 );
                else
                    nTextSize = GetOutputSizePixel().Width(); // Overflow

                Point aLogicStart = GetDrawingArea()->get_ref_device().PixelToLogic(Point(0,0));
                tools::Long nStartPos = aLogicStart.X();
                tools::Long nInvPos = nStartPos;
                if (nDifPos)
                    nInvPos += GetTextWidth(aString.copy(0,nDifPos));

                Invalidate(tools::Rectangle(nInvPos, 0, nStartPos+nTextSize, GetOutputSizePixel().Height() - 1));
            }
        }
        else
        {
            static_cast<ScEditEngineDefaulter*>(m_xEditEngine.get())->SetTextCurrentDefaults(rNewString);
        }

        aString = rNewString;

        if (!maAccTextDatas.empty())
            maAccTextDatas.back()->TextChanged();

        bInputMode = false;
    }

    if (ScTabViewShell* pActiveViewShell = bKitUpdate && comphelper::LibreOfficeKit::isActive() ?
            dynamic_cast<ScTabViewShell*>(SfxViewShell::Current()) : nullptr)
    {
        ESelection aSel = m_xEditView ? m_xEditView->GetSelection() : ESelection();
        pActiveViewShell->LOKSendFormulabarUpdate(m_xEditView.get(), rNewString, aSel);
    }

    SetScrollBarRange();
    DoScroll();
}

const OUString& ScTextWnd::GetTextString() const
{
    return aString;
}

bool ScTextWnd::IsInputActive()
{
    return HasFocus();
}

void ScTextWnd::MakeDialogEditView()
{
    if ( m_xEditView ) return;

    std::unique_ptr<ScFieldEditEngine> pNew;
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if ( pViewSh )
    {
        ScDocument& rDoc = pViewSh->GetViewData().GetDocument();
        pNew = std::make_unique<ScFieldEditEngine>(&rDoc, rDoc.GetEditEnginePool());
    }
    else
        pNew = std::make_unique<ScFieldEditEngine>(nullptr, EditEngine::CreatePool().get(), true);
    pNew->SetExecuteURL( false );
    m_xEditEngine = std::move(pNew);

    const bool bPrevUpdateLayout = m_xEditEngine->SetUpdateLayout( false );
    m_xEditEngine->SetWordDelimiters( m_xEditEngine->GetWordDelimiters() + "=" );
    m_xEditEngine->SetPaperSize( Size( bIsRTL ? USHRT_MAX : THESIZE, 300 ) );

    SfxItemSet aSet( m_xEditEngine->GetEmptyItemSet() );
    EditEngine::SetFontInfoInItemSet( aSet, aTextFont );
    lcl_ExtendEditFontAttribs( aSet );
    if ( bIsRTL )
        lcl_ModifyRTLDefaults( aSet );
    static_cast<ScEditEngineDefaulter*>(m_xEditEngine.get())->SetDefaults( std::move(aSet) );
    m_xEditEngine->SetUpdateLayout( bPrevUpdateLayout );

    m_xEditView = std::make_unique<EditView>(*m_xEditEngine, nullptr);
    m_xEditView->setEditViewCallbacks(this);

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetFieldColor();
    m_xEditView->SetBackgroundColor(aBgColor);

    if (pAcc)
    {
        pAcc->InitAcc(nullptr, m_xEditView.get(),
                      ScResId(STR_ACC_EDITLINE_NAME),
                      ScResId(STR_ACC_EDITLINE_DESCR));
    }

    if (comphelper::LibreOfficeKit::isActive())
        m_xEditView->RegisterViewShell(mpViewShell);
    m_xEditEngine->InsertView( m_xEditView.get(), EE_APPEND );

    Resize();

    if ( bIsRTL )
        lcl_ModifyRTLVisArea( m_xEditView.get() );

    if (!maAccTextDatas.empty())
        maAccTextDatas.back()->StartEdit();
}

void ScTextWnd::ImplInitSettings()
{
    bIsRTL = AllSettings::GetLayoutRTL();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    Color aBgColor= rStyleSettings.GetFieldColor();
    Color aTxtColor= rStyleSettings.GetWindowTextColor();

    aTextFont.SetFillColor   ( aBgColor );
    aTextFont.SetColor       (aTxtColor);
    Invalidate();
}

void ScTextWnd::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    // bypass WeldEditView::SetDrawingArea
    weld::CustomWidgetController::SetDrawingArea(pDrawingArea);

    // set cursor
    pDrawingArea->set_cursor(PointerStyle::Text);

    // initialize dnd, deliberately just a simple string so
    // we don't transfer the happenstance formatting in
    // the input line
    m_xHelper.set(new svt::OStringTransferable(OUString()));
    rtl::Reference<TransferDataContainer> xHelper(m_xHelper);
    SetDragDataTransferable(xHelper, DND_ACTION_COPY);

    OutputDevice& rDevice = pDrawingArea->get_ref_device();
    pDrawingArea->set_margin_start(gnBorderWidth);
    pDrawingArea->set_margin_end(gnBorderWidth);
    // leave 1 for the width of the scrolledwindow border
    pDrawingArea->set_margin_top(gnBorderHeight - 1);
    pDrawingArea->set_margin_bottom(gnBorderHeight - 1);

    // always use application font, so a font with cjk chars can be installed
    vcl::Font aAppFont = Application::GetSettings().GetStyleSettings().GetAppFont();
    weld::SetPointFont(rDevice, aAppFont);

    aTextFont = rDevice.GetFont();
    Size aFontSize = aTextFont.GetFontSize();
    aTextFont.SetFontSize(rDevice.PixelToLogic(aFontSize, MapMode(MapUnit::MapTwip)));

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    Color aBgColor = rStyleSettings.GetFieldColor();
    Color aTxtColor = rStyleSettings.GetWindowTextColor();

    aTextFont.SetTransparent(true);
    aTextFont.SetFillColor(aBgColor);
    aTextFont.SetColor(aTxtColor);
    aTextFont.SetWeight(WEIGHT_NORMAL);

    Size aSize(1, GetPixelHeightForLines(1));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());

    rDevice.SetBackground(aBgColor);
    rDevice.SetLineColor(COL_BLACK);
    rDevice.SetMapMode(MapMode(MapUnit::MapTwip));
    rDevice.SetFont(aTextFont);

    EnableRTL(false); // EditEngine can't be used with VCL EnableRTL
}

rtl::Reference<comphelper::OAccessible> ScTextWnd::CreateAccessible()
{
    pAcc = new ScAccessibleEditLineObject(this);
    return pAcc;
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

void ScTextWnd::StyleUpdated()
{
    ImplInitSettings();
    CustomWidgetController::Invalidate();
}

void ScTextWnd::TextGrabFocus()
{
    GrabFocus();
}

// Position window
ScPosWnd::ScPosWnd(vcl::Window* pParent, ScTabViewShell* pViewShell)
    : InterimItemWindow(pParent, u"modules/scalc/ui/posbox.ui"_ustr, u"PosBox"_ustr, true,
                        reinterpret_cast<sal_uInt64>(pViewShell))
    , m_xWidget(m_xBuilder->weld_combo_box(u"pos_window"_ustr))
    , m_nAsyncGetFocusId(nullptr)
    , nTipVisible(nullptr)
    , bFormulaMode(false)
{
    InitControlBase(m_xWidget.get());

    // Use calculation according to tdf#132338 to align combobox width to width of fontname combobox within formatting toolbar;
    // formatting toolbar is placed above formulabar when using multiple toolbars typically

    m_xWidget->set_entry_width_chars(1);
    Size aSize(LogicToPixel(Size(POSITION_COMBOBOX_WIDTH * 4, 0), MapMode(MapUnit::MapAppFont)));
    m_xWidget->set_size_request(aSize.Width(), -1);
    SetSizePixel(m_xContainer->get_preferred_size());

    FillRangeNames(true);

    StartListening( *SfxGetpApp() ); // For Navigator rangename updates

    m_xWidget->connect_key_press(LINK(this, ScPosWnd, KeyInputHdl));
    m_xWidget->connect_entry_activate(LINK(this, ScPosWnd, ActivateHdl));
    m_xWidget->connect_changed(LINK(this, ScPosWnd, ModifyHdl));
    m_xWidget->connect_focus_in(LINK(this, ScPosWnd, FocusInHdl));
    m_xWidget->connect_focus_out(LINK(this, ScPosWnd, FocusOutHdl));
}

ScPosWnd::~ScPosWnd()
{
    disposeOnce();
}

void ScPosWnd::dispose()
{
    EndListening( *SfxGetpApp() );

    HideTip();

    if (m_nAsyncGetFocusId)
    {
        Application::RemoveUserEvent(m_nAsyncGetFocusId);
        m_nAsyncGetFocusId = nullptr;
    }
    m_xWidget.reset();

    InterimItemWindow::dispose();
}

void ScPosWnd::SetFormulaMode( bool bSet )
{
    if ( bSet != bFormulaMode )
    {

        if ( bSet )
            FillFunctions();
        else
            FillRangeNames();

        bFormulaMode = bSet;

        HideTip();
    }
}

void ScPosWnd::SetPos( const OUString& rPosStr )
{
    if ( aPosStr != rPosStr )
    {
        aPosStr = rPosStr;
        m_xWidget->set_entry_text(aPosStr);
    }
}

// static
OUString ScPosWnd::createLocalRangeName(std::u16string_view rName, std::u16string_view rTableName)
{
    return OUString::Concat(rName) + " (" + rTableName + ")";
}

void ScPosWnd::FillRangeNames(bool initialize)
{
    std::set<OUString> aSet;
    SfxObjectShell* pObjSh = SfxObjectShell::Current();
    if (auto pDocShell = dynamic_cast<ScDocShell*>(pObjSh))
    {
        ScDocument& rDoc = pDocShell->GetDocument();

        ScRange aDummy;
        ScRangeName* pRangeNames = rDoc.GetRangeName();
        for (const auto& rEntry : *pRangeNames)
        {
            if (rEntry.second->IsValidReference(aDummy))
                aSet.insert(rEntry.second->GetName());
        }
        for (SCTAB i = 0; i < rDoc.GetTableCount(); ++i)
        {
            ScRangeName* pLocalRangeName = rDoc.GetRangeName(i);
            if (pLocalRangeName && !pLocalRangeName->empty())
            {
                OUString aTableName;
                rDoc.GetName(i, aTableName);
                for (const auto& rEntry : *pLocalRangeName)
                {
                    if (rEntry.second->IsValidReference(aDummy))
                        aSet.insert(createLocalRangeName(rEntry.second->GetName(), aTableName));
                }
            }
        }
    }

    if (!bFormulaMode && !initialize && aSet == aRangeNames)
        return;

    aRangeNames = aSet;

    m_xWidget->clear();
    m_xWidget->freeze();
    m_xWidget->append_text(ScResId(STR_MANAGE_NAMES));
    m_xWidget->append_separator(u"separator"_ustr);
    for (const auto& rItem : aSet)
    {
        m_xWidget->append_text(rItem);
    }
    m_xWidget->thaw();
    if (!aPosStr.isEmpty())
        m_xWidget->set_entry_text(aPosStr);
}

void ScPosWnd::FillFunctions()
{
    m_xWidget->clear();
    m_xWidget->freeze();

    OUString aFirstName;
    const ScAppOptions& rOpt = ScModule::get()->GetAppOptions();
    sal_uInt16 nMRUCount = rOpt.GetLRUFuncListCount();
    const sal_uInt16* pMRUList = rOpt.GetLRUFuncList();
    if (pMRUList)
    {
        const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
        sal_uInt32 nListCount = pFuncList->GetCount();
        for (sal_uInt16 i=0; i<nMRUCount; i++)
        {
            sal_uInt16 nId = pMRUList[i];
            for (sal_uInt32 j=0; j<nListCount; j++)
            {
                const ScFuncDesc* pDesc = pFuncList->GetFunction( j );
                if ( pDesc->nFIndex == nId && pDesc->mxFuncName )
                {
                    m_xWidget->append_text(*pDesc->mxFuncName);
                    if (aFirstName.isEmpty())
                        aFirstName = *pDesc->mxFuncName;
                    break; // Stop searching
                }
            }
        }
    }

    //! Re-add entry "Other..." for Function AutoPilot if it can work with text that
    // has been entered so far

    //  m_xWidget->append_text(ScResId(STR_FUNCTIONLIST_MORE));

    m_xWidget->thaw();
    m_xWidget->set_entry_text(aFirstName);
}

void ScPosWnd::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( bFormulaMode )
        return;

    const SfxHintId nHintId = rHint.GetId();
    // Does the list of range names need updating?
    if (nHintId == SfxHintId::ThisIsAnSfxEventHint)
    {
        SfxEventHintId nEventId = static_cast<const SfxEventHint&>(rHint).GetEventId();
        if ( nEventId == SfxEventHintId::ActivateDoc )
            FillRangeNames();
    }
    else
    {
        if (nHintId == SfxHintId::ScAreasChanged || nHintId == SfxHintId::ScNavigatorUpdateAll)
            FillRangeNames();
    }
}

void ScPosWnd::HideTip()
{
    if (nTipVisible)
    {
        Help::HidePopover(this, nTipVisible);
        nTipVisible = nullptr;
    }
}

static ScNameInputType lcl_GetInputType( const OUString& rText )
{
    ScNameInputType eRet = SC_NAME_INPUT_BAD_NAME;      // the more general error

    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if ( pViewSh )
    {
        ScViewData& rViewData = pViewSh->GetViewData();
        ScDocument& rDoc = rViewData.GetDocument();
        SCTAB nTab = rViewData.GetTabNo();
        ScAddress::Details aDetails( rDoc.GetAddressConvention());

        // test in same order as in SID_CURRENTCELL execute

        ScRange aRange;
        ScAddress aAddress;
        SCTAB nNameTab;
        sal_Int32 nNumeric;

        // From the context we know that when testing for a range name
        // sheet-local scope names have " (sheetname)" appended and global
        // names don't and can't contain ')', so we can force one or the other.
        const RutlNameScope eNameScope =
            ((!rText.isEmpty() && rText[rText.getLength()-1] == ')') ? RUTL_NAMES_LOCAL : RUTL_NAMES_GLOBAL);

        if (rText == ScResId(STR_MANAGE_NAMES))
            eRet = SC_MANAGE_NAMES;
        else if ( aRange.Parse( rText, rDoc, aDetails ) & ScRefFlags::VALID )
            eRet = SC_NAME_INPUT_RANGE;
        else if ( aAddress.Parse( rText, rDoc, aDetails ) & ScRefFlags::VALID )
            eRet = SC_NAME_INPUT_CELL;
        else if ( ScRangeUtil::MakeRangeFromName( rText, rDoc, nTab, aRange, eNameScope, aDetails ) )
        {
            eRet = ((eNameScope == RUTL_NAMES_LOCAL) ? SC_NAME_INPUT_NAMEDRANGE_LOCAL :
                    SC_NAME_INPUT_NAMEDRANGE_GLOBAL);
        }
        else if ( ScRangeUtil::MakeRangeFromName( rText, rDoc, nTab, aRange, RUTL_DBASE, aDetails ) )
            eRet = SC_NAME_INPUT_DATABASE;
        else if ( comphelper::string::isdigitAsciiString( rText ) &&
                  ( nNumeric = rText.toInt32() ) > 0 && nNumeric <= rDoc.MaxRow()+1 )
            eRet = SC_NAME_INPUT_ROW;
        else if ( rDoc.GetTable( rText, nNameTab ) )
            eRet = SC_NAME_INPUT_SHEET;
        else if (ScRangeData::IsNameValid(rText, rDoc)
                 == ScRangeData::IsNameValidType::NAME_VALID) // nothing found, create new range?
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

IMPL_LINK_NOARG(ScPosWnd, ModifyHdl, weld::ComboBox&, void)
{
    HideTip();

    if (m_xWidget->changed_by_direct_pick())
    {
        DoEnter();
        return;
    }

    if (bFormulaMode)
        return;

    // determine the action that would be taken for the current input

    ScNameInputType eType = lcl_GetInputType(m_xWidget->get_active_text());      // uses current view
    TranslateId pStrId;
    switch ( eType )
    {
        case SC_NAME_INPUT_CELL:
            pStrId = STR_NAME_INPUT_CELL;
            break;
        case SC_NAME_INPUT_RANGE:
        case SC_NAME_INPUT_NAMEDRANGE_LOCAL:
        case SC_NAME_INPUT_NAMEDRANGE_GLOBAL:
            pStrId = STR_NAME_INPUT_RANGE;      // named range or range reference
            break;
        case SC_NAME_INPUT_DATABASE:
            pStrId = STR_NAME_INPUT_DBRANGE;
            break;
        case SC_NAME_INPUT_ROW:
            pStrId = STR_NAME_INPUT_ROW;
            break;
        case SC_NAME_INPUT_SHEET:
            pStrId = STR_NAME_INPUT_SHEET;
            break;
        case SC_NAME_INPUT_DEFINE:
            pStrId = STR_NAME_INPUT_DEFINE;
            break;
        default:
            // other cases (error): no tip help
            break;
    }

    if (!pStrId)
        return;

    // show the help tip at the text cursor position
    Point aPos;
    vcl::Cursor* pCur = GetCursor();
    if (pCur)
        aPos = LogicToPixel( pCur->GetPos() );
    aPos = OutputToScreenPixel( aPos );
    tools::Rectangle aRect( aPos, aPos );

    OUString aText = ScResId(pStrId);
    QuickHelpFlags const nAlign = QuickHelpFlags::Left|QuickHelpFlags::Bottom;
    nTipVisible = Help::ShowPopover(this, aRect, aText, nAlign);
}

void ScPosWnd::DoEnter()
{
    bool bOpenManageNamesDialog = false;
    OUString aText = m_xWidget->get_active_text();
    if ( !aText.isEmpty() )
    {
        if ( bFormulaMode )
        {
            ScModule* pScMod = ScModule::get();
            if ( aText == ScResId(STR_FUNCTIONLIST_MORE) )
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
                ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>( SfxViewShell::Current()  );
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
                ScDocShell& rDocShell = rViewData.GetDocShell();
                ScDocument& rDoc = rDocShell.GetDocument();

                ScNameInputType eType = lcl_GetInputType( aText );
                if ( eType == SC_NAME_INPUT_BAD_NAME || eType == SC_NAME_INPUT_BAD_SELECTION )
                {
                    TranslateId pId = (eType == SC_NAME_INPUT_BAD_NAME) ? STR_NAME_ERROR_NAME : STR_NAME_ERROR_SELECTION;
                    pViewSh->ErrorMessage(pId);
                }
                else if ( eType == SC_NAME_INPUT_DEFINE )
                {
                    ScRangeName* pNames = rDoc.GetRangeName();
                    ScRange aSelection;
                    if ( pNames && !pNames->findByUpperName(ScGlobal::getCharClass().uppercase(aText)) &&
                            (rViewData.GetSimpleArea( aSelection ) == SC_MARK_SIMPLE) )
                    {
                        ScRangeName aNewRanges( *pNames );
                        ScAddress aCursor( rViewData.GetCurX(), rViewData.GetCurY(), rViewData.GetTabNo() );
                        OUString aContent(aSelection.Format(rDoc, ScRefFlags::RANGE_ABS_3D, rDoc.GetAddressConvention()));
                        ScRangeData* pNew = new ScRangeData( rDoc, aText, aContent, aCursor );
                        if ( aNewRanges.insert(pNew) )
                        {
                            rDocShell.GetDocFunc().ModifyRangeNames( aNewRanges );
                            pViewSh->UpdateInputHandler(true);
                        }
                    }
                }
                else if (eType == SC_MANAGE_NAMES)
                {
                    // dialog is only set below after calling 'ReleaseFocus_Impl' to ensure it gets focus
                    bOpenManageNamesDialog = true;
                }
                else
                {
                    bool bForceGlobalName = false;
                    // for all selection types, execute the SID_CURRENTCELL slot.
                    if (eType == SC_NAME_INPUT_CELL || eType == SC_NAME_INPUT_RANGE)
                    {
                        // Note that SID_CURRENTCELL always expects address to
                        // be in Calc A1 format.  Convert the text.
                        ScRange aRange(0,0, rViewData.GetTabNo());
                        aRange.ParseAny(aText, rDoc, rDoc.GetAddressConvention());
                        aText = aRange.Format(rDoc, ScRefFlags::RANGE_ABS_3D, ::formula::FormulaGrammar::CONV_OOO);
                    }
                    else if (eType == SC_NAME_INPUT_NAMEDRANGE_GLOBAL)
                    {
                        bForceGlobalName = true;
                    }

                    SfxStringItem aPosItem( SID_CURRENTCELL, aText );
                    SfxBoolItem aUnmarkItem( FN_PARAM_1, true );        // remove existing selection
                    // FN_PARAM_2 reserved for AlignToCursor
                    SfxBoolItem aForceGlobalName( FN_PARAM_3, bForceGlobalName );

                    pViewSh->GetViewData().GetDispatcher().ExecuteList( SID_CURRENTCELL,
                                        SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
                                        { &aPosItem, &aUnmarkItem, &aForceGlobalName });
                }
            }
        }
    }
    else
        m_xWidget->set_entry_text(aPosStr);

    ReleaseFocus_Impl();

    if (bOpenManageNamesDialog)
    {
        const sal_uInt16 nId  = ScNameDlgWrapper::GetChildWindowId();
        if (ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell())
        {
            SfxViewFrame& rViewFrm = pViewSh->GetViewFrame();
            SfxChildWindow* pWnd = rViewFrm.GetChildWindow( nId );
            ScModule::get()->SetRefDialog(nId, pWnd == nullptr);
        }
    }
}

IMPL_LINK_NOARG(ScPosWnd, ActivateHdl, weld::ComboBox&, bool)
{
    DoEnter();
    return true;
}

IMPL_LINK(ScPosWnd, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    bool bHandled = true;

    switch (rKEvt.GetKeyCode().GetCode())
    {
        case KEY_RETURN:
            bHandled = ActivateHdl(*m_xWidget);
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
                    m_xWidget->set_entry_text(aPosStr);
                ReleaseFocus_Impl();
            }
            break;
        default:
            bHandled = false;
            break;
    }

    return bHandled || ChildKeyInput(rKEvt);
}

IMPL_LINK_NOARG(ScPosWnd, OnAsyncGetFocus, void*, void)
{
    m_nAsyncGetFocusId = nullptr;
    m_xWidget->select_entry_region(0, -1);
}

IMPL_LINK_NOARG(ScPosWnd, FocusInHdl, weld::Widget&, void)
{
    if (m_nAsyncGetFocusId)
        return;
    // do it async to defeat entry in combobox having its own ideas about the focus
    m_nAsyncGetFocusId = Application::PostUserEvent(LINK(this, ScPosWnd, OnAsyncGetFocus));
}

IMPL_LINK_NOARG(ScPosWnd, FocusOutHdl, weld::Widget&, void)
{
    if (m_nAsyncGetFocusId)
    {
        Application::RemoveUserEvent(m_nAsyncGetFocusId);
        m_nAsyncGetFocusId = nullptr;
    }

    HideTip();
}

void ScPosWnd::ReleaseFocus_Impl()
{
    HideTip();

    SfxViewShell* pCurSh = SfxViewShell::Current();
    ScInputHandler* pHdl = ScModule::get()->GetInputHdl(dynamic_cast<ScTabViewShell*>(pCurSh));
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
