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

#include "printdlg.hxx"
#include "svdata.hxx"
#include "svids.hrc"
#include "jobset.h"

#include "vcl/print.hxx"
#include "vcl/dialog.hxx"
#include "vcl/button.hxx"
#include "vcl/wall.hxx"
#include "vcl/status.hxx"
#include "vcl/decoview.hxx"
#include "vcl/configsettings.hxx"
#include "vcl/help.hxx"
#include "vcl/layout.hxx"
#include "vcl/svapp.hxx"
#include "vcl/unohelp.hxx"
#include "vcl/settings.hxx"
#include "vcl/builderfactory.hxx"

#include "unotools/localedatawrapper.hxx"

#include "rtl/strbuf.hxx"

#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/awt/Size.hpp"

using namespace vcl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;

VCL_BUILDER_DECL_FACTORY(PrintPreviewWindow)
{
    (void)rMap;
    rRet = VclPtr<PrintDialog::PrintPreviewWindow>::Create(pParent);
}

VCL_BUILDER_DECL_FACTORY(ShowNupOrderWindow)
{
    (void)rMap;
    rRet = VclPtr<PrintDialog::ShowNupOrderWindow>::Create(pParent);
}

PrintDialog::PrintPreviewWindow::PrintPreviewWindow( vcl::Window* i_pParent )
    : Window( i_pParent, 0 )
    , maOrigSize( 10, 10 )
    , maPageVDev( VclPtr<VirtualDevice>::Create(*this) )
    , maToolTipString(VclResId( SV_PRINT_PRINTPREVIEW_TXT).toString())
    , mbGreyscale( false )
    , maHorzDim(VclPtr<FixedLine>::Create(this, WB_HORZ | WB_CENTER))
    , maVertDim(VclPtr<FixedLine>::Create(this, WB_VERT | WB_VCENTER))
{
    SetPaintTransparent( true );
    SetBackground();
    maPageVDev->SetBackground( Color( COL_WHITE ) );
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
    maPageVDev.disposeAndClear();
    Window::dispose();
}

const sal_Int32 PrintDialog::PrintPreviewWindow::PREVIEW_BITMAP_WIDTH = 1600;

void PrintDialog::PrintPreviewWindow::DataChanged( const DataChangedEvent& i_rDCEvt )
{
    // react on settings changed
    if( i_rDCEvt.GetType() == DataChangedEventType::SETTINGS )
    {
        maPageVDev->SetBackground( Color( COL_WHITE ) );
    }
    Window::DataChanged( i_rDCEvt );
}

void PrintDialog::PrintPreviewWindow::Resize()
{
    Size aNewSize( GetSizePixel() );
    long nTextHeight = maHorzDim->GetTextHeight();
    // leave small space for decoration
    aNewSize.Width() -= nTextHeight + 2;
    aNewSize.Height() -= nTextHeight + 2;
    Size aScaledSize;
    double fScale = 1.0;

    // #i106435# catch corner case of Size(0,0)
    Size aOrigSize( maOrigSize );
    if( aOrigSize.Width() < 1 )
        aOrigSize.Width() = aNewSize.Width();
    if( aOrigSize.Height() < 1 )
        aOrigSize.Height() = aNewSize.Height();
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
    aScaledSize.Width() = long(aScaledSize.Width()*fScale);
    aScaledSize.Height() = long(aScaledSize.Height()*fScale);

    maPreviewSize = aScaledSize;

    // #i104784# if we render the page too small then rounding issues result in
    // layout artifacts looking really bad. So scale the page unto a device that is not
    // full page size but not too small either. This also results in much better visual
    // quality of the preview, e.g. when its height approaches the number of text lines
    // find a good scaling factor

    double aAspectRatio = aScaledSize.Height() / (double) aScaledSize.Width();

    aScaledSize.Width()  = PREVIEW_BITMAP_WIDTH;
    aScaledSize.Height() = PREVIEW_BITMAP_WIDTH * aAspectRatio;

    maPageVDev->SetOutputSizePixel( aScaledSize, false );

    // position dimension lines
    Point aRef( nTextHeight + (aNewSize.Width() - maPreviewSize.Width())/2,
                nTextHeight + (aNewSize.Height() - maPreviewSize.Height())/2 );
    maHorzDim->SetPosSizePixel( Point( aRef.X(), aRef.Y() - nTextHeight ),
                               Size( maPreviewSize.Width(), nTextHeight ) );
    maVertDim->SetPosSizePixel( Point( aRef.X() - nTextHeight, aRef.Y() ),
                               Size( nTextHeight, maPreviewSize.Height() ) );

}

void PrintDialog::PrintPreviewWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
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
        Rectangle aTextRect(aOffset + Point(2, 2), Size(maPreviewSize.Width() - 4, maPreviewSize.Height() - 4));
        rRenderContext.DrawText(aTextRect, maReplacementString,
                                DrawTextFlags::Center | DrawTextFlags::VCenter |
                                DrawTextFlags::WordBreak | DrawTextFlags::MultiLine);
        rRenderContext.Pop();
    }
    else
    {
        Bitmap aPreviewBitmap(maPreviewBitmap);
        aPreviewBitmap.Scale(maPreviewSize, BmpScaleFlag::BestQuality);
        rRenderContext.DrawBitmap(aOffset, aPreviewBitmap);
    }

    Rectangle aFrameRect(aOffset + Point(-1, -1), Size(maPreviewSize.Width() + 2, maPreviewSize.Height() + 2));
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

    maOrigSize = i_rOrigSize;
    maReplacementString = i_rReplacement;
    mbGreyscale = i_bGreyscale;
    maPageVDev->SetReferenceDevice( i_nDPIX, i_nDPIY );
    maPageVDev->EnableOutput();

    // use correct measurements
    const LocaleDataWrapper& rLocWrap( GetSettings().GetLocaleDataWrapper() );
    MapUnit eUnit = MAP_MM;
    int nDigits = 0;
    if( rLocWrap.getMeasurementSystemEnum() == MEASURE_US )
    {
        eUnit = MAP_100TH_INCH;
        nDigits = 2;
    }
    Size aLogicPaperSize( LogicToLogic( i_rOrigSize, MapMode( MAP_100TH_MM ), MapMode( eUnit ) ) );
    OUString aNumText( rLocWrap.getNum( aLogicPaperSize.Width(), nDigits ) );
    aBuf.append( aNumText )
        .append( sal_Unicode( ' ' ) );
    aBuf.appendAscii( eUnit == MAP_MM ? "mm" : "in" );
    if( !i_rPaperName.isEmpty() )
    {
        aBuf.append( " (" );
        aBuf.append( i_rPaperName );
        aBuf.append( ')' );
    }
    maHorzDim->SetText( aBuf.makeStringAndClear() );

    aNumText = rLocWrap.getNum( aLogicPaperSize.Height(), nDigits );
    aBuf.append( aNumText )
        .append( sal_Unicode( ' ' ) );
    aBuf.appendAscii( eUnit == MAP_MM ? "mm" : "in" );
    maVertDim->SetText( aBuf.makeStringAndClear() );

    Resize();
    preparePreviewBitmap();
    Invalidate();
}

