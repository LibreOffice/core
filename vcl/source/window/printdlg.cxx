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

#include "rtl/ustrbuf.hxx"

#include "com/sun/star/awt/Size.hpp"

using namespace vcl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

#define HELPID_PREFIX ".HelpId:vcl:PrintDialog"
#define SMHID2( a, b ) SetSmartHelpId( SmartId( String( RTL_CONSTASCII_USTRINGPARAM( HELPID_PREFIX ":" a ":" b ) ) ) )
#define SMHID1( a ) SetSmartHelpId( SmartId( String( RTL_CONSTASCII_USTRINGPARAM( HELPID_PREFIX  ":" a ) ) ) )

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
    , maNupLine( this, VclResId( SV_PRINT_PRT_NUP ) )
    , maNupRowsTxt( this, VclResId( SV_PRINT_PRT_NUP_ROWS_TXT ) )
    , maNupRowsEdt( this, VclResId( SV_PRINT_PRT_NUP_ROWS_EDT ) )
    , maNupColTxt( this, VclResId( SV_PRINT_PRT_NUP_COLUMNS_TXT ) )
    , maNupColEdt( this, VclResId( SV_PRINT_PRT_NUP_COLUMNS_EDT ) )
    , maNupPortrait( this, VclResId( SV_PRINT_PRT_NUP_PORTRAIT ) )
    , maNupLandscape( this, VclResId( SV_PRINT_PRT_NUP_LANDSCAPE ) )
{
    FreeResource();
    maPrinters.SMHID2( "PrinterPage", "PrinterList" );
    maSetupButton.SMHID2( "PrinterPage", "Setup" );
    maType.SMHID2( "PrinterPage", "Type" );
    maTypeText.SMHID2( "PrinterPage", "TypeText" );
    maStatus.SMHID2( "PrinterPage", "Status" );
    maStatusText.SMHID2( "PrinterPage", "StatusText" );
    maLocation.SMHID2( "PrinterPage", "Locaction" );
    maLocText.SMHID2( "PrinterPage", "LocactionText" );
    maComment.SMHID2( "PrinterPage", "Comment" );
    maCommentText.SMHID2( "PrinterPage", "CommentText" );
    maNupLine.SMHID2( "PrinterPage", "NUPline" );
    maNupRowsTxt.SMHID2( "PrinterPage", "NUPRowsText" );
    maNupRowsEdt.SMHID2( "PrinterPage", "NUPRows" );
    maNupColTxt.SMHID2( "PrinterPage", "NUPColumnsText" );
    maNupColEdt.SMHID2( "PrinterPage", "NUPColumns" );
    maNupPortrait.SMHID2( "PrinterPage", "NUPPortrait" );
    maNupLandscape.SMHID2( "PrinterPage", "NUPLandscape" );
}

PrintDialog::PrinterTabPage::~PrinterTabPage()
{
}

