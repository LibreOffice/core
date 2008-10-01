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

#include "rtl/ustrbuf.hxx"

using namespace vcl;

PrintDialog::PrintPreviewWindow::PrintPreviewWindow( Window* i_pParent, const ResId& i_rId )
    : Window( i_pParent, i_rId )
{
}

PrintDialog::PrintPreviewWindow::~PrintPreviewWindow()
{
}

void PrintDialog::PrintPreviewWindow::Paint( const Rectangle& i_rRect )
{
    Window::Paint( i_rRect );

    SetFillColor( Color( COL_WHITE ) );
    SetLineColor();
    DrawRect( Rectangle( Point( 0, 0 ), GetSizePixel() ));
    Push();
    SetMapMode( MAP_100TH_MM );
    maMtf.WindStart();
    maMtf.Play( this, Point( 0, 0 ), PixelToLogic( GetSizePixel() ) );
    Pop();
}

void PrintDialog::PrintPreviewWindow::setPreview( const GDIMetaFile& i_rNewPreview )
{
    maMtf = i_rNewPreview;
    Invalidate();
}

PrintDialog::PrinterTabPage::PrinterTabPage( Window* i_pParent, const ResId& rResId )
    : TabPage( i_pParent, rResId )
    , maPrinters( this, VclResId( SV_PRINT_PRINTERS) )
    , maSetupButton( this, VclResId( SV_PRINT_PRT_SETUP ) )
    , maType( this, VclResId( SV_PRINT_PRT_TYPE ) )
    , maTypeText( this, VclResId( SV_PRINT_PRT_TYPE_TXT ) )
    , maStatus( this, VclResId( SV_PRINT_PRT_STATUS ) )
    , maStatusText(this, VclResId( SV_PRINT_PRT_STATUS_TXT ) )
    , maLocation( this, VclResId( SV_PRINT_PRT_LOCATION ) )
    , maLocText( this, VclResId( SV_PRINT_PRT_LOCATION_TXT ) )
    , maComment( this, VclResId( SV_PRINT_PRT_COMMENT ) )
    , maCommentText( this, VclResId( SV_PRINT_PRT_COMMENT_TXT ) )
    , maToFileBox( this, VclResId( SV_PRINT_PRT_TOFILE ) )
{
    FreeResource();
}

PrintDialog::PrinterTabPage::~PrinterTabPage()
{
}

PrintDialog::JobTabPage::JobTabPage( Window* i_pParent, const ResId& rResId )
    : TabPage( i_pParent, rResId )
    , maPrintRange( this, VclResId( SV_PRINT_RANGE ) )
    , maAllButton( this, VclResId( SV_PRINT_ALL ) )
    , maPagesButton( this, VclResId( SV_PRINT_PAGERANGE ) )
    , maSelectionButton( this, VclResId( SV_PRINT_SELECTION ) )
    , maPagesEdit( this, VclResId( SV_PRINT_PAGERANGE_EDIT ) )
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
}

PrintDialog::JobTabPage::~JobTabPage()
{
}

