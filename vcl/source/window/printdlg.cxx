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

#include <printdlg.hxx>
#include <svdata.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>

#include <vcl/PrinterOptions.hxx>
#include <vcl/QueueInfo.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/naturalsort.hxx>
#include <vcl/print.hxx>
#include <vcl/wall.hxx>
#include <vcl/decoview.hxx>
#include <configsettings.hxx>
#include <vcl/help.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>

#include <unotools/localedatawrapper.hxx>

#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

using namespace vcl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;

enum
{
    ORIENTATION_AUTOMATIC,
    ORIENTATION_PORTRAIT,
    ORIENTATION_LANDSCAPE
};

namespace {
   bool lcl_ListBoxCompare( const OUString& rStr1, const OUString& rStr2 )
   {
       return vcl::NaturalSortCompare( rStr1, rStr2 ) < 0;
   }
}

MoreOptionsDialog::MoreOptionsDialog(PrintDialog* i_pParent)
    : GenericDialogController(i_pParent->getDialog(), "vcl/ui/moreoptionsdialog.ui", "MoreOptionsDialog")
    , mpParent( i_pParent )
    , mxOKButton(m_xBuilder->weld_button("ok"))
    , mxCancelButton(m_xBuilder->weld_button("cancel"))
    , mxSingleJobsBox(m_xBuilder->weld_check_button("singlejobs"))
{
    mxSingleJobsBox->set_active( mpParent->isSingleJobs() );

    mxOKButton->connect_clicked( LINK( this, MoreOptionsDialog, ClickHdl ) );
    mxCancelButton->connect_clicked( LINK( this, MoreOptionsDialog, ClickHdl ) );
}

MoreOptionsDialog::~MoreOptionsDialog()
{
}

IMPL_LINK (MoreOptionsDialog, ClickHdl, weld::Button&, rButton, void)
{
    if (&rButton == mxOKButton.get())
    {
        mpParent->mbSingleJobs = mxSingleJobsBox->get_active();
        m_xDialog->response(RET_OK);
    }
    else if (&rButton == mxCancelButton.get())
    {
        m_xDialog->response(RET_CANCEL);
    }
}

PrintDialog::PrintPreviewWindow::PrintPreviewWindow(PrintDialog* pDialog)
    : mpDialog(pDialog)
    , maMtf()
    , maOrigSize( 10, 10 )
    , maPreviewSize()
    , mnDPIX(Application::GetDefaultDevice()->GetDPIX())
    , mnDPIY(Application::GetDefaultDevice()->GetDPIY())
    , maPreviewBitmap()
    , maReplacementString()
    , mbGreyscale( false )
{
}

PrintDialog::PrintPreviewWindow::~PrintPreviewWindow()
{
}

void PrintDialog::PrintPreviewWindow::Resize()
{
    Size aNewSize(GetOutputSizePixel());
    long nTextHeight = GetDrawingArea()->get_text_height();
    // leave small space for decoration
    aNewSize.AdjustWidth( -(nTextHeight + 2) );
    aNewSize.AdjustHeight( -(nTextHeight + 2) );
    Size aScaledSize;
    double fScale = 1.0;

    // #i106435# catch corner case of Size(0,0)
    Size aOrigSize( maOrigSize );
    if( aOrigSize.Width() < 1 )
        aOrigSize.setWidth( aNewSize.Width() );
    if( aOrigSize.Height() < 1 )
        aOrigSize.setHeight( aNewSize.Height() );
    if( aOrigSize.Width() > aOrigSize.Height() )
    {
        aScaledSize = Size( aNewSize.Width(), aNewSize.Width() * aOrigSize.Height() / aOrigSize.Width() );
        if( aScaledSize.Height() > aNewSize.Height() )
            fScale = double(aNewSize.Height())/double(aScaledSize.Height());
    }
    else
    {
        aScaledSize = Size( aNewSize.Height() * aOrigSize.Width() / aOrigSize.Height(), aNewSize.Height() );
        if( aScaledSize.Width() > aNewSize.Width() )
            fScale = double(aNewSize.Width())/double(aScaledSize.Width());
    }
    aScaledSize.setWidth( long(aScaledSize.Width()*fScale) );
    aScaledSize.setHeight( long(aScaledSize.Height()*fScale) );

    maPreviewSize = aScaledSize;

    // check and evtl. recreate preview bitmap
    preparePreviewBitmap();
}

void PrintDialog::PrintPreviewWindow::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    pDrawingArea->set_size_request(pDrawingArea->get_approximate_digit_width() * 45,
                                   pDrawingArea->get_text_height() * 30);
    CustomWidgetController::SetDrawingArea(pDrawingArea);
}

void PrintDialog::PrintPreviewWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    rRenderContext.Push();
    if (vcl::Window* pDefaultDevice = dynamic_cast<vcl::Window*>(Application::GetDefaultDevice()))
    {
        Font aFont(rRenderContext.GetSettings().GetStyleSettings().GetLabelFont());
        pDefaultDevice->SetPointFont(rRenderContext, aFont);
    }

    rRenderContext.SetBackground(Wallpaper(Application::GetSettings().GetStyleSettings().GetDialogColor()));
    rRenderContext.Erase();

    auto nTextHeight = rRenderContext.GetTextHeight();
    Size aSize(GetOutputSizePixel());
    Point aOffset((aSize.Width()  - maPreviewSize.Width()  + nTextHeight) / 2,
                  (aSize.Height() - maPreviewSize.Height() + nTextHeight) / 2);

    // horizontal line
    {
        auto nWidth = rRenderContext.GetTextWidth(maHorzText);

        auto nStart = aOffset.X() + (maPreviewSize.Width() - nWidth) / 2;
        rRenderContext.DrawText(Point(nStart, aOffset.Y() - nTextHeight), maHorzText, 0, maHorzText.getLength());

        DecorationView aDecoView(&rRenderContext);
        auto nTop = aOffset.Y() - (nTextHeight / 2);
        aDecoView.DrawSeparator(Point(aOffset.X(), nTop), Point(nStart - 2, nTop), false);
        aDecoView.DrawSeparator(Point(nStart + nWidth + 2, nTop), Point(aOffset.X() + maPreviewSize.Width(), nTop), false);
    }

    // vertical line
    {
        rRenderContext.Push(PushFlags::FONT);
        vcl::Font aFont(rRenderContext.GetFont());
        aFont.SetOrientation(900);
        rRenderContext.SetFont(aFont);

        auto nLeft = aOffset.X() - nTextHeight;

        auto nWidth = rRenderContext.GetTextWidth(maVertText);
        auto nStart = aOffset.Y() + (maPreviewSize.Height() + nWidth) / 2;

        rRenderContext.DrawText(Point(nLeft, nStart), maVertText, 0, maVertText.getLength());

        DecorationView aDecoView(&rRenderContext);
        nLeft = aOffset.X() - (nTextHeight / 2);
        aDecoView.DrawSeparator(Point(nLeft, aOffset.Y()), Point(nLeft, nStart - nWidth - 2), true);
        aDecoView.DrawSeparator(Point(nLeft, nStart + 2), Point(nLeft, aOffset.Y() + maPreviewSize.Height()), true);

        rRenderContext.Pop();
    }

    if (!maReplacementString.isEmpty())
    {
        // replacement is active
        tools::Rectangle aTextRect(aOffset + Point(2, 2), Size(maPreviewSize.Width() - 4, maPreviewSize.Height() - 4));
        rRenderContext.DrawText(aTextRect, maReplacementString,
                                DrawTextFlags::Center | DrawTextFlags::VCenter |
                                DrawTextFlags::WordBreak | DrawTextFlags::MultiLine);
    }
    else
    {
        BitmapEx aPreviewBitmap(maPreviewBitmap);

        // This explicit force-to-scale allows us to get the
        // mentioned best quality here. Unfortunately this is
        // currently not sure when using just ::DrawBitmap with
        // a defined size or ::DrawOutDev
        aPreviewBitmap.Scale(maPreviewSize, BmpScaleFlag::BestQuality);
        rRenderContext.DrawBitmapEx(aOffset, aPreviewBitmap);
    }

    tools::Rectangle aFrameRect(aOffset + Point(-1, -1), Size(maPreviewSize.Width() + 2, maPreviewSize.Height() + 2));
    DecorationView aDecorationView(&rRenderContext);
    aDecorationView.DrawFrame(aFrameRect, DrawFrameStyle::Group);

    rRenderContext.Pop();
}

bool PrintDialog::PrintPreviewWindow::Command( const CommandEvent& rEvt )
{
    if( rEvt.GetCommand() == CommandEventId::Wheel )
    {
        const CommandWheelData* pWheelData = rEvt.GetWheelData();
        if(pWheelData->GetDelta() > 0)
            mpDialog->previewForward();
        else if (pWheelData->GetDelta() < 0)
            mpDialog->previewBackward();
        return true;
    }
    return CustomWidgetController::Command(rEvt);
}

void PrintDialog::PrintPreviewWindow::setPreview( const GDIMetaFile& i_rNewPreview,
                                                  const Size& i_rOrigSize,
                                                  const OUString& i_rPaperName,
                                                  const OUString& i_rReplacement,
                                                  sal_Int32 i_nDPIX,
                                                  sal_Int32 i_nDPIY,
                                                  bool i_bGreyscale
                                                 )
{
    maMtf = i_rNewPreview;
    mnDPIX = i_nDPIX;
    mnDPIY = i_nDPIY;
    maOrigSize = i_rOrigSize;
    maReplacementString = i_rReplacement;
    mbGreyscale = i_bGreyscale;

    // use correct measurements
    const LocaleDataWrapper& rLocWrap(Application::GetSettings().GetLocaleDataWrapper());
    MapUnit eUnit = MapUnit::MapMM;
    int nDigits = 0;
    if( rLocWrap.getMeasurementSystemEnum() == MeasurementSystem::US )
    {
        eUnit = MapUnit::Map100thInch;
        nDigits = 2;
    }
    Size aLogicPaperSize(OutputDevice::LogicToLogic(i_rOrigSize, MapMode(MapUnit::Map100thMM), MapMode(eUnit)));
    OUString aNumText( rLocWrap.getNum( aLogicPaperSize.Width(), nDigits ) );
    OUStringBuffer aBuf;
    aBuf.append( aNumText )
        .append( u' ' );
    aBuf.appendAscii( eUnit == MapUnit::MapMM ? "mm" : "in" );
    if( !i_rPaperName.isEmpty() )
    {
        aBuf.append( " (" );
        aBuf.append( i_rPaperName );
        aBuf.append( ')' );
    }
    maHorzText = aBuf.makeStringAndClear();

    aNumText = rLocWrap.getNum( aLogicPaperSize.Height(), nDigits );
    aBuf.append( aNumText )
        .append( u' ' );
    aBuf.appendAscii( eUnit == MapUnit::MapMM ? "mm" : "in" );
    maVertText = aBuf.makeStringAndClear();

    // We have a new Metafile and evtl. a new page, so we need to reset
    // the PreviewBitmap to force new creation
    maPreviewBitmap = Bitmap();

    // sets/calculates e.g. maPreviewSize
    // also triggers preparePreviewBitmap()
    Resize();

    Invalidate();
}