void PrintDialog::PrintPreviewWindow::preparePreviewBitmap()
{
    GDIMetaFile aMtf( maMtf );

    Size aVDevSize( maPageVDev->GetOutputSizePixel() );
    const Size aLogicSize( maPageVDev->PixelToLogic( aVDevSize, MapMode( MAP_100TH_MM ) ) );
    Size aOrigSize( maOrigSize );
    if( aOrigSize.Width() < 1 )
        aOrigSize.Width() = aLogicSize.Width();
    if( aOrigSize.Height() < 1 )
        aOrigSize.Height() = aLogicSize.Height();
    double fScale = double(aLogicSize.Width())/double(aOrigSize.Width());

    maPageVDev->Erase();
    maPageVDev->Push();
    maPageVDev->SetMapMode( MAP_100TH_MM );
    DrawModeFlags nOldDrawMode = maPageVDev->GetDrawMode();
    if( mbGreyscale )
        maPageVDev->SetDrawMode( maPageVDev->GetDrawMode() |
                                ( DrawModeFlags::GrayLine | DrawModeFlags::GrayFill | DrawModeFlags::GrayText |
                                  DrawModeFlags::GrayBitmap | DrawModeFlags::GrayGradient ) );
    aMtf.WindStart();
    aMtf.Scale( fScale, fScale );
    aMtf.WindStart();

    const AntialiasingFlags nOriginalAA(maPageVDev->GetAntialiasing());
    maPageVDev->SetAntialiasing(nOriginalAA | AntialiasingFlags::EnableB2dDraw);
    aMtf.Play( maPageVDev.get(), Point( 0, 0 ), aLogicSize );
    maPageVDev->SetAntialiasing(nOriginalAA);

    maPageVDev->Pop();

    SetMapMode( MAP_PIXEL );
    maPageVDev->SetMapMode( MAP_PIXEL );

    maPreviewBitmap = Bitmap(maPageVDev->GetBitmap(Point(0, 0), aVDevSize));

    maPageVDev->SetDrawMode( nOldDrawMode );
}

PrintDialog::ShowNupOrderWindow::ShowNupOrderWindow( vcl::Window* i_pParent )
    : Window( i_pParent, WB_NOBORDER )
    , mnOrderMode( 0 )
    , mnRows( 1 )
    , mnColumns( 1 )
{
    ImplInitSettings();
}

void PrintDialog::ShowNupOrderWindow::ImplInitSettings()
{
    SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetFieldColor() ) );
}

Size PrintDialog::ShowNupOrderWindow::GetOptimalSize() const
{
    return Size(70, 70);
}

void PrintDialog::ShowNupOrderWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle& i_rRect)
{
    Window::Paint(rRenderContext, i_rRect);

    rRenderContext.SetMapMode(MAP_PIXEL);
    rRenderContext.SetTextColor(rRenderContext.GetSettings().GetStyleSettings().GetFieldTextColor());

    int nPages = mnRows * mnColumns;
    Font aFont(rRenderContext.GetSettings().GetStyleSettings().GetFieldFont());
    aFont.SetSize(Size(0, 24));
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
    aFont.SetSize(Size( 0, nFontHeight));
    rRenderContext.SetFont(aFont);
    long nTextHeight = rRenderContext.GetTextHeight();
    for (int i = 0; i < nPages; i++)
    {
        OUString aPageText(OUString::number(i + 1));
        int nX = 0, nY = 0;
        switch (mnOrderMode)
        {
        case SV_PRINT_PRT_NUP_ORDER_LRTB:
            nX = (i % mnColumns);
            nY = (i / mnColumns);
            break;
        case SV_PRINT_PRT_NUP_ORDER_TBLR:
            nX = (i / mnRows);
            nY = (i % mnRows);
            break;
        case SV_PRINT_PRT_NUP_ORDER_RLTB:
            nX = mnColumns - 1 - (i % mnColumns);
            nY = (i / mnColumns);
            break;
        case SV_PRINT_PRT_NUP_ORDER_TBRL:
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
    aDecorationView.DrawFrame(Rectangle(Point(0, 0), aOutSize), DrawFrameStyle::Group);
}

PrintDialog::NUpTabPage::NUpTabPage( VclBuilder *pUIBuilder )
{
    pUIBuilder->get(mpPagesBtn, "pagespersheetbtn");
    pUIBuilder->get(mpBrochureBtn, "brochure");
    pUIBuilder->get(mpPagesBoxTitleTxt, "pagespersheettxt");
    pUIBuilder->get(mpNupPagesBox, "paperspersheetlb");
    pUIBuilder->get(mpNupNumPagesTxt, "pagestxt");
    pUIBuilder->get(mpNupColEdt, "pagecols");
    pUIBuilder->get(mpNupTimesTxt, "by");
    pUIBuilder->get(mpNupRowsEdt, "pagerows");
    pUIBuilder->get(mpPageMarginTxt1, "pagemargintxt1");
    pUIBuilder->get(mpPageMarginEdt, "pagemarginsb");
    pUIBuilder->get(mpPageMarginTxt2, "pagemargintxt2");
    pUIBuilder->get(mpSheetMarginTxt1, "sheetmargintxt1");
    pUIBuilder->get(mpSheetMarginEdt, "sheetmarginsb");
    pUIBuilder->get(mpSheetMarginTxt2, "sheetmargintxt2");
    pUIBuilder->get(mpNupOrientationTxt, "orientationtxt");
    pUIBuilder->get(mpNupOrientationBox, "orientationlb");
    pUIBuilder->get(mpNupOrderTxt, "ordertxt");
    pUIBuilder->get(mpNupOrderBox, "orderlb");
    pUIBuilder->get(mpNupOrderWin, "orderpreview");
    pUIBuilder->get(mpBorderCB, "bordercb");
}

void PrintDialog::NUpTabPage::enableNupControls( bool bEnable )
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
    mpNupOrientationTxt->Enable( bEnable );
    mpNupOrientationBox->Enable( bEnable );
    mpNupOrderTxt->Enable( bEnable );
    mpNupOrderBox->Enable( bEnable );
    mpNupOrderWin->Enable( bEnable );
    mpBorderCB->Enable( bEnable );
}

void PrintDialog::NUpTabPage::showAdvancedControls( bool i_bShow )
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
    mpNupOrientationTxt->Show( i_bShow );
    mpNupOrientationBox->Show( i_bShow );
}

