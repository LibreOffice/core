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
#include "vcl/arrange.hxx"
#include "vcl/configsettings.hxx"
#include "vcl/help.hxx"
#include "vcl/decoview.hxx"
#include "vcl/layout.hxx"
#include "vcl/svapp.hxx"
#include "vcl/unohelp.hxx"

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

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makePrintPreviewWindow(Window *pParent, VclBuilder::stringmap &)
{
    return new PrintDialog::PrintPreviewWindow(pParent);
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeShowNupOrderWindow(Window *pParent, VclBuilder::stringmap &)
{
    return new PrintDialog::ShowNupOrderWindow(pParent);
}

PrintDialog::PrintPreviewWindow::PrintPreviewWindow( Window* i_pParent )
    : Window( i_pParent, 0 )
    , maOrigSize( 10, 10 )
    , maPageVDev( *this )
    , maToolTipString(VclResId( SV_PRINT_PRINTPREVIEW_TXT).toString())
    , mbGreyscale( false )
    , maHorzDim( this, WB_HORZ | WB_CENTER  )
    , maVertDim( this, WB_VERT | WB_VCENTER )
{
    SetPaintTransparent( sal_True );
    SetBackground();
    maPageVDev.SetBackground( Color( COL_WHITE ) );
    maHorzDim.Show();
    maVertDim.Show();

    maHorzDim.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "2.0in" ) ) );
    maVertDim.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "2.0in" ) ) );
}

PrintDialog::PrintPreviewWindow::~PrintPreviewWindow()
{
}

void PrintDialog::PrintPreviewWindow::DataChanged( const DataChangedEvent& i_rDCEvt )
{
    // react on settings changed
    if( i_rDCEvt.GetType() == DATACHANGED_SETTINGS )
    {
        maPageVDev.SetBackground( Color( COL_WHITE ) );
    }
    Window::DataChanged( i_rDCEvt );
}

void PrintDialog::PrintPreviewWindow::Resize()
{
    Size aNewSize( GetSizePixel() );
    long nTextHeight = maHorzDim.GetTextHeight();
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
    Size aPreviewMMSize( maPageVDev.PixelToLogic( aScaledSize, MapMode( MAP_100TH_MM ) ) );
    double fZoom = double(maOrigSize.Height())/double(aPreviewMMSize.Height());
    while( fZoom > 10 )
    {
        aScaledSize.Width() *= 2;
        aScaledSize.Height() *= 2;
        fZoom /= 2.0;
    }

    maPageVDev.SetOutputSizePixel( aScaledSize, sal_False );

    // position dimension lines
    Point aRef( nTextHeight + (aNewSize.Width() - maPreviewSize.Width())/2,
                nTextHeight + (aNewSize.Height() - maPreviewSize.Height())/2 );
    maHorzDim.SetPosSizePixel( Point( aRef.X(), aRef.Y() - nTextHeight ),
                               Size( maPreviewSize.Width(), nTextHeight ) );
    maVertDim.SetPosSizePixel( Point( aRef.X() - nTextHeight, aRef.Y() ),
                               Size( nTextHeight, maPreviewSize.Height() ) );

}

void PrintDialog::PrintPreviewWindow::Paint( const Rectangle& )
{
    long nTextHeight = maHorzDim.GetTextHeight();
    Size aSize( GetSizePixel() );
    Point aOffset( (aSize.Width()  - maPreviewSize.Width()  + nTextHeight) / 2 ,
                   (aSize.Height() - maPreviewSize.Height() + nTextHeight) / 2 );

    if( !maReplacementString.isEmpty() )
    {
        // replacement is active
        Push();
        Font aFont( GetSettings().GetStyleSettings().GetLabelFont() );
        SetZoomedPointFont( aFont );
        Rectangle aTextRect( aOffset + Point( 2, 2 ),
            Size( maPreviewSize.Width() - 4, maPreviewSize.Height() - 4 ) );
        DrawText( aTextRect, maReplacementString,
                  TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER | TEXT_DRAW_WORDBREAK | TEXT_DRAW_MULTILINE
                 );
        Pop();
    }
    else
    {
        GDIMetaFile aMtf( maMtf );

        Size aVDevSize( maPageVDev.GetOutputSizePixel() );
        const Size aLogicSize( maPageVDev.PixelToLogic( aVDevSize, MapMode( MAP_100TH_MM ) ) );
        Size aOrigSize( maOrigSize );
        if( aOrigSize.Width() < 1 )
            aOrigSize.Width() = aLogicSize.Width();
        if( aOrigSize.Height() < 1 )
            aOrigSize.Height() = aLogicSize.Height();
        double fScale = double(aLogicSize.Width())/double(aOrigSize.Width());


        maPageVDev.Erase();
        maPageVDev.Push();
        maPageVDev.SetMapMode( MAP_100TH_MM );
        sal_uLong nOldDrawMode = maPageVDev.GetDrawMode();
        if( mbGreyscale )
            maPageVDev.SetDrawMode( maPageVDev.GetDrawMode() |
                                    ( DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL | DRAWMODE_GRAYTEXT |
                                      DRAWMODE_GRAYBITMAP | DRAWMODE_GRAYGRADIENT ) );
        aMtf.WindStart();
        aMtf.Scale( fScale, fScale );
        aMtf.WindStart();
        aMtf.Play( &maPageVDev, Point( 0, 0 ), aLogicSize );
        maPageVDev.Pop();

        SetMapMode( MAP_PIXEL );
        maPageVDev.SetMapMode( MAP_PIXEL );
        DrawOutDev( aOffset, maPreviewSize, Point( 0, 0 ), aVDevSize, maPageVDev );
        maPageVDev.SetDrawMode( nOldDrawMode );
    }

    Rectangle aFrameRect( aOffset + Point( -1, -1 ),
        Size( maPreviewSize.Width() + 2, maPreviewSize.Height() + 2 ) );
    DecorationView aVw( this );
    aVw.DrawFrame( aFrameRect, FRAME_DRAW_GROUP );
}

