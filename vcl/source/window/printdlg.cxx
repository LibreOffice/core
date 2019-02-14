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

#include <vcl/print.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/wall.hxx>
#include <vcl/status.hxx>
#include <vcl/decoview.hxx>
#include <vcl/configsettings.hxx>
#include <vcl/help.hxx>
#include <vcl/layout.hxx>
#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/lstbox.hxx>
#include <jobset.h>

#include <unotools/localedatawrapper.hxx>

#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/Size.hpp>

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

extern "C" SAL_DLLPUBLIC_EXPORT void makePrintPreviewWindow(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap &)
{
    rRet = VclPtr<PrintDialog::PrintPreviewWindow>::Create(pParent);
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeShowNupOrderWindow(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap &)
{
    rRet = VclPtr<PrintDialog::ShowNupOrderWindow>::Create(pParent);
}

namespace {
   bool lcl_ListBoxCompare( const OUString& rStr1, const OUString& rStr2 )
   {
       return ListBox::NaturalSortCompare( rStr1, rStr2 ) < 0;
   }
}

MoreOptionsDialog::MoreOptionsDialog(VclPtr<PrintDialog> i_pParent)
    : GenericDialogController(i_pParent->GetFrameWeld(), "vcl/ui/moreoptionsdialog.ui", "MoreOptionsDialog")
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

PrintDialog::PrintPreviewWindow::PrintPreviewWindow( vcl::Window* i_pParent )
    : Window( i_pParent, 0 )
    , maMtf()
    , maOrigSize( 10, 10 )
    , maPreviewSize()
    , mnDPIX(Application::GetDefaultDevice()->GetDPIX())
    , mnDPIY(Application::GetDefaultDevice()->GetDPIY())
    , maPreviewBitmap()
    , maReplacementString()
    , maToolTipString(VclResId( SV_PRINT_PRINTPREVIEW_TXT))
    , mbGreyscale( false )
    , maHorzDim(VclPtr<FixedLine>::Create(this, WB_HORZ | WB_CENTER))
    , maVertDim(VclPtr<FixedLine>::Create(this, WB_VERT | WB_VCENTER))
{
    SetPaintTransparent( true );
    SetBackground();
    maHorzDim->Show();
    maVertDim->Show();

    maHorzDim->SetText( "2.0in" );
    maVertDim->SetText( "2.0in" );
}

PrintDialog::PrintPreviewWindow::~PrintPreviewWindow()
{
    disposeOnce();
}

void PrintDialog::PrintPreviewWindow::dispose()
{
    maHorzDim.disposeAndClear();
    maVertDim.disposeAndClear();
    Window::dispose();
}

void PrintDialog::PrintPreviewWindow::Resize()
{
    Size aNewSize( GetSizePixel() );
    long nTextHeight = maHorzDim->GetTextHeight();
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

    // position dimension lines
    Point aRef( nTextHeight + (aNewSize.Width() - maPreviewSize.Width())/2,
                nTextHeight + (aNewSize.Height() - maPreviewSize.Height())/2 );
    maHorzDim->SetPosSizePixel( Point( aRef.X(), aRef.Y() - nTextHeight ),
                               Size( maPreviewSize.Width(), nTextHeight ) );
    maVertDim->SetPosSizePixel( Point( aRef.X() - nTextHeight, aRef.Y() ),
                               Size( nTextHeight, maPreviewSize.Height() ) );

    // check and evtl. recreate preview bitmap
    preparePreviewBitmap();
}

void PrintDialog::PrintPreviewWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    long nTextHeight = maHorzDim->GetTextHeight();
    Size aSize(GetSizePixel());
    Point aOffset((aSize.Width()  - maPreviewSize.Width()  + nTextHeight) / 2,
                  (aSize.Height() - maPreviewSize.Height() + nTextHeight) / 2);

    if (!maReplacementString.isEmpty())
    {
        // replacement is active
        rRenderContext.Push();
        Font aFont(rRenderContext.GetSettings().GetStyleSettings().GetLabelFont());
        SetZoomedPointFont(rRenderContext, aFont);
        tools::Rectangle aTextRect(aOffset + Point(2, 2), Size(maPreviewSize.Width() - 4, maPreviewSize.Height() - 4));
        rRenderContext.DrawText(aTextRect, maReplacementString,
                                DrawTextFlags::Center | DrawTextFlags::VCenter |
                                DrawTextFlags::WordBreak | DrawTextFlags::MultiLine);
        rRenderContext.Pop();
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
}

void PrintDialog::PrintPreviewWindow::Command( const CommandEvent& rEvt )
{
    if( rEvt.GetCommand() == CommandEventId::Wheel )
    {
        const CommandWheelData* pWheelData = rEvt.GetWheelData();
        PrintDialog* pDlg = dynamic_cast<PrintDialog*>(GetParentDialog());
        if( pDlg )
        {
            if( pWheelData->GetDelta() > 0 )
                pDlg->previewForward();
            else if( pWheelData->GetDelta() < 0 )
                pDlg->previewBackward();
        }
    }
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
    OUStringBuffer aBuf( 256 );
    aBuf.append( maToolTipString );
    SetQuickHelpText( aBuf.makeStringAndClear() );
    maMtf = i_rNewPreview;
    mnDPIX = i_nDPIX;
    mnDPIY = i_nDPIY;
    maOrigSize = i_rOrigSize;
    maReplacementString = i_rReplacement;
    mbGreyscale = i_bGreyscale;

    // use correct measurements
    const LocaleDataWrapper& rLocWrap( GetSettings().GetLocaleDataWrapper() );
    MapUnit eUnit = MapUnit::MapMM;
    int nDigits = 0;
    if( rLocWrap.getMeasurementSystemEnum() == MeasurementSystem::US )
    {
        eUnit = MapUnit::Map100thInch;
        nDigits = 2;
    }
    Size aLogicPaperSize( LogicToLogic( i_rOrigSize, MapMode( MapUnit::Map100thMM ), MapMode( eUnit ) ) );
    OUString aNumText( rLocWrap.getNum( aLogicPaperSize.Width(), nDigits ) );
    aBuf.append( aNumText )
        .append( u' ' );
    aBuf.appendAscii( eUnit == MapUnit::MapMM ? "mm" : "in" );
    if( !i_rPaperName.isEmpty() )
    {
        aBuf.append( " (" );
        aBuf.append( i_rPaperName );
        aBuf.append( ')' );
    }
    maHorzDim->SetText( aBuf.makeStringAndClear() );

    aNumText = rLocWrap.getNum( aLogicPaperSize.Height(), nDigits );
    aBuf.append( aNumText )
        .append( u' ' );
    aBuf.appendAscii( eUnit == MapUnit::MapMM ? "mm" : "in" );
    maVertDim->SetText( aBuf.makeStringAndClear() );

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
    if(maPreviewSize.getWidth() < 0 || maPreviewSize.getHeight() < 0)
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

    SetMapMode(MapMode(MapUnit::MapPixel));
    pPrerenderVDev->SetMapMode(MapMode(MapUnit::MapPixel));

    maPreviewBitmap = pPrerenderVDev->GetBitmapEx(Point(0, 0), aVDevSize);

    pPrerenderVDev->SetDrawMode( nOldDrawMode );
}

PrintDialog::ShowNupOrderWindow::ShowNupOrderWindow( vcl::Window* i_pParent )
    : Window( i_pParent, WB_NOBORDER )
    , mnOrderMode( NupOrderType::LRTB )
    , mnRows( 1 )
    , mnColumns( 1 )
{
    SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetFieldColor() ) );
}