void PrintDialog::NUpTabPage::initFromMultiPageSetup( const vcl::PrinterController::MultiPageSetup& i_rMPS )
{
    mpNupOrderWin->Show();
    mpPagesBtn->Check();
    mpBrochureBtn->Show( false );

    // setup field units for metric fields
    const LocaleDataWrapper& rLocWrap( mpPageMarginEdt->GetLocaleDataWrapper() );
    FieldUnit eUnit = FUNIT_MM;
    sal_uInt16 nDigits = 0;
    if( rLocWrap.getMeasurementSystemEnum() == MEASURE_US )
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
    mpNupOrderBox->SelectEntryPos( i_rMPS.nOrder );
    if( i_rMPS.nRows != 1 || i_rMPS.nColumns != 1 )
    {
        mpNupPagesBox->SelectEntryPos( mpNupPagesBox->GetEntryCount()-1 );
        showAdvancedControls( true );
        mpNupOrderWin->setValues( i_rMPS.nOrder, i_rMPS.nColumns, i_rMPS.nRows );
    }
}

PrintDialog::JobTabPage::JobTabPage( VclBuilder* pUIBuilder )
    : maCollateImg( VclResId( SV_PRINT_COLLATE_IMG ) )
    , maNoCollateImg( VclResId( SV_PRINT_NOCOLLATE_IMG ) )
    , mnCollateUIMode( 0 )
{
    pUIBuilder->get(mpPrinters, "printers");
    mpPrinters->SetStyle(mpPrinters->GetStyle() | WB_SORT);
    pUIBuilder->get(mpStatusTxt, "status");
    pUIBuilder->get(mpLocationTxt, "location");
    pUIBuilder->get(mpCommentTxt, "comment");
    pUIBuilder->get(mpSetupButton, "setup");
    pUIBuilder->get(mpCopySpacer, "copyspacer");
    pUIBuilder->get(mpCopyCountField, "copycount");
    pUIBuilder->get(mpCollateBox, "collate");
    pUIBuilder->get(mpCollateImage, "collateimage");
    pUIBuilder->get(mpReverseOrderBox, "reverseorder");
    // HACK: this is not a dropdown box, but the dropdown line count
    // sets the results of GetOptimalSize in a normal ListBox
    mpPrinters->SetDropDownLineCount( 4 );
}

void PrintDialog::JobTabPage::readFromSettings()
{
    SettingsConfigItem* pItem = SettingsConfigItem::get();
    OUString aValue;

    aValue = pItem->getValue( "PrintDialog",
                              "CollateBox" );
    if( aValue.equalsIgnoreAsciiCase("alwaysoff") )
    {
        mnCollateUIMode = 1;
        mpCollateBox->Check( false );
        mpCollateBox->Enable( false );
    }
    else
    {
        mnCollateUIMode = 0;
        aValue = pItem->getValue( "PrintDialog",
                                  "Collate" );
        mpCollateBox->Check( aValue.equalsIgnoreAsciiCase("true") );
    }
}

void PrintDialog::JobTabPage::storeToSettings()
{
    SettingsConfigItem* pItem = SettingsConfigItem::get();
    pItem->setValue( "PrintDialog",
                     "CopyCount",
                     mpCopyCountField->GetText() );
    pItem->setValue( "PrintDialog",
                     "Collate",
                     mpCollateBox->IsChecked() ? OUString("true") :
                                                 OUString("false") );
}

PrintDialog::OutputOptPage::OutputOptPage( VclBuilder *pUIBuilder )
{
    pUIBuilder->get(mpToFileBox, "printtofile");
    pUIBuilder->get(mpCollateSingleJobsBox, "singleprintjob");
    pUIBuilder->get(mpPapersizeFromSetup, "papersizefromsetup");
}

void PrintDialog::OutputOptPage::readFromSettings()
{
    SettingsConfigItem* pItem = SettingsConfigItem::get();
    OUString aValue;
    aValue = pItem->getValue( "PrintDialog",
                              "CollateSingleJobs" );
    if ( aValue.equalsIgnoreAsciiCase("true") )
    {
        mpCollateSingleJobsBox->Check();
    }
    else
    {
        mpCollateSingleJobsBox->Check( false );
    }
}

void PrintDialog::OutputOptPage::storeToSettings()
{
    SettingsConfigItem* pItem = SettingsConfigItem::get();
    pItem->setValue( "PrintDialog",
                     "ToFile",
                     mpToFileBox->IsChecked() ? OUString("true")
                                             : OUString("false") );
    pItem->setValue( "PrintDialog",
                     "CollateSingleJobs",
                     mpCollateSingleJobsBox->IsChecked() ? OUString("true") :
                                                OUString("false") );
}

