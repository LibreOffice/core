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

#include <config_wasm_strip.h>

#include <sfx2/objface.hxx>
#include <vcl/help.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/syswin.hxx>
#include <vcl/weld.hxx>

#include <rtl/ustrbuf.hxx>
#include <svl/whiter.hxx>
#include <svl/slstitm.hxx>
#include <svl/eitem.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <editeng/paperinf.hxx>
#include <svx/svdview.hxx>
#include <svx/zoomslideritem.hxx>
#include <tools/svborder.hxx>
#include <osl/diagnose.h>

#include <globdoc.hxx>
#include <wdocsh.hxx>
#include <pvprtdat.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <viewopt.hxx>
#include <doc.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <pview.hxx>
#include <view.hxx>
#include <scroll.hxx>
#include <prtopt.hxx>
#include <usrpref.hxx>
#include "viewfunc.hxx"

#include <helpids.h>
#include <cmdid.h>
#include <strings.hrc>

#define ShellClass_SwPagePreview
#include <sfx2/msg.hxx>
#include <swslots.hxx>
#include <pagepreviewlayout.hxx>

#include <svx/svxdlg.hxx>

#include <memory>
#include <vcl/EnumContext.hxx>
#include <vcl/notebookbar/notebookbar.hxx>

using namespace ::com::sun::star;
SFX_IMPL_NAMED_VIEWFACTORY(SwPagePreview, "PrintPreview")
{
    SFX_VIEW_REGISTRATION(SwDocShell);
    SFX_VIEW_REGISTRATION(SwWebDocShell);
    SFX_VIEW_REGISTRATION(SwGlobalDocShell);
}

SFX_IMPL_INTERFACE(SwPagePreview, SfxViewShell)

void SwPagePreview::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu("preview");
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT,
                                            SfxVisibilityFlags::Standard|SfxVisibilityFlags::Client|SfxVisibilityFlags::FullScreen|SfxVisibilityFlags::ReadonlyDoc,
                                            ToolbarId::PView_Toolbox);
}


#define SWVIEWFLAGS SfxViewShellFlags::HAS_PRINTOPTIONS

#define MIN_PREVIEW_ZOOM 25
#define MAX_PREVIEW_ZOOM 600

static sal_uInt16 lcl_GetNextZoomStep(sal_uInt16 nCurrentZoom, bool bZoomIn)
{
    static const sal_uInt16 aZoomArr[] =
    {
        25, 50, 75, 100, 150, 200, 400, 600
    };
    const int nZoomArrSize = static_cast<int>(SAL_N_ELEMENTS(aZoomArr));
    if (bZoomIn)
    {
        for(sal_uInt16 i : aZoomArr)
        {
            if(nCurrentZoom < i)
                return i;
        }
    }
    else
    {
        for(int i = nZoomArrSize - 1; i >= 0; --i)
        {
            if(nCurrentZoom > aZoomArr[i] || !i)
                return aZoomArr[i];
        }
    }
    return bZoomIn ? MAX_PREVIEW_ZOOM : MIN_PREVIEW_ZOOM;
};

static void lcl_InvalidateZoomSlots(SfxBindings& rBindings)
{
    static sal_uInt16 const aInval[] =
    {
        SID_ATTR_ZOOM, SID_ZOOM_OUT, SID_ZOOM_IN, SID_ATTR_ZOOMSLIDER, FN_PREVIEW_ZOOM, FN_STAT_ZOOM,
        0
    };
    rBindings.Invalidate( aInval );
}

namespace {

// At first the zoom dialog
class SwPreviewZoomDlg : public weld::GenericDialogController
{
    SwPagePreviewWin& m_rParent;
    std::unique_ptr<weld::SpinButton> m_xRowEdit;
    std::unique_ptr<weld::SpinButton> m_xColEdit;

public:
    SwPreviewZoomDlg(SwPagePreviewWin& rParent)
        : GenericDialogController(rParent.GetFrameWeld(), "modules/swriter/ui/previewzoomdialog.ui", "PreviewZoomDialog")
        , m_rParent(rParent)
        , m_xRowEdit(m_xBuilder->weld_spin_button("rows"))
        , m_xColEdit(m_xBuilder->weld_spin_button("cols"))
    {
        m_xRowEdit->set_value(rParent.GetRow());
        m_xColEdit->set_value(rParent.GetCol());
    }

    void execute()
    {
        if (run() == RET_OK)
        {
            m_rParent.CalcWish(sal_uInt8(m_xRowEdit->get_value()), sal_uInt8(m_xColEdit->get_value()));
        }
    }
};

}

// all for SwPagePreviewWin
SwPagePreviewWin::SwPagePreviewWin( vcl::Window *pParent, SwPagePreview& rPView )
    : Window(pParent, WinBits(WB_CLIPCHILDREN))
    , mpViewShell(nullptr)
    , mrView(rPView)
    , mbCalcScaleForPreviewLayout(true)
    , maPaintedPreviewDocRect(tools::Rectangle(0,0,0,0))
    , mpPgPreviewLayout(nullptr)
{
    GetOutDev()->SetOutDevViewType( OutDevViewType::PrintPreview );
    SetHelpId(HID_PAGEPREVIEW);
    GetOutDev()->SetFillColor( GetBackground().GetColor() );
    GetOutDev()->SetLineColor( GetBackground().GetColor());
    SetMapMode( MapMode(MapUnit::MapTwip) );

    const SwMasterUsrPref *pUsrPref = SW_MOD()->GetUsrPref(false);
    mnRow = pUsrPref->GetPagePrevRow();     // 1 row
    mnCol = pUsrPref->GetPagePrevCol();     // 1 column
    mnSttPage = USHRT_MAX;
}

SwPagePreviewWin::~SwPagePreviewWin()
{
}

void  SwPagePreviewWin::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    if (!mpViewShell || !mpViewShell->GetLayout())
        return;

    if (USHRT_MAX == mnSttPage)        // was never calculated ? (Init-Phase!)
    {
        // This is the size to which I always relate.
        if (!maPxWinSize.Height() || !maPxWinSize.Width())
            maPxWinSize = GetOutputSizePixel();

        tools::Rectangle aRect(rRenderContext.LogicToPixel(rRect));
        mpPgPreviewLayout->Prepare(1, Point(0,0), maPxWinSize,
                                   mnSttPage, maPaintedPreviewDocRect);
        SetSelectedPage(1);
        mpPgPreviewLayout->Paint(rRenderContext, rRenderContext.PixelToLogic(aRect));
        SetPagePreview(mnRow, mnCol);
    }
    else
    {
        MapMode aMM(rRenderContext.GetMapMode());
        aMM.SetScaleX(maScale);
        aMM.SetScaleY(maScale);
        rRenderContext.SetMapMode(aMM);
        mpPgPreviewLayout->GetParentViewShell().setOutputToWindow(true);
        mpPgPreviewLayout->Paint(rRenderContext, rRect);
        mpPgPreviewLayout->GetParentViewShell().setOutputToWindow(false);
    }
}

void SwPagePreviewWin::CalcWish( sal_uInt8 nNewRow, sal_uInt8 nNewCol )
{
    if( !mpViewShell || !mpViewShell->GetLayout() )
        return;

    const sal_uInt8 nOldCol = mnCol;
    mnRow = nNewRow;
    mnCol = nNewCol;
    const sal_uInt16 nPages = mnRow * mnCol;
    const sal_uInt16 nLastSttPg = mrView.GetPageCount()+1 > nPages
                            ? mrView.GetPageCount()+1 - nPages : 0;
    if( mnSttPage > nLastSttPg )
        mnSttPage = nLastSttPg;

    mpPgPreviewLayout->Init( mnCol, mnRow, maPxWinSize );
    mpPgPreviewLayout->Prepare( mnSttPage, Point(0,0), maPxWinSize,
                              mnSttPage, maPaintedPreviewDocRect );
    SetSelectedPage( mnSttPage );
    SetPagePreview(mnRow, mnCol);
    maScale = GetMapMode().GetScaleX();

    // If changes have taken place at the columns, the special case "single column"
    // must be considered and corrected if necessary.
    if( (1 == nOldCol) != (1 == mnCol) )
        mrView.ScrollDocSzChg();

    // Order must be maintained!
    // additional invalidate page status.
    static sal_uInt16 aInval[] =
    {
        SID_ATTR_ZOOM, SID_ZOOM_OUT, SID_ZOOM_IN,
        FN_PREVIEW_ZOOM,
        FN_START_OF_DOCUMENT, FN_END_OF_DOCUMENT, FN_PAGEUP, FN_PAGEDOWN,
        FN_STAT_PAGE, FN_STAT_ZOOM,
        FN_SHOW_TWO_PAGES, FN_SHOW_MULTIPLE_PAGES,
        0
    };
    SfxBindings& rBindings = mrView.GetViewFrame()->GetBindings();
    rBindings.Invalidate( aInval );
    rBindings.Update( FN_SHOW_TWO_PAGES );
    rBindings.Update( FN_SHOW_MULTIPLE_PAGES );
    // adjust scrollbars
    mrView.ScrollViewSzChg();
}