void PrintDialog::PrintPreviewWindow::Command( const CommandEvent& rEvt )
{
    if( rEvt.GetCommand() == COMMAND_WHEEL )
    {
        const CommandWheelData* pWheelData = rEvt.GetWheelData();
        PrintDialog* pDlg = dynamic_cast<PrintDialog*>(GetParent());
        if( pDlg )
        {
            if( pWheelData->GetDelta() > 0 )
                pDlg->previewForward();
            else if( pWheelData->GetDelta() < 0 )
                pDlg->previewBackward();
            /*
            else
                huh ?
            */
        }
    }
}

void PrintDialog::PrintPreviewWindow::setPreview( const GDIMetaFile& i_rNewPreview,
                                                  const Size& i_rOrigSize,
                                                  const rtl::OUString& i_rPaperName,
                                                  const rtl::OUString& i_rReplacement,
                                                  sal_Int32 i_nDPIX,
                                                  sal_Int32 i_nDPIY,
                                                  bool i_bGreyscale
                                                 )
{
    rtl::OUStringBuffer aBuf( 256 );
    aBuf.append( maToolTipString );
    SetQuickHelpText( aBuf.makeStringAndClear() );
    maMtf = i_rNewPreview;

    maOrigSize = i_rOrigSize;
    maReplacementString = i_rReplacement;
    mbGreyscale = i_bGreyscale;
    maPageVDev.SetReferenceDevice( i_nDPIX, i_nDPIY );
    maPageVDev.EnableOutput( sal_True );

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
    String aNumText( rLocWrap.getNum( aLogicPaperSize.Width(), nDigits ) );
    aBuf.append( aNumText );
    aBuf.appendAscii( eUnit == MAP_MM ? "mm" : "in" );
    if( !i_rPaperName.isEmpty() )
    {
        aBuf.appendAscii( " (" );
        aBuf.append( i_rPaperName );
        aBuf.append( sal_Unicode(')') );
    }
    maHorzDim.SetText( aBuf.makeStringAndClear() );

    aNumText = rLocWrap.getNum( aLogicPaperSize.Height(), nDigits );
    aBuf.append( aNumText );
    aBuf.appendAscii( eUnit == MAP_MM ? "mm" : "in" );
    maVertDim.SetText( aBuf.makeStringAndClear() );

    Resize();
    Invalidate();
}

PrintDialog::ShowNupOrderWindow::ShowNupOrderWindow( Window* i_pParent )
    : Window( i_pParent, WB_NOBORDER )
    , mnOrderMode( 0 )
    , mnRows( 1 )
    , mnColumns( 1 )
{
    ImplInitSettings();
}

PrintDialog::ShowNupOrderWindow::~ShowNupOrderWindow()
{
}

void PrintDialog::ShowNupOrderWindow::ImplInitSettings()
{
    SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetFieldColor() ) );
}

Size PrintDialog::ShowNupOrderWindow::GetOptimalSize(WindowSizeType) const
{
    return Size(70, 70);
}

void PrintDialog::ShowNupOrderWindow::Paint( const Rectangle& i_rRect )
{
    Window::Paint( i_rRect );
    SetMapMode( MAP_PIXEL );
    SetTextColor( GetSettings().GetStyleSettings().GetFieldTextColor() );

    int nPages = mnRows * mnColumns;
    Font aFont( GetSettings().GetStyleSettings().GetFieldFont() );
    aFont.SetSize( Size( 0, 24 ) );
    SetFont( aFont );
    Size aSampleTextSize( GetTextWidth( rtl::OUString::valueOf( sal_Int32(nPages+1) ) ), GetTextHeight() );

    Size aOutSize( GetOutputSizePixel() );
    Size aSubSize( aOutSize.Width() / mnColumns, aOutSize.Height() / mnRows );
    // calculate font size: shrink the sample text so it fits
    double fX = double(aSubSize.Width())/double(aSampleTextSize.Width());
    double fY = double(aSubSize.Height())/double(aSampleTextSize.Height());
    double fScale = (fX < fY) ? fX : fY;
    long nFontHeight = long(24.0*fScale) - 3;
    if( nFontHeight < 5 )
        nFontHeight = 5;
    aFont.SetSize( Size( 0, nFontHeight ) );
    SetFont( aFont );
    long nTextHeight = GetTextHeight();
    for( int i = 0; i < nPages; i++ )
    {
        rtl::OUString aPageText( rtl::OUString::valueOf( sal_Int32(i+1) ) );
        int nX = 0, nY = 0;
        switch( mnOrderMode )
        {
        case SV_PRINT_PRT_NUP_ORDER_LRTB:
            nX = (i % mnColumns); nY = (i / mnColumns);
            break;
        case SV_PRINT_PRT_NUP_ORDER_TBLR:
            nX = (i / mnRows); nY = (i % mnRows);
            break;
        case SV_PRINT_PRT_NUP_ORDER_RLTB:
            nX = mnColumns - 1 - (i % mnColumns); nY = (i / mnColumns);
            break;
        case SV_PRINT_PRT_NUP_ORDER_TBRL:
            nX = mnColumns - 1 - (i / mnRows); nY = (i % mnRows);
            break;
        }
        Size aTextSize( GetTextWidth( aPageText ), nTextHeight );
        int nDeltaX = (aSubSize.Width() - aTextSize.Width()) / 2;
        int nDeltaY = (aSubSize.Height() - aTextSize.Height()) / 2;
        DrawText( Point( nX * aSubSize.Width() + nDeltaX,
                         nY * aSubSize.Height() + nDeltaY ),
                  aPageText );
    }
    DecorationView aVw( this );
    aVw.DrawFrame( Rectangle( Point( 0, 0), aOutSize ), FRAME_DRAW_GROUP );
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
    mpPagesBtn->Check( sal_True );
    mpBrochureBtn->Show( sal_False );

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
}