PrintDialog::PrintDialog( vcl::Window* i_pParent, const std::shared_ptr<PrinterController>& i_rController )
    : ModalDialog(i_pParent, "PrintDialog", "vcl/ui/printdialog.ui")
    , mpCustomOptionsUIBuilder(nullptr)
    , maPController( i_rController )
    , maNUpPage(m_pUIBuilder)
    , maJobPage(m_pUIBuilder)
    , maOptionsPage(m_pUIBuilder)
    , maNoPageStr( VclResId( SV_PRINT_NOPAGES ).toString() )
    , mnCurPage( 0 )
    , mnCachedPages( 0 )
    , maPrintToFileText( VclResId( SV_PRINT_TOFILE_TXT ).toString() )
    , maDefPrtText( VclResId( SV_PRINT_DEFPRT_TXT ).toString() )
    , mbShowLayoutPage( true )
{
    get(mpOKButton, "ok");
    get(mpCancelButton, "cancel");
    get(mpHelpButton, "help");
    get(mpForwardBtn, "forward");
    get(mpBackwardBtn, "backward");
    get(mpNumPagesText, "totalnumpages");
    get(mpPageEdit, "pageedit-nospin");
    get(mpTabCtrl, "tabcontrol");
    get(mpPreviewWindow, "preview");

    // save printbutton text, gets exchanged occasionally with print to file
    maPrintText = mpOKButton->GetText();

    // setup preview controls
    mpForwardBtn->SetStyle( mpForwardBtn->GetStyle() | WB_BEVELBUTTON );
    mpBackwardBtn->SetStyle( mpBackwardBtn->GetStyle() | WB_BEVELBUTTON );

    maPageStr = mpNumPagesText->GetText();

    // init reverse print
    maJobPage.mpReverseOrderBox->Check( maPController->getReversePrint() );

    Printer::updatePrinters();

    // fill printer listbox
    const std::vector< OUString >& rQueues( Printer::GetPrinterQueues() );
    for( std::vector< OUString >::const_iterator it = rQueues.begin();
         it != rQueues.end(); ++it )
    {
        maJobPage.mpPrinters->InsertEntry( *it );
    }
    // select current printer
    if( maJobPage.mpPrinters->GetEntryPos( maPController->getPrinter()->GetName() ) != LISTBOX_ENTRY_NOTFOUND )
    {
        maJobPage.mpPrinters->SelectEntry( maPController->getPrinter()->GetName() );
    }
    else
    {
        // fall back to last printer
        SettingsConfigItem* pItem = SettingsConfigItem::get();
        OUString aValue( pItem->getValue( "PrintDialog",
                                        "LastPrinter" ) );
        if( maJobPage.mpPrinters->GetEntryPos( aValue ) != LISTBOX_ENTRY_NOTFOUND )
        {
            maJobPage.mpPrinters->SelectEntry( aValue );
            maPController->setPrinter( VclPtrInstance<Printer>( aValue ) );
        }
        else
        {
            // fall back to default printer
            maJobPage.mpPrinters->SelectEntry( Printer::GetDefaultPrinterName() );
            maPController->setPrinter( VclPtrInstance<Printer>( Printer::GetDefaultPrinterName() ) );
        }
    }
    // not printing to file
    maPController->resetPrinterOptions( false );

    // get the first page
    preparePreview( true, true );

    // update the text fields for the printer
    updatePrinterText();

    // set a select handler
    maJobPage.mpPrinters->SetSelectHdl( LINK( this, PrintDialog, SelectHdl ) );

    // setup sizes for N-Up
    Size aNupSize( maPController->getPrinter()->PixelToLogic(
                         maPController->getPrinter()->GetPaperSizePixel(), MapMode( MAP_100TH_MM ) ) );
    if( maPController->getPrinter()->GetOrientation() == ORIENTATION_LANDSCAPE )
    {
        maNupLandscapeSize = aNupSize;
        maNupPortraitSize = Size( aNupSize.Height(), aNupSize.Width() );
    }
    else
    {
        maNupPortraitSize = aNupSize;
        maNupLandscapeSize = Size( aNupSize.Height(), aNupSize.Width() );
    }
    maNUpPage.initFromMultiPageSetup( maPController->getMultipage() );

    // setup click handler on the various buttons
    mpOKButton->SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    #if OSL_DEBUG_LEVEL > 1
    mpCancelButton->SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    #endif
    mpHelpButton->SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    mpForwardBtn->SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    mpBackwardBtn->SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );

    maJobPage.mpCollateBox->SetToggleHdl( LINK( this, PrintDialog, ToggleHdl ) );
    maJobPage.mpSetupButton->SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    maNUpPage.mpBorderCB->SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    maOptionsPage.mpToFileBox->SetToggleHdl( LINK( this, PrintDialog, ToggleHdl ) );
    maOptionsPage.mpPapersizeFromSetup->SetToggleHdl( LINK( this, PrintDialog, ToggleHdl ) );
    maJobPage.mpReverseOrderBox->SetToggleHdl( LINK( this, PrintDialog, ToggleHdl ) );
    maOptionsPage.mpCollateSingleJobsBox->SetToggleHdl( LINK( this, PrintDialog, ToggleHdl ) );
    maNUpPage.mpPagesBtn->SetToggleHdl( LINK( this, PrintDialog, ToggleRadioHdl ) );
    // setup modify hdl
    mpPageEdit->SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    maJobPage.mpCopyCountField->SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    maNUpPage.mpNupRowsEdt->SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    maNUpPage.mpNupColEdt->SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    maNUpPage.mpPageMarginEdt->SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    maNUpPage.mpSheetMarginEdt->SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );

    // setup select hdl
    maNUpPage.mpNupPagesBox->SetSelectHdl( LINK( this, PrintDialog, SelectHdl ) );
    maNUpPage.mpNupOrientationBox->SetSelectHdl( LINK( this, PrintDialog, SelectHdl ) );
    maNUpPage.mpNupOrderBox->SetSelectHdl( LINK( this, PrintDialog, SelectHdl ) );

    // setup optional UI options set by application
    setupOptionalUI();

    // remove layout page if unwanted
    if (!mbShowLayoutPage)
        mpTabCtrl->RemovePage(mpTabCtrl->GetPageId(2));

    // restore settings from last run
    readFromSettings();

    // setup dependencies
    checkControlDependencies();

    if ( maPController->getBoolProperty( "HideHelpButton", false ) )
        mpHelpButton->Hide();
    // set initial focus to "Number of copies"
    maJobPage.mpCopyCountField->GrabFocus();
    maJobPage.mpCopyCountField->SetSelection( Selection(0, 0xFFFF) );

    updateNupFromPages();
}

PrintDialog::~PrintDialog()
{
    disposeOnce();
}

void PrintDialog::dispose()
{
    delete mpCustomOptionsUIBuilder;
    mpTabCtrl.clear();
    mpPreviewWindow.clear();
    mpPageEdit.clear();
    mpNumPagesText.clear();
    mpBackwardBtn.clear();
    mpForwardBtn.clear();
    mpOKButton.clear();
    mpCancelButton.clear();
    mpHelpButton.clear();
    maPController.reset();
    ModalDialog::dispose();
}