// mnSttPage is Absolute
bool SwPagePreviewWin::MovePage( int eMoveMode )
{
    // number of pages up
    const sal_uInt16 nPages = mnRow * mnCol;
    sal_uInt16 nNewSttPage = mnSttPage;
    const sal_uInt16 nPageCount = mrView.GetPageCount();
    const sal_uInt16 nDefSttPg = GetDefSttPage();
    bool bPaintPageAtFirstCol = true;

    switch( eMoveMode )
    {
    case MV_PAGE_UP:
    {
        const sal_uInt16 nRelSttPage = mpPgPreviewLayout->ConvertAbsoluteToRelativePageNum( mnSttPage );
        const sal_uInt16 nNewAbsSttPage = nRelSttPage - nPages > 0 ?
                                          mpPgPreviewLayout->ConvertRelativeToAbsolutePageNum( nRelSttPage - nPages ) :
                                          nDefSttPg;
        nNewSttPage = nNewAbsSttPage;

        const sal_uInt16 nRelSelPage = mpPgPreviewLayout->ConvertAbsoluteToRelativePageNum( SelectedPage() );
        const sal_uInt16 nNewRelSelPage = nRelSelPage - nPages > 0 ?
                                          nRelSelPage - nPages :
                                          1;
        SetSelectedPage( mpPgPreviewLayout->ConvertRelativeToAbsolutePageNum( nNewRelSelPage ) );

        break;
    }
    case MV_PAGE_DOWN:
    {
        const sal_uInt16 nRelSttPage = mpPgPreviewLayout->ConvertAbsoluteToRelativePageNum( mnSttPage );
        const sal_uInt16 nNewAbsSttPage = mpPgPreviewLayout->ConvertRelativeToAbsolutePageNum( nRelSttPage + nPages );
        nNewSttPage = std::min(nNewAbsSttPage, nPageCount);

        const sal_uInt16 nRelSelPage = mpPgPreviewLayout->ConvertAbsoluteToRelativePageNum( SelectedPage() );
        const sal_uInt16 nNewAbsSelPage = mpPgPreviewLayout->ConvertRelativeToAbsolutePageNum( nRelSelPage + nPages );
        SetSelectedPage( std::min(nNewAbsSelPage, nPageCount) );

        break;
    }
    case MV_DOC_STT:
        nNewSttPage = nDefSttPg;
        SetSelectedPage( mpPgPreviewLayout->ConvertRelativeToAbsolutePageNum( nNewSttPage ? nNewSttPage : 1 ) );
        break;
    case MV_DOC_END:
        // correct calculation of new start page.
        nNewSttPage = nPageCount;
        SetSelectedPage( nPageCount );
        break;

    case MV_SELPAGE:
        // <nNewSttPage> and <SelectedPage()> are already set.
        // not start at first column, only if the
        // complete preview layout columns doesn't fit into window.
        if ( !mpPgPreviewLayout->DoesPreviewLayoutColsFitIntoWindow() )
            bPaintPageAtFirstCol = false;
        break;
    case MV_SCROLL:
        // check, if paint page at first column
        // has to be avoided
        if ( !mpPgPreviewLayout->DoesPreviewLayoutRowsFitIntoWindow() ||
             !mpPgPreviewLayout->DoesPreviewLayoutColsFitIntoWindow() )
            bPaintPageAtFirstCol = false;
        break;
    case MV_NEWWINSIZE:
        // nothing special to do.
        break;
    case MV_CALC:
        // re-init page preview layout.
        mpPgPreviewLayout->ReInit();

        // correct calculation of new start page.
        if( nNewSttPage > nPageCount )
            nNewSttPage = nPageCount;

        // correct selected page number
        if( SelectedPage() > nPageCount )
            SetSelectedPage( nNewSttPage ? nNewSttPage : 1 );
    }

    mpPgPreviewLayout->Prepare( nNewSttPage, Point(0,0), maPxWinSize,
                              nNewSttPage,
                              maPaintedPreviewDocRect, bPaintPageAtFirstCol );
    if( nNewSttPage == mnSttPage &&
        eMoveMode != MV_SELPAGE )
        return false;

    SetPagePreview(mnRow, mnCol);
    mnSttPage = nNewSttPage;

    // additional invalidate page status.
    static sal_uInt16 aInval[] =
    {
        FN_START_OF_DOCUMENT, FN_END_OF_DOCUMENT, FN_PAGEUP, FN_PAGEDOWN,
        FN_STAT_PAGE, 0
    };

    SfxBindings& rBindings = mrView.GetViewFrame()->GetBindings();
    rBindings.Invalidate( aInval );

    return true;
}

void SwPagePreviewWin::SetWinSize( const Size& rNewSize )
{
    // We always want the size as pixel units.
    maPxWinSize = LogicToPixel( rNewSize );

    if( USHRT_MAX == mnSttPage )
    {
        mnSttPage = GetDefSttPage();
        SetSelectedPage( GetDefSttPage() );
    }

    if ( mbCalcScaleForPreviewLayout )
    {
        mpPgPreviewLayout->Init( mnCol, mnRow, maPxWinSize );
        maScale = GetMapMode().GetScaleX();
    }
    mpPgPreviewLayout->Prepare( mnSttPage, Point(0,0), maPxWinSize,
                              mnSttPage, maPaintedPreviewDocRect );
    if ( mbCalcScaleForPreviewLayout )
    {
        SetSelectedPage( mnSttPage );
        mbCalcScaleForPreviewLayout = false;
    }
    SetPagePreview(mnRow, mnCol);
    maScale = GetMapMode().GetScaleX();
}

OUString SwPagePreviewWin::GetStatusStr( sal_uInt16 nPageCnt ) const
{
    // show physical and virtual page number of
    // selected page, if it's visible.
    const sal_uInt16 nPageNum = mpPgPreviewLayout->IsPageVisible( mpPgPreviewLayout->SelectedPage() )
        ? mpPgPreviewLayout->SelectedPage() : std::max<sal_uInt16>(mnSttPage, 1);

    OUString aStatusStr;
    const sal_uInt16 nVirtPageNum = mpPgPreviewLayout->GetVirtPageNumByPageNum( nPageNum );
    if( nVirtPageNum && nVirtPageNum != nPageNum )
    {
        aStatusStr = OUString::number(nVirtPageNum) + " " ;
    }
    return aStatusStr + OUString::number(nPageNum) + " / " + OUString::number(nPageCnt);
}

void  SwPagePreviewWin::KeyInput( const KeyEvent &rKEvt )
{
    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
    bool bHandled = false;
    if(!rKeyCode.GetModifier())
    {
        sal_uInt16 nSlot = 0;
        switch(rKeyCode.GetCode())
        {
            case KEY_ADD : nSlot = SID_ZOOM_OUT;         break;
            case KEY_ESCAPE: nSlot = FN_CLOSE_PAGEPREVIEW; break;
            case KEY_SUBTRACT : nSlot = SID_ZOOM_IN;    break;
        }
        if(nSlot)
        {
            bHandled = true;
            mrView.GetViewFrame()->GetDispatcher()->Execute(
                                nSlot, SfxCallMode::ASYNCHRON );
        }
    }
    if( !bHandled && !mrView.KeyInput( rKEvt ) )
        Window::KeyInput( rKEvt );
}

void SwPagePreviewWin::Command( const CommandEvent& rCEvt )
{
    bool bCallBase = true;
    switch( rCEvt.GetCommand() )
    {
        case CommandEventId::ContextMenu:
            SfxDispatcher::ExecutePopup();
            bCallBase = false;
        break;

        case CommandEventId::Wheel:
        case CommandEventId::StartAutoScroll:
        case CommandEventId::AutoScroll:
        {
            const CommandWheelData* pData = rCEvt.GetWheelData();
            if( pData )
            {
                const CommandWheelData aDataNew(pData->GetDelta(),pData->GetNotchDelta(),COMMAND_WHEEL_PAGESCROLL,
                    pData->GetMode(),pData->GetModifier(),pData->IsHorz(), pData->IsDeltaPixel());
                const CommandEvent aEvent( rCEvt.GetMousePosPixel(),rCEvt.GetCommand(),rCEvt.IsMouseEvent(),&aDataNew);
                bCallBase = !mrView.HandleWheelCommands( aEvent );
            }
            else
                bCallBase = !mrView.HandleWheelCommands( rCEvt );
       }
       break;
       default:
           ;
    }

    if( bCallBase )
        Window::Command( rCEvt );
}

void SwPagePreviewWin::MouseButtonDown( const MouseEvent& rMEvt )
{
    // consider single-click to set selected page
    if( MOUSE_LEFT != ( rMEvt.GetModifier() + rMEvt.GetButtons() ) )
        return;

    Point aPreviewPos( PixelToLogic( rMEvt.GetPosPixel() ) );
    Point aDocPos;
    bool bPosInEmptyPage;
    sal_uInt16 nNewSelectedPage;
    bool bIsDocPos =
        mpPgPreviewLayout->IsPreviewPosInDocPreviewPage( aPreviewPos,
                                aDocPos, bPosInEmptyPage, nNewSelectedPage );
    if ( bIsDocPos && rMEvt.GetClicks() == 2 )
    {
        // close page preview, set new cursor position and switch to
        // normal view.
        OUString sNewCursorPos = OUString::number( aDocPos.X() ) + ";" +
                               OUString::number( aDocPos.Y() ) + ";";
        mrView.SetNewCursorPos( sNewCursorPos );

        SfxViewFrame *pTmpFrame = mrView.GetViewFrame();
        pTmpFrame->GetBindings().Execute( SID_VIEWSHELL0, nullptr,
                                                SfxCallMode::ASYNCHRON );
    }
    else if ( bIsDocPos || bPosInEmptyPage )
    {
        // show clicked page as the selected one
        mpPgPreviewLayout->MarkNewSelectedPage( nNewSelectedPage );
        GetViewShell()->ShowPreviewSelection( nNewSelectedPage );
        // adjust position at vertical scrollbar.
        if ( mpPgPreviewLayout->DoesPreviewLayoutRowsFitIntoWindow() )
        {
            mrView.SetVScrollbarThumbPos( nNewSelectedPage );
        }
        // invalidate page status.
        static sal_uInt16 aInval[] =
        {
            FN_STAT_PAGE, 0
        };
        SfxBindings& rBindings = mrView.GetViewFrame()->GetBindings();
        rBindings.Invalidate( aInval );
    }
}