void PrintDialog::PrintPreviewWindow::preparePreviewBitmap()
{
    if(maPreviewSize.IsEmpty())
    {
        // not yet fully initialized, no need to prepare anything
        return;
    }

    // define an allowed number of pixels, also see
    // defaults for primitive renderers and similar. This
    // might be centralized and made dependent of 32/64bit
    const sal_uInt32 nMaxSquarePixels(500000);

    // check how big (squarePixels) the preview is currently (with
    // max value of MaxSquarePixels)
    const sal_uInt32 nCurrentSquarePixels(
        std::min(
            nMaxSquarePixels,
            static_cast<sal_uInt32>(maPreviewBitmap.GetSizePixel().getWidth())
            * static_cast<sal_uInt32>(maPreviewBitmap.GetSizePixel().getHeight())));

    // check how big (squarePixels) the preview needs to be (with
    // max value of MaxSquarePixels)
    const sal_uInt32 nRequiredSquarePixels(
        std::min(
            nMaxSquarePixels,
            static_cast<sal_uInt32>(maPreviewSize.getWidth())
            * static_cast<sal_uInt32>(maPreviewSize.getHeight())));

    // check if preview is big enough. Use a scaling value in
    // the comparison to not get bigger at the last possible moment
    // what may look awkward and pixelated (again). This means
    // to use a percentage value - if we have at least
    // that value of required pixels, we are good.
    static const double fPreventAwkwardFactor(1.35); // 35%
    if(nCurrentSquarePixels >= static_cast<sal_uInt32>(nRequiredSquarePixels * fPreventAwkwardFactor))
    {
        // at this place we also could add a mechanism to let the preview
        // bitmap 'shrink' again if it is currently 'too big' -> bigger
        // than required. I think this is not necessary for now.

        // already sufficient, done.
        return;
    }

    // check if we have enough square pixels e.g for 8x8 pixels
    if(nRequiredSquarePixels < 64)
    {
        // too small preview - let it empty
        return;
    }

    // Calculate nPlannedSquarePixels which is the required size
    // expanded by a percentage (with max value of MaxSquarePixels)
    static const double fExtraSpaceFactor(1.65); // 65%
    const sal_uInt32 nPlannedSquarePixels(
        std::min(
            nMaxSquarePixels,
            static_cast<sal_uInt32>(maPreviewSize.getWidth() * fExtraSpaceFactor)
            * static_cast<sal_uInt32>(maPreviewSize.getHeight() * fExtraSpaceFactor)));

    // calculate back new width and height - it might have been
    // truncated by MaxSquarePixels.
    // We know that w*h == nPlannedSquarePixels and w/h == ratio
    const double fRatio(static_cast<double>(maPreviewSize.getWidth()) / static_cast<double>(maPreviewSize.getHeight()));
    const double fNewWidth(sqrt(static_cast<double>(nPlannedSquarePixels) * fRatio));
    const double fNewHeight(sqrt(static_cast<double>(nPlannedSquarePixels) / fRatio));
    const Size aScaledSize(basegfx::fround(fNewWidth), basegfx::fround(fNewHeight));

    // check if this eventual maximum is already reached
    // due to having hit the MaxSquarePixels. Due to using
    // an integer AspectRatio, we cannot make a numeric exact
    // comparison - we need to compare if we are close
    const double fScaledSizeSquare(static_cast<double>(aScaledSize.getWidth() * aScaledSize.getHeight()));
    const double fPreviewSizeSquare(static_cast<double>(maPreviewBitmap.GetSizePixel().getWidth() * maPreviewBitmap.GetSizePixel().getHeight()));

    // test as equal up to 0.1% (0.001)
    if(fPreviewSizeSquare != 0.0 && fabs((fScaledSizeSquare / fPreviewSizeSquare) - 1.0) < 0.001)
    {
        // maximum is reached, avoid bigger scaling
        return;
    }

    // create temporary VDev and render to it
    ScopedVclPtrInstance<VirtualDevice> pPrerenderVDev(*Application::GetDefaultDevice());
    pPrerenderVDev->SetOutputSizePixel(aScaledSize, false);
    pPrerenderVDev->SetReferenceDevice( mnDPIX, mnDPIY );
    pPrerenderVDev->EnableOutput();
    pPrerenderVDev->SetBackground( Wallpaper(COL_WHITE) );

    GDIMetaFile aMtf( maMtf );

    Size aVDevSize( pPrerenderVDev->GetOutputSizePixel() );
    const Size aLogicSize( pPrerenderVDev->PixelToLogic( aVDevSize, MapMode( MapUnit::Map100thMM ) ) );
    Size aOrigSize( maOrigSize );
    if( aOrigSize.Width() < 1 )
        aOrigSize.setWidth( aLogicSize.Width() );
    if( aOrigSize.Height() < 1 )
        aOrigSize.setHeight( aLogicSize.Height() );
    double fScale = double(aLogicSize.Width())/double(aOrigSize.Width());

    pPrerenderVDev->Erase();
    pPrerenderVDev->Push();
    pPrerenderVDev->SetMapMode(MapMode(MapUnit::Map100thMM));
    DrawModeFlags nOldDrawMode = pPrerenderVDev->GetDrawMode();
    if( mbGreyscale )
        pPrerenderVDev->SetDrawMode( pPrerenderVDev->GetDrawMode() |
                                ( DrawModeFlags::GrayLine | DrawModeFlags::GrayFill | DrawModeFlags::GrayText |
                                  DrawModeFlags::GrayBitmap | DrawModeFlags::GrayGradient ) );
    aMtf.WindStart();
    aMtf.Scale( fScale, fScale );
    aMtf.WindStart();

    const AntialiasingFlags nOriginalAA(pPrerenderVDev->GetAntialiasing());
    pPrerenderVDev->SetAntialiasing(nOriginalAA | AntialiasingFlags::EnableB2dDraw);
    aMtf.Play( pPrerenderVDev.get(), Point( 0, 0 ), aLogicSize );
    pPrerenderVDev->SetAntialiasing(nOriginalAA);

    pPrerenderVDev->Pop();

    pPrerenderVDev->SetMapMode(MapMode(MapUnit::MapPixel));

    maPreviewBitmap = pPrerenderVDev->GetBitmapEx(Point(0, 0), aVDevSize);

    pPrerenderVDev->SetDrawMode( nOldDrawMode );
}

PrintDialog::ShowNupOrderWindow::ShowNupOrderWindow()
    : mnOrderMode( NupOrderType::LRTB )
    , mnRows( 1 )
    , mnColumns( 1 )
{
}

void PrintDialog::ShowNupOrderWindow::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    Size aSize(70, 70);
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    SetOutputSizePixel(aSize);
}

void PrintDialog::ShowNupOrderWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*i_rRect*/)
{
    rRenderContext.SetMapMode(MapMode(MapUnit::MapPixel));
    rRenderContext.SetTextColor(rRenderContext.GetSettings().GetStyleSettings().GetFieldTextColor());
    rRenderContext.SetBackground(Wallpaper(Application::GetSettings().GetStyleSettings().GetFieldColor()));
    rRenderContext.Erase();

    int nPages = mnRows * mnColumns;
    Font aFont(rRenderContext.GetSettings().GetStyleSettings().GetFieldFont());
    aFont.SetFontSize(Size(0, 24));
    rRenderContext.SetFont(aFont);
    Size aSampleTextSize(rRenderContext.GetTextWidth(OUString::number(nPages + 1)), rRenderContext.GetTextHeight());
    Size aOutSize(GetOutputSizePixel());
    Size aSubSize(aOutSize.Width() / mnColumns, aOutSize.Height() / mnRows);
    // calculate font size: shrink the sample text so it fits
    double fX = double(aSubSize.Width()) / double(aSampleTextSize.Width());
    double fY = double(aSubSize.Height()) / double(aSampleTextSize.Height());
    double fScale = (fX < fY) ? fX : fY;
    long nFontHeight = long(24.0 * fScale) - 3;
    if (nFontHeight < 5)
        nFontHeight = 5;
    aFont.SetFontSize(Size( 0, nFontHeight));
    rRenderContext.SetFont(aFont);
    long nTextHeight = rRenderContext.GetTextHeight();
    for (int i = 0; i < nPages; i++)
    {
        OUString aPageText(OUString::number(i + 1));
        int nX = 0, nY = 0;
        switch (mnOrderMode)
        {
        case NupOrderType::LRTB:
            nX = (i % mnColumns);
            nY = (i / mnColumns);
            break;
        case NupOrderType::TBLR:
            nX = (i / mnRows);
            nY = (i % mnRows);
            break;
        case NupOrderType::RLTB:
            nX = mnColumns - 1 - (i % mnColumns);
            nY = (i / mnColumns);
            break;
        case NupOrderType::TBRL:
            nX = mnColumns - 1 - (i / mnRows);
            nY = (i % mnRows);
            break;
        }
        Size aTextSize(rRenderContext.GetTextWidth(aPageText), nTextHeight);
        int nDeltaX = (aSubSize.Width() - aTextSize.Width()) / 2;
        int nDeltaY = (aSubSize.Height() - aTextSize.Height()) / 2;
        rRenderContext.DrawText(Point(nX * aSubSize.Width() + nDeltaX,
                                      nY * aSubSize.Height() + nDeltaY), aPageText);
    }
    DecorationView aDecorationView(&rRenderContext);
    aDecorationView.DrawFrame(tools::Rectangle(Point(0, 0), aOutSize), DrawFrameStyle::Group);
}

Size const & PrintDialog::getJobPageSize()
{
    if( maFirstPageSize.IsEmpty() )
    {
        maFirstPageSize = maNupPortraitSize;
        GDIMetaFile aMtf;
        if( maPController->getPageCountProtected() > 0 )
        {
            PrinterController::PageSize aPageSize = maPController->getPageFile( 0, aMtf, true );
            maFirstPageSize = aPageSize.aSize;
        }
    }
    return maFirstPageSize;
}