PrintDialog::PrintDialog( Window* i_pParent, const boost::shared_ptr<PrinterListener>& i_rListener )
    : ModalDialog( i_pParent, VclResId( SV_DLG_PRINT ) )
    , maOKButton( this, VclResId( SV_PRINT_OK ) )
    , maCancelButton( this, VclResId( SV_PRINT_CANCEL ) )
    , maPreviewWindow( this, VclResId( SV_PRINT_PAGE_PREVIEW ) )
    , maPageText( this, VclResId( SV_PRINT_PAGE_TXT ) )
    , maPageScrollbar( this, VclResId( SV_PRINT_PAGE_SCROLL ) )
    , maTabCtrl( this, VclResId( SV_PRINT_TABCTRL ) )
    , maPrinterPage( &maTabCtrl, VclResId( SV_PRINT_TAB_PRINTER ) )
    , maJobPage( &maTabCtrl, VclResId( SV_PRINT_TAB_JOB ) )
    , maButtonLine( this, VclResId( SV_PRINT_BUTTONLINE ) )
    , maPListener( i_rListener )
    , mnCurPage( 0 )
    , mnCachedPages( 0 )
{
    FreeResource();

    // insert the tab pages
    maTabCtrl.InsertPage( SV_PRINT_PAGE_PREVIEW, maPrinterPage.GetText() );
    maTabCtrl.SetTabPage( SV_PRINT_PAGE_PREVIEW, &maPrinterPage );
    maTabCtrl.InsertPage( SV_PRINT_TAB_JOB, maJobPage.GetText() );
    maTabCtrl.SetTabPage( SV_PRINT_TAB_JOB, &maJobPage );

    maPageStr = maPageText.GetText();
    // save space for the preview window
    maPreviewSpace = Rectangle( maPreviewWindow.GetPosPixel(), maPreviewWindow.GetSizePixel() );
    // get the first page
    preparePreview();

    // set up the scrollbar for the preview pages
    maPageScrollbar.SetScrollHdl( LINK( this, PrintDialog, ScrollHdl ) );
    maPageScrollbar.SetEndScrollHdl( LINK( this, PrintDialog, ScrollEndHdl ) );
    maPageScrollbar.EnableDrag( TRUE );

    // fill printer listbox
    const std::vector< rtl::OUString >& rQueues( Printer::GetPrinterQueues() );
    for( std::vector< rtl::OUString >::const_iterator it = rQueues.begin();
         it != rQueues.end(); ++it )
    {
        maPrinterPage.maPrinters.InsertEntry( *it );
    }
    // select current printer
    if( maPrinterPage.maPrinters.GetEntryPos( maPListener->getPrinter()->GetName() ) != LISTBOX_ENTRY_NOTFOUND )
        maPrinterPage.maPrinters.SelectEntry( maPListener->getPrinter()->GetName() );
    else
    {
        // fall back to default printer
        maPrinterPage.maPrinters.SelectEntry( Printer::GetDefaultPrinterName() );
        maPListener->setPrinter( boost::shared_ptr<Printer>( new Printer( Printer::GetDefaultPrinterName() ) ) );
    }
    // update the text fields for the printer
    updatePrinterText();

    // set a select handler
    maPrinterPage.maPrinters.SetSelectHdl( LINK( this, PrintDialog, SelectHdl ) );

    // setup page range edit
    rtl::OUStringBuffer aBuf( 16 );
    aBuf.append( sal_Unicode('1') );
    if( mnCachedPages > 1 )
    {
        aBuf.append( sal_Unicode('-') );
        aBuf.append( mnCachedPages );
    }
    maJobPage.maPagesEdit.SetText( aBuf.makeStringAndClear() );

    // setup click handler on the various buttons
    maJobPage.maCollateBox.SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    maJobPage.maAllButton.SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    maJobPage.maSelectionButton.SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    maJobPage.maPagesButton.SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    maPrinterPage.maSetupButton.SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );

    // setup modify hdl
    maJobPage.maCopyCountField.SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );

    // setup dependencies
    checkControlDependencies();
}

PrintDialog::~PrintDialog()
{
}

bool PrintDialog::isPrintToFile()
{
    return maPrinterPage.maToFileBox.IsChecked();
}

int PrintDialog::getCopyCount()
{
    return maJobPage.maCopyCountField.GetValue();
}

bool PrintDialog::isCollate()
{
    return maJobPage.maCopyCountField.GetValue() > 1 ? maJobPage.maCollateBox.IsChecked() : FALSE;
}

MultiSelection PrintDialog::getPageSelection()
{
    if( maJobPage.maPagesButton.IsChecked() )
        return MultiSelection( maJobPage.maPagesEdit.GetText() );
    else if( maJobPage.maAllButton.IsChecked() )
    {
        MultiSelection aSel( Range( 1, maPListener->getPageCount() ) );
        aSel.SelectAll();
        return aSel;
    }
    DBG_ERROR( "NYI: Selection" );
    return MultiSelection();
}