// Set user prefs or view options

void SwPagePreviewWin::SetPagePreview( sal_uInt8 nRow, sal_uInt8 nCol )
{
    SwMasterUsrPref *pOpt = const_cast<SwMasterUsrPref *>(SW_MOD()->GetUsrPref(false));

    if (nRow != pOpt->GetPagePrevRow() || nCol != pOpt->GetPagePrevCol())
    {
        pOpt->SetPagePrevRow( nRow );
        pOpt->SetPagePrevCol( nCol );
        pOpt->SetModified();

        // Update scrollbar!
        mrView.ScrollViewSzChg();
    }
}

/** get selected page in document preview */
sal_uInt16 SwPagePreviewWin::SelectedPage() const
{
    return mpPgPreviewLayout->SelectedPage();
}

/** set selected page number in document preview */
void SwPagePreviewWin::SetSelectedPage( sal_uInt16 _nSelectedPageNum )
{
    mpPgPreviewLayout->SetSelectedPage( _nSelectedPageNum );
}

/** method to enable/disable book preview */
bool SwPagePreviewWin::SetBookPreviewMode( const bool _bBookPreview )
{
    return mpPgPreviewLayout->SetBookPreviewMode( _bBookPreview,
                                                mnSttPage,
                                                maPaintedPreviewDocRect );
}

void SwPagePreviewWin::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    switch( rDCEvt.GetType() )
    {
    case DataChangedEventType::SETTINGS:
        // Rearrange the scrollbars or trigger resize, because the
        // size of the scrollbars may have be changed. Also the
        // size of the scrollbars has to be retrieved from the settings
        // out of the resize handler.
        if( rDCEvt.GetFlags() & AllSettingsFlags::STYLE )
            mrView.InvalidateBorder();              // Scrollbar widths
        // zoom has to be disabled if Accessibility support is switched on
        lcl_InvalidateZoomSlots(mrView.GetViewFrame()->GetBindings());
        break;

    case DataChangedEventType::PRINTER:
    case DataChangedEventType::DISPLAY:
    case DataChangedEventType::FONTS:
    case DataChangedEventType::FONTSUBSTITUTION:
        mrView.GetDocShell()->UpdateFontList(); // Font change
        mpViewShell->InvalidateLayout(true);
        if ( mpViewShell->GetWin() )
            mpViewShell->GetWin()->Invalidate();
        break;
    default: break;
    }
}

/** help method to execute SfxRequest FN_PAGEUP and FN_PAGEDOWN */
void SwPagePreview::ExecPgUpAndPgDown( const bool  _bPgUp,
                                        SfxRequest* _pReq )
{
    SwPagePreviewLayout* pPagePreviewLay = GetViewShell()->PagePreviewLayout();
    // check, if top/bottom of preview is *not* already visible.
    if( pPagePreviewLay->GetWinPagesScrollAmount( _bPgUp ? -1 : 1 ) != 0 )
    {
        if ( pPagePreviewLay->DoesPreviewLayoutRowsFitIntoWindow() &&
             pPagePreviewLay->DoesPreviewLayoutColsFitIntoWindow() )
        {
            const int eMvMode = _bPgUp ?
                                SwPagePreviewWin::MV_PAGE_UP :
                                SwPagePreviewWin::MV_PAGE_DOWN;
            if ( ChgPage( eMvMode ) )
                m_pViewWin->Invalidate();
        }
        else
        {
            SwTwips nScrollAmount;
            sal_uInt16 nNewSelectedPageNum = 0;
            const sal_uInt16 nVisPages = m_pViewWin->GetRow() * m_pViewWin->GetCol();
            if( _bPgUp )
            {
                if ( pPagePreviewLay->DoesPreviewLayoutRowsFitIntoWindow() )
                {
                    nScrollAmount = pPagePreviewLay->GetWinPagesScrollAmount( -1 );
                    if ( (m_pViewWin->SelectedPage() - nVisPages) > 0 )
                        nNewSelectedPageNum = m_pViewWin->SelectedPage() - nVisPages;
                    else
                        nNewSelectedPageNum = 1;
                }
                else
                    nScrollAmount = - std::min( m_pViewWin->GetOutDev()->GetOutputSize().Height(),
                                           m_pViewWin->GetPaintedPreviewDocRect().Top() );
            }
            else
            {
                if ( pPagePreviewLay->DoesPreviewLayoutRowsFitIntoWindow() )
                {
                    nScrollAmount = pPagePreviewLay->GetWinPagesScrollAmount( 1 );
                    if ( (m_pViewWin->SelectedPage() + nVisPages) <= mnPageCount )
                        nNewSelectedPageNum = m_pViewWin->SelectedPage() + nVisPages;
                    else
                        nNewSelectedPageNum = mnPageCount;
                }
                else
                    nScrollAmount = std::min( m_pViewWin->GetOutDev()->GetOutputSize().Height(),
                                         ( pPagePreviewLay->GetPreviewDocSize().Height() -
                                           m_pViewWin->GetPaintedPreviewDocRect().Bottom() ) );
            }
            m_pViewWin->Scroll( 0, nScrollAmount );
            if ( nNewSelectedPageNum != 0 )
            {
                m_pViewWin->SetSelectedPage( nNewSelectedPageNum );
            }
            ScrollViewSzChg();
            // additional invalidate page status.
            static sal_uInt16 aInval[] =
            {
                FN_START_OF_DOCUMENT, FN_END_OF_DOCUMENT, FN_PAGEUP, FN_PAGEDOWN,
                FN_STAT_PAGE, 0
            };
            SfxBindings& rBindings = GetViewFrame()->GetBindings();
            rBindings.Invalidate( aInval );
            m_pViewWin->Invalidate();
        }
    }

    if ( _pReq )
        _pReq->Done();
}