PrintDialog::PrintDialog(weld::Window* i_pWindow, const std::shared_ptr<PrinterController>& i_rController)
    : GenericDialogController(i_pWindow, "vcl/ui/printdialog.ui", "PrintDialog")
    , maPController( i_rController )
    , mxTabCtrl(m_xBuilder->weld_notebook("tabcontrol"))
    , mxPageLayoutFrame(m_xBuilder->weld_frame("layoutframe"))
    , mxPrinters(m_xBuilder->weld_combo_box("printersbox"))
    , mxStatusTxt(m_xBuilder->weld_label("status"))
    , mxSetupButton(m_xBuilder->weld_button("setup"))
    , mxCopyCountField(m_xBuilder->weld_spin_button("copycount"))
    , mxCollateBox(m_xBuilder->weld_check_button("collate"))
    , mxCollateImage(m_xBuilder->weld_image("collateimage"))
    , mxPageRangeEdit(m_xBuilder->weld_entry("pagerange"))
    , mxPageRangesRadioButton(m_xBuilder->weld_radio_button("rbRangePages"))
    , mxPaperSidesBox(m_xBuilder->weld_combo_box("sidesbox"))
    , mxReverseOrderBox(m_xBuilder->weld_check_button("reverseorder"))
    , mxOKButton(m_xBuilder->weld_button("ok"))
    , mxCancelButton(m_xBuilder->weld_button("cancel"))
    , mxHelpButton(m_xBuilder->weld_button("help"))
    , mxMoreOptionsBtn(m_xBuilder->weld_button("moreoptionsbtn"))
    , mxBackwardBtn(m_xBuilder->weld_button("backward"))
    , mxForwardBtn(m_xBuilder->weld_button("forward"))
    , mxFirstBtn(m_xBuilder->weld_button("btnFirst"))
    , mxLastBtn(m_xBuilder->weld_button("btnLast"))
    , mxPreviewBox(m_xBuilder->weld_check_button("previewbox"))
    , mxNumPagesText(m_xBuilder->weld_label("totalnumpages"))
    , mxPreview(new PrintPreviewWindow(this))
    , mxPreviewWindow(new weld::CustomWeld(*m_xBuilder, "preview", *mxPreview))
    , mxPageEdit(m_xBuilder->weld_entry("pageedit"))
    , mxPagesBtn(m_xBuilder->weld_radio_button("pagespersheetbtn"))
    , mxBrochureBtn(m_xBuilder->weld_radio_button("brochure"))
    , mxPagesBoxTitleTxt(m_xBuilder->weld_label("pagespersheettxt"))
    , mxNupPagesBox(m_xBuilder->weld_combo_box("pagespersheetbox"))
    , mxNupNumPagesTxt(m_xBuilder->weld_label("pagestxt"))
    , mxNupColEdt(m_xBuilder->weld_spin_button("pagecols"))
    , mxNupTimesTxt(m_xBuilder->weld_label("by"))
    , mxNupRowsEdt(m_xBuilder->weld_spin_button("pagerows"))
    , mxPageMarginTxt1(m_xBuilder->weld_label("pagemargintxt1"))
    , mxPageMarginEdt(m_xBuilder->weld_metric_spin_button("pagemarginsb", FieldUnit::MM))
    , mxPageMarginTxt2(m_xBuilder->weld_label("pagemargintxt2"))
    , mxSheetMarginTxt1(m_xBuilder->weld_label("sheetmargintxt1"))
    , mxSheetMarginEdt(m_xBuilder->weld_metric_spin_button("sheetmarginsb", FieldUnit::MM))
    , mxSheetMarginTxt2(m_xBuilder->weld_label("sheetmargintxt2"))
    , mxPaperSizeBox(m_xBuilder->weld_combo_box("papersizebox"))
    , mxOrientationBox(m_xBuilder->weld_combo_box("pageorientationbox"))
    , mxNupOrderTxt(m_xBuilder->weld_label("labelorder"))
    , mxNupOrderBox(m_xBuilder->weld_combo_box("orderbox"))
    , mxNupOrder(new ShowNupOrderWindow)
    , mxNupOrderWin(new weld::CustomWeld(*m_xBuilder, "orderpreview", *mxNupOrder))
    , mxBorderCB(m_xBuilder->weld_check_button("bordercb"))
    , mxRangeExpander(m_xBuilder->weld_expander("exRangeExpander"))
    , mxLayoutExpander(m_xBuilder->weld_expander("exLayoutExpander"))
    , mxCustom(m_xBuilder->weld_widget("customcontents"))
    , maPrintToFileText( VclResId( SV_PRINT_TOFILE_TXT ) )
    , maDefPrtText( VclResId( SV_PRINT_DEFPRT_TXT ) )
    , maNoPageStr( VclResId( SV_PRINT_NOPAGES ) )
    , maNoPreviewStr( VclResId( SV_PRINT_NOPREVIEW ) )
    , mnCurPage( 0 )
    , mnCachedPages( 0 )
    , mbCollateAlwaysOff(false)
    , mbShowLayoutFrame( true )
    , mbSingleJobs( false )
{
    // save printbutton text, gets exchanged occasionally with print to file
    maPrintText = mxOKButton->get_label();

    maPageStr = mxNumPagesText->get_label();

    Printer::updatePrinters();

    mxPrinters->append_text(maPrintToFileText);
    // fill printer listbox
    std::vector< OUString > rQueues( Printer::GetPrinterQueues() );
    std::sort( rQueues.begin(), rQueues.end(), lcl_ListBoxCompare );
    for( const auto& rQueue : rQueues )
    {
        mxPrinters->append_text(rQueue);
    }
    // select current printer
    if (mxPrinters->find_text(maPController->getPrinter()->GetName()) != -1)
        mxPrinters->set_active_text(maPController->getPrinter()->GetName());
    else
    {
        // fall back to last printer
        SettingsConfigItem* pItem = SettingsConfigItem::get();
        OUString aValue( pItem->getValue( "PrintDialog",
                                        "LastPrinter" ) );
        if (mxPrinters->find_text(aValue) != -1)
        {
            mxPrinters->set_active_text(aValue);
            maPController->setPrinter( VclPtrInstance<Printer>( aValue ) );
        }
        else
        {
            // fall back to default printer
            mxPrinters->set_active_text(Printer::GetDefaultPrinterName());
            maPController->setPrinter( VclPtrInstance<Printer>( Printer::GetDefaultPrinterName() ) );
        }
    }

    // not printing to file
    maPController->resetPrinterOptions( false );

    // update the text fields for the printer
    updatePrinterText();

    // set paper sizes listbox
    setPaperSizes();

    // setup dependencies
    checkControlDependencies();

    // setup paper sides box
    setupPaperSidesBox();

    // set initial focus to "Number of copies"
    mxCopyCountField->grab_focus();
    mxCopyCountField->select_region(0, -1);

    // setup sizes for N-Up
    Size aNupSize( maPController->getPrinter()->PixelToLogic(
                         maPController->getPrinter()->GetPaperSizePixel(), MapMode( MapUnit::Map100thMM ) ) );
    if( maPController->getPrinter()->GetOrientation() == Orientation::Landscape )
    {
        maNupLandscapeSize = aNupSize;
        // coverity[swapped_arguments : FALSE] - this is in the correct order
        maNupPortraitSize = Size( aNupSize.Height(), aNupSize.Width() );
    }
    else
    {
        maNupPortraitSize = aNupSize;
        // coverity[swapped_arguments : FALSE] - this is in the correct order
        maNupLandscapeSize = Size( aNupSize.Height(), aNupSize.Width() );
    }

    initFromMultiPageSetup( maPController->getMultipage() );

    // setup optional UI options set by application
    setupOptionalUI();

    // hide layout frame if unwanted
    mxPageLayoutFrame->set_visible(mbShowLayoutFrame);

    // restore settings from last run
    readFromSettings();

    // setup click hdl
    mxOKButton->connect_clicked(LINK(this, PrintDialog, ClickHdl));
    mxCancelButton->connect_clicked(LINK(this, PrintDialog, ClickHdl));
    mxHelpButton->connect_clicked(LINK(this, PrintDialog, ClickHdl));
    mxSetupButton->connect_clicked( LINK( this, PrintDialog, ClickHdl ) );
    mxMoreOptionsBtn->connect_clicked( LINK( this, PrintDialog, ClickHdl ) );
    mxBackwardBtn->connect_clicked(LINK(this, PrintDialog, ClickHdl));
    mxForwardBtn->connect_clicked(LINK(this, PrintDialog, ClickHdl));
    mxFirstBtn->connect_clicked(LINK(this, PrintDialog, ClickHdl));
    mxLastBtn->connect_clicked( LINK( this, PrintDialog, ClickHdl ) );
    mxPreviewBox->connect_clicked( LINK( this, PrintDialog, ClickHdl ) );
    mxBorderCB->connect_clicked( LINK( this, PrintDialog, ClickHdl ) );

    // setup toggle hdl
    mxReverseOrderBox->connect_toggled( LINK( this, PrintDialog, ToggleHdl ) );
    mxCollateBox->connect_toggled( LINK( this, PrintDialog, ToggleHdl ) );
    mxPagesBtn->connect_toggled( LINK( this, PrintDialog, ToggleHdl ) );

    // setup select hdl
    mxPrinters->connect_changed( LINK( this, PrintDialog, SelectHdl ) );
    mxPaperSidesBox->connect_changed( LINK( this, PrintDialog, SelectHdl ) );
    mxNupPagesBox->connect_changed( LINK( this, PrintDialog, SelectHdl ) );
    mxOrientationBox->connect_changed( LINK( this, PrintDialog, SelectHdl ) );
    mxNupOrderBox->connect_changed( LINK( this, PrintDialog, SelectHdl ) );
    mxPaperSizeBox->connect_changed( LINK( this, PrintDialog, SelectHdl ) );

    // setup modify hdl
    mxPageEdit->connect_activate( LINK( this, PrintDialog, ActivateHdl ) );
    mxPageEdit->connect_focus_out( LINK( this, PrintDialog, FocusOutHdl ) );
    mxCopyCountField->connect_value_changed( LINK( this, PrintDialog, SpinModifyHdl ) );
    mxNupColEdt->connect_value_changed( LINK( this, PrintDialog, SpinModifyHdl ) );
    mxNupRowsEdt->connect_value_changed( LINK( this, PrintDialog, SpinModifyHdl ) );
    mxPageMarginEdt->connect_value_changed( LINK( this, PrintDialog, MetricSpinModifyHdl ) );
    mxSheetMarginEdt->connect_value_changed( LINK( this, PrintDialog, MetricSpinModifyHdl ) );

    mxRangeExpander->connect_expanded(LINK( this, PrintDialog, ExpandHdl));
    mxLayoutExpander->connect_expanded(LINK( this, PrintDialog, ExpandHdl));

    updateNupFromPages();
}