void PrintDialog::checkControlDependencies()
{
    if( maJobPage.maCopyCountField.GetValue() > 1 )
        maJobPage.maCollateBox.Enable( TRUE );
    else
        maJobPage.maCollateBox.Enable( FALSE );

    maJobPage.maPagesEdit.Enable( maJobPage.maPagesButton.IsChecked() );
    Image aImg( maJobPage.maCollateBox.IsChecked() ? maJobPage.maCollateImg : maJobPage.maNoCollateImg );
    if( GetSettings().GetStyleSettings().GetFieldColor().IsDark() )
        aImg = maJobPage.maCollateBox.IsChecked() ? maJobPage.maCollateHCImg : maJobPage.maNoCollateHCImg;

    // adjust position and size of image
    maJobPage.maCollateImage.SetSizePixel( aImg.GetSizePixel() );
    Point aPos( maJobPage.maCollateImage.GetPosPixel() );
    aPos.Y() = maJobPage.maCollateBox.GetPosPixel().Y();
    aPos.Y() -= (aImg.GetSizePixel().Height() - maJobPage.maCollateBox.GetSizePixel().Height())/2;
    maJobPage.maCollateImage.SetPosPixel( aPos );
    maJobPage.maCollateImage.SetImage( aImg );

    // enable setup button only for printers that can be setup
    maPrinterPage.maSetupButton.Enable( maPListener->getPrinter()->HasSupport( SUPPORT_SETUPDIALOG ) );
}

void PrintDialog::updatePrinterText()
{
    const QueueInfo* pInfo = Printer::GetQueueInfo( maPrinterPage.maPrinters.GetSelectEntry(), true );
    if( pInfo )
    {
        maPrinterPage.maTypeText.SetText( pInfo->GetDriver() );
        // FIXME: status message
        // maJobPage.maStatusText.SetText();
        maPrinterPage.maLocText.SetText( pInfo->GetLocation() );
        maPrinterPage.maCommentText.SetText( pInfo->GetComment() );
    }
}

static rtl::OUString searchAndReplace( const rtl::OUString& i_rOrig, const char* i_pRepl, sal_Int32 i_nReplLen, sal_Int32 i_nReplacement )
{
    sal_Int32 nPos = i_rOrig.indexOfAsciiL( i_pRepl, i_nReplLen );
    if( nPos != -1 )
    {
        rtl::OUStringBuffer aBuf( i_rOrig.getLength() );
        aBuf.append( i_rOrig.getStr(), nPos );
        aBuf.append( i_nReplacement );
        if( nPos + i_nReplLen < i_rOrig.getLength() )
            aBuf.append( i_rOrig.getStr() + nPos + i_nReplLen );
        return aBuf.makeStringAndClear();
    }
    return i_rOrig;
}

void PrintDialog::setPreviewText( sal_Int32 nSetPage )
{
    rtl::OUString aNewText( searchAndReplace( maPageStr, "%p", 2, nSetPage+1 ) );
    aNewText = searchAndReplace( aNewText, "%n", 2, mnCachedPages );
    maPageText.SetText( aNewText );
}