// Then all for the SwPagePreview
void  SwPagePreview::Execute( SfxRequest &rReq )
{
    int eMvMode = SwPagePreviewWin::MV_DOC_END;
    sal_uInt8 nRow = 1;
    bool bRefresh = true;

    switch(rReq.GetSlot())
    {
        case SID_REFRESH_VIEW:
        case FN_STAT_PAGE:
        case FN_STAT_ZOOM:
            break;

        case FN_SHOW_MULTIPLE_PAGES:
        {
            const SfxItemSet *pArgs = rReq.GetArgs();
            if( pArgs && pArgs->Count() >= 2 )
            {
                sal_uInt8 nCols = static_cast<sal_uInt8>(pArgs->Get(SID_ATTR_TABLE_COLUMN).GetValue());
                sal_uInt8 nRows = static_cast<sal_uInt8>(pArgs->Get(SID_ATTR_TABLE_ROW).GetValue());
                m_pViewWin->CalcWish( nRows, nCols );

            }
            else
            {
                SwPreviewZoomDlg aDlg(*m_pViewWin);
                aDlg.execute();
            }
        }
        break;
        case FN_SHOW_BOOKVIEW:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();
            const SfxPoolItem* pItem;
            bool bBookPreview = GetViewShell()->GetViewOptions()->IsPagePrevBookview();
            if( pArgs && SfxItemState::SET == pArgs->GetItemState( FN_SHOW_BOOKVIEW, false, &pItem ) )
            {
                bBookPreview = static_cast< const SfxBoolItem* >( pItem )->GetValue();
                const_cast<SwViewOption*>(GetViewShell()->GetViewOptions())->SetPagePrevBookview( bBookPreview );
                    // cast is not gentleman like, but it's common use in writer and in this case
            }
            if ( m_pViewWin->SetBookPreviewMode( bBookPreview ) )
            {
                // book preview mode changed. Thus, adjust scrollbars and
                // invalidate corresponding states.
                ScrollViewSzChg();
                static sal_uInt16 aInval[] =
                {
                    FN_START_OF_DOCUMENT, FN_END_OF_DOCUMENT, FN_PAGEUP, FN_PAGEDOWN,
                    FN_STAT_PAGE, FN_SHOW_BOOKVIEW, 0
                };
                SfxBindings& rBindings = GetViewFrame()->GetBindings();
                rBindings.Invalidate( aInval );
                m_pViewWin->Invalidate();
            }

        }
        break;
        case FN_SHOW_TWO_PAGES:
            m_pViewWin->CalcWish( nRow, 2 );
            break;

        case FN_SHOW_SINGLE_PAGE:
            m_pViewWin->CalcWish( nRow, 1 );
            break;

        case FN_PREVIEW_ZOOM:
        case SID_ATTR_ZOOM:
        {
            const SfxItemSet *pArgs = rReq.GetArgs();
            const SfxPoolItem* pItem;
            ScopedVclPtr<AbstractSvxZoomDialog> pDlg;
            if(!pArgs)
            {
                SfxItemSetFixed<SID_ATTR_ZOOM, SID_ATTR_ZOOM> aCoreSet(GetPool());
                const SwViewOption* pVOpt = GetViewShell()->GetViewOptions();
                SvxZoomItem aZoom( pVOpt->GetZoomType(), pVOpt->GetZoom() );
                aZoom.SetValueSet(
                        SvxZoomEnableFlags::N50|
                        SvxZoomEnableFlags::N75|
                        SvxZoomEnableFlags::N100|
                        SvxZoomEnableFlags::N150|
                        SvxZoomEnableFlags::N200|
                        SvxZoomEnableFlags::WHOLEPAGE);
                aCoreSet.Put( aZoom );

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                pDlg.disposeAndReset(pFact->CreateSvxZoomDialog(GetViewFrame()->GetFrameWeld(), aCoreSet));
                pDlg->SetLimits( MINZOOM, MAXZOOM );

                if( pDlg->Execute() != RET_CANCEL )
                    pArgs = pDlg->GetOutputItemSet();
            }
            if( pArgs )
            {
                SvxZoomType eType = SvxZoomType::PERCENT;
                sal_uInt16 nZoomFactor = USHRT_MAX;
                if(SfxItemState::SET == pArgs->GetItemState(SID_ATTR_ZOOM, true, &pItem))
                {
                    eType = static_cast<const SvxZoomItem *>(pItem)->GetType();
                    nZoomFactor = static_cast<const SvxZoomItem *>(pItem)->GetValue();
                }
                else if(SfxItemState::SET == pArgs->GetItemState(FN_PREVIEW_ZOOM, true, &pItem))
                    nZoomFactor = static_cast<const SfxUInt16Item *>(pItem)->GetValue();
                if(USHRT_MAX != nZoomFactor)
                    SetZoom(eType, nZoomFactor);
            }
        }
        break;
        case SID_ATTR_ZOOMSLIDER :
        {
            const SfxItemSet *pArgs = rReq.GetArgs();
            const SfxPoolItem* pItem;

            if ( pArgs && SfxItemState::SET == pArgs->GetItemState(SID_ATTR_ZOOMSLIDER, true, &pItem ) )
            {
                const sal_uInt16 nCurrentZoom = static_cast<const SvxZoomSliderItem *>(pItem)->GetValue();
                SetZoom( SvxZoomType::PERCENT, nCurrentZoom );
            }
        }
        break;
        case SID_ZOOM_IN:
        case SID_ZOOM_OUT:
        {
            const SwViewOption* pVOpt = GetViewShell()->GetViewOptions();
            SetZoom(SvxZoomType::PERCENT,
                    lcl_GetNextZoomStep(pVOpt->GetZoom(), SID_ZOOM_IN == rReq.GetSlot()));
        }
        break;
        case FN_CHAR_LEFT:
        case FN_CHAR_RIGHT:
        case FN_LINE_UP:
        case FN_LINE_DOWN:
        {
            SwPagePreviewLayout* pPagePreviewLay = GetViewShell()->PagePreviewLayout();
            sal_uInt16 nNewSelectedPage;
            sal_uInt16 nNewStartPage;
            Point aNewStartPos;
            sal_Int16 nHoriMove = 0;
            sal_Int16 nVertMove = 0;
            switch(rReq.GetSlot())
            {
                case FN_CHAR_LEFT:  nHoriMove = -1; break;
                case FN_CHAR_RIGHT: nHoriMove = 1;  break;
                case FN_LINE_UP:    nVertMove = -1; break;
                case FN_LINE_DOWN:  nVertMove = 1;  break;
            }
            pPagePreviewLay->CalcStartValuesForSelectedPageMove( nHoriMove, nVertMove,
                                nNewSelectedPage, nNewStartPage, aNewStartPos );
            if ( m_pViewWin->SelectedPage() != nNewSelectedPage )
            {
                if ( pPagePreviewLay->IsPageVisible( nNewSelectedPage ) )
                {
                    pPagePreviewLay->MarkNewSelectedPage( nNewSelectedPage );
                    // adjust position at vertical scrollbar.
                    SetVScrollbarThumbPos( nNewSelectedPage );
                    bRefresh = false;
                }
                else
                {
                    m_pViewWin->SetSelectedPage( nNewSelectedPage );
                    m_pViewWin->SetSttPage( nNewStartPage );
                    bRefresh = ChgPage( SwPagePreviewWin::MV_SELPAGE );
                }
                GetViewShell()->ShowPreviewSelection( nNewSelectedPage );
                // invalidate page status.
                static sal_uInt16 aInval[] =
                {
                    FN_STAT_PAGE, 0
                };
                SfxBindings& rBindings = GetViewFrame()->GetBindings();
                rBindings.Invalidate( aInval );
                rReq.Done();
            }
            else
            {
                bRefresh = false;
            }
            break;
        }
        case FN_PAGEUP:
        case FN_PAGEDOWN:
        {
            ExecPgUpAndPgDown( rReq.GetSlot() == FN_PAGEUP, &rReq );
            break;
        }
        case SID_JUMP_TO_SPECIFIC_PAGE:
        {
            const SfxItemSet *pArgs = rReq.GetArgs();
            if( pArgs && pArgs->Count())
            {
                sal_uInt16 nPageNum = static_cast<const SfxUInt16Item &>(pArgs->Get(SID_JUMP_TO_SPECIFIC_PAGE)).GetValue();

                if( nPageNum > 0 && nPageNum <= mnPageCount )
                {
                    m_pViewWin->SetSttPage( nPageNum);
                    m_pViewWin->SetSelectedPage( nPageNum );
                    ChgPage( SwPagePreviewWin::MV_SPECIFIC_PAGE, false );
                    ScrollViewSzChg();
                }
            }
        }
        break;
        case FN_START_OF_LINE:
        case FN_START_OF_DOCUMENT:
            eMvMode = SwPagePreviewWin::MV_DOC_STT;
            [[fallthrough]];
        case FN_END_OF_LINE:
        case FN_END_OF_DOCUMENT:
            m_pViewWin->SetSelectedPage(eMvMode == SwPagePreviewWin::MV_DOC_STT ? 1 : mnPageCount);
            {
                bool bRet = ChgPage( eMvMode );
                // return value for Basic
                rReq.SetReturnValue(SfxBoolItem(rReq.GetSlot(), !bRet));

                bRefresh = bRet;
                rReq.Done();
            }
            break;

        case FN_PRINT_PAGEPREVIEW:
        {
            const SwPagePreviewPrtData* pPPVPD = m_pViewWin->GetViewShell()->GetDoc()->GetPreviewPrtData();
            // The thing with the orientation
            if(pPPVPD)
            {
                SfxPrinter* pPrinter = GetPrinter( true );
                if((pPrinter->GetOrientation() == Orientation::Landscape)
                        != pPPVPD->GetLandscape())
                    pPrinter->SetOrientation(pPPVPD->GetLandscape() ? Orientation::Landscape : Orientation::Portrait);
            }
            ::SetAppPrintOptions( m_pViewWin->GetViewShell(), false );
            m_bNormalPrint = false;
            rReq.SetSlot( SID_PRINTDOC );
            SfxViewShell::ExecuteSlot( rReq, SfxViewShell::GetInterface() );
            rReq.SetSlot( FN_PRINT_PAGEPREVIEW );
            return;
        }
        case SID_PRINTDOCDIRECT:
        case SID_PRINTDOC:
            ::SetAppPrintOptions( m_pViewWin->GetViewShell(), false );
            m_bNormalPrint = true;
            SfxViewShell::ExecuteSlot( rReq, SfxViewShell::GetInterface() );
            return;
        case FN_CLOSE_PAGEPREVIEW:
        case SID_PRINTPREVIEW:
            //  print preview is now always in the same frame as the tab view
            //  -> always switch this frame back to normal view
            //  (ScTabViewShell ctor reads stored view data)
            GetViewFrame()->GetDispatcher()->Execute( SID_VIEWSHELL0, SfxCallMode::ASYNCHRON );
            break;
        case FN_INSERT_BREAK:
        {
            sal_uInt16 nSelPage = m_pViewWin->SelectedPage();
            //if a dummy page is selected (e.g. a non-existing right/left page)
            //the direct neighbor is used
            if(GetViewShell()->IsDummyPage( nSelPage ) && GetViewShell()->IsDummyPage( --nSelPage ))
                nSelPage +=2;
            m_nNewPage = nSelPage;
            SfxViewFrame *pTmpFrame = GetViewFrame();
            pTmpFrame->GetBindings().Execute( SID_VIEWSHELL0, nullptr,
                                                    SfxCallMode::ASYNCHRON );
        }
        break;
        default:
            OSL_ENSURE(false, "wrong dispatcher");
            return;
    }

    if( bRefresh )
        m_pViewWin->Invalidate();
}

