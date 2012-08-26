/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <sfx2/objface.hxx>
#include <vcl/timer.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/help.hxx>
#include <vcl/cmdevt.hxx>
#include <vcl/button.hxx>
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
#include <pview.hrc>

#define SwPagePreView
#include <sfx2/msg.hxx>
#include <swslots.hxx>
#include <pagepreviewlayout.hxx>

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <osl/mutex.hxx>


using namespace ::com::sun::star;
SFX_IMPL_NAMED_VIEWFACTORY(SwPagePreView, "PrintPreview")
{
    SFX_VIEW_REGISTRATION(SwDocShell);
    SFX_VIEW_REGISTRATION(SwWebDocShell);
    SFX_VIEW_REGISTRATION(SwGlobalDocShell);
}

SFX_IMPL_INTERFACE(SwPagePreView, SfxViewShell, SW_RES(RID_PVIEW_TOOLBOX))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_PPREVIEW_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT|SFX_VISIBILITY_STANDARD|
                                SFX_VISIBILITY_CLIENT|SFX_VISIBILITY_FULLSCREEN|
                                SFX_VISIBILITY_READONLYDOC,
                                SW_RES(RID_PVIEW_TOOLBOX));
}

TYPEINIT1(SwPagePreView,SfxViewShell)

#define SWVIEWFLAGS ( SFX_VIEW_CAN_PRINT|SFX_VIEW_HAS_PRINTOPTIONS )

#define MIN_PREVIEW_ZOOM 25
#define MAX_PREVIEW_ZOOM 600

