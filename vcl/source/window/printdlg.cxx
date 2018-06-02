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
    SV_PRINT_PRT_NUP_ORIENTATION_PORTRAIT,
    SV_PRINT_PRT_NUP_ORIENTATION_LANDSCAPE
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
    static double fPreventAwkwardFactor(1.35); // 35%
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
    static double fExtraSpaceFactor(1.65); // 65%
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
, maCollateBmp(SV_PRINT_COLLATE_BMP)
, maNoCollateBmp(SV_PRINT_NOCOLLATE_BMP)
, mnCollateUIMode(0)
{

    get(mpOKButton, "ok");
    get(mpCancelButton, "cancel");
    get(mpHelpButton, "help");
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
    get(mpReverseOrderBox, "reverseorder");
    get(mpCopyCountField, "copycount");
    get(mpNupOrderWin, "orderpreview");
    get(mpNupPagesBox, "pagespersheetbox");
    get(mpNupOrientationBox, "pageorientationbox");
    get(mpNupOrderBox, "orderbox");
    get(mpPagesBtn, "pagespersheetbtn");
    get(mpBrochureBtn, "brochurebtn");
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
    for( std::vector< OUString >::const_iterator it = rQueues.begin();
         it != rQueues.end(); ++it )
    {
        mpPrinters->InsertEntry( *it );
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

    // setup dependencies
    checkControlDependencies();

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

    // setup click hdl
    mpOKButton->SetClickHdl(LINK(this, PrintDialog, ClickHdl));
    mpCancelButton->SetClickHdl(LINK(this, PrintDialog, ClickHdl));
    mpHelpButton->SetClickHdl(LINK(this, PrintDialog, ClickHdl));
    mpSetupButton->SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    mpBackwardBtn->SetClickHdl(LINK(this, PrintDialog, ClickHdl));
    mpForwardBtn->SetClickHdl(LINK(this, PrintDialog, ClickHdl));
    mpPreviewBox->SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    mpBorderCB->SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );

    // setup toggle hdl
    mpReverseOrderBox->SetToggleHdl( LINK( this, PrintDialog, ToggleHdl ) );
    mpCollateBox->SetToggleHdl( LINK( this, PrintDialog, ToggleHdl ) );
    mpPagesBtn->SetToggleHdl( LINK( this, PrintDialog, ToggleRadioHdl ) );
    mpBrochureBtn->SetToggleHdl( LINK( this, PrintDialog, ToggleRadioHdl ) );

    // setup select hdl
    mpPrinters->SetSelectHdl( LINK( this, PrintDialog, SelectHdl ) );
    mpNupPagesBox->SetSelectHdl( LINK( this, PrintDialog, SelectHdl ) );
    mpNupOrientationBox->SetSelectHdl( LINK( this, PrintDialog, SelectHdl ) );
    mpNupOrderBox->SetSelectHdl( LINK( this, PrintDialog, SelectHdl ) );

    // setup modify hdl
    mpPageEdit->SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    mpCopyCountField->SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    mpNupColEdt->SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    mpNupRowsEdt->SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );

    preparePreview( true, true );
}


PrintDialog::~PrintDialog()
{
    disposeOnce();
}

void PrintDialog::dispose()
{
    // mpCustomOptionsUIBuilder.reset();
    mpTabCtrl.clear();
    mpPreviewWindow.clear();
    mpPageEdit.clear();
    mpNumPagesText.clear();
    mpBackwardBtn.clear();
    mpForwardBtn.clear();
    mpPreviewBox.clear();
    mpOKButton.clear();
    mpCancelButton.clear();
    mpHelpButton.clear();
    maPController.reset();
    maControlToPropertyMap.clear();
    // maControlToNumValMap.clear();
    mpPrinters.clear();
    mpStatusTxt.clear();
    mpSetupButton.clear();
    mpCopyCountField.clear();
    mpCollateBox.clear();
    mpCollateImage.clear();
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
    mpNupOrientationBox.clear();
    mpNupOrderBox.clear();
    mpNupOrderWin.clear();
    mpBorderCB.clear();
    ModalDialog::dispose();
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

void PrintDialog::preparePreview( bool i_bNewPage, bool i_bMayUseCache )
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
                            aPrt->GetPaperName(),
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

    if( i_bNewPage )
    {
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
                                    aPrt->GetPaperName(),
                                    nPages > 0 ? OUString() : maNoPageStr,
                                    aPrt->GetDPIX(), aPrt->GetDPIY(),
                                    aPrt->GetPrinterOptions().IsConvertToGreyscales()
                                   );

        mpForwardBtn->Enable( mnCurPage < nPages-1 );
        mpBackwardBtn->Enable( mnCurPage != 0 );
        mpPageEdit->Enable( nPages > 1 );
    }
}

