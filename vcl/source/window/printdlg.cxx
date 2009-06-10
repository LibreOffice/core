/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: printdlg.cxx,v $
 * $Revision: 1.1.2.7 $
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

#include "precompiled_vcl.hxx"

#include "vcl/print.hxx"
#include "vcl/prndlg.hxx"
#include "vcl/dialog.hxx"
#include "vcl/button.hxx"
#include "vcl/svdata.hxx"
#include "vcl/svids.hrc"
#include "vcl/wall.hxx"
#include "vcl/jobset.h"
#include "vcl/status.hxx"
#include "vcl/decoview.hxx"
#include "vcl/arrange.hxx"
#include "vcl/configsettings.hxx"
#include "vcl/help.hxx"
#include "vcl/decoview.hxx"

#include "unotools/localedatawrapper.hxx"

#include "rtl/ustrbuf.hxx"

#include "com/sun/star/awt/Size.hpp"

using namespace vcl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

#define HELPID_PREFIX ".HelpId:vcl:PrintDialog"
#define SMHID2( a, b ) SetSmartHelpId( SmartId( String( RTL_CONSTASCII_USTRINGPARAM( HELPID_PREFIX ":" a ":" b ) ) ) )
#define SMHID1( a ) SetSmartHelpId( SmartId( String( RTL_CONSTASCII_USTRINGPARAM( HELPID_PREFIX  ":" a ) ) ) )

void PrintDialog::PrinterListBox::RequestHelp( const HelpEvent& i_rHEvt )
{
    if( i_rHEvt.GetMode() & HELPMODE_QUICK )
        Help::ShowBalloon( this, i_rHEvt.GetMousePosPixel(), GetHelpText() );
    else
        ListBox::RequestHelp( i_rHEvt );
}

PrintDialog::PrintPreviewWindow::PrintPreviewWindow( Window* i_pParent, const ResId& i_rId )
    : Window( i_pParent, i_rId ),
      mfScaleX( 1 ),
      mfScaleY( 1 )
{
}

PrintDialog::PrintPreviewWindow::~PrintPreviewWindow()
{
}

void PrintDialog::PrintPreviewWindow::Paint( const Rectangle& i_rRect )
{
    Window::Paint( i_rRect );

    GDIMetaFile aMtf( maMtf );

    SetFillColor( Color( COL_WHITE ) );
    SetLineColor();
    DrawRect( Rectangle( Point( 0, 0 ), GetSizePixel() ));
    Push();
    SetMapMode( MAP_100TH_MM );
    aMtf.WindStart();
    aMtf.Scale( mfScaleX, mfScaleY );
    aMtf.WindStart();
    aMtf.Play( this, Point( 0, 0 ), PixelToLogic( GetSizePixel() ) );
    Pop();
}

void PrintDialog::PrintPreviewWindow::setPreview( const GDIMetaFile& i_rNewPreview )
{
    maMtf = i_rNewPreview;
    Invalidate();
}

void PrintDialog::PrintPreviewWindow::setScale( double fScaleX, double fScaleY )
{
    mfScaleX = fScaleX;
    mfScaleY = fScaleY;
    Invalidate();
}

PrintDialog::NUpTabPage::NUpTabPage( Window* i_pParent, const ResId& rResId )
    : TabPage( i_pParent, rResId )
    , maNupLine( this, VclResId( SV_PRINT_PRT_NUP ) )
    , maNupRowsTxt( this, VclResId( SV_PRINT_PRT_NUP_ROWS_TXT ) )
    , maNupRowsEdt( this, VclResId( SV_PRINT_PRT_NUP_ROWS_EDT ) )
    , maNupColTxt( this, VclResId( SV_PRINT_PRT_NUP_COLUMNS_TXT ) )
    , maNupColEdt( this, VclResId( SV_PRINT_PRT_NUP_COLUMNS_EDT ) )
    , maNupRepTxt( this, VclResId( SV_PRINT_PRT_NUP_PAGEREPEAT_TXT ) )
    , maNupRepEdt( this, VclResId( SV_PRINT_PRT_NUP_PAGEREPEAT_EDT ) )
    , maBorderCB( this, VclResId( SV_PRINT_PRT_NUP_BORDER_CB ) )
    , maNupPortrait( this, VclResId( SV_PRINT_PRT_NUP_PORTRAIT ) )
    , maNupLandscape( this, VclResId( SV_PRINT_PRT_NUP_LANDSCAPE ) )
    , maMargins( this, VclResId( SV_PRINT_PRT_NUP_MARGINS_FL ) )
    , maLeftMarginTxt( this, VclResId( SV_PRINT_PRT_NUP_MARGINS_LEFT_TXT ) )
    , maLeftMarginEdt( this, VclResId( SV_PRINT_PRT_NUP_MARGINS_LEFT_EDT) )
    , maRightMarginTxt( this, VclResId( SV_PRINT_PRT_NUP_MARGINS_RIGHT_TXT ) )
    , maRightMarginEdt( this, VclResId( SV_PRINT_PRT_NUP_MARGINS_RIGHT_EDT ) )
    , maTopMarginTxt( this, VclResId( SV_PRINT_PRT_NUP_MARGINS_TOP_TXT ) )
    , maTopMarginEdt( this, VclResId( SV_PRINT_PRT_NUP_MARGINS_TOP_EDT ) )
    , maBottomMarginTxt( this, VclResId( SV_PRINT_PRT_NUP_MARGINS_BOTTOM_TXT ) )
    , maBottomMarginEdt( this, VclResId( SV_PRINT_PRT_NUP_MARGINS_BOTTOM_EDT ) )
    , maHSpaceTxt( this, VclResId( SV_PRINT_PRT_NUP_MARGINS_HSPACE_TXT ) )
    , maHSpaceEdt( this, VclResId( SV_PRINT_PRT_NUP_MARGINS_HSPACE_EDT ) )
    , maVSpaceTxt( this, VclResId( SV_PRINT_PRT_NUP_MARGINS_VSPACE_TXT ) )
    , maVSpaceEdt( this, VclResId( SV_PRINT_PRT_NUP_MARGINS_VSPACE_EDT ) )
{
    FreeResource();

    // setup field units for metric fields
    const LocaleDataWrapper& rLocWrap( maLeftMarginEdt.GetLocaleDataWrapper() );
    FieldUnit eUnit = FUNIT_MM;
    USHORT nDigits = 0;
    if( rLocWrap.getMeasurementSystemEnum() == MEASURE_US )
    {
        eUnit = FUNIT_INCH;
        nDigits = 2;
    }
    // set units
    maLeftMarginEdt.SetUnit( eUnit );
    maTopMarginEdt.SetUnit( eUnit );
    maRightMarginEdt.SetUnit( eUnit );
    maBottomMarginEdt.SetUnit( eUnit );
    maHSpaceEdt.SetUnit( eUnit );
    maVSpaceEdt.SetUnit( eUnit );

    // set precision
    maLeftMarginEdt.SetDecimalDigits( nDigits );
    maTopMarginEdt.SetDecimalDigits( nDigits );
    maRightMarginEdt.SetDecimalDigits( nDigits );
    maBottomMarginEdt.SetDecimalDigits( nDigits );
    maHSpaceEdt.SetDecimalDigits( nDigits );
    maVSpaceEdt.SetDecimalDigits( nDigits );

    maNupLine.SMHID2( "NUpPage", "NUPline" );
    maNupRowsTxt.SMHID2( "NUpPage", "NUPRowsText" );
    maNupRowsEdt.SMHID2( "NUpPage", "NUPRows" );
    maNupColTxt.SMHID2( "NUpPage", "NUPColumnsText" );
    maNupColEdt.SMHID2( "NUpPage", "NUPColumns" );
    maNupRepTxt.SMHID2( "NUpPage", "NUPRepeatText" );
    maNupRepEdt.SMHID2( "NUpPage", "NUPRepeat" );
    maNupPortrait.SMHID2( "NUpPage", "NUPPortrait" );
    maNupLandscape.SMHID2( "NUpPage", "NUPLandscape" );
    maBorderCB.SMHID2( "NUpPage", "NUPBorder" );
    maMargins.SMHID2( "NUpPage", "NUPMargins" );
    maLeftMarginTxt.SMHID2( "NUpPage", "NUPLeftText" );
    maLeftMarginEdt.SMHID2( "NUpPage", "NUPLeft" );
    maTopMarginTxt.SMHID2( "NUpPage", "NUPTopText" );
    maTopMarginEdt.SMHID2( "NUpPage", "NUPTop" );
    maRightMarginTxt.SMHID2( "NUpPage", "NUPRightText" );
    maRightMarginEdt.SMHID2( "NUpPage", "NUPRight" );
    maBottomMarginTxt.SMHID2( "NUpPage", "NUPBottomText" );
    maBottomMarginEdt.SMHID2( "NUpPage", "NUPBottom" );
    maHSpaceTxt.SMHID2( "NUpPage", "NUPHSpaceText" );
    maHSpaceEdt.SMHID2( "NUpPage", "NUPHSpace" );
    maVSpaceTxt.SMHID2( "NUpPage", "NUPVSpaceText" );
    maVSpaceEdt.SMHID2( "NUpPage", "NUPVSpace" );
}