IMPL_LINK_NOARG(PrintDialog, ExpandHdl, weld::Expander&, void)
{
    m_xDialog->resize_to_request();
}

PrintDialog::~PrintDialog()
{
}

void PrintDialog::setupPaperSidesBox()
{
    DuplexMode eDuplex = maPController->getPrinter()->GetDuplexMode();

    if ( eDuplex == DuplexMode::Unknown || isPrintToFile() )
    {
        mxPaperSidesBox->set_active( 0 );
        mxPaperSidesBox->set_sensitive( false );
    }
    else
    {
        mxPaperSidesBox->set_active( static_cast<sal_Int32>(eDuplex) - 1 );
        mxPaperSidesBox->set_sensitive( true );
    }
}

void PrintDialog::storeToSettings()
{
    SettingsConfigItem* pItem = SettingsConfigItem::get();

    pItem->setValue( "PrintDialog",
                     "LastPrinter",
                      isPrintToFile() ? Printer::GetDefaultPrinterName()
                                      : mxPrinters->get_active_text() );

    pItem->setValue( "PrintDialog",
                     "LastPage",
                     mxTabCtrl->get_tab_label_text(mxTabCtrl->get_current_page_ident()));

    pItem->setValue( "PrintDialog",
                     "WindowState",
                     OStringToOUString(m_xDialog->get_window_state(WindowStateMask::All), RTL_TEXTENCODING_UTF8) );

    pItem->setValue( "PrintDialog",
                     "CopyCount",
                     mxCopyCountField->get_text() );

    pItem->setValue( "PrintDialog",
                     "Collate",
                     mxCollateBox->get_active() ? OUString("true") :
                                                 OUString("false") );

    pItem->setValue( "PrintDialog",
                     "CollateSingleJobs",
                     mbSingleJobs ? OUString("true") :
                                    OUString("false") );

    pItem->setValue( "PrintDialog",
                     "HasPreview",
                     hasPreview() ? OUString("true") :
                                    OUString("false") );

    pItem->Commit();
}

void PrintDialog::readFromSettings()
{
    SettingsConfigItem* pItem = SettingsConfigItem::get();

    // read last selected tab page; if it exists, activate it
    OUString aValue = pItem->getValue( "PrintDialog",
                              "LastPage" );
    sal_uInt16 nCount = mxTabCtrl->get_n_pages();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        OString sPageId = mxTabCtrl->get_page_ident(i);
        if (aValue == mxTabCtrl->get_tab_label_text(sPageId))
        {
            mxTabCtrl->set_current_page(sPageId);
            break;
        }
    }

    // persistent window state
    aValue = pItem->getValue( "PrintDialog",
                              "WindowState" );
    if (!aValue.isEmpty())
        m_xDialog->set_window_state(OUStringToOString(aValue, RTL_TEXTENCODING_UTF8));

    // collate
    aValue = pItem->getValue( "PrintDialog",
                              "CollateBox" );
    if( aValue.equalsIgnoreAsciiCase("alwaysoff") )
    {
        mbCollateAlwaysOff = true;
        mxCollateBox->set_active( false );
        mxCollateBox->set_sensitive( false );
    }
    else
    {
        mbCollateAlwaysOff = false;
        aValue = pItem->getValue( "PrintDialog",
                                  "Collate" );
        mxCollateBox->set_active( aValue.equalsIgnoreAsciiCase("true") );
    }

    // collate single jobs
    aValue = pItem->getValue( "PrintDialog",
                              "CollateSingleJobs" );
    if ( aValue.equalsIgnoreAsciiCase("true") )
        mbSingleJobs = true;
    else
        mbSingleJobs = false;

    // preview box
    aValue = pItem->getValue( "PrintDialog",
                              "HasPreview" );
    if ( aValue.equalsIgnoreAsciiCase("false") )
        mxPreviewBox->set_active( false );
    else
        mxPreviewBox->set_active( true );

}

void PrintDialog::setPaperSizes()
{
    mxPaperSizeBox->clear();

    VclPtr<Printer> aPrt( maPController->getPrinter() );
    mePaper = aPrt->GetPaper();

    if ( isPrintToFile() )
    {
        mxPaperSizeBox->set_sensitive( false );
    }
    else
    {
        for (int nPaper = 0; nPaper < aPrt->GetPaperInfoCount(); nPaper++)
        {
            PaperInfo aInfo = aPrt->GetPaperInfo( nPaper );
            aInfo.doSloppyFit();
            Paper ePaper = aInfo.getPaper();

            const LocaleDataWrapper& rLocWrap(Application::GetSettings().GetLocaleDataWrapper());
            MapUnit eUnit = MapUnit::MapMM;
            int nDigits = 0;
            if( rLocWrap.getMeasurementSystemEnum() == MeasurementSystem::US )
            {
                eUnit = MapUnit::Map100thInch;
                nDigits = 2;
            }
            Size aSize = aPrt->GetPaperSize( nPaper );
            Size aLogicPaperSize( OutputDevice::LogicToLogic( aSize, MapMode( MapUnit::Map100thMM ), MapMode( eUnit ) ) );

            OUString aWidth( rLocWrap.getNum( aLogicPaperSize.Width(), nDigits ) );
            OUString aHeight( rLocWrap.getNum( aLogicPaperSize.Height(), nDigits ) );
            OUString aUnit = eUnit == MapUnit::MapMM ? OUString("mm") : OUString("in");
            OUString aPaperName = Printer::GetPaperName( ePaper ) + " " + aWidth + aUnit + " x " + aHeight + aUnit;

            mxPaperSizeBox->append_text(aPaperName);

            if ( ePaper == mePaper )
                mxPaperSizeBox->set_active( nPaper );
        }

        mxPaperSizeBox->set_sensitive( true );
    }
}

void PrintDialog::updatePrinterText()
{
    const OUString aDefPrt( Printer::GetDefaultPrinterName() );
    const QueueInfo* pInfo = Printer::GetQueueInfo( mxPrinters->get_active_text(), true );
    if( pInfo )
    {
        // FIXME: status text
        OUString aStatus;
        if( aDefPrt == pInfo->GetPrinterName() )
            aStatus = maDefPrtText;
        mxStatusTxt->set_label( aStatus );
    }
    else
    {
        mxStatusTxt->set_label( OUString() );
    }
}

void PrintDialog::setPreviewText()
{
    OUString aNewText( maPageStr.replaceFirst( "%n", OUString::number( mnCachedPages ) ) );
    mxNumPagesText->set_label( aNewText );
}

void PrintDialog::preparePreview( bool i_bMayUseCache )
{
    VclPtr<Printer> aPrt( maPController->getPrinter() );
    Size aCurPageSize = aPrt->PixelToLogic( aPrt->GetPaperSizePixel(), MapMode( MapUnit::Map100thMM ) );
    // tdf#123076 Get paper size for the preview top label
    mePaper = aPrt->GetPaper();
    GDIMetaFile aMtf;

    // page range may have changed depending on options
    sal_Int32 nPages = maPController->getFilteredPageCount();
    mnCachedPages = nPages;

    setPreviewText();

    if ( !hasPreview() )
    {
        mxPreview->setPreview( aMtf, aCurPageSize,
                            Printer::GetPaperName( mePaper ),
                            maNoPreviewStr,
                            aPrt->GetDPIX(), aPrt->GetDPIY(),
                            aPrt->GetPrinterOptions().IsConvertToGreyscales()
                            );

        mxForwardBtn->set_sensitive( false );
        mxBackwardBtn->set_sensitive( false );
        mxFirstBtn->set_sensitive( false );
        mxLastBtn->set_sensitive( false );

        mxPageEdit->set_sensitive( false );

        return;
    }

    if( mnCurPage >= nPages )
        mnCurPage = nPages-1;
    if( mnCurPage < 0 )
        mnCurPage = 0;


    const MapMode aMapMode( MapUnit::Map100thMM );
    if( nPages > 0 )
    {
        PrinterController::PageSize aPageSize =
            maPController->getFilteredPageFile( mnCurPage, aMtf, i_bMayUseCache );
        if( ! aPageSize.bFullPaper )
        {
            Point aOff( aPrt->PixelToLogic( aPrt->GetPageOffsetPixel(), aMapMode ) );
            aMtf.Move( aOff.X(), aOff.Y() );
        }
    }

    mxPreview->setPreview( aMtf, aCurPageSize,
                                Printer::GetPaperName( mePaper ),
                                nPages > 0 ? OUString() : maNoPageStr,
                                aPrt->GetDPIX(), aPrt->GetDPIY(),
                                aPrt->GetPrinterOptions().IsConvertToGreyscales()
                               );

    mxForwardBtn->set_sensitive( mnCurPage < nPages-1 );
    mxBackwardBtn->set_sensitive( mnCurPage != 0 );
    mxFirstBtn->set_sensitive( mnCurPage != 0 );
    mxLastBtn->set_sensitive( mnCurPage < nPages-1 );
    mxPageEdit->set_sensitive( nPages > 1 );
}

void PrintDialog::updateOrientationBox( const bool bAutomatic )
{
    if ( !bAutomatic )
    {
        Orientation eOrientation = maPController->getPrinter()->GetOrientation();
        mxOrientationBox->set_active( static_cast<sal_Int32>(eOrientation) + 1 );
    }
    else if ( hasOrientationChanged() )
    {
        mxOrientationBox->set_active( ORIENTATION_AUTOMATIC );
    }
}

bool PrintDialog::hasOrientationChanged() const
{
    const int nOrientation = mxOrientationBox->get_active();
    const Orientation eOrientation = maPController->getPrinter()->GetOrientation();

    return (nOrientation == ORIENTATION_LANDSCAPE && eOrientation == Orientation::Portrait)
        || (nOrientation == ORIENTATION_PORTRAIT && eOrientation == Orientation::Landscape);
}

// make sure paper size matches paper orientation
void PrintDialog::checkPaperSize( Size& rPaperSize )
{
    Orientation eOrientation = maPController->getPrinter()->GetOrientation();
    if ( (eOrientation == Orientation::Portrait && rPaperSize.Width() > rPaperSize.Height()) ||
         (eOrientation == Orientation::Landscape && rPaperSize.Width() < rPaperSize.Height()) )
    {
        // coverity[swapped-arguments : FALSE] - this is in the correct order
        rPaperSize = Size( rPaperSize.Height(), rPaperSize.Width() );
    }
}