Size PrintDialog::ShowNupOrderWindow::GetOptimalSize() const
{
    return Size(70, 70);
}

void PrintDialog::ShowNupOrderWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& i_rRect)
{
    Window::Paint(rRenderContext, i_rRect);

    rRenderContext.SetMapMode(MapMode(MapUnit::MapPixel));
    rRenderContext.SetTextColor(rRenderContext.GetSettings().GetStyleSettings().GetFieldTextColor());

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
    if( maFirstPageSize.Width() == 0 && maFirstPageSize.Height() == 0)
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

PrintDialog::PrintDialog(vcl::Window* i_pWindow, const std::shared_ptr<PrinterController>& i_rController)
: ModalDialog(i_pWindow, "PrintDialog", "vcl/ui/printdialog.ui")
, maPController( i_rController )
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
    get(mpOKButton, "ok");
    get(mpCancelButton, "cancel");
    get(mpHelpButton, "help");
    get(mpMoreOptionsBtn, "moreoptionsbtn");
    get(mpTabCtrl, "tabcontrol");
    get(mpPageLayoutFrame, "layoutframe");
    get(mpForwardBtn, "forward");
    get(mpBackwardBtn, "backward");
    get(mpNumPagesText, "totalnumpages");
    get(mpPageEdit, "pageedit-nospin");
    get(mpPreviewWindow, "preview");
    get(mpPreviewBox, "previewbox");
    get(mpPrinters, "printersbox");
    get(mpSetupButton, "setup");
    get(mpStatusTxt, "status");
    get(mpCollateBox, "collate");
    get(mpCollateImage, "collateimage");
    get(mpPaperSidesBox, "sidesbox");
    get(mpReverseOrderBox, "reverseorder");
    get(mpCopyCountField, "copycount");
    get(mpNupOrderWin, "orderpreview");
    get(mpNupPagesBox, "pagespersheetbox");
    get(mpOrientationBox, "pageorientationbox");
    get(mpNupOrderTxt, "labelorder");
    get(mpPaperSizeBox, "papersizebox");
    get(mpNupOrderBox, "orderbox");
    get(mpPagesBtn, "pagespersheetbtn");
    get(mpBrochureBtn, "brochure");
    get(mpPagesBoxTitleTxt, "pagespersheettxt");
    get(mpNupNumPagesTxt, "pagestxt");
    get(mpNupColEdt, "pagecols");
    get(mpNupTimesTxt, "by");
    get(mpNupRowsEdt, "pagerows");
    get(mpPageMarginTxt1, "pagemargintxt1");
    get(mpPageMarginEdt, "pagemarginsb");
    get(mpPageMarginTxt2, "pagemargintxt2");
    get(mpSheetMarginTxt1, "sheetmargintxt1");
    get(mpSheetMarginEdt, "sheetmarginsb");
    get(mpSheetMarginTxt2, "sheetmargintxt2");
    get(mpBorderCB, "bordercb");

    // save printbutton text, gets exchanged occasionally with print to file
    maPrintText = mpOKButton->GetText();

    // setup preview controls
    mpForwardBtn->SetStyle( mpForwardBtn->GetStyle() | WB_BEVELBUTTON );
    mpBackwardBtn->SetStyle( mpBackwardBtn->GetStyle() | WB_BEVELBUTTON );

    maPageStr = mpNumPagesText->GetText();

    Printer::updatePrinters();

    mpPrinters->InsertEntry( maPrintToFileText );
    // fill printer listbox
    std::vector< OUString > rQueues( Printer::GetPrinterQueues() );
    std::sort( rQueues.begin(), rQueues.end(), lcl_ListBoxCompare );
    for( const auto& rQueue : rQueues )
    {
        mpPrinters->InsertEntry( rQueue );
    }
    // select current printer
    if( mpPrinters->GetEntryPos( maPController->getPrinter()->GetName() ) != LISTBOX_ENTRY_NOTFOUND )
    {
        mpPrinters->SelectEntry( maPController->getPrinter()->GetName() );
    }
    else
    {
        // fall back to last printer
        SettingsConfigItem* pItem = SettingsConfigItem::get();
        OUString aValue( pItem->getValue( "PrintDialog",
                                        "LastPrinter" ) );
        if( mpPrinters->GetEntryPos( aValue ) != LISTBOX_ENTRY_NOTFOUND )
        {
            mpPrinters->SelectEntry( aValue );
            maPController->setPrinter( VclPtrInstance<Printer>( aValue ) );
        }
        else
        {
            // fall back to default printer
            mpPrinters->SelectEntry( Printer::GetDefaultPrinterName() );
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
    mpCopyCountField->GrabFocus();
    mpCopyCountField->SetSelection( Selection(0, 0xFFFF) );

    // setup sizes for N-Up
    Size aNupSize( maPController->getPrinter()->PixelToLogic(
                         maPController->getPrinter()->GetPaperSizePixel(), MapMode( MapUnit::Map100thMM ) ) );
    if( maPController->getPrinter()->GetOrientation() == Orientation::Landscape )
    {
        maNupLandscapeSize = aNupSize;
        maNupPortraitSize = Size( aNupSize.Height(), aNupSize.Width() );
    }
    else
    {
        maNupPortraitSize = aNupSize;
        maNupLandscapeSize = Size( aNupSize.Height(), aNupSize.Width() );
    }

    initFromMultiPageSetup( maPController->getMultipage() );

    // setup optional UI options set by application
    setupOptionalUI();

    // hide layout frame if unwanted
    mpPageLayoutFrame->Show( mbShowLayoutFrame );

    // restore settings from last run
    readFromSettings();

    // setup click hdl
    mpOKButton->SetClickHdl(LINK(this, PrintDialog, ClickHdl));
    mpCancelButton->SetClickHdl(LINK(this, PrintDialog, ClickHdl));
    mpHelpButton->SetClickHdl(LINK(this, PrintDialog, ClickHdl));
    mpSetupButton->SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    mpMoreOptionsBtn->SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    mpBackwardBtn->SetClickHdl(LINK(this, PrintDialog, ClickHdl));
    mpForwardBtn->SetClickHdl(LINK(this, PrintDialog, ClickHdl));
    mpPreviewBox->SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    mpBorderCB->SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );

    // setup toggle hdl
    mpReverseOrderBox->SetToggleHdl( LINK( this, PrintDialog, ToggleHdl ) );
    mpCollateBox->SetToggleHdl( LINK( this, PrintDialog, ToggleHdl ) );
    mpPagesBtn->SetToggleHdl( LINK( this, PrintDialog, ToggleRadioHdl ) );

    // setup select hdl
    mpPrinters->SetSelectHdl( LINK( this, PrintDialog, SelectHdl ) );
    mpPaperSidesBox->SetSelectHdl( LINK( this, PrintDialog, SelectHdl ) );
    mpNupPagesBox->SetSelectHdl( LINK( this, PrintDialog, SelectHdl ) );
    mpOrientationBox->SetSelectHdl( LINK( this, PrintDialog, SelectHdl ) );
    mpNupOrderBox->SetSelectHdl( LINK( this, PrintDialog, SelectHdl ) );
    mpPaperSizeBox->SetSelectHdl( LINK( this, PrintDialog, SelectHdl ) );

    // setup modify hdl
    mpPageEdit->SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    mpCopyCountField->SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    mpNupColEdt->SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    mpNupRowsEdt->SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    mpPageMarginEdt->SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    mpSheetMarginEdt->SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );

    updateNupFromPages();
}