void PrintDialog::readFromSettings()
{
    maJobPage.readFromSettings();
    maOptionsPage.readFromSettings();

    // read last selected tab page; if it exists, activate it
    SettingsConfigItem* pItem = SettingsConfigItem::get();
    OUString aValue = pItem->getValue( "PrintDialog",
                                            "LastPage" );
    sal_uInt16 nCount = mpTabCtrl->GetPageCount();
    for( sal_uInt16 i = 0; i < nCount; i++ )
    {
        sal_uInt16 nPageId = mpTabCtrl->GetPageId( i );
        if( aValue.equals( mpTabCtrl->GetPageText( nPageId ) ) )
        {
            mpTabCtrl->SelectTabPage( nPageId );
            break;
        }
    }
    mpOKButton->SetText( maOptionsPage.mpToFileBox->IsChecked() ? maPrintToFileText : maPrintText );

    // persistent window state
    OUString aWinState( pItem->getValue( "PrintDialog",
                                              "WindowState" ) );
    if( !aWinState.isEmpty() )
        SetWindowState( OUStringToOString( aWinState, RTL_TEXTENCODING_UTF8 ) );

    if( maOptionsPage.mpToFileBox->IsChecked() )
    {
        maPController->resetPrinterOptions( true );
        preparePreview( true, true );
    }
}

void PrintDialog::storeToSettings()
{
    maJobPage.storeToSettings();
    maOptionsPage.storeToSettings();

    // store last selected printer
    SettingsConfigItem* pItem = SettingsConfigItem::get();
    pItem->setValue( "PrintDialog",
                     "LastPrinter",
                     maJobPage.mpPrinters->GetSelectEntry() );

    pItem->setValue( "PrintDialog",
                     "LastPage",
                     mpTabCtrl->GetPageText( mpTabCtrl->GetCurPageId() ) );
    pItem->setValue( "PrintDialog",
                     "WindowState",
                     OStringToOUString( GetWindowState(), RTL_TEXTENCODING_UTF8 )
                     );
    pItem->Commit();
}

bool PrintDialog::isPrintToFile()
{
    return maOptionsPage.mpToFileBox->IsChecked();
}

bool PrintDialog::isCollate()
{
    return maJobPage.mpCopyCountField->GetValue() > 1 && maJobPage.mpCollateBox->IsChecked();
}

bool PrintDialog::isSingleJobs()
{
    return maOptionsPage.mpCollateSingleJobsBox->IsChecked();
}

void setHelpId( vcl::Window* i_pWindow, const Sequence< OUString >& i_rHelpIds, sal_Int32 i_nIndex )
{
    if( i_nIndex >= 0 && i_nIndex < i_rHelpIds.getLength() )
        i_pWindow->SetHelpId( OUStringToOString( i_rHelpIds.getConstArray()[i_nIndex], RTL_TEXTENCODING_UTF8 ) );
}

static void setHelpText( vcl::Window* i_pWindow, const Sequence< OUString >& i_rHelpTexts, sal_Int32 i_nIndex )
{
    // without a help text set and the correct smartID,
    // help texts will be retrieved from the online help system
    if( i_nIndex >= 0 && i_nIndex < i_rHelpTexts.getLength() )
        i_pWindow->SetHelpText( i_rHelpTexts.getConstArray()[i_nIndex] );
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

            delete mpCustomOptionsUIBuilder;
            mpCustomOptionsUIBuilder = new VclBuilder(pCustom, getUIRootDir(), sOptionsUIFile);
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
        OUString aDependsOnName;
        sal_Int32 nDependsOnValue = 0;
        bool bUseDependencyRow = false;

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
                bool bValue = true;
                rEntry.Value >>= bValue;
            }
            else if ( rEntry.Name == "GroupingHint" )
            {
                rEntry.Value >>= aGroupingHint;
            }
            else if ( rEntry.Name == "DependsOnName" )
            {
                rEntry.Value >>= aDependsOnName;
            }
            else if ( rEntry.Name == "DependsOnEntry" )
            {
                rEntry.Value >>= nDependsOnValue;
            }
            else if ( rEntry.Name == "AttachToDependency" )
            {
                rEntry.Value >>= bUseDependencyRow;
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
                    if( (rEntry.Value >>= aHelpText) )
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
                    if( (rEntry.Value >>= aHelpId) )
                    {
                        aHelpIds.realloc( 1 );
                        *aHelpIds.getArray() = aHelpId;
                    }
                }
            }
            else if ( rEntry.Name == "HintNoLayoutPage" )
            {
                bool bNoLayoutPage = false;
                rEntry.Value >>= bNoLayoutPage;
                mbShowLayoutPage = ! bNoLayoutPage;
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
            maNUpPage.mpBrochureBtn->SetText( aText );
            maNUpPage.mpBrochureBtn->Show();

            bool bVal = false;
            PropertyValue* pVal = maPController->getValue( aPropertyName );
            if( pVal )
                pVal->Value >>= bVal;
            maNUpPage.mpBrochureBtn->Check( bVal );
            maNUpPage.mpBrochureBtn->Enable( maPController->isUIOptionEnabled( aPropertyName ) && pVal != nullptr );
            maNUpPage.mpBrochureBtn->SetToggleHdl( LINK( this, PrintDialog, ToggleRadioHdl ) );

            maPropertyToWindowMap[ aPropertyName ].push_back( maNUpPage.mpBrochureBtn );
            maControlToPropertyMap[maNUpPage.mpBrochureBtn] = aPropertyName;

            // set help id
            setHelpId( maNUpPage.mpBrochureBtn, aHelpIds, 0 );
            // set help text
            setHelpText( maNUpPage.mpBrochureBtn, aHelpTexts, 0 );
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

            maPropertyToWindowMap[ aPropertyName ].push_back( pNewBox );
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
                maPropertyToWindowMap[ aPropertyName ].push_back( pBtn );
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

            maPropertyToWindowMap[ aPropertyName ].push_back( pList );
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

            maPropertyToWindowMap[ aPropertyName ].push_back( pField );
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

            maPropertyToWindowMap[ aPropertyName ].push_back( pField );
            maControlToPropertyMap[pField] = aPropertyName;
        }
        else
        {
            OStringBuffer sMessage;
            sMessage.append("Unsupported UI option: \"");
            sMessage.append(OUStringToOString(aCtrlType, RTL_TEXTENCODING_UTF8));
            sMessage.append('"');
            OSL_FAIL( sMessage.getStr() );
        }
    }

    // #i106506# if no brochure button, then the singular Pages radio button
    // makes no sense, so replace it by a FixedText label
    if (!maNUpPage.mpBrochureBtn->IsVisible() && maNUpPage.mpPagesBtn->IsVisible())
    {
        maNUpPage.mpPagesBoxTitleTxt->SetText( maNUpPage.mpPagesBtn->GetText() );
        maNUpPage.mpPagesBoxTitleTxt->Show();
        maNUpPage.mpPagesBtn->Show( false );
    }

    // update enable states
    checkOptionalControlDependencies();

    vcl::Window *pPageRange = get<vcl::Window>("pagerange");

    // print range not shown (currently math only) -> hide spacer line and reverse order
    if (!pPageRange || !pPageRange->IsVisible())
    {
        maJobPage.mpCopySpacer->Show( false );
        maJobPage.mpReverseOrderBox->Show( false );
    }

    if (!mpCustomOptionsUIBuilder)
        mpTabCtrl->RemovePage(mpTabCtrl->GetPageId(1));
}