// Always use this function to set paper orientation to make sure everything behaves well
void PrintDialog::setPaperOrientation( Orientation eOrientation )
{
    VclPtr<Printer> aPrt( maPController->getPrinter() );
    aPrt->SetOrientation( eOrientation );

    // check if it's necessary to swap width and height of paper
    if ( maPController->isPaperSizeFromUser() )
    {
        Size& aPaperSize = maPController->getPaperSizeFromUser();
        checkPaperSize( aPaperSize );
    }
    else if ( maPController->getPapersizeFromSetup() )
    {
        Size& aPaperSize = maPController->getPaperSizeSetup();
        checkPaperSize( aPaperSize );
    }
}

void PrintDialog::checkControlDependencies()
{
    if (mxCopyCountField->get_value() > 1)
        mxCollateBox->set_sensitive( !mbCollateAlwaysOff );
    else
        mxCollateBox->set_sensitive( false );

    OUString aImg(mxCollateBox->get_active() ? OUString(SV_PRINT_COLLATE_BMP) : OUString(SV_PRINT_NOCOLLATE_BMP));

    mxCollateImage->set_from_icon_name(aImg);

    // enable setup button only for printers that can be setup
    bool bHaveSetup = maPController->getPrinter()->HasSupport( PrinterSupport::SetupDialog );
    mxSetupButton->set_sensitive(bHaveSetup);
}

void PrintDialog::checkOptionalControlDependencies()
{
    for( const auto& rEntry : maControlToPropertyMap )
    {
        bool bShouldbeEnabled = maPController->isUIOptionEnabled( rEntry.second );

        if (bShouldbeEnabled && dynamic_cast<weld::RadioButton*>(rEntry.first))
        {
            auto r_it = maControlToNumValMap.find( rEntry.first );
            if( r_it != maControlToNumValMap.end() )
            {
                bShouldbeEnabled = maPController->isUIChoiceEnabled( rEntry.second, r_it->second );
            }
        }

        bool bIsEnabled = rEntry.first->get_sensitive();
        // Enable does not do a change check first, so can be less cheap than expected
        if (bShouldbeEnabled != bIsEnabled)
            rEntry.first->set_sensitive( bShouldbeEnabled );
    }
}

void PrintDialog::initFromMultiPageSetup( const vcl::PrinterController::MultiPageSetup& i_rMPS )
{
    mxNupOrderWin->show();
    mxPagesBtn->set_active(true);
    mxBrochureBtn->hide();

    // setup field units for metric fields
    const LocaleDataWrapper& rLocWrap(Application::GetSettings().GetLocaleDataWrapper());
    FieldUnit eUnit = FieldUnit::MM;
    sal_uInt16 nDigits = 0;
    if( rLocWrap.getMeasurementSystemEnum() == MeasurementSystem::US )
    {
        eUnit = FieldUnit::INCH;
        nDigits = 2;
    }
    // set units
    mxPageMarginEdt->set_unit( eUnit );
    mxSheetMarginEdt->set_unit( eUnit );

    // set precision
    mxPageMarginEdt->set_digits( nDigits );
    mxSheetMarginEdt->set_digits( nDigits );

    mxSheetMarginEdt->set_value( mxSheetMarginEdt->normalize( i_rMPS.nLeftMargin ), FieldUnit::MM_100TH );
    mxPageMarginEdt->set_value( mxPageMarginEdt->normalize( i_rMPS.nHorizontalSpacing ), FieldUnit::MM_100TH );
    mxBorderCB->set_active( i_rMPS.bDrawBorder );
    mxNupRowsEdt->set_value( i_rMPS.nRows );
    mxNupColEdt->set_value( i_rMPS.nColumns );
    mxNupOrderBox->set_active( static_cast<sal_Int32>(i_rMPS.nOrder) );
    if( i_rMPS.nRows != 1 || i_rMPS.nColumns != 1 )
    {
        mxNupPagesBox->set_active( mxNupPagesBox->get_count()-1 );
        showAdvancedControls( true );
        mxNupOrder->setValues( i_rMPS.nOrder, i_rMPS.nColumns, i_rMPS.nRows );
    }
}

void PrintDialog::updateNup( bool i_bMayUseCache )
{
    int nRows         = mxNupRowsEdt->get_value();
    int nCols         = mxNupColEdt->get_value();
    long nPageMargin  = mxPageMarginEdt->denormalize(mxPageMarginEdt->get_value( FieldUnit::MM_100TH ));
    long nSheetMargin = mxSheetMarginEdt->denormalize(mxSheetMarginEdt->get_value( FieldUnit::MM_100TH ));

    PrinterController::MultiPageSetup aMPS;
    aMPS.nRows         = nRows;
    aMPS.nColumns      = nCols;
    aMPS.nLeftMargin   =
    aMPS.nTopMargin    =
    aMPS.nRightMargin  =
    aMPS.nBottomMargin = nSheetMargin;

    aMPS.nHorizontalSpacing =
    aMPS.nVerticalSpacing   = nPageMargin;

    aMPS.bDrawBorder        = mxBorderCB->get_active();

    aMPS.nOrder = static_cast<NupOrderType>(mxNupOrderBox->get_active());

    int nOrientationMode = mxOrientationBox->get_active();
    if( nOrientationMode == ORIENTATION_LANDSCAPE )
        aMPS.aPaperSize = maNupLandscapeSize;
    else if( nOrientationMode == ORIENTATION_PORTRAIT )
        aMPS.aPaperSize = maNupPortraitSize;
    else // automatic mode
    {
        // get size of first real page to see if it is portrait or landscape
        // we assume same page sizes for all the pages for this
        Size aPageSize = getJobPageSize();

        Size aMultiSize( aPageSize.Width() * nCols, aPageSize.Height() * nRows );
        if( aMultiSize.Width() > aMultiSize.Height() ) // fits better on landscape
        {
            aMPS.aPaperSize = maNupLandscapeSize;
            setPaperOrientation( Orientation::Landscape );
        }
        else
        {
            aMPS.aPaperSize = maNupPortraitSize;
            setPaperOrientation( Orientation::Portrait );
        }
    }

    maPController->setMultipage( aMPS );

    mxNupOrder->setValues( aMPS.nOrder, nCols, nRows );

    preparePreview( i_bMayUseCache );
}

void PrintDialog::updateNupFromPages( bool i_bMayUseCache )
{
    int nPages = mxNupPagesBox->get_active_id().toInt32();
    int nRows   = mxNupRowsEdt->get_value();
    int nCols   = mxNupColEdt->get_value();
    long nPageMargin  = mxPageMarginEdt->denormalize(mxPageMarginEdt->get_value( FieldUnit::MM_100TH ));
    long nSheetMargin = mxSheetMarginEdt->denormalize(mxSheetMarginEdt->get_value( FieldUnit::MM_100TH ));
    bool bCustom = false;

    if( nPages == 1 )
    {
        nRows = nCols = 1;
        nSheetMargin = 0;
        nPageMargin = 0;
    }
    else if( nPages == 2 || nPages == 4 || nPages == 6 || nPages == 9 || nPages == 16 )
    {
        Size aJobPageSize( getJobPageSize() );
        bool bPortrait = aJobPageSize.Width() < aJobPageSize.Height();
        if( nPages == 2 )
        {
            if( bPortrait )
            {
                nRows = 1;
                nCols = 2;
            }
            else
            {
                nRows = 2;
                nCols = 1;
            }
        }
        else if( nPages == 4 )
            nRows = nCols = 2;
        else if( nPages == 6 )
        {
            if( bPortrait )
            {
                nRows = 2;
                nCols = 3;
            }
            else
            {
                nRows = 3;
                nCols = 2;
            }
        }
        else if( nPages == 9 )
            nRows = nCols = 3;
        else if( nPages == 16 )
            nRows = nCols = 4;
        nPageMargin = 0;
        nSheetMargin = 0;
    }
    else
        bCustom = true;

    if( nPages > 1 )
    {
        // set upper limits for margins based on job page size and rows/columns
        Size aSize( getJobPageSize() );

        // maximum sheet distance: 1/2 sheet
        long nHorzMax = aSize.Width()/2;
        long nVertMax = aSize.Height()/2;
        if( nSheetMargin > nHorzMax )
            nSheetMargin = nHorzMax;
        if( nSheetMargin > nVertMax )
            nSheetMargin = nVertMax;

        mxSheetMarginEdt->set_max(
                  mxSheetMarginEdt->normalize(
                           std::min(nHorzMax, nVertMax) ), FieldUnit::MM_100TH );

        // maximum page distance
        nHorzMax = (aSize.Width() - 2*nSheetMargin);
        if( nCols > 1 )
            nHorzMax /= (nCols-1);
        nVertMax = (aSize.Height() - 2*nSheetMargin);
        if( nRows > 1 )
            nHorzMax /= (nRows-1);

        if( nPageMargin > nHorzMax )
            nPageMargin = nHorzMax;
        if( nPageMargin > nVertMax )
            nPageMargin = nVertMax;

        mxPageMarginEdt->set_max(
                 mxSheetMarginEdt->normalize(
                           std::min(nHorzMax, nVertMax ) ), FieldUnit::MM_100TH );
    }

    mxNupRowsEdt->set_value( nRows );
    mxNupColEdt->set_value( nCols );
    mxPageMarginEdt->set_value( mxPageMarginEdt->normalize( nPageMargin ), FieldUnit::MM_100TH );
    mxSheetMarginEdt->set_value( mxSheetMarginEdt->normalize( nSheetMargin ), FieldUnit::MM_100TH );

    showAdvancedControls( bCustom );
    updateNup( i_bMayUseCache );
}

void PrintDialog::enableNupControls( bool bEnable )
{
    mxNupPagesBox->set_sensitive( bEnable );
    mxNupNumPagesTxt->set_sensitive( bEnable );
    mxNupColEdt->set_sensitive( bEnable );
    mxNupTimesTxt->set_sensitive( bEnable );
    mxNupRowsEdt->set_sensitive( bEnable );
    mxPageMarginTxt1->set_sensitive( bEnable );
    mxPageMarginEdt->set_sensitive( bEnable );
    mxPageMarginTxt2->set_sensitive( bEnable );
    mxSheetMarginTxt1->set_sensitive( bEnable );
    mxSheetMarginEdt->set_sensitive( bEnable );
    mxSheetMarginTxt2->set_sensitive( bEnable );
    mxNupOrderTxt->set_sensitive( bEnable );
    mxNupOrderBox->set_sensitive( bEnable );
    mxNupOrderWin->set_sensitive( bEnable );
    mxBorderCB->set_sensitive( bEnable );
}