PrintDialog::JobTabPage::JobTabPage( Window* i_pParent, const ResId& rResId )
    : TabPage( i_pParent, rResId )
    , maPrinters( this, VclResId( SV_PRINT_PRINTERS) )
    , maToFileBox( this, VclResId( SV_PRINT_PRT_TOFILE ) )
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
    maPrinters.SMHID2( "JobPage", "PrinterList" );
    maToFileBox.SMHID2( "JobPage", "ToFile" );
    maPrintRange.SMHID2( "JobPage", "PrintRange" );
    maAllButton.SMHID2( "JobPage", "AllButton" );
    maPagesButton.SMHID2( "JobPage", "PagesButton" );
    maSelectionButton.SMHID2( "JobPage", "SelectionButton" );
    maPagesEdit.SMHID2( "JobPage", "Pages" );
    maCopies.SMHID2( "JobPage", "CopiesLine" );
    maCopyCount.SMHID2( "JobPage", "CopiesText" );
    maCopyCountField.SMHID2( "JobPage", "Copies" );
    maCollateBox.SMHID2( "JobPage", "Collate" );
    maCollateImage.SMHID2( "JobPage", "CollateImage" );
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
    , maNoPageStr( String( VclResId( SV_PRINT_NOPAGES ) ) )
    , mnCurPage( 0 )
    , mnCachedPages( 0 )
{
    FreeResource();

    // insert the tab pages
    maTabCtrl.InsertPage( SV_PRINT_TAB_JOB, maJobPage.GetText() );
    maTabCtrl.SetTabPage( SV_PRINT_TAB_JOB, &maJobPage );
    maTabCtrl.InsertPage( SV_PRINT_PAGE_PREVIEW, maPrinterPage.GetText() );
    maTabCtrl.SetTabPage( SV_PRINT_PAGE_PREVIEW, &maPrinterPage );

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
        maJobPage.maPrinters.InsertEntry( *it );
    }
    // select current printer
    if( maPrinterPage.maPrinters.GetEntryPos( maPListener->getPrinter()->GetName() ) != LISTBOX_ENTRY_NOTFOUND )
    {
        maPrinterPage.maPrinters.SelectEntry( maPListener->getPrinter()->GetName() );
        maJobPage.maPrinters.SelectEntry( maPListener->getPrinter()->GetName() );
    }
    else
    {
        // fall back to default printer
        maPrinterPage.maPrinters.SelectEntry( Printer::GetDefaultPrinterName() );
        maJobPage.maPrinters.SelectEntry( Printer::GetDefaultPrinterName() );
        maPListener->setPrinter( boost::shared_ptr<Printer>( new Printer( Printer::GetDefaultPrinterName() ) ) );
    }
    // update the text fields for the printer
    updatePrinterText();

    // set a select handler
    maPrinterPage.maPrinters.SetSelectHdl( LINK( this, PrintDialog, SelectHdl ) );
    maJobPage.maPrinters.SetSelectHdl( LINK( this, PrintDialog, SelectHdl ) );

    // setup page range edit
    rtl::OUStringBuffer aBuf( 16 );
    aBuf.append( sal_Unicode('1') );
    if( mnCachedPages > 1 )
    {
        aBuf.append( sal_Unicode('-') );
        aBuf.append( mnCachedPages );
    }
    maJobPage.maPagesEdit.SetText( aBuf.makeStringAndClear() );

    // setup sizes for N-Up
    Size aNupSize( maPListener->getPrinter()->PixelToLogic(
                         maPListener->getPrinter()->GetPaperSizePixel(), MapMode( MAP_100TH_MM ) ) );
    if( maPListener->getPrinter()->GetOrientation() == ORIENTATION_LANDSCAPE )
    {
        maNupLandscapeSize = aNupSize;
        maNupPortraitSize = Size( aNupSize.Height(), aNupSize.Width() );
        maPrinterPage.maNupLandscape.Check();
    }
    else
    {
        maNupPortraitSize = aNupSize;
        maNupLandscapeSize = Size( aNupSize.Height(), aNupSize.Width() );
        maPrinterPage.maNupPortrait.Check();
    }

    // setup click handler on the various buttons
    maJobPage.maCollateBox.SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    maJobPage.maAllButton.SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    maJobPage.maSelectionButton.SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    maJobPage.maPagesButton.SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    maPrinterPage.maSetupButton.SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    maPrinterPage.maNupPortrait.SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );
    maPrinterPage.maNupLandscape.SetClickHdl( LINK( this, PrintDialog, ClickHdl ) );

    // setup modify hdl
    maJobPage.maCopyCountField.SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    maJobPage.maPagesEdit.SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    maPrinterPage.maNupRowsEdt.SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );
    maPrinterPage.maNupColEdt.SetModifyHdl( LINK( this, PrintDialog, ModifyHdl ) );

    // setup optional UI options set by application
    setupOptionalUI();

    // set change handler for UI options
    maPListener->setOptionChangeHdl( LINK( this, PrintDialog, UIOptionsChanged ) );

    // set min size pixel to current size
    SetMinOutputSizePixel( GetOutputSizePixel() );

    // setup dependencies
    checkControlDependencies();

    // set HelpIDs
    maOKButton.SMHID1( "OK" );
    maCancelButton.SMHID1( "Cancel" );
    maPreviewWindow.SMHID1( "Preview" );
    maPageText.SMHID1( "PageText" );
    maPageScrollbar.SMHID1( "PageScrollbar" );
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
    Window* pCurParent = 0;
    long nCurY = 0, nXPos = 5, nMaxY = 0;
    USHORT nOptPageId = 9, nCurSubGroup = 0;
    MapMode aFontMapMode( MAP_APPFONT );

    Size aTabSize = maTabCtrl.GetTabPageSizePixel();
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
        long nDependencyIndent = 0;
        sal_Int32 nCurHelpText = 0;

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
            else if( rEntry.Name.equalsAscii( "DependsOnName" ) )
            {
                rtl::OUString aDepName;
                rEntry.Value >>= aDepName;
                std::map< rtl::OUString, Window* >::iterator it( maPropertyToWindowMap.find( aDepName ) );
                if( it != maPropertyToWindowMap.end() )
                {
                    Window* pWin = it->second;
                    // still on the same page ?
                    if( pWin->GetParent() == pCurParent )
                    {
                        // is it a labeled window ?
                        if( dynamic_cast< ListBox* >(pWin) ||
                            dynamic_cast< NumericField* >(pWin) )
                        {
                            Window* pLabelWin = pWin->GetLabeledBy();
                            if( dynamic_cast<FixedText*>(pLabelWin) ) // sanity check
                                pWin = pLabelWin;
                        }
                        long nDependencyXPos = PixelToLogic( pWin->GetPosPixel(), aFontMapMode ).X();
                        if( (nDependencyXPos + 5)  > nXPos )
                            nDependencyIndent = nDependencyXPos + 5 - nXPos;
                    }
                }
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
            aCtrlType.equalsAscii( "Subgroup" ) ||
            aCtrlType.equalsAscii( "Radio" ) ||
            aCtrlType.equalsAscii( "List" )  ||
            aCtrlType.equalsAscii( "Range" )  ||
            aCtrlType.equalsAscii( "Bool" ) )
        {
            if( aCtrlType.equalsAscii( "Group" ) || ! pCurParent )
            {
                if( nCurY > nMaxY )
                    nMaxY = nCurY;

                // add new tab page
                TabPage* pNewGroup = new TabPage( &maTabCtrl );
                maControls.push_front( pNewGroup );
                pCurParent = pNewGroup;
                nCurY = 5;
                nXPos = 5;
                pNewGroup->SetText( aText );
                maTabCtrl.InsertPage( ++nOptPageId, aText );
                maTabCtrl.SetTabPage( nOptPageId, pNewGroup );

                // set help id
                setSmartId( pNewGroup, "TabPage", nOptPageId );
                // set help text
                setHelpText( pNewGroup, aHelpTexts, 0 );

                // reset subgroup counter
                nCurSubGroup = 0;
            }

            if( aCtrlType.equalsAscii( "Subgroup" ) && pCurParent )
            {
                nXPos = 5;
                FixedLine* pNewSub = new FixedLine( pCurParent );
                maControls.push_front( pNewSub );
                pNewSub->SetText( aText );
                nCurY += 4;
                Size aPixelSize( aTabSize );
                aPixelSize.Width() /= 2;
                aPixelSize.Height() = pCurParent->GetTextHeight() + 4;
                pNewSub->SetPosSizePixel( pNewSub->LogicToPixel( Point( nXPos, nCurY ), aFontMapMode ),
                                          aPixelSize );
                pNewSub->Show();
                nCurY += 12;
                nXPos += 5;

                // set help id
                setSmartId( pNewSub, "FixedLine", sal_Int32( nCurSubGroup++ ) );
                // set help text
                setHelpText( pNewSub, aHelpTexts, 0 );
            }
            else if( aCtrlType.equalsAscii( "Bool" ) && pCurParent )
            {
                // add a check box
                CheckBox* pNewBox = new CheckBox( pCurParent );
                maControls.push_front( pNewBox );
                pNewBox->SetText( aText );

                // FIXME: measure text
                pNewBox->SetPosSizePixel( pNewBox->LogicToPixel( Point( nXPos + nDependencyIndent, nCurY ), aFontMapMode ),
                                          pNewBox->LogicToPixel( Size( 100, 10 ), aFontMapMode ) );
                nCurY += 12;

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
            }
            else if( aCtrlType.equalsAscii( "Radio" ) && pCurParent )
            {
                long nOldXPos = nXPos;
                if( aText.getLength() )
                {
                    // add a FixedText:
                    FixedText* pHeading = new FixedText( pCurParent );
                    maControls.push_front( pHeading );
                    pHeading->SetText( aText );
                    Size aPixelSize( pHeading->LogicToPixel( Size( 10, 10 ), aFontMapMode ) );
                    aPixelSize.Width() = aTabSize.Width() - aPixelSize.Width();
                    pHeading->SetPosSizePixel( pHeading->LogicToPixel( Point( nXPos + nDependencyIndent, nCurY ), aFontMapMode ),
                                               aPixelSize );
                    pHeading->Show();

                    nXPos += 10;
                    nCurY += 12;

                    // set help id
                    setSmartId( pHeading, "FixedText", -1, aPropertyName );
                    // set help text
                    setHelpText( pHeading, aHelpTexts, nCurHelpText++ );
                }

                // iterate options
                sal_Int32 nSelectVal = 0;
                PropertyValue* pVal = maPListener->getValue( aPropertyName );
                if( pVal && pVal->Value.hasValue() )
                    pVal->Value >>= nSelectVal;
                for( sal_Int32 m = 0; m < aChoices.getLength(); m++ )
                {
                    RadioButton* pBtn = new RadioButton( pCurParent, m == 0 ? WB_GROUP : 0 );
                    maControls.push_front( pBtn );
                    pBtn->SetText( aChoices[m] );
                    pBtn->Check( m == nSelectVal );
                    Size aPixelSize( pBtn->LogicToPixel( Size( 10 + nXPos + nDependencyIndent, 12 ), aFontMapMode ) );
                    aPixelSize.Width() = aTabSize.Width() - aPixelSize.Width();
                    pBtn->SetPosSizePixel( pBtn->LogicToPixel( Point( 15, nCurY ), aFontMapMode ),
                                           aPixelSize );
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

                    nCurY += 12;
                }
                nXPos = nOldXPos;
            }
            else if( aCtrlType.equalsAscii( "List" ) && pCurParent )
            {
                // add a FixedText:
                FixedText* pHeading = new FixedText( pCurParent );
                maControls.push_front( pHeading );
                pHeading->SetText( aText );
                Size aPixelSize( pHeading->LogicToPixel( Size( 10, 10 ), aFontMapMode ) );
                aPixelSize.Width() += pHeading->GetTextWidth( aText );
                pHeading->SetPosSizePixel( pHeading->LogicToPixel( Point( nXPos + nDependencyIndent, nCurY ), aFontMapMode ),
                                           aPixelSize );
                pHeading->Show();

                // set help id
                setSmartId( pHeading, "FixedText", -1, aPropertyName );

                ListBox* pList = new ListBox( pCurParent, WB_DROPDOWN | WB_BORDER );
                maControls.push_front( pList );

                // iterate options
                long nMaxTextWidth = 0;
                for( sal_Int32 m = 0; m < aChoices.getLength(); m++ )
                {
                    pList->InsertEntry( aChoices[m] );
                    long nEntryWidth = pList->GetTextWidth( aChoices[m] );
                    if( nEntryWidth > nMaxTextWidth )
                        nMaxTextWidth = nEntryWidth;
                }
                nMaxTextWidth += 50;
                sal_Int32 nSelectVal = 0;
                PropertyValue* pVal = maPListener->getValue( aPropertyName );
                if( pVal && pVal->Value.hasValue() )
                    pVal->Value >>= nSelectVal;
                pList->SelectEntryPos( static_cast<USHORT>(nSelectVal) );

                aPixelSize = Size( pList->LogicToPixel( Size( 25, 12 ), aFontMapMode ) );
                aPixelSize.Width() = nMaxTextWidth;
                aPixelSize.Height() *= aChoices.getLength() > 15 ? 15 : aChoices.getLength();

                Point aListPos;
                bool bDoAlign = false;
                if( nMaxTextWidth + aPixelSize.Width() < aTabSize.Width() - 10 )
                {
                    aListPos      = pHeading->GetPosPixel();
                    aListPos.X() += pHeading->GetSizePixel().Width() + 5;

                    // align heading and list box
                    bDoAlign = true;
                }
                else
                {
                    nCurY += 12;
                    aListPos = pCurParent->LogicToPixel( Point( 15 + nDependencyIndent, nCurY ), aFontMapMode );
                }

                pList->SetPosSizePixel( aListPos, aPixelSize );
                pList->Enable( maPListener->isUIOptionEnabled( aPropertyName ) );
                pList->SetSelectHdl( LINK( this, PrintDialog, UIOption_SelectHdl ) );
                pList->Show();

                // set help id
                setSmartId( pList, "ListBox", -1, aPropertyName );
                // set help text
                setHelpText( pList, aHelpTexts, 0 );

                maPropertyToWindowMap.insert( std::pair< rtl::OUString, Window* >( aPropertyName, pList ) );
                maControlToPropertyMap[pList] = aPropertyName;
                nCurY += 16;

                if( bDoAlign )
                {
                    Point aPos = pHeading->GetPosPixel();
                    Size aSize = pHeading->GetSizePixel();
                    aPos.Y() += (pList->GetSizePixel().Height() - aSize.Height())/2;
                    pHeading->SetPosSizePixel( aPos, aSize );
                }
            }
            else if( aCtrlType.equalsAscii( "Range" ) && pCurParent )
            {
                // add a FixedText:
                FixedText* pHeading = new FixedText( pCurParent );
                maControls.push_front( pHeading );
                pHeading->SetText( aText );
                Size aPixelSize( pHeading->LogicToPixel( Size( 10, 10 ), aFontMapMode ) );
                aPixelSize.Width() += pHeading->GetTextWidth( aText );
                pHeading->SetPosSizePixel( pHeading->LogicToPixel( Point( nXPos + nDependencyIndent, nCurY ), aFontMapMode ),
                                           aPixelSize );
                pHeading->Show();

                // set help id
                setSmartId( pHeading, "FixedText", -1, aPropertyName );

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

                aPixelSize = Size( pField->LogicToPixel( Size( 80, 12 ), aFontMapMode ) );

                Point aFieldPos;
                bool bDoAlign = false;
                if( aPixelSize.Width() < aTabSize.Width() - 10 )
                {
                    aFieldPos      = pHeading->GetPosPixel();
                    aFieldPos.X() += pHeading->GetSizePixel().Width() + 5;

                    // align heading and list box
                    bDoAlign = true;
                }
                else
                {
                    nCurY += 12;
                    aFieldPos = pCurParent->LogicToPixel( Point( 15 + nDependencyIndent, nCurY ), aFontMapMode );
                }

                pField->SetPosSizePixel( aFieldPos, aPixelSize );
                pField->Enable( maPListener->isUIOptionEnabled( aPropertyName ) );
                pField->SetModifyHdl( LINK( this, PrintDialog, UIOption_ModifyHdl ) );
                pField->Show();

                // set help id
                setSmartId( pField, "NumericField", -1, aPropertyName );
                // set help text
                setHelpText( pField, aHelpTexts, 0 );

                maPropertyToWindowMap.insert( std::pair< rtl::OUString, Window* >( aPropertyName, pField ) );
                maControlToPropertyMap[pField] = aPropertyName;
                nCurY += 16;

                if( bDoAlign )
                {
                    Point aPos = pHeading->GetPosPixel();
                    Size aSize = pHeading->GetSizePixel();
                    aPos.Y() += (pField->GetSizePixel().Height() - aSize.Height())/2;
                    pHeading->SetPosSizePixel( aPos, aSize );
                }
            }
        }
        else
        {
            DBG_ERROR( "Unsupported UI option" );
        }
    }

    if( nCurY > nMaxY )
        nMaxY = nCurY;

    // resize dialog if necessary
    Size aMaxSize( LogicToPixel( Size( nMaxY, nMaxY ), aFontMapMode ) );
    if( aMaxSize.Height() > aTabSize.Height() )
    {
        Size aCurSize( GetSizePixel() );
        aCurSize.Height() += aMaxSize.Height() - aTabSize.Height();
        SetSizePixel( aCurSize );
    }
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
    if( mnCachedPages != 0 )
    {
        rtl::OUString aNewText( searchAndReplace( maPageStr, "%p", 2, nSetPage+1 ) );
        aNewText = searchAndReplace( aNewText, "%n", 2, mnCachedPages );
        maPageText.SetText( aNewText );
    }
    else
        maPageText.SetText( maNoPageStr );
}