PrintDialog::NUpTabPage::~NUpTabPage()
{
}

void PrintDialog::NUpTabPage::initFromMultiPageSetup( const vcl::PrinterListener::MultiPageSetup& i_rMPS )
{
    maLeftMarginEdt.SetValue( maLeftMarginEdt.Normalize( i_rMPS.nLeftMargin ), FUNIT_100TH_MM );
    maTopMarginEdt.SetValue( maTopMarginEdt.Normalize( i_rMPS.nTopMargin ), FUNIT_100TH_MM );
    maRightMarginEdt.SetValue( maRightMarginEdt.Normalize( i_rMPS.nRightMargin ), FUNIT_100TH_MM );
    maBottomMarginEdt.SetValue( maBottomMarginEdt.Normalize( i_rMPS.nBottomMargin ), FUNIT_100TH_MM );
    maHSpaceEdt.SetValue( maHSpaceEdt.Normalize( i_rMPS.nHorizontalSpacing ), FUNIT_100TH_MM );
    maVSpaceEdt.SetValue( maVSpaceEdt.Normalize( i_rMPS.nVerticalSpacing ), FUNIT_100TH_MM );
    maBorderCB.Check( i_rMPS.bDrawBorder );
    maNupRowsEdt.SetValue( i_rMPS.nRows );
    maNupColEdt.SetValue( i_rMPS.nColumns );
    maNupRepEdt.SetValue( i_rMPS.nRepeat );
}

void PrintDialog::NUpTabPage::readFromSettings()
{
    #if 0
    SettingsConfigItem* pItem = SettingsConfigItem::get();
    rtl::OUString aValue;

    aValue = pItem->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintDialog_NUpPage" ) ),
                              rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "NUp-Rows" ) ) );
    sal_Int32 nVal = aValue.toInt32();
    maNupRowsEdt.SetValue( sal_Int64( nVal > 1 ? nVal : 1) );

    aValue = pItem->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintDialog_NUpPage" ) ),
                              rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "NUp-Columns" ) ) );
    nVal = aValue.toInt32();
    maNupColEdt.SetValue( sal_Int64(nVal > 1 ? nVal : 1) );

    aValue = pItem->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintDialog_NUpPage" ) ),
                              rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "NUp-Portrait" ) ) );
    if( aValue.equalsIgnoreAsciiCaseAscii( "true" ) )
        maNupPortrait.Check();
    else
        maNupLandscape.Check();
    #endif
}

void PrintDialog::NUpTabPage::storeToSettings()
{
    #if 0
    SettingsConfigItem* pItem = SettingsConfigItem::get();
    pItem->setValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintDialog_NUpPage" ) ),
                     rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "NUp-Rows" ) ),
                     maNupRowsEdt.GetText() );
    pItem->setValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintDialog_NUpPage" ) ),
                     rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "NUp-Columns" ) ),
                     maNupColEdt.GetText() );
    pItem->setValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintDialog_NUpPage" ) ),
                     rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "NUp-Portrait" ) ),
                     rtl::OUString::createFromAscii( maNupPortrait.IsChecked() ? "true" : "false" ) );
    #endif
}

PrintDialog::JobTabPage::JobTabPage( Window* i_pParent, const ResId& rResId )
    : TabPage( i_pParent, rResId )
    , maPrinters( this, VclResId( SV_PRINT_PRINTERS) )
    , maSetupButton( this, VclResId( SV_PRINT_PRT_SETUP ) )
    , maToFileBox( this, VclResId( SV_PRINT_PRT_TOFILE ) )
    , maCopies( this, VclResId( SV_PRINT_COPIES ) )
    , maCopyCount( this, VclResId( SV_PRINT_COPYCOUNT ) )
    , maCopyCountField( this, VclResId( SV_PRINT_COPYCOUNT_FIELD ) )
    , maCollateBox( this, VclResId( SV_PRINT_COLLATE ) )
    , maCollateImage( this, VclResId( SV_PRINT_COLLATE_IMAGE ) )
    , maCollateImg( VclResId( SV_PRINT_COLLATE_IMG ) )
    , maCollateHCImg( VclResId( SV_PRINT_COLLATE_HC_IMG ) )
    , maNoCollateImg( VclResId( SV_PRINT_NOCOLLATE_IMG ) )
    , maNoCollateHCImg( VclResId( SV_PRINT_NOCOLLATE_HC_IMG ) )
{
    FreeResource();
    maPrinters.SMHID2( "JobPage", "PrinterList" );
    maSetupButton.SMHID2( "JobPage", "Setup" );
    maToFileBox.SMHID2( "JobPage", "ToFile" );
    maCopies.SMHID2( "JobPage", "CopiesLine" );
    maCopyCount.SMHID2( "JobPage", "CopiesText" );
    maCopyCountField.SMHID2( "JobPage", "Copies" );
    maCollateBox.SMHID2( "JobPage", "Collate" );
    maCollateImage.SMHID2( "JobPage", "CollateImage" );
}

PrintDialog::JobTabPage::~JobTabPage()
{
}

void PrintDialog::JobTabPage::readFromSettings()
{
    SettingsConfigItem* pItem = SettingsConfigItem::get();
    rtl::OUString aValue;

    aValue = pItem->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintDialog_JobPage" ) ),
                              rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ToFile" ) ) );
    maToFileBox.Check( aValue.equalsIgnoreAsciiCaseAscii( "true" ) );

    #if 0
    // do not actually make copy count persistent
    // the assumption is that this would lead to a lot of unwanted copies
    aValue = pItem->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintDialog_JobPage" ) ),
                              rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Copies" ) ) );
    sal_Int32 nVal = aValue.toInt32();
    maCopyCountField.SetValue( sal_Int64(nVal > 1 ? nVal : 1) );
    #endif

    aValue = pItem->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintDialog_JobPage" ) ),
                              rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Collate" ) ) );
    maCollateBox.Check( aValue.equalsIgnoreAsciiCaseAscii( "true" ) );
}