void PrintDialog::checkControlDependencies()
{

    if( mpCopyCountField->GetValue() > 1 )
        mpCollateBox->Enable( mnCollateUIMode == 0 );
    else
        mpCollateBox->Enable( false );

    Image aImg(mpCollateBox->IsChecked() ? maCollateBmp : maNoCollateBmp);

    Size aImgSize( aImg.GetSizePixel() );

    // adjust size of image
    mpCollateImage->SetSizePixel( aImgSize );
    mpCollateImage->SetImage( aImg );

    // enable setup button only for printers that can be setup
    bool bHaveSetup = maPController->getPrinter()->HasSupport( PrinterSupport::SetupDialog );
    mpSetupButton->Enable(bHaveSetup);
}

void PrintDialog::initFromMultiPageSetup( const vcl::PrinterController::MultiPageSetup& i_rMPS )
{
    mpNupOrderWin->Show();
    mpPagesBtn->Check();
    mpBrochureBtn->Show( false );

    // setup field units for metric fields
    const LocaleDataWrapper& rLocWrap( mpPageMarginEdt->GetLocaleDataWrapper() );
    FieldUnit eUnit = FUNIT_MM;
    sal_uInt16 nDigits = 0;
    if( rLocWrap.getMeasurementSystemEnum() == MeasurementSystem::US )
    {
        eUnit = FUNIT_INCH;
        nDigits = 2;
    }
    // set units
    mpPageMarginEdt->SetUnit( eUnit );
    mpSheetMarginEdt->SetUnit( eUnit );

    // set precision
    mpPageMarginEdt->SetDecimalDigits( nDigits );
    mpSheetMarginEdt->SetDecimalDigits( nDigits );

    mpSheetMarginEdt->SetValue( mpSheetMarginEdt->Normalize( i_rMPS.nLeftMargin ), FUNIT_100TH_MM );
    mpPageMarginEdt->SetValue( mpPageMarginEdt->Normalize( i_rMPS.nHorizontalSpacing ), FUNIT_100TH_MM );
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

void PrintDialog::updateNup()
{
    int nRows         = int(mpNupRowsEdt->GetValue());
    int nCols         = int(mpNupColEdt->GetValue());
    long nPageMargin  = mpPageMarginEdt->Denormalize(mpPageMarginEdt->GetValue( FUNIT_100TH_MM ));
    long nSheetMargin = mpSheetMarginEdt->Denormalize(mpSheetMarginEdt->GetValue( FUNIT_100TH_MM ));

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

    int nOrientationMode = mpNupOrientationBox->GetSelectedEntryPos();
    if( nOrientationMode == SV_PRINT_PRT_NUP_ORIENTATION_LANDSCAPE )
        aMPS.aPaperSize = maNupLandscapeSize;
    else if( nOrientationMode == SV_PRINT_PRT_NUP_ORIENTATION_PORTRAIT )
        aMPS.aPaperSize = maNupPortraitSize;
    else // automatic mode
    {
        // get size of first real page to see if it is portrait or landscape
        // we assume same page sizes for all the pages for this
        Size aPageSize = getJobPageSize();

        Size aMultiSize( aPageSize.Width() * nCols, aPageSize.Height() * nRows );
        if( aMultiSize.Width() > aMultiSize.Height() ) // fits better on landscape
            aMPS.aPaperSize = maNupLandscapeSize;
        else
            aMPS.aPaperSize = maNupPortraitSize;
    }

    maPController->setMultipage( aMPS );

    mpNupOrderWin->setValues( aMPS.nOrder, nCols, nRows );

    preparePreview( true, true );
}

void PrintDialog::updateNupFromPages()
{
    sal_IntPtr nPages = sal_IntPtr(mpNupPagesBox->GetSelectedEntryData());
    int nRows   = int(mpNupRowsEdt->GetValue());
    int nCols   = int(mpNupColEdt->GetValue());
    long nPageMargin  = mpPageMarginEdt->Denormalize(mpPageMarginEdt->GetValue( FUNIT_100TH_MM ));
    long nSheetMargin = mpSheetMarginEdt->Denormalize(mpSheetMarginEdt->GetValue( FUNIT_100TH_MM ));
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
                           std::min(nHorzMax, nVertMax) ), FUNIT_100TH_MM );

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
                           std::min(nHorzMax, nVertMax ) ), FUNIT_100TH_MM );
    }

    mpNupRowsEdt->SetValue( nRows );
    mpNupColEdt->SetValue( nCols );
    mpPageMarginEdt->SetValue( mpPageMarginEdt->Normalize( nPageMargin ), FUNIT_100TH_MM );
    mpSheetMarginEdt->SetValue( mpSheetMarginEdt->Normalize( nSheetMargin ), FUNIT_100TH_MM );

    showAdvancedControls( bCustom );

    updateNup();
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
        //storeToSettings();
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
        preparePreview( true, true );
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

            //checkOptionalControlDependencies();

            // update preview and page settings
            preparePreview();
        }
        if( mpBrochureBtn->IsChecked() )
        {
            mpNupPagesBox->SelectEntryPos( 0 );
            updateNupFromPages();
            showAdvancedControls( false );
            //enableNupControls( false );
        }
    }
    else if( pButton == mpPagesBtn )
    {
        //enableNupControls( true );
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
        preparePreview( true, true );
    }
    else if( pButton == mpBorderCB )
    {
        updateNup();
    }
    else
    {
        if( pButton == mpSetupButton )
        {
            maPController->setupPrinter(GetFrameWeld());

            // tdf#63905 don't use cache: page size may change
            preparePreview();
        }
        checkControlDependencies();
    }

}