void PrintDialog::NUpTabPage::readFromSettings()
{
}

void PrintDialog::NUpTabPage::storeToSettings()
{
}

PrintDialog::JobTabPage::JobTabPage( VclBuilder* pUIBuilder )
    : maCollateImg( VclResId( SV_PRINT_COLLATE_IMG ) )
    , maNoCollateImg( VclResId( SV_PRINT_NOCOLLATE_IMG ) )
    , mnCollateUIMode( 0 )
{
    pUIBuilder->get(mpPrinters, "printers");
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
    rtl::OUString aValue;

    aValue = pItem->getValue( rtl::OUString( "PrintDialog"  ),
                              rtl::OUString( "CollateBox"  ) );
    if( aValue.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("alwaysoff")) )
    {
        mnCollateUIMode = 1;
        mpCollateBox->Check( sal_False );
        mpCollateBox->Enable( sal_False );
    }
    else
    {
        mnCollateUIMode = 0;
        aValue = pItem->getValue( rtl::OUString( "PrintDialog"  ),
                                  rtl::OUString( "Collate"  ) );
        mpCollateBox->Check( aValue.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("true")) );
    }
}

void PrintDialog::JobTabPage::storeToSettings()
{
    SettingsConfigItem* pItem = SettingsConfigItem::get();
    pItem->setValue( rtl::OUString( "PrintDialog"  ),
                     rtl::OUString( "CopyCount"  ),
                     mpCopyCountField->GetText() );
    pItem->setValue( rtl::OUString( "PrintDialog"  ),
                     rtl::OUString( "Collate"  ),
                     mpCollateBox->IsChecked() ? rtl::OUString("true") :
                                                 rtl::OUString("false") );
}

PrintDialog::OutputOptPage::OutputOptPage( VclBuilder *pUIBuilder )
{
    pUIBuilder->get(mpToFileBox, "printtofile");
    pUIBuilder->get(mpCollateSingleJobsBox, "singleprintjob");
}

void PrintDialog::OutputOptPage::readFromSettings()
{
    SettingsConfigItem* pItem = SettingsConfigItem::get();
    rtl::OUString aValue;
    aValue = pItem->getValue( rtl::OUString( "PrintDialog"  ),
                              rtl::OUString( "CollateSingleJobs"  ) );
    if ( aValue.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("true")) )
    {
        mpCollateSingleJobsBox->Check( sal_True );
    }
    else
    {
        mpCollateSingleJobsBox->Check( sal_False );
    }
}

void PrintDialog::OutputOptPage::storeToSettings()
{
    SettingsConfigItem* pItem = SettingsConfigItem::get();
    pItem->setValue( rtl::OUString( "PrintDialog"  ),
                     rtl::OUString( "ToFile"  ),
                     mpToFileBox->IsChecked() ? rtl::OUString("true")
                                             : rtl::OUString("false") );
    pItem->setValue( rtl::OUString( "PrintDialog"  ),
                     rtl::OUString( "CollateSingleJobs"  ),
                     mpCollateSingleJobsBox->IsChecked() ? rtl::OUString("true") :
                                                rtl::OUString("false") );
}