void PrintDialog::preparePreview()
{
    sal_Int32 nPages = maPListener->getPageCount();
    mnCachedPages = nPages;

    setPreviewText( mnCurPage );

    maPageScrollbar.SetRange( Range( 0, nPages-1 ) );
    maPageScrollbar.SetThumbPos( mnCurPage );

    boost::shared_ptr<Printer> aPrt( maPListener->getPrinter() );


    const MapMode aMapMode( MAP_100TH_MM );
    Size aPageSize;
    JobSetup aPageSetup( aPrt->GetJobSetup() );
    maPListener->getPageParameters( mnCurPage, aPageSetup, aPageSize );
    ImplUpdateJobSetupPaper( aPageSetup ); // fill in physical paper size
    Size aPaperSize( aPageSetup.ImplGetConstData()->mnPaperWidth,
                     aPageSetup.ImplGetConstData()->mnPaperHeight );
    if( aPageSetup.ImplGetConstData()->meOrientation == ORIENTATION_LANDSCAPE )
    {
        aPaperSize.Width() = aPageSetup.ImplGetConstData()->mnPaperHeight;
        aPaperSize.Height() = aPageSetup.ImplGetConstData()->mnPaperWidth;
    }

    aPrt->Push();
    aPrt->EnableOutput( FALSE );
    aPrt->SetMapMode( aMapMode );

    GDIMetaFile aMtf;
    aMtf.SetPrefSize( aPageSize );
    aMtf.SetPrefMapMode( aMapMode );
    aMtf.Record( &(*aPrt) );

    maPListener->printPage( mnCurPage );

    aMtf.Stop();
    aMtf.WindStart();
    aPrt->Pop();

    Size aPreviewSize;
    Point aPreviewPos = maPreviewSpace.TopLeft();
    const long nW = maPreviewSpace.GetSize().Width();
    const long nH = maPreviewSpace.GetSize().Height();
    if( aPaperSize.Width() > aPaperSize.Height() )
    {
        aPreviewSize = Size( nW, nW * aPaperSize.Height() / aPaperSize.Width() );
        aPreviewPos.Y() += (maPreviewSpace.GetHeight() - aPreviewSize.Height())/2;
    }
    else
    {
        aPreviewSize = Size( nH * aPaperSize.Width() / aPaperSize.Height(), nH );
        aPreviewPos.X() += (maPreviewSpace.GetWidth() - aPreviewSize.Width())/2;
    }
    maPreviewWindow.SetPosSizePixel( aPreviewPos, aPreviewSize );
    const Size aLogicSize( maPreviewWindow.PixelToLogic( maPreviewWindow.GetSizePixel(), MapMode( MAP_100TH_MM ) ) );
    aMtf.Scale( double(aLogicSize.Width())/double(aPaperSize.Width()),
                double(aLogicSize.Height())/double(aPaperSize.Height()) );
    maPreviewWindow.setPreview( aMtf );
}

IMPL_LINK( PrintDialog, ScrollHdl, ScrollBar*, pScrBar )
{
    if( pScrBar == &maPageScrollbar )
    {
        sal_Int32 nNewPage = static_cast<sal_Int32>( maPageScrollbar.GetThumbPos() );
        setPreviewText( nNewPage );
    }
    return 0;
}

IMPL_LINK( PrintDialog, ScrollEndHdl, ScrollBar*, pScrBar )
{
    if( pScrBar == &maPageScrollbar )
    {
        sal_Int32 nNewPage = static_cast<sal_Int32>( maPageScrollbar.GetThumbPos() );
        if( nNewPage != mnCurPage )
        {
            mnCurPage = nNewPage;
            preparePreview();
        }
    }
    return 0;
}

IMPL_LINK( PrintDialog, SelectHdl, ListBox*, pBox )
{
    if( pBox == &maPrinterPage.maPrinters )
    {
        // set new printer
        maPListener->setPrinter( boost::shared_ptr<Printer>( new Printer( maPrinterPage.maPrinters.GetSelectEntry() ) ) );
        // update text fields
        updatePrinterText();
    }
    return 0;
}

IMPL_LINK( PrintDialog, ClickHdl, Button*, pButton )
{
    if( pButton == &maPrinterPage.maSetupButton )
    {
        maPListener->getPrinter()->Setup( this );
    }
    checkControlDependencies();
    return 0;
}

IMPL_LINK( PrintDialog, ModifyHdl, Edit*, EMPTYARG )
{
    checkControlDependencies();
    return 0;
}