void PrintDialog::DataChanged( const DataChangedEvent& i_rDCEvt )
{
    // react on settings changed
    if( i_rDCEvt.GetType() == DataChangedEventType::SETTINGS )
        checkControlDependencies();
    ModalDialog::DataChanged( i_rDCEvt );
}

void PrintDialog::checkControlDependencies()
{
    if( maJobPage.mpCopyCountField->GetValue() > 1 )
        maJobPage.mpCollateBox->Enable( maJobPage.mnCollateUIMode == 0 );
    else
        maJobPage.mpCollateBox->Enable( false );

    Image aImg( maJobPage.mpCollateBox->IsChecked() ? maJobPage.maCollateImg : maJobPage.maNoCollateImg );

    Size aImgSize( aImg.GetSizePixel() );

    // adjust size of image
    maJobPage.mpCollateImage->SetSizePixel( aImgSize );
    maJobPage.mpCollateImage->SetImage( aImg );

    // enable setup button only for printers that can be setup
    bool bHaveSetup = maPController->getPrinter()->HasSupport( SUPPORT_SETUPDIALOG );
    maJobPage.mpSetupButton->Enable(bHaveSetup);
}

void PrintDialog::checkOptionalControlDependencies()
{
    for( auto it = maControlToPropertyMap.begin();
         it != maControlToPropertyMap.end(); ++it )
    {
        bool bShouldbeEnabled = maPController->isUIOptionEnabled( it->second );
        if( ! bShouldbeEnabled )
        {
            // enable controls that are directly attached to a dependency anyway
            // if the normally disabled controls get modified, change the dependency
            // so the control would be enabled
            // example: in print range "Print All" is selected, "Page Range" is then of course
            // not selected and the Edit for the Page Range would be disabled
            // as a convenience we should enable the Edit anyway and automatically select
            // "Page Range" instead of "Print All" if the Edit gets modified
            if( maReverseDependencySet.find( it->second ) != maReverseDependencySet.end() )
            {
                OUString aDep( maPController->getDependency( it->second ) );
                // if the dependency is at least enabled, then enable this control anyway
                if( !aDep.isEmpty() && maPController->isUIOptionEnabled( aDep ) )
                    bShouldbeEnabled = true;
            }
        }

        if( bShouldbeEnabled && dynamic_cast<RadioButton*>(it->first.get()) )
        {
            auto r_it = maControlToNumValMap.find( it->first );
            if( r_it != maControlToNumValMap.end() )
            {
                bShouldbeEnabled = maPController->isUIChoiceEnabled( it->second, r_it->second );
            }
        }

        bool bIsEnabled = it->first->IsEnabled();
        // Enable does not do a change check first, so can be less cheap than expected
        if( bShouldbeEnabled != bIsEnabled )
            it->first->Enable( bShouldbeEnabled );
    }
}

static OUString searchAndReplace( const OUString& i_rOrig, const char* i_pRepl, sal_Int32 i_nReplLen, const OUString& i_rRepl )
{
    sal_Int32 nPos = i_rOrig.indexOfAsciiL( i_pRepl, i_nReplLen );
    if( nPos != -1 )
    {
        OUStringBuffer aBuf( i_rOrig.getLength() );
        aBuf.append( i_rOrig.getStr(), nPos );
        aBuf.append( i_rRepl );
        if( nPos + i_nReplLen < i_rOrig.getLength() )
            aBuf.append( i_rOrig.getStr() + nPos + i_nReplLen );
        return aBuf.makeStringAndClear();
    }
    return i_rOrig;
}

void PrintDialog::updatePrinterText()
{
    const OUString aDefPrt( Printer::GetDefaultPrinterName() );
    const QueueInfo* pInfo = Printer::GetQueueInfo( maJobPage.mpPrinters->GetSelectEntry(), true );
    if( pInfo )
    {
        maJobPage.mpLocationTxt->SetText( pInfo->GetLocation() );
        maJobPage.mpCommentTxt->SetText( pInfo->GetComment() );
        // FIXME: status text
        OUString aStatus;
        if( aDefPrt == pInfo->GetPrinterName() )
            aStatus = maDefPrtText;
        maJobPage.mpStatusTxt->SetText( aStatus );
    }
    else
    {
        maJobPage.mpLocationTxt->SetText( OUString() );
        maJobPage.mpCommentTxt->SetText( OUString() );
        maJobPage.mpStatusTxt->SetText( OUString() );
    }
}

void PrintDialog::setPreviewText( sal_Int32 )
{
    OUString aNewText( searchAndReplace( maPageStr, "%n", 2, OUString::number( mnCachedPages )  ) );
    mpNumPagesText->SetText( aNewText );
}

void PrintDialog::preparePreview( bool i_bNewPage, bool i_bMayUseCache )
{
    // page range may have changed depending on options
    sal_Int32 nPages = maPController->getFilteredPageCount();
    mnCachedPages = nPages;

    if( mnCurPage >= nPages )
        mnCurPage = nPages-1;
    if( mnCurPage < 0 )
        mnCurPage = 0;

    setPreviewText( mnCurPage );

    mpPageEdit->SetMin( 1 );
    mpPageEdit->SetMax( nPages );

    if( i_bNewPage )
    {
        const MapMode aMapMode( MAP_100TH_MM );
        GDIMetaFile aMtf;
        VclPtr<Printer> aPrt( maPController->getPrinter() );
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

        Size aCurPageSize = aPrt->PixelToLogic( aPrt->GetPaperSizePixel(), MapMode( MAP_100TH_MM ) );
        mpPreviewWindow->setPreview( aMtf, aCurPageSize,
                                    aPrt->GetPaperName( false ),
                                    nPages > 0 ? OUString() : maNoPageStr,
                                    aPrt->GetDPIX(), aPrt->GetDPIY(),
                                    aPrt->GetPrinterOptions().IsConvertToGreyscales()
                                   );

        mpForwardBtn->Enable( mnCurPage < nPages-1 );
        mpBackwardBtn->Enable( mnCurPage != 0 );
        mpPageEdit->Enable( nPages > 1 );
    }
}