void PrintDialog::JobTabPage::storeToSettings()
{
    SettingsConfigItem* pItem = SettingsConfigItem::get();
    pItem->setValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintDialog_JobPage" ) ),
                     rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ToFile" ) ),
                     rtl::OUString::createFromAscii( maToFileBox.IsChecked() ? "true" : "false" ) );
    pItem->setValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintDialog_JobPage" ) ),
                     rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Copies" ) ),
                     maCopyCountField.GetText() );
    pItem->setValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintDialog_JobPage" ) ),
                     rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Collate" ) ),
                     rtl::OUString::createFromAscii( maCollateBox.IsChecked() ? "true" : "false" ) );
}

PrintDialog::PrintDialog( Window* i_pParent, const boost::shared_ptr<PrinterListener>& i_rListener )
    : ModalDialog( i_pParent, VclResId( SV_DLG_PRINT ) )
    , maOKButton( this, VclResId( SV_PRINT_OK ) )
    , maCancelButton( this, VclResId( SV_PRINT_CANCEL ) )
    , maPreviewWindow( this, VclResId( SV_PRINT_PAGE_PREVIEW ) )
    , maPageEdit( this, VclResId( SV_PRINT_PAGE_EDIT ) )
    , maNumPagesText( this, VclResId( SV_PRINT_PAGE_TXT ) )
    , maForwardBtn( this, VclResId( SV_PRINT_PAGE_FORWARD ) )
    , maBackwardBtn( this, VclResId( SV_PRINT_PAGE_BACKWARD ) )
    , maTabCtrl( this, VclResId( SV_PRINT_TABCTRL ) )
    , maNUpPage( &maTabCtrl, VclResId( SV_PRINT_TAB_NUP ) )
    , maJobPage( &maTabCtrl, VclResId( SV_PRINT_TAB_JOB ) )
    , maButtonLine( this, VclResId( SV_PRINT_BUTTONLINE ) )
    , maPListener( i_rListener )
    , maNoPageStr( String( VclResId( SV_PRINT_NOPAGES ) ) )
    , mnCurPage( 0 )
    , mnCachedPages( 0 )
    , maCommentText( String( VclResId( SV_PRINT_PRT_COMMENT ) ) )
    , maStatusText( String( VclResId( SV_PRINT_PRT_STATUS ) ) )
    , maLocationText( String( VclResId( SV_PRINT_PRT_LOCATION ) ) )
    , maTypeText( String( VclResId( SV_PRINT_PRT_TYPE ) ) )
    , maPreviewCtrlRow( NULL, false )
{
    FreeResource();

    // setup preview controls
    maForwardBtn.SetStyle( maForwardBtn.GetStyle() | WB_BEVELBUTTON );
    maBackwardBtn.SetStyle( maBackwardBtn.GetStyle() | WB_BEVELBUTTON );
    maPreviewCtrlRow.setParentWindow( this );
    maPreviewCtrlRow.addWindow( &maPageEdit );
    maPreviewCtrlRow.addWindow( &maNumPagesText );
    maPreviewCtrlRow.addChild( new vcl::Spacer( &maPreviewCtrlRow ) );
    maPreviewCtrlRow.addWindow( &maBackwardBtn );
    maPreviewCtrlRow.addWindow( &maForwardBtn );

    // insert the tab pages
    maTabCtrl.InsertPage( SV_PRINT_TAB_JOB, maJobPage.GetText() );
    maTabCtrl.SetTabPage( SV_PRINT_TAB_JOB, &maJobPage );
    maTabCtrl.InsertPage( SV_PRINT_PAGE_PREVIEW, maNUpPage.GetText() );
    maTabCtrl.SetTabPage( SV_PRINT_PAGE_PREVIEW, &maNUpPage );

    // set symbols on forward and backward button
    maBackwardBtn.SetSymbol( SYMBOL_PREV );
    maForwardBtn.SetSymbol( SYMBOL_NEXT );
    maBackwardBtn.ImplSetSmallSymbol( TRUE );
    maForwardBtn.ImplSetSmallSymbol( TRUE );

    maPageStr = maNumPagesText.GetText();
    // save space for the preview window
    maPreviewSpace = Rectangle( maPreviewWindow.GetPosPixel(), maPreviewWindow.GetSizePixel() );
    // get the first page
    preparePreview( true, true );

    // fill printer listbox
    const std::vector< rtl::OUString >& rQueues( Printer::GetPrinterQueues() );
    for( std::vector< rtl::OUString >::const_iterator it = rQueues.begin();
         it != rQueues.end(); ++it )
    {
        maJobPage.maPrinters.InsertEntry( *it );
    }
    // select current printer
    if( maJobPage.maPrinters.GetEntryPos( maPListener->getPrinter()->GetName() ) != LISTBOX_ENTRY_NOTFOUND )
    {
        maJobPage.maPrinters.SelectEntry( maPListener->getPrinter()->GetName() );
    }
    else
    {
        // fall back to last printer
        SettingsConfigItem* pItem = SettingsConfigItem::get();
        String aValue( pItem->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintDialog" ) ),
                                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LastPrinter" ) ) ) );
        if( maJobPage.maPrinters.GetEntryPos( aValue ) != LISTBOX_ENTRY_NOTFOUND )
        {
            maJobPage.maPrinters.SelectEntry( aValue );
            maPListener->setPrinter( boost::shared_ptr<Printer>( new Printer( aValue ) ) );
        }
        else
        {
            // fall back to default printer
            maJobPage.maPrinters.SelectEntry( Printer::GetDefaultPrinterName() );
            maPListener->setPrinter( boost::shared_ptr<Printer>( new Printer( Printer::GetDefaultPrinterName() ) ) );
        }
    }
    // update the text fields for the printer
    updatePrinterText();

    // set a select handler
    maJobPage.maPrinters.SetSelectHdl( LINK( this, PrintDialog, SelectHdl ) );

    // setup sizes for N-Up
    Size aNupSize( maPListener->getPrinter()->PixelToLogic(
                         maPListener->getPrinter()->GetPaperSizePixel(), MapMode( MAP_100TH_MM ) ) );
    if( maPListener->getPrinter()->GetOrientation() == ORIENTATION_LANDSCAPE )
    {
        maNupLandscapeSize = aNupSize;
        maNupPortraitSize = Size( aNupSize.Height(), aNupSize.Width() );
        maNUpPage.maNupLandscape.Check();
    }
    else
    {
        maNupPortraitSize = aNupSize;
        maNupLandscapeSize = Size( aNupSize.Height(), aNupSize.Width() );
        maNUpPage.maNupPortrait.Check();
    }
    maNUpPage.initFromMultiPageSetup( maPListener->getMultipage() );


    // setup click handler on the various buttons
    maOKButton.SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    #if OSL_DEBUG_LEVEL > 1
    maCancelButton.SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    #endif
    maForwardBtn.SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    maBackwardBtn.SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    maJobPage.maCollateBox.SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    maJobPage.maSetupButton.SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    maNUpPage.maNupPortrait.SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    maNUpPage.maNupLandscape.SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    maNUpPage.maBorderCB.SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );

    // setup modify hdl
    maPageEdit.SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    maJobPage.maCopyCountField.SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    maNUpPage.maNupRowsEdt.SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    maNUpPage.maNupColEdt.SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    maNUpPage.maNupRepEdt.SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    maNUpPage.maLeftMarginEdt.SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    maNUpPage.maTopMarginEdt.SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    maNUpPage.maRightMarginEdt.SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    maNUpPage.maBottomMarginEdt.SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    maNUpPage.maHSpaceEdt.SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    maNUpPage.maVSpaceEdt.SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );

    // setup optional UI options set by application
    setupOptionalUI();

    // set change handler for UI options
    maPListener->setOptionChangeHdl( LINK( this, PrintDialog, UIOptionsChanged ) );

    // set min size pixel to current size
    SetMinOutputSizePixel( GetOutputSizePixel() );

    // restore settings from last run
    readFromSettings();

    // setup dependencies
    checkControlDependencies();

    // set HelpIDs
    maOKButton.SMHID1( "OK" );
    maCancelButton.SMHID1( "Cancel" );
    maPreviewWindow.SMHID1( "Preview" );
    maNumPagesText.SMHID1( "NumPagesText" );
    maPageEdit.SMHID1( "PageEdit" );
    maForwardBtn.SMHID1( "ForwardBtn" );
    maBackwardBtn.SMHID1( "BackwardBtn" );
    maTabCtrl.SMHID1( "TabPages" );
}