void  SwPagePreview::GetState( SfxItemSet& rSet )
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    OSL_ENSURE(nWhich, "empty set");
    SwPagePreviewLayout* pPagePreviewLay = GetViewShell()->PagePreviewLayout();

    while(nWhich)
    {
        switch(nWhich)
        {
        case SID_BROWSER_MODE:
        case FN_PRINT_LAYOUT:
            rSet.DisableItem(nWhich);
            break;
        case FN_START_OF_DOCUMENT:
        {
            if ( pPagePreviewLay->IsPageVisible( 1 ) )
                rSet.DisableItem(nWhich);
            break;
        }
        case FN_END_OF_DOCUMENT:
        {
            if ( pPagePreviewLay->IsPageVisible( mnPageCount ) )
                rSet.DisableItem(nWhich);
            break;
        }
        case FN_PAGEUP:
        {
            if( pPagePreviewLay->GetWinPagesScrollAmount( -1 ) == 0 )
                rSet.DisableItem(nWhich);
            break;
        }
        case FN_PAGEDOWN:
        {
            if( pPagePreviewLay->GetWinPagesScrollAmount( 1 ) == 0 )
                rSet.DisableItem(nWhich);
            break;
        }

        case FN_STAT_PAGE:
            {
                std::vector<OUString> aStringList
                {
                    m_sPageStr + m_pViewWin->GetStatusStr(mnPageCount),
                    OUString()
                };
                rSet.Put(SfxStringListItem(FN_STAT_PAGE, &aStringList));
            }
            break;

        case SID_ATTR_ZOOM:
        case FN_STAT_ZOOM:
            {
                    const SwViewOption* pVOpt = GetViewShell()->GetViewOptions();
                    SvxZoomItem aZoom(pVOpt->GetZoomType(), pVOpt->GetZoom());
                    aZoom.SetValueSet(
                            SvxZoomEnableFlags::N50|
                            SvxZoomEnableFlags::N75|
                            SvxZoomEnableFlags::N100|
                            SvxZoomEnableFlags::N150|
                            SvxZoomEnableFlags::N200);
                    rSet.Put( aZoom );
            }
        break;
        case SID_ATTR_ZOOMSLIDER :
            {
                    const SwViewOption* pVOpt = GetViewShell()->GetViewOptions();
                    const sal_uInt16 nCurrentZoom = pVOpt->GetZoom();
                    SvxZoomSliderItem aZoomSliderItem( nCurrentZoom, MINZOOM, MAXZOOM );
                    aZoomSliderItem.AddSnappingPoint( 100 );
                    rSet.Put( aZoomSliderItem );
            }
        break;
        case FN_PREVIEW_ZOOM:
        {
                const SwViewOption* pVOpt = GetViewShell()->GetViewOptions();
                rSet.Put(SfxUInt16Item(nWhich, pVOpt->GetZoom()));
        }
        break;
        case SID_ZOOM_IN:
        case SID_ZOOM_OUT:
        {
            const SwViewOption* pVOpt = GetViewShell()->GetViewOptions();
            if((SID_ZOOM_IN == nWhich && pVOpt->GetZoom() >= MAX_PREVIEW_ZOOM) ||
                    (SID_ZOOM_OUT == nWhich && pVOpt->GetZoom() <= MIN_PREVIEW_ZOOM))
            {
                rSet.DisableItem(nWhich);
            }
        }
        break;
        case FN_SHOW_MULTIPLE_PAGES:
        // should never be disabled
        break;
        case FN_SHOW_BOOKVIEW:
        {
            bool b = GetViewShell()->GetViewOptions()->IsPagePrevBookview();
            rSet.Put(SfxBoolItem(nWhich, b));
        }
        break;

        case FN_SHOW_TWO_PAGES:
            if( 2 == m_pViewWin->GetCol() && 1 == m_pViewWin->GetRow() )
                rSet.DisableItem( nWhich );
            break;

        case FN_PRINT_PAGEPREVIEW:
            // has the same status like the normal printing
            {
                const SfxPoolItem* pItem;
                SfxItemSetFixed<SID_PRINTDOC, SID_PRINTDOC> aSet( *rSet.GetPool() );
                GetSlotState( SID_PRINTDOC, SfxViewShell::GetInterface(), &aSet );
                if( SfxItemState::DISABLED == aSet.GetItemState( SID_PRINTDOC,
                        false, &pItem ))
                    rSet.DisableItem( nWhich );
                else if( SfxItemState::SET == aSet.GetItemState( SID_PRINTDOC,
                        false, &pItem ))
                {
                    const_cast<SfxPoolItem*>(pItem)->SetWhich( FN_PRINT_PAGEPREVIEW );
                    rSet.Put( *pItem );
                }
            }
            break;

        case SID_PRINTPREVIEW:
            rSet.Put( SfxBoolItem( nWhich, true ) );
            break;

        case SID_PRINTDOC:
        case SID_PRINTDOCDIRECT:
            GetSlotState( nWhich, SfxViewShell::GetInterface(), &rSet );
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

void  SwPagePreview::StateUndo(SfxItemSet& rSet)
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    while (nWhich)
    {
        rSet.DisableItem(nWhich);
        nWhich = aIter.NextWhich();
    }
}

void SwPagePreview::Init()
{
    if ( GetViewShell()->HasDrawView() )
        GetViewShell()->GetDrawView()->SetAnimationEnabled( false );

    m_bNormalPrint = true;

    // Check and process the DocSize. The shell could not be found via
    // the handler, because the shell is unknown to the SFX management
    // within the CTOR phase.

    const SwViewOption * pPrefs = SW_MOD()->GetUsrPref(false);

    mbHScrollbarEnabled = pPrefs->IsViewHScrollBar();
    mbVScrollbarEnabled = pPrefs->IsViewVScrollBar();

    // Update the fields
    // ATTENTION: Do cast the EditShell up, to use the SS.
    //            At the methods the current shell will be queried!
    SwEditShell* pESh = dynamic_cast<SwEditShell*>(GetViewShell());
    bool bIsModified = pESh != nullptr && pESh->IsModified();

    SwViewOption aOpt( *pPrefs );
    aOpt.SetPagePreview(true);
    aOpt.SetTab( false );
    aOpt.SetBlank( false );
    aOpt.SetHardBlank( false );
    aOpt.SetParagraph( false );
    aOpt.SetLineBreak( false );
    aOpt.SetPageBreak( false );
    aOpt.SetColumnBreak( false );
    aOpt.SetSoftHyph( false );
    aOpt.SetFieldName( false );
    aOpt.SetPostIts( false );
    aOpt.SetShowBookmarks( false );
    aOpt.SetShowHiddenChar( false );
    aOpt.SetShowHiddenField( false );
    aOpt.SetShowHiddenPara( false );
    aOpt.SetViewHRuler( false );
    aOpt.SetViewVRuler( false );
    aOpt.SetGraphic( true );
    aOpt.SetTable( true );
    aOpt.SetSnap( false );
    aOpt.SetGridVisible( false );
    aOpt.SetOnlineSpell( false );
    aOpt.SetHideWhitespaceMode( false );

    GetViewShell()->ApplyViewOptions( aOpt );
#ifndef ENABLE_WASM_STRIP_ACCESSIBILITY
    GetViewShell()->ApplyAccessibilityOptions(SW_MOD()->GetAccessibilityOptions());
#endif

    // adjust view shell option to the same as for print
    SwPrintData const aPrintOptions = *SW_MOD()->GetPrtOptions(false);
    GetViewShell()->AdjustOptionsForPagePreview( aPrintOptions );

    GetViewShell()->CalcLayout();
    DocSzChgd( GetViewShell()->GetDocSize() );

    if( !bIsModified && pESh != nullptr )
        pESh->ResetModified();
}

SwPagePreview::SwPagePreview(SfxViewFrame *pViewFrame, SfxViewShell* pOldSh):
    SfxViewShell( pViewFrame, SWVIEWFLAGS ),
    m_pViewWin( VclPtr<SwPagePreviewWin>::Create(&GetViewFrame()->GetWindow(), *this ) ),
    m_nNewPage(USHRT_MAX),
    m_sPageStr(SwResId(STR_PAGE)),
    m_pHScrollbar(nullptr),
    m_pVScrollbar(nullptr),
    m_pScrollFill(VclPtr<ScrollBarBox>::Create( &pViewFrame->GetWindow(), WB_SIZEABLE )),
    mnPageCount( 0 ),
    mbResetFormDesignMode( false ),
    mbFormDesignModeToReset( false )
{
    SetName("PageView");
    SetWindow( m_pViewWin );
    CreateScrollbar( true );
    CreateScrollbar( false );

    //notify notebookbar change in context
    SfxShell::SetContextBroadcasterEnabled(true);
    SfxShell::SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::Printpreview));
    SfxShell::BroadcastContextForActivation(true);
    //removelisteners for notebookbar
    if (SfxViewFrame* pCurrent = SfxViewFrame::Current())
        if (auto& pBar = pCurrent->GetWindow().GetSystemWindow()->GetNotebookBar())
            pBar->ControlListenerForCurrentController(false);

    SfxObjectShell* pObjShell = pViewFrame->GetObjectShell();
    if ( !pOldSh )
    {
        // Exists already a view on the document?
        SfxViewFrame *pF = SfxViewFrame::GetFirst( pObjShell );
        if ( pF == pViewFrame )
            pF = SfxViewFrame::GetNext( *pF, pObjShell );
        if ( pF )
            pOldSh = pF->GetViewShell();
    }

    SwViewShell *pVS, *pNew;

    if (SwPagePreview* pPagePreview = dynamic_cast<SwPagePreview*>(pOldSh))
        pVS = pPagePreview->GetViewShell();
    else
    {
        if (SwView* pView = dynamic_cast<SwView *>(pOldSh))
        {
            pVS = pView->GetWrtShellPtr();
            // save the current ViewData of the previous SwView
            pOldSh->WriteUserData( m_sSwViewData );
        }
        else
            pVS = GetDocShell()->GetWrtShell();
        if( pVS )
        {
            // Set the current page as the first.
            sal_uInt16 nPhysPg, nVirtPg;
            static_cast<SwCursorShell*>(pVS)->GetPageNum( nPhysPg, nVirtPg, true, false );
            if( 1 != m_pViewWin->GetCol() && 1 == nPhysPg )
                --nPhysPg;
            m_pViewWin->SetSttPage( nPhysPg );
        }
    }

    // for form shell remember design mode of draw view
    // of previous view shell
    if ( pVS && pVS->HasDrawView() )
    {
        mbResetFormDesignMode = true;
        mbFormDesignModeToReset = pVS->GetDrawView()->IsDesignMode();
    }

    if( pVS )
        pNew = new SwViewShell( *pVS, m_pViewWin, nullptr, VSHELLFLAG_ISPREVIEW );
    else
        pNew = new SwViewShell(
                *static_cast<SwDocShell*>(pViewFrame->GetObjectShell())->GetDoc(),
                m_pViewWin, nullptr, nullptr, VSHELLFLAG_ISPREVIEW );

    m_pViewWin->SetViewShell( pNew );
    pNew->SetSfxViewShell( this );
    Init();
}