IMPL_LINK( PrintDialog, SelectHdl, ListBox&, rBox, void )
{
    if(  &rBox == mpPrinters )
    {

        if ( rBox.GetSelectedEntryPos() != 0)
        {
            OUString aNewPrinter( rBox.GetSelectedEntry() );
            // set new printer
            maPController->setPrinter( VclPtrInstance<Printer>( aNewPrinter ) );
            maPController->resetPrinterOptions( false  );
            // update text fields
            mpOKButton->SetText( maPrintText );
            updatePrinterText();
            preparePreview();
        }
        else // print to file
        {
            // use the default printer or FIXME: the last used one?
            maPController->setPrinter( VclPtrInstance<Printer>( Printer::GetDefaultPrinterName() ) );
            mpOKButton->SetText( maPrintToFileText );
            maPController->resetPrinterOptions( true );
            preparePreview( true, true );
        }
    }
    else if( &rBox == mpNupOrientationBox || &rBox == mpNupOrderBox )
    {
       updateNup();
    }
    else if( &rBox == mpNupPagesBox )
    {
        if( !mpPagesBtn->IsChecked() )
            mpPagesBtn->Check();
        updateNupFromPages();
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
        preparePreview( true, true );
    }
    else if( &rEdit == mpCopyCountField )
    {
        maPController->setValue( "CopyCount",
                               makeAny( sal_Int32(mpCopyCountField->GetValue()) ) );
        maPController->setValue( "Collate",
                               makeAny( isCollate() ) );
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