PrintDialog::~PrintDialog()
{
    while( ! maControls.empty() )
    {
        delete maControls.front();
        maControls.pop_front();
    }
}

void PrintDialog::readFromSettings()
{
    maJobPage.readFromSettings();
    maNUpPage.readFromSettings();

    // read last selected tab page; if it exists, actiavte it
    SettingsConfigItem* pItem = SettingsConfigItem::get();
    rtl::OUString aValue = pItem->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintDialog" ) ),
                                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LastPage" ) ) );
    USHORT nCount = maTabCtrl.GetPageCount();
    for( USHORT i = 0; i < nCount; i++ )
    {
        USHORT nPageId = maTabCtrl.GetPageId( i );
        if( aValue.equals( maTabCtrl.GetPageText( nPageId ) ) )
        {
            maTabCtrl.SelectTabPage( nPageId );
            break;
        }
    }
}

void PrintDialog::storeToSettings()
{
    maJobPage.storeToSettings();
    maNUpPage.storeToSettings();

    // store last selected printer
    SettingsConfigItem* pItem = SettingsConfigItem::get();
    pItem->setValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintDialog" ) ),
                     rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LastPrinter" ) ),
                     maJobPage.maPrinters.GetSelectEntry() );

    pItem->setValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintDialog" ) ),
                     rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LastPage" ) ),
                     maTabCtrl.GetPageText( maTabCtrl.GetCurPageId() ) );
    pItem->Commit();
}

bool PrintDialog::isPrintToFile()
{
    return maJobPage.maToFileBox.IsChecked();
}

int PrintDialog::getCopyCount()
{
    return static_cast<int>(maJobPage.maCopyCountField.GetValue());
}

bool PrintDialog::isCollate()
{
    return maJobPage.maCopyCountField.GetValue() > 1 ? maJobPage.maCollateBox.IsChecked() : FALSE;
}

static void setSmartId( Window* i_pWindow, const char* i_pType, sal_Int32 i_nId = -1, const rtl::OUString& i_rPropName = rtl::OUString() )
{
    rtl::OUStringBuffer aBuf( 256 );
    aBuf.appendAscii( HELPID_PREFIX );
    if( i_rPropName.getLength() )
    {
        aBuf.append( sal_Unicode( ':' ) );
        aBuf.append( i_rPropName );
    }
    if( i_pType )
    {
        aBuf.append( sal_Unicode( ':' ) );
        aBuf.appendAscii( i_pType );
    }
    if( i_nId >= 0 )
    {
        aBuf.append( sal_Unicode( ':' ) );
        aBuf.append( i_nId );
    }
    i_pWindow->SetSmartHelpId( SmartId( aBuf.makeStringAndClear() ) );
}

static void setHelpText( Window* i_pWindow, const Sequence< rtl::OUString >& i_rHelpTexts, sal_Int32 i_nIndex )
{
    if( i_nIndex >= 0 && i_nIndex < i_rHelpTexts.getLength() )
        i_pWindow->SetHelpText( i_rHelpTexts.getConstArray()[i_nIndex] );
}

