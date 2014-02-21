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


#include <sfx2/objface.hxx>
#include <vcl/timer.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/help.hxx>
#include <vcl/cmdevt.hxx>
#include <vcl/button.hxx>
#include <vcl/settings.hxx>

#include <svl/whiter.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/progress.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/msgbox.hxx>
#include <svx/stddlg.hxx>
#include <editeng/paperinf.hxx>
#include <svl/srchitem.hxx>
#include <svx/svdview.hxx>
#include <svx/dlgutil.hxx>
#include <svx/zoomslideritem.hxx>
#include <svx/svxids.hrc>

#include <swwait.hxx>
#include <globdoc.hxx>
#include <wdocsh.hxx>
#include <pvprtdat.hxx>
#include <swmodule.hxx>
#include <modcfg.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <viewopt.hxx>
#include <doc.hxx>
#include <pview.hxx>
#include <view.hxx>
#include <textsh.hxx>
#include <scroll.hxx>
#include <prtopt.hxx>
#include <docstat.hxx>
#include <usrpref.hxx>
#include <viewfunc.hxx>

#include <helpid.h>
#include <cmdid.h>
#include <globals.hrc>
#include <popup.hrc>
#include <view.hrc>

#define SwPagePreview
#include <sfx2/msg.hxx>
#include <swslots.hxx>
#include <pagepreviewlayout.hxx>

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <osl/mutex.hxx>

#include <boost/scoped_ptr.hpp>

using namespace ::com::sun::star;
SFX_IMPL_NAMED_VIEWFACTORY(SwPagePreview, "PrintPreview")
{
    SFX_VIEW_REGISTRATION(SwDocShell);
    SFX_VIEW_REGISTRATION(SwWebDocShell);
    SFX_VIEW_REGISTRATION(SwGlobalDocShell);
}

SFX_IMPL_INTERFACE(SwPagePreview, SfxViewShell, SW_RES(RID_PVIEW_TOOLBOX))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_PPREVIEW_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT|SFX_VISIBILITY_STANDARD|
                                SFX_VISIBILITY_CLIENT|SFX_VISIBILITY_FULLSCREEN|
                                SFX_VISIBILITY_READONLYDOC,
                                SW_RES(RID_PVIEW_TOOLBOX));
}

TYPEINIT1(SwPagePreview,SfxViewShell)

#define SWVIEWFLAGS ( SFX_VIEW_CAN_PRINT|SFX_VIEW_HAS_PRINTOPTIONS )

#define MIN_PREVIEW_ZOOM 25
#define MAX_PREVIEW_ZOOM 600