PrintDialog::~PrintDialog()
{
    disposeOnce();
}

void PrintDialog::dispose()
{
    mpCustomOptionsUIBuilder.reset();
    mpTabCtrl.clear();
    mpPageLayoutFrame.clear();
    mpPreviewWindow.clear();
    mpPageEdit.clear();
    mpNumPagesText.clear();
    mpBackwardBtn.clear();
    mpForwardBtn.clear();
    mpPreviewBox.clear();
    mpOKButton.clear();
    mpCancelButton.clear();
    mpHelpButton.clear();
    mpMoreOptionsBtn.clear();
    maPController.reset();
    maControlToPropertyMap.clear();
    maControlToNumValMap.clear();
    mpPrinters.clear();
    mpStatusTxt.clear();
    mpSetupButton.clear();
    mpCopyCountField.clear();
    mpCollateBox.clear();
    mpCollateImage.clear();
    mpPaperSidesBox.clear();
    mpReverseOrderBox.clear();
    mpPagesBtn.clear();
    mpBrochureBtn.clear();
    mpPagesBoxTitleTxt.clear();
    mpNupPagesBox.clear();
    mpNupNumPagesTxt.clear();
    mpNupColEdt.clear();
    mpNupTimesTxt.clear();
    mpNupRowsEdt.clear();
    mpPageMarginTxt1.clear();
    mpPageMarginEdt.clear();
    mpPageMarginTxt2.clear();
    mpSheetMarginTxt1.clear();
    mpSheetMarginEdt.clear();
    mpSheetMarginTxt2.clear();
    mpPaperSizeBox.clear();
    mpOrientationBox.clear();
    mpNupOrderBox.clear();
    mpNupOrderWin.clear();
    mpNupOrderTxt.clear();
    mpBorderCB.clear();
    mxMoreOptionsDlg.reset();
    ModalDialog::dispose();
}

void PrintDialog::setupPaperSidesBox()
{
    DuplexMode eDuplex = maPController->getPrinter()->GetDuplexMode();

    if ( eDuplex == DuplexMode::Unknown || isPrintToFile() )
    {
        mpPaperSidesBox->SelectEntryPos( 0 );
        mpPaperSidesBox->Enable( false );
    }
    else
    {
        mpPaperSidesBox->SelectEntryPos( static_cast<sal_Int32>(eDuplex) - 1 );
        mpPaperSidesBox->Enable( true );
    }
}