sal_uInt16 lcl_GetNextZoomStep(sal_uInt16 nCurrentZoom, sal_Bool bZoomIn)
{
    static sal_uInt16 aZoomArr[] =
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

void lcl_InvalidateZoomSlots(SfxBindings& rBindings)
{
    static sal_uInt16 const aInval[] =
    {
        SID_ATTR_ZOOM, SID_ZOOM_OUT, SID_ZOOM_IN, SID_ATTR_ZOOMSLIDER, FN_PREVIEW_ZOOM, FN_STAT_ZOOM,
        0
    };
    rBindings.Invalidate( aInval );
}

// erstmal der Zoom-Dialog
class SwPreViewZoomDlg : public SvxStandardDialog
{
    FixedText       aRowLbl;
    NumericField    aRowEdit;
    FixedText       aColLbl;
    NumericField    aColEdit;

    OKButton        aOkBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;

    virtual void    Apply();

public:
    SwPreViewZoomDlg( SwPagePreViewWin& rParent );
    ~SwPreViewZoomDlg();
};

SwPreViewZoomDlg::SwPreViewZoomDlg( SwPagePreViewWin& rParent ) :
    SvxStandardDialog( &rParent, SW_RES(DLG_PAGEPREVIEW_ZOOM) ),
    aRowLbl(this,SW_RES(FT_ROW)),
    aRowEdit(this,SW_RES(ED_ROW)),
    aColLbl(this,SW_RES(FT_COL)),
    aColEdit(this,SW_RES(ED_COL)),
    aOkBtn(this,SW_RES(BT_OK)),
    aCancelBtn(this,SW_RES(BT_CANCEL)),
    aHelpBtn(this,SW_RES(BT_HELP))
{
    FreeResource();

    aRowEdit.SetValue( rParent.GetRow() );
    aColEdit.SetValue( rParent.GetCol() );
}

SwPreViewZoomDlg::~SwPreViewZoomDlg() {}
void  SwPreViewZoomDlg::Apply()
{
    ((SwPagePreViewWin*)GetParent())->CalcWish(
                sal_uInt8(aRowEdit.GetValue()),
                sal_uInt8(aColEdit.GetValue()) );
}

// alles fuers SwPagePreViewWin
SwPagePreViewWin::SwPagePreViewWin( Window *pParent, SwPagePreView& rPView )
    : Window( pParent, WinBits( WB_CLIPCHILDREN) ),
    mpViewShell( 0 ),
    mrView( rPView ),
    mbCalcScaleForPreviewLayout( true ),
    maPaintedPreviewDocRect( Rectangle(0,0,0,0) )
{
    SetOutDevViewType( OUTDEV_VIEWTYPE_PRINTPREVIEW );
    SetHelpId(HID_PAGEPREVIEW);
    SetFillColor( GetBackground().GetColor() );
    SetLineColor( GetBackground().GetColor());
    SetMapMode( MapMode(MAP_TWIP) );

    const SwMasterUsrPref *pUsrPref = SW_MOD()->GetUsrPref(sal_False);
    mnRow = pUsrPref->GetPagePrevRow();     // 1 Zeile
    mnCol = pUsrPref->GetPagePrevCol();     // 1 Spalte
    mnSttPage = USHRT_MAX;
}

SwPagePreViewWin::~SwPagePreViewWin()
{
    delete mpViewShell;
}

void  SwPagePreViewWin::Paint( const Rectangle& rRect )
{
    if( !mpViewShell || !mpViewShell->GetLayout() )
        return;

    if( USHRT_MAX == mnSttPage )        // wurde noch nie berechnet ? (Init-Phase!)
    {
        // das ist die Size, auf die ich mich immer beziehe
        if( !maPxWinSize.Height() || !maPxWinSize.Width() )
            maPxWinSize = GetOutputSizePixel();

        Rectangle aRect( LogicToPixel( rRect ));
        mpPgPrevwLayout->Prepare( 1, Point(0,0), maPxWinSize,
                                  mnSttPage, maPaintedPreviewDocRect );
        SetSelectedPage( 1 );
        mpPgPrevwLayout->Paint( PixelToLogic( aRect ) );
        SetPagePreview(mnRow, mnCol);
    }
    else
    {
        MapMode aMM( GetMapMode() );
        aMM.SetScaleX( maScale );
        aMM.SetScaleY( maScale );
        SetMapMode( aMM );
        mpPgPrevwLayout->Paint( rRect );
    }
}

void SwPagePreViewWin::CalcWish( sal_uInt8 nNewRow, sal_uInt8 nNewCol )
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

    mpPgPrevwLayout->Init( mnCol, mnRow, maPxWinSize, true );
    mpPgPrevwLayout->Prepare( mnSttPage, Point(0,0), maPxWinSize,
                              mnSttPage, maPaintedPreviewDocRect );
    SetSelectedPage( mnSttPage );
    SetPagePreview(mnRow, mnCol);
    maScale = GetMapMode().GetScaleX();

    // falls an der Spaltigkeit gedreht wurde, so muss der Sonderfall
    // Einspaltig beachtet und ggfs. der Scrollbar korrigiert werden
    if( (1 == nOldCol) ^ (1 == mnCol) )
        mrView.ScrollDocSzChg();

    // Sortierung muss eingehalten werden!!
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

/*--------------------------------------------------------------------
    Beschreibung:, mnSttPage is Absolute
 --------------------------------------------------------------------*/
int SwPagePreViewWin::MovePage( int eMoveMode )
{
    // soviele Seiten hoch
    sal_uInt16 nPages = mnRow * mnCol;
    sal_uInt16 nNewSttPage = mnSttPage;
    sal_uInt16 nPageCount = mrView.GetPageCount();
    sal_uInt16 nDefSttPg = GetDefSttPage();
    bool bPaintPageAtFirstCol = true;

    switch( eMoveMode )
    {
    case MV_PAGE_UP:
    {
        const sal_uInt16 nRelSttPage = mpPgPrevwLayout->ConvertAbsoluteToRelativePageNum( mnSttPage );
        const sal_uInt16 nNewAbsSttPage = nRelSttPage - nPages > 0 ?
                                          mpPgPrevwLayout->ConvertRelativeToAbsolutePageNum( nRelSttPage - nPages ) :
                                          nDefSttPg;
        nNewSttPage = nNewAbsSttPage;

        const sal_uInt16 nRelSelPage = mpPgPrevwLayout->ConvertAbsoluteToRelativePageNum( SelectedPage() );
        const sal_uInt16 nNewRelSelPage = nRelSelPage - nPages > 0 ?
                                          nRelSelPage - nPages :
                                          1;
        SetSelectedPage( mpPgPrevwLayout->ConvertRelativeToAbsolutePageNum( nNewRelSelPage ) );

        break;
    }
    case MV_PAGE_DOWN:
    {
        const sal_uInt16 nRelSttPage = mpPgPrevwLayout->ConvertAbsoluteToRelativePageNum( mnSttPage );
        const sal_uInt16 nNewAbsSttPage = mpPgPrevwLayout->ConvertRelativeToAbsolutePageNum( nRelSttPage + nPages );
        nNewSttPage = nNewAbsSttPage < nPageCount ? nNewAbsSttPage : nPageCount;

        const sal_uInt16 nRelSelPage = mpPgPrevwLayout->ConvertAbsoluteToRelativePageNum( SelectedPage() );
        const sal_uInt16 nNewAbsSelPage = mpPgPrevwLayout->ConvertRelativeToAbsolutePageNum( nRelSelPage + nPages );
        SetSelectedPage( nNewAbsSelPage < nPageCount ? nNewAbsSelPage : nPageCount );

        break;
    }
    case MV_DOC_STT:
        nNewSttPage = nDefSttPg;
        SetSelectedPage( mpPgPrevwLayout->ConvertRelativeToAbsolutePageNum( nNewSttPage ? nNewSttPage : 1 ) );
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
        if ( !mpPgPrevwLayout->DoesPreviewLayoutColsFitIntoWindow() )
            bPaintPageAtFirstCol = false;
        break;
    case MV_SCROLL:
        // check, if paint page at first column
        // has to be avoided
        if ( !mpPgPrevwLayout->DoesPreviewLayoutRowsFitIntoWindow() ||
             !mpPgPrevwLayout->DoesPreviewLayoutColsFitIntoWindow() )
            bPaintPageAtFirstCol = false;
        break;
    case MV_NEWWINSIZE:
        // nothing special to do.
        break;
    case MV_CALC:
        // re-init page preview layout.
        mpPgPrevwLayout->ReInit();

        // correct calculation of new start page.
        if( nNewSttPage > nPageCount )
            nNewSttPage = nPageCount;

        // correct selected page number
        if( SelectedPage() > nPageCount )
            SetSelectedPage( nNewSttPage ? nNewSttPage : 1 );
    }

    mpPgPrevwLayout->Prepare( nNewSttPage, Point(0,0), maPxWinSize,
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

void SwPagePreViewWin::SetWinSize( const Size& rNewSize )
{
    // die Size wollen wir aber immer in Pixel-Einheiten haben
    maPxWinSize = LogicToPixel( rNewSize );

    if( USHRT_MAX == mnSttPage )
    {
        mnSttPage = GetDefSttPage();
        SetSelectedPage( GetDefSttPage() );
    }

    if ( mbCalcScaleForPreviewLayout )
    {
        mpPgPrevwLayout->Init( mnCol, mnRow, maPxWinSize, true );
        maScale = GetMapMode().GetScaleX();
    }
    mpPgPrevwLayout->Prepare( mnSttPage, Point(0,0), maPxWinSize,
                              mnSttPage, maPaintedPreviewDocRect );
    if ( mbCalcScaleForPreviewLayout )
    {
        SetSelectedPage( mnSttPage );
        mbCalcScaleForPreviewLayout = false;
    }
    SetPagePreview(mnRow, mnCol);
    maScale = GetMapMode().GetScaleX();
}

OUString SwPagePreViewWin::GetStatusStr( sal_uInt16 nPageCnt ) const
{
    // show physical and virtual page number of
    // selected page, if it's visible.
    sal_uInt16 nPageNum;
    if ( mpPgPrevwLayout->IsPageVisible( mpPgPrevwLayout->SelectedPage() ) )
    {
        nPageNum = mpPgPrevwLayout->SelectedPage();
    }
    else
    {
        nPageNum = mnSttPage > 1 ? mnSttPage : 1;
    }
    OUStringBuffer aStatusStr;
    sal_uInt16 nVirtPageNum = mpPgPrevwLayout->GetVirtPageNumByPageNum( nPageNum );
    if( nVirtPageNum && nVirtPageNum != nPageNum )
    {
        aStatusStr.append( static_cast<sal_Int32>(nVirtPageNum) ).append( ' ' );
    }
    aStatusStr.append( static_cast<sal_Int32>(nPageNum) );
    aStatusStr.append( " / " );
    aStatusStr.append( static_cast<sal_Int32>(nPageCnt) );
    return aStatusStr.makeStringAndClear();
}

void  SwPagePreViewWin::KeyInput( const KeyEvent &rKEvt )
{
    const KeyCode& rKeyCode = rKEvt.GetKeyCode();
    sal_uInt16 nKey = rKeyCode.GetCode();
    sal_Bool bHandled = sal_False;
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
            bHandled = sal_True;
            mrView.GetViewFrame()->GetDispatcher()->Execute(
                                nSlot, SFX_CALLMODE_ASYNCHRON );
        }
    }
    if( !bHandled && !mrView.KeyInput( rKEvt ) )
        Window::KeyInput( rKEvt );
}

void SwPagePreViewWin::Command( const CommandEvent& rCEvt )
{
    sal_Bool bCallBase = sal_True;
    switch( rCEvt.GetCommand() )
    {
        case COMMAND_CONTEXTMENU:
            mrView.GetViewFrame()->GetDispatcher()->ExecutePopup();
            bCallBase = sal_False;
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

void SwPagePreViewWin::MouseButtonDown( const MouseEvent& rMEvt )
{
    // consider single-click to set selected page
    if( MOUSE_LEFT == ( rMEvt.GetModifier() + rMEvt.GetButtons() ) )
    {
        Point aPrevwPos( PixelToLogic( rMEvt.GetPosPixel() ) );
        Point aDocPos;
        bool bPosInEmptyPage;
        sal_uInt16 nNewSelectedPage;
        bool bIsDocPos =
            mpPgPrevwLayout->IsPrevwPosInDocPrevwPage( aPrevwPos,
                                    aDocPos, bPosInEmptyPage, nNewSelectedPage );
        if ( bIsDocPos && rMEvt.GetClicks() == 2 )
        {
            // close page preview, set new cursor position and switch to
            // normal view.
            String sNewCrsrPos( String::CreateFromInt32( aDocPos.X() ));
            ((( sNewCrsrPos += ';' )
                            += String::CreateFromInt32( aDocPos.Y() )) )
                            += ';';
            mrView.SetNewCrsrPos( sNewCrsrPos );

            SfxViewFrame *pTmpFrm = mrView.GetViewFrame();
            pTmpFrm->GetBindings().Execute( SID_VIEWSHELL0, NULL, 0,
                                                    SFX_CALLMODE_ASYNCHRON );
        }
        else if ( bIsDocPos || bPosInEmptyPage )
        {
            // show clicked page as the selected one
            mpPgPrevwLayout->MarkNewSelectedPage( nNewSelectedPage );
            GetViewShell()->ShowPreViewSelection( nNewSelectedPage );
            // adjust position at vertical scrollbar.
            if ( mpPgPrevwLayout->DoesPreviewLayoutRowsFitIntoWindow() )
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

/******************************************************************************
 *  Beschreibung: Userprefs bzw Viewoptions setzen
 ******************************************************************************/
void SwPagePreViewWin::SetPagePreview( sal_uInt8 nRow, sal_uInt8 nCol )
{
    SwMasterUsrPref *pOpt = (SwMasterUsrPref *)SW_MOD()->GetUsrPref(sal_False);

    if (nRow != pOpt->GetPagePrevRow() || nCol != pOpt->GetPagePrevCol())
    {
        pOpt->SetPagePrevRow( nRow );
        pOpt->SetPagePrevCol( nCol );
        pOpt->SetModified();

        //Scrollbar updaten!
        mrView.ScrollViewSzChg();
    }
}

/** get selected page in document preview

    @author OD
*/
sal_uInt16 SwPagePreViewWin::SelectedPage() const
{
    return mpPgPrevwLayout->SelectedPage();
}

/** set selected page number in document preview

    @author OD
*/
void SwPagePreViewWin::SetSelectedPage( sal_uInt16 _nSelectedPageNum )
{
    mpPgPrevwLayout->SetSelectedPage( _nSelectedPageNum );
}

/** method to enable/disable book preview

    @author OD
*/
bool SwPagePreViewWin::SetBookPreviewMode( const bool _bBookPreview )
{
    return mpPgPrevwLayout->SetBookPreviewMode( _bBookPreview,
                                                mnSttPage,
                                                maPaintedPreviewDocRect );
}

void SwPagePreViewWin::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    switch( rDCEvt.GetType() )
    {
    case DATACHANGED_SETTINGS:
        // ScrollBars neu anordnen bzw. Resize ausloesen, da sich
        // ScrollBar-Groesse geaendert haben kann. Dazu muss dann im
        // Resize-Handler aber auch die Groesse der ScrollBars aus
        // den Settings abgefragt werden.
        if( rDCEvt.GetFlags() & SETTINGS_STYLE )
            mrView.InvalidateBorder();              //Scrollbarbreiten
        // zoom has to be disabled if Accessibility support is switched on
        lcl_InvalidateZoomSlots(mrView.GetViewFrame()->GetBindings());
        break;

    case DATACHANGED_PRINTER:
    case DATACHANGED_DISPLAY:
    case DATACHANGED_FONTS:
    case DATACHANGED_FONTSUBSTITUTION:
        mrView.GetDocShell()->UpdateFontList(); //Fontwechsel
        if ( mpViewShell->GetWin() )
            mpViewShell->GetWin()->Invalidate();
        break;
    }
}

/** help method to execute SfxRequest FN_PAGEUP and FN_PAGEDOWN

    @author OD
*/
void SwPagePreView::_ExecPgUpAndPgDown( const bool  _bPgUp,
                                        SfxRequest* _pReq )
{
    SwPagePreviewLayout* pPagePrevwLay = GetViewShell()->PagePreviewLayout();
    // check, if top/bottom of preview is *not* already visible.
    if( pPagePrevwLay->GetWinPagesScrollAmount( _bPgUp ? -1 : 1 ) != 0 )
    {
        if ( pPagePrevwLay->DoesPreviewLayoutRowsFitIntoWindow() &&
             pPagePrevwLay->DoesPreviewLayoutColsFitIntoWindow() )
        {
            const int eMvMode = _bPgUp ?
                                SwPagePreViewWin::MV_PAGE_UP :
                                SwPagePreViewWin::MV_PAGE_DOWN;
            if ( ChgPage( eMvMode, sal_True ) )
                aViewWin.Invalidate();
        }
        else
        {
            SwTwips nScrollAmount;
            sal_uInt16 nNewSelectedPageNum = 0;
            const sal_uInt16 nVisPages = aViewWin.GetRow() * aViewWin.GetCol();
            if( _bPgUp )
            {
                if ( pPagePrevwLay->DoesPreviewLayoutRowsFitIntoWindow() )
                {
                    nScrollAmount = pPagePrevwLay->GetWinPagesScrollAmount( -1 );
                    if ( (aViewWin.SelectedPage() - nVisPages) > 0 )
                        nNewSelectedPageNum = aViewWin.SelectedPage() - nVisPages;
                    else
                        nNewSelectedPageNum = 1;
                }
                else
                    nScrollAmount = - Min( aViewWin.GetOutputSize().Height(),
                                           aViewWin.GetPaintedPreviewDocRect().Top() );
            }
            else
            {
                if ( pPagePrevwLay->DoesPreviewLayoutRowsFitIntoWindow() )
                {
                    nScrollAmount = pPagePrevwLay->GetWinPagesScrollAmount( 1 );
                    if ( (aViewWin.SelectedPage() + nVisPages) <= mnPageCount )
                        nNewSelectedPageNum = aViewWin.SelectedPage() + nVisPages;
                    else
                        nNewSelectedPageNum = mnPageCount;
                }
                else
                    nScrollAmount = Min( aViewWin.GetOutputSize().Height(),
                                         ( pPagePrevwLay->GetPrevwDocSize().Height() -
                                           aViewWin.GetPaintedPreviewDocRect().Bottom() ) );
            }
            aViewWin.Scroll( 0, nScrollAmount );
            if ( nNewSelectedPageNum != 0 )
            {
                aViewWin.SetSelectedPage( nNewSelectedPageNum );
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
            aViewWin.Invalidate();
        }
    }

    if ( _pReq )
        _pReq->Done();
}

// dann mal alles fuer die SwPagePreView
void  SwPagePreView::Execute( SfxRequest &rReq )
{
    int eMvMode;
    sal_uInt8 nRow = 1;
    sal_Bool bRetVal = sal_False;
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
                aViewWin.CalcWish( nRows, nCols );

            }
            else
                SwPreViewZoomDlg( aViewWin ).Execute();

        }
        break;
        case FN_SHOW_BOOKVIEW:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();
            const SfxPoolItem* pItem;
            bool bBookPreview = GetViewShell()->GetViewOptions()->IsPagePrevBookview();
            if( pArgs && SFX_ITEM_SET == pArgs->GetItemState( FN_SHOW_BOOKVIEW, sal_False, &pItem ) )
            {
                bBookPreview = static_cast< const SfxBoolItem* >( pItem )->GetValue();
                ( ( SwViewOption* ) GetViewShell()->GetViewOptions() )->SetPagePrevBookview( bBookPreview );
                    // cast is not gentleman like, but it's common use in writer and in this case
            }
            if ( aViewWin.SetBookPreviewMode( bBookPreview ) )
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
                aViewWin.Invalidate();
            }

        }
        break;
        case FN_SHOW_TWO_PAGES:
            aViewWin.CalcWish( nRow, 2 );
            break;

        case FN_PREVIEW_ZOOM:
        case SID_ATTR_ZOOM:
        {
            const SfxItemSet *pArgs = rReq.GetArgs();
            const SfxPoolItem* pItem;
            AbstractSvxZoomDialog *pDlg = 0;
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
                    pDlg = pFact->CreateSvxZoomDialog(&GetViewFrame()->GetWindow(), aCoreSet);
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
                if(SFX_ITEM_SET == pArgs->GetItemState(SID_ATTR_ZOOM, sal_True, &pItem))
                {
                    eType = ((const SvxZoomItem *)pItem)->GetType();
                    nZoomFactor = ((const SvxZoomItem *)pItem)->GetValue();
                }
                else if(SFX_ITEM_SET == pArgs->GetItemState(FN_PREVIEW_ZOOM, sal_True, &pItem))
                    nZoomFactor = ((const SfxUInt16Item *)pItem)->GetValue();
                if(USHRT_MAX != nZoomFactor)
                    SetZoom(eType, nZoomFactor);
            }
            delete pDlg;
        }
        break;
        case SID_ATTR_ZOOMSLIDER :
        {
            const SfxItemSet *pArgs = rReq.GetArgs();
            const SfxPoolItem* pItem;

            if ( pArgs && SFX_ITEM_SET == pArgs->GetItemState(SID_ATTR_ZOOMSLIDER, sal_True, &pItem ) )
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
            SwPagePreviewLayout* pPagePrevwLay = GetViewShell()->PagePreviewLayout();
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
            pPagePrevwLay->CalcStartValuesForSelectedPageMove( nHoriMove, nVertMove,
                                nNewSelectedPage, nNewStartPage, aNewStartPos );
            if ( aViewWin.SelectedPage() != nNewSelectedPage )
            {
                if ( pPagePrevwLay->IsPageVisible( nNewSelectedPage ) )
                {
                    pPagePrevwLay->MarkNewSelectedPage( nNewSelectedPage );
                    // adjust position at vertical scrollbar.
                    SetVScrollbarThumbPos( nNewSelectedPage );
                    bRefresh = false;
                }
                else
                {
                    aViewWin.SetSelectedPage( nNewSelectedPage );
                    aViewWin.SetSttPage( nNewStartPage );
                    int nRet = ChgPage( SwPagePreViewWin::MV_SELPAGE, sal_True );
                    bRefresh = 0 != nRet;
                }
                GetViewShell()->ShowPreViewSelection( nNewSelectedPage );
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
            aViewWin.SetSelectedPage( 1 );
            eMvMode = SwPagePreViewWin::MV_DOC_STT; bRetVal = sal_True; goto MOVEPAGE;
        case FN_END_OF_LINE:
        case FN_END_OF_DOCUMENT:
            aViewWin.SetSelectedPage( mnPageCount );
            eMvMode = SwPagePreViewWin::MV_DOC_END; bRetVal = sal_True; goto MOVEPAGE;
MOVEPAGE:
            {
                int nRet = ChgPage( eMvMode, sal_True );
                // return value fuer Basic
                if(bRetVal)
                    rReq.SetReturnValue(SfxBoolItem(rReq.GetSlot(), nRet == 0));

                bRefresh = 0 != nRet;
                rReq.Done();
            }
            break;

        case FN_PRINT_PAGEPREVIEW:
        {
            const SwPagePreViewPrtData* pPPVPD = aViewWin.GetViewShell()->GetDoc()->GetPreViewPrtData();
            // die Sache mit der Orientation
            if(pPPVPD)
            {
                SfxPrinter* pPrinter = GetPrinter( sal_True );
                if((pPrinter->GetOrientation() == ORIENTATION_LANDSCAPE)
                        != pPPVPD->GetLandscape())
                    pPrinter->SetOrientation(pPPVPD->GetLandscape() ? ORIENTATION_LANDSCAPE : ORIENTATION_PORTRAIT);
            }
            ::SetAppPrintOptions( aViewWin.GetViewShell(), sal_False );
            bNormalPrint = sal_False;
            sal_uInt16 nPrtSlot = SID_PRINTDOC;
            rReq.SetSlot( nPrtSlot );
            SfxViewShell::ExecuteSlot( rReq, SfxViewShell::GetInterface() );
            rReq.SetSlot( FN_PRINT_PAGEPREVIEW );
            return;
        }
        case SID_PRINTDOCDIRECT:
        case SID_PRINTDOC:
            ::SetAppPrintOptions( aViewWin.GetViewShell(), sal_False );
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
            sal_uInt16 nSelPage = aViewWin.SelectedPage();
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
        aViewWin.Invalidate();
}

void  SwPagePreView::GetState( SfxItemSet& rSet )
{
    SfxWhichIter aIter(rSet);
    sal_uInt8 nRow = 1;
    sal_uInt16 nWhich = aIter.FirstWhich();
    OSL_ENSURE(nWhich, "empty set");
    SwPagePreviewLayout* pPagePrevwLay = GetViewShell()->PagePreviewLayout();
    // zoom has to be disabled if Accessibility support is switched on
    sal_Bool bZoomEnabled = sal_True; // !Application::GetSettings().GetMiscSettings().GetEnableATToolSupport();

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
            if ( pPagePrevwLay->IsPageVisible( 1 ) )
                rSet.DisableItem(nWhich);
            break;
        }
        case FN_END_OF_DOCUMENT:
        {
            if ( pPagePrevwLay->IsPageVisible( mnPageCount ) )
                rSet.DisableItem(nWhich);
            break;
        }
        case FN_PAGEUP:
        {
            if( pPagePrevwLay->GetWinPagesScrollAmount( -1 ) == 0 )
                rSet.DisableItem(nWhich);
            break;
        }
        case FN_PAGEDOWN:
        {
            if( pPagePrevwLay->GetWinPagesScrollAmount( 1 ) == 0 )
                rSet.DisableItem(nWhich);
            break;
        }

        case FN_STAT_PAGE:
            {
                OUString aStr = sPageStr + aViewWin.GetStatusStr( mnPageCount );
                rSet.Put( SfxStringItem( nWhich, aStr) );
            }
            break;

        case SID_ATTR_ZOOM:
        case FN_STAT_ZOOM:
            {
                if(bZoomEnabled)
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
                else
                    rSet.DisableItem(nWhich);
            }
        break;
        case SID_ATTR_ZOOMSLIDER :
            {
                if(bZoomEnabled)
                {
                    const SwViewOption* pVOpt = GetViewShell()->GetViewOptions();
                    const sal_uInt16 nCurrentZoom = pVOpt->GetZoom();
                    SvxZoomSliderItem aZoomSliderItem( nCurrentZoom, MINZOOM, MAXZOOM );
                    aZoomSliderItem.AddSnappingPoint( 100 );
                    rSet.Put( aZoomSliderItem );
                }
                else
                    rSet.DisableItem(nWhich);
            }
        break;
        case FN_PREVIEW_ZOOM:
        {
            if(bZoomEnabled)
            {
                const SwViewOption* pVOpt = GetViewShell()->GetViewOptions();
                rSet.Put(SfxUInt16Item(nWhich, pVOpt->GetZoom()));
            }
            else
                rSet.DisableItem(nWhich);
        }
        break;
        case SID_ZOOM_IN:
        case SID_ZOOM_OUT:
        {
            const SwViewOption* pVOpt = GetViewShell()->GetViewOptions();
            if(!bZoomEnabled || (SID_ZOOM_OUT == nWhich && pVOpt->GetZoom() >= MAX_PREVIEW_ZOOM)||
              (SID_ZOOM_IN == nWhich && pVOpt->GetZoom() <= MIN_PREVIEW_ZOOM))
            {
                rSet.DisableItem(nWhich);
            }
        }
        break;
        case FN_SHOW_MULTIPLE_PAGES:
        //should never be disabled
        break;
        case FN_SHOW_BOOKVIEW:
        {
            sal_Bool b = GetViewShell()->GetViewOptions()->IsPagePrevBookview();
            rSet.Put(SfxBoolItem(nWhich, b));
        }
        break;

        case FN_SHOW_TWO_PAGES:
            if( 2 == aViewWin.GetCol() && nRow == aViewWin.GetRow() )
                rSet.DisableItem( nWhich );
            break;

        case FN_PRINT_PAGEPREVIEW:
            // hat den gleichen Status wie das normale Drucken
            {
                const SfxPoolItem* pItem;
                SfxItemSet aSet( *rSet.GetPool(), SID_PRINTDOC, SID_PRINTDOC );
                GetSlotState( SID_PRINTDOC, SfxViewShell::GetInterface(), &aSet );
                if( SFX_ITEM_DISABLED == aSet.GetItemState( SID_PRINTDOC,
                        sal_False, &pItem ))
                    rSet.DisableItem( nWhich );
                else if( SFX_ITEM_SET == aSet.GetItemState( SID_PRINTDOC,
                        sal_False, &pItem ))
                {
                    ((SfxPoolItem*)pItem)->SetWhich( FN_PRINT_PAGEPREVIEW );
                    rSet.Put( *pItem );
                }
            }
            break;

        case SID_PRINTPREVIEW:
            rSet.Put( SfxBoolItem( nWhich, sal_True ) );
            break;

        case SID_PRINTDOC:
        case SID_PRINTDOCDIRECT:
            GetSlotState( nWhich, SfxViewShell::GetInterface(), &rSet );
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

void  SwPagePreView::StateUndo(SfxItemSet& rSet)
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    while (nWhich)
    {
        rSet.DisableItem(nWhich);
        nWhich = aIter.NextWhich();
    }
}

void SwPagePreView::Init(const SwViewOption * pPrefs)
{
    if ( GetViewShell()->HasDrawView() )
        GetViewShell()->GetDrawView()->SetAnimationEnabled( sal_False );

    bNormalPrint = sal_True;

    // Die DocSize erfragen und verarbeiten. Ueber die Handler konnte
    // die Shell nicht gefunden werden, weil die Shell innerhalb CTOR-Phase
    // nicht in der SFX-Verwaltung bekannt ist.

    if( !pPrefs )
        pPrefs = SW_MOD()->GetUsrPref(sal_False);

    // die Felder aktualisieren
    // ACHTUNG: hochcasten auf die EditShell, um die SS zu nutzen.
    //          In den Methoden wird auf die akt. Shell abgefragt!
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

SwPagePreView::SwPagePreView(SfxViewFrame *pViewFrame, SfxViewShell* pOldSh):
    SfxViewShell( pViewFrame, SWVIEWFLAGS ),
    aViewWin( &pViewFrame->GetWindow(), *this ),
    nNewPage(USHRT_MAX),
    sPageStr(SW_RES(STR_PAGE)),
    pHScrollbar(0),
    pVScrollbar(0),
    pPageUpBtn(0),
    pPageDownBtn(0),
    pScrollFill(new ScrollBarBox( &pViewFrame->GetWindow(),
        pViewFrame->GetFrame().GetParentFrame() ? 0 : WB_SIZEABLE )),
    mnPageCount( 0 ),
    mbResetFormDesignMode( false ),
    mbFormDesignModeToReset( false )
{
    SetName(rtl::OUString("PageView" ));
    SetWindow( &aViewWin );
    SetHelpId(SW_PAGEPREVIEW);
    _CreateScrollbar( sal_True );
    _CreateScrollbar( sal_False );

    SfxObjectShell* pObjShell = pViewFrame->GetObjectShell();
    if ( !pOldSh )
    {
        //Gibt es schon eine Sicht auf das Dokument?
        SfxViewFrame *pF = SfxViewFrame::GetFirst( pObjShell );
        if ( pF == pViewFrame )
            pF = SfxViewFrame::GetNext( *pF, pObjShell );
        if ( pF )
            pOldSh = pF->GetViewShell();
    }

    ViewShell *pVS, *pNew;

    if( pOldSh && pOldSh->IsA( TYPE( SwPagePreView ) ) )
        pVS = ((SwPagePreView*)pOldSh)->GetViewShell();
    else
    {
        if( pOldSh && pOldSh->IsA( TYPE( SwView ) ) )
        {
            pVS = ((SwView*)pOldSh)->GetWrtShellPtr();
            // save the current ViewData of the previous SwView
            pOldSh->WriteUserData( sSwViewData, sal_False );
        }
        else
            pVS = GetDocShell()->GetWrtShell();
        if( pVS )
        {
            // setze die akt. Seite als die erste
            sal_uInt16 nPhysPg, nVirtPg;
            ((SwCrsrShell*)pVS)->GetPageNum( nPhysPg, nVirtPg, sal_True, sal_False );
            if( 1 != aViewWin.GetCol() && 1 == nPhysPg )
                --nPhysPg;
            aViewWin.SetSttPage( nPhysPg );
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
        pNew = new ViewShell( *pVS, &aViewWin, 0, VSHELLFLAG_ISPREVIEW );
    else
        pNew = new ViewShell(
                *((SwDocShell*)pViewFrame->GetObjectShell())->GetDoc(),
                &aViewWin, 0, 0, VSHELLFLAG_ISPREVIEW );

    aViewWin.SetViewShell( pNew );
    pNew->SetSfxViewShell( this );
    Init();
}

SwPagePreView::~SwPagePreView()
{
    SetWindow( 0 );

    delete pScrollFill;
    delete pHScrollbar;
    delete pVScrollbar;
    delete pPageUpBtn;
    delete pPageDownBtn;

}

SwDocShell* SwPagePreView::GetDocShell()
{
    return PTR_CAST(SwDocShell, GetViewFrame()->GetObjectShell());
}

int SwPagePreView::_CreateScrollbar( sal_Bool bHori )
{
    Window *pMDI = &GetViewFrame()->GetWindow();
    SwScrollbar** ppScrollbar = bHori ? &pHScrollbar : &pVScrollbar;

    OSL_ENSURE( !*ppScrollbar, "vorher abpruefen!" );

    if( !bHori )
    {

        pPageUpBtn      = new ImageButton(pMDI, SW_RES( BTN_PAGEUP ) );
        pPageUpBtn->SetHelpId(GetStaticInterface()->GetSlot(FN_PAGEUP)->GetCommand());
        pPageDownBtn    = new ImageButton(pMDI, SW_RES( BTN_PAGEDOWN ) );
        pPageDownBtn->SetHelpId(GetStaticInterface()->GetSlot(FN_PAGEDOWN)->GetCommand());
        Link aLk( LINK( this, SwPagePreView, BtnPage ) );
        pPageUpBtn->SetClickHdl( aLk );
        pPageDownBtn->SetClickHdl( aLk );
        pPageUpBtn->Show();
        pPageDownBtn->Show();
    }

    *ppScrollbar = new SwScrollbar( pMDI, bHori );

    ScrollDocSzChg();
    (*ppScrollbar)->EnableDrag( sal_True );
    (*ppScrollbar)->SetEndScrollHdl( LINK( this, SwPagePreView, EndScrollHdl ));


    (*ppScrollbar)->SetScrollHdl( LINK( this, SwPagePreView, ScrollHdl ));

    InvalidateBorder();
    (*ppScrollbar)->ExtendedShow();
    return 1;
}

/*
 * Button-Handler
 */
IMPL_LINK_INLINE_START( SwPagePreView, BtnPage, Button *, pButton )
{
    // use new helper method to perform page up
    // respectively page down.
    _ExecPgUpAndPgDown( pButton == pPageUpBtn );
    return 0;
}
IMPL_LINK_INLINE_END( SwPagePreView, BtnPage, Button *, pButton )

int SwPagePreView::ChgPage( int eMvMode, int bUpdateScrollbar )
{
    Rectangle aPixVisArea( aViewWin.LogicToPixel( aVisArea ) );
    int bChg = aViewWin.MovePage( eMvMode ) ||
               eMvMode == SwPagePreViewWin::MV_CALC ||
               eMvMode == SwPagePreViewWin::MV_NEWWINSIZE;
    aVisArea = aViewWin.PixelToLogic( aPixVisArea );

    if( bChg )
    {
        // Statusleiste updaten
        OUString aStr = sPageStr + aViewWin.GetStatusStr( mnPageCount );
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

// ab hier alles aus der SwView uebernommen
void SwPagePreView::CalcAndSetBorderPixel( SvBorder &rToFill, sal_Bool /*bInner*/ )
{
    const StyleSettings &rSet = aViewWin.GetSettings().GetStyleSettings();
    const long nTmp = rSet.GetScrollBarSize();
    if ( pVScrollbar->IsVisible( true ) )
        rToFill.Right()  = nTmp;
    if ( pHScrollbar->IsVisible( true ) )
        rToFill.Bottom() = nTmp;
    SetBorderPixel( rToFill );
}

void  SwPagePreView::InnerResizePixel( const Point &rOfst, const Size &rSize )
{
    SvBorder aBorder;
    CalcAndSetBorderPixel( aBorder, sal_True );
    Rectangle aRect( rOfst, rSize );
    aRect += aBorder;
    ViewResizePixel( aViewWin, aRect.TopLeft(), aRect.GetSize(),
                    aViewWin.GetOutputSizePixel(),
                    sal_True,
                    *pVScrollbar, *pHScrollbar, pPageUpBtn, pPageDownBtn, 0,
                    *pScrollFill );

    //EditWin niemals einstellen!
    //VisArea niemals einstellen!
}

void SwPagePreView::OuterResizePixel( const Point &rOfst, const Size &rSize )
{
    SvBorder aBorder;
    CalcAndSetBorderPixel( aBorder, sal_False );

    //EditWin niemals einstellen!

    Size aTmpSize( aViewWin.GetOutputSizePixel() );
    Point aBottomRight( aViewWin.PixelToLogic( Point( aTmpSize.Width(), aTmpSize.Height() ) ) );
    SetVisArea( Rectangle( Point(), aBottomRight ) );

    //Aufruf der DocSzChgd-Methode der Scrollbars ist noetig, da vom maximalen
    //Scrollrange immer die halbe Hoehe der VisArea abgezogen wird.
    if ( pVScrollbar && aTmpSize.Width() > 0 && aTmpSize.Height() > 0 )
    {
        ScrollDocSzChg();
    }

    SvBorder aBorderNew;
    CalcAndSetBorderPixel( aBorderNew, sal_False );
    ViewResizePixel( aViewWin, rOfst, rSize, aViewWin.GetOutputSizePixel(),
                        sal_False, *pVScrollbar,
                        *pHScrollbar, pPageUpBtn, pPageDownBtn, 0, *pScrollFill );
}

void SwPagePreView::SetVisArea( const Rectangle &rRect, sal_Bool bUpdateScrollbar )
{
    const Point aTopLeft(AlignToPixel(rRect.TopLeft()));
    const Point aBottomRight(AlignToPixel(rRect.BottomRight()));
    Rectangle aLR(aTopLeft,aBottomRight);

    if(aLR == aVisArea)
        return;
        // keine negative Position, keine neg. Groesse

    if(aLR.Top() < 0)
    {
        aLR.Bottom() += Abs(aLR.Top());
        aLR.Top() = 0;
    }

    if(aLR.Left() < 0)
    {
        aLR.Right() += Abs(aLR.Left());
        aLR.Left() = 0;
    }
    if(aLR.Right() < 0) aLR.Right() = 0;
    if(aLR.Bottom() < 0) aLR.Bottom() = 0;
    if(aLR == aVisArea ||
        // Leeres Rechteck nicht beachten
        ( 0 == aLR.Bottom() - aLR.Top() && 0 == aLR.Right() - aLR.Left() ) )
        return;

    if( aLR.Left() > aLR.Right() || aLR.Top() > aLR.Bottom() )
        return;

    //Bevor die Daten veraendert werden ggf. ein Update rufen. Dadurch wird
    //sichergestellt, da? anliegende Paints korrekt in Dokumentkoordinaten
    //umgerechnet werden.
    //Vorsichtshalber tun wir das nur wenn an der Shell eine Action laeuft,
    //denn dann wir nicht wirklich gepaintet sondern die Rechtecke werden
    //lediglich (in Dokumentkoordinaten) vorgemerkt.
    if( GetViewShell()->ActionPend() )
        aViewWin.Update();

    // setze am View-Win die aktuelle Size
    aVisArea = aLR;
    aViewWin.SetWinSize( aLR.GetSize() );
    ChgPage( SwPagePreViewWin::MV_NEWWINSIZE, bUpdateScrollbar );

    aViewWin.Invalidate();
}

IMPL_LINK( SwPagePreView, ScrollHdl, SwScrollbar *, pScrollbar )
{
    if(!GetViewShell())
        return 0;
    if( !pScrollbar->IsHoriScroll() &&
        pScrollbar->GetType() == SCROLL_DRAG &&
        Help::IsQuickHelpEnabled() &&
        GetViewShell()->PagePreviewLayout()->DoesPreviewLayoutRowsFitIntoWindow())
    {
        // wieviele Seiten scrollen ??
        String sStateStr(sPageStr);
        sal_uInt16 nThmbPos = (sal_uInt16)pScrollbar->GetThumbPos();
        if( 1 == aViewWin.GetCol() || !nThmbPos )
            ++nThmbPos;
        sStateStr += String::CreateFromInt32( nThmbPos );
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

IMPL_LINK( SwPagePreView, EndScrollHdl, SwScrollbar *, pScrollbar )
{
    if(!GetViewShell())
        return 0;

    // boolean to avoid unnecessary invalidation of the window.
    bool bInvalidateWin = true;

    if( !pScrollbar->IsHoriScroll() )       // scroll vertically
    {
        if ( Help::IsQuickHelpEnabled() )
            Help::ShowQuickHelp(pScrollbar, Rectangle(), aEmptyStr, 0);
        if ( GetViewShell()->PagePreviewLayout()->DoesPreviewLayoutRowsFitIntoWindow() )
        {
            // wieviele Seiten scrollen ??
            sal_uInt16 nThmbPos = (sal_uInt16)pScrollbar->GetThumbPos();
            // adjust to new preview functionality
            if( nThmbPos != aViewWin.SelectedPage() )
            {
                // consider case that page <nThmbPos>
                // is already visible
                SwPagePreviewLayout* pPagePrevwLay = GetViewShell()->PagePreviewLayout();
                if ( pPagePrevwLay->IsPageVisible( nThmbPos ) )
                {
                    pPagePrevwLay->MarkNewSelectedPage( nThmbPos );
                    // invalidation of window is unnecessary
                    bInvalidateWin = false;
                }
                else
                {
                    // consider whether layout columns
                    // fit or not.
                    if ( !pPagePrevwLay->DoesPreviewLayoutColsFitIntoWindow() )
                    {
                        aViewWin.SetSttPage( nThmbPos );
                        aViewWin.SetSelectedPage( nThmbPos );
                        ChgPage( SwPagePreViewWin::MV_SCROLL, sal_False );
                        // update scrollbars
                        ScrollViewSzChg();
                    }
                    else
                    {
                        // correct scroll amount
                        const sal_Int16 nPageDiff = nThmbPos - aViewWin.SelectedPage();
                        const sal_uInt16 nVisPages = aViewWin.GetRow() * aViewWin.GetCol();
                        sal_Int16 nWinPagesToScroll = nPageDiff / nVisPages;
                        if ( nPageDiff % nVisPages )
                        {
                            // decrease/increase number of preview pages to scroll
                            nPageDiff < 0 ? --nWinPagesToScroll : ++nWinPagesToScroll;
                        }
                        aViewWin.SetSelectedPage( nThmbPos );
                        aViewWin.Scroll( 0, pPagePrevwLay->GetWinPagesScrollAmount( nWinPagesToScroll ) );
                    }
                }
                // update accessibility
                GetViewShell()->ShowPreViewSelection( nThmbPos );
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
            aViewWin.Scroll(0, nThmbPos - aViewWin.GetPaintedPreviewDocRect().Top());
        }
    }
    else
    {
        long nThmbPos = pScrollbar->GetThumbPos();
        aViewWin.Scroll(nThmbPos - aViewWin.GetPaintedPreviewDocRect().Left(), 0);
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
        aViewWin.Invalidate();
    }
    return 0;
}

Point SwPagePreView::AlignToPixel(const Point &rPt) const
{
    return aViewWin.PixelToLogic( aViewWin.LogicToPixel( rPt ) );
}

void SwPagePreView::DocSzChgd( const Size &rSz )
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
        ChgPage( SwPagePreViewWin::MV_CALC, sal_True );
        ScrollDocSzChg();

        aViewWin.Invalidate();
    }
}

void SwPagePreView::ScrollViewSzChg()
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
            sal_uInt16 nVisPages = aViewWin.GetRow() * aViewWin.GetCol();

            pVScrollbar->SetVisibleSize( nVisPages );
            // set selected page as scroll bar position,
            // if it is visible.
            SwPagePreviewLayout* pPagePrevwLay = GetViewShell()->PagePreviewLayout();
            if ( pPagePrevwLay->IsPageVisible( aViewWin.SelectedPage() ) )
            {
                pVScrollbar->SetThumbPos( aViewWin.SelectedPage() );
            }
            else
            {
                pVScrollbar->SetThumbPos( aViewWin.GetSttPage() );
            }
            pVScrollbar->SetLineSize( aViewWin.GetCol() );
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
            const Rectangle& rDocRect = aViewWin.GetPaintedPreviewDocRect();
            const Size& rPreviewSize =
                    GetViewShell()->PagePreviewLayout()->GetPrevwDocSize();
            pVScrollbar->SetRangeMax(rPreviewSize.Height()) ;
            long nVisHeight = rDocRect.GetHeight();
            pVScrollbar->SetVisibleSize( nVisHeight );
            pVScrollbar->SetThumbPos( rDocRect.Top() );
            pVScrollbar->SetLineSize( nVisHeight / 10 );
            pVScrollbar->SetPageSize( nVisHeight / 2 );

            bShowVScrollbar = true;
        }

        ShowVScrollbar(bShowVScrollbar);
        pPageUpBtn->Show(bShowVScrollbar);
        pPageDownBtn->Show(bShowVScrollbar);
    }
    if(pHScrollbar)
    {
        const Rectangle& rDocRect = aViewWin.GetPaintedPreviewDocRect();
        const Size& rPreviewSize =
                GetViewShell()->PagePreviewLayout()->GetPrevwDocSize();
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

        ShowHScrollbar(bShowHScrollbar);
    }
    pScrollFill->Show(bShowVScrollbar && bShowHScrollbar);
}

void SwPagePreView::ScrollDocSzChg()
{
    ScrollViewSzChg();
}

// alles zum Thema Drucken
SfxPrinter*  SwPagePreView::GetPrinter( sal_Bool bCreate )
{
    return aViewWin.GetViewShell()->getIDocumentDeviceAccess()->getPrinter( bCreate );
}

sal_uInt16  SwPagePreView::SetPrinter( SfxPrinter *pNew, sal_uInt16 nDiffFlags, bool )
{
    ViewShell &rSh = *GetViewShell();
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

    const sal_Bool bChgOri = nDiffFlags & SFX_PRINTER_CHG_ORIENTATION ? sal_True : sal_False;
    const sal_Bool bChgSize= nDiffFlags & SFX_PRINTER_CHG_SIZE ? sal_True : sal_False;
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
            aViewWin.CalcWish( aViewWin.GetRow(), aViewWin.GetCol() );
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

SfxTabPage*  SwPagePreView::CreatePrintOptionsPage( Window *pParent,
                                                const SfxItemSet &rOptions )
{
    return ::CreatePrintOptionsPage( pParent, rOptions, !bNormalPrint );
}

Size  SwPagePreView::GetOptimalSizePixel() const
{
    OSL_FAIL( "overloaded virtual method <SwPagePreView::GetOptimalSizePixel()> needed ??" );
    return Size( -1, -1 );
}

void SwPagePreViewWin::SetViewShell( ViewShell* pShell )
{
    mpViewShell = pShell;
    if ( mpViewShell && mpViewShell->IsPreView() )
    {
        mpPgPrevwLayout = mpViewShell->PagePreviewLayout();
    }
}

void SwPagePreViewWin::RepaintCoreRect( const SwRect& rRect )
{
    // #i24183#
    if ( mpPgPrevwLayout->PreviewLayoutValid() )
    {
        mpPgPrevwLayout->Repaint( Rectangle( rRect.Pos(), rRect.SSize() ) );
    }
}

/** method to adjust preview to a new zoom factor

    #i19975# also consider zoom type - adding parameter <_eZoomType>
*/
void SwPagePreViewWin::AdjustPreviewToNewZoom( const sal_uInt16 _nZoomFactor,
                                               const SvxZoomType _eZoomType )
{
    // #i19975# consider zoom type
    if ( _eZoomType == SVX_ZOOM_WHOLEPAGE )
    {
        mnRow = 1;
        mnCol = 1;
        mpPgPrevwLayout->Init( mnCol, mnRow, maPxWinSize, true );
        mpPgPrevwLayout->Prepare( mnSttPage, Point(0,0), maPxWinSize,
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
                mpPgPrevwLayout->GetPreviewStartPosForNewScale( aNewScale, maScale, aNewWinSize );

        // remember new scaling and prepare preview paint
        // Note: paint of preview will be performed by a corresponding invalidate
        //          due to property changes.
        maScale = aNewScale;
        mpPgPrevwLayout->Prepare( 0, aNewPaintStartPos, maPxWinSize,
                                  mnSttPage, maPaintedPreviewDocRect );
    }

}

/**
 * pixel scrolling - horizontally always or vertically
 * when less than the desired number of rows fits into
 * the view
 */
void SwPagePreViewWin::Scroll(long nXMove, long nYMove, sal_uInt16 /*nFlags*/)
{
    maPaintedPreviewDocRect.Move(nXMove, nYMove);
    mpPgPrevwLayout->Prepare( 0, maPaintedPreviewDocRect.TopLeft(),
                              maPxWinSize, mnSttPage,
                              maPaintedPreviewDocRect );

}

sal_Bool SwPagePreView::HandleWheelCommands( const CommandEvent& rCEvt )
{
    sal_Bool bOk = sal_False;
    const CommandWheelData* pWData = rCEvt.GetWheelData();
    if( pWData && COMMAND_WHEEL_ZOOM == pWData->GetMode() )
    {
        if(!Application::GetSettings().GetMiscSettings().GetEnableATToolSupport())
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
        bOk = aViewWin.HandleScrollCommand( rCEvt, pHScrollbar, pVScrollbar );
    return bOk;
}

uno::Reference< ::com::sun::star::accessibility::XAccessible >
    SwPagePreViewWin::CreateAccessible()
{
    SolarMutexGuard aGuard; // this should have happend already!!!

    OSL_ENSURE( GetViewShell() != NULL, "We need a view shell" );
    return GetViewShell()->CreateAccessiblePreview();
}

void SwPagePreView::ApplyAccessiblityOptions(SvtAccessibilityOptions& rAccessibilityOptions)
{
    GetViewShell()->ApplyAccessiblityOptions(rAccessibilityOptions);
}

void SwPagePreView::ShowHScrollbar(sal_Bool bShow)
{
    pHScrollbar->Show(bShow);
    InvalidateBorder();
}

void SwPagePreView::ShowVScrollbar(sal_Bool bShow)
{
    pVScrollbar->Show(bShow);
    InvalidateBorder();
}

void SwPagePreView::SetZoom(SvxZoomType eType, sal_uInt16 nFactor)
{
    ViewShell& rSh = *GetViewShell();
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
        aViewWin.AdjustPreviewToNewZoom( nFactor, eType );
        ScrollViewSzChg();
    }
}

/** adjust position of vertical scrollbar

    @author OD
*/
void SwPagePreView::SetVScrollbarThumbPos( const sal_uInt16 _nNewThumbPos )
{
    if ( pVScrollbar )
    {
        pVScrollbar->SetThumbPos( _nNewThumbPos );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