void PrintDialog::showAdvancedControls( bool i_bShow )
{
    mxNupNumPagesTxt->set_visible( i_bShow );
    mxNupColEdt->set_visible( i_bShow );
    mxNupTimesTxt->set_visible( i_bShow );
    mxNupRowsEdt->set_visible( i_bShow );
    mxPageMarginTxt1->set_visible( i_bShow );
    mxPageMarginEdt->set_visible( i_bShow );
    mxPageMarginTxt2->set_visible( i_bShow );
    mxSheetMarginTxt1->set_visible( i_bShow );
    mxSheetMarginEdt->set_visible( i_bShow );
    mxSheetMarginTxt2->set_visible( i_bShow );
}

namespace
{
    void setHelpId( weld::Widget* i_pWindow, const Sequence< OUString >& i_rHelpIds, sal_Int32 i_nIndex )
    {
        if( i_nIndex >= 0 && i_nIndex < i_rHelpIds.getLength() )
            i_pWindow->set_help_id( OUStringToOString( i_rHelpIds.getConstArray()[i_nIndex], RTL_TEXTENCODING_UTF8 ) );
    }

    void setHelpText( weld::Widget* i_pWindow, const Sequence< OUString >& i_rHelpTexts, sal_Int32 i_nIndex )
    {
        // without a help text set and the correct smartID,
        // help texts will be retrieved from the online help system
        if( i_nIndex >= 0 && i_nIndex < i_rHelpTexts.getLength() )
            i_pWindow->set_tooltip_text(i_rHelpTexts.getConstArray()[i_nIndex]);
    }
}

void PrintDialog::setupOptionalUI()
{
    const Sequence< PropertyValue >& rOptions( maPController->getUIOptions() );
    for( const auto& rOption : rOptions )
    {
        if (rOption.Name == "OptionsUIFile")
        {
            OUString sOptionsUIFile;
            rOption.Value >>= sOptionsUIFile;
            mxCustomOptionsUIBuilder.reset(Application::CreateBuilder(mxCustom.get(), sOptionsUIFile));
            std::unique_ptr<weld::Container> xWindow = mxCustomOptionsUIBuilder->weld_container("box");
            xWindow->show();
            continue;
        }

        Sequence< beans::PropertyValue > aOptProp;
        rOption.Value >>= aOptProp;

        // extract ui element
        OUString aCtrlType;
        OString aID;
        OUString aText;
        OUString aPropertyName;
        Sequence< OUString > aChoices;
        Sequence< sal_Bool > aChoicesDisabled;
        Sequence< OUString > aHelpTexts;
        Sequence< OUString > aIDs;
        Sequence< OUString > aHelpIds;
        sal_Int64 nMinValue = 0, nMaxValue = 0;
        OUString aGroupingHint;

        for( const beans::PropertyValue& rEntry : std::as_const(aOptProp) )
        {
            if ( rEntry.Name == "ID" )
            {
                rEntry.Value >>= aIDs;
                aID = OUStringToOString(aIDs[0], RTL_TEXTENCODING_UTF8);
            }
            if ( rEntry.Name == "Text" )
            {
                rEntry.Value >>= aText;
            }
            else if ( rEntry.Name == "ControlType" )
            {
                rEntry.Value >>= aCtrlType;
            }
            else if ( rEntry.Name == "Choices" )
            {
                rEntry.Value >>= aChoices;
            }
            else if ( rEntry.Name == "ChoicesDisabled" )
            {
                rEntry.Value >>= aChoicesDisabled;
            }
            else if ( rEntry.Name == "Property" )
            {
                PropertyValue aVal;
                rEntry.Value >>= aVal;
                aPropertyName = aVal.Name;
            }
            else if ( rEntry.Name == "Enabled" )
            {
            }
            else if ( rEntry.Name == "GroupingHint" )
            {
                rEntry.Value >>= aGroupingHint;
            }
            else if ( rEntry.Name == "DependsOnName" )
            {
            }
            else if ( rEntry.Name == "DependsOnEntry" )
            {
            }
            else if ( rEntry.Name == "AttachToDependency" )
            {
            }
            else if ( rEntry.Name == "MinValue" )
            {
                rEntry.Value >>= nMinValue;
            }
            else if ( rEntry.Name == "MaxValue" )
            {
                rEntry.Value >>= nMaxValue;
            }
            else if ( rEntry.Name == "HelpText" )
            {
                if( ! (rEntry.Value >>= aHelpTexts) )
                {
                    OUString aHelpText;
                    if( rEntry.Value >>= aHelpText )
                    {
                        aHelpTexts.realloc( 1 );
                        *aHelpTexts.getArray() = aHelpText;
                    }
                }
            }
            else if ( rEntry.Name == "HelpId" )
            {
                if( ! (rEntry.Value >>= aHelpIds ) )
                {
                    OUString aHelpId;
                    if( rEntry.Value >>= aHelpId )
                    {
                        aHelpIds.realloc( 1 );
                        *aHelpIds.getArray() = aHelpId;
                    }
                }
            }
            else if ( rEntry.Name == "HintNoLayoutPage" )
            {
                bool bHasLayoutFrame = false;
                rEntry.Value >>= bHasLayoutFrame;
                mbShowLayoutFrame = !bHasLayoutFrame;
            }
        }

        if (aCtrlType == "Group")
        {
            aID = "custom";

            weld::Container* pPage = mxTabCtrl->get_page(aID);
            if (!pPage)
                continue;

            mxTabCtrl->set_tab_label_text(aID, aText);

            // set help id
            if (aHelpIds.hasElements())
                pPage->set_help_id(OUStringToOString(aHelpIds.getConstArray()[0], RTL_TEXTENCODING_UTF8));

            // set help text
            if (aHelpTexts.hasElements())
                pPage->set_tooltip_text(aHelpTexts.getConstArray()[0]);

            pPage->show();
        }
        else if (aCtrlType == "Subgroup" && !aID.isEmpty())
        {
            std::unique_ptr<weld::Widget> xWidget;
            // since 'New Print Dialog Design' fromwhich in calc is not a frame anymore
            if (aID == "fromwhich")
            {
                std::unique_ptr<weld::Label> xLabel = m_xBuilder->weld_label(aID);
                xLabel->set_label(aText);
                xWidget = std::move(xLabel);
            }
            else
            {
                std::unique_ptr<weld::Frame> xFrame = m_xBuilder->weld_frame(aID);
                if (!xFrame && mxCustomOptionsUIBuilder)
                    xFrame = mxCustomOptionsUIBuilder->weld_frame(aID);
                if (xFrame)
                {
                    xFrame->set_label(aText);
                    xWidget = std::move(xFrame);
                }
            }

            if (!xWidget)
                continue;

            // set help id
            setHelpId(xWidget.get(), aHelpIds, 0);
            // set help text
            setHelpText(xWidget.get(), aHelpTexts, 0);

            xWidget->show();
        }
        // EVIL
        else if( aCtrlType == "Bool" && aGroupingHint == "LayoutPage" && aPropertyName == "PrintProspect" )
        {
            mxBrochureBtn->set_label(aText);
            mxBrochureBtn->show();

            bool bVal = false;
            PropertyValue* pVal = maPController->getValue( aPropertyName );
            if( pVal )
                pVal->Value >>= bVal;
            mxBrochureBtn->set_active( bVal );
            mxBrochureBtn->set_sensitive( maPController->isUIOptionEnabled( aPropertyName ) && pVal != nullptr );
            mxBrochureBtn->connect_toggled( LINK( this, PrintDialog, ToggleHdl ) );

            maPropertyToWindowMap[aPropertyName].emplace_back(mxBrochureBtn.get());
            maControlToPropertyMap[mxBrochureBtn.get()] = aPropertyName;

            // set help id
            setHelpId( mxBrochureBtn.get(), aHelpIds, 0 );
            // set help text
            setHelpText( mxBrochureBtn.get(), aHelpTexts, 0 );
        }
        else if (aCtrlType == "Bool")
        {
            // add a check box
            std::unique_ptr<weld::CheckButton> xNewBox = m_xBuilder->weld_check_button(aID);
            if (!xNewBox && mxCustomOptionsUIBuilder)
                xNewBox = mxCustomOptionsUIBuilder->weld_check_button(aID);
            if (!xNewBox)
                continue;

            xNewBox->set_label( aText );
            xNewBox->show();

            bool bVal = false;
            PropertyValue* pVal = maPController->getValue( aPropertyName );
            if( pVal )
                pVal->Value >>= bVal;
            xNewBox->set_active( bVal );
            xNewBox->connect_toggled( LINK( this, PrintDialog, UIOption_CheckHdl ) );

            maExtraControls.emplace_back(std::move(xNewBox));

            weld::Widget* pWidget = maExtraControls.back().get();

            maPropertyToWindowMap[aPropertyName].emplace_back(pWidget);
            maControlToPropertyMap[pWidget] = aPropertyName;

            // set help id
            setHelpId(pWidget, aHelpIds, 0);
            // set help text
            setHelpText(pWidget, aHelpTexts, 0);
        }
        else if (aCtrlType == "Radio")
        {
            sal_Int32 nCurHelpText = 0;

            // iterate options
            sal_Int32 nSelectVal = 0;
            PropertyValue* pVal = maPController->getValue( aPropertyName );
            if( pVal && pVal->Value.hasValue() )
                pVal->Value >>= nSelectVal;
            for( sal_Int32 m = 0; m < aChoices.getLength(); m++ )
            {
                aID = OUStringToOString(aIDs[m], RTL_TEXTENCODING_UTF8);
                std::unique_ptr<weld::RadioButton> xBtn = m_xBuilder->weld_radio_button(aID);
                if (!xBtn && mxCustomOptionsUIBuilder)
                    xBtn = mxCustomOptionsUIBuilder->weld_radio_button(aID);
                if (!xBtn)
                    continue;

                xBtn->set_label( aChoices[m] );
                xBtn->set_active( m == nSelectVal );
                xBtn->connect_toggled( LINK( this, PrintDialog, UIOption_RadioHdl ) );
                if( aChoicesDisabled.getLength() > m && aChoicesDisabled[m] )
                    xBtn->set_sensitive( false );
                xBtn->show();

                maExtraControls.emplace_back(std::move(xBtn));

                weld::Widget* pWidget = maExtraControls.back().get();

                maPropertyToWindowMap[ aPropertyName ].emplace_back(pWidget);
                maControlToPropertyMap[pWidget] = aPropertyName;
                maControlToNumValMap[pWidget] = m;

                // set help id
                setHelpId( pWidget, aHelpIds, nCurHelpText );
                // set help text
                setHelpText( pWidget, aHelpTexts, nCurHelpText );
                nCurHelpText++;
            }
        }
        else if ( aCtrlType == "List" )
        {
            std::unique_ptr<weld::ComboBox> xList = m_xBuilder->weld_combo_box(aID);
            if (!xList && mxCustomOptionsUIBuilder)
                xList = mxCustomOptionsUIBuilder->weld_combo_box(aID);
            if (!xList)
                continue;

            // iterate options
            for( const auto& rChoice : std::as_const(aChoices) )
                xList->append_text(rChoice);

            sal_Int32 nSelectVal = 0;
            PropertyValue* pVal = maPController->getValue( aPropertyName );
            if( pVal && pVal->Value.hasValue() )
                pVal->Value >>= nSelectVal;
            xList->set_active(nSelectVal);
            xList->connect_changed( LINK( this, PrintDialog, UIOption_SelectHdl ) );
            xList->show();

            maExtraControls.emplace_back(std::move(xList));

            weld::Widget* pWidget = maExtraControls.back().get();

            maPropertyToWindowMap[ aPropertyName ].emplace_back(pWidget);
            maControlToPropertyMap[pWidget] = aPropertyName;

            // set help id
            setHelpId( pWidget, aHelpIds, 0 );
            // set help text
            setHelpText( pWidget, aHelpTexts, 0 );
        }
        else if ( aCtrlType == "Range" )
        {
            std::unique_ptr<weld::SpinButton> xField = m_xBuilder->weld_spin_button(aID);
            if (!xField && mxCustomOptionsUIBuilder)
                xField = mxCustomOptionsUIBuilder->weld_spin_button(aID);
            if (!xField)
                continue;

            // set min/max and current value
            if(nMinValue != nMaxValue)
                xField->set_range(nMinValue, nMaxValue);

            sal_Int64 nCurVal = 0;
            PropertyValue* pVal = maPController->getValue( aPropertyName );
            if( pVal && pVal->Value.hasValue() )
                pVal->Value >>= nCurVal;
            xField->set_value( nCurVal );
            xField->connect_value_changed( LINK( this, PrintDialog, UIOption_SpinModifyHdl ) );
            xField->show();

            maExtraControls.emplace_back(std::move(xField));

            weld::Widget* pWidget = maExtraControls.back().get();

            maPropertyToWindowMap[ aPropertyName ].emplace_back(pWidget);
            maControlToPropertyMap[pWidget] = aPropertyName;

            // set help id
            setHelpId( pWidget, aHelpIds, 0 );
            // set help text
            setHelpText( pWidget, aHelpTexts, 0 );
        }
        else if (aCtrlType == "Edit")
        {
            std::unique_ptr<weld::Entry> xField = m_xBuilder->weld_entry(aID);
            if (!xField && mxCustomOptionsUIBuilder)
                xField = mxCustomOptionsUIBuilder->weld_entry(aID);
            if (!xField)
                continue;

            OUString aCurVal;
            PropertyValue* pVal = maPController->getValue( aPropertyName );
            if( pVal && pVal->Value.hasValue() )
                pVal->Value >>= aCurVal;
            xField->set_text( aCurVal );
            xField->connect_changed( LINK( this, PrintDialog, UIOption_EntryModifyHdl ) );
            xField->show();

            maExtraControls.emplace_back(std::move(xField));

            weld::Widget* pWidget = maExtraControls.back().get();

            maPropertyToWindowMap[ aPropertyName ].emplace_back(pWidget);
            maControlToPropertyMap[pWidget] = aPropertyName;

            // set help id
            setHelpId( pWidget, aHelpIds, 0 );
            // set help text
            setHelpText( pWidget, aHelpTexts, 0 );
        }
        else
        {
            SAL_WARN( "vcl", "Unsupported UI option: \"" << aCtrlType << '"');
        }
    }

    // #i106506# if no brochure button, then the singular Pages radio button
    // makes no sense, so replace it by a FixedText label
    if (!mxBrochureBtn->get_visible() && mxPagesBtn->get_visible())
    {
        mxPagesBoxTitleTxt->set_label(mxPagesBtn->get_label());
        mxPagesBoxTitleTxt->show();
        mxPagesBtn->hide();

        mxPagesBoxTitleTxt->set_accessible_relation_label_for(mxNupPagesBox.get());
        mxNupPagesBox->set_accessible_relation_labeled_by(mxPagesBoxTitleTxt.get());
        mxPagesBtn->set_accessible_relation_label_for(nullptr);
    }

    // update enable states
    checkOptionalControlDependencies();

    // print range not shown (currently math only) -> hide spacer line and reverse order
    if (!mxPageRangeEdit->get_visible())
    {
        mxReverseOrderBox->hide();
    }

    if (!mxCustomOptionsUIBuilder)
        mxTabCtrl->remove_page(mxTabCtrl->get_page_ident(1));
}