void PrintDialog::storeToSettings()
{
    SettingsConfigItem* pItem = SettingsConfigItem::get();

    pItem->setValue( "PrintDialog",
                     "LastPrinter",
                      isPrintToFile() ? Printer::GetDefaultPrinterName()
                                      : mpPrinters->GetSelectedEntry() );

    pItem->setValue( "PrintDialog",
                     "LastPage",
                     mpTabCtrl->GetPageText( mpTabCtrl->GetCurPageId() ) );

    pItem->setValue( "PrintDialog",
                     "WindowState",
                     OStringToOUString( GetWindowState(), RTL_TEXTENCODING_UTF8 ) );

    pItem->setValue( "PrintDialog",
                     "CopyCount",
                     mpCopyCountField->GetText() );

    pItem->setValue( "PrintDialog",
                     "Collate",
                     mpCollateBox->IsChecked() ? OUString("true") :
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
    OUString aValue;

    // read last selected tab page; if it exists, activate it
    aValue = pItem->getValue( "PrintDialog",
                              "LastPage" );
    sal_uInt16 nCount = mpTabCtrl->GetPageCount();
    for( sal_uInt16 i = 0; i < nCount; i++ )
    {
        sal_uInt16 nPageId = mpTabCtrl->GetPageId( i );

        if( aValue == mpTabCtrl->GetPageText( nPageId ) )
        {
            mpTabCtrl->SelectTabPage( nPageId );
            break;
        }
    }

    // persistent window state
    aValue = pItem->getValue( "PrintDialog",
                              "WindowState" );
    if( !aValue.isEmpty() )
        SetWindowState( OUStringToOString( aValue, RTL_TEXTENCODING_UTF8 ) );

    // collate
    aValue = pItem->getValue( "PrintDialog",
                              "CollateBox" );
    if( aValue.equalsIgnoreAsciiCase("alwaysoff") )
    {
        mbCollateAlwaysOff = true;
        mpCollateBox->Check( false );
        mpCollateBox->Enable( false );
    }
    else
    {
        mbCollateAlwaysOff = false;
        aValue = pItem->getValue( "PrintDialog",
                                  "Collate" );
        mpCollateBox->Check( aValue.equalsIgnoreAsciiCase("true") );
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
        mpPreviewBox->Check( false );
    else
        mpPreviewBox->Check( true );

}

void PrintDialog::setPaperSizes()
{
    mpPaperSizeBox->Clear();

    VclPtr<Printer> aPrt( maPController->getPrinter() );
    mePaper = aPrt->GetPaper();

    if ( isPrintToFile() )
    {
        mpPaperSizeBox->Enable( false );
    }
    else
    {
        for (int nPaper = 0; nPaper < aPrt->GetPaperInfoCount(); nPaper++)
        {
            PaperInfo aInfo = aPrt->GetPaperInfo( nPaper );
            aInfo.doSloppyFit();
            Paper ePaper = aInfo.getPaper();

            const LocaleDataWrapper& rLocWrap( GetSettings().GetLocaleDataWrapper() );
            MapUnit eUnit = MapUnit::MapMM;
            int nDigits = 0;
            if( rLocWrap.getMeasurementSystemEnum() == MeasurementSystem::US )
            {
                eUnit = MapUnit::Map100thInch;
                nDigits = 2;
            }
            Size aSize = aPrt->GetPaperSize( nPaper );
            Size aLogicPaperSize( LogicToLogic( aSize, MapMode( MapUnit::Map100thMM ), MapMode( eUnit ) ) );

            OUString aWidth( rLocWrap.getNum( aLogicPaperSize.Width(), nDigits ) );
            OUString aHeight( rLocWrap.getNum( aLogicPaperSize.Height(), nDigits ) );
            OUString aUnit = eUnit == MapUnit::MapMM ? OUString("mm") : OUString("in");
            OUString aPaperName = Printer::GetPaperName( ePaper ) + " " + aWidth + aUnit + " x " + aHeight + aUnit;

            mpPaperSizeBox->InsertEntry( aPaperName );

            if ( ePaper == mePaper )
                mpPaperSizeBox->SelectEntryPos( nPaper );
        }

        mpPaperSizeBox->Enable( true );
    }
}

void PrintDialog::updatePrinterText()
{
    const OUString aDefPrt( Printer::GetDefaultPrinterName() );
    const QueueInfo* pInfo = Printer::GetQueueInfo( mpPrinters->GetSelectedEntry(), true );
    if( pInfo )
    {
        // FIXME: status text
        OUString aStatus;
        if( aDefPrt == pInfo->GetPrinterName() )
            aStatus = maDefPrtText;
        mpStatusTxt->SetText( aStatus );
    }
    else
    {
        mpStatusTxt->SetText( OUString() );
    }
}

void PrintDialog::setPreviewText()
{
    OUString aNewText( maPageStr.replaceFirst( "%n", OUString::number( mnCachedPages ) ) );
    mpNumPagesText->SetText( aNewText );
}

void PrintDialog::preparePreview( bool i_bMayUseCache )
{
    VclPtr<Printer> aPrt( maPController->getPrinter() );
    Size aCurPageSize = aPrt->PixelToLogic( aPrt->GetPaperSizePixel(), MapMode( MapUnit::Map100thMM ) );
    GDIMetaFile aMtf;

    // page range may have changed depending on options
    sal_Int32 nPages = maPController->getFilteredPageCount();
    mnCachedPages = nPages;

    mpPageEdit->SetMin( 1 );
    mpPageEdit->SetMax( nPages );

    setPreviewText();

    if ( !hasPreview() )
    {
        mpPreviewWindow->setPreview( aMtf, aCurPageSize,
                            Printer::GetPaperName( mePaper ),
                            maNoPreviewStr,
                            aPrt->GetDPIX(), aPrt->GetDPIY(),
                            aPrt->GetPrinterOptions().IsConvertToGreyscales()
                            );

        mpForwardBtn->Enable( false );
        mpBackwardBtn->Enable( false );
        mpPageEdit->Enable( false );

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

    mpPreviewWindow->setPreview( aMtf, aCurPageSize,
                                Printer::GetPaperName( mePaper ),
                                nPages > 0 ? OUString() : maNoPageStr,
                                aPrt->GetDPIX(), aPrt->GetDPIY(),
                                aPrt->GetPrinterOptions().IsConvertToGreyscales()
                               );

    mpForwardBtn->Enable( mnCurPage < nPages-1 );
    mpBackwardBtn->Enable( mnCurPage != 0 );
    mpPageEdit->Enable( nPages > 1 );
}

void PrintDialog::updateOrientationBox( const bool bAutomatic )
{
    if ( !bAutomatic )
    {
        Orientation eOrientation = maPController->getPrinter()->GetOrientation();
        mpOrientationBox->SelectEntryPos( static_cast<sal_Int32>(eOrientation) + 1 );
    }
    else if ( hasOrientationChanged() )
    {
        mpOrientationBox->SelectEntryPos( ORIENTATION_AUTOMATIC );
    }
}

bool PrintDialog::hasOrientationChanged() const
{
    const int nOrientation = mpOrientationBox->GetSelectedEntryPos();
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

    if( mpCopyCountField->GetValue() > 1 )
        mpCollateBox->Enable( !mbCollateAlwaysOff );
    else
        mpCollateBox->Enable( false );

    Image aImg(StockImage::Yes, mpCollateBox->IsChecked() ? OUString(SV_PRINT_COLLATE_BMP) : OUString(SV_PRINT_NOCOLLATE_BMP));

    Size aImgSize( aImg.GetSizePixel() );

    // adjust size of image
    mpCollateImage->SetSizePixel( aImgSize );
    mpCollateImage->SetImage( aImg );

    // enable setup button only for printers that can be setup
    bool bHaveSetup = maPController->getPrinter()->HasSupport( PrinterSupport::SetupDialog );
    mpSetupButton->Enable(bHaveSetup);
}

void PrintDialog::checkOptionalControlDependencies()
{
    for( const auto& rEntry : maControlToPropertyMap )
    {
        bool bShouldbeEnabled = maPController->isUIOptionEnabled( rEntry.second );

        if( bShouldbeEnabled && dynamic_cast<RadioButton*>(rEntry.first.get()) )
        {
            auto r_it = maControlToNumValMap.find( rEntry.first );
            if( r_it != maControlToNumValMap.end() )
            {
                bShouldbeEnabled = maPController->isUIChoiceEnabled( rEntry.second, r_it->second );
            }
        }

        bool bIsEnabled = rEntry.first->IsEnabled();
        // Enable does not do a change check first, so can be less cheap than expected
        if( bShouldbeEnabled != bIsEnabled )
            rEntry.first->Enable( bShouldbeEnabled );
    }
}

void PrintDialog::initFromMultiPageSetup( const vcl::PrinterController::MultiPageSetup& i_rMPS )
{
    mpNupOrderWin->Show();
    mpPagesBtn->Check();
    mpBrochureBtn->Show( false );

    // setup field units for metric fields
    const LocaleDataWrapper& rLocWrap( mpPageMarginEdt->GetLocaleDataWrapper() );
    FieldUnit eUnit = FieldUnit::MM;
    sal_uInt16 nDigits = 0;
    if( rLocWrap.getMeasurementSystemEnum() == MeasurementSystem::US )
    {
        eUnit = FieldUnit::INCH;
        nDigits = 2;
    }
    // set units
    mpPageMarginEdt->SetUnit( eUnit );
    mpSheetMarginEdt->SetUnit( eUnit );

    // set precision
    mpPageMarginEdt->SetDecimalDigits( nDigits );
    mpSheetMarginEdt->SetDecimalDigits( nDigits );

    mpSheetMarginEdt->SetValue( mpSheetMarginEdt->Normalize( i_rMPS.nLeftMargin ), FieldUnit::MM_100TH );
    mpPageMarginEdt->SetValue( mpPageMarginEdt->Normalize( i_rMPS.nHorizontalSpacing ), FieldUnit::MM_100TH );
    mpBorderCB->Check( i_rMPS.bDrawBorder );
    mpNupRowsEdt->SetValue( i_rMPS.nRows );
    mpNupColEdt->SetValue( i_rMPS.nColumns );
    mpNupOrderBox->SelectEntryPos( static_cast<sal_Int32>(i_rMPS.nOrder) );
    if( i_rMPS.nRows != 1 || i_rMPS.nColumns != 1 )
    {
        mpNupPagesBox->SelectEntryPos( mpNupPagesBox->GetEntryCount()-1 );
        showAdvancedControls( true );
        mpNupOrderWin->setValues( i_rMPS.nOrder, i_rMPS.nColumns, i_rMPS.nRows );
    }
}

void PrintDialog::updateNup( bool i_bMayUseCache )
{
    int nRows         = int(mpNupRowsEdt->GetValue());
    int nCols         = int(mpNupColEdt->GetValue());
    long nPageMargin  = mpPageMarginEdt->Denormalize(mpPageMarginEdt->GetValue( FieldUnit::MM_100TH ));
    long nSheetMargin = mpSheetMarginEdt->Denormalize(mpSheetMarginEdt->GetValue( FieldUnit::MM_100TH ));

    PrinterController::MultiPageSetup aMPS;
    aMPS.nRows         = nRows;
    aMPS.nColumns      = nCols;
    aMPS.nLeftMargin   =
    aMPS.nTopMargin    =
    aMPS.nRightMargin  =
    aMPS.nBottomMargin = nSheetMargin;

    aMPS.nHorizontalSpacing =
    aMPS.nVerticalSpacing   = nPageMargin;

    aMPS.bDrawBorder        = mpBorderCB->IsChecked();

    aMPS.nOrder = static_cast<NupOrderType>(mpNupOrderBox->GetSelectedEntryPos());

    int nOrientationMode = mpOrientationBox->GetSelectedEntryPos();
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

    mpNupOrderWin->setValues( aMPS.nOrder, nCols, nRows );

    preparePreview( i_bMayUseCache );
}

void PrintDialog::updateNupFromPages( bool i_bMayUseCache )
{
    sal_IntPtr nPages = sal_IntPtr(mpNupPagesBox->GetSelectedEntryData());
    int nRows   = int(mpNupRowsEdt->GetValue());
    int nCols   = int(mpNupColEdt->GetValue());
    long nPageMargin  = mpPageMarginEdt->Denormalize(mpPageMarginEdt->GetValue( FieldUnit::MM_100TH ));
    long nSheetMargin = mpSheetMarginEdt->Denormalize(mpSheetMarginEdt->GetValue( FieldUnit::MM_100TH ));
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

        mpSheetMarginEdt->SetMax(
                  mpSheetMarginEdt->Normalize(
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

        mpPageMarginEdt->SetMax(
                 mpSheetMarginEdt->Normalize(
                           std::min(nHorzMax, nVertMax ) ), FieldUnit::MM_100TH );
    }

    mpNupRowsEdt->SetValue( nRows );
    mpNupColEdt->SetValue( nCols );
    mpPageMarginEdt->SetValue( mpPageMarginEdt->Normalize( nPageMargin ), FieldUnit::MM_100TH );
    mpSheetMarginEdt->SetValue( mpSheetMarginEdt->Normalize( nSheetMargin ), FieldUnit::MM_100TH );

    showAdvancedControls( bCustom );
    updateNup( i_bMayUseCache );
}

void PrintDialog::enableNupControls( bool bEnable )
{
    mpNupPagesBox->Enable( bEnable );
    mpNupNumPagesTxt->Enable( bEnable );
    mpNupColEdt->Enable( bEnable );
    mpNupTimesTxt->Enable( bEnable );
    mpNupRowsEdt->Enable( bEnable );
    mpPageMarginTxt1->Enable( bEnable );
    mpPageMarginEdt->Enable( bEnable );
    mpPageMarginTxt2->Enable( bEnable );
    mpSheetMarginTxt1->Enable( bEnable );
    mpSheetMarginEdt->Enable( bEnable );
    mpSheetMarginTxt2->Enable( bEnable );
    mpNupOrderTxt->Enable( bEnable );
    mpNupOrderBox->Enable( bEnable );
    mpNupOrderWin->Enable( bEnable );
    mpBorderCB->Enable( bEnable );
}

void PrintDialog::showAdvancedControls( bool i_bShow )
{
    mpNupNumPagesTxt->Show( i_bShow );
    mpNupColEdt->Show( i_bShow );
    mpNupTimesTxt->Show( i_bShow );
    mpNupRowsEdt->Show( i_bShow );
    mpPageMarginTxt1->Show( i_bShow );
    mpPageMarginEdt->Show( i_bShow );
    mpPageMarginTxt2->Show( i_bShow );
    mpSheetMarginTxt1->Show( i_bShow );
    mpSheetMarginEdt->Show( i_bShow );
    mpSheetMarginTxt2->Show( i_bShow );
}

namespace
{
    void setHelpId( vcl::Window* i_pWindow, const Sequence< OUString >& i_rHelpIds, sal_Int32 i_nIndex )
    {
        if( i_nIndex >= 0 && i_nIndex < i_rHelpIds.getLength() )
            i_pWindow->SetHelpId( OUStringToOString( i_rHelpIds.getConstArray()[i_nIndex], RTL_TEXTENCODING_UTF8 ) );
    }

    void setHelpText( vcl::Window* i_pWindow, const Sequence< OUString >& i_rHelpTexts, sal_Int32 i_nIndex )
    {
        // without a help text set and the correct smartID,
        // help texts will be retrieved from the online help system
        if( i_nIndex >= 0 && i_nIndex < i_rHelpTexts.getLength() )
            i_pWindow->SetHelpText( i_rHelpTexts.getConstArray()[i_nIndex] );
    }
}

void PrintDialog::setupOptionalUI()
{
    const Sequence< PropertyValue >& rOptions( maPController->getUIOptions() );
    for( int i = 0; i < rOptions.getLength(); i++ )
    {
        if (rOptions[i].Name == "OptionsUIFile")
        {
            OUString sOptionsUIFile;
            rOptions[i].Value >>= sOptionsUIFile;

            vcl::Window *pCustom = get<vcl::Window>("customcontents");

            mpCustomOptionsUIBuilder.reset(new VclBuilder(pCustom, getUIRootDir(), sOptionsUIFile));
            vcl::Window *pWindow = mpCustomOptionsUIBuilder->get_widget_root();
            pWindow->Show();
            continue;
        }

        Sequence< beans::PropertyValue > aOptProp;
        rOptions[i].Value >>= aOptProp;

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

        for( int n = 0; n < aOptProp.getLength(); n++ )
        {
            const beans::PropertyValue& rEntry( aOptProp[ n ] );
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

        if (aCtrlType == "Group" && !aID.isEmpty())
        {
            TabPage *pPage = get<TabPage>(aID);
            if (!pPage && mpCustomOptionsUIBuilder)
                pPage = mpCustomOptionsUIBuilder->get<TabPage>(aID);

            if (!pPage)
                continue;

            sal_uInt16 nPageId = mpTabCtrl->GetPageId(*pPage);

            mpTabCtrl->SetPageText(nPageId, aText);

            // set help id
            if (aHelpIds.getLength() > 0)
                mpTabCtrl->SetHelpId(nPageId, OUStringToOString(aHelpIds.getConstArray()[0], RTL_TEXTENCODING_UTF8));

            // set help text
            if (aHelpTexts.getLength() > 0)
                mpTabCtrl->SetHelpText(nPageId, aHelpTexts.getConstArray()[0]);

            pPage->Show();
        }
        else if (aCtrlType == "Subgroup" && !aID.isEmpty())
        {
            vcl::Window *pFrame = get<vcl::Window>(aID);
            if (!pFrame && mpCustomOptionsUIBuilder)
                pFrame = mpCustomOptionsUIBuilder->get<vcl::Window>(aID);

            if (!pFrame)
                continue;

            pFrame->SetText(aText);

            // set help id
            setHelpId(pFrame, aHelpIds, 0);
            // set help text
            setHelpText(pFrame, aHelpTexts, 0);

            pFrame->Show();
        }
        // EVIL
        else if( aCtrlType == "Bool" && aGroupingHint == "LayoutPage" && aPropertyName == "PrintProspect" )
        {
            mpBrochureBtn->SetText( aText );
            mpBrochureBtn->Show();

            bool bVal = false;
            PropertyValue* pVal = maPController->getValue( aPropertyName );
            if( pVal )
                pVal->Value >>= bVal;
            mpBrochureBtn->Check( bVal );
            mpBrochureBtn->Enable( maPController->isUIOptionEnabled( aPropertyName ) && pVal != nullptr );
            mpBrochureBtn->SetToggleHdl( LINK( this, PrintDialog, ToggleRadioHdl ) );

            maPropertyToWindowMap[ aPropertyName ].emplace_back(mpBrochureBtn );
            maControlToPropertyMap[mpBrochureBtn] = aPropertyName;

            // set help id
            setHelpId( mpBrochureBtn, aHelpIds, 0 );
            // set help text
            setHelpText( mpBrochureBtn, aHelpTexts, 0 );
        }
        else if (aCtrlType == "Bool")
        {
            // add a check box
            CheckBox* pNewBox = get<CheckBox>(aID);
            if (!pNewBox && mpCustomOptionsUIBuilder)
                pNewBox = mpCustomOptionsUIBuilder->get<CheckBox>(aID);

            if (!pNewBox)
                continue;

            pNewBox->SetText( aText );
            pNewBox->Show();

            bool bVal = false;
            PropertyValue* pVal = maPController->getValue( aPropertyName );
            if( pVal )
                pVal->Value >>= bVal;
            pNewBox->Check( bVal );
            pNewBox->SetToggleHdl( LINK( this, PrintDialog, UIOption_CheckHdl ) );

            maPropertyToWindowMap[ aPropertyName ].emplace_back(pNewBox );
            maControlToPropertyMap[pNewBox] = aPropertyName;

            // set help id
            setHelpId( pNewBox, aHelpIds, 0 );
            // set help text
            setHelpText( pNewBox, aHelpTexts, 0 );
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
                RadioButton* pBtn = get<RadioButton>(aID);
                if (!pBtn && mpCustomOptionsUIBuilder)
                    pBtn = mpCustomOptionsUIBuilder->get<RadioButton>(aID);

                if (!pBtn)
                    continue;

                pBtn->SetText( aChoices[m] );
                pBtn->Check( m == nSelectVal );
                pBtn->SetToggleHdl( LINK( this, PrintDialog, UIOption_RadioHdl ) );
                if( aChoicesDisabled.getLength() > m && aChoicesDisabled[m] )
                    pBtn->Enable( false );
                pBtn->Show();
                maPropertyToWindowMap[ aPropertyName ].emplace_back(pBtn );
                maControlToPropertyMap[pBtn] = aPropertyName;
                maControlToNumValMap[pBtn] = m;

                // set help id
                setHelpId( pBtn, aHelpIds, nCurHelpText );
                // set help text
                setHelpText( pBtn, aHelpTexts, nCurHelpText );
                nCurHelpText++;
            }
        }
        else if ( aCtrlType == "List" )
        {
            ListBox* pList = get<ListBox>(aID);
            if (!pList && mpCustomOptionsUIBuilder)
                pList = mpCustomOptionsUIBuilder->get<ListBox>(aID);

            if (!pList)
                continue;

            // iterate options
            for( sal_Int32 m = 0; m < aChoices.getLength(); m++ )
            {
                pList->InsertEntry( aChoices[m] );
            }
            sal_Int32 nSelectVal = 0;
            PropertyValue* pVal = maPController->getValue( aPropertyName );
            if( pVal && pVal->Value.hasValue() )
                pVal->Value >>= nSelectVal;
            pList->SelectEntryPos( static_cast<sal_uInt16>(nSelectVal) );
            pList->SetSelectHdl( LINK( this, PrintDialog, UIOption_SelectHdl ) );
            pList->SetDropDownLineCount( static_cast<sal_uInt16>(aChoices.getLength()) );
            pList->Show();

            // set help id
            setHelpId( pList, aHelpIds, 0 );
            // set help text
            setHelpText( pList, aHelpTexts, 0 );

            maPropertyToWindowMap[ aPropertyName ].emplace_back(pList );
            maControlToPropertyMap[pList] = aPropertyName;
        }
        else if ( aCtrlType == "Range" )
        {
            NumericField* pField = get<NumericField>(aID);
            if (!pField && mpCustomOptionsUIBuilder)
                pField = mpCustomOptionsUIBuilder->get<NumericField>(aID);

            if (!pField)
                continue;

            // set min/max and current value
            if( nMinValue != nMaxValue )
            {
                pField->SetMin( nMinValue );
                pField->SetMax( nMaxValue );
            }
            sal_Int64 nCurVal = 0;
            PropertyValue* pVal = maPController->getValue( aPropertyName );
            if( pVal && pVal->Value.hasValue() )
                pVal->Value >>= nCurVal;
            pField->SetValue( nCurVal );
            pField->SetModifyHdl( LINK( this, PrintDialog, UIOption_ModifyHdl ) );
            pField->Show();

            // set help id
            setHelpId( pField, aHelpIds, 0 );
            // set help text
            setHelpText( pField, aHelpTexts, 0 );

            maPropertyToWindowMap[ aPropertyName ].emplace_back(pField );
            maControlToPropertyMap[pField] = aPropertyName;
        }
        else if (aCtrlType == "Edit")
        {
            Edit *pField = get<Edit>(aID);
            if (!pField && mpCustomOptionsUIBuilder)
                pField = mpCustomOptionsUIBuilder->get<Edit>(aID);

            if (!pField)
                continue;

            OUString aCurVal;
            PropertyValue* pVal = maPController->getValue( aPropertyName );
            if( pVal && pVal->Value.hasValue() )
                pVal->Value >>= aCurVal;
            pField->SetText( aCurVal );
            pField->SetModifyHdl( LINK( this, PrintDialog, UIOption_ModifyHdl ) );
            pField->Show();

            // set help id
            setHelpId( pField, aHelpIds, 0 );
            // set help text
            setHelpText( pField, aHelpTexts, 0 );

            maPropertyToWindowMap[ aPropertyName ].emplace_back(pField );
            maControlToPropertyMap[pField] = aPropertyName;
        }
        else
        {
            SAL_WARN( "vcl", "Unsupported UI option: \"" << aCtrlType << '"');
        }
    }

    // #i106506# if no brochure button, then the singular Pages radio button
    // makes no sense, so replace it by a FixedText label
    if (!mpBrochureBtn->IsVisible() && mpPagesBtn->IsVisible())
    {
        mpPagesBoxTitleTxt->SetText( mpPagesBtn->GetText() );
        mpPagesBoxTitleTxt->Show();
        mpPagesBtn->Show( false );

        mpPagesBoxTitleTxt->SetAccessibleRelationLabelFor(mpNupPagesBox);
        mpNupPagesBox->SetAccessibleRelationLabeledBy(mpPagesBoxTitleTxt);
        mpPagesBtn->SetAccessibleRelationLabelFor(nullptr);
    }

    // update enable states
    checkOptionalControlDependencies();

    vcl::Window *pPageRange = get<vcl::Window>("pagerange");

    // print range not shown (currently math only) -> hide spacer line and reverse order
    if (!pPageRange || !pPageRange->IsVisible())
    {
        mpReverseOrderBox->Show( false );
    }

    if (!mpCustomOptionsUIBuilder)
        mpTabCtrl->RemovePage(mpTabCtrl->GetPageId(1));
}

void PrintDialog::makeEnabled( vcl::Window* i_pWindow )
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
        const std::vector< VclPtr<vcl::Window> >& rWindows( it->second );
        if( ! rWindows.empty() )
        {
            bool bVal = false;
            sal_Int32 nVal = -1;
            if( pValue->Value >>= bVal )
            {
                // we should have a CheckBox for this one
                CheckBox* pBox = dynamic_cast< CheckBox* >( rWindows.front().get() );
                if( pBox )
                {
                    pBox->Check( bVal );
                }
                else if ( i_rProperty == "PrintProspect" )
                {
                    // EVIL special case
                    if( bVal )
                        mpBrochureBtn->Check();
                    else
                        mpPagesBtn->Check();
                }
                else
                {
                    SAL_WARN( "vcl", "missing a checkbox" );
                }
            }
            else if( pValue->Value >>= nVal )
            {
                // this could be a ListBox or a RadioButtonGroup
                ListBox* pList = dynamic_cast< ListBox* >( rWindows.front().get() );
                if( pList )
                {
                    pList->SelectEntryPos( static_cast< sal_uInt16 >(nVal) );
                }
                else if( nVal >= 0 && nVal < sal_Int32(rWindows.size() ) )
                {
                    RadioButton* pBtn = dynamic_cast< RadioButton* >( rWindows[nVal].get() );
                    SAL_WARN_IF( !pBtn, "vcl", "unexpected control for property" );
                    if( pBtn )
                        pBtn->Check();
                }
            }
        }
    }
}

bool PrintDialog::isPrintToFile()
{
    return ( mpPrinters->GetSelectedEntryPos() == 0 );
}

bool PrintDialog::isCollate()
{
    return mpCopyCountField->GetValue() > 1 && mpCollateBox->IsChecked();
}

bool PrintDialog::hasPreview()
{
    return mpPreviewBox->IsChecked();
}

PropertyValue* PrintDialog::getValueForWindow( vcl::Window* i_pWindow ) const
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

IMPL_LINK( PrintDialog, ToggleHdl, CheckBox&, rButton, void )
{
    ClickHdl(&rButton);
}

IMPL_LINK( PrintDialog, ToggleRadioHdl, RadioButton&, rButton, void )
{
    ClickHdl(static_cast<Button*>(&rButton));
}

IMPL_LINK ( PrintDialog, ClickHdl, Button*, pButton, void )
{
    if( pButton == mpOKButton || pButton == mpCancelButton )
    {
        storeToSettings();
        EndDialog( pButton == mpOKButton ? RET_OK : RET_CANCEL );
    }
    else if( pButton == mpHelpButton )
    {
        // start help system
        Help* pHelp = Application::GetHelp();
        if( pHelp )
        {
            pHelp->Start( "vcl/ui/printdialog", mpOKButton );
        }
    }
    else if ( pButton == mpPreviewBox )
    {
        preparePreview( true );
    }
    else if( pButton == mpForwardBtn )
    {
        previewForward();
    }
    else if( pButton == mpBackwardBtn )
    {
        previewBackward();
    }
    else if( pButton == mpBrochureBtn )
    {
        PropertyValue* pVal = getValueForWindow( pButton );
        if( pVal )
        {
            bool bVal = mpBrochureBtn->IsChecked();
            pVal->Value <<= bVal;

            checkOptionalControlDependencies();

            // update preview and page settings
            preparePreview(false);
        }
        if( mpBrochureBtn->IsChecked() )
        {
            mpOrientationBox->Enable( false );
            mpOrientationBox->SelectEntryPos( ORIENTATION_LANDSCAPE );
            mpNupPagesBox->SelectEntryPos( 0 );
            updateNupFromPages();
            showAdvancedControls( false );
            enableNupControls( false );
        }
    }
    else if( pButton == mpPagesBtn )
    {
        mpOrientationBox->Enable( true );
        mpOrientationBox->SelectEntryPos( ORIENTATION_AUTOMATIC );
        enableNupControls( true );
        updateNupFromPages();
    }
    else if( pButton == mpCollateBox )
    {
        maPController->setValue( "Collate",
                                 makeAny( isCollate() ) );
        checkControlDependencies();
    }
    else if( pButton == mpReverseOrderBox )
    {
        bool bChecked = mpReverseOrderBox->IsChecked();
        maPController->setReversePrint( bChecked );
        maPController->setValue( "PrintReverse",
                                 makeAny( bChecked ) );
        preparePreview( true );
    }
    else if( pButton == mpBorderCB )
    {
        updateNup();
    }
    else if ( pButton == mpMoreOptionsBtn )
    {
        mxMoreOptionsDlg.reset(new MoreOptionsDialog(this));
        mxMoreOptionsDlg->run();
    }
    else
    {
        if( pButton == mpSetupButton )
        {
            maPController->setupPrinter(GetFrameWeld());

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
                        mpPaperSizeBox->SelectEntryPos( nPaper );
                        break;
                    }
                }
            }

            updateOrientationBox( false );

            // tdf#63905 don't use cache: page size may change
            preparePreview(false);
        }
        checkControlDependencies();
    }

}