void PrintDialog::preparePreview( bool i_bNewPage )
{
    // page range may have changed depending on options
    sal_Int32 nPages = maPListener->getFilteredPageCount();
    mnCachedPages = nPages;

    if( mnCurPage >= nPages )
        mnCurPage = nPages-1;
    if( mnCurPage < 0 )
        mnCurPage = 0;

    setPreviewText( mnCurPage );

    maPageScrollbar.SetRange( Range( 0, nPages ) );
    maPageScrollbar.SetThumbPos( mnCurPage );
    maPageScrollbar.SetVisibleSize( 1 );

    boost::shared_ptr<Printer> aPrt( maPListener->getPrinter() );


    if( i_bNewPage )
    {
        const MapMode aMapMode( MAP_100TH_MM );
        GDIMetaFile aMtf;
        if( nPages > 0 )
            maCurPageSize = maPListener->getFilteredPageFile( mnCurPage, aMtf );

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
    int nRows = int(maPrinterPage.maNupRowsEdt.GetValue());
    int nCols = int(maPrinterPage.maNupColEdt.GetValue());

    maPListener->setMultipage( nRows, nCols,
                               maPrinterPage.maNupPortrait.IsChecked()
                               ? maNupPortraitSize : maNupLandscapeSize );

    preparePreview();
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
    if( pBox == &maPrinterPage.maPrinters || pBox == &maJobPage.maPrinters )
    {
        String aNewPrinter( pBox->GetSelectEntry() );
        maJobPage.maPrinters.SelectEntry( aNewPrinter );
        maPrinterPage.maPrinters.SelectEntry( aNewPrinter );
        // set new printer
        maPListener->setPrinter( boost::shared_ptr<Printer>( new Printer( aNewPrinter ) ) );
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
    if( ( pButton == &maJobPage.maAllButton        ||
          pButton == &maJobPage.maPagesButton      ||
          pButton == &maJobPage.maSelectionButton )
       && ((RadioButton*)pButton)->IsChecked() )
    {
        if( pButton == &maJobPage.maAllButton )
            maPListener->setPrintSelection( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "all" ) ) );
        else if( pButton == &maJobPage.maPagesButton )
            maPListener->setPrintSelection( maJobPage.maPagesEdit.GetText() );
        else
            maPListener->setPrintSelection( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "selection" ) ) );

        preparePreview();
    }
    if( pButton == &maPrinterPage.maNupPortrait || pButton == &maPrinterPage.maNupLandscape )
        updateNup();
    return 0;
}