void PrintDialog::makeEnabled( weld::Widget* i_pWindow )
{
    auto it = maControlToPropertyMap.find( i_pWindow );
    if( it != maControlToPropertyMap.end() )
    {
        OUString aDependency( maPController->makeEnabled( it->second ) );
        if( !aDependency.isEmpty() )
            updateWindowFromProperty( aDependency );
    }
}

void PrintDialog::updateWindowFromProperty( const OUString& i_rProperty )
{
    beans::PropertyValue* pValue = maPController->getValue( i_rProperty );
    auto it = maPropertyToWindowMap.find( i_rProperty );
    if( pValue && it != maPropertyToWindowMap.end() )
    {
        const auto& rWindows( it->second );
        if( ! rWindows.empty() )
        {
            bool bVal = false;
            sal_Int32 nVal = -1;
            if( pValue->Value >>= bVal )
            {
                // we should have a CheckBox for this one
                weld::CheckButton* pBox = dynamic_cast<weld::CheckButton*>(rWindows.front());
                if( pBox )
                {
                    pBox->set_active( bVal );
                }
                else if ( i_rProperty == "PrintProspect" )
                {
                    // EVIL special case
                    if( bVal )
                        mxBrochureBtn->set_active(true);
                    else
                        mxPagesBtn->set_active(true);
                }
                else
                {
                    SAL_WARN( "vcl", "missing a checkbox" );
                }
            }
            else if( pValue->Value >>= nVal )
            {
                // this could be a ListBox or a RadioButtonGroup
                weld::ComboBox* pList = dynamic_cast<weld::ComboBox*>(rWindows.front());
                if( pList )
                {
                    pList->set_active( static_cast< sal_uInt16 >(nVal) );
                }
                else if( nVal >= 0 && nVal < sal_Int32(rWindows.size() ) )
                {
                    weld::RadioButton* pBtn = dynamic_cast<weld::RadioButton*>(rWindows[nVal]);
                    SAL_WARN_IF( !pBtn, "vcl", "unexpected control for property" );
                    if( pBtn )
                        pBtn->set_active(true);
                }
            }
        }
    }
}

bool PrintDialog::isPrintToFile() const
{
    return ( mxPrinters->get_active() == 0 );
}

bool PrintDialog::isCollate() const
{
    return mxCopyCountField->get_value() > 1 && mxCollateBox->get_active();
}

bool PrintDialog::hasPreview() const
{
    return mxPreviewBox->get_active();
}

PropertyValue* PrintDialog::getValueForWindow( weld::Widget* i_pWindow ) const
{
    PropertyValue* pVal = nullptr;
    auto it = maControlToPropertyMap.find( i_pWindow );
    if( it != maControlToPropertyMap.end() )
    {
        pVal = maPController->getValue( it->second );
        SAL_WARN_IF( !pVal, "vcl", "property value not found" );
    }
    else
    {
        OSL_FAIL( "changed control not in property map" );
    }
    return pVal;
}

IMPL_LINK(PrintDialog, ToggleHdl, weld::ToggleButton&, rButton, void)
{
    ClickHdl(rButton);
}

IMPL_LINK(PrintDialog, ClickHdl, weld::Button&, rButton, void)
{
    if (&rButton == mxOKButton.get() || &rButton == mxCancelButton.get())
    {
        storeToSettings();
        m_xDialog->response(&rButton == mxOKButton.get() ? RET_OK : RET_CANCEL);
    }
    else if( &rButton == mxHelpButton.get() )
    {
        // start help system
        Help* pHelp = Application::GetHelp();
        if( pHelp )
        {
            pHelp->Start("vcl/ui/printdialog/PrintDialog", mxOKButton.get());
        }
    }
    else if ( &rButton == mxPreviewBox.get() )
    {
        preparePreview( true );
    }
    else if( &rButton == mxForwardBtn.get() )
    {
        previewForward();
    }
    else if( &rButton == mxBackwardBtn.get() )
    {
        previewBackward();
    }
    else if( &rButton == mxFirstBtn.get() )
    {
        previewFirst();
    }
    else if( &rButton == mxLastBtn.get() )
    {
        previewLast();
    }
    else if( &rButton == mxBrochureBtn.get() )
    {
        PropertyValue* pVal = getValueForWindow( &rButton );
        if( pVal )
        {
            bool bVal = mxBrochureBtn->get_active();
            pVal->Value <<= bVal;

            checkOptionalControlDependencies();

            // update preview and page settings
            preparePreview(false);
        }
        if( mxBrochureBtn->get_active() )
        {
            mxOrientationBox->set_sensitive( false );
            mxOrientationBox->set_active( ORIENTATION_LANDSCAPE );
            mxNupPagesBox->set_active( 0 );
            updateNupFromPages();
            showAdvancedControls( false );
            enableNupControls( false );
        }
    }
    else if( &rButton == mxPagesBtn.get() )
    {
        mxOrientationBox->set_sensitive( true );
        mxOrientationBox->set_active( ORIENTATION_AUTOMATIC );
        enableNupControls( true );
        updateNupFromPages();
    }
    else if( &rButton == mxCollateBox.get() )
    {
        maPController->setValue( "Collate",
                                 makeAny( isCollate() ) );
        checkControlDependencies();
    }
    else if( &rButton == mxReverseOrderBox.get() )
    {
        bool bChecked = mxReverseOrderBox->get_active();
        maPController->setReversePrint( bChecked );
        maPController->setValue( "PrintReverse",
                                 makeAny( bChecked ) );
        preparePreview( true );
    }
    else if( &rButton == mxBorderCB.get() )
    {
        updateNup();
    }
    else if ( &rButton == mxMoreOptionsBtn.get() )
    {
        mxMoreOptionsDlg.reset(new MoreOptionsDialog(this));
        mxMoreOptionsDlg->run();
    }
    else
    {
        if( &rButton == mxSetupButton.get() )
        {
            maPController->setupPrinter(m_xDialog.get());

            if ( !isPrintToFile() )
            {
                VclPtr<Printer> aPrt( maPController->getPrinter() );
                mePaper = aPrt->GetPaper();

                for (int nPaper = 0; nPaper < aPrt->GetPaperInfoCount(); nPaper++ )
                {
                    PaperInfo aInfo = aPrt->GetPaperInfo( nPaper );
                    aInfo.doSloppyFit();
                    Paper ePaper = aInfo.getPaper();

                    if ( mePaper == ePaper )
                    {
                        mxPaperSizeBox->set_active( nPaper );
                        break;
                    }
                }
            }

            updateOrientationBox( false );
            setupPaperSidesBox();

            // tdf#63905 don't use cache: page size may change
            preparePreview(false);
        }
        checkControlDependencies();
    }

}