void PrintDialog::setupOptionalUI()
{
    const long nBorderWidth = maJobPage.maCopies.GetPosPixel().X();

    vcl::RowOrColumn aPrintRangeRow( NULL, false, nBorderWidth );
    std::vector<vcl::RowOrColumn*> aDynamicColumns;
    vcl::RowOrColumn* pCurColumn = 0;

    aPrintRangeRow.setParentWindow( &maJobPage );
    aPrintRangeRow.setOuterBorder( nBorderWidth );

    Window* pCurParent = 0, *pDynamicPageParent = 0;
    USHORT nOptPageId = 9, nCurSubGroup = 0;
    bool bOnJobPage = false;

    std::multimap< rtl::OUString, vcl::RowOrColumn* > aPropertyToDependencyRowMap;

    const Sequence< PropertyValue >& rOptions( maPListener->getUIOptions() );
    for( int i = 0; i < rOptions.getLength(); i++ )
    {
        Sequence< beans::PropertyValue > aOptProp;
        rOptions[i].Value >>= aOptProp;

        // extract ui element
        bool bEnabled = true;
        rtl::OUString aCtrlType;
        rtl::OUString aText;
        rtl::OUString aPropertyName;
        Sequence< rtl::OUString > aChoices;
        Sequence< rtl::OUString > aHelpTexts;
        sal_Int64 nMinValue = 0, nMaxValue = 0;
        sal_Int32 nCurHelpText = 0;
        sal_Bool bOnJobPageValue = sal_False;
        rtl::OUString aDependsOnName;
        sal_Int32 nDependsOnValue = 0;
        sal_Bool bUseDependencyRow = sal_False;

        for( int n = 0; n < aOptProp.getLength(); n++ )
        {
            const beans::PropertyValue& rEntry( aOptProp[ n ] );
            if( rEntry.Name.equalsAscii( "Text" ) )
            {
                rEntry.Value >>= aText;
            }
            else if( rEntry.Name.equalsAscii( "ControlType" ) )
            {
                rEntry.Value >>= aCtrlType;
            }
            else if( rEntry.Name.equalsAscii( "Choices" ) )
            {
                rEntry.Value >>= aChoices;
            }
            else if( rEntry.Name.equalsAscii( "Property" ) )
            {
                PropertyValue aVal;
                rEntry.Value >>= aVal;
                aPropertyName = aVal.Name;
            }
            else if( rEntry.Name.equalsAscii( "Enabled" ) )
            {
                sal_Bool bValue = sal_True;
                rEntry.Value >>= bValue;
                bEnabled = bValue;
            }
            else if( rEntry.Name.equalsAscii( "PutOnJobPage" ) )
            {
                rEntry.Value >>= bOnJobPageValue;
            }
            else if( rEntry.Name.equalsAscii( "DependsOnName" ) )
            {
                rEntry.Value >>= aDependsOnName;
            }
            else if( rEntry.Name.equalsAscii( "DependsOnEntry" ) )
            {
                rEntry.Value >>= nDependsOnValue;
            }
            else if( rEntry.Name.equalsAscii( "AttachToDependency" ) )
            {
                rEntry.Value >>= bUseDependencyRow;
            }
            else if( rEntry.Name.equalsAscii( "MinValue" ) )
            {
                rEntry.Value >>= nMinValue;
            }
            else if( rEntry.Name.equalsAscii( "MaxValue" ) )
            {
                rEntry.Value >>= nMaxValue;
            }
            else if( rEntry.Name.equalsAscii( "HelpText" ) )
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
        }

        if( aCtrlType.equalsAscii( "Group" ) ||
            ( ! pCurParent && ! (bOnJobPage || bOnJobPageValue) ) )
        {
            // add new tab page
            TabPage* pNewGroup = new TabPage( &maTabCtrl );
            maControls.push_front( pNewGroup );
            pCurParent = pNewGroup;
            pNewGroup->SetText( aText );
            maTabCtrl.InsertPage( ++nOptPageId, aText );
            maTabCtrl.SetTabPage( nOptPageId, pNewGroup );

            // set help id
            setSmartId( pNewGroup, "TabPage", nOptPageId );
            // set help text
            setHelpText( pNewGroup, aHelpTexts, 0 );

            // reset subgroup counter
            nCurSubGroup = 0;

            aDynamicColumns.push_back( new vcl::RowOrColumn( NULL, true, nBorderWidth ) );
            pCurColumn = aDynamicColumns.back();
            pCurColumn->setParentWindow( pNewGroup );
            pCurColumn->setOuterBorder( nBorderWidth );
        }
        else if( aCtrlType.equalsAscii( "Subgroup" ) && (pCurParent || bOnJobPageValue) )
        {
            // change to job page or back if necessary
            if( (bOnJobPage && ! bOnJobPageValue) ||
                (! bOnJobPage && bOnJobPageValue) )
            {
                bOnJobPage = bOnJobPageValue;
                if( bOnJobPage )
                {
                    pDynamicPageParent = pCurParent;    // save current parent
                    pCurParent = &maJobPage;            // set job page as current parent
                }
                else
                {
                    pCurParent = pDynamicPageParent;    // set current tab page as parent
                }
            }

            if( bOnJobPage )
            {
                // create a new column in the PrintRange row
                vcl::RowOrColumn* pNewColumn = new vcl::RowOrColumn( &aPrintRangeRow, true, nBorderWidth );
                aPrintRangeRow.addChild( pNewColumn );
                pCurColumn = pNewColumn;
            }
            else
                pCurColumn = aDynamicColumns.back();

            // create group FixedLine
            FixedLine* pNewSub = new FixedLine( pCurParent );
            maControls.push_front( pNewSub );
            pNewSub->SetText( aText );
            pNewSub->Show();

            // set help id
            setSmartId( pNewSub, "FixedLine", sal_Int32( nCurSubGroup++ ) );
            // set help text
            setHelpText( pNewSub, aHelpTexts, 0 );

            // add group to current column
            pCurColumn->addWindow( pNewSub );
        }
        else
        {
            vcl::RowOrColumn* pSaveCurColumn = pCurColumn;

            if( bUseDependencyRow && aDependsOnName.getLength() )
            {
                // find the correct dependency row (if any)
                std::pair< std::multimap< rtl::OUString, vcl::RowOrColumn* >::iterator,
                           std::multimap< rtl::OUString, vcl::RowOrColumn* >::iterator > aDepRange;
                aDepRange = aPropertyToDependencyRowMap.equal_range( aDependsOnName );
                if( aDepRange.first != aDepRange.second )
                {
                    while( nDependsOnValue && aDepRange.first != aDepRange.second )
                    {
                        nDependsOnValue--;
                        ++aDepRange.first;
                    }
                    if( aDepRange.first != aPropertyToDependencyRowMap.end() )
                    {
                        pCurColumn = aDepRange.first->second;
                    }
                }
            }
            if( aCtrlType.equalsAscii( "Bool" ) && pCurParent )
            {
                // add a check box
                CheckBox* pNewBox = new CheckBox( pCurParent );
                maControls.push_front( pNewBox );
                pNewBox->SetText( aText );
                pNewBox->Show();

                sal_Bool bVal = sal_False;
                PropertyValue* pVal = maPListener->getValue( aPropertyName );
                if( pVal )
                    pVal->Value >>= bVal;
                pNewBox->Check( bVal );
                pNewBox->Enable( maPListener->isUIOptionEnabled( aPropertyName ) && pVal != NULL );
                pNewBox->SetToggleHdl( LINK( this, PrintDialog, UIOption_CheckHdl ) );

                maPropertyToWindowMap.insert( std::pair< rtl::OUString, Window* >( aPropertyName, pNewBox ) );
                maControlToPropertyMap[pNewBox] = aPropertyName;

                // set help id
                setSmartId( pNewBox, "CheckBox", -1, aPropertyName );
                // set help text
                setHelpText( pNewBox, aHelpTexts, 0 );

                vcl::RowOrColumn* pDependencyRow = new vcl::RowOrColumn( pCurColumn, false );
                pCurColumn->addChild( pDependencyRow );
                aPropertyToDependencyRowMap.insert( std::pair< rtl::OUString, vcl::RowOrColumn* >( aPropertyName, pDependencyRow ) );

                // add checkbox to current column
                pDependencyRow->addWindow( pNewBox );
            }
            else if( aCtrlType.equalsAscii( "Radio" ) && pCurParent )
            {
                vcl::RowOrColumn* pRadioColumn = pCurColumn;
                if( aText.getLength() )
                {
                    // add a FixedText:
                    FixedText* pHeading = new FixedText( pCurParent );
                    maControls.push_front( pHeading );
                    pHeading->SetText( aText );
                    pHeading->Show();

                    // set help id
                    setSmartId( pHeading, "FixedText", -1, aPropertyName );
                    // set help text
                    setHelpText( pHeading, aHelpTexts, nCurHelpText++ );
                    // add fixed text to current column
                    pCurColumn->addWindow( pHeading );
                    // add an indent to the current column
                    vcl::Indenter* pIndent = new vcl::Indenter( pCurColumn, 15 );
                    pCurColumn->addChild( pIndent );
                    // and create a column inside the indent
                    pRadioColumn = new vcl::RowOrColumn( pIndent );
                    pIndent->setChild( pRadioColumn );
                }
                // iterate options
                sal_Int32 nSelectVal = 0;
                PropertyValue* pVal = maPListener->getValue( aPropertyName );
                if( pVal && pVal->Value.hasValue() )
                    pVal->Value >>= nSelectVal;
                for( sal_Int32 m = 0; m < aChoices.getLength(); m++ )
                {
                    vcl::RowOrColumn* pDependencyRow = new vcl::RowOrColumn( pCurColumn, false );
                    pRadioColumn->addChild( pDependencyRow );
                    aPropertyToDependencyRowMap.insert( std::pair< rtl::OUString, vcl::RowOrColumn* >( aPropertyName, pDependencyRow ) );

                    RadioButton* pBtn = new RadioButton( pCurParent, m == 0 ? WB_GROUP : 0 );
                    maControls.push_front( pBtn );
                    pBtn->SetText( aChoices[m] );
                    pBtn->Check( m == nSelectVal );
                    pBtn->Enable( maPListener->isUIOptionEnabled( aPropertyName ) );
                    pBtn->SetToggleHdl( LINK( this, PrintDialog, UIOption_RadioHdl ) );
                    pBtn->Show();
                    maPropertyToWindowMap.insert( std::pair< rtl::OUString, Window* >( aPropertyName, pBtn ) );
                    maControlToPropertyMap[pBtn] = aPropertyName;
                    maControlToNumValMap[pBtn] = m;

                    // set help id
                    setSmartId( pBtn, "RadioButton", m, aPropertyName );
                    // set help text
                    setHelpText( pBtn, aHelpTexts, nCurHelpText++ );
                    // add the radio button to the column
                    pDependencyRow->addWindow( pBtn );
                }
            }
            else if( ( aCtrlType.equalsAscii( "List" )   ||
                       aCtrlType.equalsAscii( "Range" )  ||
                       aCtrlType.equalsAscii( "Edit" )
                     ) && pCurParent )
            {
                // create a row in the current column
                vcl::RowOrColumn* pFieldColumn = new vcl::RowOrColumn( pCurColumn, false );
                pCurColumn->addChild( pFieldColumn );
                aPropertyToDependencyRowMap.insert( std::pair< rtl::OUString, vcl::RowOrColumn* >( aPropertyName, pFieldColumn ) );

                if( aText.getLength() )
                {
                    // add a FixedText:
                    FixedText* pHeading = new FixedText( pCurParent, WB_VCENTER );
                    maControls.push_front( pHeading );
                    pHeading->SetText( aText );
                    pHeading->Show();

                    // set help id
                    setSmartId( pHeading, "FixedText", -1, aPropertyName );

                    // add to row
                    pFieldColumn->addWindow( pHeading );
                }

                if( aCtrlType.equalsAscii( "List" ) )
                {
                    ListBox* pList = new ListBox( pCurParent, WB_DROPDOWN | WB_BORDER );
                    maControls.push_front( pList );

                    // iterate options
                    for( sal_Int32 m = 0; m < aChoices.getLength(); m++ )
                    {
                        pList->InsertEntry( aChoices[m] );
                    }
                    sal_Int32 nSelectVal = 0;
                    PropertyValue* pVal = maPListener->getValue( aPropertyName );
                    if( pVal && pVal->Value.hasValue() )
                        pVal->Value >>= nSelectVal;
                    pList->SelectEntryPos( static_cast<USHORT>(nSelectVal) );
                    pList->Enable( maPListener->isUIOptionEnabled( aPropertyName ) );
                    pList->SetSelectHdl( LINK( this, PrintDialog, UIOption_SelectHdl ) );
                    pList->SetDropDownLineCount( static_cast<USHORT>(aChoices.getLength()) );
                    pList->Show();

                    // set help id
                    setSmartId( pList, "ListBox", -1, aPropertyName );
                    // set help text
                    setHelpText( pList, aHelpTexts, 0 );

                    maPropertyToWindowMap.insert( std::pair< rtl::OUString, Window* >( aPropertyName, pList ) );
                    maControlToPropertyMap[pList] = aPropertyName;

                    // finish the pair
                    pFieldColumn->addWindow( pList );
                }
                else if( aCtrlType.equalsAscii( "Range" ) )
                {
                    NumericField* pField = new NumericField( pCurParent, WB_BORDER | WB_SPIN );
                    maControls.push_front( pField );

                    // set min/max and current value
                    if( nMinValue != nMaxValue )
                    {
                        pField->SetMin( nMinValue );
                        pField->SetMax( nMaxValue );
                    }
                    sal_Int64 nCurVal = 0;
                    PropertyValue* pVal = maPListener->getValue( aPropertyName );
                    if( pVal && pVal->Value.hasValue() )
                        pVal->Value >>= nCurVal;
                    pField->SetValue( nCurVal );

                    pField->Enable( maPListener->isUIOptionEnabled( aPropertyName ) );
                    pField->SetModifyHdl( LINK( this, PrintDialog, UIOption_ModifyHdl ) );
                    pField->Show();

                    // set help id
                    setSmartId( pField, "NumericField", -1, aPropertyName );
                    // set help text
                    setHelpText( pField, aHelpTexts, 0 );

                    maPropertyToWindowMap.insert( std::pair< rtl::OUString, Window* >( aPropertyName, pField ) );
                    maControlToPropertyMap[pField] = aPropertyName;

                    // add to row
                    pFieldColumn->addWindow( pField );
                }
                else if( aCtrlType.equalsAscii( "Edit" ) )
                {
                    Edit* pField = new Edit( pCurParent, WB_BORDER );
                    maControls.push_front( pField );

                    rtl::OUString aCurVal;
                    PropertyValue* pVal = maPListener->getValue( aPropertyName );
                    if( pVal && pVal->Value.hasValue() )
                        pVal->Value >>= aCurVal;
                    pField->SetText( aCurVal );
                    pField->Enable( maPListener->isUIOptionEnabled( aPropertyName ) );
                    pField->SetModifyHdl( LINK( this, PrintDialog, UIOption_ModifyHdl ) );
                    pField->Show();

                    // set help id
                    setSmartId( pField, "Edit", -1, aPropertyName );
                    // set help text
                    setHelpText( pField, aHelpTexts, 0 );

                    maPropertyToWindowMap.insert( std::pair< rtl::OUString, Window* >( aPropertyName, pField ) );
                    maControlToPropertyMap[pField] = aPropertyName;

                    // add to row
                    pFieldColumn->addWindow( pField, 2 );
                }
            }
            else
            {
                DBG_ERROR( "Unsupported UI option" );
            }

            pCurColumn = pSaveCurColumn;
        }
    }

    // calculate job page
    long nJobPageCurY = maJobPage.maCollateImage.GetPosPixel().Y();
    nJobPageCurY += maJobPage.maCollateImage.GetSizePixel().Height();

    Size aMaxSize = aPrintRangeRow.getOptimalSize( WINDOWSIZE_PREFERRED );
    aMaxSize.Height() += nJobPageCurY;

    Size aMaxPageSize;
    for( std::vector< vcl::RowOrColumn* >::iterator it = aDynamicColumns.begin();
         it != aDynamicColumns.end(); ++it )
    {
        Size aPageSize( (*it)->getOptimalSize( WINDOWSIZE_PREFERRED ) );
        if( aPageSize.Width() > aMaxPageSize.Width() )
            aMaxPageSize.Width() = aPageSize.Width();
        if( aPageSize.Height() > aMaxPageSize.Height() )
            aMaxPageSize.Height() = aPageSize.Height();
    }
    if( aMaxPageSize.Width() > aMaxSize.Width() )
        aMaxSize.Width() = aMaxPageSize.Width();
    if( aMaxPageSize.Height() > aMaxSize.Height() )
        aMaxSize.Height() = aMaxPageSize.Height();

    // resize dialog if necessary
    Size aTabSize = maTabCtrl.GetTabPageSizePixel();
    if( aMaxSize.Height() > aTabSize.Height() || aMaxSize.Width() > aTabSize.Width() )
    {
        Size aCurSize( GetSizePixel() );
        if( aMaxSize.Height() > aTabSize.Height() )
            aCurSize.Height() += aMaxSize.Height() - aTabSize.Height();
        if( aMaxSize.Width() > aTabSize.Width() )
        {
            aCurSize.Width() += aMaxSize.Width() - aTabSize.Width();
            // and the tab ctrl needs more space, too
            aTabSize.Width() = aMaxSize.Width();
            maTabCtrl.SetSizePixel( aTabSize );
        }
        SetSizePixel( aCurSize );
    }

    // and finally arrange controls
    aTabSize = maTabCtrl.GetTabPageSizePixel();
    aPrintRangeRow.setManagedArea( Rectangle( Point( 0, nJobPageCurY ),
                                   Size( aTabSize.Width(), aTabSize.Height() - nJobPageCurY ) ) );
    for( std::vector< vcl::RowOrColumn* >::iterator it = aDynamicColumns.begin();
         it != aDynamicColumns.end(); ++it )
    {
        (*it)->setManagedArea( Rectangle( Point(), aMaxPageSize ) );
        delete *it;
        *it = NULL;
    }
}