SwPagePreview::~SwPagePreview()
{
    SetWindow( nullptr );
    SwViewShell* pVShell =  m_pViewWin->GetViewShell();
    pVShell->SetWin(nullptr);
    delete pVShell;

    m_pViewWin.disposeAndClear();
    if (SfxViewFrame* pCurrent = SfxViewFrame::Current())
        if (auto& pBar = pCurrent->GetWindow().GetSystemWindow()->GetNotebookBar())
            pBar->ControlListenerForCurrentController(true); // start listening now
    m_pScrollFill.disposeAndClear();
    m_pHScrollbar.disposeAndClear();
    m_pVScrollbar.disposeAndClear();
}

SwDocShell* SwPagePreview::GetDocShell()
{
    return dynamic_cast<SwDocShell*>( GetViewFrame()->GetObjectShell() );
}

void SwPagePreview::CreateScrollbar( bool bHori )
{
    vcl::Window *pMDI = &GetViewFrame()->GetWindow();
    VclPtr<SwScrollbar>& ppScrollbar = bHori ? m_pHScrollbar : m_pVScrollbar;

    assert(!ppScrollbar); //check beforehand!

    ppScrollbar = VclPtr<SwScrollbar>::Create( pMDI, bHori );

    ScrollDocSzChg();
    ppScrollbar->EnableDrag();
    ppScrollbar->SetEndScrollHdl( LINK( this, SwPagePreview, EndScrollHdl ));

    ppScrollbar->SetScrollHdl( LINK( this, SwPagePreview, ScrollHdl ));

    InvalidateBorder();
    ppScrollbar->ExtendedShow();
}

bool SwPagePreview::ChgPage( int eMvMode, bool bUpdateScrollbar )
{
    tools::Rectangle aPixVisArea( m_pViewWin->LogicToPixel( m_aVisArea ) );
    bool bChg = m_pViewWin->MovePage( eMvMode ) ||
               eMvMode == SwPagePreviewWin::MV_CALC ||
               eMvMode == SwPagePreviewWin::MV_NEWWINSIZE;
    m_aVisArea = m_pViewWin->PixelToLogic( aPixVisArea );

    if( bChg )
    {
        // Update statusbar
        SfxBindings& rBindings = GetViewFrame()->GetBindings();

        if( bUpdateScrollbar )
        {
            ScrollViewSzChg();

            static sal_uInt16 aInval[] =
            {
                FN_START_OF_DOCUMENT, FN_END_OF_DOCUMENT,
                FN_PAGEUP, FN_PAGEDOWN, 0
            };
            rBindings.Invalidate( aInval );
        }
        std::vector<OUString> aStringList
        {
            m_sPageStr + m_pViewWin->GetStatusStr(mnPageCount),
            OUString()
        };
        rBindings.SetState(SfxStringListItem(FN_STAT_PAGE, &aStringList));
    }
    return bChg;
}

// From here, everything was taken from the SwView.
void SwPagePreview::CalcAndSetBorderPixel( SvBorder &rToFill )
{
    const StyleSettings &rSet = m_pViewWin->GetSettings().GetStyleSettings();
    const tools::Long nTmp = rSet.GetScrollBarSize();
    if ( m_pVScrollbar->IsVisible( true ) )
        rToFill.Right()  = nTmp;
    if ( m_pHScrollbar->IsVisible( true ) )
        rToFill.Bottom() = nTmp;
    SetBorderPixel( rToFill );
}

void  SwPagePreview::InnerResizePixel( const Point &rOfst, const Size &rSize, bool )
{
    SvBorder aBorder;
    CalcAndSetBorderPixel( aBorder );
    tools::Rectangle aRect( rOfst, rSize );
    aRect += aBorder;
    ViewResizePixel( *m_pViewWin->GetOutDev(), aRect.TopLeft(), aRect.GetSize(),
                    m_pViewWin->GetOutputSizePixel(),
                    *m_pVScrollbar, *m_pHScrollbar, *m_pScrollFill );

    // Never set EditWin !
    // Never set VisArea !
}

void SwPagePreview::OuterResizePixel( const Point &rOfst, const Size &rSize )
{
    SvBorder aBorder;
    CalcAndSetBorderPixel( aBorder );

    // Never set EditWin !

    Size aTmpSize( m_pViewWin->GetOutputSizePixel() );
    Point aBottomRight( m_pViewWin->PixelToLogic( Point( aTmpSize.Width(), aTmpSize.Height() ) ) );
    SetVisArea( tools::Rectangle( Point(), aBottomRight ) );

    // Call of the DocSzChgd-Method of the scrollbars is necessary,
    // because from the maximum scroll range half the height of the
    // VisArea is always deducted.
    if ( m_pVScrollbar && !aTmpSize.IsEmpty() )
    {
        ScrollDocSzChg();
    }

    SvBorder aBorderNew;
    CalcAndSetBorderPixel( aBorderNew );
    ViewResizePixel( *m_pViewWin->GetOutDev(), rOfst, rSize, m_pViewWin->GetOutputSizePixel(),
                    *m_pVScrollbar, *m_pHScrollbar, *m_pScrollFill );
}

void SwPagePreview::SetVisArea( const tools::Rectangle &rRect )
{
    const Point aTopLeft(AlignToPixel(rRect.TopLeft()));
    const Point aBottomRight(AlignToPixel(rRect.BottomRight()));
    tools::Rectangle aLR(aTopLeft,aBottomRight);

    if(aLR == m_aVisArea)
        return;
        // No negative position, no negative size

    if(aLR.Top() < 0)
    {
        aLR.AdjustBottom(std::abs(aLR.Top()) );
        aLR.SetTop( 0 );
    }

    if(aLR.Left() < 0)
    {
        aLR.AdjustRight(std::abs(aLR.Left()) );
        aLR.SetLeft( 0 );
    }
    if(aLR.Right() < 0) aLR.SetRight( 0 );
    if(aLR.Bottom() < 0) aLR.SetBottom( 0 );
    if(aLR == m_aVisArea ||
        // Ignore empty rectangle
        ( 0 == aLR.Bottom() - aLR.Top() && 0 == aLR.Right() - aLR.Left() ) )
        return;

    if( aLR.Left() > aLR.Right() || aLR.Top() > aLR.Bottom() )
        return;

    // Before the data can be changed call an update if necessary.
    // Thereby ensured, that adjacent paints are correctly converted into
    // document coordinates.
    // As a precaution, we do this only when at the shell runs an action,
    // because then we do not really paint but the rectangles are just
    // bookmarked (in document coordinates).
    if( GetViewShell()->ActionPend() )
        m_pViewWin->PaintImmediately();

    // Set at View-Win the current size
    m_aVisArea = aLR;
    m_pViewWin->SetWinSize( aLR.GetSize() );
    ChgPage( SwPagePreviewWin::MV_NEWWINSIZE );

    m_pViewWin->Invalidate();
}

IMPL_LINK( SwPagePreview, ScrollHdl, ScrollBar *, p, void )
{
    SwScrollbar* pScrollbar = static_cast<SwScrollbar*>(p);
    if(!GetViewShell())
        return;
    if( !pScrollbar->IsHoriScroll() &&
        pScrollbar->GetType() == ScrollType::Drag &&
        Help::IsQuickHelpEnabled() &&
        GetViewShell()->PagePreviewLayout()->DoesPreviewLayoutRowsFitIntoWindow())
    {
        // Scroll how many pages??
        OUString sStateStr(m_sPageStr);
        tools::Long nThmbPos = pScrollbar->GetThumbPos();
        if( 1 == m_pViewWin->GetCol() || !nThmbPos )
            ++nThmbPos;
        sStateStr += OUString::number( nThmbPos );
        Point aPos = pScrollbar->GetParent()->OutputToScreenPixel(
                                        pScrollbar->GetPosPixel());
        aPos.setY( pScrollbar->OutputToScreenPixel(pScrollbar->GetPointerPosPixel()).Y() );
        tools::Rectangle aRect;
        aRect.SetLeft( aPos.X() -8 );
        aRect.SetRight( aRect.Left() );
        aRect.SetTop( aPos.Y() );
        aRect.SetBottom( aRect.Top() );

        Help::ShowQuickHelp(pScrollbar, aRect, sStateStr,
                QuickHelpFlags::Right|QuickHelpFlags::VCenter);

    }
    else
        EndScrollHdl( pScrollbar );
}