Size PrintDialog::getJobPageSize()
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

void PrintDialog::updateNupFromPages()
{
    sal_IntPtr nPages = sal_IntPtr(maNUpPage.mpNupPagesBox->GetSelectEntryData());
    int nRows   = int(maNUpPage.mpNupRowsEdt->GetValue());
    int nCols   = int(maNUpPage.mpNupColEdt->GetValue());
    long nPageMargin  = long(maNUpPage.mpPageMarginEdt->Denormalize(maNUpPage.mpPageMarginEdt->GetValue( FUNIT_100TH_MM )));
    long nSheetMargin = long(maNUpPage.mpSheetMarginEdt->Denormalize(maNUpPage.mpSheetMarginEdt->GetValue( FUNIT_100TH_MM )));
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
                nRows = 1, nCols = 2;
            else
                nRows = 2, nCols = 1;
        }
        else if( nPages == 4 )
            nRows = nCols = 2;
        else if( nPages == 6 )
        {
            if( bPortrait )
                nRows = 2, nCols = 3;
            else
                nRows = 3, nCols = 2;
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

        maNUpPage.mpSheetMarginEdt->SetMax(
                  maNUpPage.mpSheetMarginEdt->Normalize(
                           nHorzMax > nVertMax ? nVertMax : nHorzMax ), FUNIT_100TH_MM );

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

        maNUpPage.mpPageMarginEdt->SetMax(
                 maNUpPage.mpSheetMarginEdt->Normalize(
                           nHorzMax > nVertMax ? nVertMax : nHorzMax ), FUNIT_100TH_MM );
    }

    maNUpPage.mpNupRowsEdt->SetValue( nRows );
    maNUpPage.mpNupColEdt->SetValue( nCols );
    maNUpPage.mpPageMarginEdt->SetValue( maNUpPage.mpPageMarginEdt->Normalize( nPageMargin ), FUNIT_100TH_MM );
    maNUpPage.mpSheetMarginEdt->SetValue( maNUpPage.mpSheetMarginEdt->Normalize( nSheetMargin ), FUNIT_100TH_MM );

    maNUpPage.showAdvancedControls( bCustom );

    updateNup();
}

void PrintDialog::updateNup()
{
    int nRows         = int(maNUpPage.mpNupRowsEdt->GetValue());
    int nCols         = int(maNUpPage.mpNupColEdt->GetValue());
    long nPageMargin  = long(maNUpPage.mpPageMarginEdt->Denormalize(maNUpPage.mpPageMarginEdt->GetValue( FUNIT_100TH_MM )));
    long nSheetMargin = long(maNUpPage.mpSheetMarginEdt->Denormalize(maNUpPage.mpSheetMarginEdt->GetValue( FUNIT_100TH_MM )));

    PrinterController::MultiPageSetup aMPS;
    aMPS.nRows         = nRows;
    aMPS.nColumns      = nCols;
    aMPS.nRepeat       = 1;
    aMPS.nLeftMargin   =
    aMPS.nTopMargin    =
    aMPS.nRightMargin  =
    aMPS.nBottomMargin = nSheetMargin;

    aMPS.nHorizontalSpacing =
    aMPS.nVerticalSpacing   = nPageMargin;

    aMPS.bDrawBorder        = maNUpPage.mpBorderCB->IsChecked();

    int nOrderMode = maNUpPage.mpNupOrderBox->GetSelectEntryPos();
    if( nOrderMode == SV_PRINT_PRT_NUP_ORDER_LRTB )
        aMPS.nOrder = PrinterController::LRTB;
    else if( nOrderMode == SV_PRINT_PRT_NUP_ORDER_TBLR )
        aMPS.nOrder = PrinterController::TBLR;
    else if( nOrderMode == SV_PRINT_PRT_NUP_ORDER_RLTB )
        aMPS.nOrder = PrinterController::RLTB;
    else if( nOrderMode == SV_PRINT_PRT_NUP_ORDER_TBRL )
        aMPS.nOrder = PrinterController::TBRL;

    int nOrientationMode = maNUpPage.mpNupOrientationBox->GetSelectEntryPos();
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

    maNUpPage.mpNupOrderWin->setValues( nOrderMode, nCols, nRows );

    preparePreview( true, true );
}

IMPL_LINK_TYPED( PrintDialog, SelectHdl, ListBox&, rBox, void )
{
    if(  &rBox == maJobPage.mpPrinters )
    {
        OUString aNewPrinter( rBox.GetSelectEntry() );
        // set new printer
        maPController->setPrinter( VclPtrInstance<Printer>( aNewPrinter ) );
        maPController->resetPrinterOptions( maOptionsPage.mpToFileBox->IsChecked() );
        // update text fields
        updatePrinterText();
        preparePreview();
    }
    else if( &rBox == maNUpPage.mpNupOrientationBox || &rBox == maNUpPage.mpNupOrderBox )
    {
        updateNup();
    }
    else if( &rBox == maNUpPage.mpNupPagesBox )
    {
        if( !maNUpPage.mpPagesBtn->IsChecked() )
            maNUpPage.mpPagesBtn->Check();
        updateNupFromPages();
    }
}

IMPL_LINK_TYPED( PrintDialog, ToggleRadioHdl, RadioButton&, rButton, void )
{
    ClickHdl(static_cast<Button*>(&rButton));
}

IMPL_LINK_TYPED( PrintDialog, ToggleHdl, CheckBox&, rButton, void )
{
    ClickHdl(&rButton);
}