IMPL_LINK( PrintDialog, SelectHdl, ListBox&, rBox, void )
{
    if(  &rBox == mpPrinters )
    {
        if ( !isPrintToFile() )
        {
            OUString aNewPrinter( rBox.GetSelectedEntry() );
            // set new printer
            maPController->setPrinter( VclPtrInstance<Printer>( aNewPrinter ) );
            maPController->resetPrinterOptions( false  );

            updateOrientationBox();

            // update text fields
            mpOKButton->SetText( maPrintText );
            updatePrinterText();
            setPaperSizes();
            preparePreview(false);
        }
        else // print to file
        {
            // use the default printer or FIXME: the last used one?
            maPController->setPrinter( VclPtrInstance<Printer>( Printer::GetDefaultPrinterName() ) );
            mpOKButton->SetText( maPrintToFileText );
            maPController->resetPrinterOptions( true );

            setPaperSizes();
            updateOrientationBox();
            preparePreview( true );
        }

        setupPaperSidesBox();
    }
    else if ( &rBox == mpPaperSidesBox )
    {
        DuplexMode eDuplex = static_cast<DuplexMode>(mpPaperSidesBox->GetSelectedEntryPos() + 1);
        maPController->getPrinter()->SetDuplexMode( eDuplex );
    }
    else if( &rBox == mpOrientationBox )
    {
        int nOrientation = mpOrientationBox->GetSelectedEntryPos();
        if ( nOrientation != ORIENTATION_AUTOMATIC )
            setPaperOrientation( static_cast<Orientation>( nOrientation - 1 ) );

        updateNup( false );
    }
    else if ( &rBox == mpNupOrderBox )
    {
        updateNup();
    }
    else if( &rBox == mpNupPagesBox )
    {
        if( !mpPagesBtn->IsChecked() )
            mpPagesBtn->Check();
        updateNupFromPages( false );
    }
    else if ( &rBox == mpPaperSizeBox )
    {
        VclPtr<Printer> aPrt( maPController->getPrinter() );
        PaperInfo aInfo = aPrt->GetPaperInfo( rBox.GetSelectedEntryPos() );
        aInfo.doSloppyFit();
        mePaper = aInfo.getPaper();

        if ( mePaper == PAPER_USER )
            aPrt->SetPaperSizeUser( Size( aInfo.getWidth(), aInfo.getHeight() ) );
        else
            aPrt->SetPaper( mePaper );

        Size aPaperSize = Size( aInfo.getWidth(), aInfo.getHeight() );
        checkPaperSize( aPaperSize );
        maPController->setPaperSizeFromUser( aPaperSize );

        preparePreview(false);
    }
}