IMPL_LINK( PrintDialog, SelectHdl, weld::ComboBox&, rBox, void )
{
    if (&rBox == mxPrinters.get())
    {
        if ( !isPrintToFile() )
        {
            OUString aNewPrinter(rBox.get_active_text());
            // set new printer
            maPController->setPrinter( VclPtrInstance<Printer>( aNewPrinter ) );
            maPController->resetPrinterOptions( false  );

            updateOrientationBox();

            // update text fields
            mxOKButton->set_label(maPrintText);
            updatePrinterText();
            setPaperSizes();
            preparePreview(true);
        }
        else // print to file
        {
            // use the default printer or FIXME: the last used one?
            maPController->setPrinter( VclPtrInstance<Printer>( Printer::GetDefaultPrinterName() ) );
            mxOKButton->set_label(maPrintToFileText);
            maPController->resetPrinterOptions( true );

            setPaperSizes();
            updateOrientationBox();
            preparePreview( true );
        }

        setupPaperSidesBox();
    }
    else if ( &rBox == mxPaperSidesBox.get() )
    {
        DuplexMode eDuplex = static_cast<DuplexMode>(mxPaperSidesBox->get_active() + 1);
        maPController->getPrinter()->SetDuplexMode( eDuplex );
    }
    else if( &rBox == mxOrientationBox.get() )
    {
        int nOrientation = mxOrientationBox->get_active();
        if ( nOrientation != ORIENTATION_AUTOMATIC )
            setPaperOrientation( static_cast<Orientation>( nOrientation - 1 ) );

        updateNup( false );
    }
    else if ( &rBox == mxNupOrderBox.get() )
    {
        updateNup();
    }
    else if( &rBox == mxNupPagesBox.get() )
    {
        if( !mxPagesBtn->get_active() )
            mxPagesBtn->set_active(true);
        updateNupFromPages( false );
    }
    else if ( &rBox == mxPaperSizeBox.get() )
    {
        VclPtr<Printer> aPrt( maPController->getPrinter() );
        PaperInfo aInfo = aPrt->GetPaperInfo( rBox.get_active() );
        aInfo.doSloppyFit();
        mePaper = aInfo.getPaper();

        if ( mePaper == PAPER_USER )
            aPrt->SetPaperSizeUser( Size( aInfo.getWidth(), aInfo.getHeight() ) );
        else
            aPrt->SetPaper( mePaper );

        Size aPaperSize( aInfo.getWidth(), aInfo.getHeight() );
        checkPaperSize( aPaperSize );
        maPController->setPaperSizeFromUser( aPaperSize );

        preparePreview(true);
    }
}

IMPL_LINK_NOARG(PrintDialog, MetricSpinModifyHdl, weld::MetricSpinButton&, void)
{
    checkControlDependencies();
    updateNupFromPages();
}

IMPL_LINK_NOARG(PrintDialog, FocusOutHdl, weld::Widget&, void)
{
    ActivateHdl(*mxPageEdit);
}

IMPL_LINK_NOARG(PrintDialog, ActivateHdl, weld::Entry&, bool)
{
    sal_Int32 nPage = mxPageEdit->get_text().toInt32();
    if (nPage < 1)
    {
        nPage = 1;
        mxPageEdit->set_text("1");
    }
    else if (nPage > mnCachedPages)
    {
        nPage = mnCachedPages;
        mxPageEdit->set_text(OUString::number(mnCachedPages));
    }
    int nNewCurPage = nPage - 1;
    if (nNewCurPage != mnCurPage)
    {
        mnCurPage = nNewCurPage;
        preparePreview(true);
    }
    return true;
}

IMPL_LINK( PrintDialog, SpinModifyHdl, weld::SpinButton&, rEdit, void )
{
    checkControlDependencies();
    if (&rEdit == mxNupRowsEdt.get() || &rEdit == mxNupColEdt.get())
    {
       updateNupFromPages();
    }
    else if( &rEdit == mxCopyCountField.get() )
    {
        maPController->setValue( "CopyCount",
                               makeAny( sal_Int32(mxCopyCountField->get_value()) ) );
        maPController->setValue( "Collate",
                               makeAny( isCollate() ) );
    }
}

IMPL_LINK( PrintDialog, UIOption_CheckHdl, weld::ToggleButton&, i_rBox, void )
{
    PropertyValue* pVal = getValueForWindow( &i_rBox );
    if( pVal )
    {
        makeEnabled( &i_rBox );

        bool bVal = i_rBox.get_active();
        pVal->Value <<= bVal;

        checkOptionalControlDependencies();

        // update preview and page settings
        preparePreview(false);
    }
}

IMPL_LINK( PrintDialog, UIOption_RadioHdl, weld::ToggleButton&, i_rBtn, void )
{
    // this handler gets called for all radiobuttons that get unchecked, too
    // however we only want one notification for the new value (that is for
    // the button that gets checked)
    if( i_rBtn.get_active() )
    {
        PropertyValue* pVal = getValueForWindow( &i_rBtn );
        auto it = maControlToNumValMap.find( &i_rBtn );
        if( pVal && it != maControlToNumValMap.end() )
        {
            makeEnabled( &i_rBtn );

            sal_Int32 nVal = it->second;
            pVal->Value <<= nVal;

            updateOrientationBox();

            checkOptionalControlDependencies();

            // tdf#41205 give focus to the page range edit if the corresponding radio button was selected
            if (pVal->Name == "PrintContent" && mxPageRangesRadioButton->get_active())
                mxPageRangeEdit->grab_focus();

            // update preview and page settings
            preparePreview(false);
        }
    }
}

IMPL_LINK( PrintDialog, UIOption_SelectHdl, weld::ComboBox&, i_rBox, void )
{
    PropertyValue* pVal = getValueForWindow( &i_rBox );
    if( pVal )
    {
        makeEnabled( &i_rBox );

        sal_Int32 nVal( i_rBox.get_active() );
        pVal->Value <<= nVal;

        //If we are in impress we start in print slides mode and get a
        //maFirstPageSize for slides which are usually landscape mode, if we
        //change to notes which are usually in portrait mode, and then visit
        //n-up print, we will assume notes are in landscape unless we throw
        //away maFirstPageSize when we change page content type
        if (pVal->Name == "PageContentType")
            maFirstPageSize = Size();

        checkOptionalControlDependencies();

        // update preview and page settings
        preparePreview(false);
    }
}

IMPL_LINK( PrintDialog, UIOption_SpinModifyHdl, weld::SpinButton&, i_rBox, void )
{
    PropertyValue* pVal = getValueForWindow( &i_rBox );
    if( pVal )
    {
        makeEnabled( &i_rBox );

        sal_Int64 nVal = i_rBox.get_value();
        pVal->Value <<= nVal;

        checkOptionalControlDependencies();

        // update preview and page settings
        preparePreview(false);
    }
}

IMPL_LINK( PrintDialog, UIOption_EntryModifyHdl, weld::Entry&, i_rBox, void )
{
    PropertyValue* pVal = getValueForWindow( &i_rBox );
    if( pVal )
    {
        makeEnabled( &i_rBox );

        OUString aVal( i_rBox.get_text() );
        pVal->Value <<= aVal;

        checkOptionalControlDependencies();

        // update preview and page settings
        preparePreview(false);
    }
}

void PrintDialog::previewForward()
{
    sal_Int32 nValue = mxPageEdit->get_text().toInt32() + 1;
    if (nValue <= mnCachedPages)
    {
        mxPageEdit->set_text(OUString::number(nValue));
        ActivateHdl(*mxPageEdit);
    }
}

void PrintDialog::previewBackward()
{
    sal_Int32 nValue = mxPageEdit->get_text().toInt32() - 1;
    if (nValue >= 1)
    {
        mxPageEdit->set_text(OUString::number(nValue));
        ActivateHdl(*mxPageEdit);
    }
}

void PrintDialog::previewFirst()
{
    mxPageEdit->set_text("1");
    ActivateHdl(*mxPageEdit);
}

void PrintDialog::previewLast()
{
    mxPageEdit->set_text(OUString::number(mnCachedPages));
    ActivateHdl(*mxPageEdit);
}

// PrintProgressDialog
PrintProgressDialog::PrintProgressDialog(weld::Window* i_pParent, int i_nMax)
    : GenericDialogController(i_pParent, "vcl/ui/printprogressdialog.ui", "PrintProgressDialog")
    , mbCanceled(false)
    , mnCur(0)
    , mnMax(i_nMax)
    , mxText(m_xBuilder->weld_label("label"))
    , mxProgress(m_xBuilder->weld_progress_bar("progressbar"))
    , mxButton(m_xBuilder->weld_button("cancel"))
{
    if( mnMax < 1 )
        mnMax = 1;

    maStr = mxText->get_label();

    //just multiply largest value by 10 and take the width of that string as
    //the max size we will want
    OUString aNewText( maStr.replaceFirst( "%p", OUString::number( mnMax * 10 ) ) );
    aNewText = aNewText.replaceFirst( "%n", OUString::number( mnMax * 10 ) );
    mxText->set_label( aNewText );
    mxText->set_size_request(mxText->get_preferred_size().Width(), -1);

    //Pick a useful max width
    mxProgress->set_size_request(mxProgress->get_approximate_digit_width() * 25, -1);

    mxButton->connect_clicked( LINK( this, PrintProgressDialog, ClickHdl ) );
}

PrintProgressDialog::~PrintProgressDialog()
{
}

IMPL_LINK_NOARG(PrintProgressDialog, ClickHdl, weld::Button&, void)
{
    mbCanceled = true;
}

void PrintProgressDialog::setProgress( int i_nCurrent )
{
    mnCur = i_nCurrent;

    if( mnMax < 1 )
        mnMax = 1;

    mxProgress->set_percentage(mnCur*100/mnMax);

    OUString aNewText( maStr.replaceFirst( "%p", OUString::number( mnCur ) ) );
    aNewText = aNewText.replaceFirst( "%n", OUString::number( mnMax ) );
    mxText->set_label( aNewText );
}

void PrintProgressDialog::tick()
{
    if( mnCur < mnMax )
        setProgress( ++mnCur );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