void PrintDialog::checkControlDependencies()
{
    if( maJobPage.maCopyCountField.GetValue() > 1 )
        maJobPage.maCollateBox.Enable( TRUE );
    else
        maJobPage.maCollateBox.Enable( FALSE );

    Image aImg( maJobPage.maCollateBox.IsChecked() ? maJobPage.maCollateImg : maJobPage.maNoCollateImg );
    if( GetSettings().GetStyleSettings().GetFieldColor().IsDark() )
        aImg = maJobPage.maCollateBox.IsChecked() ? maJobPage.maCollateHCImg : maJobPage.maNoCollateHCImg;

    // adjust size of image
    maJobPage.maCollateImage.SetSizePixel( aImg.GetSizePixel() );
    maJobPage.maCollateImage.SetImage( aImg );

    // enable setup button only for printers that can be setup
    bool bHaveSetup = maPListener->getPrinter()->HasSupport( SUPPORT_SETUPDIALOG );
    maJobPage.maSetupButton.Enable( bHaveSetup );
    if( bHaveSetup )
    {
        if( ! maJobPage.maSetupButton.IsVisible() )
        {
            Point aPrinterPos( maJobPage.maPrinters.GetPosPixel() );
            Point aSetupPos( maJobPage.maSetupButton.GetPosPixel() );
            Size aPrinterSize( maJobPage.maPrinters.GetSizePixel() );
            aPrinterSize.Width() = aSetupPos.X() - aPrinterPos.X() - LogicToPixel( Size( 5, 5 ), MapMode( MAP_APPFONT ) ).Width();
            maJobPage.maPrinters.SetSizePixel( aPrinterSize );
            maJobPage.maSetupButton.Show();
        }
    }
    else
    {
        if( maJobPage.maSetupButton.IsVisible() )
        {
            Point aPrinterPos( maJobPage.maPrinters.GetPosPixel() );
            Point aSetupPos( maJobPage.maSetupButton.GetPosPixel() );
            Size aPrinterSize( maJobPage.maPrinters.GetSizePixel() );
            Size aSetupSize( maJobPage.maSetupButton.GetSizePixel() );
            aPrinterSize.Width() = aSetupPos.X() + aSetupSize.Width() - aPrinterPos.X();
            maJobPage.maPrinters.SetSizePixel( aPrinterSize );
            maJobPage.maSetupButton.Hide();
        }
    }
}