IMPL_LINK( PrintDialog, ModifyHdl, Edit&, rEdit, void )
{
    checkControlDependencies();
    if( &rEdit == mpNupRowsEdt || &rEdit == mpNupColEdt ||
       &rEdit == mpSheetMarginEdt || &rEdit == mpPageMarginEdt
      )
    {
       updateNupFromPages();
    }
    else if( &rEdit == mpPageEdit )
    {
        mnCurPage = sal_Int32( mpPageEdit->GetValue() - 1 );
        preparePreview( true );
    }
    else if( &rEdit == mpCopyCountField )
    {
        maPController->setValue( "CopyCount",
                               makeAny( sal_Int32(mpCopyCountField->GetValue()) ) );
        maPController->setValue( "Collate",
                               makeAny( isCollate() ) );
    }
}

IMPL_LINK( PrintDialog, UIOption_CheckHdl, CheckBox&, i_rBox, void )
{
    PropertyValue* pVal = getValueForWindow( &i_rBox );
    if( pVal )
    {
        makeEnabled( &i_rBox );

        bool bVal = i_rBox.IsChecked();
        pVal->Value <<= bVal;

        checkOptionalControlDependencies();

        // update preview and page settings
        preparePreview(false);
    }
}

IMPL_LINK( PrintDialog, UIOption_RadioHdl, RadioButton&, i_rBtn, void )
{
    // this handler gets called for all radiobuttons that get unchecked, too
    // however we only want one notification for the new value (that is for
    // the button that gets checked)
    if( i_rBtn.IsChecked() )
    {
        PropertyValue* pVal = getValueForWindow( &i_rBtn );
        auto it = maControlToNumValMap.find( &i_rBtn );
        if( pVal && it != maControlToNumValMap.end() )
        {
            makeEnabled( &i_rBtn );

            sal_Int32 nVal = it->second;
            pVal->Value <<= nVal;

            // tdf#63905 use paper size set in printer properties
            if (pVal->Name == "PageOptions")
                maPController->resetPaperToLastConfigured();

            updateOrientationBox();

            checkOptionalControlDependencies();

            // update preview and page settings
            preparePreview(false);
        }
    }
}