IMPL_LINK( SwPagePreview, EndScrollHdl, ScrollBar *, p, void )
{
    SwScrollbar* pScrollbar = static_cast<SwScrollbar*>(p);
    if(!GetViewShell())
        return;

    // boolean to avoid unnecessary invalidation of the window.
    bool bInvalidateWin = true;

    if( !pScrollbar->IsHoriScroll() )       // scroll vertically
    {
        if ( Help::IsQuickHelpEnabled() )
            Help::ShowQuickHelp(pScrollbar, tools::Rectangle(), OUString());
        if ( GetViewShell()->PagePreviewLayout()->DoesPreviewLayoutRowsFitIntoWindow() )
        {
            // Scroll how many pages ??
            const sal_uInt16 nThmbPos = o3tl::narrowing<sal_uInt16>(pScrollbar->GetThumbPos());
            // adjust to new preview functionality
            if( nThmbPos != m_pViewWin->SelectedPage() )
            {
                // consider case that page <nThmbPos>
                // is already visible
                SwPagePreviewLayout* pPagePreviewLay = GetViewShell()->PagePreviewLayout();
                if ( pPagePreviewLay->IsPageVisible( nThmbPos ) )
                {
                    pPagePreviewLay->MarkNewSelectedPage( nThmbPos );
                    // invalidation of window is unnecessary
                    bInvalidateWin = false;
                }
                else
                {
                    // consider whether layout columns
                    // fit or not.
                    if ( !pPagePreviewLay->DoesPreviewLayoutColsFitIntoWindow() )
                    {
                        m_pViewWin->SetSttPage( nThmbPos );
                        m_pViewWin->SetSelectedPage( nThmbPos );
                        ChgPage( SwPagePreviewWin::MV_SCROLL, false );
                        // update scrollbars
                        ScrollViewSzChg();
                    }
                    else
                    {
                        // correct scroll amount
                        const sal_Int16 nPageDiff = nThmbPos - m_pViewWin->SelectedPage();
                        const sal_uInt16 nVisPages = m_pViewWin->GetRow() * m_pViewWin->GetCol();
                        sal_Int16 nWinPagesToScroll = nPageDiff / nVisPages;
                        if ( nPageDiff % nVisPages )
                        {
                            // decrease/increase number of preview pages to scroll
                            nPageDiff < 0 ? --nWinPagesToScroll : ++nWinPagesToScroll;
                        }
                        m_pViewWin->SetSelectedPage( nThmbPos );
                        m_pViewWin->Scroll( 0, pPagePreviewLay->GetWinPagesScrollAmount( nWinPagesToScroll ) );
                    }
                }
                // update accessibility
                GetViewShell()->ShowPreviewSelection( nThmbPos );
            }
            else
            {
                // invalidation of window is unnecessary
                bInvalidateWin = false;
            }
        }
        else
        {
            tools::Long nThmbPos = pScrollbar->GetThumbPos();
            m_pViewWin->Scroll(0, nThmbPos - m_pViewWin->GetPaintedPreviewDocRect().Top());
        }
    }
    else
    {
        tools::Long nThmbPos = pScrollbar->GetThumbPos();
        m_pViewWin->Scroll(nThmbPos - m_pViewWin->GetPaintedPreviewDocRect().Left(), 0);
    }
    // additional invalidate page status.
    static sal_uInt16 aInval[] =
    {
        FN_START_OF_DOCUMENT, FN_END_OF_DOCUMENT, FN_PAGEUP, FN_PAGEDOWN,
        FN_STAT_PAGE, 0
    };
    SfxBindings& rBindings = GetViewFrame()->GetBindings();
    rBindings.Invalidate( aInval );
    // control invalidation of window
    if ( bInvalidateWin )
    {
        m_pViewWin->Invalidate();
    }
}

Point SwPagePreview::AlignToPixel(const Point &rPt) const
{
    return m_pViewWin->PixelToLogic( m_pViewWin->LogicToPixel( rPt ) );
}

void SwPagePreview::DocSzChgd( const Size &rSz )
{
    if( m_aDocSize == rSz )
        return;

    m_aDocSize = rSz;

    // #i96726#
    // Due to the multiple page layout it is needed to trigger recalculation
    // of the page preview layout, even if the count of pages is not changing.
    mnPageCount = GetViewShell()->GetNumPages();

    if( m_aVisArea.GetWidth() )
    {
        ChgPage( SwPagePreviewWin::MV_CALC );
        ScrollDocSzChg();

        m_pViewWin->Invalidate();
    }
}

void SwPagePreview::ScrollViewSzChg()
{
    if(!GetViewShell())
        return ;

    bool bShowVScrollbar = false, bShowHScrollbar = false;

    if(m_pVScrollbar)
    {
        if(GetViewShell()->PagePreviewLayout()->DoesPreviewLayoutRowsFitIntoWindow())
        {
            //vertical scrolling by row
            // adjust to new preview functionality
            const sal_uInt16 nVisPages = m_pViewWin->GetRow() * m_pViewWin->GetCol();

            m_pVScrollbar->SetVisibleSize( nVisPages );
            // set selected page as scroll bar position,
            // if it is visible.
            SwPagePreviewLayout* pPagePreviewLay = GetViewShell()->PagePreviewLayout();
            if ( pPagePreviewLay->IsPageVisible( m_pViewWin->SelectedPage() ) )
            {
                m_pVScrollbar->SetThumbPos( m_pViewWin->SelectedPage() );
            }
            else
            {
                m_pVScrollbar->SetThumbPos( m_pViewWin->GetSttPage() );
            }
            m_pVScrollbar->SetLineSize( m_pViewWin->GetCol() );
            m_pVScrollbar->SetPageSize( nVisPages );
            // calculate and set scrollbar range
            Range aScrollbarRange( 1, mnPageCount );
            // increase range by one, because left-top-corner is left blank.
            ++aScrollbarRange.Max();
            // increase range in order to access all pages
            aScrollbarRange.Max() += ( nVisPages - 1 );
            m_pVScrollbar->SetRange( aScrollbarRange );

            bShowVScrollbar = nVisPages < mnPageCount;
        }
        else //vertical scrolling by pixel
        {
            const tools::Rectangle& rDocRect = m_pViewWin->GetPaintedPreviewDocRect();
            const Size& rPreviewSize =
                    GetViewShell()->PagePreviewLayout()->GetPreviewDocSize();
            m_pVScrollbar->SetRangeMax(rPreviewSize.Height()) ;
            tools::Long nVisHeight = rDocRect.GetHeight();
            m_pVScrollbar->SetVisibleSize( nVisHeight );
            m_pVScrollbar->SetThumbPos( rDocRect.Top() );
            m_pVScrollbar->SetLineSize( nVisHeight / 10 );
            m_pVScrollbar->SetPageSize( nVisHeight / 2 );

            bShowVScrollbar = true;
        }

        if (!mbVScrollbarEnabled)
            bShowVScrollbar = false;

        ShowVScrollbar(bShowVScrollbar);
    }
    if(m_pHScrollbar)
    {
        const tools::Rectangle& rDocRect = m_pViewWin->GetPaintedPreviewDocRect();
        const Size& rPreviewSize =
                GetViewShell()->PagePreviewLayout()->GetPreviewDocSize();
        Range aRange(0,0);

        if(rDocRect.GetWidth() < rPreviewSize.Width())
        {
            bShowHScrollbar = true;

            tools::Long nVisWidth = rDocRect.GetWidth();
            tools::Long nThumb = rDocRect.Left();
            aRange = Range(0, rPreviewSize.Width());

            m_pHScrollbar->SetRange( aRange );
            m_pHScrollbar->SetVisibleSize( nVisWidth );
            m_pHScrollbar->SetThumbPos( nThumb );
            m_pHScrollbar->SetLineSize( nVisWidth / 10 );
            m_pHScrollbar->SetPageSize( nVisWidth / 2 );
        }

        if (!mbHScrollbarEnabled)
            bShowHScrollbar = false;

        ShowHScrollbar(bShowHScrollbar);
    }
    m_pScrollFill->Show(bShowVScrollbar && bShowHScrollbar);
}

void SwPagePreview::ScrollDocSzChg()
{
    ScrollViewSzChg();
}

// All about printing
SfxPrinter*  SwPagePreview::GetPrinter( bool bCreate )
{
    return m_pViewWin->GetViewShell()->getIDocumentDeviceAccess().getPrinter( bCreate );
}