IMPL_LINK_TYPED( PrintDialog, ClickHdl, Button*, pButton, void )
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
    else if( pButton == mpForwardBtn )
    {
        previewForward();
    }
    else if( pButton == mpBackwardBtn )
    {
        previewBackward();
    }
    else if( pButton == maOptionsPage.mpToFileBox )
    {
        mpOKButton->SetText( maOptionsPage.mpToFileBox->IsChecked() ? maPrintToFileText : maPrintText );
        maPController->resetPrinterOptions( maOptionsPage.mpToFileBox->IsChecked() );
        preparePreview( true, true );
    }
    else if( pButton == maOptionsPage.mpPapersizeFromSetup )
    {
        bool bChecked = maOptionsPage.mpPapersizeFromSetup->IsChecked();
        maPController->setPapersizeFromSetup( bChecked );
        maPController->setValue( "PapersizeFromSetup",
                                 makeAny( bChecked ) );
        preparePreview( true, true );
    }
    else if( pButton == maNUpPage.mpBrochureBtn )
    {
        PropertyValue* pVal = getValueForWindow( pButton );
        if( pVal )
        {
            bool bVal = maNUpPage.mpBrochureBtn->IsChecked();
            pVal->Value <<= bVal;

            checkOptionalControlDependencies();

            // update preview and page settings
            preparePreview();
        }
        if( maNUpPage.mpBrochureBtn->IsChecked() )
        {
            maNUpPage.mpNupPagesBox->SelectEntryPos( 0 );
            updateNupFromPages();
            maNUpPage.showAdvancedControls( false );
            maNUpPage.enableNupControls( false );
        }
    }
    else if( pButton == maNUpPage.mpPagesBtn )
    {
        maNUpPage.enableNupControls( true );
        updateNupFromPages();
    }
    else if( pButton == maJobPage.mpCollateBox )
    {
        maPController->setValue( "Collate",
                                 makeAny( isCollate() ) );
        checkControlDependencies();
    }
    else if( pButton == maJobPage.mpReverseOrderBox )
    {
        bool bChecked = maJobPage.mpReverseOrderBox->IsChecked();
        maPController->setReversePrint( bChecked );
        maPController->setValue( "PrintReverse",
                                 makeAny( bChecked ) );
        preparePreview( true, true );
    }
    else if( pButton == maNUpPage.mpBorderCB )
    {
        updateNup();
    }
    else
    {
        if( pButton == maJobPage.mpSetupButton )
        {
            maPController->setupPrinter( this );

            // tdf#63905 don't use cache: page size may change
            preparePreview();
        }
        checkControlDependencies();
    }
}

IMPL_LINK_TYPED( PrintDialog, ModifyHdl, Edit&, rEdit, void )
{
    checkControlDependencies();
    if( &rEdit == maNUpPage.mpNupRowsEdt || &rEdit == maNUpPage.mpNupColEdt ||
        &rEdit == maNUpPage.mpSheetMarginEdt || &rEdit == maNUpPage.mpPageMarginEdt
       )
    {
        updateNupFromPages();
    }
    else if( &rEdit == mpPageEdit )
    {
        mnCurPage = sal_Int32( mpPageEdit->GetValue() - 1 );
        preparePreview( true, true );
    }
    else if( &rEdit == maJobPage.mpCopyCountField )
    {
        maPController->setValue( "CopyCount",
                               makeAny( sal_Int32(maJobPage.mpCopyCountField->GetValue()) ) );
        maPController->setValue( "Collate",
                               makeAny( isCollate() ) );
    }
}

PropertyValue* PrintDialog::getValueForWindow( vcl::Window* i_pWindow ) const
{
    PropertyValue* pVal = nullptr;
    auto it = maControlToPropertyMap.find( i_pWindow );
    if( it != maControlToPropertyMap.end() )
    {
        pVal = maPController->getValue( it->second );
        DBG_ASSERT( pVal, "property value not found" );
    }
    else
    {
        OSL_FAIL( "changed control not in property map" );
    }
    return pVal;
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
                        maNUpPage.mpBrochureBtn->Check();
                    else
                        maNUpPage.mpPagesBtn->Check();
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
                    DBG_ASSERT( pBtn, "unexpected control for property" );
                    if( pBtn )
                        pBtn->Check();
                }
            }
        }
    }
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

IMPL_LINK_TYPED( PrintDialog, UIOption_CheckHdl, CheckBox&, i_rBox, void )
{
    PropertyValue* pVal = getValueForWindow( &i_rBox );
    if( pVal )
    {
        makeEnabled( &i_rBox );

        bool bVal = i_rBox.IsChecked();
        pVal->Value <<= bVal;

        checkOptionalControlDependencies();

        // update preview and page settings
        preparePreview();
    }
}

IMPL_LINK_TYPED( PrintDialog, UIOption_RadioHdl, RadioButton&, i_rBtn, void )
{
    // this handler gets called for all radiobuttons that get unchecked, too
    // however we only want one notificaction for the new value (that is for
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

            checkOptionalControlDependencies();

            // update preview and page settings
            preparePreview();
        }
    }
}

IMPL_LINK_TYPED( PrintDialog, UIOption_SelectHdl, ListBox&, i_rBox, void )
{
    PropertyValue* pVal = getValueForWindow( &i_rBox );
    if( pVal )
    {
        makeEnabled( &i_rBox );

        sal_Int32 nVal( i_rBox.GetSelectEntryPos() );
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
        preparePreview();
    }
}

IMPL_LINK_TYPED( PrintDialog, UIOption_ModifyHdl, Edit&, i_rBox, void )
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
        preparePreview();
    }
}

void PrintDialog::Command( const CommandEvent& rEvt )
{
    if( rEvt.GetCommand() == CommandEventId::Wheel )
    {
        const CommandWheelData* pWheelData = rEvt.GetWheelData();
        if( pWheelData->GetDelta() > 0 )
            previewForward();
        else if( pWheelData->GetDelta() < 0 )
            previewBackward();
    }
}

void PrintDialog::Resize()
{
    // maLayout.setManagedArea( Rectangle( Point( 0, 0 ), GetSizePixel() ) );
    // and do the preview; however the metafile does not need to be gotten anew
    preparePreview( false );

    Dialog::Resize();
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
    OUString aNewText( searchAndReplace( maStr, "%p", 2, OUString::number( mnMax * 10 ) ) );
    aNewText = searchAndReplace( aNewText, "%n", 2, OUString::number( mnMax * 10 ) );
    mpText->SetText( aNewText );
    mpText->set_width_request(mpText->get_preferred_size().Width());

    //Pick a useful max width
    mpProgress->set_width_request(mpProgress->LogicToPixel(Size(100, 0), MapMode(MAP_APPFONT)).Width());

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

IMPL_LINK_TYPED( PrintProgressDialog, ClickHdl, Button*, pButton, void )
{
    if( pButton == mpButton )
        mbCanceled = true;
}

void PrintProgressDialog::setProgress( int i_nCurrent, int i_nMax )
{
    mnCur = i_nCurrent;
    if( i_nMax != -1 )
        mnMax = i_nMax;

    if( mnMax < 1 )
        mnMax = 1;

    mpProgress->SetValue(mnCur*100/mnMax);

    OUString aNewText( searchAndReplace( maStr, "%p", 2, OUString::number( mnCur ) ) );
    aNewText = searchAndReplace( aNewText, "%n", 2, OUString::number( mnMax ) );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