IMPL_LINK( PrintDialog, ModifyHdl, Edit*, pEdit )
{
    checkControlDependencies();
    if( pEdit == &maJobPage.maPagesEdit && maJobPage.maPagesButton.IsChecked() )
    {
        maPListener->setPrintSelection( maJobPage.maPagesEdit.GetText() );
        preparePreview();
    }
    else if( pEdit == &maPrinterPage.maNupRowsEdt || pEdit == &maPrinterPage.maNupColEdt )
    {
        updateNup();
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
                 - 2 * aPixDiff.Height()
                 - maPageText.GetSizePixel().Height()
                 - maPageScrollbar.GetSizePixel().Height();
    long nPreviewLength = std::min( nMaxX, nMaxY );
    maPreviewSpace = Rectangle( Point( aPixDiff.Width(), aPixDiff.Height() ),
                                Size( nPreviewLength, nPreviewLength ) );

    // position text and scrollbar below preview
    aBtnRect = Rectangle( Point( aPixDiff.Width(), 2*aPixDiff.Height() + nPreviewLength ),
                          Size( nPreviewLength, maPageScrollbar.GetSizePixel().Height() ) );
    maPageScrollbar.SetPosSizePixel( aBtnRect.TopLeft(), aBtnRect.GetSize() );

    aBtnRect.Top() = aBtnRect.Bottom() + aPixDiff.Height()/2;
    aBtnRect.Bottom() = aBtnRect.Top() + maPageText.GetSizePixel().Height() - 1;
    maPageText.SetPosSizePixel( aBtnRect.TopLeft(), aBtnRect.GetSize() );

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

    rtl::OUString aNewText( searchAndReplace( maStr, "%p", 2, mnCur ) );
    aNewText = searchAndReplace( aNewText, "%n", 2, mnMax );
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