PrintDialog::PrintDialog( Window* i_pParent, const boost::shared_ptr<PrinterController>& i_rController )
    : ModalDialog(i_pParent, "PrintDialog", "vcl/ui/printdialog.ui")
    , mpCustomOptionsUIBuilder(NULL)
    , maPController( i_rController )
    , maNUpPage(m_pUIBuilder)
    , maJobPage(m_pUIBuilder)
    , maOptionsPage(m_pUIBuilder)
    , maNoPageStr( VclResId( SV_PRINT_NOPAGES ).toString() )
    , mnCurPage( 0 )
    , mnCachedPages( 0 )
    , maPrintToFileText( VclResId( SV_PRINT_TOFILE_TXT ).toString() )
    , maDefPrtText( VclResId( SV_PRINT_DEFPRT_TXT ).toString() )
    , mbShowLayoutPage( sal_True )
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

    // set symbols on forward and backward button
    mpBackwardBtn->SetSymbol( SYMBOL_PREV );
    mpForwardBtn->SetSymbol( SYMBOL_NEXT );
    mpBackwardBtn->ImplSetSmallSymbol( sal_True );
    mpForwardBtn->ImplSetSmallSymbol( sal_True );

    maPageStr = mpNumPagesText->GetText();

    // init reverse print
    maJobPage.mpReverseOrderBox->Check( maPController->getReversePrint() );

    // fill printer listbox
    const std::vector< rtl::OUString >& rQueues( Printer::GetPrinterQueues() );
    for( std::vector< rtl::OUString >::const_iterator it = rQueues.begin();
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
        String aValue( pItem->getValue( rtl::OUString( "PrintDialog"  ),
                                        rtl::OUString( "LastPrinter"  ) ) );
        if( maJobPage.mpPrinters->GetEntryPos( aValue ) != LISTBOX_ENTRY_NOTFOUND )
        {
            maJobPage.mpPrinters->SelectEntry( aValue );
            maPController->setPrinter( boost::shared_ptr<Printer>( new Printer( aValue ) ) );
        }
        else
        {
            // fall back to default printer
            maJobPage.mpPrinters->SelectEntry( Printer::GetDefaultPrinterName() );
            maPController->setPrinter( boost::shared_ptr<Printer>( new Printer( Printer::GetDefaultPrinterName() ) ) );
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

    maJobPage.mpCollateBox->SetToggleHdl( LINK( this, PrintDialog, ClickHdl ) );
    maJobPage.mpSetupButton->SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    maNUpPage.mpBorderCB->SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    maOptionsPage.mpToFileBox->SetToggleHdl( LINK( this, PrintDialog, ClickHdl ) );
    maJobPage.mpReverseOrderBox->SetToggleHdl( LINK( this, PrintDialog, ClickHdl ) );
    maOptionsPage.mpCollateSingleJobsBox->SetToggleHdl( LINK( this, PrintDialog, ClickHdl ) );
    maNUpPage.mpPagesBtn->SetToggleHdl( LINK( this, PrintDialog, ClickHdl ) );
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

    // set change handler for UI options
    maPController->setOptionChangeHdl( LINK( this, PrintDialog, UIOptionsChanged ) );

    // remove layout page if unwanted
    if (!mbShowLayoutPage)
        mpTabCtrl->RemovePage(mpTabCtrl->GetPageId(2));

    // restore settings from last run
    readFromSettings();

    // setup dependencies
    checkControlDependencies();

    if ( maPController->getBoolProperty( rtl::OUString( "HideHelpButton" ), sal_False ) )
        mpHelpButton->Hide();
    // set initial focus to "Number of copies"
    maJobPage.mpCopyCountField->GrabFocus();
    maJobPage.mpCopyCountField->SetSelection( Selection(0, 0xFFFF) );

    updateNupFromPages();
}

PrintDialog::~PrintDialog()
{
    delete mpCustomOptionsUIBuilder;
}

void PrintDialog::readFromSettings()
{
    maJobPage.readFromSettings();
    maNUpPage.readFromSettings();
    maOptionsPage.readFromSettings();

    // read last selected tab page; if it exists, actiavte it
    SettingsConfigItem* pItem = SettingsConfigItem::get();
    rtl::OUString aValue = pItem->getValue( rtl::OUString( "PrintDialog"  ),
                                            rtl::OUString( "LastPage"  ) );
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
    rtl::OUString aWinState( pItem->getValue( rtl::OUString( "PrintDialog"  ),
                                              rtl::OUString( "WindowState"  ) ) );
    if( !aWinState.isEmpty() )
        SetWindowState( rtl::OUStringToOString( aWinState, RTL_TEXTENCODING_UTF8 ) );

    if( maOptionsPage.mpToFileBox->IsChecked() )
    {
        maPController->resetPrinterOptions( true );
        preparePreview( true, true );
    }
}

void PrintDialog::storeToSettings()
{
    maJobPage.storeToSettings();
    maNUpPage.storeToSettings();
    maOptionsPage.storeToSettings();

    // store last selected printer
    SettingsConfigItem* pItem = SettingsConfigItem::get();
    pItem->setValue( rtl::OUString( "PrintDialog"  ),
                     rtl::OUString( "LastPrinter"  ),
                     maJobPage.mpPrinters->GetSelectEntry() );

    pItem->setValue( rtl::OUString( "PrintDialog"  ),
                     rtl::OUString( "LastPage"  ),
                     mpTabCtrl->GetPageText( mpTabCtrl->GetCurPageId() ) );
    pItem->setValue( rtl::OUString( "PrintDialog"  ),
                     rtl::OUString( "WindowState"  ),
                     rtl::OStringToOUString( GetWindowState(), RTL_TEXTENCODING_UTF8 )
                     );
    pItem->Commit();
}

bool PrintDialog::isPrintToFile()
{
    return maOptionsPage.mpToFileBox->IsChecked();
}

bool PrintDialog::isCollate()
{
    return maJobPage.mpCopyCountField->GetValue() > 1 ? maJobPage.mpCollateBox->IsChecked() : sal_False;
}

bool PrintDialog::isSingleJobs()
{
    return maOptionsPage.mpCollateSingleJobsBox->IsChecked();
}

void setHelpId( Window* i_pWindow, const Sequence< rtl::OUString >& i_rHelpIds, sal_Int32 i_nIndex )
{
    if( i_nIndex >= 0 && i_nIndex < i_rHelpIds.getLength() )
        i_pWindow->SetHelpId( rtl::OUStringToOString( i_rHelpIds.getConstArray()[i_nIndex], RTL_TEXTENCODING_UTF8 ) );
}

static void setHelpText( Window* i_pWindow, const Sequence< rtl::OUString >& i_rHelpTexts, sal_Int32 i_nIndex )
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
            rtl::OUString sOptionsUIFile;
            rOptions[i].Value >>= sOptionsUIFile;

            Window *pCustom = get<Window>("customcontents");

            delete mpCustomOptionsUIBuilder;
            mpCustomOptionsUIBuilder = new VclBuilder(pCustom, getUIRootDir(), sOptionsUIFile);
            Window *pWindow = mpCustomOptionsUIBuilder->get_widget_root();
            pWindow->Show();
            continue;
        }

        Sequence< beans::PropertyValue > aOptProp;
        rOptions[i].Value >>= aOptProp;

        // extract ui element
        rtl::OUString aCtrlType;
        rtl::OString aID;
        rtl::OUString aText;
        rtl::OUString aPropertyName;
        Sequence< rtl::OUString > aChoices;
        Sequence< sal_Bool > aChoicesDisabled;
        Sequence< rtl::OUString > aHelpTexts;
        Sequence< rtl::OUString > aIDs;
        Sequence< rtl::OUString > aHelpIds;
        sal_Int64 nMinValue = 0, nMaxValue = 0;
        rtl::OUString aGroupingHint;
        rtl::OUString aDependsOnName;
        sal_Int32 nDependsOnValue = 0;
        sal_Bool bUseDependencyRow = sal_False;

        for( int n = 0; n < aOptProp.getLength(); n++ )
        {
            const beans::PropertyValue& rEntry( aOptProp[ n ] );
            if ( rEntry.Name == "ID" )
            {
                rEntry.Value >>= aIDs;
                aID = rtl::OUStringToOString(aIDs[0], RTL_TEXTENCODING_UTF8);
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
                sal_Bool bValue = sal_True;
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
                    rtl::OUString aHelpText;
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
                    rtl::OUString aHelpId;
                    if( (rEntry.Value >>= aHelpId) )
                    {
                        aHelpIds.realloc( 1 );
                        *aHelpIds.getArray() = aHelpId;
                    }
                }
            }
            else if ( rEntry.Name == "HintNoLayoutPage" )
            {
                sal_Bool bNoLayoutPage = sal_False;
                rEntry.Value >>= bNoLayoutPage;
                mbShowLayoutPage = ! bNoLayoutPage;
            }
        }

        if (aCtrlType == "Group" && !aID.isEmpty())
        {
            TabPage *pPage = get<TabPage>(aID);
            if (!pPage && mpCustomOptionsUIBuilder)
                pPage = mpCustomOptionsUIBuilder->get<TabPage>(aID);
            sal_uInt16 nPageId = mpTabCtrl->GetPageId(*pPage);

            mpTabCtrl->SetPageText(nPageId, aText);

            // set help id
            if (aHelpIds.getLength() > 0)
                mpTabCtrl->SetHelpId(nPageId, rtl::OUStringToOString(aHelpIds.getConstArray()[0], RTL_TEXTENCODING_UTF8));

            // set help text
            if (aHelpTexts.getLength() > 0)
                mpTabCtrl->SetHelpText(nPageId, aHelpTexts.getConstArray()[0]);

            pPage->Show();
        }
        else if (aCtrlType == "Subgroup" && !aID.isEmpty())
        {
            Window *pFrame = get<Window>(aID);
            if (!pFrame && mpCustomOptionsUIBuilder)
                pFrame = mpCustomOptionsUIBuilder->get<Window>(aID);

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

            sal_Bool bVal = sal_False;
            PropertyValue* pVal = maPController->getValue( aPropertyName );
            if( pVal )
                pVal->Value >>= bVal;
            maNUpPage.mpBrochureBtn->Check( bVal );
            maNUpPage.mpBrochureBtn->Enable( maPController->isUIOptionEnabled( aPropertyName ) && pVal != NULL );
            maNUpPage.mpBrochureBtn->SetToggleHdl( LINK( this, PrintDialog, ClickHdl ) );

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

            pNewBox->SetText( aText );
            pNewBox->Show();

            sal_Bool bVal = sal_False;
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
                aID = rtl::OUStringToOString(aIDs[m], RTL_TEXTENCODING_UTF8);
                RadioButton* pBtn = get<RadioButton>(aID);
                if (!pBtn && mpCustomOptionsUIBuilder)
                    pBtn = mpCustomOptionsUIBuilder->get<RadioButton>(aID);

                pBtn->SetText( aChoices[m] );
                pBtn->Check( m == nSelectVal );
                pBtn->SetToggleHdl( LINK( this, PrintDialog, UIOption_RadioHdl ) );
                if( aChoicesDisabled.getLength() > m && aChoicesDisabled[m] == sal_True )
                    pBtn->Enable( sal_False );
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

            rtl::OUString aCurVal;
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
            rtl::OStringBuffer sMessage;
            sMessage.append(RTL_CONSTASCII_STRINGPARAM("Unsupported UI option: \""));
            sMessage.append(rtl::OUStringToOString(aCtrlType, RTL_TEXTENCODING_UTF8));
            sMessage.append('"');
            OSL_FAIL( sMessage.getStr() );
        }
    }

    // #i106506# if no brochure button, then the singular Pages radio button
    // makes no sense, so replace it by a FixedText label
    if (!maNUpPage.mpBrochureBtn->IsVisible() && maNUpPage.mpPagesBtn->IsVisible())
    {
        maNUpPage.mpPagesBoxTitleTxt->SetText( maNUpPage.mpPagesBtn->GetText() );
        maNUpPage.mpPagesBoxTitleTxt->Show( sal_True );
        maNUpPage.mpPagesBtn->Show( sal_False );
    }

    // update enable states
    checkOptionalControlDependencies();

    Window *pPageRange = get<Window>("pagerange");

    // print range not shown (currently math only) -> hide spacer line and reverse order
    if (!pPageRange || !pPageRange->IsVisible())
    {
        maJobPage.mpCopySpacer->Show( sal_False );
        maJobPage.mpReverseOrderBox->Show( sal_False );
    }

    if (!mpCustomOptionsUIBuilder)
        mpTabCtrl->RemovePage(mpTabCtrl->GetPageId(1));
}