static sal_uInt16 lcl_GetNextZoomStep(sal_uInt16 nCurrentZoom, sal_Bool bZoomIn)
{
    static const sal_uInt16 aZoomArr[] =
    {
        25, 50, 75, 100, 150, 200, 400, 600
    };
    const sal_uInt16 nZoomArrSize = sizeof(aZoomArr)/sizeof(sal_uInt16);
    if(bZoomIn)
        for(int i = nZoomArrSize - 1; i >= 0; --i)
        {
            if(nCurrentZoom > aZoomArr[i] || !i)
                return aZoomArr[i];
        }
    else
        for(int i = 0; i < nZoomArrSize; ++i)
        {
            if(nCurrentZoom < aZoomArr[i])
                return aZoomArr[i];
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

// At first the zoom dialog
class SwPreviewZoomDlg : public SvxStandardDialog
{
    NumericField* m_pRowEdit;
    NumericField* m_pColEdit;

    virtual void  Apply();

public:
    SwPreviewZoomDlg( SwPagePreviewWin& rParent );
};

SwPreviewZoomDlg::SwPreviewZoomDlg( SwPagePreviewWin& rParent )
    : SvxStandardDialog(&rParent, "PreviewZoomDialog", "modules/swriter/ui/previewzoomdialog.ui")
{
    get(m_pRowEdit, "rows");
    get(m_pColEdit, "cols");

    m_pRowEdit->SetValue( rParent.GetRow() );
    m_pColEdit->SetValue( rParent.GetCol() );
}

void  SwPreviewZoomDlg::Apply()
{
    ((SwPagePreviewWin*)GetParent())->CalcWish(
                sal_uInt8(m_pRowEdit->GetValue()),
                sal_uInt8(m_pColEdit->GetValue()) );
}

// all for SwPagePreviewWin
SwPagePreviewWin::SwPagePreviewWin( Window *pParent, SwPagePreview& rPView )
    : Window(pParent, WinBits(WB_CLIPCHILDREN))
    , mpViewShell(0)
    , mrView(rPView)
    , mbCalcScaleForPreviewLayout(true)
    , maPaintedPreviewDocRect(Rectangle(0,0,0,0))
    , mpPgPreviewLayout(NULL)
{
    SetOutDevViewType( OUTDEV_VIEWTYPE_PRINTPREVIEW );
    SetHelpId(HID_PAGEPREVIEW);
    SetFillColor( GetBackground().GetColor() );
    SetLineColor( GetBackground().GetColor());
    SetMapMode( MapMode(MAP_TWIP) );

    const SwMasterUsrPref *pUsrPref = SW_MOD()->GetUsrPref(sal_False);
    mnRow = pUsrPref->GetPagePrevRow();     // 1 row
    mnCol = pUsrPref->GetPagePrevCol();     // 1 column
    mnSttPage = USHRT_MAX;
}

SwPagePreviewWin::~SwPagePreviewWin()
{
}

void  SwPagePreviewWin::Paint( const Rectangle& rRect )
{
    if( !mpViewShell || !mpViewShell->GetLayout() )
        return;

    if( USHRT_MAX == mnSttPage )        // was never calculated ? (Init-Phase!)
    {
        // This is the size to which I always relate.
        if( !maPxWinSize.Height() || !maPxWinSize.Width() )
            maPxWinSize = GetOutputSizePixel();

        Rectangle aRect( LogicToPixel( rRect ));
        mpPgPreviewLayout->Prepare( 1, Point(0,0), maPxWinSize,
                                  mnSttPage, maPaintedPreviewDocRect );
        SetSelectedPage( 1 );
        mpPgPreviewLayout->Paint( PixelToLogic( aRect ) );
        SetPagePreview(mnRow, mnCol);
    }
    else
    {
        MapMode aMM( GetMapMode() );
        aMM.SetScaleX( maScale );
        aMM.SetScaleY( maScale );
        SetMapMode( aMM );
        mpPgPreviewLayout->Paint( rRect );
    }
}

void SwPagePreviewWin::CalcWish( sal_uInt8 nNewRow, sal_uInt8 nNewCol )
{
    if( !mpViewShell || !mpViewShell->GetLayout() )
        return;

    sal_uInt16 nOldCol = mnCol;
    mnRow = nNewRow;
    mnCol = nNewCol;
    sal_uInt16 nPages = mnRow * mnCol,
           nLastSttPg = mrView.GetPageCount()+1 > nPages
                            ? mrView.GetPageCount()+1 - nPages : 0;
    if( mnSttPage > nLastSttPg )
        mnSttPage = nLastSttPg;

    mpPgPreviewLayout->Init( mnCol, mnRow, maPxWinSize, true );
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
int SwPagePreviewWin::MovePage( int eMoveMode )
{
    // number of pages up
    sal_uInt16 nPages = mnRow * mnCol;
    sal_uInt16 nNewSttPage = mnSttPage;
    sal_uInt16 nPageCount = mrView.GetPageCount();
    sal_uInt16 nDefSttPg = GetDefSttPage();
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
        nNewSttPage = nNewAbsSttPage < nPageCount ? nNewAbsSttPage : nPageCount;

        const sal_uInt16 nRelSelPage = mpPgPreviewLayout->ConvertAbsoluteToRelativePageNum( SelectedPage() );
        const sal_uInt16 nNewAbsSelPage = mpPgPreviewLayout->ConvertRelativeToAbsolutePageNum( nRelSelPage + nPages );
        SetSelectedPage( nNewAbsSelPage < nPageCount ? nNewAbsSelPage : nPageCount );

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
        return sal_False;

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

    return sal_True;
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
        mpPgPreviewLayout->Init( mnCol, mnRow, maPxWinSize, true );
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
    sal_uInt16 nPageNum;
    if ( mpPgPreviewLayout->IsPageVisible( mpPgPreviewLayout->SelectedPage() ) )
    {
        nPageNum = mpPgPreviewLayout->SelectedPage();
    }
    else
    {
        nPageNum = mnSttPage > 1 ? mnSttPage : 1;
    }
    OUStringBuffer aStatusStr;
    sal_uInt16 nVirtPageNum = mpPgPreviewLayout->GetVirtPageNumByPageNum( nPageNum );
    if( nVirtPageNum && nVirtPageNum != nPageNum )
    {
        aStatusStr.append( OUString::number(nVirtPageNum) + " " );
    }
    aStatusStr.append( OUString::number(nPageNum) + " / " + OUString::number(nPageCnt) );
    return aStatusStr.makeStringAndClear();
}

void  SwPagePreviewWin::KeyInput( const KeyEvent &rKEvt )
{
    const KeyCode& rKeyCode = rKEvt.GetKeyCode();
    sal_uInt16 nKey = rKeyCode.GetCode();
    bool bHandled = false;
    if(!rKeyCode.GetModifier())
    {
        sal_uInt16 nSlot = 0;
        switch(nKey)
        {
            case KEY_ADD : nSlot = SID_ZOOM_OUT;         break;
            case KEY_ESCAPE: nSlot = FN_CLOSE_PAGEPREVIEW; break;
            case KEY_SUBTRACT : nSlot = SID_ZOOM_IN;    break;
        }
        if(nSlot)
        {
            bHandled = true;
            mrView.GetViewFrame()->GetDispatcher()->Execute(
                                nSlot, SFX_CALLMODE_ASYNCHRON );
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
        case COMMAND_CONTEXTMENU:
            mrView.GetViewFrame()->GetDispatcher()->ExecutePopup();
            bCallBase = false;
        break;

        case COMMAND_WHEEL:
        case COMMAND_STARTAUTOSCROLL:
        case COMMAND_AUTOSCROLL:
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
    if( MOUSE_LEFT == ( rMEvt.GetModifier() + rMEvt.GetButtons() ) )
    {
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
            OUString sNewCrsrPos = OUString::number( aDocPos.X() ) + ";" +
                                   OUString::number( aDocPos.Y() ) + ";";
            mrView.SetNewCrsrPos( sNewCrsrPos );

            SfxViewFrame *pTmpFrm = mrView.GetViewFrame();
            pTmpFrm->GetBindings().Execute( SID_VIEWSHELL0, NULL, 0,
                                                    SFX_CALLMODE_ASYNCHRON );
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
}

// Set user prefs or view options

void SwPagePreviewWin::SetPagePreview( sal_uInt8 nRow, sal_uInt8 nCol )
{
    SwMasterUsrPref *pOpt = (SwMasterUsrPref *)SW_MOD()->GetUsrPref(sal_False);

    if (nRow != pOpt->GetPagePrevRow() || nCol != pOpt->GetPagePrevCol())
    {
        pOpt->SetPagePrevRow( nRow );
        pOpt->SetPagePrevCol( nCol );
        pOpt->SetModified();

        // Update scrollbar!
        mrView.ScrollViewSzChg();
    }
}

/** get selected page in document preview

    @author OD
*/
sal_uInt16 SwPagePreviewWin::SelectedPage() const
{
    return mpPgPreviewLayout->SelectedPage();
}

/** set selected page number in document preview

    @author OD
*/
void SwPagePreviewWin::SetSelectedPage( sal_uInt16 _nSelectedPageNum )
{
    mpPgPreviewLayout->SetSelectedPage( _nSelectedPageNum );
}

/** method to enable/disable book preview

    @author OD
*/
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
    case DATACHANGED_SETTINGS:
        // Rearrange the scrollbars or trigger resize, because the
        // size of the scrollbars may have be changed. Also the
        // size of the scrollbars has to be retrieved from the settings
        // out of the resize handler.
        if( rDCEvt.GetFlags() & SETTINGS_STYLE )
            mrView.InvalidateBorder();              // Scrollbar widths
        // zoom has to be disabled if Accessibility support is switched on
        lcl_InvalidateZoomSlots(mrView.GetViewFrame()->GetBindings());
        break;

    case DATACHANGED_PRINTER:
    case DATACHANGED_DISPLAY:
    case DATACHANGED_FONTS:
    case DATACHANGED_FONTSUBSTITUTION:
        mrView.GetDocShell()->UpdateFontList(); // Font change
        if ( mpViewShell->GetWin() )
            mpViewShell->GetWin()->Invalidate();
        break;
    }
}

/** help method to execute SfxRequest FN_PAGEUP and FN_PAGEDOWN

    @author OD
*/
void SwPagePreview::_ExecPgUpAndPgDown( const bool  _bPgUp,
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
            if ( ChgPage( eMvMode, sal_True ) )
                pViewWin->Invalidate();
        }
        else
        {
            SwTwips nScrollAmount;
            sal_uInt16 nNewSelectedPageNum = 0;
            const sal_uInt16 nVisPages = pViewWin->GetRow() * pViewWin->GetCol();
            if( _bPgUp )
            {
                if ( pPagePreviewLay->DoesPreviewLayoutRowsFitIntoWindow() )
                {
                    nScrollAmount = pPagePreviewLay->GetWinPagesScrollAmount( -1 );
                    if ( (pViewWin->SelectedPage() - nVisPages) > 0 )
                        nNewSelectedPageNum = pViewWin->SelectedPage() - nVisPages;
                    else
                        nNewSelectedPageNum = 1;
                }
                else
                    nScrollAmount = - std::min( pViewWin->GetOutputSize().Height(),
                                           pViewWin->GetPaintedPreviewDocRect().Top() );
            }
            else
            {
                if ( pPagePreviewLay->DoesPreviewLayoutRowsFitIntoWindow() )
                {
                    nScrollAmount = pPagePreviewLay->GetWinPagesScrollAmount( 1 );
                    if ( (pViewWin->SelectedPage() + nVisPages) <= mnPageCount )
                        nNewSelectedPageNum = pViewWin->SelectedPage() + nVisPages;
                    else
                        nNewSelectedPageNum = mnPageCount;
                }
                else
                    nScrollAmount = std::min( pViewWin->GetOutputSize().Height(),
                                         ( pPagePreviewLay->GetPreviewDocSize().Height() -
                                           pViewWin->GetPaintedPreviewDocRect().Bottom() ) );
            }
            pViewWin->Scroll( 0, nScrollAmount );
            if ( nNewSelectedPageNum != 0 )
            {
                pViewWin->SetSelectedPage( nNewSelectedPageNum );
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
            pViewWin->Invalidate();
        }
    }

    if ( _pReq )
        _pReq->Done();
}

// Then all for the SwPagePreview
void  SwPagePreview::Execute( SfxRequest &rReq )
{
    int eMvMode;
    sal_uInt8 nRow = 1;
    bool bRetVal = false;
    bool bRefresh = true;

    switch(rReq.GetSlot())
    {
        case FN_REFRESH_VIEW:
        case FN_STAT_PAGE:
        case FN_STAT_ZOOM:
            break;

        case FN_SHOW_MULTIPLE_PAGES:
        {
            const SfxItemSet *pArgs = rReq.GetArgs();
            if( pArgs && pArgs->Count() >= 2 )
            {
                sal_uInt8 nCols = (sal_uInt8)((SfxUInt16Item &)pArgs->Get(
                                        SID_ATTR_TABLE_COLUMN)).GetValue();
                sal_uInt8 nRows = (sal_uInt8)((SfxUInt16Item &)pArgs->Get(
                                        SID_ATTR_TABLE_ROW)).GetValue();
                pViewWin->CalcWish( nRows, nCols );

            }
            else
                SwPreviewZoomDlg( *pViewWin ).Execute();

        }
        break;
        case FN_SHOW_BOOKVIEW:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();
            const SfxPoolItem* pItem;
            bool bBookPreview = GetViewShell()->GetViewOptions()->IsPagePrevBookview();
            if( pArgs && SFX_ITEM_SET == pArgs->GetItemState( FN_SHOW_BOOKVIEW, false, &pItem ) )
            {
                bBookPreview = static_cast< const SfxBoolItem* >( pItem )->GetValue();
                ( ( SwViewOption* ) GetViewShell()->GetViewOptions() )->SetPagePrevBookview( bBookPreview );
                    // cast is not gentleman like, but it's common use in writer and in this case
            }
            if ( pViewWin->SetBookPreviewMode( bBookPreview ) )
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
                pViewWin->Invalidate();
            }

        }
        break;
        case FN_SHOW_TWO_PAGES:
            pViewWin->CalcWish( nRow, 2 );
            break;

        case FN_PREVIEW_ZOOM:
        case SID_ATTR_ZOOM:
        {
            const SfxItemSet *pArgs = rReq.GetArgs();
            const SfxPoolItem* pItem;
            boost::scoped_ptr<AbstractSvxZoomDialog> pDlg;
            if(!pArgs)
            {
                SfxItemSet aCoreSet(GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM);
                const SwViewOption* pVOpt = GetViewShell()->GetViewOptions();
                SvxZoomItem aZoom( (SvxZoomType)pVOpt->GetZoomType(),
                                            pVOpt->GetZoom() );
                aZoom.SetValueSet(
                        SVX_ZOOM_ENABLE_50|
                        SVX_ZOOM_ENABLE_75|
                        SVX_ZOOM_ENABLE_100|
                        SVX_ZOOM_ENABLE_150|
                        SVX_ZOOM_ENABLE_200|
                        SVX_ZOOM_ENABLE_WHOLEPAGE);
                aCoreSet.Put( aZoom );

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if(pFact)
                {
                    pDlg.reset(pFact->CreateSvxZoomDialog(&GetViewFrame()->GetWindow(), aCoreSet));
                    OSL_ENSURE(pDlg, "Dialogdiet fail!");
                }

                pDlg->SetLimits( MINZOOM, MAXZOOM );

                if( pDlg->Execute() != RET_CANCEL )
                    pArgs = pDlg->GetOutputItemSet();
            }
            if( pArgs )
            {
                enum SvxZoomType eType = SVX_ZOOM_PERCENT;
                sal_uInt16 nZoomFactor = USHRT_MAX;
                if(SFX_ITEM_SET == pArgs->GetItemState(SID_ATTR_ZOOM, true, &pItem))
                {
                    eType = ((const SvxZoomItem *)pItem)->GetType();
                    nZoomFactor = ((const SvxZoomItem *)pItem)->GetValue();
                }
                else if(SFX_ITEM_SET == pArgs->GetItemState(FN_PREVIEW_ZOOM, true, &pItem))
                    nZoomFactor = ((const SfxUInt16Item *)pItem)->GetValue();
                if(USHRT_MAX != nZoomFactor)
                    SetZoom(eType, nZoomFactor);
            }
        }
        break;
        case SID_ATTR_ZOOMSLIDER :
        {
            const SfxItemSet *pArgs = rReq.GetArgs();
            const SfxPoolItem* pItem;

            if ( pArgs && SFX_ITEM_SET == pArgs->GetItemState(SID_ATTR_ZOOMSLIDER, true, &pItem ) )
            {
                const sal_uInt16 nCurrentZoom = ((const SvxZoomSliderItem *)pItem)->GetValue();
                SetZoom( SVX_ZOOM_PERCENT, nCurrentZoom );
            }
        }
        break;
        case SID_ZOOM_IN:
        case SID_ZOOM_OUT:
        {
            enum SvxZoomType eType = SVX_ZOOM_PERCENT;
            const SwViewOption* pVOpt = GetViewShell()->GetViewOptions();
            SetZoom(eType,
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
            if ( pViewWin->SelectedPage() != nNewSelectedPage )
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
                    pViewWin->SetSelectedPage( nNewSelectedPage );
                    pViewWin->SetSttPage( nNewStartPage );
                    bRefresh = ChgPage( SwPagePreviewWin::MV_SELPAGE, sal_True );
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
            _ExecPgUpAndPgDown( rReq.GetSlot() == FN_PAGEUP, &rReq );
            break;
        }
        case FN_START_OF_LINE:
        case FN_START_OF_DOCUMENT:
            pViewWin->SetSelectedPage( 1 );
            eMvMode = SwPagePreviewWin::MV_DOC_STT; bRetVal = true; goto MOVEPAGE;
        case FN_END_OF_LINE:
        case FN_END_OF_DOCUMENT:
            pViewWin->SetSelectedPage( mnPageCount );
            eMvMode = SwPagePreviewWin::MV_DOC_END; bRetVal = true; goto MOVEPAGE;
MOVEPAGE:
            {
                bool nRet = ChgPage( eMvMode, sal_True );
                // return value fuer Basic
                if(bRetVal)
                    rReq.SetReturnValue(SfxBoolItem(rReq.GetSlot(), !nRet));

                bRefresh = nRet;
                rReq.Done();
            }
            break;

        case FN_PRINT_PAGEPREVIEW:
        {
            const SwPagePreviewPrtData* pPPVPD = pViewWin->GetViewShell()->GetDoc()->GetPreviewPrtData();
            // The thing with the orientation
            if(pPPVPD)
            {
                SfxPrinter* pPrinter = GetPrinter( sal_True );
                if((pPrinter->GetOrientation() == ORIENTATION_LANDSCAPE)
                        != pPPVPD->GetLandscape())
                    pPrinter->SetOrientation(pPPVPD->GetLandscape() ? ORIENTATION_LANDSCAPE : ORIENTATION_PORTRAIT);
            }
            ::SetAppPrintOptions( pViewWin->GetViewShell(), sal_False );
            bNormalPrint = sal_False;
            sal_uInt16 nPrtSlot = SID_PRINTDOC;
            rReq.SetSlot( nPrtSlot );
            SfxViewShell::ExecuteSlot( rReq, SfxViewShell::GetInterface() );
            rReq.SetSlot( FN_PRINT_PAGEPREVIEW );
            return;
        }
        case SID_PRINTDOCDIRECT:
        case SID_PRINTDOC:
            ::SetAppPrintOptions( pViewWin->GetViewShell(), sal_False );
            bNormalPrint = sal_True;
            SfxViewShell::ExecuteSlot( rReq, SfxViewShell::GetInterface() );
            return;
        case FN_CLOSE_PAGEPREVIEW:
        case SID_PRINTPREVIEW:
            //  print preview is now always in the same frame as the tab view
            //  -> always switch this frame back to normal view
            //  (ScTabViewShell ctor reads stored view data)
            GetViewFrame()->GetDispatcher()->Execute( SID_VIEWSHELL0, 0, 0, SFX_CALLMODE_ASYNCHRON );
            break;
        case FN_INSERT_BREAK:
        {
            sal_uInt16 nSelPage = pViewWin->SelectedPage();
            //if a dummy page is selected (e.g. a non-existing right/left page)
            //the direct neighbor is used
            if(GetViewShell()->IsDummyPage( nSelPage ) && GetViewShell()->IsDummyPage( --nSelPage ))
                nSelPage +=2;
            SetNewPage( nSelPage );
            SfxViewFrame *pTmpFrm = GetViewFrame();
            pTmpFrm->GetBindings().Execute( SID_VIEWSHELL0, NULL, 0,
                                                    SFX_CALLMODE_ASYNCHRON );
        }
        break;
        default:
            OSL_ENSURE(!this, "wrong dispatcher");
            return;
    }

    if( bRefresh )
        pViewWin->Invalidate();
}

void  SwPagePreview::GetState( SfxItemSet& rSet )
{
    SfxWhichIter aIter(rSet);
    sal_uInt8 nRow = 1;
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
                OUString aStr = sPageStr + pViewWin->GetStatusStr( mnPageCount );
                rSet.Put( SfxStringItem( nWhich, aStr) );
            }
            break;

        case SID_ATTR_ZOOM:
        case FN_STAT_ZOOM:
            {
                    const SwViewOption* pVOpt = GetViewShell()->GetViewOptions();
                    SvxZoomItem aZoom((SvxZoomType)pVOpt->GetZoomType(),
                                        pVOpt->GetZoom());
                    aZoom.SetValueSet(
                            SVX_ZOOM_ENABLE_50|
                            SVX_ZOOM_ENABLE_75|
                            SVX_ZOOM_ENABLE_100|
                            SVX_ZOOM_ENABLE_150|
                            SVX_ZOOM_ENABLE_200);
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
            if((SID_ZOOM_OUT == nWhich && pVOpt->GetZoom() >= MAX_PREVIEW_ZOOM)||
              (SID_ZOOM_IN == nWhich && pVOpt->GetZoom() <= MIN_PREVIEW_ZOOM))
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
            sal_Bool b = GetViewShell()->GetViewOptions()->IsPagePrevBookview();
            rSet.Put(SfxBoolItem(nWhich, b));
        }
        break;

        case FN_SHOW_TWO_PAGES:
            if( 2 == pViewWin->GetCol() && nRow == pViewWin->GetRow() )
                rSet.DisableItem( nWhich );
            break;

        case FN_PRINT_PAGEPREVIEW:
            // has the same status like the normal printing
            {
                const SfxPoolItem* pItem;
                SfxItemSet aSet( *rSet.GetPool(), SID_PRINTDOC, SID_PRINTDOC );
                GetSlotState( SID_PRINTDOC, SfxViewShell::GetInterface(), &aSet );
                if( SFX_ITEM_DISABLED == aSet.GetItemState( SID_PRINTDOC,
                        false, &pItem ))
                    rSet.DisableItem( nWhich );
                else if( SFX_ITEM_SET == aSet.GetItemState( SID_PRINTDOC,
                        false, &pItem ))
                {
                    ((SfxPoolItem*)pItem)->SetWhich( FN_PRINT_PAGEPREVIEW );
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

void SwPagePreview::Init(const SwViewOption * pPrefs)
{
    if ( GetViewShell()->HasDrawView() )
        GetViewShell()->GetDrawView()->SetAnimationEnabled( sal_False );

    bNormalPrint = sal_True;

    // Check and process the DocSize. The shell could not be found via
    // the handler, because the shell is unknown to the SFX management
    // within the CTOR phase.

    if( !pPrefs )
        pPrefs = SW_MOD()->GetUsrPref(sal_False);

    mbHScrollbarEnabled = pPrefs->IsViewHScrollBar();
    mbVScrollbarEnabled = pPrefs->IsViewVScrollBar();

    // Update the fields
    // ATTENTION: Do cast the EditShell up, to use the SS.
    //            At the methodes the current shell will be queried!
    SwEditShell* pESh = (SwEditShell*)GetViewShell();
    sal_Bool bIsModified = pESh->IsModified();


    SwViewOption aOpt( *pPrefs );
    aOpt.SetPagePreview(sal_True);
    aOpt.SetTab( sal_False );
    aOpt.SetBlank( sal_False );
    aOpt.SetHardBlank( sal_False );
    aOpt.SetParagraph( sal_False );
    aOpt.SetLineBreak( sal_False );
    aOpt.SetPageBreak( sal_False );
    aOpt.SetColumnBreak( sal_False );
    aOpt.SetSoftHyph( sal_False );
    aOpt.SetFldName( sal_False );
    aOpt.SetPostIts( sal_False );
    aOpt.SetShowHiddenChar( sal_False );
    aOpt.SetShowHiddenField( sal_False );
    aOpt.SetShowHiddenPara( sal_False );
    aOpt.SetViewHRuler( sal_False );
    aOpt.SetViewVRuler( sal_False );
    aOpt.SetGraphic( sal_True );
    aOpt.SetTable( sal_True );
    aOpt.SetSnap( sal_False );
    aOpt.SetGridVisible( sal_False );

    GetViewShell()->ApplyViewOptions( aOpt );
    GetViewShell()->ApplyAccessiblityOptions(SW_MOD()->GetAccessibilityOptions());

    // adjust view shell option to the same as for print
    SwPrintData const aPrintOptions = *SW_MOD()->GetPrtOptions(false);
    GetViewShell()->AdjustOptionsForPagePreview( aPrintOptions );

    GetViewShell()->CalcLayout();
    DocSzChgd( GetViewShell()->GetDocSize() );

    if( !bIsModified )
        pESh->ResetModified();
}

SwPagePreview::SwPagePreview(SfxViewFrame *pViewFrame, SfxViewShell* pOldSh):
    SfxViewShell( pViewFrame, SWVIEWFLAGS ),
    pViewWin( new SwPagePreviewWin(&(GetViewFrame())->GetWindow(), *this ) ),
    nNewPage(USHRT_MAX),
    sPageStr(SW_RES(STR_PAGE)),
    pHScrollbar(0),
    pVScrollbar(0),
    pScrollFill(new ScrollBarBox( &pViewFrame->GetWindow(),
        pViewFrame->GetFrame().GetParentFrame() ? 0 : WB_SIZEABLE )),
    mnPageCount( 0 ),
    mbResetFormDesignMode( false ),
    mbFormDesignModeToReset( false )
{
    SetName(OUString("PageView" ));
    SetWindow( pViewWin );
    SetHelpId(SW_PAGEPREVIEW);
    _CreateScrollbar( sal_True );
    _CreateScrollbar( sal_False );

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

    if( pOldSh && pOldSh->IsA( TYPE( SwPagePreview ) ) )
        pVS = ((SwPagePreview*)pOldSh)->GetViewShell();
    else
    {
        if( pOldSh && pOldSh->IsA( TYPE( SwView ) ) )
        {
            pVS = ((SwView*)pOldSh)->GetWrtShellPtr();
            // save the current ViewData of the previous SwView
            pOldSh->WriteUserData( sSwViewData, false );
        }
        else
            pVS = GetDocShell()->GetWrtShell();
        if( pVS )
        {
            // Set the current page as the first.
            sal_uInt16 nPhysPg, nVirtPg;
            ((SwCrsrShell*)pVS)->GetPageNum( nPhysPg, nVirtPg, sal_True, sal_False );
            if( 1 != pViewWin->GetCol() && 1 == nPhysPg )
                --nPhysPg;
            pViewWin->SetSttPage( nPhysPg );
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
        pNew = new SwViewShell( *pVS, pViewWin, 0, VSHELLFLAG_ISPREVIEW );
    else
        pNew = new SwViewShell(
                *((SwDocShell*)pViewFrame->GetObjectShell())->GetDoc(),
                pViewWin, 0, 0, VSHELLFLAG_ISPREVIEW );

    pViewWin->SetViewShell( pNew );
    pNew->SetSfxViewShell( this );
    Init();
}

SwPagePreview::~SwPagePreview()
{
    SetWindow( 0 );
    SwViewShell* pVShell =  pViewWin->GetViewShell();
    pVShell->SetWin(0);
    delete pVShell;
    delete pViewWin;

    delete pScrollFill;
    delete pHScrollbar;
    delete pVScrollbar;
}

SwDocShell* SwPagePreview::GetDocShell()
{
    return PTR_CAST(SwDocShell, GetViewFrame()->GetObjectShell());
}

int SwPagePreview::_CreateScrollbar( sal_Bool bHori )
{
    Window *pMDI = &GetViewFrame()->GetWindow();
    SwScrollbar** ppScrollbar = bHori ? &pHScrollbar : &pVScrollbar;

    OSL_ENSURE( !*ppScrollbar, "check beforehand!" );

    *ppScrollbar = new SwScrollbar( pMDI, bHori );

    ScrollDocSzChg();
    (*ppScrollbar)->EnableDrag( true );
    (*ppScrollbar)->SetEndScrollHdl( LINK( this, SwPagePreview, EndScrollHdl ));

    (*ppScrollbar)->SetScrollHdl( LINK( this, SwPagePreview, ScrollHdl ));

    InvalidateBorder();
    (*ppScrollbar)->ExtendedShow();
    return 1;
}

bool SwPagePreview::ChgPage( int eMvMode, int bUpdateScrollbar )
{
    Rectangle aPixVisArea( pViewWin->LogicToPixel( aVisArea ) );
    bool bChg = pViewWin->MovePage( eMvMode ) ||
               eMvMode == SwPagePreviewWin::MV_CALC ||
               eMvMode == SwPagePreviewWin::MV_NEWWINSIZE;
    aVisArea = pViewWin->PixelToLogic( aPixVisArea );

    if( bChg )
    {
        // Update statusbar
        OUString aStr = sPageStr + pViewWin->GetStatusStr( mnPageCount );
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
        rBindings.SetState( SfxStringItem( FN_STAT_PAGE, aStr ) );
    }
    return bChg;
}

// From here, everything was taken from the SwView.
void SwPagePreview::CalcAndSetBorderPixel( SvBorder &rToFill, sal_Bool /*bInner*/ )
{
    const StyleSettings &rSet = pViewWin->GetSettings().GetStyleSettings();
    const long nTmp = rSet.GetScrollBarSize();
    if ( pVScrollbar->IsVisible( true ) )
        rToFill.Right()  = nTmp;
    if ( pHScrollbar->IsVisible( true ) )
        rToFill.Bottom() = nTmp;
    SetBorderPixel( rToFill );
}

void  SwPagePreview::InnerResizePixel( const Point &rOfst, const Size &rSize )
{
    SvBorder aBorder;
    CalcAndSetBorderPixel( aBorder, sal_True );
    Rectangle aRect( rOfst, rSize );
    aRect += aBorder;
    ViewResizePixel( *pViewWin, aRect.TopLeft(), aRect.GetSize(),
                    pViewWin->GetOutputSizePixel(),
                    *pVScrollbar, *pHScrollbar, *pScrollFill );

    // Never set EditWin !
    // Never set VisArea !
}

void SwPagePreview::OuterResizePixel( const Point &rOfst, const Size &rSize )
{
    SvBorder aBorder;
    CalcAndSetBorderPixel( aBorder, sal_False );

    // Never set EditWin !

    Size aTmpSize( pViewWin->GetOutputSizePixel() );
    Point aBottomRight( pViewWin->PixelToLogic( Point( aTmpSize.Width(), aTmpSize.Height() ) ) );
    SetVisArea( Rectangle( Point(), aBottomRight ) );

    // Call of the DocSzChgd-Methode of the scrollbars is necessary,
    // because from the maximum scoll range half the height of the
    // VisArea is always deducted.
    if ( pVScrollbar && aTmpSize.Width() > 0 && aTmpSize.Height() > 0 )
    {
        ScrollDocSzChg();
    }

    SvBorder aBorderNew;
    CalcAndSetBorderPixel( aBorderNew, sal_False );
    ViewResizePixel( *pViewWin, rOfst, rSize, pViewWin->GetOutputSizePixel(),
                    *pVScrollbar, *pHScrollbar, *pScrollFill );
}

void SwPagePreview::SetVisArea( const Rectangle &rRect, sal_Bool bUpdateScrollbar )
{
    const Point aTopLeft(AlignToPixel(rRect.TopLeft()));
    const Point aBottomRight(AlignToPixel(rRect.BottomRight()));
    Rectangle aLR(aTopLeft,aBottomRight);

    if(aLR == aVisArea)
        return;
        // No negative position, no negative size

    if(aLR.Top() < 0)
    {
        aLR.Bottom() += std::abs(aLR.Top());
        aLR.Top() = 0;
    }

    if(aLR.Left() < 0)
    {
        aLR.Right() += std::abs(aLR.Left());
        aLR.Left() = 0;
    }
    if(aLR.Right() < 0) aLR.Right() = 0;
    if(aLR.Bottom() < 0) aLR.Bottom() = 0;
    if(aLR == aVisArea ||
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
        pViewWin->Update();

    // Set at View-Win the current size
    aVisArea = aLR;
    pViewWin->SetWinSize( aLR.GetSize() );
    ChgPage( SwPagePreviewWin::MV_NEWWINSIZE, bUpdateScrollbar );

    pViewWin->Invalidate();
}

IMPL_LINK( SwPagePreview, ScrollHdl, SwScrollbar *, pScrollbar )
{
    if(!GetViewShell())
        return 0;
    if( !pScrollbar->IsHoriScroll() &&
        pScrollbar->GetType() == SCROLL_DRAG &&
        Help::IsQuickHelpEnabled() &&
        GetViewShell()->PagePreviewLayout()->DoesPreviewLayoutRowsFitIntoWindow())
    {
        // Scroll how many pages??
        OUString sStateStr(sPageStr);
        sal_uInt16 nThmbPos = (sal_uInt16)pScrollbar->GetThumbPos();
        if( 1 == pViewWin->GetCol() || !nThmbPos )
            ++nThmbPos;
        sStateStr += OUString::number( nThmbPos );
        Point aPos = pScrollbar->GetParent()->OutputToScreenPixel(
                                        pScrollbar->GetPosPixel());
        aPos.Y() = pScrollbar->OutputToScreenPixel(pScrollbar->GetPointerPosPixel()).Y();
        Rectangle aRect;
        aRect.Left()    = aPos.X() -8;
        aRect.Right()   = aRect.Left();
        aRect.Top()     = aPos.Y();
        aRect.Bottom()  = aRect.Top();

        Help::ShowQuickHelp(pScrollbar, aRect, sStateStr,
                QUICKHELP_RIGHT|QUICKHELP_VCENTER);

    }
    else
        EndScrollHdl( pScrollbar );
    return 0;
}

IMPL_LINK( SwPagePreview, EndScrollHdl, SwScrollbar *, pScrollbar )
{
    if(!GetViewShell())
        return 0;

    // boolean to avoid unnecessary invalidation of the window.
    bool bInvalidateWin = true;

    if( !pScrollbar->IsHoriScroll() )       // scroll vertically
    {
        if ( Help::IsQuickHelpEnabled() )
            Help::ShowQuickHelp(pScrollbar, Rectangle(), OUString(), 0);
        if ( GetViewShell()->PagePreviewLayout()->DoesPreviewLayoutRowsFitIntoWindow() )
        {
            // Scroll how many pages ??
            sal_uInt16 nThmbPos = (sal_uInt16)pScrollbar->GetThumbPos();
            // adjust to new preview functionality
            if( nThmbPos != pViewWin->SelectedPage() )
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
                        pViewWin->SetSttPage( nThmbPos );
                        pViewWin->SetSelectedPage( nThmbPos );
                        ChgPage( SwPagePreviewWin::MV_SCROLL, sal_False );
                        // update scrollbars
                        ScrollViewSzChg();
                    }
                    else
                    {
                        // correct scroll amount
                        const sal_Int16 nPageDiff = nThmbPos - pViewWin->SelectedPage();
                        const sal_uInt16 nVisPages = pViewWin->GetRow() * pViewWin->GetCol();
                        sal_Int16 nWinPagesToScroll = nPageDiff / nVisPages;
                        if ( nPageDiff % nVisPages )
                        {
                            // decrease/increase number of preview pages to scroll
                            nPageDiff < 0 ? --nWinPagesToScroll : ++nWinPagesToScroll;
                        }
                        pViewWin->SetSelectedPage( nThmbPos );
                        pViewWin->Scroll( 0, pPagePreviewLay->GetWinPagesScrollAmount( nWinPagesToScroll ) );
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
            long nThmbPos = pScrollbar->GetThumbPos();
            pViewWin->Scroll(0, nThmbPos - pViewWin->GetPaintedPreviewDocRect().Top());
        }
    }
    else
    {
        long nThmbPos = pScrollbar->GetThumbPos();
        pViewWin->Scroll(nThmbPos - pViewWin->GetPaintedPreviewDocRect().Left(), 0);
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
        pViewWin->Invalidate();
    }
    return 0;
}

Point SwPagePreview::AlignToPixel(const Point &rPt) const
{
    return pViewWin->PixelToLogic( pViewWin->LogicToPixel( rPt ) );
}

void SwPagePreview::DocSzChgd( const Size &rSz )
{
    if( aDocSz == rSz )
        return;

    aDocSz = rSz;

    // #i96726#
    // Due to the multiple page layout it is needed to trigger recalculation
    // of the page preview layout, even if the count of pages is not changing.
    mnPageCount = GetViewShell()->GetNumPages();

    if( aVisArea.GetWidth() )
    {
        ChgPage( SwPagePreviewWin::MV_CALC, sal_True );
        ScrollDocSzChg();

        pViewWin->Invalidate();
    }
}

void SwPagePreview::ScrollViewSzChg()
{
    if(!GetViewShell())
        return ;

    bool bShowVScrollbar = false, bShowHScrollbar = false;

    if(pVScrollbar)
    {
        if(GetViewShell()->PagePreviewLayout()->DoesPreviewLayoutRowsFitIntoWindow())
        {
            //vertical scrolling by row
            // adjust to new preview functionality
            sal_uInt16 nVisPages = pViewWin->GetRow() * pViewWin->GetCol();

            pVScrollbar->SetVisibleSize( nVisPages );
            // set selected page as scroll bar position,
            // if it is visible.
            SwPagePreviewLayout* pPagePreviewLay = GetViewShell()->PagePreviewLayout();
            if ( pPagePreviewLay->IsPageVisible( pViewWin->SelectedPage() ) )
            {
                pVScrollbar->SetThumbPos( pViewWin->SelectedPage() );
            }
            else
            {
                pVScrollbar->SetThumbPos( pViewWin->GetSttPage() );
            }
            pVScrollbar->SetLineSize( pViewWin->GetCol() );
            pVScrollbar->SetPageSize( nVisPages );
            // calculate and set scrollbar range
            Range aScrollbarRange( 1, mnPageCount );
            // increase range by one, because left-top-corner is left blank.
            ++aScrollbarRange.Max();
            // increase range in order to access all pages
            aScrollbarRange.Max() += ( nVisPages - 1 );
            pVScrollbar->SetRange( aScrollbarRange );

            bShowVScrollbar = nVisPages < mnPageCount;
        }
        else //vertical scrolling by pixel
        {
            const Rectangle& rDocRect = pViewWin->GetPaintedPreviewDocRect();
            const Size& rPreviewSize =
                    GetViewShell()->PagePreviewLayout()->GetPreviewDocSize();
            pVScrollbar->SetRangeMax(rPreviewSize.Height()) ;
            long nVisHeight = rDocRect.GetHeight();
            pVScrollbar->SetVisibleSize( nVisHeight );
            pVScrollbar->SetThumbPos( rDocRect.Top() );
            pVScrollbar->SetLineSize( nVisHeight / 10 );
            pVScrollbar->SetPageSize( nVisHeight / 2 );

            bShowVScrollbar = true;
        }

        if (!mbVScrollbarEnabled)
            bShowVScrollbar = false;

        ShowVScrollbar(bShowVScrollbar);
    }
    if(pHScrollbar)
    {
        const Rectangle& rDocRect = pViewWin->GetPaintedPreviewDocRect();
        const Size& rPreviewSize =
                GetViewShell()->PagePreviewLayout()->GetPreviewDocSize();
        long nVisWidth = 0;
        long nThumb   = 0;
        Range aRange(0,0);

        if(rDocRect.GetWidth() < rPreviewSize.Width())
        {
            bShowHScrollbar = true;

            nVisWidth = rDocRect.GetWidth();
            nThumb = rDocRect.Left();
            aRange = Range(0, rPreviewSize.Width());

            pHScrollbar->SetRange( aRange );
            pHScrollbar->SetVisibleSize( nVisWidth );
            pHScrollbar->SetThumbPos( nThumb );
            pHScrollbar->SetLineSize( nVisWidth / 10 );
            pHScrollbar->SetPageSize( nVisWidth / 2 );
        }

        if (!mbHScrollbarEnabled)
            bShowHScrollbar = false;

        ShowHScrollbar(bShowHScrollbar);
    }
    pScrollFill->Show(bShowVScrollbar && bShowHScrollbar);
}

void SwPagePreview::ScrollDocSzChg()
{
    ScrollViewSzChg();
}

// All about printing
SfxPrinter*  SwPagePreview::GetPrinter( sal_Bool bCreate )
{
    return pViewWin->GetViewShell()->getIDocumentDeviceAccess()->getPrinter( bCreate );
}

sal_uInt16  SwPagePreview::SetPrinter( SfxPrinter *pNew, sal_uInt16 nDiffFlags, bool )
{
    SwViewShell &rSh = *GetViewShell();
    SfxPrinter* pOld = rSh.getIDocumentDeviceAccess()->getPrinter( false );
    if ( pOld && pOld->IsPrinting() )
        return SFX_PRINTERROR_BUSY;

    SwEditShell &rESh = (SwEditShell&)rSh;  //Buh...
    if( ( SFX_PRINTER_PRINTER | SFX_PRINTER_JOBSETUP ) & nDiffFlags )
    {
        rSh.getIDocumentDeviceAccess()->setPrinter( pNew, true, true );
        if( nDiffFlags & SFX_PRINTER_PRINTER )
            rESh.SetModified();
    }
    if ( ( nDiffFlags & SFX_PRINTER_OPTIONS ) == SFX_PRINTER_OPTIONS )
        ::SetPrinter( rSh.getIDocumentDeviceAccess(), pNew, sal_False );

    const bool bChgOri = nDiffFlags & SFX_PRINTER_CHG_ORIENTATION;
    const bool bChgSize= nDiffFlags & SFX_PRINTER_CHG_SIZE;
    if ( bChgOri || bChgSize )
    {
        rESh.StartAllAction();
        if ( bChgOri )
            rSh.ChgAllPageOrientation( sal_uInt16(pNew->GetOrientation()) );
        if ( bChgSize )
        {
            Size aSz( SvxPaperInfo::GetPaperSize( pNew ) );
            rSh.ChgAllPageSize( aSz );
        }
        if( !bNormalPrint )
            pViewWin->CalcWish( pViewWin->GetRow(), pViewWin->GetCol() );
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

SfxTabPage*  SwPagePreview::CreatePrintOptionsPage( Window *pParent,
                                                const SfxItemSet &rOptions )
{
    return ::CreatePrintOptionsPage( pParent, rOptions, !bNormalPrint );
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
        mpPgPreviewLayout->Repaint( Rectangle( rRect.Pos(), rRect.SSize() ) );
    }
}

/** method to adjust preview to a new zoom factor

    #i19975# also consider zoom type - adding parameter <_eZoomType>
*/
void SwPagePreviewWin::AdjustPreviewToNewZoom( const sal_uInt16 _nZoomFactor,
                                               const SvxZoomType _eZoomType )
{
    // #i19975# consider zoom type
    if ( _eZoomType == SVX_ZOOM_WHOLEPAGE )
    {
        mnRow = 1;
        mnCol = 1;
        mpPgPreviewLayout->Init( mnCol, mnRow, maPxWinSize, true );
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
void SwPagePreviewWin::Scroll(long nXMove, long nYMove, sal_uInt16 /*nFlags*/)
{
    maPaintedPreviewDocRect.Move(nXMove, nYMove);
    mpPgPreviewLayout->Prepare( 0, maPaintedPreviewDocRect.TopLeft(),
                              maPxWinSize, mnSttPage,
                              maPaintedPreviewDocRect );

}

sal_Bool SwPagePreview::HandleWheelCommands( const CommandEvent& rCEvt )
{
    sal_Bool bOk = sal_False;
    const CommandWheelData* pWData = rCEvt.GetWheelData();
    if( pWData && COMMAND_WHEEL_ZOOM == pWData->GetMode() )
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
            SetZoom(SVX_ZOOM_PERCENT, nFactor);
        }
        bOk = sal_True;
    }
    else
        bOk = pViewWin->HandleScrollCommand( rCEvt, pHScrollbar, pVScrollbar );
    return bOk;
}

uno::Reference< ::com::sun::star::accessibility::XAccessible >
    SwPagePreviewWin::CreateAccessible()
{
    SolarMutexGuard aGuard; // this should have happened already!!!

    OSL_ENSURE( GetViewShell() != NULL, "We need a view shell" );
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAcc = GetAccessible( false );
    if (xAcc.is())
    {
        return xAcc;
    }
    if (mpViewShell)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAccPreview = mpViewShell->CreateAccessiblePreview();
        SetAccessible(xAccPreview);
    }
    return GetAccessible( false );
}

// MT: Removed Windows::SwitchView() introduced with IA2 CWS.
// There are other notifications for this when the active view has chnaged, so please update the code to use that event mechanism
void SwPagePreviewWin::SwitchView()
{
#ifdef ACCESSIBLE_LAYOUT
    if (!Application::IsAccessibilityEnabled())
    {
        return ;
    }
    if (mpViewShell)
    {
        mpViewShell->InvalidateAccessibleFocus();
    }
#endif
}

void SwPagePreview::ApplyAccessiblityOptions(SvtAccessibilityOptions& rAccessibilityOptions)
{
    GetViewShell()->ApplyAccessiblityOptions(rAccessibilityOptions);
}

void SwPagePreview::ShowHScrollbar(sal_Bool bShow)
{
    pHScrollbar->Show(bShow);
    InvalidateBorder();
}

void SwPagePreview::ShowVScrollbar(sal_Bool bShow)
{
    pVScrollbar->Show(bShow);
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
        pViewWin->AdjustPreviewToNewZoom( nFactor, eType );
        ScrollViewSzChg();
    }
}

/** adjust position of vertical scrollbar

    @author OD
*/
void SwPagePreview::SetVScrollbarThumbPos( const sal_uInt16 _nNewThumbPos )
{
    if ( pVScrollbar )
    {
        pVScrollbar->SetThumbPos( _nNewThumbPos );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