IMPL_LINK( PrintDialog, UIOption_SelectHdl, ListBox&, i_rBox, void )
{
    PropertyValue* pVal = getValueForWindow( &i_rBox );
    if( pVal )
    {
        makeEnabled( &i_rBox );

        sal_Int32 nVal( i_rBox.GetSelectedEntryPos() );
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

IMPL_LINK( PrintDialog, UIOption_ModifyHdl, Edit&, i_rBox, void )
{
    PropertyValue* pVal = getValueForWindow( &i_rBox );
    if( pVal )
    {
        makeEnabled( &i_rBox );

        NumericField* pNum = dynamic_cast<NumericField*>(&i_rBox);
        MetricField* pMetric = dynamic_cast<MetricField*>(&i_rBox);
        if( pNum )
        {
            sal_Int64 nVal = pNum->GetValue();
            pVal->Value <<= nVal;
        }
        else if( pMetric )
        {
            sal_Int64 nVal = pMetric->GetValue();
            pVal->Value <<= nVal;
        }
        else
        {
            OUString aVal( i_rBox.GetText() );
            pVal->Value <<= aVal;
        }

        checkOptionalControlDependencies();

        // update preview and page settings
        preparePreview(false);
    }
}

void PrintDialog::previewForward()
{
    mpPageEdit->Up();
}

void PrintDialog::previewBackward()
{
    mpPageEdit->Down();
}


// PrintProgressDialog

PrintProgressDialog::PrintProgressDialog(vcl::Window* i_pParent, int i_nMax)
    : ModelessDialog(i_pParent, "PrintProgressDialog", "vcl/ui/printprogressdialog.ui")
    , mbCanceled(false)
    , mnCur(0)
    , mnMax(i_nMax)
{
    get(mpButton, "cancel");
    get(mpProgress, "progressbar");
    get(mpText, "label");

    if( mnMax < 1 )
        mnMax = 1;

    maStr = mpText->GetText();

    //just multiply largest value by 10 and take the width of that string as
    //the max size we will want
    OUString aNewText( maStr.replaceFirst( "%p", OUString::number( mnMax * 10 ) ) );
    aNewText = aNewText.replaceFirst( "%n", OUString::number( mnMax * 10 ) );
    mpText->SetText( aNewText );
    mpText->set_width_request(mpText->get_preferred_size().Width());

    //Pick a useful max width
    mpProgress->set_width_request(mpProgress->LogicToPixel(Size(100, 0), MapMode(MapUnit::MapAppFont)).Width());

    mpButton->SetClickHdl( LINK( this, PrintProgressDialog, ClickHdl ) );

}

PrintProgressDialog::~PrintProgressDialog()
{
    disposeOnce();
}

void PrintProgressDialog::dispose()
{
    mpText.clear();
    mpProgress.clear();
    mpButton.clear();
    ModelessDialog::dispose();
}

IMPL_LINK( PrintProgressDialog, ClickHdl, Button*, pButton, void )
{
    if( pButton == mpButton )
        mbCanceled = true;
}

void PrintProgressDialog::setProgress( int i_nCurrent )
{
    mnCur = i_nCurrent;

    if( mnMax < 1 )
        mnMax = 1;

    mpProgress->SetValue(mnCur*100/mnMax);

    OUString aNewText( maStr.replaceFirst( "%p", OUString::number( mnCur ) ) );
    aNewText = aNewText.replaceFirst( "%n", OUString::number( mnMax ) );
    mpText->SetText( aNewText );
}

void PrintProgressDialog::tick()
{
    if( mnCur < mnMax )
        setProgress( ++mnCur );
}

void PrintProgressDialog::reset()
{
    mbCanceled = false;
    setProgress( 0 );
}