void PrintDialog::checkOptionalControlDependencies()
{
    for( std::map< Window*, rtl::OUString >::iterator it = maControlToPropertyMap.begin();
         it != maControlToPropertyMap.end(); ++it )
    {
        bool bShouldbeEnabled = maPListener->isUIOptionEnabled( it->second );
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
    const QueueInfo* pInfo = Printer::GetQueueInfo( maJobPage.maPrinters.GetSelectEntry(), true );
    if( pInfo )
    {
        rtl::OUStringBuffer aBuf( 256 );
        aBuf.append( searchAndReplace( maTypeText, "%s", 2, pInfo->GetDriver() ) );
        aBuf.append( sal_Unicode( '\n' ) );
        aBuf.append( searchAndReplace( maLocationText, "%s", 2, pInfo->GetLocation() ) );
        aBuf.append( sal_Unicode( '\n' ) );
        aBuf.append( searchAndReplace( maCommentText, "%s", 2, pInfo->GetComment() ) );
        aBuf.append( sal_Unicode( '\n' ) );
        maJobPage.maPrinters.SetHelpText( aBuf.makeStringAndClear() );
    }
}

void PrintDialog::setPreviewText( sal_Int32 )
{
    if( mnCachedPages != 0 )
    {
        rtl::OUString aNewText( searchAndReplace( maPageStr, "%n", 2, rtl::OUString::valueOf( mnCachedPages )  ) );
        maNumPagesText.SetText( aNewText );
    }
    else
        maNumPagesText.SetText( maNoPageStr );
}

void PrintDialog::preparePreview( bool i_bNewPage, bool i_bMayUseCache )
{
    // page range may have changed depending on options
    sal_Int32 nPages = maPListener->getFilteredPageCount();
    mnCachedPages = nPages;

    if( mnCurPage >= nPages )
        mnCurPage = nPages-1;
    if( mnCurPage < 0 )
        mnCurPage = 0;

    setPreviewText( mnCurPage );

    maPageEdit.SetMin( 1 );
    maPageEdit.SetMax( nPages );

    boost::shared_ptr<Printer> aPrt( maPListener->getPrinter() );


    if( i_bNewPage )
    {
        const MapMode aMapMode( MAP_100TH_MM );
        GDIMetaFile aMtf;
        if( nPages > 0 )
            maCurPageSize = maPListener->getFilteredPageFile( mnCurPage, aMtf, i_bMayUseCache );

        maPreviewWindow.setPreview( aMtf );
    }
    // catch corner case of strange page size
    if( maCurPageSize.Width() == 0 || maCurPageSize.Height() == 0 )
        maCurPageSize = aPrt->PixelToLogic( aPrt->GetPaperSizePixel(), MapMode( MAP_100TH_MM ) );

    Size aPreviewSize;
    Point aPreviewPos = maPreviewSpace.TopLeft();
    const long nW = maPreviewSpace.GetSize().Width();
    const long nH = maPreviewSpace.GetSize().Height();
    if( maCurPageSize.Width() > maCurPageSize.Height() )
    {
        aPreviewSize = Size( nW, nW * maCurPageSize.Height() / maCurPageSize.Width() );
        aPreviewPos.Y() += (maPreviewSpace.GetHeight() - aPreviewSize.Height())/2;
    }
    else
    {
        aPreviewSize = Size( nH * maCurPageSize.Width() / maCurPageSize.Height(), nH );
        aPreviewPos.X() += (maPreviewSpace.GetWidth() - aPreviewSize.Width())/2;
    }

    maPreviewWindow.SetPosSizePixel( aPreviewPos, aPreviewSize );
    const Size aLogicSize( maPreviewWindow.PixelToLogic( maPreviewWindow.GetSizePixel(), MapMode( MAP_100TH_MM ) ) );
    maPreviewWindow.setScale( double(aLogicSize.Width())/double(maCurPageSize.Width()),
                              double(aLogicSize.Height())/double(maCurPageSize.Height()) );
}

void PrintDialog::updateNup()
{
    int nRows   = int(maNUpPage.maNupRowsEdt.GetValue());
    int nCols   = int(maNUpPage.maNupColEdt.GetValue());
    int nRepeat = int(maNUpPage.maNupRepEdt.GetValue());

    PrinterListener::MultiPageSetup aMPS;
    aMPS.nRows         = nRows;
    aMPS.nColumns      = nCols;
    aMPS.nRepeat       = nRepeat;
    aMPS.aPaperSize    = maNUpPage.maNupPortrait.IsChecked()
                         ? maNupPortraitSize : maNupLandscapeSize;
    aMPS.nLeftMargin   = long(maNUpPage.maLeftMarginEdt.Denormalize(maNUpPage.maLeftMarginEdt.GetValue( FUNIT_100TH_MM )));
    aMPS.nTopMargin    = long(maNUpPage.maTopMarginEdt.Denormalize(maNUpPage.maTopMarginEdt.GetValue( FUNIT_100TH_MM )));
    aMPS.nRightMargin  = long(maNUpPage.maRightMarginEdt.Denormalize(maNUpPage.maRightMarginEdt.GetValue( FUNIT_100TH_MM )));
    aMPS.nBottomMargin = long(maNUpPage.maBottomMarginEdt.Denormalize(maNUpPage.maBottomMarginEdt.GetValue( FUNIT_100TH_MM )));

    aMPS.nHorizontalSpacing = long(maNUpPage.maHSpaceEdt.Denormalize(maNUpPage.maHSpaceEdt.GetValue( FUNIT_100TH_MM )));
    aMPS.nVerticalSpacing   = long(maNUpPage.maVSpaceEdt.Denormalize(maNUpPage.maVSpaceEdt.GetValue( FUNIT_100TH_MM )));

    aMPS.bDrawBorder        = maNUpPage.maBorderCB.IsChecked();

    maPListener->setMultipage( aMPS );

    preparePreview( true, true );
}

IMPL_LINK( PrintDialog, SelectHdl, ListBox*, pBox )
{
    if(  pBox == &maJobPage.maPrinters )
    {
        String aNewPrinter( pBox->GetSelectEntry() );
        maJobPage.maPrinters.SelectEntry( aNewPrinter );
        // set new printer
        maPListener->setPrinter( boost::shared_ptr<Printer>( new Printer( aNewPrinter ) ) );
        // update text fields
        updatePrinterText();
    }
    return 0;
}

IMPL_LINK( PrintDialog, ClickHdl, Button*, pButton )
{
    if( pButton == &maOKButton || pButton == &maCancelButton )
    {
        storeToSettings();
        EndDialog( pButton == &maOKButton );
    }
    else if( pButton == &maForwardBtn )
    {
        maPageEdit.Up();
    }
    else if( pButton == &maBackwardBtn )
    {
        maPageEdit.Down();
    }
    else
    {
        if( pButton == &maJobPage.maSetupButton )
        {
            maPListener->getPrinter()->Setup( this );
        }
        checkControlDependencies();
        if( pButton == &maNUpPage.maNupPortrait || pButton == &maNUpPage.maNupLandscape || pButton == &maNUpPage.maBorderCB )
            updateNup();
    }
    return 0;
}

IMPL_LINK( PrintDialog, ModifyHdl, Edit*, pEdit )
{
    checkControlDependencies();
    if( pEdit == &maNUpPage.maNupRowsEdt || pEdit == &maNUpPage.maNupColEdt || pEdit == &maNUpPage.maNupRepEdt ||
        pEdit == &maNUpPage.maLeftMarginEdt || pEdit == &maNUpPage.maTopMarginEdt ||
        pEdit == &maNUpPage.maRightMarginEdt || pEdit == &maNUpPage.maBottomMarginEdt ||
        pEdit == &maNUpPage.maHSpaceEdt || pEdit == &maNUpPage.maVSpaceEdt
       )
    {
        updateNup();
    }
    else if( pEdit == &maPageEdit )
    {
        mnCurPage = sal_Int32( maPageEdit.GetValue() - 1 );
        preparePreview( true, true );
    }
    return 0;
}

IMPL_LINK( PrintDialog, UIOptionsChanged, void*, EMPTYARG )
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
        pVal = maPListener->getValue( it->second );
        DBG_ASSERT( pVal, "property value not found" );
    }
    else
    {
        DBG_ERROR( "changed control not in property map" );
    }
    return pVal;
}