void PrintDialog::DataChanged( const DataChangedEvent& i_rDCEvt )
{
    // react on settings changed
    if( i_rDCEvt.GetType() == DATACHANGED_SETTINGS )
        checkControlDependencies();
    ModalDialog::DataChanged( i_rDCEvt );
}

void PrintDialog::checkControlDependencies()
{
    if( maJobPage.mpCopyCountField->GetValue() > 1 )
        maJobPage.mpCollateBox->Enable( maJobPage.mnCollateUIMode == 0 );
    else
        maJobPage.mpCollateBox->Enable( sal_False );

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
    for( std::map< Window*, rtl::OUString >::iterator it = maControlToPropertyMap.begin();
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
                rtl::OUString aDep( maPController->getDependency( it->second ) );
                // if the dependency is at least enabled, then enable this control anyway
                if( !aDep.isEmpty() && maPController->isUIOptionEnabled( aDep ) )
                    bShouldbeEnabled = true;
            }
        }

        if( bShouldbeEnabled && dynamic_cast<RadioButton*>(it->first) )
        {
            std::map< Window*, sal_Int32 >::const_iterator r_it = maControlToNumValMap.find( it->first );
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

static rtl::OUString searchAndReplace( const rtl::OUString& i_rOrig, const char* i_pRepl, sal_Int32 i_nReplLen, const rtl::OUString& i_rRepl )
{
    sal_Int32 nPos = i_rOrig.indexOfAsciiL( i_pRepl, i_nReplLen );
    if( nPos != -1 )
    {
        rtl::OUStringBuffer aBuf( i_rOrig.getLength() );
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
    const rtl::OUString aDefPrt( Printer::GetDefaultPrinterName() );
    const QueueInfo* pInfo = Printer::GetQueueInfo( maJobPage.mpPrinters->GetSelectEntry(), true );
    if( pInfo )
    {
        maJobPage.mpLocationTxt->SetText( pInfo->GetLocation() );
        maJobPage.mpCommentTxt->SetText( pInfo->GetComment() );
        // FIXME: status text
        rtl::OUString aStatus;
        if( aDefPrt == pInfo->GetPrinterName() )
            aStatus = maDefPrtText;
        maJobPage.mpStatusTxt->SetText( aStatus );
    }
    else
    {
        maJobPage.mpLocationTxt->SetText( String() );
        maJobPage.mpCommentTxt->SetText( String() );
        maJobPage.mpStatusTxt->SetText( String() );
    }
}

void PrintDialog::setPreviewText( sal_Int32 )
{
    rtl::OUString aNewText( searchAndReplace( maPageStr, "%n", 2, rtl::OUString::valueOf( mnCachedPages )  ) );
    mpNumPagesText->SetText( aNewText );

    // if layout is already established the refresh layout of
    // preview controls since text length may have changes
    if( mxPreviewCtrls.get() )
        mxPreviewCtrls->setManagedArea( mxPreviewCtrls->getManagedArea() );
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
        boost::shared_ptr<Printer> aPrt( maPController->getPrinter() );
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
                                    nPages > 0 ? rtl::OUString() : maNoPageStr,
                                    aPrt->ImplGetDPIX(), aPrt->ImplGetDPIY(),
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
    long nPages = long(maNUpPage.mpNupPagesBox->GetEntryData(maNUpPage.mpNupPagesBox->GetSelectEntryPos()));
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

    int nOrderMode = int(sal_IntPtr(maNUpPage.mpNupOrderBox->GetEntryData(
                           maNUpPage.mpNupOrderBox->GetSelectEntryPos() )));
    if( nOrderMode == SV_PRINT_PRT_NUP_ORDER_LRTB )
        aMPS.nOrder = PrinterController::LRTB;
    else if( nOrderMode == SV_PRINT_PRT_NUP_ORDER_TBLR )
        aMPS.nOrder = PrinterController::TBLR;
    else if( nOrderMode == SV_PRINT_PRT_NUP_ORDER_RLTB )
        aMPS.nOrder = PrinterController::RLTB;
    else if( nOrderMode == SV_PRINT_PRT_NUP_ORDER_TBRL )
        aMPS.nOrder = PrinterController::TBRL;

    int nOrientationMode = int(sal_IntPtr(maNUpPage.mpNupOrientationBox->GetEntryData(
                                 maNUpPage.mpNupOrientationBox->GetSelectEntryPos() )));
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

IMPL_LINK( PrintDialog, SelectHdl, ListBox*, pBox )
{
    if(  pBox == maJobPage.mpPrinters )
    {
        String aNewPrinter( pBox->GetSelectEntry() );
        // set new printer
        maPController->setPrinter( boost::shared_ptr<Printer>( new Printer( aNewPrinter ) ) );
        maPController->resetPrinterOptions( maOptionsPage.mpToFileBox->IsChecked() );
        // update text fields
        updatePrinterText();
        preparePreview( true, false );
    }
    else if( pBox == maNUpPage.mpNupOrientationBox || pBox == maNUpPage.mpNupOrderBox )
    {
        updateNup();
    }
    else if( pBox == maNUpPage.mpNupPagesBox )
    {
        if( !maNUpPage.mpPagesBtn->IsChecked() )
            maNUpPage.mpPagesBtn->Check();
        updateNupFromPages();
    }

    return 0;
}

IMPL_LINK( PrintDialog, ClickHdl, Button*, pButton )
{
    if( pButton == mpOKButton || pButton == mpCancelButton )
    {
        storeToSettings();
        EndDialog( pButton == mpOKButton );
    }
    else if( pButton == mpHelpButton )
    {
        // start help system
        Help* pHelp = Application::GetHelp();
        if( pHelp )
        {
            pHelp->Start( rtl::OUString("vcl/ui/printdialog"), mpOKButton );
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
    else if( pButton == maNUpPage.mpBrochureBtn )
    {
        PropertyValue* pVal = getValueForWindow( pButton );
        if( pVal )
        {
            sal_Bool bVal = maNUpPage.mpBrochureBtn->IsChecked();
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
        maPController->setValue( rtl::OUString( "Collate"  ),
                                 makeAny( sal_Bool(isCollate()) ) );
        checkControlDependencies();
    }
    else if( pButton == maJobPage.mpReverseOrderBox )
    {
        sal_Bool bChecked = maJobPage.mpReverseOrderBox->IsChecked();
        maPController->setReversePrint( bChecked );
        maPController->setValue( rtl::OUString( "PrintReverse"  ),
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
            preparePreview( true, true );
        }
        checkControlDependencies();
    }
    return 0;
}

IMPL_LINK( PrintDialog, ModifyHdl, Edit*, pEdit )
{
    checkControlDependencies();
    if( pEdit == maNUpPage.mpNupRowsEdt || pEdit == maNUpPage.mpNupColEdt ||
        pEdit == maNUpPage.mpSheetMarginEdt || pEdit == maNUpPage.mpPageMarginEdt
       )
    {
        updateNupFromPages();
    }
    else if( pEdit == mpPageEdit )
    {
        mnCurPage = sal_Int32( mpPageEdit->GetValue() - 1 );
        preparePreview( true, true );
    }
    else if( pEdit == maJobPage.mpCopyCountField )
    {
        maPController->setValue( rtl::OUString( "CopyCount"  ),
                               makeAny( sal_Int32(maJobPage.mpCopyCountField->GetValue()) ) );
        maPController->setValue( rtl::OUString( "Collate"  ),
                               makeAny( sal_Bool(isCollate()) ) );
    }
    return 0;
}

IMPL_LINK_NOARG(PrintDialog, UIOptionsChanged)
{
    checkOptionalControlDependencies();
    return 0;
}

PropertyValue* PrintDialog::getValueForWindow( Window* i_pWindow ) const
{
    PropertyValue* pVal = NULL;
    std::map< Window*, rtl::OUString >::const_iterator it = maControlToPropertyMap.find( i_pWindow );
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

void PrintDialog::updateWindowFromProperty( const rtl::OUString& i_rProperty )
{
    beans::PropertyValue* pValue = maPController->getValue( i_rProperty );
    std::map< rtl::OUString, std::vector< Window* > >::const_iterator it = maPropertyToWindowMap.find( i_rProperty );
    if( pValue && it != maPropertyToWindowMap.end() )
    {
        const std::vector< Window* >& rWindows( it->second );
        if( ! rWindows.empty() )
        {
            sal_Bool bVal = sal_False;
            sal_Int32 nVal = -1;
            if( pValue->Value >>= bVal )
            {
                // we should have a CheckBox for this one
                CheckBox* pBox = dynamic_cast< CheckBox* >( rWindows.front() );
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
                    DBG_ASSERT( 0, "missing a checkbox" );
                }
            }
            else if( pValue->Value >>= nVal )
            {
                // this could be a ListBox or a RadioButtonGroup
                ListBox* pList = dynamic_cast< ListBox* >( rWindows.front() );
                if( pList )
                {
                    pList->SelectEntryPos( static_cast< sal_uInt16 >(nVal) );
                }
                else if( nVal >= 0 && nVal < sal_Int32(rWindows.size() ) )
                {
                    RadioButton* pBtn = dynamic_cast< RadioButton* >( rWindows[nVal] );
                    DBG_ASSERT( pBtn, "unexpected control for property" );
                    if( pBtn )
                        pBtn->Check();
                }
            }
        }
    }
}

void PrintDialog::makeEnabled( Window* i_pWindow )
{
    std::map< Window*, rtl::OUString >::const_iterator it = maControlToPropertyMap.find( i_pWindow );
    if( it != maControlToPropertyMap.end() )
    {
        rtl::OUString aDependency( maPController->makeEnabled( it->second ) );
        if( !aDependency.isEmpty() )
            updateWindowFromProperty( aDependency );
    }
}

IMPL_LINK( PrintDialog, UIOption_CheckHdl, CheckBox*, i_pBox )
{
    PropertyValue* pVal = getValueForWindow( i_pBox );
    if( pVal )
    {
        makeEnabled( i_pBox );

        sal_Bool bVal = i_pBox->IsChecked();
        pVal->Value <<= bVal;

        checkOptionalControlDependencies();

        // update preview and page settings
        preparePreview();
    }
    return 0;
}

IMPL_LINK( PrintDialog, UIOption_RadioHdl, RadioButton*, i_pBtn )
{
    // this handler gets called for all radiobuttons that get unchecked, too
    // however we only want one notificaction for the new value (that is for
    // the button that gets checked)
    if( i_pBtn->IsChecked() )
    {
        PropertyValue* pVal = getValueForWindow( i_pBtn );
        std::map< Window*, sal_Int32 >::const_iterator it = maControlToNumValMap.find( i_pBtn );
        if( pVal && it != maControlToNumValMap.end() )
        {
            makeEnabled( i_pBtn );

            sal_Int32 nVal = it->second;
            pVal->Value <<= nVal;

            checkOptionalControlDependencies();

            // update preview and page settings
            preparePreview();
        }
    }
    return 0;
}

IMPL_LINK( PrintDialog, UIOption_SelectHdl, ListBox*, i_pBox )
{
    PropertyValue* pVal = getValueForWindow( i_pBox );
    if( pVal )
    {
        makeEnabled( i_pBox );

        sal_Int32 nVal( i_pBox->GetSelectEntryPos() );
        pVal->Value <<= nVal;

        checkOptionalControlDependencies();

        // update preview and page settings
        preparePreview();
    }
    return 0;
}

IMPL_LINK( PrintDialog, UIOption_ModifyHdl, Edit*, i_pBox )
{
    PropertyValue* pVal = getValueForWindow( i_pBox );
    if( pVal )
    {
        makeEnabled( i_pBox );

        NumericField* pNum = dynamic_cast<NumericField*>(i_pBox);
        MetricField* pMetric = dynamic_cast<MetricField*>(i_pBox);
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
            rtl::OUString aVal( i_pBox->GetText() );
            pVal->Value <<= aVal;
        }

        checkOptionalControlDependencies();

        // update preview and page settings
        preparePreview();
    }
    return 0;
}

void PrintDialog::Command( const CommandEvent& rEvt )
{
    if( rEvt.GetCommand() == COMMAND_WHEEL )
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

// -----------------------------------------------------------------------------
//
// PrintProgressDialog
//
// -----------------------------------------------------------------------------

PrintProgressDialog::PrintProgressDialog( Window* i_pParent, int i_nMax ) :
    ModelessDialog( i_pParent, VclResId( SV_DLG_PRINT_PROGRESS ) ),
    maText( this, VclResId( SV_PRINT_PROGRESS_TEXT ) ),
    maButton( this, VclResId( SV_PRINT_PROGRESS_CANCEL ) ),
    mbCanceled( false ),
    mnCur( 0 ),
    mnMax( i_nMax ),
    mnProgressHeight( 15 ),
    mbNativeProgress( false )
{
    FreeResource();

    if( mnMax < 1 )
        mnMax = 1;

    maStr = maText.GetText();

    maButton.SetClickHdl( LINK( this, PrintProgressDialog, ClickHdl ) );

}

PrintProgressDialog::~PrintProgressDialog()
{
}

IMPL_LINK( PrintProgressDialog, ClickHdl, Button*, pButton )
{
    if( pButton == &maButton )
        mbCanceled = true;

    return 0;
}

void PrintProgressDialog::implCalcProgressRect()
{
    if( IsNativeControlSupported( CTRL_PROGRESS, PART_ENTIRE_CONTROL ) )
    {
        ImplControlValue aValue;
        Rectangle aControlRegion( Point(), Size( 100, mnProgressHeight ) );
        Rectangle aNativeControlRegion, aNativeContentRegion;
        if( GetNativeControlRegion( CTRL_PROGRESS, PART_ENTIRE_CONTROL, aControlRegion,
                                    CTRL_STATE_ENABLED, aValue, rtl::OUString(),
                                    aNativeControlRegion, aNativeContentRegion ) )
        {
            mnProgressHeight = aNativeControlRegion.GetHeight();
        }
        mbNativeProgress = true;
    }
    maProgressRect = Rectangle( Point( 10, maText.GetPosPixel().Y() + maText.GetSizePixel().Height() + 8 ),
                                Size( GetSizePixel().Width() - 20, mnProgressHeight ) );
}

void PrintProgressDialog::setProgress( int i_nCurrent, int i_nMax )
{
    if( maProgressRect.IsEmpty() )
        implCalcProgressRect();

    mnCur = i_nCurrent;
    if( i_nMax != -1 )
        mnMax = i_nMax;

    if( mnMax < 1 )
        mnMax = 1;

    rtl::OUString aNewText( searchAndReplace( maStr, "%p", 2, rtl::OUString::valueOf( mnCur ) ) );
    aNewText = searchAndReplace( aNewText, "%n", 2, rtl::OUString::valueOf( mnMax ) );
    maText.SetText( aNewText );

    // update progress
    Invalidate( maProgressRect, INVALIDATE_UPDATE );
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

void PrintProgressDialog::Paint( const Rectangle& )
{
    if( maProgressRect.IsEmpty() )
        implCalcProgressRect();

    Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    Color aPrgsColor = rStyleSettings.GetHighlightColor();
    if ( aPrgsColor == rStyleSettings.GetFaceColor() )
        aPrgsColor = rStyleSettings.GetDarkShadowColor();
    SetLineColor();
    SetFillColor( aPrgsColor );

    const long nOffset = 3;
    const long nWidth = 3*mnProgressHeight/2;
    const long nFullWidth = nWidth + nOffset;
    const long nMaxCount = maProgressRect.GetWidth() / nFullWidth;
    DrawProgress( this, maProgressRect.TopLeft(),
                        nOffset,
                        nWidth,
                        mnProgressHeight,
                        static_cast<sal_uInt16>(0),
                        static_cast<sal_uInt16>(10000*mnCur/mnMax),
                        static_cast<sal_uInt16>(10000/nMaxCount),
                        maProgressRect
                        );
    Pop();

    if( ! mbNativeProgress )
    {
        DecorationView aDecoView( this );
        Rectangle aFrameRect( maProgressRect );
        aFrameRect.Left() -= nOffset;
        aFrameRect.Right() += nOffset;
        aFrameRect.Top() -= nOffset;
        aFrameRect.Bottom() += nOffset;
        aDecoView.DrawFrame( aFrameRect );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