sal_uInt16  SwPagePreview::SetPrinter( SfxPrinter *pNew, SfxPrinterChangeFlags nDiffFlags )
{
    SwViewShell &rSh = *GetViewShell();
    SfxPrinter* pOld = rSh.getIDocumentDeviceAccess().getPrinter( false );
    if ( pOld && pOld->IsPrinting() )
        return SFX_PRINTERROR_BUSY;

    SwEditShell &rESh = static_cast<SwEditShell&>(rSh);  //Buh...
    if( ( SfxPrinterChangeFlags::PRINTER | SfxPrinterChangeFlags::JOBSETUP ) & nDiffFlags )
    {
        rSh.getIDocumentDeviceAccess().setPrinter( pNew, true, true );
        if( nDiffFlags & SfxPrinterChangeFlags::PRINTER )
            rESh.SetModified();
    }
    if ( ( nDiffFlags & SfxPrinterChangeFlags::OPTIONS ) == SfxPrinterChangeFlags::OPTIONS )
        ::SetPrinter( &rSh.getIDocumentDeviceAccess(), pNew, false );

    const bool bChgOri  = bool(nDiffFlags & SfxPrinterChangeFlags::CHG_ORIENTATION);
    const bool bChgSize = bool(nDiffFlags & SfxPrinterChangeFlags::CHG_SIZE);
    if ( bChgOri || bChgSize )
    {
        rESh.StartAllAction();
        if ( bChgOri )
            rSh.ChgAllPageOrientation( pNew->GetOrientation() );
        if ( bChgSize )
        {
            Size aSz( SvxPaperInfo::GetPaperSize( pNew ) );
            rSh.ChgAllPageSize( aSz );
        }
        if( !m_bNormalPrint )
            m_pViewWin->CalcWish( m_pViewWin->GetRow(), m_pViewWin->GetCol() );
        rESh.SetModified();
        rESh.EndAllAction();

        static sal_uInt16 aInval[] =
        {
            SID_ATTR_LONG_ULSPACE, SID_ATTR_LONG_LRSPACE,
            SID_RULER_BORDERS, SID_RULER_PAGE_POS, 0
        };
#if OSL_DEBUG_LEVEL > 0
        {
            const sal_uInt16* pPtr = aInval + 1;
            do {
                OSL_ENSURE( *(pPtr - 1) < *pPtr, "wrong sorting!" );
            } while( *++pPtr );
        }
#endif

        GetViewFrame()->GetBindings().Invalidate(aInval);
    }

    return 0;
}

bool SwPagePreview::HasPrintOptionsPage() const
{
    return true;
}

std::unique_ptr<SfxTabPage> SwPagePreview::CreatePrintOptionsPage(weld::Container* pPage, weld::DialogController* pController,
                                                         const SfxItemSet &rOptions)
{
    return ::CreatePrintOptionsPage(pPage, pController, rOptions, !m_bNormalPrint);
}

void SwPagePreviewWin::SetViewShell( SwViewShell* pShell )
{
    mpViewShell = pShell;
    if ( mpViewShell && mpViewShell->IsPreview() )
    {
        mpPgPreviewLayout = mpViewShell->PagePreviewLayout();
    }
}

void SwPagePreviewWin::RepaintCoreRect( const SwRect& rRect )
{
    // #i24183#
    if ( mpPgPreviewLayout->PreviewLayoutValid() )
    {
        mpPgPreviewLayout->Repaint( tools::Rectangle( rRect.Pos(), rRect.SSize() ) );
    }
}

/** method to adjust preview to a new zoom factor

    #i19975# also consider zoom type - adding parameter <_eZoomType>
*/
void SwPagePreviewWin::AdjustPreviewToNewZoom( const sal_uInt16 _nZoomFactor,
                                               const SvxZoomType _eZoomType )
{
    // #i19975# consider zoom type
    if ( _eZoomType == SvxZoomType::WHOLEPAGE )
    {
        mnRow = 1;
        mnCol = 1;
        mpPgPreviewLayout->Init( mnCol, mnRow, maPxWinSize );
        mpPgPreviewLayout->Prepare( mnSttPage, Point(0,0), maPxWinSize,
                                  mnSttPage, maPaintedPreviewDocRect );
        SetSelectedPage( mnSttPage );
        SetPagePreview(mnRow, mnCol);
        maScale = GetMapMode().GetScaleX();
    }
    else if ( _nZoomFactor != 0 )
    {
        // calculate new scaling and set mapping mode appropriately.
        Fraction aNewScale( _nZoomFactor, 100 );
        MapMode aNewMapMode = GetMapMode();
        aNewMapMode.SetScaleX( aNewScale );
        aNewMapMode.SetScaleY( aNewScale );
        SetMapMode( aNewMapMode );

        // calculate new start position for preview paint
        Size aNewWinSize = PixelToLogic( maPxWinSize );
        Point aNewPaintStartPos =
                mpPgPreviewLayout->GetPreviewStartPosForNewScale( aNewScale, maScale, aNewWinSize );

        // remember new scaling and prepare preview paint
        // Note: paint of preview will be performed by a corresponding invalidate
        //          due to property changes.
        maScale = aNewScale;
        mpPgPreviewLayout->Prepare( 0, aNewPaintStartPos, maPxWinSize,
                                  mnSttPage, maPaintedPreviewDocRect );
    }

}

/**
 * pixel scrolling - horizontally always or vertically
 * when less than the desired number of rows fits into
 * the view
 */
void SwPagePreviewWin::Scroll(tools::Long nXMove, tools::Long nYMove, ScrollFlags /*nFlags*/)
{
    maPaintedPreviewDocRect.Move(nXMove, nYMove);
    mpPgPreviewLayout->Prepare( 0, maPaintedPreviewDocRect.TopLeft(),
                              maPxWinSize, mnSttPage,
                              maPaintedPreviewDocRect );

}

bool SwPagePreview::HandleWheelCommands( const CommandEvent& rCEvt )
{
    bool bOk = false;
    const CommandWheelData* pWData = rCEvt.GetWheelData();
    if( pWData && CommandWheelMode::ZOOM == pWData->GetMode() )
    {
        //only the Preference shouldn't control the Zoom, it is better to detect AT tools running. So the bridge can be used here
        if (!Application::GetSettings().GetMiscSettings().GetEnableATToolSupport())
        {
            sal_uInt16 nFactor = GetViewShell()->GetViewOptions()->GetZoom();
            const sal_uInt16 nOffset = 10;
            if( 0L > pWData->GetDelta() )
            {
                nFactor -= nOffset;
                if(nFactor < MIN_PREVIEW_ZOOM)
                    nFactor = MIN_PREVIEW_ZOOM;
            }
            else
            {
                nFactor += nOffset;
                if(nFactor > MAX_PREVIEW_ZOOM)
                    nFactor = MAX_PREVIEW_ZOOM;
            }
            SetZoom(SvxZoomType::PERCENT, nFactor);
        }
        bOk = true;
    }
    else
        bOk = m_pViewWin->HandleScrollCommand( rCEvt, m_pHScrollbar, m_pVScrollbar );
    return bOk;
}

uno::Reference< css::accessibility::XAccessible >
    SwPagePreviewWin::CreateAccessible()
{
    SolarMutexGuard aGuard; // this should have happened already!!!
#ifndef ENABLE_WASM_STRIP_ACCESSIBILITY
    OSL_ENSURE( GetViewShell() != nullptr, "We need a view shell" );
    css::uno::Reference< css::accessibility::XAccessible > xAcc = GetAccessible( false );
    if (xAcc.is())
    {
        return xAcc;
    }
    if (mpViewShell)
    {
        css::uno::Reference< css::accessibility::XAccessible > xAccPreview = mpViewShell->CreateAccessiblePreview();
        SetAccessible(xAccPreview);
    }
#endif
    return GetAccessible( false );
}

void SwPagePreview::ApplyAccessibilityOptions(SvtAccessibilityOptions const & rAccessibilityOptions)
{
    GetViewShell()->ApplyAccessibilityOptions(rAccessibilityOptions);
}

void SwPagePreview::ShowHScrollbar(bool bShow)
{
    m_pHScrollbar->Show(bShow);
    InvalidateBorder();
}

void SwPagePreview::ShowVScrollbar(bool bShow)
{
    m_pVScrollbar->Show(bShow);
    InvalidateBorder();
}

void SwPagePreview::EnableHScrollbar(bool bEnable)
{
    if (mbHScrollbarEnabled != bEnable)
    {
        mbHScrollbarEnabled = bEnable;
        ScrollViewSzChg();
    }
}

void SwPagePreview::EnableVScrollbar(bool bEnable)
{
    if (mbVScrollbarEnabled != bEnable)
    {
        mbVScrollbarEnabled = bEnable;
        ScrollViewSzChg();
    }
}

void SwPagePreview::SetZoom(SvxZoomType eType, sal_uInt16 nFactor)
{
    SwViewShell& rSh = *GetViewShell();
    SwViewOption aOpt(*rSh.GetViewOptions());
    // perform action only on changes of zoom or zoom type.
    if ( aOpt.GetZoom() != nFactor ||
         aOpt.GetZoomType() != eType )
    {
        aOpt.SetZoom(nFactor);
        aOpt.SetZoomType(eType);
        rSh.ApplyViewOptions( aOpt );
        lcl_InvalidateZoomSlots(GetViewFrame()->GetBindings());
        // #i19975# also consider zoom type
        m_pViewWin->AdjustPreviewToNewZoom( nFactor, eType );
        ScrollViewSzChg();
    }
}

/** adjust position of vertical scrollbar */
void SwPagePreview::SetVScrollbarThumbPos( const sal_uInt16 _nNewThumbPos )
{
    if ( m_pVScrollbar )
    {
        m_pVScrollbar->SetThumbPos( _nNewThumbPos );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