IMPL_LINK( PrintDialog, UIOption_CheckHdl, CheckBox*, i_pBox )
{
    PropertyValue* pVal = getValueForWindow( i_pBox );
    if( pVal )
    {
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

void PrintDialog::Paint( const Rectangle& i_rRect )
{
    ModalDialog::Paint( i_rRect );

    #if 0
    // sadly Tab panes are not a reliable choice for a grouping background
    // since they depend on the tab items above in some themes
    if( IsNativeControlSupported( CTRL_TAB_PANE, PART_ENTIRE_CONTROL) )
    {
        Rectangle aPrevBg( maPreviewBackground );
        #ifdef QUARTZ
        // FIXME: this interacts with vcl/aqua/source/gdi/salnativewidgets.cxx where
        // some magic offsets are added to the area
        aPrevBg.Top() += 10;
        #endif
        const ImplControlValue aControlValue( BUTTONVALUE_DONTKNOW, rtl::OUString(), 0 );

        ControlState nState = CTRL_STATE_ENABLED;
        Region aCtrlRegion( aPrevBg );
        DrawNativeControl( CTRL_TAB_PANE, PART_ENTIRE_CONTROL, aCtrlRegion, nState,
                           aControlValue, rtl::OUString() );
    }
    #else
    DecorationView aVw( this );
    aVw.DrawFrame( maPreviewBackground, FRAME_DRAW_IN );
    #endif
}

void PrintDialog::Resize()
{
    Size aPixDiff( LogicToPixel( Size( 5, 5 ), MapMode( MAP_APPFONT ) ) );
    Size aWindowSize( GetOutputSizePixel() );

    // position buttons from lower end, right to left
    Size aBtnSize( maCancelButton.GetSizePixel() );
    Rectangle aBtnRect( Point( aWindowSize.Width() - aPixDiff.Width() - aBtnSize.Width(),
                               aWindowSize.Height() - aPixDiff.Height() - aBtnSize.Height() ),
                        aBtnSize );
    maCancelButton.SetPosSizePixel( aBtnRect.TopLeft(), aBtnRect.GetSize() );
    aBtnSize = maOKButton.GetSizePixel();
    aBtnRect = Rectangle( Point( aBtnRect.Left() - aPixDiff.Width() - aBtnSize.Width(),
                                 aWindowSize.Height() - aPixDiff.Height() - aBtnSize.Height() ),
                          aBtnSize );
    maOKButton.SetPosSizePixel( aBtnRect.TopLeft(), aBtnRect.GetSize() );
    aBtnSize = maButtonLine.GetSizePixel();

    // position fixed line above buttons
    aBtnRect = Rectangle( Point( 0, aBtnRect.Top() - aPixDiff.Width() - aBtnSize.Height()/2 ),
                          Size( aWindowSize.Width(), aBtnSize.Height() ) );
    maButtonLine.SetPosSizePixel( aBtnRect.TopLeft(), aBtnRect.GetSize() );

    // position tab control on upper right
    aBtnSize = maTabCtrl.GetSizePixel();
    aBtnRect = Rectangle( Point( aWindowSize.Width() - aPixDiff.Width() - aBtnSize.Width(),
                                aPixDiff.Height() ),
                          Size( aBtnSize.Width(), maButtonLine.GetPosPixel().Y() - 2*aPixDiff.Height() ) );
    maTabCtrl.SetPosSizePixel( aBtnRect.TopLeft(), aBtnRect.GetSize() );

    // set size for preview
    long nMaxX = maTabCtrl.GetPosPixel().X() - 2*aPixDiff.Width();
    long nMaxY = maButtonLine.GetPosPixel().Y()
                 - 4 * aPixDiff.Height()
                 - maForwardBtn.GetSizePixel().Height();
    long nPreviewLength = std::min( nMaxX, nMaxY );
    maPreviewSpace = Rectangle( Point( aPixDiff.Width(), 2 * aPixDiff.Height() ),
                                Size( nPreviewLength, nPreviewLength ) );

    // position text and slider below preview, aligned
    Size aPrefSize( maPreviewCtrlRow.getOptimalSize( WINDOWSIZE_PREFERRED ) );
    aPrefSize.Width() = nPreviewLength - 2* aPixDiff.Width();
    Point aCtrlPos( 2*aPixDiff.Width(), 3*aPixDiff.Height() + nPreviewLength );
    maPreviewCtrlRow.setManagedArea( Rectangle( aCtrlPos, aPrefSize ) );
    maPreviewBackground.Left() = aPixDiff.Width() - 2;
    maPreviewBackground.Top() = aPixDiff.Height() - 2;
    maPreviewBackground.Right() = aPixDiff.Width() + nPreviewLength + 2;
    maPreviewBackground.Bottom() = maPreviewCtrlRow.getManagedArea().Bottom() + aPixDiff.Height();

    // and do the preview; however the metafile does not need to be gotten anew
    preparePreview( false );

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
        Region aControlRegion( Rectangle( Point(), Size( 100, mnProgressHeight ) ) );
        Region aNativeControlRegion, aNativeContentRegion;
        if( GetNativeControlRegion( CTRL_PROGRESS, PART_ENTIRE_CONTROL, aControlRegion,
                                    CTRL_STATE_ENABLED, aValue, rtl::OUString(),
                                    aNativeControlRegion, aNativeContentRegion ) )
        {
            mnProgressHeight = aNativeControlRegion.GetBoundRect().GetHeight();
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
                        static_cast<USHORT>(0),
                        static_cast<USHORT>(10000*mnCur/mnMax),
                        static_cast<USHORT>(10000/nMaxCount),
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
