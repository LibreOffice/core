/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pdfwriter_impl.cxx,v $
 *
 *  $Revision: 1.95 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 17:30:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>

#include <pdfwriter_impl.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/debug.hxx>
#include <tools/zcodec.hxx>
#include <tools/stream.hxx>
#include <virdev.hxx>
#include <bmpacc.hxx>
#include <bitmapex.hxx>
#include <image.hxx>
#include <outdev.h>
#include <sallayout.hxx>
#include <metric.hxx>
#include <svsys.h>
#include <salgdi.hxx>
#include <svapp.hxx>
#include <osl/thread.h>
#include <osl/file.h>
#include <rtl/crc.h>
#include <rtl/digest.h>

#include "implncvt.hxx"


using namespace vcl;
using namespace rtl;

#if OSL_DEBUG_LEVEL < 2
#define COMPRESS_PAGES
#endif

#ifdef DO_TEST_PDF
void doTestCode()
{
    static const char* pHome = getenv( "HOME"  );
    rtl::OUString aTestFile( RTL_CONSTASCII_USTRINGPARAM( "file://" ) );
    aTestFile += rtl::OUString( pHome, strlen( pHome ), RTL_TEXTENCODING_MS_1252 );
    aTestFile += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/pdf_export_test.pdf" ) );

    PDFWriter::PDFWriterContext aContext;
    aContext.URL            = aTestFile;
    aContext.Version        = PDFWriter::PDF_1_4;
    aContext.Tagged         = true;

    PDFWriter aWriter( aContext );
    aWriter.NewPage();
    // set duration of 3 sec for first page
    aWriter.SetAutoAdvanceTime( 3 );
    aWriter.SetMapMode( MapMode( MAP_100TH_MM ) );

    aWriter.SetFillColor( Color( COL_LIGHTRED ) );
    aWriter.SetLineColor( Color( COL_LIGHTGREEN ) );
    aWriter.DrawRect( Rectangle( Point( 2000, 200 ), Size( 8000, 3000 ) ), 5000, 2000 );

    aWriter.SetFont( Font( String( RTL_CONSTASCII_USTRINGPARAM( "Times" ) ), Size( 0, 500 ) ) );
    aWriter.SetTextColor( Color( COL_BLACK ) );
    aWriter.SetLineColor( Color( COL_BLACK ) );
    aWriter.SetFillColor( Color( COL_LIGHTBLUE ) );

    Rectangle aRect( Point( 5000, 5000 ), Size( 6000, 3000 ) );
    aWriter.DrawRect( aRect );
    aWriter.DrawText( aRect, String( RTL_CONSTASCII_USTRINGPARAM( "Link annot 1" ) ) );
    sal_Int32 nFirstLink = aWriter.CreateLink( aRect );
    PDFNote aNote;
    aNote.Title = String( RTL_CONSTASCII_USTRINGPARAM( "A small test note" ) );
    aNote.Contents = String( RTL_CONSTASCII_USTRINGPARAM( "There is no business like show business like no business i know. Everything about it is appealing." ) );
    aWriter.CreateNote( Rectangle( Point( aRect.Right(), aRect.Top() ), Size( 6000, 3000 ) ), aNote );

    Rectangle aTargetRect( Point( 3000, 23000 ), Size( 12000, 6000 ) );
    aWriter.SetFillColor( Color( COL_LIGHTGREEN ) );
    aWriter.DrawRect( aTargetRect );
    aWriter.DrawText( aTargetRect, String( RTL_CONSTASCII_USTRINGPARAM( "Dest second link" ) ) );
    sal_Int32 nSecondDest = aWriter.CreateDest( aTargetRect );

    aWriter.BeginStructureElement( PDFWriter::Section );
    aWriter.BeginStructureElement( PDFWriter::Heading );
    aWriter.DrawText( Point(4500, 9000), String( RTL_CONSTASCII_USTRINGPARAM( "A small structure test" ) ) );
    aWriter.EndStructureElement();
    aWriter.BeginStructureElement( PDFWriter::Paragraph );
    aWriter.SetStructureAttribute( PDFWriter::WritingMode, PDFWriter::LrTb );
    aWriter.SetStructureAttribute( PDFWriter::TextDecorationType, PDFWriter::Underline );
    aWriter.DrawText( Rectangle( Point( 4500, 10000 ), Size( 12000, 6000 ) ),
                      String( RTL_CONSTASCII_USTRINGPARAM( "It was the best of PDF, it was the worst of PDF ... or so. This is a pretty nonsensical text to denote a paragraph. I suggest you stop reading it. Because if you read on you might get bored. So continue on your on risk. Hey, you're still here ? Why do you continue to read this as it is of no use at all ? OK, it's your time, but still... . Woah, i even get bored writing this, so let's end this here and now." ) ),
                      TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK
                      );
    aWriter.SetActualText( String( RTL_CONSTASCII_USTRINGPARAM( "It was the best of PDF, it was the worst of PDF ... or so. This is a pretty nonsensical text to denote a paragraph. I suggest you stop reading it. Because if you read on you might get bored. So continue on your on risk. Hey, you're still here ? Why do you continue to read this as it is of no use at all ? OK, it's your time, but still... . Woah, i even get bored writing this, so let's end this here and now." ) ) );
    aWriter.SetAlternateText( String( RTL_CONSTASCII_USTRINGPARAM( "This paragraph contains some lengthy nonsense to test structural element emission of PDFWriter." ) ) );
    aWriter.EndStructureElement();
    sal_Int32 nLongPara = aWriter.BeginStructureElement( PDFWriter::Paragraph );
    aWriter.SetStructureAttribute( PDFWriter::WritingMode, PDFWriter::LrTb );
    aWriter.DrawText( Rectangle( Point( 4500, 19000 ), Size( 12000, 1000 ) ),
                      String( RTL_CONSTASCII_USTRINGPARAM( "This paragraph is nothing special either but ends on the next page structurewise" ) ),
                      TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK
                      );

    aWriter.NewPage();
    // set transitional mode
    aWriter.SetPageTransition( PDFWriter::WipeRightToLeft, 1500 );
    aWriter.SetMapMode( MapMode( MAP_100TH_MM ) );
    aWriter.SetTextColor( Color( COL_BLACK ) );
    aWriter.SetFont( Font( String( RTL_CONSTASCII_USTRINGPARAM( "Times" ) ), Size( 0, 500 ) ) );
    aWriter.DrawText( Rectangle( Point( 4500, 1500 ), Size( 12000, 3000 ) ),
                      String( RTL_CONSTASCII_USTRINGPARAM( "Here's where all things come to an end ... well at least the paragaph from the last page." ) ),
                      TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK
                      );
    aWriter.EndStructureElement();

    aWriter.SetFillColor( Color( COL_LIGHTBLUE ) );
    // disable structure
    aWriter.BeginStructureElement( PDFWriter::NonStructElement );
    aWriter.DrawRect( aRect );
    aWriter.BeginStructureElement( PDFWriter::Paragraph );
    aWriter.DrawText( aRect, String( RTL_CONSTASCII_USTRINGPARAM( "Link annot 2" ) ) );
    sal_Int32 nSecondLink = aWriter.CreateLink( aRect );

    aWriter.SetFillColor( Color( COL_LIGHTGREEN ) );
    aWriter.BeginStructureElement( PDFWriter::ListItem );
    aWriter.DrawRect( aTargetRect );
    aWriter.DrawText( aTargetRect, String( RTL_CONSTASCII_USTRINGPARAM( "Dest first link" ) ) );
    sal_Int32 nFirstDest = aWriter.CreateDest( aTargetRect );
    // enable structure
    aWriter.EndStructureElement();
    // add something to the long paragraph as an afterthought
    sal_Int32 nSaveStruct = aWriter.GetCurrentStructureElement();
    aWriter.SetCurrentStructureElement( nLongPara );
    aWriter.DrawText( Rectangle( Point( 4500,4500 ),  Size( 12000, 1000 ) ),
                      String( RTL_CONSTASCII_USTRINGPARAM( "Add something to the longish paragraph above." ) ),
                      TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
    aWriter.SetCurrentStructureElement( nSaveStruct );
    aWriter.EndStructureElement();
    aWriter.EndStructureElement();
    aWriter.BeginStructureElement( PDFWriter::Figure );
    aWriter.BeginStructureElement( PDFWriter::Caption );
    aWriter.DrawText( Point( 4500, 9000 ), String( RTL_CONSTASCII_USTRINGPARAM( "Some drawing stuff inside the structure" ) ) );
    aWriter.EndStructureElement();
    aWriter.DrawEllipse( Rectangle( Point( 4500, 9600 ), Size( 12000, 3000 ) ) );
    // test transparency
    // draw background
    Rectangle aTranspRect( Point( 7500, 13500 ), Size( 9000, 6000 ) );
    aWriter.SetFillColor( Color( COL_LIGHTRED ) );
    aWriter.DrawRect( aTranspRect );
    aWriter.BeginTransparencyGroup();

    aWriter.SetFillColor( Color( COL_LIGHTGREEN ) );
    aWriter.DrawEllipse( aTranspRect );
    aWriter.SetTextColor( Color( COL_LIGHTBLUE ) );
    aWriter.DrawText( aTranspRect,
                      String( RTL_CONSTASCII_USTRINGPARAM( "Some transparent text" ) ),
                      TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );

    aWriter.EndTransparencyGroup( aTranspRect, 50 );

    // prepare an alpha mask
    Bitmap aTransMask( Size( 256, 256 ), 8, &Bitmap::GetGreyPalette( 256 ) );
    BitmapWriteAccess* pAcc = aTransMask.AcquireWriteAccess();
    for( int nX = 0; nX < 256; nX++ )
        for( int nY = 0; nY < 256; nY++ )
            pAcc->SetPixel( nX, nY, BitmapColor( (BYTE)((nX+nY)/2) ) );
    aTransMask.ReleaseAccess( pAcc );

    aWriter.DrawBitmap( Point( 600, 13500 ), Size( 3000, 3000 ), aTransMask );

    aTranspRect = Rectangle( Point( 4200, 13500 ), Size( 3000, 3000 ) );
    aWriter.SetFillColor( Color( COL_LIGHTRED ) );
    aWriter.DrawRect( aTranspRect );
    aWriter.SetFillColor( Color( COL_LIGHTGREEN ) );
    aWriter.DrawEllipse( aTranspRect );
    aWriter.SetTextColor( Color( COL_LIGHTBLUE ) );
    aWriter.DrawText( aTranspRect,
                      String( RTL_CONSTASCII_USTRINGPARAM( "Some transparent text" ) ),
                      TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
    aTranspRect = Rectangle( Point( 1500, 16500 ), Size( 4800, 3000 ) );
    aWriter.SetFillColor( Color( COL_LIGHTRED ) );
    aWriter.DrawRect( aTranspRect );
    aWriter.BeginTransparencyGroup();
    aWriter.SetFillColor( Color( COL_LIGHTGREEN ) );
    aWriter.DrawEllipse( aTranspRect );
    aWriter.SetTextColor( Color( COL_LIGHTBLUE ) );
    aWriter.DrawText( aTranspRect,
                      String( RTL_CONSTASCII_USTRINGPARAM( "Some transparent text" ) ),
                      TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
    aWriter.EndTransparencyGroup( aTranspRect, aTransMask );

    Bitmap aImageBmp( Size( 256, 256 ), 24 );
    pAcc = aImageBmp.AcquireWriteAccess();
    pAcc->SetFillColor( Color( 0xff, 0, 0xff ) );
    pAcc->FillRect( Rectangle( Point( 0, 0 ), Size( 256, 256 ) ) );
    aImageBmp.ReleaseAccess( pAcc );
    BitmapEx aBmpEx( aImageBmp, AlphaMask( aTransMask ) );
    aWriter.DrawBitmapEx( Point( 1500, 19500 ), Size( 4800, 3000 ), aBmpEx );


    aWriter.EndStructureElement();
    aWriter.EndStructureElement();

    LineInfo aLI( LINE_DASH, 3 );
    aLI.SetDashCount( 2 );
    aLI.SetDashLen( 50 );
    aLI.SetDotCount( 2 );
    aLI.SetDotLen( 25 );
    aLI.SetDistance( 15 );
    Point aLIPoints[] = { Point( 4000, 10000 ),
                          Point( 8000, 12000 ),
                          Point( 3000, 19000 ) };
    Polygon aLIPoly( 3, aLIPoints );
    aWriter.SetLineColor( Color( COL_BLUE ) );
    aWriter.SetFillColor();
    aWriter.DrawPolyLine( aLIPoly, aLI );

    aLI.SetDashCount( 4 );
    aLIPoly.Move( 1000, 1000 );
    aWriter.DrawPolyLine( aLIPoly, aLI );

    aWriter.NewPage();
    aWriter.SetMapMode( MapMode( MAP_100TH_MM ) );
    aWriter.SetFont( Font( String( RTL_CONSTASCII_USTRINGPARAM( "Times" ) ), Size( 0, 500 ) ) );
    aWriter.SetTextColor( Color( COL_BLACK ) );
    aRect = Rectangle( Point( 4500, 6000 ), Size( 6000, 1500 ) );
    aWriter.DrawRect( aRect );
    aWriter.DrawText( aRect, String( RTL_CONSTASCII_USTRINGPARAM( "www.heise.de" ) ) );
    sal_Int32 nURILink = aWriter.CreateLink( aRect );
    aWriter.SetLinkURL( nURILink, OUString( RTL_CONSTASCII_USTRINGPARAM( "http://www.heise.de" ) ) );

    aWriter.SetLinkDest( nFirstLink, nFirstDest );
    aWriter.SetLinkDest( nSecondLink, nSecondDest );

    // include a button
    PDFWriter::PushButtonWidget aBtn;
    aBtn.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "testButton" ) );
    aBtn.Description = OUString( RTL_CONSTASCII_USTRINGPARAM( "A test button" ) );
    aBtn.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "hit me" ) );
    aBtn.Location = Rectangle( Point( 4500, 9000 ), Size( 4500, 3000 ) );
    aBtn.Border = aBtn.Background = true;
    aWriter.CreateControl( aBtn );

    // include a uri button
    PDFWriter::PushButtonWidget aUriBtn;
    aUriBtn.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "wwwButton" ) );
    aUriBtn.Description = OUString( RTL_CONSTASCII_USTRINGPARAM( "A URI button" ) );
    aUriBtn.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "to www" ) );
    aUriBtn.Location = Rectangle( Point( 9500, 9000 ), Size( 4500, 3000 ) );
    aUriBtn.Border = aUriBtn.Background = true;
    aUriBtn.URL = OUString( RTL_CONSTASCII_USTRINGPARAM( "http://www.heise.de" ) );
    aWriter.CreateControl( aUriBtn );

    // include a dest button
    PDFWriter::PushButtonWidget aDstBtn;
    aDstBtn.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "destButton" ) );
    aDstBtn.Description = OUString( RTL_CONSTASCII_USTRINGPARAM( "A Dest button" ) );
    aDstBtn.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "to paragraph" ) );
    aDstBtn.Location = Rectangle( Point( 14500, 9000 ), Size( 4500, 3000 ) );
    aDstBtn.Border = aDstBtn.Background = true;
    aDstBtn.Dest = nFirstDest;
    aWriter.CreateControl( aDstBtn );

    PDFWriter::CheckBoxWidget aCBox;
    aCBox.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "textCheckBox" ) );
    aCBox.Description = OUString( RTL_CONSTASCII_USTRINGPARAM( "A test check box" ) );
    aCBox.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "check me" ) );
    aCBox.Location = Rectangle( Point( 4500, 13500 ), Size( 3000, 750 ) );
    aCBox.Checked = true;
    aCBox.Border = aCBox.Background = false;
    aWriter.CreateControl( aCBox );

    PDFWriter::CheckBoxWidget aCBox2;
    aCBox2.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "textCheckBox2" ) );
    aCBox2.Description = OUString( RTL_CONSTASCII_USTRINGPARAM( "Another test check box" ) );
    aCBox2.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "check me right" ) );
    aCBox2.Location = Rectangle( Point( 4500, 14250 ), Size( 3000, 750 ) );
    aCBox2.Checked = true;
    aCBox2.Border = aCBox2.Background = false;
    aCBox2.ButtonIsLeft = false;
    aWriter.CreateControl( aCBox2 );

    PDFWriter::RadioButtonWidget aRB1;
    aRB1.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "rb1_1" ) );
    aRB1.Description = OUString( RTL_CONSTASCII_USTRINGPARAM( "radio 1 button 1" ) );
    aRB1.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "Despair" ) );
    aRB1.Location = Rectangle( Point( 4500, 15000 ), Size( 6000, 1000 ) );
    aRB1.Selected = true;
    aRB1.RadioGroup = 1;
    aRB1.Border = aRB1.Background = true;
    aRB1.ButtonIsLeft = false;
    aRB1.BorderColor = Color( COL_LIGHTGREEN );
    aRB1.BackgroundColor = Color( COL_LIGHTBLUE );
    aRB1.TextColor = Color( COL_LIGHTRED );
    aRB1.TextFont = Font( String( RTL_CONSTASCII_USTRINGPARAM( "Courier" ) ), Size( 0, 800 ) );
    aWriter.CreateControl( aRB1 );

    PDFWriter::RadioButtonWidget aRB2;
    aRB2.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "rb2_1" ) );
    aRB2.Description = OUString( RTL_CONSTASCII_USTRINGPARAM( "radio 2 button 1" ) );
    aRB2.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "Joy" ) );
    aRB2.Location = Rectangle( Point( 10500, 15000 ), Size( 3000, 1000 ) );
    aRB2.Selected = true;
    aRB2.RadioGroup = 2;
    aWriter.CreateControl( aRB2 );

    PDFWriter::RadioButtonWidget aRB3;
    aRB3.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "rb1_2" ) );
    aRB3.Description = OUString( RTL_CONSTASCII_USTRINGPARAM( "radio 1 button 2" ) );
    aRB3.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "Desperation" ) );
    aRB3.Location = Rectangle( Point( 4500, 16000 ), Size( 3000, 1000 ) );
    aRB3.Selected = true;
    aRB3.RadioGroup = 1;
    aWriter.CreateControl( aRB3 );

    PDFWriter::EditWidget aEditBox;
    aEditBox.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "testEdit" ) );
    aEditBox.Description = OUString( RTL_CONSTASCII_USTRINGPARAM( "A test edit field" ) );
    aEditBox.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "A little test text" ) );
    aEditBox.TextStyle = TEXT_DRAW_LEFT | TEXT_DRAW_VCENTER;
    aEditBox.Location = Rectangle( Point( 10000, 18000 ), Size( 5000, 1500 ) );
    aEditBox.MaxLen = 100;
    aEditBox.Border = aEditBox.Background = true;
    aEditBox.BorderColor = Color( COL_BLACK );
    aWriter.CreateControl( aEditBox );

    // normal list box
    PDFWriter::ListBoxWidget aLstBox;
    aLstBox.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "testListBox" ) );
    aLstBox.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "One" ) );
    aLstBox.Description = OUString( RTL_CONSTASCII_USTRINGPARAM( "select me" ) );
    aLstBox.Location = Rectangle( Point( 4500, 18000 ), Size( 3000, 1500 ) );
    aLstBox.Sort = true;
    aLstBox.MultiSelect = true;
    aLstBox.Border = aLstBox.Background = true;
    aLstBox.BorderColor = Color( COL_BLACK );
    aLstBox.Entries.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "One" ) ) );
    aLstBox.Entries.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "Two" ) ) );
    aLstBox.Entries.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "Three" ) ) );
    aLstBox.Entries.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "Four" ) ) );
    aWriter.CreateControl( aLstBox );

    // dropdown list box
    aLstBox.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "testDropDownListBox" ) );
    aLstBox.DropDown = true;
    aLstBox.Location = Rectangle( Point( 4500, 19500 ), Size( 3000, 500 ) );
    aWriter.CreateControl( aLstBox );

    // combo box
    PDFWriter::ComboBoxWidget aComboBox;
    aComboBox.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "testComboBox" ) );
    aComboBox.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "test a combobox" ) );
    aComboBox.Entries.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "Larry" ) ) );
    aComboBox.Entries.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "Curly" ) ) );
    aComboBox.Entries.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "Moe" ) ) );
    aComboBox.Location = Rectangle( Point( 4500, 20000 ), Size( 3000, 500 ) );
    aWriter.CreateControl( aComboBox );

    // test outlines
    sal_Int32 nPage1OL = aWriter.CreateOutlineItem();
    aWriter.SetOutlineItemText( nPage1OL, OUString( RTL_CONSTASCII_USTRINGPARAM( "Page 1" ) ) );
    aWriter.SetOutlineItemDest( nPage1OL, nSecondDest );
    aWriter.CreateOutlineItem( nPage1OL, OUString( RTL_CONSTASCII_USTRINGPARAM( "Dest 2" ) ), nSecondDest );
    aWriter.CreateOutlineItem( nPage1OL, OUString( RTL_CONSTASCII_USTRINGPARAM( "Dest 2 revisited" ) ), nSecondDest );
    aWriter.CreateOutlineItem( nPage1OL, OUString( RTL_CONSTASCII_USTRINGPARAM( "Dest 2 again" ) ), nSecondDest );
    sal_Int32 nPage2OL = aWriter.CreateOutlineItem();
    aWriter.SetOutlineItemText( nPage2OL, OUString( RTL_CONSTASCII_USTRINGPARAM( "Page 2" ) ) );
    aWriter.CreateOutlineItem( nPage2OL, OUString( RTL_CONSTASCII_USTRINGPARAM( "Dest 1" ) ), nFirstDest );

    aWriter.Emit();
}
#endif
static const sal_Int32 nLog10Divisor = 1;
static const double fDivisor = 10.0;

static inline double pixelToPoint( sal_Int32 px ) { return double(px)/fDivisor; }
static inline double pixelToPoint( double px ) { return px/fDivisor; }
static inline sal_Int32 pointToPixel( double pt ) { return sal_Int32(pt*fDivisor); }

static void appendHex( sal_Int8 nInt, OStringBuffer& rBuffer )
{
    static const sal_Char pHexDigits[] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                           '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    rBuffer.append( pHexDigits[ (nInt >> 4) & 15 ] );
    rBuffer.append( pHexDigits[ nInt & 15 ] );
}

static void appendName( const OUString& rStr, OStringBuffer& rBuffer )
{
    OString aStr( OUStringToOString( rStr, RTL_TEXTENCODING_UTF8 ) );
    const sal_Char* pStr = aStr.getStr();
    int nLen = aStr.getLength();
    for( int i = 0; i < nLen; i++ )
    {
        /*  #i16920# PDF recommendation: output UTF8, any byte
         *  outside the interval [33(=ASCII'!');126(=ASCII'~')]
         *  should be escaped hexadecimal
         *  for the sake of ghostscript which also reads PDF
         *  but has a narrower acceptance rate we only pass
         *  alphanumerics and '-' literally.
         */
        if( (pStr[i] >= 'A' && pStr[i] <= 'Z' ) ||
            (pStr[i] >= 'a' && pStr[i] <= 'z' ) ||
            (pStr[i] >= '0' && pStr[i] <= '9' ) ||
            pStr[i] == '-' )
        {
            rBuffer.append( pStr[i] );
        }
        else
        {
            rBuffer.append( '#' );
            appendHex( (sal_Int8)pStr[i], rBuffer );
        }
    }
}

static void appendName( const sal_Char* pStr, OStringBuffer& rBuffer )
{
    while( pStr && *pStr )
    {
        if( (*pStr >= 'A' && *pStr <= 'Z' ) ||
            (*pStr >= 'a' && *pStr <= 'z' ) ||
            (*pStr >= '0' && *pStr <= '9' ) ||
            *pStr == '-' )
        {
            rBuffer.append( *pStr );
        }
        else
        {
            rBuffer.append( '#' );
            appendHex( (sal_Int8)*pStr, rBuffer );
        }
        pStr++;
    }
}

static void appendUnicodeTextString( const rtl::OUString& rString, OStringBuffer& rBuffer )
{
    rBuffer.append( "<FEFF" );
    const sal_Unicode* pStr = rString.getStr();
    sal_Int32 nLen = rString.getLength();
    for( int i = 0; i < nLen; i++ )
    {
        sal_Unicode aChar = pStr[i];
        appendHex( (sal_Int8)(aChar >> 8), rBuffer );
        appendHex( (sal_Int8)(aChar & 255 ), rBuffer );
    }
    rBuffer.append( ">" );
}

OString PDFWriterImpl::convertWidgetFieldName( const rtl::OUString& rString )
{
    OStringBuffer aBuffer( rString.getLength()+64 );
    appendName( rString, aBuffer );

    // replace all '.' by '_'
    sal_Int32 nLen = aBuffer.getLength();
    for( sal_Int32 i = 0; i < nLen; i++ )
    {
        sal_Char aChar = aBuffer.charAt( i );
        if( aChar == '.' )
            aBuffer.setCharAt( i, '_' );
    }

    OString aRet = aBuffer.makeStringAndClear();
    std::hash_map<OString, sal_Int32, OStringHash>::iterator it = m_aFieldNameMap.find( aRet );

    if( it != m_aFieldNameMap.end() ) // not unique
    {
        std::hash_map< OString, sal_Int32, OStringHash >::const_iterator check_it;
        OString aTry;
        do
        {
            OStringBuffer aUnique( aRet.getLength() + 16 );
            aUnique.append( aRet );
            aUnique.append( '_' );
            aUnique.append( it->second );
            it->second++;
            aTry = aUnique.makeStringAndClear();
            check_it = m_aFieldNameMap.find( aTry );
        } while( check_it != m_aFieldNameMap.end() );
        aRet = aTry;
    }
    else
        m_aFieldNameMap[ aRet ] = 2;
    return aRet;
}

static void appendFixedInt( sal_Int32 nValue, OStringBuffer& rBuffer, sal_Int32 nPrecision = nLog10Divisor )
{
    if( nValue < 0 )
    {
        rBuffer.append( '-' );
        nValue = -nValue;
    }
    sal_Int32 nFactor = 1, nDiv = nPrecision;
    while( nDiv-- )
        nFactor *= 10;

    sal_Int32 nInt      = nValue / nFactor;
    rBuffer.append( nInt );
    if( nFactor > 1 )
    {
        sal_Int32 nDecimal  = nValue % nFactor;
        if( nDecimal )
        {
            rBuffer.append( '.' );
            // omit trailing zeros
            while( (nDecimal % 10) == 0 )
                nDecimal /= 10;
            rBuffer.append( nDecimal );
        }
    }
}


// appends a double. PDF does not accept exponential format, only fixed point
static void appendDouble( double fValue, OStringBuffer& rBuffer, int nPrecision = 5 )
{
    bool bNeg = false;
    if( fValue < 0.0 )
    {
        bNeg = true;
        fValue=-fValue;
    }

    sal_Int64 nInt = (sal_Int64)fValue;
    fValue -= (double)nInt;
    // optimizing hardware may lead to a value of 1.0 after the subtraction
    if( fValue == 1.0 || log10( 1.0-fValue ) <= -nPrecision )
    {
        nInt++;
        fValue = 0.0;
    }
    sal_Int64 nFrac = 0;
    if( fValue )
    {
        fValue *= pow( 10.0, (double)nPrecision );
        nFrac = (sal_Int64)fValue;
    }
    if( bNeg && ( nInt || nFrac ) )
        rBuffer.append( '-' );
    rBuffer.append( nInt );
    if( nFrac )
    {
        int i;
        rBuffer.append( '.' );
        sal_Int64 nBound = (sal_Int64)(pow( 10.0, nPrecision - 1.0 )+0.5);
        for ( i = 0; ( i < nPrecision ) && nFrac; i++ )
        {
            sal_Int64 nNumb = nFrac / nBound;
            nFrac -= nNumb * nBound;
            rBuffer.append( nNumb );
            nBound /= 10;
        }
    }
}


static void appendColor( const Color& rColor, OStringBuffer& rBuffer )
{

    if( rColor != Color( COL_TRANSPARENT ) )
    {
        appendDouble( (double)rColor.GetRed() / 255.0, rBuffer );
        rBuffer.append( ' ' );
        appendDouble( (double)rColor.GetGreen() / 255.0, rBuffer );
        rBuffer.append( ' ' );
        appendDouble( (double)rColor.GetBlue() / 255.0, rBuffer );
    }
}

static void appendStrokingColor( const Color& rColor, OStringBuffer& rBuffer )
{
    if( rColor != Color( COL_TRANSPARENT ) )
    {
        appendColor( rColor, rBuffer );
        rBuffer.append( " RG" );
    }
}

static void appendNonStrokingColor( const Color& rColor, OStringBuffer& rBuffer )
{
    if( rColor != Color( COL_TRANSPARENT ) )
    {
        appendColor( rColor, rBuffer );
        rBuffer.append( " rg" );
    }
}

// matrix helper class
namespace vcl
{
/*  for sparse matrices of the form (2D linear transformations)
 *  f[0] f[1] 0
 *  f[2] f[3] 0
 *  f[4] f[5] 1
 */
class Matrix3
{
    double f[6];

    void set( double *pn ) { for( int i = 0 ; i < 5; i++ ) f[i] = pn[i]; }
public:
    Matrix3();
    ~Matrix3() {}

    void skew( double alpha, double beta );
    void scale( double sx, double sy );
    void rotate( double angle );
    void translate( double tx, double ty );

    void append( PDFWriterImpl::PDFPage& rPage, OStringBuffer& rBuffer, Point* pBack = NULL );

    Point transform( const Point& rPoint );
};
}

Matrix3::Matrix3()
{
    // initialize to unity
    f[0] = 1.0;
    f[1] = 0.0;
    f[2] = 0.0;
    f[3] = 1.0;
    f[4] = 0.0;
    f[5] = 0.0;
}

Point Matrix3::transform( const Point& rOrig )
{
    double x = (double)rOrig.X(), y = (double)rOrig.Y();
    return Point( (int)(x*f[0] + y*f[2] + f[4]), (int)(x*f[1] + y*f[3] + f[5]) );
}

void Matrix3::skew( double alpha, double beta )
{
    double fn[6];
    double tb = tan( beta );
    fn[0] = f[0] + f[2]*tb;
    fn[1] = f[1];
    fn[2] = f[2] + f[3]*tb;
    fn[3] = f[3];
    fn[4] = f[4] + f[5]*tb;
    fn[5] = f[5];
    if( alpha != 0.0 )
    {
        double ta = tan( alpha );
        fn[1] += f[0]*ta;
        fn[3] += f[2]*ta;
        fn[5] += f[4]*ta;
    }
    set( fn );
}

void Matrix3::scale( double sx, double sy )
{
    double fn[6];
    fn[0] = sx*f[0];
    fn[1] = sy*f[1];
    fn[2] = sx*f[2];
    fn[3] = sy*f[3];
    fn[4] = sx*f[4];
    fn[5] = sy*f[5];
    set( fn );
}

void Matrix3::rotate( double angle )
{
    double fn[6];
    double fSin = sin(angle);
    double fCos = cos(angle);
    fn[0] = f[0]*fCos - f[1]*fSin;
    fn[1] = f[0]*fSin + f[1]*fCos;
    fn[2] = f[2]*fCos - f[3]*fSin;
    fn[3] = f[2]*fSin + f[3]*fCos;
    fn[4] = f[4]*fCos - f[5]*fSin;
    fn[5] = f[4]*fSin + f[5]*fCos;
    set( fn );
}

void Matrix3::translate( double tx, double ty )
{
    f[4] += tx;
    f[5] += ty;
}

void Matrix3::append( PDFWriterImpl::PDFPage& rPage, OStringBuffer& rBuffer, Point* pBack )
{
    appendDouble( f[0], rBuffer );
    rBuffer.append( ' ' );
    appendDouble( f[1], rBuffer );
    rBuffer.append( ' ' );
    appendDouble( f[2], rBuffer );
    rBuffer.append( ' ' );
    appendDouble( f[3], rBuffer );
    rBuffer.append( ' ' );
    rPage.appendPoint( Point( (long)f[4], (long)f[5] ), rBuffer, false, pBack );
}



PDFWriterImpl::PDFPage::PDFPage( PDFWriterImpl* pWriter, sal_Int32 nPageWidth, sal_Int32 nPageHeight, PDFWriter::Orientation eOrientation )
        :
        m_pWriter( pWriter ),
        m_nPageWidth( nPageWidth ),
        m_nPageHeight( nPageHeight ),
        m_eOrientation( eOrientation ),
        m_nPageObject( 0 ),  // invalid object number
        m_nPageIndex( -1 ), // invalid index
        m_nStreamObject( 0 ),
        m_nStreamLengthObject( 0 ),
        m_nBeginStreamPos( 0 ),
        m_eTransition( PDFWriter::Regular ),
        m_nTransTime( 0 ),
        m_nDuration( 0 ),
        m_bHasWidgets( false )
{
    // object ref must be only ever updated in emit()
    m_nPageObject = m_pWriter->createObject();
}

PDFWriterImpl::PDFPage::~PDFPage()
{
}

void PDFWriterImpl::PDFPage::beginStream()
{
    m_nStreamObject = m_pWriter->createObject();
    if( ! m_pWriter->updateObject( m_nStreamObject ) )
        return;

    m_nStreamLengthObject = m_pWriter->createObject();
    // write content stream header
    OStringBuffer aLine;
    aLine.append( m_nStreamObject );
    aLine.append( " 0 obj\n<</Length " );
    aLine.append( m_nStreamLengthObject );
    aLine.append( " 0 R" );
#if defined ( COMPRESS_PAGES ) && !defined ( DEBUG_DISABLE_PDFCOMPRESSION )
    aLine.append( "/Filter/FlateDecode" );
#endif
    aLine.append( ">>\nstream\n" );
    if( ! m_pWriter->writeBuffer( aLine.getStr(), aLine.getLength() ) )
        return;
    if( osl_File_E_None != osl_getFilePos( m_pWriter->m_aFile, &m_nBeginStreamPos ) )
    {
        osl_closeFile( m_pWriter->m_aFile );
        m_pWriter->m_bOpen = false;
    }
#if defined ( COMPRESS_PAGES ) && !defined ( DEBUG_DISABLE_PDFCOMPRESSION )
    m_pWriter->beginCompression();
#endif
}

void PDFWriterImpl::PDFPage::endStream()
{
    sal_uInt64 nEndStreamPos;
    if( osl_File_E_None != osl_getFilePos( m_pWriter->m_aFile, &nEndStreamPos ) )
    {
        osl_closeFile( m_pWriter->m_aFile );
        m_pWriter->m_bOpen = false;
        return;
    }
    if( ! m_pWriter->writeBuffer( "\nendstream\nendobj\n\n", 19 ) )
        return;
    // emit stream length object
    if( ! m_pWriter->updateObject( m_nStreamLengthObject ) )
        return;
    OStringBuffer aLine;
    aLine.append( m_nStreamLengthObject );
    aLine.append( " 0 obj\n" );
    aLine.append( (sal_Int64)(nEndStreamPos-m_nBeginStreamPos) );
    aLine.append( "\nendobj\n\n" );
    m_pWriter->writeBuffer( aLine.getStr(), aLine.getLength() );
}

bool PDFWriterImpl::PDFPage::emit(sal_Int32 nParentObject )
{
    // emit page object
    if( ! m_pWriter->updateObject( m_nPageObject ) )
        return false;
    OStringBuffer aLine;

    aLine.append( m_nPageObject );
    aLine.append( " 0 obj\n"
                  "<</Type/Page/Parent " );
    aLine.append( nParentObject );
    aLine.append( " 0 R" );
    aLine.append( "/Resources " );
    aLine.append( m_pWriter->getResourceDictObj() );
    aLine.append( " 0 R" );
    if( m_nPageWidth && m_nPageHeight )
    {
        aLine.append( "/MediaBox[0 0 " );
        aLine.append( m_nPageWidth );
        aLine.append( ' ' );
        aLine.append( m_nPageHeight );
        aLine.append( "]" );
    }
    switch( m_eOrientation )
    {
        case PDFWriter::Landscape: aLine.append( "/Rotate 90\n" );break;
        case PDFWriter::Seascape:  aLine.append( "/Rotate -90\n" );break;
        case PDFWriter::Portrait:  aLine.append( "/Rotate 0\n" );break;

        case PDFWriter::Inherit:
        default:
            break;
    }
    int nAnnots = m_aAnnotations.size();
    if( nAnnots > 0 )
    {
        aLine.append( "/Annots[\n" );
        for( int i = 0; i < nAnnots; i++ )
        {
            aLine.append( m_aAnnotations[i] );
            aLine.append( " 0 R" );
            aLine.append( ((i+1)%15) ? " " : "\n" );
        }
        aLine.append( "]\n" );
    }
    #if 0
    // FIXME: implement tab order as Structure Tree
    if( m_bHasWidgets && m_pWriter->getVersion() >= PDFWriter::PDF_1_5 )
        aLine.append( "   /Tabs /S\n" );
    #endif
    if( m_aMCIDParents.size() > 0 )
    {
        OStringBuffer aStructParents( 1024 );
        aStructParents.append( "[ " );
        int nParents = m_aMCIDParents.size();
        for( int i = 0; i < nParents; i++ )
        {
            aStructParents.append( m_aMCIDParents[i] );
            aStructParents.append( " 0 R" );
            aStructParents.append( ((i%10) == 9) ? "\n" : " " );
        }
        aStructParents.append( "]" );
        m_pWriter->m_aStructParentTree.push_back( aStructParents.makeStringAndClear() );

        aLine.append( "/StructParents " );
        aLine.append( sal_Int32(m_pWriter->m_aStructParentTree.size()-1) );
        aLine.append( "\n" );
    }
    if( m_nDuration > 0 )
    {
        aLine.append( "/Dur " );
        aLine.append( (sal_Int32)m_nDuration );
        aLine.append( "\n" );
    }
    if( m_eTransition != PDFWriter::Regular && m_nTransTime > 0 )
    {
        // transition duration
        aLine.append( "/Trans<</D " );
        appendDouble( (double)m_nTransTime/1000.0, aLine, 3 );
        aLine.append( "\n" );
        const char *pStyle = NULL, *pDm = NULL, *pM = NULL, *pDi = NULL;
        switch( m_eTransition )
        {
            case PDFWriter::SplitHorizontalInward:
                pStyle = "Split"; pDm = "H"; pM = "I"; break;
            case PDFWriter::SplitHorizontalOutward:
                pStyle = "Split"; pDm = "H"; pM = "O"; break;
            case PDFWriter::SplitVerticalInward:
                pStyle = "Split"; pDm = "V"; pM = "I"; break;
            case PDFWriter::SplitVerticalOutward:
                pStyle = "Split"; pDm = "V"; pM = "O"; break;
            case PDFWriter::BlindsHorizontal:
                pStyle = "Blinds"; pDm = "H"; break;
            case PDFWriter::BlindsVertical:
                pStyle = "Blinds"; pDm = "V"; break;
            case PDFWriter::BoxInward:
                pStyle = "Box"; pM = "I"; break;
            case PDFWriter::BoxOutward:
                pStyle = "Box"; pM = "O"; break;
            case PDFWriter::WipeLeftToRight:
                pStyle = "Wipe"; pDi = "0"; break;
            case PDFWriter::WipeBottomToTop:
                pStyle = "Wipe"; pDi = "90"; break;
            case PDFWriter::WipeRightToLeft:
                pStyle = "Wipe"; pDi = "180"; break;
            case PDFWriter::WipeTopToBottom:
                pStyle = "Wipe"; pDi = "270"; break;
            case PDFWriter::Dissolve:
                pStyle = "Dissolve"; break;
            case PDFWriter::GlitterLeftToRight:
                pStyle = "Glitter"; pDi = "0"; break;
            case PDFWriter::GlitterTopToBottom:
                pStyle = "Glitter"; pDi = "270"; break;
            case PDFWriter::GlitterTopLeftToBottomRight:
                pStyle = "Glitter"; pDi = "315"; break;
            case PDFWriter::Regular:
                break;
        }
        // transition style
        if( pStyle )
        {
            aLine.append( "/S/" );
            aLine.append( pStyle );
            aLine.append( "\n" );
        }
        if( pDm )
        {
            aLine.append( "/Dm/" );
            aLine.append( pDm );
            aLine.append( "\n" );
        }
        if( pM )
        {
            aLine.append( "/M/" );
            aLine.append( pM );
            aLine.append( "\n" );
        }
        if( pDi  )
        {
            aLine.append( "/Di " );
            aLine.append( pDi );
            aLine.append( "\n" );
        }
        aLine.append( ">>\n" );
    }
    if( m_pWriter->getVersion() > PDFWriter::PDF_1_3 )
    {
        aLine.append( "/Group<</S/Transparency/CS/DeviceRGB/I true>>" );
    }
    aLine.append( "/Contents " );
    aLine.append( m_nStreamObject );
    aLine.append( " 0 R>>\nendobj\n\n" );
    return m_pWriter->writeBuffer( aLine.getStr(), aLine.getLength() );
}

namespace vcl
{
template < class GEOMETRY >
GEOMETRY lcl_convert( const MapMode& _rSource, const MapMode& _rDest, OutputDevice* _pPixelConversion, const GEOMETRY& _rObject )
{
    GEOMETRY aPoint;
    if ( MAP_PIXEL == _rSource.GetMapUnit() )
    {
        aPoint = _pPixelConversion->PixelToLogic( _rObject, _rDest );
    }
    else
    {
        aPoint = OutputDevice::LogicToLogic( _rObject, _rSource, _rDest );
    }
    return aPoint;
}
}

void PDFWriterImpl::PDFPage::appendPoint( const Point& rPoint, OStringBuffer& rBuffer, bool bNeg, Point* pOutPoint ) const
{
    if( pOutPoint )
    {
        Point aPoint( lcl_convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                                   m_pWriter->m_aMapMode,
                                   m_pWriter->getReferenceDevice(),
                                   rPoint ) );
        *pOutPoint = aPoint;
    }

    Point aPoint( lcl_convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                               m_pWriter->m_aMapMode,
                               m_pWriter->getReferenceDevice(),
                               rPoint ) );

    sal_Int32 nValue    = aPoint.X();
    if( bNeg )
        nValue = -nValue;

    appendFixedInt( nValue, rBuffer );

    rBuffer.append( ' ' );

    nValue      = pointToPixel(getHeight()) - aPoint.Y();
    if( bNeg )
        nValue = -nValue;

    appendFixedInt( nValue, rBuffer );
}

void PDFWriterImpl::PDFPage::appendRect( const Rectangle& rRect, OStringBuffer& rBuffer ) const
{
    appendPoint( rRect.BottomLeft() + Point( 0, 1 ), rBuffer );
    rBuffer.append( ' ' );
    appendMappedLength( (sal_Int32)rRect.GetWidth(), rBuffer, false );
    rBuffer.append( ' ' );
    appendMappedLength( (sal_Int32)rRect.GetHeight(), rBuffer, true );
    rBuffer.append( " re" );
}

void PDFWriterImpl::PDFPage::convertRect( Rectangle& rRect ) const
{
    Point aLL = lcl_convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                             m_pWriter->m_aMapMode,
                             m_pWriter->getReferenceDevice(),
                             rRect.BottomLeft() + Point( 0, 1 )
                             );
    Size aSize = lcl_convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                              m_pWriter->m_aMapMode,
                              m_pWriter->getReferenceDevice(),
                              rRect.GetSize() );
    rRect.Left()    = aLL.X();
    rRect.Right()   = aLL.X() + aSize.Width();
    rRect.Top()     = pointToPixel(getHeight()) - aLL.Y();
    rRect.Bottom()  = rRect.Top() + aSize.Height();
}

void PDFWriterImpl::PDFPage::appendPolygon( const Polygon& rPoly, OStringBuffer& rBuffer, bool bClose ) const
{
    USHORT nPoints = rPoly.GetSize();
    /*
     *  #108582# applications do weird things
     */
    sal_uInt32 nBufLen = rBuffer.getLength();
    if( nPoints > 0 )
    {
        const BYTE* pFlagArray = rPoly.GetConstFlagAry();
        appendPoint( rPoly[0], rBuffer );
        rBuffer.append( " m\n" );
        for( USHORT i = 1; i < nPoints; i++ )
        {
            if( pFlagArray && pFlagArray[i] == POLY_CONTROL && nPoints-i > 2 )
            {
                // bezier
                DBG_ASSERT( pFlagArray[i+1] == POLY_CONTROL && pFlagArray[i+2] != POLY_CONTROL, "unexpected sequence of control points" );
                appendPoint( rPoly[i], rBuffer );
                rBuffer.append( " " );
                appendPoint( rPoly[i+1], rBuffer );
                rBuffer.append( " " );
                appendPoint( rPoly[i+2], rBuffer );
                rBuffer.append( " c" );
                i += 2; // add additionally consumed points
            }
            else
            {
                // line
                appendPoint( rPoly[i], rBuffer );
                rBuffer.append( " l" );
            }
            if( (rBuffer.getLength() - nBufLen) > 65 )
            {
                rBuffer.append( "\n" );
                nBufLen = rBuffer.getLength();
            }
            else
                rBuffer.append( " " );
        }
        if( bClose )
            rBuffer.append( "h\n" );
    }
}

void PDFWriterImpl::PDFPage::appendPolyPolygon( const PolyPolygon& rPolyPoly, OStringBuffer& rBuffer, bool bClose ) const
{
    USHORT nPolygons = rPolyPoly.Count();
    for( USHORT n = 0; n < nPolygons; n++ )
        appendPolygon( rPolyPoly[n], rBuffer, bClose );
}

void PDFWriterImpl::PDFPage::appendMappedLength( sal_Int32 nLength, OStringBuffer& rBuffer, bool bVertical, sal_Int32* pOutLength ) const
{
    sal_Int32 nValue = nLength;
    if ( nLength < 0 )
    {
        rBuffer.append( '-' );
        nValue = -nLength;
    }
    Size aSize( lcl_convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                             m_pWriter->m_aMapMode,
                             m_pWriter->getReferenceDevice(),
                             Size( nValue, nValue ) ) );
    nValue = bVertical ? aSize.Height() : aSize.Width();
    if( pOutLength )
        *pOutLength = ((nLength < 0 ) ? -nValue : nValue);

    appendFixedInt( nValue, rBuffer, 1 );
}

void PDFWriterImpl::PDFPage::appendMappedLength( double fLength, OStringBuffer& rBuffer, bool bVertical, sal_Int32* pOutLength ) const
{
    Size aSize( lcl_convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                             m_pWriter->m_aMapMode,
                             m_pWriter->getReferenceDevice(),
                             Size( 1000, 1000 ) ) );
    if( pOutLength )
        *pOutLength = (sal_Int32)(fLength*(double)(bVertical ? aSize.Height() : aSize.Width())/1000.0);
    fLength *= pixelToPoint((double)(bVertical ? aSize.Height() : aSize.Width()) / 1000.0);
    appendDouble( fLength, rBuffer );
}

bool PDFWriterImpl::PDFPage::appendLineInfo( const LineInfo& rInfo, OStringBuffer& rBuffer ) const
{
    bool bRet = true;
    if( rInfo.GetStyle() == LINE_DASH )
    {
        rBuffer.append( "[ " );
        if( rInfo.GetDashLen() == rInfo.GetDotLen() ) // degraded case
        {
            appendMappedLength( (sal_Int32)rInfo.GetDashLen(), rBuffer );
            rBuffer.append( ' ' );
            appendMappedLength( (sal_Int32)rInfo.GetDistance(), rBuffer );
            rBuffer.append( ' ' );
        }
        else
        {
            // check for implementation limits of dash array
            // in PDF reader apps (e.g. acroread)
            if( 2*(rInfo.GetDashCount() + rInfo.GetDotCount()) > 10 )
                bRet = false;
            for( int n = 0; n < rInfo.GetDashCount(); n++ )
            {
                appendMappedLength( (sal_Int32)rInfo.GetDashLen(), rBuffer );
                rBuffer.append( ' ' );
                appendMappedLength( (sal_Int32)rInfo.GetDistance(), rBuffer );
                rBuffer.append( ' ' );
            }
            for( int m = 0; m < rInfo.GetDotCount(); m++ )
            {
                appendMappedLength( (sal_Int32)rInfo.GetDotLen(), rBuffer );
                rBuffer.append( ' ' );
                appendMappedLength( (sal_Int32)rInfo.GetDistance(), rBuffer );
                rBuffer.append( ' ' );
            }
        }
        rBuffer.append( "] 0 d\n" );
    }
    if( rInfo.GetWidth() > 1 )
    {
        appendMappedLength( (sal_Int32)rInfo.GetWidth(), rBuffer );
        rBuffer.append( " w\n" );
    }
    else if( rInfo.GetWidth() == 0 )
        rBuffer.append( "0 w\n" );
    return bRet;
}

void PDFWriterImpl::PDFPage::appendWaveLine( sal_Int32 nWidth, sal_Int32 nY, sal_Int32 nDelta, OStringBuffer& rBuffer ) const
{
    if( nWidth <= 0 )
        return;
    if( nDelta < 1 )
        nDelta = 1;

    rBuffer.append( "0 " );
    appendMappedLength( nY, rBuffer, true );
    rBuffer.append( " m\n" );
    for( sal_Int32 n = 0; n < nWidth; )
    {
        n += nDelta;
        appendMappedLength( n, rBuffer, false );
        rBuffer.append( ' ' );
        appendMappedLength( nDelta+nY, rBuffer, true );
        rBuffer.append( ' ' );
        n += nDelta;
        appendMappedLength( n, rBuffer, false );
        rBuffer.append( ' ' );
        appendMappedLength( nY, rBuffer, true );
        rBuffer.append( " v " );
        if( n < nWidth )
        {
            n += nDelta;
            appendMappedLength( n, rBuffer, false );
            rBuffer.append( ' ' );
            appendMappedLength( nY-nDelta, rBuffer, true );
            rBuffer.append( ' ' );
            n += nDelta;
            appendMappedLength( n, rBuffer, false );
            rBuffer.append( ' ' );
            appendMappedLength( nY, rBuffer, true );
            rBuffer.append( " v\n" );
        }
    }
    rBuffer.append( "S\n" );
}

/*
 *  class PDFWriterImpl
 */

PDFWriterImpl::PDFWriterImpl( const PDFWriter::PDFWriterContext& rContext )
        :
        m_pReferenceDevice( NULL ),
        m_aMapMode( MAP_POINT, Point(), Fraction( 1L, pointToPixel(1) ), Fraction( 1L, pointToPixel(1) ) ),
        m_nCurrentStructElement( 0 ),
        m_bEmitStructure( true ),
        m_bNewMCID( false ),
        m_nCurrentControl( -1 ),
        m_nNextFID( 1 ),
        m_nInheritedPageWidth( 595 ),  // default A4
        m_nInheritedPageHeight( 842 ), // default A4
        m_eInheritedOrientation( PDFWriter::Portrait ),
        m_nCurrentPage( -1 ),
        m_nResourceDict( -1 ),
        m_nFontResourceDict( -1 ),
        m_pCodec( NULL )
{
#ifdef DO_TEST_PDF
    static bool bOnce = true;
    if( bOnce )
    {
        bOnce = false;
        doTestCode();
    }
#endif
    m_aContext = rContext;
    m_aStructure.push_back( PDFStructureElement() );
    m_aStructure[0].m_nOwnElement       = 0;
    m_aStructure[0].m_nParentElement    = 0;

    Font aFont;
    aFont.SetName( String( RTL_CONSTASCII_USTRINGPARAM( "Times" ) ) );
    aFont.SetSize( Size( 0, 12 ) );

    GraphicsState aState;
    aState.m_aMapMode       = m_aMapMode;
    aState.m_aFont          = aFont;
    m_aGraphicsStack.push_front( aState );

    oslFileError  aError = osl_openFile( m_aContext.URL.pData, &m_aFile, osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );
    if( aError != osl_File_E_None )
    {
        if( aError == osl_File_E_EXIST )
        {
            aError = osl_openFile( m_aContext.URL.pData, &m_aFile, osl_File_OpenFlag_Write );
            if( aError == osl_File_E_None )
                aError = osl_setFileSize( m_aFile, 0 );
        }
    }
    if( aError != osl_File_E_None )
        return;

    m_bOpen = true;
    // write header
    OStringBuffer aBuffer( 20 );
    aBuffer.append( "%PDF-" );
    switch( m_aContext.Version )
    {
        case PDFWriter::PDF_1_2: aBuffer.append( "1.2" );break;
        case PDFWriter::PDF_1_3: aBuffer.append( "1.3" );break;
        default:
        case PDFWriter::PDF_1_4: aBuffer.append( "1.4" );break;
        case PDFWriter::PDF_1_5: aBuffer.append( "1.5" );break;
    }
    // append something binary as comment (suggested in PDF Reference)
    aBuffer.append( "\n%äüöß\n" );
    if( !writeBuffer( aBuffer.getStr(), aBuffer.getLength() ) )
    {
        osl_closeFile( m_aFile );
        m_bOpen = false;
        return;
    }

    // insert outline root
    m_aOutline.push_back( PDFOutlineEntry() );
}

PDFWriterImpl::~PDFWriterImpl()
{
    delete static_cast<VirtualDevice*>(m_pReferenceDevice);
}

void PDFWriterImpl::setDocInfo( const PDFDocInfo& rInfo )
{
    m_aDocInfo.Title                = rInfo.Title;
    m_aDocInfo.Author               = rInfo.Author;
    m_aDocInfo.Subject              = rInfo.Subject;
    m_aDocInfo.Keywords             = rInfo.Keywords;
    m_aDocInfo.Creator              = rInfo.Creator;
    m_aDocInfo.Producer             = rInfo.Producer;
}

void PDFWriterImpl::emitComment( const char* pComment )
{
    OStringBuffer aLine( 64 );
    aLine.append( "% " );
    aLine.append( (const sal_Char*)pComment );
    aLine.append( "\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

bool PDFWriterImpl::compressStream( SvMemoryStream* pStream )
{
#ifndef DEBUG_DISABLE_PDFCOMPRESSION
    pStream->Seek( STREAM_SEEK_TO_END );
    ULONG nEndPos = pStream->Tell();
    pStream->Seek( STREAM_SEEK_TO_BEGIN );
    ZCodec* pCodec = new ZCodec( 0x4000, 0x4000 );
    SvMemoryStream aStream;
    pCodec->BeginCompression();
    pCodec->Write( aStream, (const BYTE*)pStream->GetData(), nEndPos );
    pCodec->EndCompression();
    delete pCodec;
    nEndPos = aStream.Tell();
    pStream->Seek( STREAM_SEEK_TO_BEGIN );
    aStream.Seek( STREAM_SEEK_TO_BEGIN );
    pStream->SetStreamSize( nEndPos );
    pStream->Write( aStream.GetData(), nEndPos );
    return true;
#else
    return false;
#endif
}

void PDFWriterImpl::beginCompression()
{
#ifndef DEBUG_DISABLE_PDFCOMPRESSION
    m_pCodec = new ZCodec( 0x4000, 0x4000 );
    m_pMemStream = new SvMemoryStream();
    m_pCodec->BeginCompression();
#endif
}

void PDFWriterImpl::endCompression()
{
#ifndef DEBUG_DISABLE_PDFCOMPRESSION
    if( m_pCodec )
    {
        m_pCodec->EndCompression();
        delete m_pCodec;
        m_pCodec = NULL;
        sal_uInt64 nLen = m_pMemStream->Tell();
        m_pMemStream->Seek( 0 );
        writeBuffer( m_pMemStream->GetData(), nLen );
        delete m_pMemStream;
        m_pMemStream = NULL;
    }
#endif
}

bool PDFWriterImpl::writeBuffer( const void* pBuffer, sal_uInt64 nBytes )
{
    if( ! m_bOpen ) // we are already down the drain
        return false;

    if( ! nBytes ) // huh ?
        return true;

    if( m_aOutputStreams.begin() != m_aOutputStreams.end() )
    {
        m_aOutputStreams.front().m_pStream->Seek( STREAM_SEEK_TO_END );
        m_aOutputStreams.front().m_pStream->Write( pBuffer, sal::static_int_cast<sal_Size>(nBytes) );
        return true;
    }

    sal_uInt64 nWritten;
    if( m_pCodec )
    {
        m_pCodec->Write( *m_pMemStream, static_cast<const BYTE*>(pBuffer), (ULONG)nBytes );
        nWritten = nBytes;
    }
    else
    {
        if( osl_writeFile( m_aFile, pBuffer, nBytes, &nWritten ) != osl_File_E_None )
            nWritten = 0;

        if( nWritten != nBytes )
        {
            osl_closeFile( m_aFile );
            m_bOpen = false;
        }
    }

    return nWritten == nBytes;
}

OutputDevice* PDFWriterImpl::getReferenceDevice()
{
    if( ! m_pReferenceDevice )
    {
        VirtualDevice*  pVDev = new VirtualDevice( 0 );

        m_pReferenceDevice = pVDev;

        pVDev->SetReferenceDevice( VirtualDevice::REFDEV_MODE_PDF1 );

        pVDev->SetOutputSizePixel( Size( 640, 480 ) );
        pVDev->SetMapMode( MAP_MM );

        m_pReferenceDevice->mpPDFWriter = this;
        m_pReferenceDevice->ImplUpdateFontData( TRUE );
    }
    return m_pReferenceDevice;
}

class ImplPdfBuiltinFontData : public ImplFontData
{
private:
    const PDFWriterImpl::BuiltinFont& mrBuiltin;

public:
    enum {PDF_FONT_MAGIC = 0xBDFF0A1C };
                                        ImplPdfBuiltinFontData( const PDFWriterImpl::BuiltinFont& );
    const PDFWriterImpl::BuiltinFont*   GetBuiltinFont() const  { return &mrBuiltin; }

    virtual ImplFontData*               Clone() const { return new ImplPdfBuiltinFontData(*this); }
    virtual ImplFontEntry*              CreateFontInstance( ImplFontSelectData& ) const;
};

inline const ImplPdfBuiltinFontData* GetPdfFontData( const ImplFontData* pFontData )
{
    const ImplPdfBuiltinFontData* pFD = NULL;
    if( pFontData && pFontData->CheckMagic( ImplPdfBuiltinFontData::PDF_FONT_MAGIC ) )
        pFD = static_cast<const ImplPdfBuiltinFontData*>( pFontData );
    return pFD;
}

static ImplDevFontAttributes GetDevFontAttributes( const PDFWriterImpl::BuiltinFont& rBuiltin )
{
    ImplDevFontAttributes aDFA;
    aDFA.maName         = String::CreateFromAscii( rBuiltin.m_pName );
    aDFA.maStyleName    = String::CreateFromAscii( rBuiltin.m_pStyleName );
    aDFA.meFamily       = rBuiltin.m_eFamily;
    aDFA.mbSymbolFlag   = (rBuiltin.m_eCharSet == RTL_TEXTENCODING_SYMBOL);
    aDFA.mePitch        = rBuiltin.m_ePitch;
    aDFA.meWeight       = rBuiltin.m_eWeight;
    aDFA.meItalic       = rBuiltin.m_eItalic;
    aDFA.meWidthType    = rBuiltin.m_eWidthType;

    aDFA.mbOrientation  = true;
    aDFA.mbDevice       = true;
    aDFA.mnQuality      = 50000;
    aDFA.mbSubsettable  = false;
    aDFA.mbEmbeddable   = false;
    return aDFA;
}

ImplPdfBuiltinFontData::ImplPdfBuiltinFontData( const PDFWriterImpl::BuiltinFont& rBuiltin )
:   ImplFontData( GetDevFontAttributes(rBuiltin), PDF_FONT_MAGIC ),
    mrBuiltin( rBuiltin )
{}

ImplFontEntry* ImplPdfBuiltinFontData::CreateFontInstance( ImplFontSelectData& rFSD ) const
{
    ImplFontEntry* pEntry = new ImplFontEntry( rFSD );
    return pEntry;
}

ImplDevFontList* PDFWriterImpl::filterDevFontList( ImplDevFontList* pFontList )
{
    DBG_ASSERT( m_aSubsets.size() == 0, "Fonts changing during PDF generation, document will be invalid" );
    ImplDevFontList* pFiltered = pFontList->Clone( true, true );

    // append the PDF builtin fonts
    for( unsigned int i = 0; i < sizeof(m_aBuiltinFonts)/sizeof(m_aBuiltinFonts[0]); i++ )
    {
        ImplFontData* pNewData = new ImplPdfBuiltinFontData( m_aBuiltinFonts[i] );
        pFiltered->Add( pNewData );
    }
    return pFiltered;
}

bool PDFWriterImpl::isBuiltinFont( ImplFontData* pFont ) const
{
    const ImplPdfBuiltinFontData* pFD = GetPdfFontData( pFont );
    return (pFD != NULL);
}

void PDFWriterImpl::getFontMetric( ImplFontSelectData* pSelect, ImplFontMetricData* pMetric ) const
{
    const ImplPdfBuiltinFontData* pFD = GetPdfFontData( pSelect->mpFontData );
    if( !pFD )
        return;
    const BuiltinFont* pBuiltinFont = pFD->GetBuiltinFont();

    pMetric->mnOrientation  = sal::static_int_cast<short>(pSelect->mnOrientation);
    pMetric->meFamily       = pBuiltinFont->m_eFamily;
    pMetric->mePitch        = pBuiltinFont->m_ePitch;
    pMetric->meWeight       = pBuiltinFont->m_eWeight;
    pMetric->meItalic       = pBuiltinFont->m_eItalic;
    pMetric->mbSymbolFlag   = pFD->IsSymbolFont();
    pMetric->mnWidth        = pSelect->mnHeight;
    pMetric->mnAscent       = ( pSelect->mnHeight * +pBuiltinFont->m_nAscent + 500 ) / 1000;
    pMetric->mnDescent      = ( pSelect->mnHeight * -pBuiltinFont->m_nDescent + 500 ) / 1000;
    pMetric->mnIntLeading   = 0;
    pMetric->mnExtLeading   = 0;
    pMetric->mnSlant        = 0;
    pMetric->mbScalableFont = true;
    pMetric->mbDevice       = true;
}

// -----------------------------------------------------------------------

namespace vcl {

class PDFSalLayout : public GenericSalLayout
{
    PDFWriterImpl&  mrPDFWriterImpl;
    const PDFWriterImpl::BuiltinFont& mrBuiltinFont;
    bool            mbIsSymbolFont;
    long            mnPixelPerEM;
    String          maOrigText;

public:
                    PDFSalLayout( PDFWriterImpl&,
                                  const PDFWriterImpl::BuiltinFont&,
                                  long nPixelPerEM, int nOrientation );

    void            SetText( const String& rText )  { maOrigText = rText; }
    virtual bool    LayoutText( ImplLayoutArgs& );
    virtual void    InitFont() const;
    virtual void    DrawText( SalGraphics& ) const;
};

}

// -----------------------------------------------------------------------

PDFSalLayout::PDFSalLayout( PDFWriterImpl& rPDFWriterImpl,
    const PDFWriterImpl::BuiltinFont& rBuiltinFont,
    long nPixelPerEM, int nOrientation )
:   mrPDFWriterImpl( rPDFWriterImpl ),
    mrBuiltinFont( rBuiltinFont ),
    mnPixelPerEM( nPixelPerEM )
{
    mbIsSymbolFont = (rBuiltinFont.m_eCharSet == RTL_TEXTENCODING_SYMBOL);
    SetOrientation( nOrientation );
}

// -----------------------------------------------------------------------

bool PDFSalLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    const String aText( rArgs.mpStr+rArgs.mnMinCharPos, sal::static_int_cast<xub_StrLen>(rArgs.mnEndCharPos-rArgs.mnMinCharPos) );
    SetText( aText );
    SetUnitsPerPixel( 1000 );

    rtl_UnicodeToTextConverter aConv = rtl_createTextToUnicodeConverter( RTL_TEXTENCODING_MS_1252 );

    Point aNewPos( 0, 0 );
    bool bRightToLeft;
    for( int nCharPos = -1; rArgs.GetNextPos( &nCharPos, &bRightToLeft ); )
    {
        sal_Unicode cChar = rArgs.mpStr[ nCharPos ];
        if( bRightToLeft )
            cChar = GetMirroredChar( cChar );

        if( cChar & 0xff00 )
        {
            // some characters can be used by conversion
            if( (cChar >= 0xf000) && mbIsSymbolFont )
                cChar -= 0xf000;
            else
            {
                sal_Char aBuf[4];
                sal_uInt32 nInfo;
                sal_Size nSrcCvtChars;

                sal_Size nConv = rtl_convertUnicodeToText( aConv,
                                                           NULL,
                                                           &cChar, 1,
                                                           aBuf, 1,
                                                           RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR,
                                                           &nInfo, &nSrcCvtChars );
                // check whether conversion was possible
                // else fallback font is needed as the standard fonts
                // are handled via WinAnsi encoding
                if( nConv > 0 )
                    cChar = ((sal_Unicode)aBuf[0]) & 0x00ff;
            }
        }
        if( cChar & 0xff00 )
        {
            cChar = 0;   // NotDef glyph
            rArgs.NeedFallback( nCharPos, bRightToLeft );
        }

        long nGlyphWidth = (long)mrBuiltinFont.m_aWidths[cChar] * mnPixelPerEM;
        long nGlyphFlags = (nGlyphWidth > 0) ? 0 : GlyphItem::IS_IN_CLUSTER;
        if( bRightToLeft )
            nGlyphFlags |= GlyphItem::IS_RTL_GLYPH;
        // TODO: get kerning from builtin fonts
        GlyphItem aGI( nCharPos, cChar, aNewPos, nGlyphFlags, nGlyphWidth );
        AppendGlyph( aGI );

        aNewPos.X() += nGlyphWidth;
    }

    rtl_destroyUnicodeToTextConverter( aConv );

    return true;
}

// -----------------------------------------------------------------------

void PDFSalLayout::InitFont() const
{
    // TODO: recreate font with all its attributes
}

// -----------------------------------------------------------------------

void PDFSalLayout::DrawText( SalGraphics& ) const
{
    mrPDFWriterImpl.drawLayout( *const_cast<PDFSalLayout*>(this), maOrigText, true );
}

// -----------------------------------------------------------------------

SalLayout* PDFWriterImpl::GetTextLayout( ImplLayoutArgs& rArgs, ImplFontSelectData* pSelect )
{
    DBG_ASSERT( (pSelect->mpFontData != NULL),
        "PDFWriterImpl::GetTextLayout mpFontData is NULL" );

    const ImplPdfBuiltinFontData* pFD = GetPdfFontData( pSelect->mpFontData );
    if( !pFD )
        return NULL;
    const BuiltinFont* pBuiltinFont = pFD->GetBuiltinFont();

    long nPixelPerEM = pSelect->mnWidth ? pSelect->mnWidth : pSelect->mnHeight;
    int nOrientation = pSelect->mnOrientation;
    PDFSalLayout* pLayout = new PDFSalLayout( *this, *pBuiltinFont, nPixelPerEM, nOrientation );
    pLayout->SetText( rArgs.mpStr );
    return pLayout;
}

sal_Int32 PDFWriterImpl::newPage( sal_Int32 nPageWidth, sal_Int32 nPageHeight, PDFWriter::Orientation eOrientation )
{
    endPage();
    m_nCurrentPage = m_aPages.size();
    m_aPages.push_back( PDFPage(this, nPageWidth, nPageHeight, eOrientation ) );
    m_aPages.back().m_nPageIndex = m_nCurrentPage;
    m_aPages.back().beginStream();

    // setup global graphics state
    // linewidth is 0 (as thin as possible) by default
    writeBuffer( "0 w\n", 4 );

    return m_nCurrentPage;
}

void PDFWriterImpl::endPage()
{
    if( m_aPages.begin() != m_aPages.end() )
    {
        // close eventual MC sequence
        endStructureElementMCSeq();

        // sanity check
        if( m_aOutputStreams.begin() != m_aOutputStreams.end() )
        {
            DBG_ERROR( "redirection across pages !!!" );
            m_aOutputStreams.clear(); // leak !
            m_aMapMode.SetOrigin( Point() );
        }

        m_aGraphicsStack.clear();
        m_aGraphicsStack.push_back( GraphicsState() );

        // this should pop the PDF graphics stack if necessary
        updateGraphicsState();

        if( m_pCodec )
            endCompression();

        m_aPages.back().endStream();

        // reset the default font
        Font aFont;
        aFont.SetName( String( RTL_CONSTASCII_USTRINGPARAM( "Times" ) ) );
        aFont.SetSize( Size( 0, 12 ) );

        m_aCurrentPDFState = m_aGraphicsStack.front();
        m_aGraphicsStack.front().m_aFont =  aFont;

        for( std::list<BitmapEmit>::iterator it = m_aBitmaps.begin();
             it != m_aBitmaps.end(); ++it )
        {
            if( ! it->m_aBitmap.IsEmpty() )
            {
                writeBitmapObject( *it );
                it->m_aBitmap = BitmapEx();
            }
        }
        for( std::list<JPGEmit>::iterator jpeg = m_aJPGs.begin(); jpeg != m_aJPGs.end(); ++jpeg )
        {
            if( jpeg->m_pStream )
            {
                writeJPG( *jpeg );
                delete jpeg->m_pStream;
                jpeg->m_pStream = NULL;
                jpeg->m_aMask = Bitmap();
            }
        }
        for( std::list<TransparencyEmit>::iterator t = m_aTransparentObjects.begin();
             t != m_aTransparentObjects.end(); ++t )
        {
            if( t->m_pContentStream )
            {
                writeTransparentObject( *t );
                delete t->m_pContentStream;
                t->m_pContentStream = NULL;
            }
        }
    }
}

sal_Int32 PDFWriterImpl::createObject()
{
    m_aObjects.push_back( ~0U );
    return m_aObjects.size();
}

bool PDFWriterImpl::updateObject( sal_Int32 n )
{
    if( ! m_bOpen )
        return false;

    sal_uInt64 nOffset = ~0U;
    oslFileError aError = osl_getFilePos( m_aFile, &nOffset );
    DBG_ASSERT( aError == osl_File_E_None, "could not register object" );
    if( aError != osl_File_E_None )
    {
        osl_closeFile( m_aFile );
        m_bOpen = false;
    }
    m_aObjects[ n-1 ] = nOffset;
    return aError == osl_File_E_None;
}

#define CHECK_RETURN( x ) if( !(x) ) return 0

sal_Int32 PDFWriterImpl::emitStructParentTree( sal_Int32 nObject )
{
    if( nObject > 0 )
    {
        OStringBuffer aLine( 1024 );

        aLine.append( nObject );
        aLine.append( " 0 obj\n"
                      "<</Nums[\n" );
        sal_Int32 nTreeItems = m_aStructParentTree.size();
        for( sal_Int32 n = 0; n < nTreeItems; n++ )
        {
            aLine.append( n );
            aLine.append( ' ' );
            aLine.append( m_aStructParentTree[n] );
            aLine.append( "\n" );
        }
        aLine.append( "]>>\nendobj\n\n" );
        CHECK_RETURN( updateObject( nObject ) );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }
    return nObject;
}

const sal_Char* PDFWriterImpl::getAttributeTag( PDFWriter::StructAttribute eAttr )
{
    static std::map< PDFWriter::StructAttribute, const char* > aAttributeStrings;
    // fill maps once
    if( aAttributeStrings.empty() )
    {
        aAttributeStrings[ PDFWriter::Placement ]           = "Placement";
        aAttributeStrings[ PDFWriter::WritingMode ]         = "WritingMode";
        aAttributeStrings[ PDFWriter::SpaceBefore ]         = "SpaceBefore";
        aAttributeStrings[ PDFWriter::SpaceAfter ]          = "SpaceAfter";
        aAttributeStrings[ PDFWriter::StartIndent ]         = "StartIndent";
        aAttributeStrings[ PDFWriter::EndIndent ]           = "EndIndent";
        aAttributeStrings[ PDFWriter::TextIndent ]          = "TextIndent";
        aAttributeStrings[ PDFWriter::TextAlign ]           = "TextAlign";
        aAttributeStrings[ PDFWriter::Width ]               = "Width";
        aAttributeStrings[ PDFWriter::Height ]              = "Height";
        aAttributeStrings[ PDFWriter::BlockAlign ]          = "BlockAlign";
        aAttributeStrings[ PDFWriter::InlineAlign ]         = "InlineAlign";
        aAttributeStrings[ PDFWriter::LineHeight ]          = "LineHeight";
        aAttributeStrings[ PDFWriter::BaselineShift ]       = "BaselineShift";
        aAttributeStrings[ PDFWriter::TextDecorationType ]  = "TextDecorationType";
        aAttributeStrings[ PDFWriter::ListNumbering ]       = "ListNumbering";
        aAttributeStrings[ PDFWriter::RowSpan ]             = "RowSpan";
        aAttributeStrings[ PDFWriter::ColSpan ]             = "ColSpan";
        aAttributeStrings[ PDFWriter::LinkAnnotation ]      = "LinkAnnotation";
    }

    std::map< PDFWriter::StructAttribute, const char* >::const_iterator it =
        aAttributeStrings.find( eAttr );

#if OSL_DEBUG_LEVEL > 1
    if( it == aAttributeStrings.end() )
        fprintf( stderr, "invalid PDFWriter::StructAttribute %d\n", eAttr );
#endif

    return it != aAttributeStrings.end() ? it->second : "";
}

const sal_Char* PDFWriterImpl::getAttributeValueTag( PDFWriter::StructAttributeValue eVal )
{
    static std::map< PDFWriter::StructAttributeValue, const char* > aValueStrings;

    if( aValueStrings.empty() )
    {
        aValueStrings[ PDFWriter::NONE ]                    = "None";
        aValueStrings[ PDFWriter::Block ]                   = "Block";
        aValueStrings[ PDFWriter::Inline ]                  = "Inline";
        aValueStrings[ PDFWriter::Before ]                  = "Before";
        aValueStrings[ PDFWriter::After ]                   = "After";
        aValueStrings[ PDFWriter::Start ]                   = "Start";
        aValueStrings[ PDFWriter::End ]                     = "End";
        aValueStrings[ PDFWriter::LrTb ]                    = "LrTb";
        aValueStrings[ PDFWriter::RlTb ]                    = "RlTb";
        aValueStrings[ PDFWriter::TbRl ]                    = "TbRl";
        aValueStrings[ PDFWriter::Center ]                  = "Center";
        aValueStrings[ PDFWriter::Justify ]                 = "Justify";
        aValueStrings[ PDFWriter::Auto ]                    = "Auto";
        aValueStrings[ PDFWriter::Middle ]                  = "Middle";
        aValueStrings[ PDFWriter::Normal ]                  = "Normal";
        aValueStrings[ PDFWriter::Underline ]               = "Underline";
        aValueStrings[ PDFWriter::Overline ]                = "Overline";
        aValueStrings[ PDFWriter::LineThrough ]             = "LineThrough";
        aValueStrings[ PDFWriter::Disc ]                    = "Disc";
        aValueStrings[ PDFWriter::Circle ]                  = "Circle";
        aValueStrings[ PDFWriter::Square ]                  = "Square";
        aValueStrings[ PDFWriter::Decimal ]                 = "Decimal";
        aValueStrings[ PDFWriter::UpperRoman ]              = "UpperRoman";
        aValueStrings[ PDFWriter::LowerRoman ]              = "LowerRoman";
        aValueStrings[ PDFWriter::UpperAlpha ]              = "UpperAlpha";
        aValueStrings[ PDFWriter::LowerAlpha ]              = "LowerAlpha";
    }

    std::map< PDFWriter::StructAttributeValue, const char* >::const_iterator it =
        aValueStrings.find( eVal );

#if OSL_DEBUG_LEVEL > 1
    if( it == aValueStrings.end() )
        fprintf( stderr, "invalid PDFWriter::StructAttributeValue %d\n", eVal );
#endif

    return it != aValueStrings.end() ? it->second : "";
}

static void appendStructureAttributeLine( PDFWriter::StructAttribute eAttr, const PDFWriterImpl::PDFStructureAttribute& rVal, OStringBuffer& rLine )
{
    rLine.append( "/" );
    rLine.append( PDFWriterImpl::getAttributeTag( eAttr ) );

    if( rVal.eValue != PDFWriter::Invalid )
    {
        rLine.append( "/" );
        rLine.append( PDFWriterImpl::getAttributeValueTag( rVal.eValue ) );
    }
    else
    {
        // numerical value
        rLine.append( " " );
        appendFixedInt( rVal.nValue, rLine );
    }
    rLine.append( "\n" );
}

OString PDFWriterImpl::emitStructureAttributes( PDFStructureElement& rEle )
{
    // create layout, list and table attribute sets
    OStringBuffer aLayout(256), aList(64), aTable(64);
    for( PDFStructAttributes::const_iterator it = rEle.m_aAttributes.begin();
         it != rEle.m_aAttributes.end(); ++it )
    {
        if( it->first == PDFWriter::ListNumbering )
            appendStructureAttributeLine( it->first, it->second, aList );
        else if( it->first == PDFWriter::RowSpan ||
                 it->first == PDFWriter::ColSpan )
            appendStructureAttributeLine( it->first, it->second, aTable );
        else if( it->first == PDFWriter::LinkAnnotation )
        {
            sal_Int32 nLink = it->second.nValue;
            std::map< sal_Int32, sal_Int32 >::const_iterator link_it =
                m_aLinkPropertyMap.find( nLink );
            if( link_it != m_aLinkPropertyMap.end() )
                nLink = link_it->second;
            if( nLink >= 0 && nLink < (sal_Int32)m_aLinks.size() )
            {
                // update struct parent of link
                OStringBuffer aStructParentEntry( 32 );
                aStructParentEntry.append( rEle.m_nObject );
                aStructParentEntry.append( " 0 R" );
                m_aStructParentTree.push_back( aStructParentEntry.makeStringAndClear() );
                m_aLinks[ nLink ].m_nStructParent = m_aStructParentTree.size()-1;

                sal_Int32 nRefObject = createObject();
                OStringBuffer aRef( 256 );
                aRef.append( nRefObject );
                aRef.append( " 0 obj\n"
                             "<</Type/OBJR/Obj " );
                aRef.append( m_aLinks[ nLink ].m_nObject );
                aRef.append( " 0 R>>\n"
                             "endobj\n\n"
                             );
                updateObject( nRefObject );
                writeBuffer( aRef.getStr(), aRef.getLength() );

                rEle.m_aKids.push_back( PDFStructureElementKid( nRefObject ) );
            }
            else
            {
                DBG_ERROR( "unresolved link id for Link structure" );
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "unresolved link id %ld for Link structure\n", nLink );
                {
                    OStringBuffer aLine( "unresolved link id " );
                    aLine.append( nLink );
                    aLine.append( " for Link structure" );
                    emitComment( aLine.getStr() );
                }
#endif
            }
        }
        else
            appendStructureAttributeLine( it->first, it->second, aLayout );
    }
    if( ! rEle.m_aBBox.IsEmpty() )
    {
        aLayout.append( "/BBox[" );
        appendFixedInt( rEle.m_aBBox.Left(), aLayout );
        aLayout.append( " " );
        appendFixedInt( rEle.m_aBBox.Top(), aLayout );
        aLayout.append( " " );
        appendFixedInt( rEle.m_aBBox.Right(), aLayout );
        aLayout.append( " " );
        appendFixedInt( rEle.m_aBBox.Bottom(), aLayout );
        aLayout.append( "]\n" );
    }

    std::vector< sal_Int32 > aAttribObjects;
    if( aLayout.getLength() )
    {
        aAttribObjects.push_back( createObject() );
        updateObject( aAttribObjects.back() );
        OStringBuffer aObj( 64 );
        aObj.append( aAttribObjects.back() );
        aObj.append( " 0 obj\n"
                     "<</O/Layout\n" );
        aLayout.append( ">>\nendobj\n\n" );
        writeBuffer( aObj.getStr(), aObj.getLength() );
        writeBuffer( aLayout.getStr(), aLayout.getLength() );
    }
    if( aList.getLength() )
    {
        aAttribObjects.push_back( createObject() );
        updateObject( aAttribObjects.back() );
        OStringBuffer aObj( 64 );
        aObj.append( aAttribObjects.back() );
        aObj.append( " 0 obj\n"
                     "<</O/List\n" );
        aList.append( ">>\nendobj\n\n" );
        writeBuffer( aObj.getStr(), aObj.getLength() );
        writeBuffer( aList.getStr(), aList.getLength() );
    }
    if( aTable.getLength() )
    {
        aAttribObjects.push_back( createObject() );
        updateObject( aAttribObjects.back() );
        OStringBuffer aObj( 64 );
        aObj.append( aAttribObjects.back() );
        aObj.append( " 0 obj\n"
                     "<</O/Table\n" );
        aTable.append( ">>\nendobj\n\n" );
        writeBuffer( aObj.getStr(), aObj.getLength() );
        writeBuffer( aTable.getStr(), aTable.getLength() );
    }

    OStringBuffer aRet( 64 );
    if( aAttribObjects.size() > 1 )
        aRet.append( " [" );
    for( std::vector< sal_Int32 >::const_iterator at_it = aAttribObjects.begin();
         at_it != aAttribObjects.end(); ++at_it )
    {
        aRet.append( " " );
        aRet.append( *at_it );
        aRet.append( " 0 R" );
    }
    if( aAttribObjects.size() > 1 )
        aRet.append( " ]" );
    return aRet.makeStringAndClear();
}

sal_Int32 PDFWriterImpl::emitStructure( PDFStructureElement& rEle )
{
    if(
       // do not emit NonStruct and its children
       rEle.m_eType == PDFWriter::NonStructElement &&
       rEle.m_nOwnElement != rEle.m_nParentElement // but of course emit the struct tree root
       )
        return 0;

    for( std::list< sal_Int32 >::const_iterator it = rEle.m_aChildren.begin(); it != rEle.m_aChildren.end(); ++it )
    {
        if( *it > 0 && *it < sal_Int32(m_aStructure.size()) )
        {
            PDFStructureElement& rChild = m_aStructure[ *it ];
            if( rChild.m_eType != PDFWriter::NonStructElement )
            {
                if( rChild.m_nParentElement == rEle.m_nOwnElement )
                    emitStructure( rChild );
                else
                {
                    DBG_ERROR( "PDFWriterImpl::emitStructure: invalid child structure element" );
#if OSL_DEBUG_LEVEL > 1
                    fprintf( stderr, "PDFWriterImpl::emitStructure: invalid child structure elemnt with id %ld\n", *it );
#endif
                }
            }
        }
        else
        {
            DBG_ERROR( "PDFWriterImpl::emitStructure: invalid child structure id" );
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "PDFWriterImpl::emitStructure: invalid child structure id %ld\n", *it );
#endif
        }
    }

    OStringBuffer aLine( 512 );
    aLine.append( rEle.m_nObject );
    aLine.append( " 0 obj\n"
                  "<</Type" );
    sal_Int32 nParentTree = -1;
    if( rEle.m_nOwnElement == rEle.m_nParentElement )
    {
        nParentTree = createObject();
        CHECK_RETURN( nParentTree );
        aLine.append( "/StructTreeRoot\n" );
        aLine.append( "/ParentTree " );
        aLine.append( nParentTree );
        aLine.append( " 0 R\n" );
    }
    else
    {
        aLine.append( "/StructElem\n"
                      "/S/" );
        aLine.append( getStructureTag( rEle.m_eType ) );
        aLine.append( "\n"
                      "/P " );
        aLine.append( m_aStructure[ rEle.m_nParentElement ].m_nObject );
        aLine.append( " 0 R\n"
                      "/Pg " );
        aLine.append( rEle.m_nFirstPageObject );
        aLine.append( " 0 R\n" );
        if( rEle.m_aActualText.getLength() )
        {
            aLine.append( "/ActualText" );
            appendUnicodeTextString( rEle.m_aActualText, aLine );
            aLine.append( "\n" );
        }
        if( rEle.m_aAltText.getLength() )
        {
            aLine.append( "/Alt" );
            appendUnicodeTextString( rEle.m_aAltText, aLine );
            aLine.append( "\n" );
        }
    }
    if( ! rEle.m_aBBox.IsEmpty() || rEle.m_aAttributes.size() )
    {
        OString aAttribs =  emitStructureAttributes( rEle );
        if( aAttribs.getLength() )
        {
            aLine.append( "/A" );
            aLine.append( aAttribs );
            aLine.append( "\n" );
        }
    }
    if( ! rEle.m_aKids.empty() )
    {
        unsigned int i = 0;
        aLine.append( "/K[" );
        for( std::list< PDFStructureElementKid >::const_iterator it =
                 rEle.m_aKids.begin(); it != rEle.m_aKids.end(); ++it, i++ )
        {
            if( it->nMCID == -1 )
            {
                aLine.append( it->nObject );
                aLine.append( " 0 R" );
                aLine.append( ( (i & 15) == 15 ) ? "\n" : " " );
            }
            else
            {
                if( it->nObject == rEle.m_nFirstPageObject )
                {
                    aLine.append( it->nMCID );
                    aLine.append( " " );
                }
                else
                {
                    aLine.append( "<</Type/MCR/Pg " );
                    aLine.append( it->nObject );
                    aLine.append( " 0 R /MCID " );
                    aLine.append( it->nMCID );
                    aLine.append( ">>\n" );
                }
            }
        }
        aLine.append( "]\n" );
    }
    aLine.append( ">>\nendobj\n\n" );

    CHECK_RETURN( updateObject( rEle.m_nObject ) );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    CHECK_RETURN( emitStructParentTree( nParentTree ) );

    return rEle.m_nObject;
}

bool PDFWriterImpl::emitGradients()
{
    for( std::list<GradientEmit>::iterator it = m_aGradients.begin();
         it != m_aGradients.end(); ++it )
    {
        CHECK_RETURN( writeGradientFunction( *it ) );
    }
    return true;
}

bool PDFWriterImpl::emitTilings()
{
    OStringBuffer aTilingStream( 1024 );
    OStringBuffer aTilingObj( 1024 );

    for( std::list<BitmapPatternEmit>::const_iterator it = m_aTilings.begin(); it != m_aTilings.end(); ++it )
    {
        aTilingStream.setLength( 0 );
        aTilingObj.setLength( 0 );

        sal_Int32 nX = (sal_Int32)it->m_aRectangle.Left();
        sal_Int32 nY = (sal_Int32)it->m_aRectangle.Bottom();
        sal_Int32 nW = (sal_Int32)it->m_aRectangle.GetWidth();
        sal_Int32 nH = (sal_Int32)it->m_aRectangle.GetHeight();

        appendFixedInt( nW, aTilingStream );
        aTilingStream.append( " 0 0 " );
        appendFixedInt( nH, aTilingStream );
        aTilingStream.append( ' ' );
        appendFixedInt( nX, aTilingStream );
        aTilingStream.append( ' ' );
        appendFixedInt( nY, aTilingStream );
        aTilingStream.append( " cm\n/Im" );
        aTilingStream.append( it->m_nBitmapObject );
        aTilingStream.append( " Do\n" );

        // write pattern object
        aTilingObj.append( it->m_nObject );
        aTilingObj.append( " 0 obj\n" );
        aTilingObj.append( "<</Type/Pattern/PatternType 1\n"
                           "/PaintType 1\n"
                           "/TilingType 1\n"
                           "/BBox[" );
        appendFixedInt( nX, aTilingObj );
        aTilingObj.append( ' ' );
        appendFixedInt( nY, aTilingObj );
        aTilingObj.append( ' ' );
        appendFixedInt( nX+nW, aTilingObj );
        aTilingObj.append( ' ' );
        appendFixedInt( nY+nH, aTilingObj );
        aTilingObj.append( "]\n"
                           "/XStep " );
        appendDouble( pixelToPoint(nW), aTilingObj, 1 );
        aTilingObj.append( "\n"
                           "/YStep " );
        appendDouble( pixelToPoint(nH), aTilingObj, 1 );
        aTilingObj.append( "\n"
                           "/Resources<<\n"
                           "/XObject<</Im" );
        aTilingObj.append( it->m_nBitmapObject );
        aTilingObj.append( ' ' );
        aTilingObj.append( it->m_nBitmapObject );
        aTilingObj.append( " 0 R>>>>\n"
                           "/Length " );
        aTilingObj.append( (sal_Int32)aTilingStream.getLength() );
        aTilingObj.append( ">>\nstream\n" );
        CHECK_RETURN( updateObject( it->m_nObject ) );
        CHECK_RETURN( writeBuffer( aTilingObj.getStr(), aTilingObj.getLength() ) );
        CHECK_RETURN( writeBuffer( aTilingStream.getStr(), aTilingStream.getLength() ) );
        aTilingObj.setLength( 0 );
        aTilingObj.append( "\nendstream\nendobj\n\n" );
        CHECK_RETURN( writeBuffer( aTilingObj.getStr(), aTilingObj.getLength() ) );
    }
    return true;
}

sal_Int32 PDFWriterImpl::emitBuiltinFont( ImplFontData* pFont )
{
    const ImplPdfBuiltinFontData* pFD = GetPdfFontData( pFont );
    if( !pFD )
        return 0;
    const BuiltinFont* pBuiltinFont = pFD->GetBuiltinFont();

    OStringBuffer aLine( 1024 );

    sal_Int32 nFontObject = createObject();
    CHECK_RETURN( updateObject( nFontObject ) );
    aLine.append( nFontObject );
    aLine.append( " 0 obj\n"
                  "<</Type/Font/Subtype/Type1/BaseFont/" );
    appendName( pBuiltinFont->m_pPSName, aLine );
    aLine.append( "\n" );
    if( pBuiltinFont->m_eCharSet != RTL_TEXTENCODING_SYMBOL )
         aLine.append( "/Encoding/WinAnsiEncoding\n" );
    aLine.append( ">>\nendobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    return nFontObject;
}

std::map< sal_Int32, sal_Int32 > PDFWriterImpl::emitEmbeddedFont( ImplFontData* pFont, EmbedFont& rEmbed )
{
    std::map< sal_Int32, sal_Int32 > aRet;
    if( isBuiltinFont( pFont ) )
    {
        aRet[ rEmbed.m_nNormalFontID ] = emitBuiltinFont( pFont );
        return aRet;
    }

    sal_Int32 nFontObject = 0;
    sal_Int32 nStreamObject = 0;
    sal_Int32 nFontDescriptor = 0;

    // prepare font encoding
    const std::map< sal_Unicode, sal_Int32 >* pEncoding = m_pReferenceDevice->mpGraphics->GetFontEncodingVector( pFont, NULL );
    sal_Int32 nToUnicodeStream = 0;
    sal_uInt8 nEncoding[256];
    sal_Unicode nEncodedCodes[256];
    if( pEncoding )
    {
        memset( nEncodedCodes, 0, sizeof(nEncodedCodes) );
        memset( nEncoding, 0, sizeof(nEncoding) );
        for( std::map< sal_Unicode, sal_Int32 >::const_iterator it = pEncoding->begin(); it != pEncoding->end(); ++it )
        {
            if( it->second != -1 )
            {
                sal_Int32 nCode = (sal_Int32)(it->second & 0x000000ff);
                nEncoding[ nCode ] = static_cast<sal_uInt8>( nCode );
                nEncodedCodes[ nCode ] = it->first;
            }
        }
    }

    FontSubsetInfo aInfo;
    sal_Int32 pWidths[256];
    const unsigned char* pFontData = NULL;
    long nFontLen = 0;
    sal_Int32 nLength1, nLength2;
    if( (pFontData = (const unsigned char*)m_pReferenceDevice->mpGraphics->GetEmbedFontData( pFont, nEncodedCodes, pWidths, aInfo, &nFontLen )) != NULL )
    {
        if( aInfo.m_nFontType != SAL_FONTSUBSETINFO_TYPE_TYPE1 )
            goto streamend;
        // see whether it is pfb or pfa; if it is a pfb, fill ranges
        // of 6 bytes that are not part of the font program
        std::list< int > aSections;
        std::list< int >::const_iterator it;
        int nIndex = 0;
        while( pFontData[nIndex] == 0x80 )
        {
            aSections.push_back( nIndex );
            if( pFontData[nIndex+1] == 0x03 )
                break;
            sal_Int32 nBytes =
                ((sal_Int32)pFontData[nIndex+2])            |
                ((sal_Int32)pFontData[nIndex+3]) << 8       |
                ((sal_Int32)pFontData[nIndex+4]) << 16      |
                ((sal_Int32)pFontData[nIndex+5]) << 24;
            nIndex += nBytes+6;
        }

        // search for eexec
        nIndex = 0;
        int nEndAsciiIndex;
        int nBeginBinaryIndex;
        int nEndBinaryIndex;
        do
        {
            while( nIndex < nFontLen-4 &&
                   ( pFontData[nIndex] != 'e'   ||
                     pFontData[nIndex+1] != 'e' ||
                     pFontData[nIndex+2] != 'x' ||
                     pFontData[nIndex+3] != 'e' ||
                     pFontData[nIndex+4] != 'c'
                     )
                   )
                nIndex++;
            // check whether we are in a excluded section
            for( it = aSections.begin(); it != aSections.end() && (nIndex < *it || nIndex > ((*it) + 5) ); ++it )
                ;
        } while( it != aSections.end() && nIndex < nFontLen-4 );
        // this should end the ascii part
        if( nIndex > nFontLen-5 )
            goto streamend;

        nEndAsciiIndex = nIndex+4;
        // now count backwards until we can account for 512 '0'
        // which is the endmarker of the (hopefully) binary data
        // do not count the pfb header sections
        int nFound = 0;
        nIndex =  nFontLen-1;
        while( nIndex > 0 && nFound < 512 )
        {
            for( it = aSections.begin(); it != aSections.end() && (nIndex < *it || nIndex > ((*it) + 5) ); ++it )
                ;
            if( it == aSections.end() )
            {
                // inside the 512 '0' block there may only be whitespace
                // according to T1 spec; probably it would be to simple
                // if all fonts complied
                if( pFontData[nIndex] == '0' )
                    nFound++;
                else if( nFound > 0                 &&
                         pFontData[nIndex] != '\r'      &&
                         pFontData[nIndex] != '\t'      &&
                         pFontData[nIndex] != '\n'      &&
                         pFontData[nIndex] != ' ' )
                    break;
            }
            nIndex--;
        }
        if( nIndex < 1 || nIndex <= nEndAsciiIndex )
            goto streamend;
        // there may be whitespace to ignore before the 512 '0'
        while( pFontData[nIndex] == '\r' || pFontData[nIndex] == '\n' )
        {
            nIndex--;
            for( it = aSections.begin(); it != aSections.end() && (nIndex < *it || nIndex > ((*it) + 5) ); ++it )
                ;
            if( it != aSections.end() )
            {
                nIndex = (*it)-1;
                break; // this is surely a binary boundary, in ascii case it wouldn't matter
            }
        }
        nEndBinaryIndex = nIndex;

        // search for beginning of binary section
        nBeginBinaryIndex = nEndAsciiIndex;
        do
        {
            nBeginBinaryIndex++;
            for( it = aSections.begin(); it != aSections.end() && (nBeginBinaryIndex < *it || nBeginBinaryIndex > ((*it) + 5) ); ++it )
                ;
        } while( nBeginBinaryIndex < nEndBinaryIndex &&
                 ( pFontData[nBeginBinaryIndex] == '\r' ||
                   pFontData[nBeginBinaryIndex] == '\n' ||
                   it != aSections.end() ) );

        // it seems to be vital to copy the exact whitespace between binary data
        // and eexec, else a invalid font results. so make nEndAsciiIndex
        // always immediate in front of nBeginBinaryIndex
        nEndAsciiIndex = nBeginBinaryIndex-1;
        for( it = aSections.begin(); it != aSections.end() && (nEndAsciiIndex < *it || nEndAsciiIndex > ((*it)+5)); ++it )
            ;
        if( it != aSections.end() )
            nEndAsciiIndex = (*it)-1;

        nLength1 = nEndAsciiIndex+1; // including the last character
        for( it = aSections.begin(); it != aSections.end() && *it < nEndAsciiIndex; ++it )
            nLength1 -= 6; // decrease by pfb section size

        // if the first four bytes are all ascii hex characters, then binary data
        // has to be converted to real binary data
        for( nIndex = 0; nIndex < 4 &&
                 ( ( pFontData[ nBeginBinaryIndex+nIndex ] >= '0' && pFontData[ nBeginBinaryIndex+nIndex ] <= '9' ) ||
                   ( pFontData[ nBeginBinaryIndex+nIndex ] >= 'a' && pFontData[ nBeginBinaryIndex+nIndex ] <= 'f' ) ||
                   ( pFontData[ nBeginBinaryIndex+nIndex ] >= 'A' && pFontData[ nBeginBinaryIndex+nIndex ] <= 'F' )
                   ); ++nIndex )
            ;
        bool bConvertHexData = true;
        if( nIndex < 4 )
        {
            bConvertHexData = false;
            nLength2 = nEndBinaryIndex - nBeginBinaryIndex + 1; // include the last byte
            for( it = aSections.begin(); it != aSections.end(); ++it )
                if( *it > nBeginBinaryIndex && *it < nEndBinaryIndex )
                    nLength2 -= 6;
        }
        else
        {
            // count the hex ascii characters to get nLength2
            nLength2 = 0;
            int nNextSectionIndex = 0;
            for( it = aSections.begin(); it != aSections.end() && *it < nBeginBinaryIndex; ++it )
                ;
            if( it != aSections.end() )
                nNextSectionIndex = *it;
            for( nIndex = nBeginBinaryIndex; nIndex <= nEndBinaryIndex; nIndex++ )
            {
                if( nIndex == nNextSectionIndex )
                {
                    nIndex += 6;
                    ++it;
                    nNextSectionIndex = (it == aSections.end() ? 0 : *it );
                }
                if( ( pFontData[ nIndex ] >= '0' && pFontData[ nIndex ] <= '9' ) ||
                    ( pFontData[ nIndex ] >= 'a' && pFontData[ nIndex ] <= 'f' ) ||
                    ( pFontData[ nIndex ] >= 'A' && pFontData[ nIndex ] <= 'F' ) )
                    nLength2++;
            }
            DBG_ASSERT( !(nLength2 & 1), "uneven number of hex chars in binary pfa section" );
            nLength2 /= 2;
        }

        // now we can actually write the font stream !
        OStringBuffer aLine( 512 );
        nStreamObject = createObject();
        if( !updateObject(nStreamObject))
            goto streamend;
        sal_Int32 nStreamLengthObject = createObject();
        aLine.append( nStreamObject );
        aLine.append( " 0 obj\n"
                      "<</Length " );
        aLine.append( nStreamLengthObject );
        aLine.append( " 0 R"
#ifndef DEBUG_DISABLE_PDFCOMPRESSION
                      "/Filter/FlateDecode"
#endif
                      "/Length1 " );
        aLine.append( nLength1 );
        aLine.append( " /Length2 " );
        aLine.append( nLength2 );
        aLine.append( " /Length3 0>>\n"
                      "stream\n" );
        if( !writeBuffer( aLine.getStr(), aLine.getLength() ) )
            goto streamend;

        sal_uInt64 nBeginStreamPos = 0;
        osl_getFilePos( m_aFile, &nBeginStreamPos );

        beginCompression();

        // write ascii section
        if( aSections.begin() == aSections.end() )
        {
            if( ! writeBuffer( pFontData, nEndAsciiIndex+1 ) )
                goto streamend;
        }
        else
        {
            // first section always starts at 0
            it = aSections.begin();
            nIndex = (*it)+6;
            ++it;
            while( *it < nEndAsciiIndex )
            {
                if( ! writeBuffer( pFontData+nIndex, (*it)-nIndex ) )
                    goto streamend;
                nIndex = (*it)+6;
                ++it;
            }
            // write partial last section
            if( ! writeBuffer( pFontData+nIndex, nEndAsciiIndex-nIndex+1 ) )
                goto streamend;
        }

        // write binary section
        if( ! bConvertHexData )
        {
            if( aSections.begin() == aSections.end() )
            {
                if( ! writeBuffer( pFontData+nBeginBinaryIndex, nEndBinaryIndex-nBeginBinaryIndex+1 ) )
                    goto streamend;
            }
            else
            {
                for( it = aSections.begin(); *it < nBeginBinaryIndex; ++it )
                    ;
                if( *it > nEndBinaryIndex )
                {
                    if( ! writeBuffer( pFontData+nBeginBinaryIndex, nEndBinaryIndex-nBeginBinaryIndex+1 ) )
                        goto streamend;
                }
                else
                {
                    // write first partial section
                    if( ! writeBuffer( pFontData+nBeginBinaryIndex, (*it) - nBeginBinaryIndex ) )
                        goto streamend;
                    nIndex = (*it)+6;
                    ++it;
                    while( *it < nEndBinaryIndex )
                    {
                        if( ! writeBuffer( pFontData+nIndex, (*it)-nIndex ) )
                            goto streamend;
                        nIndex = (*it)+6;
                        ++it;
                    }
                    // write partial last section
                    if( ! writeBuffer( pFontData+nIndex, nEndBinaryIndex-nIndex+1 ) )
                        goto streamend;
                }
            }
        }
        else
        {
            unsigned char* pWriteBuffer = (unsigned char*)rtl_allocateMemory( nLength2 );
            memset( pWriteBuffer, 0, nLength2 );
            int nWriteIndex = 0;

            int nNextSectionIndex = 0;
            for( it = aSections.begin(); it != aSections.end() && *it < nBeginBinaryIndex; ++it )
                ;
            if( it != aSections.end() )
                nNextSectionIndex = *it;
            for( nIndex = nBeginBinaryIndex; nIndex <= nEndBinaryIndex; nIndex++ )
            {
                if( nIndex == nNextSectionIndex )
                {
                    nIndex += 6;
                    ++it;
                    nNextSectionIndex = (it == aSections.end() ? 0 : *it );
                }
                unsigned char cNibble = 0x80;
                if( pFontData[ nIndex ] >= '0' && pFontData[ nIndex ] <= '9' )
                    cNibble = pFontData[nIndex] - '0';
                else if( pFontData[ nIndex ] >= 'a' && pFontData[ nIndex ] <= 'f' )
                    cNibble = pFontData[nIndex] - 'a' + 10;
                else if( pFontData[ nIndex ] >= 'A' && pFontData[ nIndex ] <= 'F' )
                    cNibble = pFontData[nIndex] - 'A' + 10;
                if( cNibble != 0x80 )
                {
                    if( !(nWriteIndex & 1 ) )
                        cNibble <<= 4;
                    pWriteBuffer[ nWriteIndex/2 ] |= cNibble;
                    nWriteIndex++;
                }
            }
            if( ! writeBuffer( pWriteBuffer, nLength2 ) )
                goto streamend;

            rtl_freeMemory( pWriteBuffer );
        }
        endCompression();


        sal_uInt64 nEndStreamPos = 0;
        osl_getFilePos( m_aFile, &nEndStreamPos );

        // and finally close the stream
        aLine.setLength( 0 );
        aLine.append( "\nendstream\nendobj\n\n" );
        if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
            goto streamend;

        // write stream length object
        aLine.setLength( 0 );
        if( ! updateObject( nStreamLengthObject ) )
            goto streamend;
        aLine.append( nStreamLengthObject );
        aLine.append( " 0 obj\n" );
        aLine.append( (sal_Int64)(nEndStreamPos-nBeginStreamPos ) );
        aLine.append( "\nendobj\n\n" );
        if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
            goto streamend;
    }

    if( nStreamObject )
        // write font descriptor
        nFontDescriptor = emitFontDescriptor( pFont, aInfo, 0, nStreamObject );

    if( nFontDescriptor )
    {
        if( pEncoding )
            nToUnicodeStream = createToUnicodeCMap( nEncoding, nEncodedCodes, sizeof(nEncoding)/sizeof(nEncoding[0]) );

        // write font object
        sal_Int32 nObject = createObject();
        if( ! updateObject( nObject ) )
            goto streamend;

        OStringBuffer aLine( 1024 );
        aLine.append( nObject );
        aLine.append( " 0 obj\n"
                      "<</Type/Font/Subtype/Type1/BaseFont/" );
        appendName( aInfo.m_aPSName, aLine );
        aLine.append( "\n" );
        if( !pFont->mbSymbolFlag &&  pEncoding == 0 )
            aLine.append( "/Encoding/WinAnsiEncoding\n" );
        if( nToUnicodeStream )
        {
            aLine.append( "/ToUnicode " );
            aLine.append( nToUnicodeStream );
            aLine.append( " 0 R\n" );
        }
        aLine.append( "/FirstChar 0 /LastChar 255\n"
                      "/Widths[" );
        for( int i = 0; i < 256; i++ )
        {
            aLine.append( pWidths[i] );
            aLine.append( ((i&15) == 15) ? "\n" : " " );
        }
        aLine.append( "]\n"
                      "/FontDescriptor " );
        aLine.append( nFontDescriptor );
        aLine.append( " 0 R>>\n"
                      "endobj\n\n" );
        if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
            goto streamend;

        nFontObject = nObject;

        aRet[ rEmbed.m_nNormalFontID ] = nObject;

        // write additional encodings
        for( std::list< EmbedEncoding >::iterator enc_it = rEmbed.m_aExtendedEncodings.begin(); enc_it != rEmbed.m_aExtendedEncodings.end(); ++enc_it )
        {
            sal_Int32 aEncWidths[ 256 ];
            // emit encoding dict
            sal_Int32 nEncObject = createObject();
            if( ! updateObject( nEncObject ) )
                goto streamend;

            OutputDevice* pRef = getReferenceDevice();
            pRef->Push( PUSH_FONT | PUSH_MAPMODE );
            pRef->SetMapMode( MapMode( MAP_PIXEL ) );
            Font aFont( pFont->GetFamilyName(), pFont->GetStyleName(), Size( 0, 1000 ) );
            aFont.SetWeight( pFont->GetWeight() );
            aFont.SetItalic( pFont->GetSlant() );
            aFont.SetPitch( pFont->GetPitch() );
            pRef->SetFont( aFont );
            pRef->ImplNewFont();

            aLine.setLength( 0 );
            aLine.append( nEncObject );
            aLine.append( " 0 obj\n"
                          "<</Type/Encoding/Differences[ 0\n" );
            int nEncoded = 0;
            for( std::vector< EmbedCode >::iterator str_it = enc_it->m_aEncVector.begin(); str_it != enc_it->m_aEncVector.end(); ++str_it )
            {
                String aStr( str_it->m_aUnicode );
                aEncWidths[nEncoded] = pRef->GetTextWidth( aStr );
                nEncodedCodes[nEncoded] = str_it->m_aUnicode;
                nEncoding[nEncoded] = sal::static_int_cast<sal_uInt8>(nEncoded);

                aLine.append( " /" );
                aLine.append( str_it->m_aName );
                if( !((++nEncoded) & 15) )
                    aLine.append( "\n" );
            }
            aLine.append( "]>>\n"
                          "endobj\n\n" );

            pRef->Pop();

            if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
                goto streamend;

            nToUnicodeStream = createToUnicodeCMap( nEncoding, nEncodedCodes, nEncoded );

            nObject = createObject();
            if( ! updateObject( nObject ) )
                goto streamend;

            aLine.setLength( 0 );
            aLine.append( nObject );
            aLine.append( " 0 obj\n"
                          "<</Type/Font/Subtype/Type1/BaseFont/" );
            appendName( aInfo.m_aPSName, aLine );
            aLine.append( "\n" );
            aLine.append( "/Encoding " );
            aLine.append( nEncObject );
            aLine.append( " 0 R\n" );
            if( nToUnicodeStream )
            {
                aLine.append( "/ToUnicode " );
                aLine.append( nToUnicodeStream );
                aLine.append( " 0 R\n" );
            }
            aLine.append( "/FirstChar 0\n"
                          "/LastChar " );
            aLine.append( (sal_Int32)(nEncoded-1) );
            aLine.append( "\n"
                          "/Widths[" );
            for( int i = 0; i < nEncoded; i++ )
            {
                aLine.append( aEncWidths[i] );
                aLine.append( ((i&15) == 15) ? "\n" : " " );
            }
            aLine.append( " ]\n"
                          "/FontDescriptor " );
            aLine.append( nFontDescriptor );
            aLine.append( " 0 R>>\n"
                          "endobj\n\n" );
            if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
                goto streamend;

            aRet[ enc_it->m_nFontID ] = nObject;
        }
    }

  streamend:
    if( pFontData )
        m_pReferenceDevice->mpGraphics->FreeEmbedFontData( pFontData, nFontLen );

    return aRet;
}

static void appendSubsetName( int nSubsetID, const OUString& rPSName, OStringBuffer& rBuffer )
{
    if( nSubsetID )
    {
        for( int i = 0; i < 6; i++ )
        {
            int nOffset = (nSubsetID % 26);
            nSubsetID /= 26;
            rBuffer.append( (sal_Char)('A'+nOffset) );
        }
        rBuffer.append( '+' );
    }
    appendName( rPSName, rBuffer );
}

sal_Int32 PDFWriterImpl::createToUnicodeCMap( sal_uInt8* pEncoding, sal_Unicode* pUnicodes, int nGlyphs )
{
    int nMapped = 0, n = 0;
    for( n = 0; n < nGlyphs; n++ )
        if( pUnicodes[n] )
            nMapped++;

    if( nMapped == 0 )
        return 0;

    sal_Int32 nStream = createObject();
    CHECK_RETURN( updateObject( nStream ) );

    OStringBuffer aContents( 1024 );
    aContents.append(
                     "/CIDInit/ProcSet findresource begin\n"
                     "12 dict begin\n"
                     "begincmap\n"
                     "/CIDSystemInfo<<\n"
                     "/Registry (Adobe)\n"
                     "/Ordering (UCS)\n"
                     "/Supplement 0\n"
                     ">> def\n"
                     "/CMapName/Adobe-Identity-UCS def\n"
                     "/CMapType 2 def\n"
                     "1 begincodespacerange\n"
                     "<00> <FF>\n"
                     "endcodespacerange\n"
                     );
    int nCount = 0;
    for( n = 0; n < nGlyphs; n++ )
    {
        if( pUnicodes[n] )
        {
            if( (nCount % 100) == 0 )
            {
                if( nCount )
                    aContents.append( "endbfchar\n" );
                aContents.append( (sal_Int32)((nMapped-nCount > 100) ? 100 : nMapped-nCount ) );
                aContents.append( " beginbfchar\n" );
            }
            aContents.append( '<' );
            appendHex( (sal_Int8)pEncoding[n], aContents );
            aContents.append( "> <" );
            appendHex( (sal_Int8)(pUnicodes[n] / 256), aContents );
            appendHex( (sal_Int8)(pUnicodes[n] & 255), aContents );
            aContents.append( ">\n" );
            nCount++;
        }
    }
    aContents.append( "endbfchar\n"
                      "endcmap\n"
                      "CMapName currentdict /CMap defineresource pop\n"
                      "end\n"
                      "end\n" );
#ifndef DEBUG_DISABLE_PDFCOMPRESSION
    ZCodec* pCodec = new ZCodec( 0x4000, 0x4000 );
    SvMemoryStream aStream;
    pCodec->BeginCompression();
    pCodec->Write( aStream, (const BYTE*)aContents.getStr(), aContents.getLength() );
    pCodec->EndCompression();
    delete pCodec;
#endif

    OStringBuffer aLine( 40 );

    aLine.append( nStream );
    aLine.append( " 0 obj\n<</Length " );
#ifndef DEBUG_DISABLE_PDFCOMPRESSION
    sal_Int32 nLen = (sal_Int32)aStream.Tell();
    aStream.Seek( 0 );
    aLine.append( nLen );
    aLine.append( "/Filter/FlateDecode" );
#else
    aLine.append( aContents.getLength() );
#endif
    aLine.append( ">>\nstream\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
#ifndef DEBUG_DISABLE_PDFCOMPRESSION
    CHECK_RETURN( writeBuffer( aStream.GetData(), nLen ) );
#else
    CHECK_RETURN( writeBuffer( aContents.getStr(), aContents.getLength() ) );
#endif
    aLine.setLength( 0 );
    aLine.append( "\nendstream\n"
                  "endobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    return nStream;
}

sal_Int32 PDFWriterImpl::emitFontDescriptor( ImplFontData* pFont, FontSubsetInfo& rInfo, sal_Int32 nSubsetID, sal_Int32 nFontStream )
{
    OStringBuffer aLine( 1024 );
    // get font flags, see PDF reference 1.4 p. 358
    // possibly characters outside Adobe standard encoding
    // so set Symbolic flag
    sal_Int32 nFontFlags = (1<<2);
    if( pFont->GetSlant() == ITALIC_NORMAL || pFont->GetSlant() == ITALIC_OBLIQUE )
        nFontFlags |= (1 << 6);
    if( pFont->GetPitch() == PITCH_FIXED )
        nFontFlags |= 1;
    if( pFont->GetFamilyType() == FAMILY_SCRIPT )
        nFontFlags |= (1 << 3);
    else if( pFont->GetFamilyType() == FAMILY_ROMAN )
        nFontFlags |= (1 << 1);

    sal_Int32 nFontDescriptor = createObject();
    CHECK_RETURN( updateObject( nFontDescriptor ) );
    aLine.setLength( 0 );
    aLine.append( nFontDescriptor );
    aLine.append( " 0 obj\n"
                  "<</Type/FontDescriptor/FontName/" );
    appendSubsetName( nSubsetID, rInfo.m_aPSName, aLine );
    aLine.append( "\n"
                  "/Flags " );
    aLine.append( nFontFlags );
    aLine.append( "\n"
                  "/FontBBox[" );
    // note: Top and Bottom are reversed in VCL and PDF rectangles
    aLine.append( (sal_Int32)rInfo.m_aFontBBox.TopLeft().X() );
    aLine.append( ' ' );
    aLine.append( (sal_Int32)rInfo.m_aFontBBox.TopLeft().Y() );
    aLine.append( ' ' );
    aLine.append( (sal_Int32)rInfo.m_aFontBBox.BottomRight().X() );
    aLine.append( ' ' );
    aLine.append( (sal_Int32)(rInfo.m_aFontBBox.BottomRight().Y()+1) );
    aLine.append( "]/ItalicAngle " );
    if( pFont->GetSlant() == ITALIC_OBLIQUE || pFont->GetSlant() == ITALIC_NORMAL )
        aLine.append( "-30" );
    else
        aLine.append( "0" );
    aLine.append( "\n"
                  "/Ascent " );
    aLine.append( (sal_Int32)rInfo.m_nAscent );
    aLine.append( "\n"
                  "/Descent " );
    aLine.append( (sal_Int32)-rInfo.m_nDescent );
    aLine.append( "\n"
                  "/CapHeight " );
    aLine.append( (sal_Int32)rInfo.m_nCapHeight );
    // According to PDF reference 1.4 StemV is required
    // seems a tad strange to me, but well ...
    aLine.append( "\n"
                  "/StemV 80\n"
                  "/FontFile" );
    switch( rInfo.m_nFontType )
    {
        case SAL_FONTSUBSETINFO_TYPE_TRUETYPE:
            aLine.append( '2' );
            break;
        case SAL_FONTSUBSETINFO_TYPE_TYPE1:
            break;
        default:
            DBG_ERROR( "unknown fonttype in PDF font descriptor" );
            return 0;
    }
    aLine.append( ' ' );
    aLine.append( nFontStream );
    aLine.append( " 0 R>>\n"
                  "endobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    return nFontDescriptor;
}

sal_Int32 PDFWriterImpl::emitFonts()
{
    sal_Int32 nFontDict = 0;

    if( ! m_pReferenceDevice->ImplGetGraphics() )
        return 0;

    OStringBuffer aLine( 1024 );
    char buf[8192];

    std::map< sal_Int32, sal_Int32 > aFontIDToObject;

    OUString aTmpName;
    osl_createTempFile( NULL, NULL, &aTmpName.pData );
    for( FontSubsetData::iterator it = m_aSubsets.begin(); it != m_aSubsets.end(); ++it )
    {
        for( FontEmitList::iterator lit = it->second.m_aSubsets.begin(); lit != it->second.m_aSubsets.end(); ++lit )
        {
            sal_Int32 pGlyphIDs[ 256 ];
            sal_Int32 pWidths[ 256 ];
            sal_uInt8 pEncoding[ 256 ];
            sal_Unicode pUnicodes[ 256 ];
            int nGlyphs = 1;
            // fill arrays and prepare encoding index map
            sal_Int32 nToUnicodeStream = 0;

            memset( pGlyphIDs, 0, sizeof( pGlyphIDs ) );
            memset( pEncoding, 0, sizeof( pEncoding ) );
            memset( pUnicodes, 0, sizeof( pUnicodes ) );
            for( FontEmitMapping::iterator fit = lit->m_aMapping.begin(); fit != lit->m_aMapping.end();++fit )
            {
                sal_uInt8 nEnc = fit->second.m_nSubsetGlyphID;

                DBG_ASSERT( pGlyphIDs[nEnc] == 0 && pEncoding[nEnc] == 0, "duplicate glyph" );
                DBG_ASSERT( nEnc <= lit->m_aMapping.size(), "invalid glyph encoding" );

                pGlyphIDs[ nEnc ] = fit->first;
                pEncoding[ nEnc ] = nEnc;
                pUnicodes[ nEnc ] = fit->second.m_aUnicode;
                if( pUnicodes[ nEnc ] )
                    nToUnicodeStream = 1;
                if( nGlyphs < 256 )
                    nGlyphs++;
                else
                {
                    DBG_ERROR( "too many glyphs for subset" );
                }
            }
            FontSubsetInfo aSubsetInfo;
            if( m_pReferenceDevice->mpGraphics->CreateFontSubset( aTmpName, it->first, pGlyphIDs, pEncoding, pWidths, nGlyphs, aSubsetInfo ) )
            {
                DBG_ASSERT( aSubsetInfo.m_nFontType == SAL_FONTSUBSETINFO_TYPE_TRUETYPE, "wrong font type in font subset" );
                // create font stream
                oslFileHandle aFontFile;
                CHECK_RETURN( (osl_File_E_None == osl_openFile( aTmpName.pData, &aFontFile, osl_File_OpenFlag_Read ) ) );
                // get file size
                sal_uInt64 nLength;
                CHECK_RETURN( (osl_File_E_None == osl_setFilePos( aFontFile, osl_Pos_End, 0 ) ) );
                CHECK_RETURN( (osl_File_E_None == osl_getFilePos( aFontFile, &nLength ) ) );
                CHECK_RETURN( (osl_File_E_None == osl_setFilePos( aFontFile, osl_Pos_Absolut, 0 ) ) );

                sal_Int32 nFontStream = createObject();
                sal_Int32 nStreamLengthObject = createObject();
                CHECK_RETURN( updateObject( nFontStream ) );
                aLine.setLength( 0 );
                aLine.append( nFontStream );
                aLine.append( " 0 obj\n"
                              "<</Length " );
                aLine.append( (sal_Int32)nStreamLengthObject );
                aLine.append( " 0 R"
#ifndef DEBUG_DISABLE_PDFCOMPRESSION
                              "/Filter/FlateDecode"
#endif
                              "/Length1 " );
                aLine.append( (sal_Int32)nLength );
                aLine.append( ">>\n"
                              "stream\n" );
                CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

                sal_uInt64 nStartPos = 0;
                CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nStartPos ) ) );

                // copy font file
                beginCompression();
                sal_uInt64 nRead;
                sal_Bool bEOF = sal_False;
                do
                {
                    CHECK_RETURN( (osl_File_E_None == osl_readFile( aFontFile, buf, sizeof( buf ), &nRead ) ) );
                    CHECK_RETURN( writeBuffer( buf, nRead ) );
                    CHECK_RETURN( (osl_File_E_None == osl_isEndOfFile( aFontFile, &bEOF ) ) );
                } while( ! bEOF );
                endCompression();
                // close the file
                osl_closeFile( aFontFile );

                sal_uInt64 nEndPos = 0;
                CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nEndPos ) ) );
                // end the stream
                aLine.setLength( 0 );
                aLine.append( "\nendstream\nendobj\n\n" );
                CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

                // emit stream length object
                CHECK_RETURN( updateObject( nStreamLengthObject ) );
                aLine.setLength( 0 );
                aLine.append( nStreamLengthObject );
                aLine.append( " 0 obj\n" );
                aLine.append( (sal_Int64)(nEndPos-nStartPos) );
                aLine.append( "\nendobj\n\n" );
                CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

                // write font descriptor
                sal_Int32 nFontDescriptor = emitFontDescriptor( it->first, aSubsetInfo, lit->m_nFontID, nFontStream );

                if( nToUnicodeStream )
                    nToUnicodeStream = createToUnicodeCMap( pEncoding, pUnicodes, nGlyphs );

                sal_Int32 nFontObject = createObject();
                CHECK_RETURN( updateObject( nFontObject ) );
                aLine.setLength( 0 );
                aLine.append( nFontObject );
                aLine.append( " 0 obj\n"
                              "<</Type/Font/Subtype/TrueType/BaseFont/" );
                appendSubsetName( lit->m_nFontID, aSubsetInfo.m_aPSName, aLine );
                aLine.append( "\n"
                              "/FirstChar 0\n"
                              "/LastChar " );
                aLine.append( (sal_Int32)(nGlyphs-1) );
                aLine.append( "\n"
                              "/Widths[" );
                for( int i = 0; i < nGlyphs; i++ )
                {
                    aLine.append( pWidths[ i ] );
                    aLine.append( ((i & 15) == 15) ? "\n" : " " );
                }
                aLine.append( "]\n"
                              "/FontDescriptor " );
                aLine.append( nFontDescriptor );
                aLine.append( " 0 R\n" );
                if( nToUnicodeStream )
                {
                    aLine.append( "/ToUnicode " );
                    aLine.append( nToUnicodeStream );
                    aLine.append( " 0 R\n" );
                }
                aLine.append( ">>\n"
                              "endobj\n\n" );
                CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

                aFontIDToObject[ lit->m_nFontID ] = nFontObject;
            }
        }
    }
    osl_removeFile( aTmpName.pData );

    // emit embedded fonts
    for( FontEmbedData::iterator eit = m_aEmbeddedFonts.begin(); eit != m_aEmbeddedFonts.end(); ++eit )
    {
        std::map< sal_Int32, sal_Int32 > aObjects = emitEmbeddedFont( eit->first, eit->second );
        for( std::map< sal_Int32, sal_Int32 >::iterator fit = aObjects.begin(); fit != aObjects.end(); ++fit )
        {
            CHECK_RETURN( fit->second );
            aFontIDToObject[ fit->first ] = fit->second;
        }
    }

    nFontDict = getFontDictObj();
    aLine.setLength( 0 );
    aLine.append( nFontDict );
    aLine.append( " 0 obj\n"
                  "<<\n" );
    for( std::map< sal_Int32, sal_Int32 >::iterator mit = aFontIDToObject.begin(); mit != aFontIDToObject.end(); ++mit )
    {
        aLine.append( "/F" );
        aLine.append( mit->first );
        aLine.append( ' ' );
        aLine.append( mit->second );
        aLine.append( " 0 R\n" );
    }
    // emit helvetica and ZapfDingbats font for widget apperances / variable text
    if( ! m_aWidgets.empty() )
    {
        ImplPdfBuiltinFontData aHelvData(m_aBuiltinFonts[4]);
        aLine.append( "/HelvReg " );
        aLine.append( emitBuiltinFont( &aHelvData ) );
        aLine.append( " 0 R\n" );
        ImplPdfBuiltinFontData aZapfData(m_aBuiltinFonts[13]);
        aLine.append( "/ZaDb " );
        aLine.append( emitBuiltinFont( &aZapfData ) );
        aLine.append( " 0 R\n" );
    }
    aLine.append( ">>\n"
                  "endobj\n\n" );
    CHECK_RETURN( updateObject( nFontDict ) );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    return nFontDict;
}

sal_Int32 PDFWriterImpl::emitResources()
{
    OStringBuffer aLine( 512 );

    // emit shadings
    sal_Int32 nShadingDict = 0;
    if( m_aGradients.begin() != m_aGradients.end() )
    {
        CHECK_RETURN( emitGradients() );
        aLine.setLength( 0 );
        aLine.append( nShadingDict = createObject() );
        aLine.append( " 0 obj\n"
                      "<<" );
        for( std::list<GradientEmit>::iterator it = m_aGradients.begin();
             it != m_aGradients.end(); ++it )
        {
            aLine.append( "/P" );
            aLine.append( it->m_nObject );
            aLine.append( ' ' );
            aLine.append( it->m_nObject );
            aLine.append( " 0 R\n" );
        }
        aLine.append( ">>\n"
                      "endobj\n\n" );
        CHECK_RETURN( updateObject( nShadingDict ) );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }

    // emit patterns
    sal_Int32 nPatternDict = 0;
    if( m_aTilings.begin() != m_aTilings.end() )
    {
        CHECK_RETURN( emitTilings() );
        aLine.setLength( 0 );
        aLine.append( nPatternDict = createObject() );
        aLine.append( " 0 obj\n<<" );
        for( std::list<BitmapPatternEmit>::const_iterator tile = m_aTilings.begin();
             tile != m_aTilings.end(); ++tile )
        {
            aLine.append( "/P" );
            aLine.append( tile->m_nObject );
            aLine.append( ' ' );
            aLine.append( tile->m_nObject );
            aLine.append( " 0 R\n" );
        }
        aLine.append( ">>\n"
                      "endobj\n\n" );
        CHECK_RETURN( updateObject( nPatternDict ) );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }

    // emit font dict
    sal_Int32 nFontDict = emitFonts();

    // emit xobject dict
    sal_Int32 nXObjectDict = 0;
    std::list< sal_Int32 > aExtGStates;
    if( m_aBitmaps.begin() != m_aBitmaps.end()      ||
        m_aJPGs.begin() != m_aJPGs.end()            ||
        m_aTransparentObjects.begin() != m_aTransparentObjects.end()
        )
    {
        aLine.setLength( 0 );
        nXObjectDict = createObject();
        aLine.append( nXObjectDict );
        aLine.append( " 0 obj\n"
                      "<<" );
        for( std::list<BitmapEmit>::const_iterator it = m_aBitmaps.begin();
             it != m_aBitmaps.end(); ++it )
        {
            aLine.append( "/Im" );
            aLine.append( it->m_nObject );
            aLine.append( ' ' );
            aLine.append( it->m_nObject );
            aLine.append( " 0 R\n" );
        }
        for( std::list<JPGEmit>::const_iterator jpeg = m_aJPGs.begin(); jpeg != m_aJPGs.end(); ++jpeg )
        {
            aLine.append( "/Im" );
            aLine.append( jpeg->m_nObject );
            aLine.append( ' ' );
            aLine.append( jpeg->m_nObject );
            aLine.append( " 0 R\n" );
        }
        for( std::list<TransparencyEmit>::const_iterator t = m_aTransparentObjects.begin();
             t != m_aTransparentObjects.end(); ++t )
        {
            aLine.append( "/Tr" );
            aLine.append( t->m_nObject );
            aLine.append( ' ' );
            aLine.append( t->m_nObject );
            aLine.append( " 0 R\n" );
            if( t->m_nExtGStateObject > 0 )
                aExtGStates.push_back( t->m_nExtGStateObject );
        }

        aLine.append( ">>\n"
                      "endobj\n\n" );
        CHECK_RETURN( updateObject( nXObjectDict ) );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }

    // emit ExtGStates
    sal_Int32 nExtGStateObject = 0;
    if( !aExtGStates.empty() )
    {
        nExtGStateObject = createObject();
        CHECK_RETURN( updateObject( nExtGStateObject ) );
        aLine.setLength( 0 );
        aLine.append( nExtGStateObject );
        aLine.append( " 0 obj\n"
                      "<<" );
        int i = 0;
        while( ! aExtGStates.empty() )
        {
            aLine.append( "/EGS" );
            aLine.append( aExtGStates.front() );
            aLine.append( " " );
            aLine.append( aExtGStates.front() );
            aLine.append( " 0 R" );
            aLine.append( ((i%5) == 4) ? "\n" : " " );
            aExtGStates.pop_front();
            i++;
        }
        aLine.append( "\n>>\nendobj\n\n" );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }

    // emit Resource dict
    sal_Int32 nResourceDict = getResourceDictObj();
    CHECK_RETURN( updateObject( nResourceDict ) );
    aLine.setLength( 0 );
    aLine.append( nResourceDict );
    aLine.append( " 0 obj\n<<" );
    if( nFontDict )
    {
        aLine.append( "/Font " );
        aLine.append( nFontDict );
        aLine.append( " 0 R\n" );
    }
    if( nXObjectDict )
    {
        aLine.append( "/XObject " );
        aLine.append( nXObjectDict );
        aLine.append( " 0 R\n" );
    }
    if( nExtGStateObject )
    {
        aLine.append( "/ExtGState " );
        aLine.append( nExtGStateObject );
        aLine.append( " 0 R\n" );
    }
    if( nShadingDict )
    {
        aLine.append( "/Shading " );
        aLine.append( nShadingDict );
        aLine.append( " 0 R\n" );
    }
    if( nPatternDict )
    {
        aLine.append( "/Pattern " );
        aLine.append( nPatternDict );
        aLine.append( " 0 R\n" );
    }
    aLine.append( "/ProcSet[/PDF/Text" );
    if( nXObjectDict )
        aLine.append( "/ImageC/ImageI/ImageB" );
    aLine.append( "]>>\n"
                  "endobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    return nResourceDict;
}

sal_Int32 PDFWriterImpl::emitOutline()
{
    int i, nItems = m_aOutline.size();

    // do we have an outline at all ?
    if( nItems < 2 )
        return 0;

    // reserve object numbers for all outline items
    for( i = 0; i < nItems; ++i )
        m_aOutline[i].m_nObject = createObject();

    // update all parent, next and prev object ids
    for( i = 0; i < nItems; ++i )
    {
        PDFOutlineEntry& rItem = m_aOutline[i];
        int nChildren = rItem.m_aChildren.size();

        if( nChildren )
        {
            for( int n = 0; n < nChildren; ++n )
            {
                PDFOutlineEntry& rChild = m_aOutline[ rItem.m_aChildren[n] ];

                rChild.m_nParentObject = rItem.m_nObject;
                rChild.m_nPrevObject = (n > 0) ? m_aOutline[ rItem.m_aChildren[n-1] ].m_nObject : 0;
                rChild.m_nNextObject = (n < nChildren-1) ? m_aOutline[ rItem.m_aChildren[n+1] ].m_nObject : 0;
            }

        }
    }

    // emit hierarchy
    for( i = 0; i < nItems; ++i )
    {
        PDFOutlineEntry& rItem = m_aOutline[i];
        OStringBuffer aLine( 1024 );

        CHECK_RETURN( updateObject( rItem.m_nObject ) );
        aLine.append( rItem.m_nObject );
        aLine.append( " 0 obj\n" );
        aLine.append( "<<" );
        if( ! rItem.m_aChildren.empty() )
        {
            // children list: Count, First, Last
            aLine.append( "/Count " );
            aLine.append( (sal_Int32)rItem.m_aChildren.size() );
            aLine.append( "/First " );
            aLine.append( m_aOutline[rItem.m_aChildren.front()].m_nObject );
            aLine.append( " 0 R/Last " );
            aLine.append( m_aOutline[rItem.m_aChildren.back()].m_nObject );
            aLine.append( " 0 R\n" );
        }
        if( i > 0 )
        {
            // Title, Dest, Parent, Prev, Next
            aLine.append( "/Title" );
            appendUnicodeTextString( rItem.m_aTitle, aLine );
            aLine.append( "\n" );
            // Dest is not required
            if( rItem.m_nDestID >= 0 && rItem.m_nDestID < (sal_Int32)m_aDests.size() )
            {
                aLine.append( "/Dest" );
                appendDest( rItem.m_nDestID, aLine );
            }
            aLine.append( "/Parent " );
            aLine.append( rItem.m_nParentObject );
            aLine.append( " 0 R" );
            if( rItem.m_nPrevObject )
            {
                aLine.append( "/Prev " );
                aLine.append( rItem.m_nPrevObject );
                aLine.append( " 0 R" );
            }
            if( rItem.m_nNextObject )
            {
                aLine.append( "/Next " );
                aLine.append( rItem.m_nNextObject );
                aLine.append( " 0 R" );
            }
        }
        aLine.append( ">>\nendobj\n\n" );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }

    return m_aOutline[0].m_nObject;
}

#undef CHECK_RETURN
#define CHECK_RETURN( x ) if( !x ) return false

bool PDFWriterImpl::appendDest( sal_Int32 nDestID, OStringBuffer& rBuffer )
{
    if( nDestID < 0 || nDestID >= (sal_Int32)m_aDests.size() )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "ERROR: invalid dest %d requested\n", (int)nDestID );
#endif
        return false;
    }


    const PDFDest& rDest        = m_aDests[ nDestID ];
    const PDFPage& rDestPage    = m_aPages[ rDest.m_nPage ];

    rBuffer.append( '[' );
    rBuffer.append( rDestPage.m_nPageObject );
    rBuffer.append( " 0 R" );

    switch( rDest.m_eType )
    {
        case PDFWriter::XYZ:
        default:
            rBuffer.append( "/XYZ " );
            appendFixedInt( rDest.m_aRect.Left(), rBuffer );
            rBuffer.append( ' ' );
            appendFixedInt( rDest.m_aRect.Bottom(), rBuffer );
            rBuffer.append( " 0" );
            break;
        case PDFWriter::Fit:
            rBuffer.append( "/Fit" );
            break;
        case PDFWriter::FitRectangle:
            rBuffer.append( "/FitR " );
            appendFixedInt( rDest.m_aRect.Left(), rBuffer );
            rBuffer.append( ' ' );
            appendFixedInt( rDest.m_aRect.Top(), rBuffer );
            rBuffer.append( ' ' );
            appendFixedInt( rDest.m_aRect.Right(), rBuffer );
            rBuffer.append( ' ' );
            appendFixedInt( rDest.m_aRect.Bottom(), rBuffer );
            break;
        case PDFWriter::FitHorizontal:
            rBuffer.append( "/FitH " );
            appendFixedInt( rDest.m_aRect.Bottom(), rBuffer );
            break;
        case PDFWriter::FitVertical:
            rBuffer.append( "/FitV " );
            appendFixedInt( rDest.m_aRect.Left(), rBuffer );
            break;
        case PDFWriter::FitPageBoundingBox:
            rBuffer.append( "/FitB" );
            break;
        case PDFWriter::FitPageBoundingBoxHorizontal:
            rBuffer.append( "/FitBH " );
            appendFixedInt( rDest.m_aRect.Bottom(), rBuffer );
            break;
        case PDFWriter::FitPageBoundingBoxVertical:
            rBuffer.append( "/FitBV " );
            appendFixedInt( rDest.m_aRect.Left(), rBuffer );
            break;
    }
    rBuffer.append( ']' );

    return true;
}

bool PDFWriterImpl::emitLinkAnnotations()
{
    int nAnnots = m_aLinks.size();
    for( int i = 0; i < nAnnots; i++ )
    {
        const PDFLink& rLink            = m_aLinks[i];
        if( ! updateObject( rLink.m_nObject ) )
            continue;

        OStringBuffer aLine( 1024 );
        aLine.append( rLink.m_nObject );
        aLine.append( " 0 obj\n" );
        aLine.append( "<</Type/Annot/Subtype/Link/Border[0 0 0]/Rect[" );

        appendFixedInt( rLink.m_aRect.Left()-7, aLine );//the +7 to have a better shape of the border rectangle
        aLine.append( ' ' );
        appendFixedInt( rLink.m_aRect.Top(), aLine );
        aLine.append( ' ' );
        appendFixedInt( rLink.m_aRect.Right()+7, aLine );//the +7 to have a better shape of the border rectangle
        aLine.append( ' ' );
        appendFixedInt( rLink.m_aRect.Bottom(), aLine );
        aLine.append( "]" );
        if( rLink.m_nDest >= 0 )
        {
            aLine.append( "/Dest" );
            appendDest( rLink.m_nDest, aLine );
        }
        else
        {
            aLine.append( "/A<</Type/Action/S/URI\n"
                          "/URI(" );
            aLine.append( rtl::OUStringToOString( rLink.m_aURL, RTL_TEXTENCODING_ASCII_US ) );
            aLine.append( ")>>\n" );
        }
        if( rLink.m_nStructParent > 0 )
        {
            aLine.append( "/StructParent " );
            aLine.append( rLink.m_nStructParent );
        }
        aLine.append( ">>\nendobj\n\n" );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }

    return true;
}

bool PDFWriterImpl::emitNoteAnnotations()
{
    // emit note annotations
    int nAnnots = m_aNotes.size();
    for( int i = 0; i < nAnnots; i++ )
    {
        const PDFNoteEntry& rNote       = m_aNotes[i];
        if( ! updateObject( rNote.m_nObject ) )
            return false;

        OStringBuffer aLine( 1024 );
        aLine.append( rNote.m_nObject );
        aLine.append( " 0 obj\n" );
        aLine.append( "<</Type/Annot/Subtype/Text/Rect[" );

        appendFixedInt( rNote.m_aRect.Left(), aLine );
        aLine.append( ' ' );
        appendFixedInt( rNote.m_aRect.Top(), aLine );
        aLine.append( ' ' );
        appendFixedInt( rNote.m_aRect.Right(), aLine );
        aLine.append( ' ' );
        appendFixedInt( rNote.m_aRect.Bottom(), aLine );
        aLine.append( "]" );

        // contents of the note (type text string)
        aLine.append( "/Contents\n" );
        appendUnicodeTextString( rNote.m_aContents.Contents, aLine );
        aLine.append( "\n" );

        // optional title
        if( rNote.m_aContents.Title.Len() )
        {
            aLine.append( "/T" );
            appendUnicodeTextString( rNote.m_aContents.Title, aLine );
            aLine.append( "\n" );
        }

        aLine.append( ">>\nendobj\n\n" );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }
    return true;
}

Font PDFWriterImpl::replaceFont( const Font& rControlFont, const Font&  rAppSetFont )
{
    bool bAdjustSize = false;

    Font aFont( rControlFont );
    if( ! aFont.GetName().Len() )
    {
        aFont = rAppSetFont;
        if( rControlFont.GetHeight() )
            aFont.SetSize( Size( 0, rControlFont.GetHeight() ) );
        else
            bAdjustSize = true;
    }
    else if( ! aFont.GetHeight() )
    {
        aFont.SetSize( rAppSetFont.GetSize() );
        bAdjustSize = true;
    }
    if( bAdjustSize )
    {
        Size aFontSize = aFont.GetSize();
        OutputDevice* pDefDev = Application::GetDefaultDevice();
        aFontSize = OutputDevice::LogicToLogic( aFontSize, pDefDev->GetMapMode(), getMapMode() );
        aFont.SetSize( aFontSize );
    }
    return aFont;
}

static inline const Color& replaceColor( const Color& rCol1, const Color& rCol2 )
{
    return (rCol1 == Color( COL_TRANSPARENT )) ? rCol2 : rCol1;
}

void PDFWriterImpl::createDefaultPushButtonAppearance( PDFWidget& rButton, const PDFWriter::PushButtonWidget& rWidget )
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();

    // save graphics state
    push( sal::static_int_cast<sal_uInt16>(~0U) );

    // transform relative to control's coordinates since an
    // appearance stream is a form XObject
    // this relies on the m_aRect member of rButton NOT already being transformed
    // to default user space
    if( rWidget.Background || rWidget.Border )
    {
        setLineColor( rWidget.Border ? replaceColor( rWidget.BorderColor, rSettings.GetLightColor() ) : Color( COL_TRANSPARENT ) );
        setFillColor( rWidget.Background ? replaceColor( rWidget.BackgroundColor, rSettings.GetDialogColor() ) : Color( COL_TRANSPARENT ) );
        drawRectangle( rWidget.Location );
    }
    // prepare font to use
    Font aFont = replaceFont( rWidget.TextFont, rSettings.GetPushButtonFont() );
    setFont( aFont );
    setTextColor( replaceColor( rWidget.TextColor, rSettings.GetButtonTextColor() ) );

    drawText( rButton.m_aRect, rButton.m_aText, rButton.m_nTextStyle );

    // create DA string while local mapmode is still in place
    // (that is before endRedirect())
    OStringBuffer aDA( 256 );
    appendNonStrokingColor( replaceColor( rWidget.TextColor, rSettings.GetButtonTextColor() ), aDA );
    aDA.append( " /HelvReg " );
    m_aPages[m_nCurrentPage].appendMappedLength( sal_Int32( aFont.GetHeight() ), aDA );
    aDA.append( " Tf" );
    rButton.m_aDAString = aDA.makeStringAndClear();

    pop();

    rButton.m_aAppearances[ "N" ][ "Standard" ] = new SvMemoryStream();

    /* seems like a bad hack but at least works in both AR5 and 6:
       we draw the button ourselves and tell AR
       the button would be totally transparent with no text

       One would expect that simply setting a normal appearance
       should suffice, but no, as soon as the user actually presses
       the button and an action is tied to it (gasp! a button that
       does something) the appearance gets replaced by some crap that AR
       creates on the fly even if no DA or MK is given. On AR6 at least
       the DA and MK work as expected, but on AR5 this creates a region
       fille with the background color but nor text. Urgh.
    */
    rButton.m_aMKDict = "/BC [] /BG [] /CA ()";
}

Font PDFWriterImpl::drawFieldBorder( PDFWidget& rIntern,
                                     const PDFWriter::AnyWidget& rWidget,
                                     const StyleSettings& rSettings )
{
    Font aFont = replaceFont( rWidget.TextFont, rSettings.GetFieldFont() );
    aFont.SetName( String( RTL_CONSTASCII_USTRINGPARAM( "Helvetica" ) ) );

    if( rWidget.Background || rWidget.Border )
    {
        if( rWidget.Border && rWidget.BorderColor == Color( COL_TRANSPARENT ) )
        {
            sal_Int32 nDelta = getReferenceDevice()->ImplGetDPIX() / 500;
            if( nDelta < 1 )
                nDelta = 1;
            setLineColor( Color( COL_TRANSPARENT ) );
            Rectangle aRect = rIntern.m_aRect;
            setFillColor( rSettings.GetLightBorderColor() );
            drawRectangle( aRect );
            aRect.Left()  += nDelta; aRect.Top()     += nDelta;
            aRect.Right() -= nDelta; aRect.Bottom()  -= nDelta;
            setFillColor( rSettings.GetFieldColor() );
            drawRectangle( aRect );
            setFillColor( rSettings.GetLightColor() );
            drawRectangle( Rectangle( Point( aRect.Left(), aRect.Bottom()-nDelta ), aRect.BottomRight() ) );
            drawRectangle( Rectangle( Point( aRect.Right()-nDelta, aRect.Top() ), aRect.BottomRight() ) );
            setFillColor( rSettings.GetDarkShadowColor() );
            drawRectangle( Rectangle( aRect.TopLeft(), Point( aRect.Left()+nDelta, aRect.Bottom() ) ) );
            drawRectangle( Rectangle( aRect.TopLeft(), Point( aRect.Right(), aRect.Top()+nDelta ) ) );
        }
        else
        {
            setLineColor( rWidget.Border ? replaceColor( rWidget.BorderColor, rSettings.GetShadowColor() ) : Color( COL_TRANSPARENT ) );
            setFillColor( rWidget.Background ? replaceColor( rWidget.BackgroundColor, rSettings.GetFieldColor() ) : Color( COL_TRANSPARENT ) );
            drawRectangle( rIntern.m_aRect );
        }

        if( rWidget.Border )
        {
            // adjust edit area accounting for border
            sal_Int32 nDelta = aFont.GetHeight()/4;
            if( nDelta < 1 )
                nDelta = 1;
            rIntern.m_aRect.Left()  += nDelta;
            rIntern.m_aRect.Top()   += nDelta;
            rIntern.m_aRect.Right() -= nDelta;
            rIntern.m_aRect.Bottom()-= nDelta;
        }
    }
    return aFont;
}

void PDFWriterImpl::createDefaultEditAppearance( PDFWidget& rEdit, const PDFWriter::EditWidget& rWidget )
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    SvMemoryStream* pEditStream = new SvMemoryStream( 1024, 1024 );

    push( sal::static_int_cast<sal_uInt16>(~0U) );

    // prepare font to use, draw field border
    Font aFont = drawFieldBorder( rEdit, rWidget, rSettings );

    // prepare DA string
    OStringBuffer aDA( 32 );
    appendNonStrokingColor( replaceColor( rWidget.TextColor, rSettings.GetFieldTextColor() ), aDA );
    aDA.append( " /HelvReg " );
    m_aPages[ m_nCurrentPage ].appendMappedLength( sal_Int32( aFont.GetHeight() ), aDA );
    aDA.append( " Tf" );

    /*  create an empty appearance stream, let the viewer create
        the appearance at runtime. This is because AR5 seems to
        paint the widget appearance always, and a dynamically created
        appearance on top of it. AR6 is well behaved in that regard, so
        that behaviour seems to be a bug. Anyway this empty appearance
        relies on /NeedAppearances in the AcroForm dictionary set to "true"
     */
    beginRedirect( pEditStream, rEdit.m_aRect );
    OStringBuffer aAppearance( 32 );
    aAppearance.append( "/Tx BMC\nEMC\n" );
    writeBuffer( aAppearance.getStr(), aAppearance.getLength() );

    endRedirect();
    pop();

    rEdit.m_aAppearances[ "N" ][ "Standard" ] = pEditStream;

    rEdit.m_aDAString = aDA.makeStringAndClear();
}

void PDFWriterImpl::createDefaultListBoxAppearance( PDFWidget& rBox, const PDFWriter::ListBoxWidget& rWidget )
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    SvMemoryStream* pListBoxStream = new SvMemoryStream( 1024, 1024 );

    push( sal::static_int_cast<sal_uInt16>(~0U) );

    // prepare font to use, draw field border
    Font aFont = drawFieldBorder( rBox, rWidget, rSettings );

    beginRedirect( pListBoxStream, rBox.m_aRect );
    OStringBuffer aAppearance( 64 );

#if 0
    if( ! rWidget.DropDown )
    {
        // prepare linewidth for DA string hack, see below
        Size aFontSize = lcl_convert( m_aGraphicsStack.front().m_aMapMode,
                                      m_aMapMode,
                                      getReferenceDevice(),
                                      Size( 0, aFont.GetHeight() ) );
        sal_Int32 nLW = aFontSize.Height() / 40;
        appendFixedInt( nLW > 0 ? nLW : 1, aAppearance );
        aAppearance.append( " w\n" );
        writeBuffer( aAppearance.getStr(), aAppearance.getLength() );
        aAppearance.setLength( 0 );
    }
#endif

    setLineColor( Color( COL_TRANSPARENT ) );
    setFillColor( replaceColor( rWidget.BackgroundColor, rSettings.GetFieldColor() ) );
    drawRectangle( rBox.m_aRect );

    // empty appearance, see createDefaultEditAppearance for reference
    aAppearance.append( "/Tx BMC\nEMC\n" );
    writeBuffer( aAppearance.getStr(), aAppearance.getLength() );

    endRedirect();
    pop();

    rBox.m_aAppearances[ "N" ][ "Standard" ] = pListBoxStream;

    // prepare DA string
    OStringBuffer aDA( 256 );
#if 0
    if( !rWidget.DropDown )
    {
        /* another of AR5's peculiarities: the selected item of a choice
           field is highlighted using the non stroking color - same as the
           text color. so workaround that by using text rendering mode 2
           (fill, then stroke) and set the stroking color
         */
        appendStrokingColor( replaceColor( rWidget.BackgroundColor, rSettings.GetFieldColor() ), aDA );
        aDA.append( " 2 Tr " );
    }
#endif
    appendNonStrokingColor( replaceColor( rWidget.TextColor, rSettings.GetFieldTextColor() ), aDA );
    aDA.append( " /HelvReg " );
    m_aPages[ m_nCurrentPage ].appendMappedLength( sal_Int32( aFont.GetHeight() ), aDA );
    aDA.append( " Tf" );
    rBox.m_aDAString = aDA.makeStringAndClear();
}

void PDFWriterImpl::createDefaultCheckBoxAppearance( PDFWidget& rBox, const PDFWriter::CheckBoxWidget& rWidget )
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();

    // save graphics state
    push( sal::static_int_cast<sal_uInt16>(~0U) );

    if( rWidget.Background || rWidget.Border )
    {
        setLineColor( rWidget.Border ? replaceColor( rWidget.BorderColor, rSettings.GetCheckedColor() ) : Color( COL_TRANSPARENT ) );
        setFillColor( rWidget.Background ? replaceColor( rWidget.BackgroundColor, rSettings.GetFieldColor() ) : Color( COL_TRANSPARENT ) );
        drawRectangle( rBox.m_aRect );
    }

    Font aFont = replaceFont( rWidget.TextFont, rSettings.GetRadioCheckFont() );
    setFont( aFont );
    Size aFontSize = aFont.GetSize();
    sal_Int32 nDelta = aFontSize.Height()/10;
    if( nDelta < 1 )
        nDelta = 1;

    Rectangle aCheckRect, aTextRect;
    if( rWidget.ButtonIsLeft )
    {
        aCheckRect.Left()   = rBox.m_aRect.Left() + nDelta;
        aCheckRect.Top()    = rBox.m_aRect.Top() + (rBox.m_aRect.GetHeight()-aFontSize.Height())/2;
        aCheckRect.Right()  = aCheckRect.Left() + aFontSize.Height();
        aCheckRect.Bottom() = aCheckRect.Top() + aFontSize.Height();

        aTextRect.Left()    = rBox.m_aRect.Left() + aCheckRect.GetWidth()+5*nDelta;
        aTextRect.Top()     = rBox.m_aRect.Top();
        aTextRect.Right()   = aTextRect.Left() + rBox.m_aRect.GetWidth() - aCheckRect.GetWidth()-6*nDelta;
        aTextRect.Bottom()  = rBox.m_aRect.Bottom();
    }
    else
    {
        aCheckRect.Left()   = rBox.m_aRect.Right() - nDelta - aFontSize.Height();
        aCheckRect.Top()    = rBox.m_aRect.Top() + (rBox.m_aRect.GetHeight()-aFontSize.Height())/2;
        aCheckRect.Right()  = aCheckRect.Left() + aFontSize.Height();
        aCheckRect.Bottom() = aCheckRect.Top() + aFontSize.Height();

        aTextRect.Left()    = rBox.m_aRect.Left();
        aTextRect.Top()     = rBox.m_aRect.Top();
        aTextRect.Right()   = aTextRect.Left() + rBox.m_aRect.GetWidth() - aCheckRect.GetWidth()-6*nDelta;
        aTextRect.Bottom()  = rBox.m_aRect.Bottom();
    }
    setLineColor( Color( COL_BLACK ) );
    setFillColor( Color( COL_TRANSPARENT ) );
    OStringBuffer aLW( 32 );
    aLW.append( "q " );
    m_aPages[m_nCurrentPage].appendMappedLength( nDelta, aLW );
    aLW.append( " w " );
    writeBuffer( aLW.getStr(), aLW.getLength() );
    drawRectangle( aCheckRect );
    writeBuffer( " Q\n", 3 );
    setTextColor( replaceColor( rWidget.TextColor, rSettings.GetRadioCheckTextColor() ) );
    drawText( aTextRect, rBox.m_aText, rBox.m_nTextStyle );

    pop();

    OStringBuffer aDA( 256 );
    appendNonStrokingColor( replaceColor( rWidget.TextColor, rSettings.GetRadioCheckTextColor() ), aDA );
    aDA.append( " /ZaDb 0 Tf" );
    rBox.m_aDAString = aDA.makeStringAndClear();
    rBox.m_aMKDict = "/CA (8)";

    rBox.m_aRect = aCheckRect;

    // create appearance streams
    sal_Char cMark = '8';
    sal_Int32 nCharXOffset = 1000-m_aBuiltinFonts[13].m_aWidths[sal_Int32(cMark)];
    nCharXOffset *= aCheckRect.GetHeight();
    nCharXOffset /= 2000;
    sal_Int32 nCharYOffset = 1000-
        (m_aBuiltinFonts[13].m_nAscent+m_aBuiltinFonts[13].m_nDescent); // descent is negative
    nCharYOffset *= aCheckRect.GetHeight();
    nCharYOffset /= 2000;

    SvMemoryStream* pCheckStream = new SvMemoryStream( 256, 256 );
    beginRedirect( pCheckStream, aCheckRect );
    aDA.append( "/Tx BMC\nq BT\n" );
    appendNonStrokingColor( replaceColor( rWidget.TextColor, rSettings.GetRadioCheckTextColor() ), aDA );
    aDA.append( " /ZaDb " );
    m_aPages[ m_nCurrentPage ].appendMappedLength( sal_Int32( aCheckRect.GetHeight() ), aDA );
    aDA.append( " Tf\n" );
    m_aPages[ m_nCurrentPage ].appendMappedLength( nCharXOffset, aDA );
    aDA.append( " " );
    m_aPages[ m_nCurrentPage ].appendMappedLength( nCharYOffset, aDA );
    aDA.append( " Td (" );
    aDA.append( cMark );
    aDA.append( ") Tj\nET\nQ\nEMC\n" );
    writeBuffer( aDA.getStr(), aDA.getLength() );
    endRedirect();
    rBox.m_aAppearances[ "N" ][ "Yes" ] = pCheckStream;

    SvMemoryStream* pUncheckStream = new SvMemoryStream( 256, 256 );
    beginRedirect( pUncheckStream, aCheckRect );
    writeBuffer( "/Tx BMC\nEMC\n", 12 );
    endRedirect();
    rBox.m_aAppearances[ "N" ][ "Off" ] = pUncheckStream;
}

void PDFWriterImpl::createDefaultRadioButtonAppearance( PDFWidget& rBox, const PDFWriter::RadioButtonWidget& rWidget )
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();

    // save graphics state
    push( sal::static_int_cast<sal_uInt16>(~0U) );

    if( rWidget.Background || rWidget.Border )
    {
        setLineColor( rWidget.Border ? replaceColor( rWidget.BorderColor, rSettings.GetCheckedColor() ) : Color( COL_TRANSPARENT ) );
        setFillColor( rWidget.Background ? replaceColor( rWidget.BackgroundColor, rSettings.GetFieldColor() ) : Color( COL_TRANSPARENT ) );
        drawRectangle( rBox.m_aRect );
    }

    Font aFont = replaceFont( rWidget.TextFont, rSettings.GetRadioCheckFont() );
    setFont( aFont );
    Size aFontSize = aFont.GetSize();
    sal_Int32 nDelta = aFontSize.Height()/10;
    if( nDelta < 1 )
        nDelta = 1;

    Rectangle aCheckRect, aTextRect;
    if( rWidget.ButtonIsLeft )
    {
        aCheckRect.Left()   = rBox.m_aRect.Left() + nDelta;
        aCheckRect.Top()    = rBox.m_aRect.Top() + (rBox.m_aRect.GetHeight()-aFontSize.Height())/2;
        aCheckRect.Right()  = aCheckRect.Left() + aFontSize.Height();
        aCheckRect.Bottom() = aCheckRect.Top() + aFontSize.Height();

        aTextRect.Left()    = rBox.m_aRect.Left() + aCheckRect.GetWidth()+5*nDelta;
        aTextRect.Top()     = rBox.m_aRect.Top();
        aTextRect.Right()   = aTextRect.Left() + rBox.m_aRect.GetWidth() - aCheckRect.GetWidth()-6*nDelta;
        aTextRect.Bottom()  = rBox.m_aRect.Bottom();
    }
    else
    {
        aCheckRect.Left()   = rBox.m_aRect.Right() - nDelta - aFontSize.Height();
        aCheckRect.Top()    = rBox.m_aRect.Top() + (rBox.m_aRect.GetHeight()-aFontSize.Height())/2;
        aCheckRect.Right()  = aCheckRect.Left() + aFontSize.Height();
        aCheckRect.Bottom() = aCheckRect.Top() + aFontSize.Height();

        aTextRect.Left()    = rBox.m_aRect.Left();
        aTextRect.Top()     = rBox.m_aRect.Top();
        aTextRect.Right()   = aTextRect.Left() + rBox.m_aRect.GetWidth() - aCheckRect.GetWidth()-6*nDelta;
        aTextRect.Bottom()  = rBox.m_aRect.Bottom();
    }
    setLineColor( Color( COL_BLACK ) );
    setFillColor( Color( COL_TRANSPARENT ) );
    OStringBuffer aLW( 32 );
    aLW.append( "q " );
    m_aPages[ m_nCurrentPage ].appendMappedLength( nDelta, aLW );
    aLW.append( " w " );
    writeBuffer( aLW.getStr(), aLW.getLength() );
    drawEllipse( aCheckRect );
    writeBuffer( " Q\n", 3 );
    setTextColor( replaceColor( rWidget.TextColor, rSettings.GetRadioCheckTextColor() ) );
    drawText( aTextRect, rBox.m_aText, rBox.m_nTextStyle );

    pop();

    OStringBuffer aDA( 256 );
    appendNonStrokingColor( replaceColor( rWidget.TextColor, rSettings.GetRadioCheckTextColor() ), aDA );
    aDA.append( " /ZaDb 0 Tf" );
    rBox.m_aDAString = aDA.makeStringAndClear();
    rBox.m_aMKDict = "/CA (l)";

    rBox.m_aRect = aCheckRect;

    // create appearance streams
    push( sal::static_int_cast<sal_uInt16>(~0U) );
    SvMemoryStream* pCheckStream = new SvMemoryStream( 256, 256 );

    beginRedirect( pCheckStream, aCheckRect );
    aDA.append( "/Tx BMC\nq BT\n" );
    appendNonStrokingColor( replaceColor( rWidget.TextColor, rSettings.GetRadioCheckTextColor() ), aDA );
    aDA.append( " /ZaDb " );
    m_aPages[m_nCurrentPage].appendMappedLength( sal_Int32( aCheckRect.GetHeight() ), aDA );
    aDA.append( " Tf\n0 0 Td\nET\nQ\n" );
    writeBuffer( aDA.getStr(), aDA.getLength() );
    setFillColor( replaceColor( rWidget.TextColor, rSettings.GetRadioCheckTextColor() ) );
    setLineColor( Color( COL_TRANSPARENT ) );
    aCheckRect.Left()   += 3*nDelta;
    aCheckRect.Top()    += 3*nDelta;
    aCheckRect.Bottom() -= 3*nDelta;
    aCheckRect.Right()  -= 3*nDelta;
    drawEllipse( aCheckRect );
    writeBuffer( "\nEMC\n", 5 );
    endRedirect();

    pop();
    rBox.m_aAppearances[ "N" ][ "Yes" ] = pCheckStream;

    SvMemoryStream* pUncheckStream = new SvMemoryStream( 256, 256 );
    beginRedirect( pUncheckStream, aCheckRect );
    writeBuffer( "/Tx BMC\nEMC\n", 12 );
    endRedirect();
    rBox.m_aAppearances[ "N" ][ "Off" ] = pUncheckStream;
}

bool PDFWriterImpl::emitAppearances( PDFWidget& rWidget, OStringBuffer& rAnnotDict )
{

    // TODO: check and insert default streams
    rtl::OString aStandardAppearance;
    switch( rWidget.m_eType )
    {
        case PDFWriter::CheckBox:
            aStandardAppearance = OUStringToOString( rWidget.m_aValue, RTL_TEXTENCODING_ASCII_US );
            break;
        default:
            break;
    }

    if( rWidget.m_aAppearances.size() )
    {
        rAnnotDict.append( "/AP<<\n" );
        for( PDFAppearanceMap::iterator dict_it = rWidget.m_aAppearances.begin(); dict_it != rWidget.m_aAppearances.end(); ++dict_it )
        {
            rAnnotDict.append( "/" );
            rAnnotDict.append( dict_it->first );
            bool bUseSubDict = (dict_it->second.size() > 1);
            rAnnotDict.append( bUseSubDict ? "<<" : " " );

            for( PDFAppearanceStreams::const_iterator stream_it = dict_it->second.begin();
                 stream_it != dict_it->second.end(); ++stream_it )
            {
                SvMemoryStream* pApppearanceStream = stream_it->second;
                dict_it->second[ stream_it->first ] = NULL;

                bool bDeflate = compressStream( pApppearanceStream );

                pApppearanceStream->Seek( STREAM_SEEK_TO_END );
                sal_Int64 nStreamLen = pApppearanceStream->Tell();
                pApppearanceStream->Seek( STREAM_SEEK_TO_BEGIN );
                sal_Int32 nObject = createObject();
                CHECK_RETURN( updateObject( nObject ) );
                OStringBuffer aLine;
                aLine.append( nObject );

                aLine.append( " 0 obj\n"
                              "<</Type/XObject\n"
                              "/Subtype/Form\n"
                              "/BBox[0 0 " );
                appendFixedInt( rWidget.m_aRect.GetWidth()-1, aLine );
                aLine.append( " " );
                appendFixedInt( rWidget.m_aRect.GetHeight()-1, aLine );
                aLine.append( "]\n"
                              "/Resources " );
                aLine.append( getResourceDictObj() );
                aLine.append( " 0 R\n"
                              "/Length " );
                aLine.append( nStreamLen );
                aLine.append( "\n" );
                if( bDeflate )
                    aLine.append( "/Filter/FlateDecode\n" );
                aLine.append( ">>\nstream\n" );
                CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
                CHECK_RETURN( writeBuffer( pApppearanceStream->GetData(), nStreamLen ) );
                CHECK_RETURN( writeBuffer( "\nendstream\nendobj\n\n", 19 ) );

                if( bUseSubDict )
                {
                    rAnnotDict.append( " /" );
                    rAnnotDict.append( stream_it->first );
                    rAnnotDict.append( " " );
                }
                rAnnotDict.append( nObject );
                rAnnotDict.append( " 0 R" );

                delete pApppearanceStream;
            }

            rAnnotDict.append( bUseSubDict ? ">>\n" : "\n" );
        }
        rAnnotDict.append( ">>\n" );
        if( aStandardAppearance.getLength() )
        {
            rAnnotDict.append( "/AS /" );
            rAnnotDict.append( aStandardAppearance );
            rAnnotDict.append( "\n" );
        }
    }

    return true;
}

bool PDFWriterImpl::emitWidgetAnnotations()
{
    ensureUniqueRadioOnValues();

    int nAnnots = m_aWidgets.size();
    for( int a = 0; a < nAnnots; a++ )
    {
        PDFWidget& rWidget = m_aWidgets[a];

        OStringBuffer aLine( 1024 );
        OStringBuffer aValue( 256 );
        aLine.append( rWidget.m_nObject );
        aLine.append( " 0 obj\n"
                      "<<" );
        // emit widget annotation only for terminal fields
        if( rWidget.m_aKids.empty() )
        {
            aLine.append( "/Type/Annot/Subtype/Widget/F 4\n"
                          "/Rect[" );
            appendFixedInt( rWidget.m_aRect.Left()-1, aLine );
            aLine.append( ' ' );
            appendFixedInt( rWidget.m_aRect.Top()+1, aLine );
            aLine.append( ' ' );
            appendFixedInt( rWidget.m_aRect.Right()+1, aLine );
            aLine.append( ' ' );
            appendFixedInt( rWidget.m_aRect.Bottom()-1, aLine );
            aLine.append( "]\n" );
        }
        aLine.append( "/FT/" );
        switch( rWidget.m_eType )
        {
            case PDFWriter::RadioButton:
            case PDFWriter::CheckBox:
                // for radio buttons only the RadioButton field, not the
                // CheckBox children should have a value, else acrobat reader
                // does not always check the right button
                // of course real check boxes (not belonging to a readio group)
                // need their values, too
                if( rWidget.m_eType == PDFWriter::RadioButton || rWidget.m_nRadioGroup < 0 )
                {
                    aValue.append( "/" );
                    // check for radio group with all buttons unpressed
                    if( rWidget.m_aValue.getLength() == 0 )
                        aValue.append( "Off" );
                    else
                        appendName( rWidget.m_aValue, aValue );
                }
            case PDFWriter::PushButton:
                aLine.append( "Btn" );
                break;
            case PDFWriter::ListBox:
            case PDFWriter::ComboBox:
                if( rWidget.m_nFlags & 0x200000 ) // multiselect
                {
                    aValue.append( "[" );
                    appendUnicodeTextString( rWidget.m_aValue, aValue );
                    aValue.append( "]" );
                }
                else
                    appendUnicodeTextString( rWidget.m_aValue, aValue );
                aLine.append( "Ch" );
                break;
            case PDFWriter::Edit:
                aLine.append( "Tx" );
                appendUnicodeTextString( rWidget.m_aValue, aValue );
                break;
        }
        aLine.append( "\n" );
        aLine.append( "/P " );
        aLine.append( m_aPages[ rWidget.m_nPage ].m_nPageObject );
        aLine.append( " 0 R\n" );

        if( rWidget.m_nParent )
        {
            aLine.append( "/Parent " );
            aLine.append( rWidget.m_nParent );
            aLine.append( " 0 R\n" );
        }
        if( rWidget.m_aKids.size() )
        {
            aLine.append( "/Kids[" );
            for( unsigned int i = 0; i < rWidget.m_aKids.size(); i++ )
            {
                aLine.append( rWidget.m_aKids[i] );
                aLine.append( " 0 R" );
                aLine.append( ( (i&15) == 15 ) ? "\n" : " " );
            }
            aLine.append( "]\n" );
        }
        if( rWidget.m_aName.getLength() )
        {
            aLine.append( "/T (" );
            aLine.append( rWidget.m_aName );
            aLine.append( ")\n" );
        }
        if( m_aContext.Version > PDFWriter::PDF_1_2 )
        {
            // the alternate field name should be unicode able since it is
            // supposed to be used in UI
            aLine.append( "/TU" );
            appendUnicodeTextString( rWidget.m_aDescription, aLine );
            aLine.append( "\n" );
        }

        if( rWidget.m_nFlags )
        {
            aLine.append( "/Ff " );
            aLine.append( rWidget.m_nFlags );
            aLine.append( "\n" );
        }
        if( aValue.getLength() )
        {
            OString aVal = aValue.makeStringAndClear();
            aLine.append( "/V " );
            aLine.append( aVal );
            aLine.append( "\n"
                          "/DV " );
            aLine.append( aVal );
            aLine.append( "\n" );
        }
        if( rWidget.m_eType == PDFWriter::ListBox || rWidget.m_eType == PDFWriter::ComboBox )
        {
            sal_Int32 nTI = -1;
            aLine.append( "/Opt[\n" );
            sal_Int32 i = 0;
            for( std::list< OUString >::const_iterator it = rWidget.m_aListEntries.begin(); it != rWidget.m_aListEntries.end(); ++it, ++i )
            {
                appendUnicodeTextString( *it, aLine );
                aLine.append( "\n" );
                if( *it == rWidget.m_aValue )
                    nTI = i;
            }
            aLine.append( "]\n" );
            if( nTI > 0 )
            {
                aLine.append( "/TI " );
                aLine.append( nTI );
                aLine.append( "\n" );
                if( rWidget.m_nFlags & 0x200000 ) // Multiselect
                {
                    aLine.append( "/I [" );
                    aLine.append( nTI );
                    aLine.append( "]\n" );
                }
            }
        }
        if( rWidget.m_eType == PDFWriter::Edit && rWidget.m_nMaxLen > 0 )
        {
            aLine.append( "/MaxLen " );
            aLine.append( rWidget.m_nMaxLen );
            aLine.append( "\n" );
        }
        if( rWidget.m_eType == PDFWriter::PushButton )
        {
            OStringBuffer aDest;
            if( appendDest( rWidget.m_nDest, aDest ) )
            {
                aLine.append( "/AA<</D<</Type/Action/S/GoTo/D " );
                aLine.append( aDest.makeStringAndClear() );
                aLine.append( ">>>>\n" );
            }
            else if( rWidget.m_aListEntries.empty() )
            {
                // create a reset form action
                aLine.append( "/AA<</D<</Type/Action/S/ResetForm>>>>\n" );
            }
            else if( rWidget.m_bSubmit )
            {
                // create a submit form action
                aLine.append( "/AA<</D<</Type/Action/S/SubmitForm/F(" );
                aLine.append( OUStringToOString( rWidget.m_aListEntries.front(), RTL_TEXTENCODING_ASCII_US ) );
                aLine.append( ")/Flags " );
                sal_Int32 nFlags = 0;
                switch( m_aContext.SubmitFormat )
                {
                    case PDFWriter::HTML:
                        nFlags |= 4;
                        break;
                    case PDFWriter::XML:
                        if( m_aContext.Version > PDFWriter::PDF_1_3 )
                            nFlags |= 32;
                        break;
                    case PDFWriter::PDF:
                        if( m_aContext.Version > PDFWriter::PDF_1_3 )
                            nFlags |= 256;
                        break;
                    case PDFWriter::FDF:
                    default:
                        break;
                }
                aLine.append( nFlags );
                aLine.append( ">>>>\n" );
            }
            else
            {
                // create a URI action
                aLine.append( "/AA<</D<</Type/Action/S/URI/URI(" );
                aLine.append( OUStringToOString( rWidget.m_aListEntries.front(), RTL_TEXTENCODING_ASCII_US ) );
                aLine.append( ")>>>>\n" );
            }
        }
        if( rWidget.m_aDAString.getLength() )
        {
            aLine.append( "/DR<</Font " );
            aLine.append( getFontDictObj() );
            aLine.append( " 0 R>>\n" );
            aLine.append( "/DA(" );
            aLine.append( rWidget.m_aDAString );
            aLine.append( ")\n" );
            if( rWidget.m_nTextStyle & TEXT_DRAW_CENTER )
                aLine.append( "/Q 1\n" );
            else if( rWidget.m_nTextStyle & TEXT_DRAW_RIGHT )
                aLine.append( "/Q 2\n" );

        }

        // appearance charactristics for terminal fields
        // which are supposed to have an appearance constructed
        // by the viewer application
        if( rWidget.m_aMKDict.getLength() )
        {
            aLine.append( "/MK<<" );
            aLine.append( rWidget.m_aMKDict );
            aLine.append( ">>\n" );
        }

        CHECK_RETURN( emitAppearances( rWidget, aLine ) );

        aLine.append( ">>\n"
                      "endobj\n\n" );
        CHECK_RETURN( updateObject( rWidget.m_nObject ) );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }
    return true;
}

bool PDFWriterImpl::emitAnnotations()
{
    if( m_aPages.size() < 1 )
        return false;

    CHECK_RETURN( emitLinkAnnotations() );

    CHECK_RETURN( emitNoteAnnotations() );

    CHECK_RETURN( emitWidgetAnnotations() );

    return true;
}

#undef CHECK_RETURN
#define CHECK_RETURN( x ) if( !x ) return false

bool PDFWriterImpl::emitCatalog()
{
    // build page tree
    // currently there is only one node that contains all leaves

    // first create a page tree node id
    sal_Int32 nTreeNode = createObject();

    // emit global resource dictionary (page emit needs it)
    CHECK_RETURN( emitResources() );

    // emit all pages
    for( std::vector<PDFPage>::iterator it = m_aPages.begin(); it != m_aPages.end(); ++it )
        if( ! it->emit( nTreeNode ) )
            return false;

    sal_Int32 nOutlineDict = emitOutline();

    sal_Int32 nStructureDict = 0;
    if(m_aStructure.size() > 1)
    {
        nStructureDict = m_aStructure[0].m_nObject = createObject();
        emitStructure( m_aStructure[ 0 ] );
    }

    // adjust tree node file offset
    if( ! updateObject( nTreeNode ) )
        return false;

    // emit tree node
    OStringBuffer aLine( 2048 );
    aLine.append( nTreeNode );
    aLine.append( " 0 obj\n" );
    aLine.append( "<</Type/Pages\n" );
    aLine.append( "/Resources " );
    aLine.append( getResourceDictObj() );
    aLine.append( " 0 R\n" );

    switch( m_eInheritedOrientation )
    {
        case PDFWriter::Landscape: aLine.append( "/Rotate 90\n" );break;
        case PDFWriter::Seascape: aLine.append( "/Rotate -90\n" );break;

        case PDFWriter::Inherit: // actually Inherit would be a bug, but insignificant
        case PDFWriter::Portrait:
        default:
            break;
    }
    aLine.append( "/MediaBox[ 0 0 " );
    aLine.append( m_nInheritedPageWidth );
    aLine.append( ' ' );
    aLine.append( m_nInheritedPageHeight );
    aLine.append( " ]\n"
                  "/Kids[ " );
    unsigned int i = 0;
    for( std::vector<PDFPage>::const_iterator iter = m_aPages.begin(); iter != m_aPages.end(); ++iter, i++ )
    {
        aLine.append( iter->m_nPageObject );
        aLine.append( " 0 R" );
        aLine.append( ( (i&15) == 15 ) ? "\n" : " " );
    }
    aLine.append( "]\n"
                  "/Count " );
    aLine.append( (sal_Int32)m_aPages.size() );
    aLine.append( ">>\n"
                  "endobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    // emit annotation objects
    CHECK_RETURN( emitAnnotations() );

    // emit Catalog
    m_nCatalogObject = createObject();
    if( ! updateObject( m_nCatalogObject ) )
        return false;
    aLine.setLength( 0 );
    aLine.append( m_nCatalogObject );
    aLine.append( " 0 obj\n"
                  "<</Type/Catalog/Pages " );
    aLine.append( nTreeNode );
    aLine.append( " 0 R\n" );

    if( m_aContext.PageLayout != PDFWriter::DefaultLayout )
        switch(  m_aContext.PageLayout )
        {
        default :
        case  PDFWriter::SinglePage :
            aLine.append( "/PageLayout/SinglePage\n" );
            break;
        case  PDFWriter::Continuous :
            aLine.append( "/PageLayout/OneColumn\n" );
            break;
        case  PDFWriter::ContinuousFacing :
//the flag m_aContext.FirstPageLeft below is used to set the page on the left side
            aLine.append( "/PageLayout/TwoColumnRight\n" );//odd page on the right side
            break;
        }
    if( m_aContext.PDFDocumentMode != PDFWriter::ModeDefault && !m_aContext.OpenInFullScreenMode )
        switch(  m_aContext.PDFDocumentMode )
        {
        default :
            aLine.append( "/PageMode/UseNone\n" );
            break;
        case PDFWriter::UseOutlines :
            aLine.append( "/PageMode/UseOutlines\n" ); //document is opened with outline pane open
            break;
        case PDFWriter::UseThumbs :
            aLine.append( "/PageMode/UseThumbs\n" ); //document is opened with thumbnails pane open
            break;
        }
    else if( m_aContext.OpenInFullScreenMode )
        aLine.append( "/PageMode/FullScreen\n" ); //document is opened full screen

    switch( m_aContext.PDFDocumentAction )
    {
    case PDFWriter::ActionDefault :     //do nothing, this is the Acrobat default
    default:
        break;
    case PDFWriter::FitInWindow :
        aLine.append( "/OpenAction[0 /Fit]\n" ); //Open fit page
        break;
    case PDFWriter::FitWidth :
        aLine.append( "/OpenAction[0 /FitH " );
        aLine.append( m_nInheritedPageHeight );//Open fit width
        aLine.append( "]\n" );
        break;
    case PDFWriter::FitVisible :
        aLine.append( "/OpenAction[0 /FitBH " );
        aLine.append( m_nInheritedPageHeight );//Open fit visible
        aLine.append( "]\n" );
        break;
    }
// viewer preferences, if we had some, then emit
    if( m_aContext.HideViewerToolbar ||
        ( m_aContext.Version > PDFWriter::PDF_1_3 && m_aDocInfo.Title.Len() && m_aContext.DisplayPDFDocumentTitle ) ||
        m_aContext.HideViewerMenubar ||
        m_aContext.HideViewerWindowControls || m_aContext.FitWindow ||
        m_aContext.CenterWindow || (m_aContext.FirstPageLeft  &&  m_aContext.PageLayout == PDFWriter::ContinuousFacing ) ||
        m_aContext.OpenInFullScreenMode )
    {
        aLine.append( "/ViewerPreferences<<" );
        if( m_aContext.HideViewerToolbar )
            aLine.append( "/HideToolbar true\n" );
        if( m_aContext.HideViewerMenubar )
            aLine.append( "/HideMenubar true\n" );
        if( m_aContext.HideViewerWindowControls )
            aLine.append( "/HideWindowUI true\n" );
        if( m_aContext.FitWindow )
            aLine.append("/FitWindow true\n");
        if( m_aContext.CenterWindow )
            aLine.append("/CenterWindow true\n");
        if( m_aContext.Version > PDFWriter::PDF_1_3 && m_aDocInfo.Title.Len() && m_aContext.DisplayPDFDocumentTitle )
            aLine.append( "/DisplayDocTitle true\n" );
        if( m_aContext.FirstPageLeft &&  m_aContext.PageLayout == PDFWriter::ContinuousFacing )
            aLine.append("/Direction/R2L\n");
        if( m_aContext.OpenInFullScreenMode )
            switch( m_aContext.PDFDocumentMode )
            {
            default :
            case PDFWriter::ModeDefault :
                aLine.append("/NonFullScreenPageMode/UseNone\n");
                break;
            case PDFWriter::UseOutlines :
                aLine.append("/NonFullScreenPageMode/UseOutlines\n");
                break;
            case PDFWriter::UseThumbs :
                aLine.append("/NonFullScreenPageMode/UseThumbs\n");
                break;
            }
        aLine.append( ">>\n" );
    }

    if( nOutlineDict )
    {
        aLine.append( "/Outlines " );
        aLine.append( nOutlineDict );
        aLine.append( " 0 R\n" );
    }
    if( nStructureDict )
    {
        aLine.append( "/StructTreeRoot " );
        aLine.append( nStructureDict );
        aLine.append( " 0 R\n" );
    }
    if( m_aContext.Tagged && m_aContext.Version > PDFWriter::PDF_1_3 )
    {
        aLine.append( "/MarkInfo<</Marked true>>\n" );
    }
    if( m_aWidgets.size() > 0 )
    {
        aLine.append( "/AcroForm<</Fields[\n" );
        int nWidgets = m_aWidgets.size();
        int nOut = 0;
        for( int j = 0; j < nWidgets; j++ )
        {
            // output only root fields
            if( m_aWidgets[j].m_nParent < 1 )
            {
                aLine.append( m_aWidgets[j].m_nObject );
                aLine.append( (nOut++ % 5)==4 ? " 0 R\n" : " 0 R " );
            }
        }
        aLine.append( "\n]/DR " );
        aLine.append( getResourceDictObj() );
        aLine.append( " 0 R /NeedAppearances true>>\n" );
    }
    aLine.append( ">>\n"
                  "endobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    return true;
}

sal_Int32 PDFWriterImpl::emitInfoDict( OString& rIDOut )
{
    sal_Int32 nObject = createObject();

    if( updateObject( nObject ) )
    {
        OStringBuffer aLine( 1024 );
        OStringBuffer aID( 1024 );
        aLine.append( nObject );
        aLine.append( " 0 obj\n"
                      "<<" );
        if( m_aDocInfo.Title.Len() )
        {
            aLine.append( "/Title" );
            appendUnicodeTextString( m_aDocInfo.Title, aLine );
            appendUnicodeTextString( m_aDocInfo.Title, aID );
            aLine.append( "\n" );
        }
        if( m_aDocInfo.Author.Len() )
        {
            aLine.append( "/Author" );
            appendUnicodeTextString( m_aDocInfo.Author, aLine );
            appendUnicodeTextString( m_aDocInfo.Author, aID );
            aLine.append( "\n" );
        }
        if( m_aDocInfo.Subject.Len() )
        {
            aLine.append( "/Subject" );
            appendUnicodeTextString( m_aDocInfo.Subject, aLine );
            appendUnicodeTextString( m_aDocInfo.Subject, aID );
            aLine.append( "\n" );
        }
        if( m_aDocInfo.Keywords.Len() )
        {
            aLine.append( "/Keywords" );
            appendUnicodeTextString( m_aDocInfo.Keywords, aLine );
            appendUnicodeTextString( m_aDocInfo.Keywords, aID );
            aLine.append( "\n" );
        }
        if( m_aDocInfo.Creator.Len() )
        {
            aLine.append( "/Creator" );
            appendUnicodeTextString( m_aDocInfo.Creator, aLine );
            appendUnicodeTextString( m_aDocInfo.Creator, aID );
            aLine.append( "\n" );
        }
        if( m_aDocInfo.Producer.Len() )
        {
            aLine.append( "/Producer" );
            appendUnicodeTextString( m_aDocInfo.Producer, aLine );
            appendUnicodeTextString( m_aDocInfo.Producer, aID );
            aLine.append( "\n" );
        }
        TimeValue aTVal, aGMT;
        oslDateTime aDT;
        osl_getSystemTime( &aGMT );
        osl_getLocalTimeFromSystemTime( &aGMT, &aTVal );
        osl_getDateTimeFromTimeValue( &aTVal, &aDT );
        OStringBuffer aDateString(64);
        aDateString.append( "(D:" );
        aDateString.append( (sal_Char)('0' + ((aDT.Year/1000)%10)) );
        aDateString.append( (sal_Char)('0' + ((aDT.Year/100)%10)) );
        aDateString.append( (sal_Char)('0' + ((aDT.Year/10)%10)) );
        aDateString.append( (sal_Char)('0' + ((aDT.Year)%10)) );
        aDateString.append( (sal_Char)('0' + ((aDT.Month/10)%10)) );
        aDateString.append( (sal_Char)('0' + ((aDT.Month)%10)) );
        aDateString.append( (sal_Char)('0' + ((aDT.Day/10)%10)) );
        aDateString.append( (sal_Char)('0' + ((aDT.Day)%10)) );
        aDateString.append( (sal_Char)('0' + ((aDT.Hours/10)%10)) );
        aDateString.append( (sal_Char)('0' + ((aDT.Hours)%10)) );
        aDateString.append( (sal_Char)('0' + ((aDT.Minutes/10)%10)) );
        aDateString.append( (sal_Char)('0' + ((aDT.Minutes)%10)) );
        aDateString.append( (sal_Char)('0' + ((aDT.Seconds/10)%10)) );
        aDateString.append( (sal_Char)('0' + ((aDT.Seconds)%10)) );
        sal_uInt32 nDelta = 0;
        if( aGMT.Seconds > aTVal.Seconds )
        {
            aDateString.append( "-" );
            nDelta = aGMT.Seconds-aTVal.Seconds;
        }
        else if( aGMT.Seconds < aTVal.Seconds )
        {
            aDateString.append( "+" );
            nDelta = aTVal.Seconds-aGMT.Seconds;
        }
        else
            aDateString.append( "Z" );
        if( nDelta )
        {
            aDateString.append( (sal_Char)('0' + ((nDelta/36000)%10)) );
            aDateString.append( (sal_Char)('0' + ((nDelta/3600)%10)) );
            aDateString.append( "'" );
            aDateString.append( (sal_Char)('0' + ((nDelta/600)%6)) );
            aDateString.append( (sal_Char)('0' + ((nDelta/60)%10)) );
        }
        aDateString.append( "')" );
        aLine.append( "/CreationDate " );
        aLine.append( aDateString.getStr(), aDateString.getLength() );
        aLine.append( "\n" );
        aID.append( aDateString.getStr(), aDateString.getLength() );

        aLine.append( ">>\nendobj\n\n" );
        if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
            nObject = 0;

        rIDOut = aID.makeStringAndClear();
    }
    else
        nObject = 0;

    return nObject;
}

bool PDFWriterImpl::emitTrailer()
{
    // emit doc info
    OString aInfoValuesOut;
    sal_Int32 nDocInfoObject = emitInfoDict( aInfoValuesOut );

    // emit xref table

    // remember start
    sal_uInt64 nXRefOffset = 0;
    CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nXRefOffset )) );
    CHECK_RETURN( writeBuffer( "xref\n", 5 ) );

    sal_Int32 nObjects = m_aObjects.size();
    OStringBuffer aLine;
    aLine.append( "0 " );
    aLine.append( (sal_Int32)(nObjects+1) );
    aLine.append( "\n" );
    aLine.append( "0000000000 65535 f \n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    for( sal_Int32 i = 0; i < nObjects; i++ )
    {
        aLine.setLength( 0 );
        OString aOffset = OString::valueOf( (sal_Int64)m_aObjects[i] );
        for( sal_Int32 j = 0; j < (10-aOffset.getLength()); j++ )
            aLine.append( '0' );
        aLine.append( aOffset );
        aLine.append( " 00000 n \n" );
        DBG_ASSERT( aLine.getLength() == 20, "invalid xref entry" );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }

    // setup document id
    OStringBuffer aDocID(32);
    rtlDigest aDigest = rtl_digest_createMD5();
    if( aDigest )
    {
        sal_uInt64 nOffset = 0;
        if( osl_File_E_None == osl_getFilePos( m_aFile, &nOffset ) )
        {
            TimeValue aGMT;
            osl_getSystemTime( &aGMT );
            rtlDigestError nError = rtl_digest_updateMD5( aDigest, &aGMT, sizeof( aGMT ) );
            if( nError == rtl_Digest_E_None )
                nError = rtl_digest_updateMD5( aDigest, m_aContext.URL.getStr(), m_aContext.URL.getLength()*sizeof(sal_Unicode) );
            if( nError == rtl_Digest_E_None )
                nError = rtl_digest_updateMD5( aDigest, &nOffset, sizeof( nOffset ) );
            if( nError == rtl_Digest_E_None )
                nError = rtl_digest_updateMD5( aDigest, aInfoValuesOut.getStr(), aInfoValuesOut.getLength() );
            if( nError == rtl_Digest_E_None )
            {
                sal_uInt8 nMD5Sum[ RTL_DIGEST_LENGTH_MD5 ];
                rtl_digest_getMD5( aDigest, nMD5Sum, sizeof(nMD5Sum) );
                for( unsigned int i = 0; i < sizeof(nMD5Sum)/sizeof(nMD5Sum[0]); i++ )
                    appendHex( nMD5Sum[i], aDocID );
            }
        }
        rtl_digest_destroyMD5( aDigest );
    }

    // emit trailer
    aLine.setLength( 0 );
    aLine.append( "trailer\n"
                  "<</Size " );
    aLine.append( (sal_Int32)(nObjects+1) );
    aLine.append( "\n"
                  "/Root " );
    aLine.append( m_nCatalogObject );
    aLine.append( " 0 R\n" );
    if( nDocInfoObject )
    {
        aLine.append( "/Info " );
        aLine.append( nDocInfoObject );
        aLine.append( " 0 R\n" );
    }
    if( aDocID.getLength() )
    {
        aLine.append( "/ID [ <" );
        aLine.append( aDocID.getStr(), aDocID.getLength() );
        aLine.append( ">\n"
                      "<" );
        aLine.append( aDocID.getStr(), aDocID.getLength() );
        aLine.append( "> ]\n" );
    }
    aLine.append( ">>\n"
                  "startxref\n" );
    aLine.append( (sal_Int64)nXRefOffset );
    aLine.append( "\n"
                  "%%EOF\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    return true;
}

struct AnnotationSortEntry
{
    sal_Int32 nTabOrder;
    sal_Int32 nObject;
    sal_Int32 nWidgetIndex;

    AnnotationSortEntry( sal_Int32 nTab, sal_Int32 nObj, sal_Int32 nI ) :
        nTabOrder( nTab ),
        nObject( nObj ),
        nWidgetIndex( nI )
    {}
};

struct AnnotSortContainer
{
    std::set< sal_Int32 >               aObjects;
    std::vector< AnnotationSortEntry >    aSortedAnnots;
};

struct AnnotSorterLess
{
    std::vector< PDFWriterImpl::PDFWidget >& m_rWidgets;

    AnnotSorterLess( std::vector< PDFWriterImpl::PDFWidget >& rWidgets ) : m_rWidgets( rWidgets ) {}

    bool operator()( const AnnotationSortEntry& rLeft, const AnnotationSortEntry& rRight )
    {
        if( rLeft.nTabOrder < rRight.nTabOrder )
            return true;
        if( rRight.nTabOrder < rLeft.nTabOrder )
            return false;
        if( rLeft.nWidgetIndex < 0 && rRight.nWidgetIndex < 0 )
            return false;
        if( rRight.nWidgetIndex < 0 )
            return true;
        if( rLeft.nWidgetIndex < 0 )
            return false;
        // remember: widget rects are in PDF coordinates, so they are ordered down up
        if( m_rWidgets[ rLeft.nWidgetIndex ].m_aRect.Top() >
            m_rWidgets[ rRight.nWidgetIndex ].m_aRect.Top() )
            return true;
        if( m_rWidgets[ rRight.nWidgetIndex ].m_aRect.Top() >
            m_rWidgets[ rLeft.nWidgetIndex ].m_aRect.Top() )
            return false;
        if( m_rWidgets[ rLeft.nWidgetIndex ].m_aRect.Left() <
            m_rWidgets[ rRight.nWidgetIndex ].m_aRect.Left() )
            return true;
        return false;
    }
};

void PDFWriterImpl::sortWidgets()
{
    // sort widget annotations on each page as per their
    // TabOrder attribute
    std::hash_map< sal_Int32, AnnotSortContainer > sorted;
    int nWidgets = m_aWidgets.size();
    for( int nW = 0; nW < nWidgets; nW++ )
    {
        const PDFWidget& rWidget = m_aWidgets[nW];
        AnnotSortContainer& rCont = sorted[ rWidget.m_nPage ];
        // optimize vector allocation
        if( rCont.aSortedAnnots.empty() )
            rCont.aSortedAnnots.reserve( m_aPages[ rWidget.m_nPage ].m_aAnnotations.size() );
        // insert widget to tab sorter
        // RadioButtons are not page annotations, only their individual check boxes are
        if( rWidget.m_eType != PDFWriter::RadioButton )
        {
            rCont.aObjects.insert( rWidget.m_nObject );
            rCont.aSortedAnnots.push_back( AnnotationSortEntry( rWidget.m_nTabOrder, rWidget.m_nObject, nW ) );
        }
    }
    for( std::hash_map< sal_Int32, AnnotSortContainer >::iterator it = sorted.begin(); it != sorted.end(); ++it )
    {
        // append entries for non widget annotations
        PDFPage& rPage = m_aPages[ it->first ];
        unsigned int nAnnots = rPage.m_aAnnotations.size();
        for( unsigned int nA = 0; nA < nAnnots; nA++ )
            if( it->second.aObjects.find( rPage.m_aAnnotations[nA] ) == it->second.aObjects.end())
                it->second.aSortedAnnots.push_back( AnnotationSortEntry( 10000, rPage.m_aAnnotations[nA], -1 ) );

        AnnotSorterLess aLess( m_aWidgets );
        std::stable_sort( it->second.aSortedAnnots.begin(), it->second.aSortedAnnots.end(), aLess );
        // sanity check
        if( it->second.aSortedAnnots.size() == nAnnots)
        {
            for( unsigned int nA = 0; nA < nAnnots; nA++ )
                rPage.m_aAnnotations[nA] = it->second.aSortedAnnots[nA].nObject;
        }
        else
        {
            DBG_ASSERT( 0, "wrong number of sorted annotations" );
            #if OSL_DEBUG_LEVEL > 0
            fprintf( stderr, "PDFWriterImpl::sortWidgets(): wrong number of sorted assertions on page nr %d\n"
                             "    %d sorted and %d unsorted\n", it->first, it->second.aSortedAnnots.size(), nAnnots );
            #endif
        }
    }

    // FIXME: implement tab order in structure tree for PDF 1.5
}

bool PDFWriterImpl::emit()
{
    endPage();

    // resort structure tree and annotations if necessary
    // needed for widget tab order
    sortWidgets();

    // emit catalog
    CHECK_RETURN( emitCatalog() );

    // emit trailer
    CHECK_RETURN( emitTrailer() );

    osl_closeFile( m_aFile );
    m_bOpen = false;

    return true;
}

void PDFWriterImpl::registerGlyphs(
                                   int nGlyphs,
                                   sal_Int32* pGlyphs,
                                   sal_Unicode* pUnicodes,
                                   sal_uInt8* pMappedGlyphs,
                                   sal_Int32* pMappedFontObjects,
                                   ImplFontData* pFallbackFonts[] )
{
    ImplFontData* pDevFont = m_pReferenceDevice->mpFontEntry->maFontSelData.mpFontData;
    for( int i = 0; i < nGlyphs; i++ )
    {
        if( ! pGlyphs[i] )
            continue;

        ImplFontData* pCurrentFont = pFallbackFonts[i] ? pFallbackFonts[i] : pDevFont;

        if( pCurrentFont->mbSubsettable )
        {

            FontSubset& rSubset = m_aSubsets[ pCurrentFont ];
            // search for glyphID
            FontMapping::iterator it = rSubset.m_aMapping.find( pGlyphs[i] );
            if( it != rSubset.m_aMapping.end() )
            {
                pMappedFontObjects[i] = it->second.m_nFontID;
                pMappedGlyphs[i] = it->second.m_nSubsetGlyphID;
            }
            else
            {
                // create new subset if necessary
                if( rSubset.m_aSubsets.begin() == rSubset.m_aSubsets.end() ||
                    rSubset.m_aSubsets.back().m_aMapping.size() > 254 )
                {
                    rSubset.m_aSubsets.push_back( FontEmit( m_nNextFID++ ) );
                }

                // copy font id
                pMappedFontObjects[i] = rSubset.m_aSubsets.back().m_nFontID;
                // create new glyph in subset
                sal_uInt8 nNewId = sal::static_int_cast<sal_uInt8>(rSubset.m_aSubsets.back().m_aMapping.size()+1);
                pMappedGlyphs[i] = nNewId;

                // add new glyph to emitted font subset
                GlyphEmit& rNewGlyphEmit = rSubset.m_aSubsets.back().m_aMapping[ pGlyphs[i] ];
                rNewGlyphEmit.m_nSubsetGlyphID = nNewId;
                rNewGlyphEmit.m_aUnicode = (pUnicodes ? pUnicodes[i] : 0);

                // add new glyph to font mapping
                Glyph& rNewGlyph = rSubset.m_aMapping[ pGlyphs[i] ];
                rNewGlyph.m_nFontID = pMappedFontObjects[i];
                rNewGlyph.m_nSubsetGlyphID = nNewId;
            }
        }
        else
        {
            sal_Int32 nFontID = 0;
            FontEmbedData::iterator it = m_aEmbeddedFonts.find( pCurrentFont );
            if( it != m_aEmbeddedFonts.end() )
                nFontID = it->second.m_nNormalFontID;
            else
            {
                nFontID = m_nNextFID++;
                m_aEmbeddedFonts[ pCurrentFont ] = EmbedFont();
                m_aEmbeddedFonts[ pCurrentFont ].m_nNormalFontID = nFontID;
            }
            EmbedFont& rEmbedFont = m_aEmbeddedFonts[pCurrentFont];

            const std::map< sal_Unicode, sal_Int32 >* pEncoding = NULL;
            const std::map< sal_Unicode, rtl::OString >* pNonEncoded = NULL;
            getReferenceDevice()->ImplGetGraphics();
            pEncoding = m_pReferenceDevice->mpGraphics->GetFontEncodingVector( pCurrentFont, &pNonEncoded );

            std::map< sal_Unicode, sal_Int32 >::const_iterator enc_it;
            std::map< sal_Unicode, rtl::OString >::const_iterator nonenc_it;

            sal_Int32 nCurFontID = nFontID;
            sal_Unicode cChar = pUnicodes[i];
            if( pEncoding )
            {
                enc_it = pEncoding->find( cChar );
                if( enc_it != pEncoding->end() && enc_it->second > 0 )
                {
                    DBG_ASSERT( (enc_it->second & 0xffffff00) == 0, "Invalid character code" );
                    cChar = (sal_Unicode)enc_it->second;
                }
                else if( (enc_it == pEncoding->end() || enc_it->second == -1) &&
                         pNonEncoded &&
                         (nonenc_it = pNonEncoded->find( cChar )) != pNonEncoded->end() )
                {
                    nCurFontID = 0;
                    // find non encoded glyph
                    for( std::list< EmbedEncoding >::iterator nec_it = rEmbedFont.m_aExtendedEncodings.begin(); nec_it != rEmbedFont.m_aExtendedEncodings.end(); ++nec_it )
                    {
                        if( nec_it->m_aCMap.find( cChar ) != nec_it->m_aCMap.end() )
                        {
                            nCurFontID = nec_it->m_nFontID;
                            cChar = (sal_Unicode)nec_it->m_aCMap[ cChar ];
                            break;
                        }
                    }
                    if( nCurFontID == 0 ) // new nonencoded glyph
                    {
                        if( rEmbedFont.m_aExtendedEncodings.empty() || rEmbedFont.m_aExtendedEncodings.back().m_aEncVector.size() == 255 )
                        {
                            rEmbedFont.m_aExtendedEncodings.push_back( EmbedEncoding() );
                            rEmbedFont.m_aExtendedEncodings.back().m_nFontID = m_nNextFID++;
                        }
                        EmbedEncoding& rEncoding = rEmbedFont.m_aExtendedEncodings.back();
                        rEncoding.m_aEncVector.push_back( EmbedCode() );
                        rEncoding.m_aEncVector.back().m_aUnicode = cChar;
                        rEncoding.m_aEncVector.back().m_aName = nonenc_it->second;
                        rEncoding.m_aCMap[ cChar ] = (sal_Int8)(rEncoding.m_aEncVector.size()-1);
                        nCurFontID = rEncoding.m_nFontID;
                        cChar = (sal_Unicode)rEncoding.m_aCMap[ cChar ];
                    }
                }
                else
                    pEncoding = NULL;
            }
            if( ! pEncoding )
            {
                if( cChar & 0xff00 )
                {
                    // some characters can be used by conversion
                    if( cChar >= 0xf000 && cChar <= 0xf0ff ) // symbol encoding in private use area
                        cChar -= 0xf000;
                    else
                    {
                        String aString( cChar);
                        ByteString aChar( aString, RTL_TEXTENCODING_MS_1252 );
                        cChar = ((sal_Unicode)aChar.GetChar( 0 )) & 0x00ff;
                    }
                }
            }
            pMappedGlyphs[ i ] = (sal_Int8)cChar;
            pMappedFontObjects[ i ] = nCurFontID;
        }
    }
}

void PDFWriterImpl::drawRelief( SalLayout& rLayout, const String& rText, bool bTextLines )
{
    push( PUSH_ALL );

    FontRelief eRelief = m_aCurrentPDFState.m_aFont.GetRelief();

    Color aTextColor = m_aCurrentPDFState.m_aFont.GetColor();
    Color aTextLineColor = m_aCurrentPDFState.m_aTextLineColor;
    Color aReliefColor( COL_LIGHTGRAY );
    if( aTextColor == COL_BLACK )
        aTextColor = Color( COL_WHITE );
    if( aTextLineColor == COL_BLACK )
        aTextLineColor = Color( COL_WHITE );
    if( aTextColor == COL_WHITE )
        aReliefColor = Color( COL_BLACK );

    Font aSetFont = m_aCurrentPDFState.m_aFont;
    aSetFont.SetRelief( RELIEF_NONE );
    aSetFont.SetShadow( FALSE );

    aSetFont.SetColor( aReliefColor );
    setTextLineColor( aTextLineColor );
    setFont( aSetFont );
    long nOff = 1 + getReferenceDevice()->mnDPIX/300;
    if( eRelief == RELIEF_ENGRAVED )
        nOff = -nOff;

    rLayout.DrawOffset() += Point( nOff, nOff );
    updateGraphicsState();
    drawLayout( rLayout, rText, bTextLines );

    rLayout.DrawOffset() -= Point( nOff, nOff );
    setTextLineColor( aTextLineColor );
    aSetFont.SetColor( aTextColor );
    setFont( aSetFont );
    updateGraphicsState();
    drawLayout( rLayout, rText, bTextLines );

    // clean up the mess
    pop();
}

void PDFWriterImpl::drawShadow( SalLayout& rLayout, const String& rText, bool bTextLines )
{
    Font aSaveFont = m_aCurrentPDFState.m_aFont;
    Color aSaveTextLineColor = m_aCurrentPDFState.m_aTextLineColor;

    Font& rFont = m_aCurrentPDFState.m_aFont;
    if( rFont.GetColor() == Color( COL_BLACK ) || rFont.GetColor().GetLuminance() < 8 )
        rFont.SetColor( Color( COL_LIGHTGRAY ) );
    else
        rFont.SetColor( Color( COL_BLACK ) );
    rFont.SetShadow( FALSE );
    rFont.SetOutline( FALSE );
    setFont( rFont );
    setTextLineColor( rFont.GetColor() );
    updateGraphicsState();

    long nOff = 1 + ((m_pReferenceDevice->mpFontEntry->mnLineHeight-24)/24);
    if( rFont.IsOutline() )
        nOff++;
    rLayout.DrawBase() += Point( nOff, nOff );
    drawLayout( rLayout, rText, bTextLines );
    rLayout.DrawBase() -= Point( nOff, nOff );

    setFont( aSaveFont );
    setTextLineColor( aSaveTextLineColor );
    updateGraphicsState();
}

void PDFWriterImpl::drawLayout( SalLayout& rLayout, const String& rText, bool bTextLines )
{
    // relief takes precedence over shadow (see outdev3.cxx)
    if(  m_aCurrentPDFState.m_aFont.GetRelief() != RELIEF_NONE )
    {
        drawRelief( rLayout, rText, bTextLines );
        return;
    }
    else if( m_aCurrentPDFState.m_aFont.IsShadow() )
        drawShadow( rLayout, rText, bTextLines );

    OStringBuffer aLine( 512 );

    const int nMaxGlyphs = 256;

    sal_Int32 pGlyphs[nMaxGlyphs];
    sal_uInt8 pMappedGlyphs[nMaxGlyphs];
    sal_Int32 pMappedFontObjects[nMaxGlyphs];
    sal_Unicode pUnicodes[nMaxGlyphs];
    int pCharPosAry[nMaxGlyphs];
    sal_Int32 nAdvanceWidths[nMaxGlyphs];
    ImplFontData* pFallbackFonts[nMaxGlyphs];
    sal_Int32 *pAdvanceWidths = m_aCurrentPDFState.m_aFont.IsVertical() ? nAdvanceWidths : NULL;
    sal_Int32 nGlyphFlags[nMaxGlyphs];
    int nGlyphs;
    int nIndex = 0;
    Point aCurPos, aLastPos(0, 0), aCumulativePos(0,0), aGlyphPos;
    bool bFirst = true, bWasYChange = false;
    int nMinCharPos = 0, nMaxCharPos = rText.Len()-1;
    double fXScale = 1.0;
    double fSkew = 0.0;
    sal_Int32 nFontHeight = m_pReferenceDevice->mpFontEntry->maFontSelData.mnHeight;
    TextAlign eAlign = m_aCurrentPDFState.m_aFont.GetAlign();

    // transform font height back to current units
    // note: the layout calculates in outdevs device pixel !!
    nFontHeight = m_pReferenceDevice->ImplDevicePixelToLogicHeight( nFontHeight );
    if( m_aCurrentPDFState.m_aFont.GetWidth() )
    {
        Font aFont( m_aCurrentPDFState.m_aFont );
        aFont.SetWidth( 0 );
        FontMetric aMetric = m_pReferenceDevice->GetFontMetric( aFont );
        if( aMetric.GetWidth() != m_aCurrentPDFState.m_aFont.GetWidth() )
        {
            fXScale =
                (double)m_aCurrentPDFState.m_aFont.GetWidth() /
                (double)aMetric.GetWidth();
        }
        // force state before GetFontMetric
        m_pReferenceDevice->ImplNewFont();
    }

    // perform artificial italics if necessary
    if( ( m_aCurrentPDFState.m_aFont.GetItalic() == ITALIC_NORMAL ||
          m_aCurrentPDFState.m_aFont.GetItalic() == ITALIC_OBLIQUE ) &&
        !( m_pReferenceDevice->mpFontEntry->maFontSelData.mpFontData->GetSlant() == ITALIC_NORMAL ||
           m_pReferenceDevice->mpFontEntry->maFontSelData.mpFontData->GetSlant() == ITALIC_OBLIQUE )
        )
    {
        fSkew = M_PI/12.0;
    }

    // if the mapmode is distorted we need to adjust for that also
    if( m_aCurrentPDFState.m_aMapMode.GetScaleX() != m_aCurrentPDFState.m_aMapMode.GetScaleY() )
    {
        fXScale *= double(m_aCurrentPDFState.m_aMapMode.GetScaleX()) / double(m_aCurrentPDFState.m_aMapMode.GetScaleY());
    }

    int nAngle = m_aCurrentPDFState.m_aFont.GetOrientation();
    // normalize angles
    while( nAngle < 0 )
        nAngle += 3600;
    nAngle = nAngle % 3600;
    double fAngle = (double)nAngle * M_PI / 1800.0;

    Matrix3 aRotScale;
    aRotScale.scale( fXScale, 1.0 );
    if( fAngle != 0.0 )
        aRotScale.rotate( -fAngle );

    bool bPop = false;
    bool bABold = false;
    // artificial bold necessary ?
    if( m_pReferenceDevice->mpFontEntry->maFontSelData.mpFontData->GetWeight() <= WEIGHT_MEDIUM &&
        m_pReferenceDevice->mpFontEntry->maFontSelData.GetWeight() > WEIGHT_MEDIUM )
    {
        if( ! bPop )
            aLine.append( "q " );
        bPop = true;
        bABold = true;
    }
    // setup text colors (if necessary)
    Color aStrokeColor( COL_TRANSPARENT );
    Color aNonStrokeColor( COL_TRANSPARENT );

    if( m_aCurrentPDFState.m_aFont.IsOutline() )
    {
        aStrokeColor = m_aCurrentPDFState.m_aFont.GetColor();
        aNonStrokeColor = Color( COL_WHITE );
    }
    else
        aNonStrokeColor = m_aCurrentPDFState.m_aFont.GetColor();
    if( bABold )
        aStrokeColor = m_aCurrentPDFState.m_aFont.GetColor();

    if( aStrokeColor != Color( COL_TRANSPARENT ) && aStrokeColor != m_aCurrentPDFState.m_aLineColor )
    {
        if( ! bPop )
            aLine.append( "q " );
        bPop = true;
        appendStrokingColor( aStrokeColor, aLine );
        aLine.append( "\n" );
    }
    if( aNonStrokeColor != Color( COL_TRANSPARENT ) && aNonStrokeColor != m_aCurrentPDFState.m_aFillColor )
    {
        if( ! bPop )
            aLine.append( "q " );
        bPop = true;
        appendNonStrokingColor( aNonStrokeColor, aLine );
        aLine.append( "\n" );
    }

    // begin text object
    aLine.append( "BT\n" );
    // outline attribute ?
    if( m_aCurrentPDFState.m_aFont.IsOutline() || bABold )
    {
        // set correct text mode, set stroke width
        aLine.append( "2 Tr " ); // fill, then stroke

        if( m_aCurrentPDFState.m_aFont.IsOutline() )
        {
            // unclear what to do in case of outline and artificial bold
            // for the time being outline wins
            aLine.append( "0.25 w \n" );
        }
        else
        {
            double fW = (double)m_aCurrentPDFState.m_aFont.GetHeight() / 30.0;
            m_aPages.back().appendMappedLength( fW, aLine );
            aLine.append ( " w\n" );
        }
    }

    FontMetric aRefDevFontMetric = m_pReferenceDevice->GetFontMetric();

    sal_Int32 nLastMappedFont = -1;
    while( (nGlyphs = rLayout.GetNextGlyphs( nMaxGlyphs, pGlyphs, aCurPos, nIndex, pAdvanceWidths, pCharPosAry )) != 0 )
    {
        bWasYChange = (aGlyphPos.Y() != aCurPos.Y());
        aGlyphPos = aCurPos;
        // back transformation to current coordinate system
        aCurPos = m_pReferenceDevice->PixelToLogic( aCurPos );

        Point aOffset;
        if ( eAlign == ALIGN_BOTTOM )
            aOffset.Y() -= aRefDevFontMetric.GetDescent();
        else if ( eAlign == ALIGN_TOP )
            aOffset.Y() += aRefDevFontMetric.GetAscent();

        if( aOffset.X() || aOffset.Y() )
        {
            aOffset = aRotScale.transform( aOffset );
            aCurPos += aOffset;
        }

        for( int i = 0; i < nGlyphs; i++ )
        {
            if( pGlyphs[i] & GF_FONTMASK )
                pFallbackFonts[i] = ((MultiSalLayout&)rLayout).GetFallbackFontData((pGlyphs[i] & GF_FONTMASK) >> GF_FONTSHIFT);
            else
                pFallbackFonts[i] = NULL;

            nGlyphFlags[i] = (pGlyphs[i] & GF_FLAGMASK);
#ifndef WNT
            // #104930# workaround for Win32 bug: the glyph ids are actually
            // Unicodes for vertical fonts because Win32 does not return
            // the correct glyph ids; this is indicated by GF_ISCHAR which is
            // needed in SalGraphics::CreateFontSubset to convert the Unicodes
            // to vertical glyph ids. Doing this here on a per character
            // basis would be a major performance hit.
            pGlyphs[i] &= GF_IDXMASK;
#endif
            if( pCharPosAry[i] >= nMinCharPos && pCharPosAry[i] <= nMaxCharPos )
                pUnicodes[i] = rText.GetChar( sal::static_int_cast<xub_StrLen>(pCharPosAry[i]) );
            else
                pUnicodes[i] = 0;
            // note: in case of ctl one character may result
            // in multiple glyphs. The current SalLayout
            // implementations set -1 then to indicate that no direct
            // mapping is possible
        }
        registerGlyphs( nGlyphs, pGlyphs, pUnicodes, pMappedGlyphs, pMappedFontObjects, pFallbackFonts );

        if( pAdvanceWidths )
        {
            // have to emit each glyph on its own
            long nXOffset = 0;
            for( int n = 0; n < nGlyphs; n++ )
            {
                double fDeltaAngle = 0.0;
                double fYScale = 1.0;
                double fTempXScale = fXScale;
                double fSkewB = fSkew;
                double fSkewA = 0.0;

                Point aDeltaPos;
                if( ( nGlyphFlags[n] & GF_ROTMASK ) == GF_ROTL )
                {
                    fDeltaAngle = M_PI/2.0;
                    aDeltaPos.X() = aRefDevFontMetric.GetAscent();
                    aDeltaPos.Y() = (int)((double)m_pReferenceDevice->GetFontMetric().GetDescent() * fXScale);
                    fYScale = fXScale;
                    fTempXScale = 1.0;
                    fSkewA = -fSkewB;
                    fSkewB = 0.0;
                }
                else if( ( nGlyphFlags[n] & GF_ROTMASK ) == GF_ROTR )
                {
                    fDeltaAngle = -M_PI/2.0;
                    aDeltaPos.X() = (int)((double)aRefDevFontMetric.GetDescent()*fXScale);
                    aDeltaPos.Y() = -aRefDevFontMetric.GetAscent();
                    fYScale = fXScale;
                    fTempXScale = 1.0;
                    fSkewA = fSkewB;
                    fSkewB = 0.0;
                }
                aDeltaPos += (m_pReferenceDevice->PixelToLogic( Point( (int)((double)nXOffset/fXScale)/rLayout.GetUnitsPerPixel(), 0 ) ) - m_pReferenceDevice->PixelToLogic( Point() ) );
                nXOffset += pAdvanceWidths[n];
                if( ! pGlyphs[n] )
                    continue;


                aDeltaPos = aRotScale.transform( aDeltaPos );

                Matrix3 aMat;
                if( fSkewB != 0.0 || fSkewA != 0.0 )
                    aMat.skew( fSkewA, fSkewB );
                aMat.scale( fTempXScale, fYScale );
                aMat.rotate( fAngle+fDeltaAngle );
                aMat.translate( aCurPos.X()+aDeltaPos.X(), aCurPos.Y()+aDeltaPos.Y() );
                aMat.append( m_aPages.back(), aLine );
                aLine.append( " Tm" );
                if( nLastMappedFont != pMappedFontObjects[n] )
                {
                    nLastMappedFont = pMappedFontObjects[n];
                    aLine.append( " /F" );
                    aLine.append( pMappedFontObjects[n] );
                    aLine.append( ' ' );
                    m_aPages.back().appendMappedLength( nFontHeight, aLine, true );
                    aLine.append( " Tf" );
                }
                aLine.append( " <" );
                appendHex( (sal_Int8)pMappedGlyphs[n], aLine );
                aLine.append( "> Tj\n" );
            }
        }
        else // normal case
        {
            // optimize use of Td vs. Tm
            if( fAngle == 0.0 && fXScale == 1.0 && ( !bFirst || fSkew == 0.0 ) )
            {
                if( bFirst )
                {
                    m_aPages.back().appendPoint( aCurPos, aLine, false, &aCumulativePos );
                    bFirst = false;
                    aLastPos = aCurPos;
                }
                else
                {
                    sal_Int32 nDiffL = 0;
                    Point aDiff = aCurPos - aLastPos;
                    m_aPages.back().appendMappedLength( (sal_Int32)aDiff.X(), aLine, false, &nDiffL );
                    aCumulativePos.X() += nDiffL;
                    aLine.append( ' ' );
                    if( bWasYChange )
                    {
                        m_aPages.back().appendMappedLength( (sal_Int32)aDiff.Y(), aLine, true, &nDiffL );
                        aCumulativePos.Y() += nDiffL;
                    }
                    else
                    {
                        aLine.append( '0' );
                    }
                    // back project last position to catch rounding errors
                    Point aBackPos = lcl_convert( m_aMapMode,
                                                  m_aGraphicsStack.front().m_aMapMode,
                                                  getReferenceDevice(),
                                                  aCumulativePos
                                                  );
                    // catch rounding error in back projection on Y axis;
                    // else the back projection can produce a sinuous text baseline
                    if( ! bWasYChange )
                        aBackPos.Y() = aLastPos.Y();
                    aLastPos = aBackPos;
                }
                aLine.append( " Td " );
            }
            else
            {
                Matrix3 aMat;
                if( fSkew != 0.0 )
                    aMat.skew( 0.0, fSkew );
                aMat.scale( fXScale, 1.0 );
                aMat.rotate( fAngle );
                aMat.translate( aCurPos.X(), aCurPos.Y() );
                aMat.append( m_aPages.back(), aLine, &aCumulativePos );
                aLine.append( " Tm\n" );
                aLastPos = aCurPos;
                bFirst = false;
            }
            int nLast = 0;
            while( nLast < nGlyphs )
            {
                while( ! pGlyphs[nLast] && nLast < nGlyphs )
                    nLast++;
                if( nLast >= nGlyphs )
                    break;

                int nNext = nLast+1;
                while( nNext < nGlyphs && pMappedFontObjects[ nNext ] == pMappedFontObjects[nLast] && pGlyphs[nNext] )
                    nNext++;
                if( nLastMappedFont != pMappedFontObjects[nLast] )
                {
                    aLine.append( "/F" );
                    aLine.append( pMappedFontObjects[nLast] );
                    aLine.append( ' ' );
                    m_aPages.back().appendMappedLength( nFontHeight, aLine, true );
                    aLine.append( " Tf " );
                    nLastMappedFont = pMappedFontObjects[nLast];
                }
                aLine.append( "<" );
                for( int i = nLast; i < nNext; i++ )
                {
                    appendHex( (sal_Int8)pMappedGlyphs[i], aLine );
                    if( i && (i % 35) == 0 )
                        aLine.append( "\n" );
                }
                aLine.append( "> Tj\n" );

                nLast = nNext;
            }
        }
    }

    // end textobject
    aLine.append( "ET\n" );
    if( bPop )
        aLine.append( "Q\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );

    // draw eventual textlines
    FontStrikeout eStrikeout = m_aCurrentPDFState.m_aFont.GetStrikeout();
    FontUnderline eUnderline = m_aCurrentPDFState.m_aFont.GetUnderline();
    if( bTextLines &&
        (
         ( eUnderline != UNDERLINE_NONE && eUnderline != UNDERLINE_DONTKNOW ) ||
         ( eStrikeout != STRIKEOUT_NONE && eStrikeout != STRIKEOUT_DONTKNOW )
         )
        )
    {
        BOOL bUnderlineAbove = OutputDevice::ImplIsUnderlineAbove( m_aCurrentPDFState.m_aFont );
        if( m_aCurrentPDFState.m_aFont.IsWordLineMode() )
        {
            Point aPos, aStartPt;
            sal_Int32 nWidth = 0, nAdvance=0;
            for( int nStart = 0;;)
            {
                sal_Int32 nGlyphIndex;
                if( !rLayout.GetNextGlyphs( 1, &nGlyphIndex, aPos, nStart, &nAdvance ) )
                    break;

                if( !rLayout.IsSpacingGlyph( nGlyphIndex ) )
                {
                    if( !nWidth )
                        aStartPt = aPos;

                    nWidth += nAdvance;
                }
                else if( nWidth > 0 )
                {
                    drawTextLine( m_pReferenceDevice->PixelToLogic( aStartPt ),
                                  m_pReferenceDevice->ImplDevicePixelToLogicWidth( nWidth ),
                                  eStrikeout, eUnderline, bUnderlineAbove );
                    nWidth = 0;
                }
            }

            if( nWidth > 0 )
            {
                drawTextLine( m_pReferenceDevice->PixelToLogic( aStartPt ),
                              m_pReferenceDevice->ImplDevicePixelToLogicWidth( nWidth ),
                              eStrikeout, eUnderline, bUnderlineAbove );
            }
        }
        else
        {
            Point aStartPt = rLayout.GetDrawPosition();
            int nWidth = rLayout.GetTextWidth() / rLayout.GetUnitsPerPixel();
            drawTextLine( m_pReferenceDevice->PixelToLogic( aStartPt ),
                          m_pReferenceDevice->ImplDevicePixelToLogicWidth( nWidth ),
                          eStrikeout, eUnderline, bUnderlineAbove );
        }
    }

    // write eventual emphasis marks
    if( m_aCurrentPDFState.m_aFont.GetEmphasisMark() & EMPHASISMARK_STYLE )
    {
        PolyPolygon             aEmphPoly;
        Rectangle               aEmphRect1;
        Rectangle               aEmphRect2;
        long                    nEmphYOff;
        long                    nEmphWidth;
        long                    nEmphHeight;
        BOOL                    bEmphPolyLine;
        FontEmphasisMark        nEmphMark;

        push( PUSH_ALL );

        aLine.setLength( 0 );
        aLine.append( "q\n" );

        nEmphMark = m_pReferenceDevice->ImplGetEmphasisMarkStyle( m_aCurrentPDFState.m_aFont );
        if ( nEmphMark & EMPHASISMARK_POS_BELOW )
            nEmphHeight = m_pReferenceDevice->mnEmphasisDescent;
        else
            nEmphHeight = m_pReferenceDevice->mnEmphasisAscent;
        m_pReferenceDevice->ImplGetEmphasisMark( aEmphPoly,
                                                 bEmphPolyLine,
                                                 aEmphRect1,
                                                 aEmphRect2,
                                                 nEmphYOff,
                                                 nEmphWidth,
                                                 nEmphMark,
                                                 m_pReferenceDevice->ImplDevicePixelToLogicWidth(nEmphHeight),
                                                 m_pReferenceDevice->mpFontEntry->mnOrientation );
        if ( bEmphPolyLine )
        {
            setLineColor( m_aCurrentPDFState.m_aFont.GetColor() );
            setFillColor( Color( COL_TRANSPARENT ) );
        }
        else
        {
            setFillColor( m_aCurrentPDFState.m_aFont.GetColor() );
            setLineColor( Color( COL_TRANSPARENT ) );
        }
        writeBuffer( aLine.getStr(), aLine.getLength() );

        Point aOffset = Point(0,0);

        if ( nEmphMark & EMPHASISMARK_POS_BELOW )
            aOffset.Y() += m_pReferenceDevice->mpFontEntry->maMetric.mnDescent + nEmphYOff;
        else
            aOffset.Y() -= m_pReferenceDevice->mpFontEntry->maMetric.mnAscent + nEmphYOff;

        long nEmphWidth2     = nEmphWidth / 2;
        long nEmphHeight2    = nEmphHeight / 2;
        aOffset += Point( nEmphWidth2, nEmphHeight2 );

        if ( eAlign == ALIGN_BOTTOM )
            aOffset.Y() -= m_pReferenceDevice->mpFontEntry->maMetric.mnDescent;
        else if ( eAlign == ALIGN_TOP )
            aOffset.Y() += m_pReferenceDevice->mpFontEntry->maMetric.mnAscent;

        for( int nStart = 0;;)
        {
            Point aPos;
            sal_Int32 nGlyphIndex, nAdvance;
            if( !rLayout.GetNextGlyphs( 1, &nGlyphIndex, aPos, nStart, &nAdvance ) )
                break;

            if( !rLayout.IsSpacingGlyph( nGlyphIndex ) )
            {
                Point aAdjOffset = aOffset;
                aAdjOffset.X() += (nAdvance - nEmphWidth) / 2;
                aAdjOffset = aRotScale.transform( aAdjOffset );

                aAdjOffset -= Point( nEmphWidth2, nEmphHeight2 );

                aPos += aAdjOffset;
                aPos = m_pReferenceDevice->PixelToLogic( aPos );
                drawEmphasisMark( aPos.X(), aPos.Y(),
                                  aEmphPoly, bEmphPolyLine,
                                  aEmphRect1, aEmphRect2 );
            }
        }

        writeBuffer( "Q\n", 2 );
        pop();
    }

}

void PDFWriterImpl::drawEmphasisMark( long nX, long nY,
                                      const PolyPolygon& rPolyPoly, BOOL bPolyLine,
                                      const Rectangle& rRect1, const Rectangle& rRect2 )
{
    // TODO: pass nWidth as width of this mark
    // long nWidth = 0;

    if ( rPolyPoly.Count() )
    {
        if ( bPolyLine )
        {
            Polygon aPoly = rPolyPoly.GetObject( 0 );
            aPoly.Move( nX, nY );
            drawPolyLine( aPoly );
        }
        else
        {
            PolyPolygon aPolyPoly = rPolyPoly;
            aPolyPoly.Move( nX, nY );
            drawPolyPolygon( aPolyPoly );
        }
    }

    if ( !rRect1.IsEmpty() )
    {
        Rectangle aRect( Point( nX+rRect1.Left(),
                                nY+rRect1.Top() ), rRect1.GetSize() );
        drawRectangle( aRect );
    }

    if ( !rRect2.IsEmpty() )
    {
        Rectangle aRect( Point( nX+rRect2.Left(),
                                nY+rRect2.Top() ), rRect2.GetSize() );

        drawRectangle( aRect );
    }
}

void PDFWriterImpl::drawText( const Point& rPos, const String& rText, xub_StrLen nIndex, xub_StrLen nLen, bool bTextLines )
{
    MARK( "drawText" );

    updateGraphicsState();

    // get a layout from the OuputDevice's SalGraphics
    // this also enforces font substitution and sets the font on SalGraphics
    SalLayout* pLayout = m_pReferenceDevice->ImplLayout( rText, nIndex, nLen, rPos );
    if( pLayout )
    {
        drawLayout( *pLayout, rText, bTextLines );
        pLayout->Release();
    }
}

void PDFWriterImpl::drawTextArray( const Point& rPos, const String& rText, const sal_Int32* pDXArray, xub_StrLen nIndex, xub_StrLen nLen, bool bTextLines )
{
    MARK( "drawText with array" );

    updateGraphicsState();

    // get a layout from the OuputDevice's SalGraphics
    // this also enforces font substitution and sets the font on SalGraphics
    SalLayout* pLayout = m_pReferenceDevice->ImplLayout( rText, nIndex, nLen, rPos, 0, pDXArray );
    if( pLayout )
    {
        drawLayout( *pLayout, rText, bTextLines );
        pLayout->Release();
    }
}

void PDFWriterImpl::drawStretchText( const Point& rPos, ULONG nWidth, const String& rText, xub_StrLen nIndex, xub_StrLen nLen, bool bTextLines )
{
    MARK( "drawStretchText" );

    updateGraphicsState();

    // get a layout from the OuputDevice's SalGraphics
    // this also enforces font substitution and sets the font on SalGraphics
    SalLayout* pLayout = m_pReferenceDevice->ImplLayout( rText, nIndex, nLen, rPos, nWidth );
    if( pLayout )
    {
        drawLayout( *pLayout, rText, bTextLines );
        pLayout->Release();
    }
}

void PDFWriterImpl::drawText( const Rectangle& rRect, const String& rOrigStr, USHORT nStyle, bool bTextLines )
{
    long        nWidth          = rRect.GetWidth();
    long        nHeight         = rRect.GetHeight();

    if ( nWidth <= 0 || nHeight <= 0 )
        return;

    MARK( "drawText with rectangle" );

    updateGraphicsState();

    // clip with rectangle
    OStringBuffer aLine;
    aLine.append( "q " );
    m_aPages.back().appendRect( rRect, aLine );
    aLine.append( " W* n\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );

    // if disabled text is needed, put in here

    Point       aPos            = rRect.TopLeft();

    long        nTextHeight     = m_pReferenceDevice->GetTextHeight();
    xub_StrLen  nMnemonicPos    = STRING_NOTFOUND;

    String aStr = rOrigStr;
    if ( nStyle & TEXT_DRAW_MNEMONIC )
        aStr = m_pReferenceDevice->GetNonMnemonicString( aStr, nMnemonicPos );

    // multiline text
    if ( nStyle & TEXT_DRAW_MULTILINE )
    {
        XubString               aLastLine;
        ImplMultiTextLineInfo   aMultiLineInfo;
        ImplTextLineInfo*       pLineInfo;
        long                    nMaxTextWidth;
        xub_StrLen              i;
        xub_StrLen              nLines;
        xub_StrLen              nFormatLines;

        if ( nTextHeight )
        {
            nMaxTextWidth = m_pReferenceDevice->ImplGetTextLines( aMultiLineInfo, nWidth, aStr, nStyle );
            nLines = (xub_StrLen)(nHeight/nTextHeight);
            nFormatLines = aMultiLineInfo.Count();
            if ( !nLines )
                nLines = 1;
            if ( nFormatLines > nLines )
            {
                if ( nStyle & TEXT_DRAW_ENDELLIPSIS )
                {
                    // handle last line
                    nFormatLines = nLines-1;

                    pLineInfo = aMultiLineInfo.GetLine( nFormatLines );
                    aLastLine = aStr.Copy( pLineInfo->GetIndex() );
                    aLastLine.ConvertLineEnd( LINEEND_LF );
                    // replace line feed by space
                    xub_StrLen nLastLineLen = aLastLine.Len();
                    for ( i = 0; i < nLastLineLen; i++ )
                    {
                        if ( aLastLine.GetChar( i ) == _LF )
                            aLastLine.SetChar( i, ' ' );
                    }
                    aLastLine = m_pReferenceDevice->GetEllipsisString( aLastLine, nWidth, nStyle );
                    nStyle &= ~(TEXT_DRAW_VCENTER | TEXT_DRAW_BOTTOM);
                    nStyle |= TEXT_DRAW_TOP;
                }
            }

            // vertical alignment
            if ( nStyle & TEXT_DRAW_BOTTOM )
                aPos.Y() += nHeight-(nFormatLines*nTextHeight);
            else if ( nStyle & TEXT_DRAW_VCENTER )
                aPos.Y() += (nHeight-(nFormatLines*nTextHeight))/2;

            // draw all lines excluding the last
            for ( i = 0; i < nFormatLines; i++ )
            {
                pLineInfo = aMultiLineInfo.GetLine( i );
                if ( nStyle & TEXT_DRAW_RIGHT )
                    aPos.X() += nWidth-pLineInfo->GetWidth();
                else if ( nStyle & TEXT_DRAW_CENTER )
                    aPos.X() += (nWidth-pLineInfo->GetWidth())/2;
                xub_StrLen nIndex   = pLineInfo->GetIndex();
                xub_StrLen nLineLen = pLineInfo->GetLen();
                drawText( aPos, aStr, nIndex, nLineLen, bTextLines );
                // mnemonics should not appear in documents,
                // if the need arises, put them in here
                aPos.Y() += nTextHeight;
                aPos.X() = rRect.Left();
            }


            // output last line left adjusted since it was shortened
            if ( aLastLine.Len() )
                drawText( aPos, aLastLine, 0, STRING_LEN, bTextLines );
        }
    }
    else
    {
        long nTextWidth = m_pReferenceDevice->GetTextWidth( aStr );

        // Evt. Text kuerzen
        if ( nTextWidth > nWidth )
        {
            if ( nStyle & (TEXT_DRAW_ENDELLIPSIS | TEXT_DRAW_PATHELLIPSIS | TEXT_DRAW_NEWSELLIPSIS) )
            {
                aStr = m_pReferenceDevice->GetEllipsisString( aStr, nWidth, nStyle );
                nStyle &= ~(TEXT_DRAW_CENTER | TEXT_DRAW_RIGHT);
                nStyle |= TEXT_DRAW_LEFT;
                nTextWidth = m_pReferenceDevice->GetTextWidth( aStr );
            }
        }

        // vertical alignment
        if ( nStyle & TEXT_DRAW_RIGHT )
            aPos.X() += nWidth-nTextWidth;
        else if ( nStyle & TEXT_DRAW_CENTER )
            aPos.X() += (nWidth-nTextWidth)/2;

        if ( nStyle & TEXT_DRAW_BOTTOM )
            aPos.Y() += nHeight-nTextHeight;
        else if ( nStyle & TEXT_DRAW_VCENTER )
            aPos.Y() += (nHeight-nTextHeight)/2;

        // mnemonics should be inserted here if the need arises

        // draw the actual text
        drawText( aPos, aStr, 0, STRING_LEN, bTextLines );
    }

    // reset clip region to original value
    aLine.setLength( 0 );
    aLine.append( "Q\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawLine( const Point& rStart, const Point& rStop )
{
    MARK( "drawLine" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) )
        return;

    OStringBuffer aLine;
    m_aPages.back().appendPoint( rStart, aLine );
    aLine.append( " m " );
    m_aPages.back().appendPoint( rStop, aLine );
    aLine.append( " l S\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawLine( const Point& rStart, const Point& rStop, const LineInfo& rInfo )
{
    MARK( "drawLine with LineInfo" );
    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) )
        return;

    if( rInfo.GetStyle() == LINE_SOLID && rInfo.GetWidth() < 2 )
    {
        drawLine( rStart, rStop );
        return;
    }

    OStringBuffer aLine;

    aLine.append( "q " );
    if( m_aPages.back().appendLineInfo( rInfo, aLine ) )
    {
        m_aPages.back().appendPoint( rStart, aLine );
        aLine.append( " m " );
        m_aPages.back().appendPoint( rStop, aLine );
        aLine.append( " l S Q\n" );

        writeBuffer( aLine.getStr(), aLine.getLength() );
    }
    else
    {
        PDFWriter::ExtLineInfo aInfo;
        convertLineInfoToExtLineInfo( rInfo, aInfo );
        Point aPolyPoints[2] = { rStart, rStop };
        Polygon aPoly( 2, aPolyPoints );
        drawPolyLine( aPoly, aInfo );
    }
}

void PDFWriterImpl::drawWaveLine( const Point& rStart, const Point& rStop, sal_Int32 nDelta, sal_Int32 nLineWidth )
{
    Point aDiff( rStop-rStart );
    double fLen = sqrt( (double)(aDiff.X()*aDiff.X() + aDiff.Y()*aDiff.Y()) );
    if( fLen < 1.0 )
        return;

    MARK( "drawWaveLine" );
    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) )
        return;

    OStringBuffer aLine( 512 );
    aLine.append( "q " );
    m_aPages.back().appendMappedLength( nLineWidth, aLine, true );
    aLine.append( " w " );

    appendDouble( (double)aDiff.X()/fLen, aLine );
    aLine.append( ' ' );
    appendDouble( -(double)aDiff.Y()/fLen, aLine );
    aLine.append( ' ' );
    appendDouble( (double)aDiff.Y()/fLen, aLine );
    aLine.append( ' ' );
    appendDouble( (double)aDiff.X()/fLen, aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( rStart, aLine );
    aLine.append( " cm " );
    m_aPages.back().appendWaveLine( (sal_Int32)fLen, 0, nDelta, aLine );
    aLine.append( "Q\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

#define WCONV( x ) m_pReferenceDevice->ImplDevicePixelToLogicWidth( x )
#define HCONV( x ) m_pReferenceDevice->ImplDevicePixelToLogicHeight( x )

void PDFWriterImpl::drawTextLine( const Point& rPos, long nWidth, FontStrikeout eStrikeout, FontUnderline eUnderline, bool bUnderlineAbove )
{
    if ( !nWidth ||
         ( ((eStrikeout == STRIKEOUT_NONE)||(eStrikeout == STRIKEOUT_DONTKNOW)) &&
           ((eUnderline == UNDERLINE_NONE)||(eUnderline == UNDERLINE_DONTKNOW)) ) )
        return;

    MARK( "drawTextLine" );
    updateGraphicsState();

    // note: units in pFontEntry are ref device pixel
    ImplFontEntry*  pFontEntry = m_pReferenceDevice->mpFontEntry;
    Color           aUnderlineColor = m_aCurrentPDFState.m_aTextLineColor;
    Color           aStrikeoutColor = m_aCurrentPDFState.m_aFont.GetColor();
    long            nLineHeight = 0;
    long            nLinePos = 0;
    long            nLinePos2 = 0;
    bool            bNormalLines = true;

    if ( bNormalLines &&
         ((eStrikeout == STRIKEOUT_SLASH) || (eStrikeout == STRIKEOUT_X)) )
    {
        String aStrikeoutChar = String::CreateFromAscii( eStrikeout == STRIKEOUT_SLASH ? "/" : "X" );
        String aStrikeout = aStrikeoutChar;
        while( m_pReferenceDevice->GetTextWidth( aStrikeout ) < nWidth )
            aStrikeout.Append( aStrikeout );

        // do not get broader than nWidth modulo 1 character
        while( m_pReferenceDevice->GetTextWidth( aStrikeout ) >= nWidth )
            aStrikeout.Erase( 0, 1 );
        aStrikeout.Append( aStrikeoutChar );
        BOOL bShadow = m_aCurrentPDFState.m_aFont.IsShadow();
        if( bShadow )
        {
            Font aFont = m_aCurrentPDFState.m_aFont;
            aFont.SetShadow( FALSE );
            setFont( aFont );
            updateGraphicsState();
        }

        // strikeout string is left aligned non-CTL text
        ULONG nOrigTLM = m_pReferenceDevice->GetLayoutMode();
        m_pReferenceDevice->SetLayoutMode( TEXT_LAYOUT_BIDI_STRONG|TEXT_LAYOUT_COMPLEX_DISABLED );
        drawText( rPos, aStrikeout, 0, aStrikeout.Len(), false );
        m_pReferenceDevice->SetLayoutMode( nOrigTLM );

        if( bShadow )
        {
            Font aFont = m_aCurrentPDFState.m_aFont;
            aFont.SetShadow( TRUE );
            setFont( aFont );
            updateGraphicsState();
        }

        switch( eUnderline )
        {
            case UNDERLINE_NONE:
            case UNDERLINE_DONTKNOW:
            case UNDERLINE_SMALLWAVE:
            case UNDERLINE_WAVE:
            case UNDERLINE_DOUBLEWAVE:
            case UNDERLINE_BOLDWAVE:
            {
                bNormalLines = FALSE;
            }
            break;
            default:
            {
                ; // No gcc warning
            }
        }
    }

    Point aPos( rPos );
    TextAlign eAlign = m_aCurrentPDFState.m_aFont.GetAlign();
    if( eAlign == ALIGN_TOP )
        aPos.Y() += HCONV( pFontEntry->maMetric.mnAscent );
    else if( eAlign == ALIGN_BOTTOM )
        aPos.Y() -= HCONV( pFontEntry->maMetric.mnDescent );

    OStringBuffer aLine( 512 );
    // save GS
    aLine.append( "q " );

    // rotate and translate matrix
    double fAngle = (double)m_aCurrentPDFState.m_aFont.GetOrientation() * M_PI / 1800.0;
    Matrix3 aMat;
    aMat.rotate( fAngle );
    aMat.translate( aPos.X(), aPos.Y() );
    aMat.append( m_aPages.back(), aLine );
    aLine.append( " cm\n" );

    if ( aUnderlineColor.GetTransparency() != 0 )
        aUnderlineColor = aStrikeoutColor;

    if ( (eUnderline == UNDERLINE_SMALLWAVE) ||
         (eUnderline == UNDERLINE_WAVE) ||
         (eUnderline == UNDERLINE_DOUBLEWAVE) ||
         (eUnderline == UNDERLINE_BOLDWAVE) )
    {
        appendStrokingColor( aUnderlineColor, aLine );
        aLine.append( "\n" );

        if ( bUnderlineAbove )
        {
            if ( !pFontEntry->maMetric.mnAboveWUnderlineSize )
                m_pReferenceDevice->ImplInitAboveTextLineSize();
            nLinePos = HCONV( pFontEntry->maMetric.mnAboveWUnderlineOffset );
            nLineHeight = HCONV( pFontEntry->maMetric.mnAboveWUnderlineSize );
        }
        else
        {
            if ( !pFontEntry->maMetric.mnWUnderlineSize )
                m_pReferenceDevice->ImplInitTextLineSize();
            nLinePos = HCONV( pFontEntry->maMetric.mnWUnderlineOffset );
            nLineHeight = HCONV( pFontEntry->maMetric.mnWUnderlineSize );

        }
        if ( (eUnderline == UNDERLINE_SMALLWAVE) &&
             (nLineHeight > 3) )
            nLineHeight = 3;

        long nLineWidth = getReferenceDevice()->mnDPIX/450;
        if( ! nLineWidth )
            nLineWidth = 1;

        if ( eUnderline == UNDERLINE_BOLDWAVE )
            nLineWidth = 3*nLineWidth;

        m_aPages.back().appendMappedLength( (sal_Int32)nLineWidth, aLine );
        aLine.append( " w " );

        if ( eUnderline == UNDERLINE_DOUBLEWAVE )
        {
            long nOrgLineHeight = nLineHeight;
            nLineHeight /= 3;
            if ( nLineHeight < 2 )
            {
                if ( nOrgLineHeight > 1 )
                    nLineHeight = 2;
                else
                    nLineHeight = 1;
            }
            long nLineDY = nOrgLineHeight-(nLineHeight*2);
            if ( nLineDY < nLineWidth )
                nLineDY = nLineWidth;
            long nLineDY2 = nLineDY/2;
            if ( !nLineDY2 )
                nLineDY2 = 1;

            nLinePos -= nLineWidth-nLineDY2;

            m_aPages.back().appendWaveLine( nWidth, -nLinePos, 2*nLineHeight, aLine );

            nLinePos += nLineWidth+nLineDY;
            m_aPages.back().appendWaveLine( nWidth, -nLinePos, 2*nLineHeight, aLine );
        }
        else
        {
            if( eUnderline != UNDERLINE_BOLDWAVE )
                nLinePos -= nLineWidth/2;
            m_aPages.back().appendWaveLine( nWidth, -nLinePos, nLineHeight, aLine );
        }

        if ( (eStrikeout == STRIKEOUT_NONE) ||
             (eStrikeout == STRIKEOUT_DONTKNOW) )
            bNormalLines = false;
    }

    if ( bNormalLines )
    {
        if ( eUnderline > UNDERLINE_BOLDWAVE )
            eUnderline = UNDERLINE_SINGLE;

        if ( (eUnderline == UNDERLINE_SINGLE) ||
             (eUnderline == UNDERLINE_DOTTED) ||
             (eUnderline == UNDERLINE_DASH) ||
             (eUnderline == UNDERLINE_LONGDASH) ||
             (eUnderline == UNDERLINE_DASHDOT) ||
             (eUnderline == UNDERLINE_DASHDOTDOT) )
        {
            if ( bUnderlineAbove )
            {
                if ( !pFontEntry->maMetric.mnAboveUnderlineSize )
                    m_pReferenceDevice->ImplInitAboveTextLineSize();
                nLineHeight = HCONV( pFontEntry->maMetric.mnAboveUnderlineSize );
                nLinePos    = HCONV( pFontEntry->maMetric.mnAboveUnderlineOffset );
            }
            else
            {
                if ( !pFontEntry->maMetric.mnUnderlineSize )
                    m_pReferenceDevice->ImplInitTextLineSize();
                nLineHeight = HCONV( pFontEntry->maMetric.mnUnderlineSize );
                nLinePos    = HCONV( pFontEntry->maMetric.mnUnderlineOffset );
            }
        }
        else if ( (eUnderline == UNDERLINE_BOLD) ||
                  (eUnderline == UNDERLINE_BOLDDOTTED) ||
                  (eUnderline == UNDERLINE_BOLDDASH) ||

                  (eUnderline == UNDERLINE_BOLDLONGDASH) ||
                  (eUnderline == UNDERLINE_BOLDDASHDOT) ||
                  (eUnderline == UNDERLINE_BOLDDASHDOTDOT) )
        {
            if ( bUnderlineAbove )
            {
                if ( !pFontEntry->maMetric.mnAboveBUnderlineSize )
                    m_pReferenceDevice->ImplInitAboveTextLineSize();
                nLineHeight = HCONV( pFontEntry->maMetric.mnAboveBUnderlineSize );
                nLinePos    = HCONV( pFontEntry->maMetric.mnAboveBUnderlineOffset );
            }
            else
            {
                if ( !pFontEntry->maMetric.mnBUnderlineSize )
                    m_pReferenceDevice->ImplInitTextLineSize();
                nLineHeight = HCONV( pFontEntry->maMetric.mnBUnderlineSize );
                nLinePos    = HCONV( pFontEntry->maMetric.mnBUnderlineOffset );
                nLinePos += nLineHeight/2;
            }
        }
        else if ( eUnderline == UNDERLINE_DOUBLE )
        {
            if ( bUnderlineAbove )
            {
                if ( !pFontEntry->maMetric.mnAboveDUnderlineSize )
                    m_pReferenceDevice->ImplInitAboveTextLineSize();
                nLineHeight = HCONV( pFontEntry->maMetric.mnAboveDUnderlineSize );
                nLinePos    = HCONV( pFontEntry->maMetric.mnAboveDUnderlineOffset1 );
                nLinePos2   = HCONV( pFontEntry->maMetric.mnAboveDUnderlineOffset2 );
            }
            else
            {
                if ( !pFontEntry->maMetric.mnDUnderlineSize )
                    m_pReferenceDevice->ImplInitTextLineSize();
                nLineHeight = HCONV( pFontEntry->maMetric.mnDUnderlineSize );
                nLinePos    = HCONV( pFontEntry->maMetric.mnDUnderlineOffset1 );
                nLinePos2   = HCONV( pFontEntry->maMetric.mnDUnderlineOffset2 );
            }
        }
        else
            nLineHeight = 0;

        if ( nLineHeight )
        {
            m_aPages.back().appendMappedLength( (sal_Int32)nLineHeight, aLine, true );
            aLine.append( " w " );
            appendStrokingColor( aUnderlineColor, aLine );
            aLine.append( "\n" );

            if ( (eUnderline == UNDERLINE_DOTTED) ||
                 (eUnderline == UNDERLINE_BOLDDOTTED) )
            {
                aLine.append( "[ " );
                m_aPages.back().appendMappedLength( (sal_Int32)nLineHeight, aLine, false );
                aLine.append( " ] 0 d\n" );
            }
            else if ( (eUnderline == UNDERLINE_DASH) ||
                      (eUnderline == UNDERLINE_LONGDASH) ||
                      (eUnderline == UNDERLINE_BOLDDASH) ||
                      (eUnderline == UNDERLINE_BOLDLONGDASH) )
            {
                sal_Int32 nDashLength = 4*nLineHeight;
                sal_Int32 nVoidLength = 2*nLineHeight;
                if ( ( eUnderline == UNDERLINE_LONGDASH ) || ( eUnderline == UNDERLINE_BOLDLONGDASH ) )
                    nDashLength = 8*nLineHeight;

                aLine.append( "[ " );
                m_aPages.back().appendMappedLength( nDashLength, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                aLine.append( " ] 0 d\n" );
            }
            else if ( (eUnderline == UNDERLINE_DASHDOT) ||
                      (eUnderline == UNDERLINE_BOLDDASHDOT) )
            {
                sal_Int32 nDashLength = 4*nLineHeight;
                sal_Int32 nVoidLength = 2*nLineHeight;
                aLine.append( "[ " );
                m_aPages.back().appendMappedLength( nDashLength, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( (sal_Int32)nLineHeight, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                aLine.append( " ] 0 d\n" );
            }
            else if ( (eUnderline == UNDERLINE_DASHDOTDOT) ||

                      (eUnderline == UNDERLINE_BOLDDASHDOTDOT) )
            {
                sal_Int32 nDashLength = 4*nLineHeight;
                sal_Int32 nVoidLength = 2*nLineHeight;
                aLine.append( "[ " );
                m_aPages.back().appendMappedLength( nDashLength, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( (sal_Int32)nLineHeight, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( (sal_Int32)nLineHeight, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                aLine.append( " ] 0 d\n" );
            }

            aLine.append( "0 " );
            m_aPages.back().appendMappedLength( (sal_Int32)(-nLinePos), aLine, true );
            aLine.append( " m " );
            m_aPages.back().appendMappedLength( (sal_Int32)nWidth, aLine, false );
            aLine.append( ' ' );
            m_aPages.back().appendMappedLength( (sal_Int32)(-nLinePos), aLine, true );
            aLine.append( " l S\n" );
            if ( eUnderline == UNDERLINE_DOUBLE )
            {
                aLine.append( "0 " );
                m_aPages.back().appendMappedLength( (sal_Int32)(-nLinePos2-nLineHeight), aLine, true );
                aLine.append( " m " );
                m_aPages.back().appendMappedLength( (sal_Int32)nWidth, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( (sal_Int32)(-nLinePos2-nLineHeight), aLine, true );
                aLine.append( " l S\n" );
            }
        }

        if ( eStrikeout > STRIKEOUT_X )
            eStrikeout = STRIKEOUT_SINGLE;

        if ( eStrikeout == STRIKEOUT_SINGLE )
        {
            if ( !pFontEntry->maMetric.mnStrikeoutSize )
                m_pReferenceDevice->ImplInitTextLineSize();
            nLineHeight = HCONV( pFontEntry->maMetric.mnStrikeoutSize );
            nLinePos    = HCONV( pFontEntry->maMetric.mnStrikeoutOffset );
        }
        else if ( eStrikeout == STRIKEOUT_BOLD )
        {
            if ( !pFontEntry->maMetric.mnBStrikeoutSize )
                m_pReferenceDevice->ImplInitTextLineSize();
            nLineHeight = HCONV( pFontEntry->maMetric.mnBStrikeoutSize );
            nLinePos    = HCONV( pFontEntry->maMetric.mnBStrikeoutOffset );

        }
        else if ( eStrikeout == STRIKEOUT_DOUBLE )
        {
            if ( !pFontEntry->maMetric.mnDStrikeoutSize )
                m_pReferenceDevice->ImplInitTextLineSize();
            nLineHeight = HCONV( pFontEntry->maMetric.mnDStrikeoutSize );
            nLinePos    = HCONV( pFontEntry->maMetric.mnDStrikeoutOffset1 );
            nLinePos2   = HCONV( pFontEntry->maMetric.mnDStrikeoutOffset2 );
        }
        else
            nLineHeight = 0;

        if ( nLineHeight )
        {
            m_aPages.back().appendMappedLength( (sal_Int32)nLineHeight, aLine, true );
            aLine.append( " w " );
            appendStrokingColor( aStrikeoutColor, aLine );
            aLine.append( "\n" );

            aLine.append( "0 " );
            m_aPages.back().appendMappedLength( (sal_Int32)(-nLinePos), aLine, true );
            aLine.append( " m " );
            m_aPages.back().appendMappedLength( (sal_Int32)nWidth, aLine, true );
            aLine.append( ' ' );
            m_aPages.back().appendMappedLength( (sal_Int32)(-nLinePos), aLine, true );
            aLine.append( " l S\n" );

            if ( eStrikeout == STRIKEOUT_DOUBLE )
            {
                aLine.append( "0 " );
                m_aPages.back().appendMappedLength( (sal_Int32)(-nLinePos2-nLineHeight), aLine, true );
                aLine.append( " m " );
                m_aPages.back().appendMappedLength( (sal_Int32)nWidth, aLine, true );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( (sal_Int32)(-nLinePos2-nLineHeight), aLine, true );
                aLine.append( " l S\n" );

            }
        }
    }
    aLine.append( "Q\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawPolygon( const Polygon& rPoly )
{
    MARK( "drawPolygon" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

    int nPoints = rPoly.GetSize();
    OStringBuffer aLine( 20 * nPoints );
    m_aPages.back().appendPolygon( rPoly, aLine );
    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor != Color( COL_TRANSPARENT ) )
        aLine.append( "B*\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( "S\n" );
    else
        aLine.append( "f*\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawPolyPolygon( const PolyPolygon& rPolyPoly )
{
    MARK( "drawPolyPolygon" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

    int nPolygons = rPolyPoly.Count();

    OStringBuffer aLine( 40 * nPolygons );
    m_aPages.back().appendPolyPolygon( rPolyPoly, aLine );
    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor != Color( COL_TRANSPARENT ) )
        aLine.append( "B*\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( "S\n" );
    else
        aLine.append( "f*\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawTransparent( const PolyPolygon& rPolyPoly, sal_uInt32 nTransparentPercent )
{
    DBG_ASSERT( nTransparentPercent <= 100, "invalid alpha value" );
    nTransparentPercent = nTransparentPercent % 100;

    MARK( "drawTransparent" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

    if( m_aContext.Version < PDFWriter::PDF_1_4 )
    {
        drawPolyPolygon( rPolyPoly );
        return;
    }

    // create XObject
    m_aTransparentObjects.push_back( TransparencyEmit() );
    // FIXME: polygons with beziers may yield incorrect bound rect
    m_aTransparentObjects.back().m_aBoundRect     = rPolyPoly.GetBoundRect();
    // convert rectangle to default user space
    m_aPages.back().convertRect( m_aTransparentObjects.back().m_aBoundRect );
    m_aTransparentObjects.back().m_nObject          = createObject();
    m_aTransparentObjects.back().m_nExtGStateObject = createObject();
    m_aTransparentObjects.back().m_fAlpha           = (double)(100-nTransparentPercent) / 100.0;
    m_aTransparentObjects.back().m_pContentStream   = new SvMemoryStream( 256, 256 );
    // create XObject's content stream
    OStringBuffer aContent( 256 );
    m_aPages.back().appendPolyPolygon( rPolyPoly, aContent );
    if( m_aCurrentPDFState.m_aLineColor != Color( COL_TRANSPARENT ) &&
        m_aCurrentPDFState.m_aFillColor != Color( COL_TRANSPARENT ) )
        aContent.append( " B*\n" );
    else if( m_aCurrentPDFState.m_aLineColor != Color( COL_TRANSPARENT ) )
        aContent.append( " S\n" );
    else
        aContent.append( " f*\n" );
    m_aTransparentObjects.back().m_pContentStream->Write( aContent.getStr(), aContent.getLength() );

    OStringBuffer aLine( 80 );
    // insert XObject
    aLine.append( "q /EGS" );
    aLine.append( m_aTransparentObjects.back().m_nExtGStateObject );
    aLine.append( " gs " );
    aLine.append( "/Tr" );
    aLine.append( m_aTransparentObjects.back().m_nObject );
    aLine.append( " Do Q\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::beginRedirect( SvStream* pStream, const Rectangle& rTargetRect )
{
    m_aOutputStreams.push_front( StreamRedirect() );
    m_aOutputStreams.front().m_pStream = pStream;
    m_aOutputStreams.front().m_aMapMode = m_aMapMode;

    if( !rTargetRect.IsEmpty() )
    {
        Rectangle aTargetRect = lcl_convert( m_aGraphicsStack.front().m_aMapMode,
                                             m_aMapMode,
                                             getReferenceDevice(),
                                             rTargetRect );
        Point aDelta = aTargetRect.BottomLeft();
        sal_Int32 nPageHeight = m_aPages[m_nCurrentPage].getHeight();
        aDelta.Y() = aTargetRect.Bottom() - pointToPixel(nPageHeight);
        m_aMapMode.SetOrigin( m_aMapMode.GetOrigin() + aDelta );
    }

    // setup graphics state for independent object stream

    // force reemitting colors
    m_aCurrentPDFState.m_aLineColor = Color( COL_TRANSPARENT );
    m_aCurrentPDFState.m_aFillColor = Color( COL_TRANSPARENT );
}

SvStream* PDFWriterImpl::endRedirect()
{
    SvStream* pStream = NULL;
    if( m_aOutputStreams.begin() != m_aOutputStreams.end() )
    {
        pStream     = m_aOutputStreams.front().m_pStream;
        m_aMapMode  = m_aOutputStreams.front().m_aMapMode;
        m_aOutputStreams.pop_front();
    }

    // force reemitting colors
    m_aCurrentPDFState.m_aLineColor = Color( COL_TRANSPARENT );
    m_aCurrentPDFState.m_aFillColor = Color( COL_TRANSPARENT );

    return pStream;
}

void PDFWriterImpl::beginTransparencyGroup()
{
    if( m_aContext.Version >= PDFWriter::PDF_1_4 )
        beginRedirect( new SvMemoryStream( 1024, 1024 ), Rectangle() );
}

void PDFWriterImpl::endTransparencyGroup( const Rectangle& rBoundingBox, sal_uInt32 nTransparentPercent )
{
    DBG_ASSERT( nTransparentPercent <= 100, "invalid alpha value" );
    nTransparentPercent = nTransparentPercent % 100;

    if( m_aContext.Version >= PDFWriter::PDF_1_4 )
    {
        // create XObject
        m_aTransparentObjects.push_back( TransparencyEmit() );
        m_aTransparentObjects.back().m_aBoundRect   = rBoundingBox;
        // convert rectangle to default user space
        m_aPages.back().convertRect( m_aTransparentObjects.back().m_aBoundRect );
        m_aTransparentObjects.back().m_nObject      = createObject();
        m_aTransparentObjects.back().m_fAlpha       = (double)(100-nTransparentPercent) / 100.0;
        // get XObject's content stream
        m_aTransparentObjects.back().m_pContentStream = static_cast<SvMemoryStream*>(endRedirect());
        m_aTransparentObjects.back().m_nExtGStateObject = createObject();

        OStringBuffer aLine( 80 );
        // insert XObject
        aLine.append( "q /EGS" );
        aLine.append( m_aTransparentObjects.back().m_nExtGStateObject );
        aLine.append( " gs /Tr" );
        aLine.append( m_aTransparentObjects.back().m_nObject );
        aLine.append( " Do Q\n" );
        writeBuffer( aLine.getStr(), aLine.getLength() );
    }
}

void PDFWriterImpl::endTransparencyGroup( const Rectangle& rBoundingBox, const Bitmap& rAlphaMask )
{
    if( m_aContext.Version >= PDFWriter::PDF_1_4 )
    {
        // create XObject
        m_aTransparentObjects.push_back( TransparencyEmit() );
        m_aTransparentObjects.back().m_aBoundRect   = rBoundingBox;
        // convert rectangle to default user space
        m_aPages.back().convertRect( m_aTransparentObjects.back().m_aBoundRect );
        m_aTransparentObjects.back().m_nObject      = createObject();
        m_aTransparentObjects.back().m_fAlpha       = 0.0;
        // get XObject's content stream
        m_aTransparentObjects.back().m_pContentStream = static_cast<SvMemoryStream*>(endRedirect());
        m_aTransparentObjects.back().m_nExtGStateObject = createObject();

        // draw soft mask
        beginRedirect( new SvMemoryStream( 1024, 1024 ), Rectangle() );
        drawBitmap( rBoundingBox.TopLeft(), rBoundingBox.GetSize(), rAlphaMask );
        m_aTransparentObjects.back().m_pSoftMaskStream = static_cast<SvMemoryStream*>(endRedirect());

        OStringBuffer aLine( 80 );
        // insert XObject
        aLine.append( "q /EGS" );
        aLine.append( m_aTransparentObjects.back().m_nExtGStateObject );
        aLine.append( " gs /Tr" );
        aLine.append( m_aTransparentObjects.back().m_nObject );
        aLine.append( " Do Q\n" );
        writeBuffer( aLine.getStr(), aLine.getLength() );
    }
}

void PDFWriterImpl::drawRectangle( const Rectangle& rRect )
{
    MARK( "drawRectangle" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

    OStringBuffer aLine( 40 );
    m_aPages.back().appendRect( rRect, aLine );

    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor != Color( COL_TRANSPARENT ) )
        aLine.append( " B*\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( " S\n" );
    else
        aLine.append( " f*\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawRectangle( const Rectangle& rRect, sal_uInt32 nHorzRound, sal_uInt32 nVertRound )
{
    MARK( "drawRectangle with rounded edges" );

    if( !nHorzRound && !nVertRound )
        drawRectangle( rRect );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

    if( nHorzRound > (sal_uInt32)rRect.GetWidth()/2 )
        nHorzRound = rRect.GetWidth()/2;
    if( nVertRound > (sal_uInt32)rRect.GetHeight()/2 )
        nVertRound = rRect.GetHeight()/2;

    Point aPoints[16];
    const double kappa = 0.5522847498;
    const sal_uInt32 kx = (sal_uInt32)((kappa*(double)nHorzRound)+0.5);
    const sal_uInt32 ky = (sal_uInt32)((kappa*(double)nVertRound)+0.5);

    aPoints[1]  = Point( rRect.TopLeft().X() + nHorzRound, rRect.TopLeft().Y() );
    aPoints[0]  = Point( aPoints[1].X() - kx, aPoints[1].Y() );
    aPoints[2]  = Point( rRect.TopRight().X()+1 - nHorzRound, aPoints[1].Y() );
    aPoints[3]  = Point( aPoints[2].X()+kx, aPoints[2].Y() );

    aPoints[5]  = Point( rRect.TopRight().X()+1, rRect.TopRight().Y()+nVertRound );
    aPoints[4]  = Point( aPoints[5].X(), aPoints[5].Y()-ky );
    aPoints[6]  = Point( aPoints[5].X(), rRect.BottomRight().Y()+1 - nVertRound );
    aPoints[7]  = Point( aPoints[6].X(), aPoints[6].Y()+ky );

    aPoints[9]  = Point( rRect.BottomRight().X()+1-nHorzRound, rRect.BottomRight().Y()+1 );
    aPoints[8]  = Point( aPoints[9].X()+kx, aPoints[9].Y() );
    aPoints[10] = Point( rRect.BottomLeft().X() + nHorzRound, aPoints[9].Y() );
    aPoints[11] = Point( aPoints[10].X()-kx, aPoints[10].Y() );

    aPoints[13] = Point( rRect.BottomLeft().X(), rRect.BottomLeft().Y()+1-nVertRound );
    aPoints[12] = Point( aPoints[13].X(), aPoints[13].Y()+ky );
    aPoints[14] = Point( rRect.TopLeft().X(), rRect.TopLeft().Y()+nVertRound );
    aPoints[15] = Point( aPoints[14].X(), aPoints[14].Y()-ky );


    OStringBuffer aLine( 80 );
    m_aPages.back().appendPoint( aPoints[1], aLine );
    aLine.append( " m " );
    m_aPages.back().appendPoint( aPoints[2], aLine );
    aLine.append( " l " );
    m_aPages.back().appendPoint( aPoints[3], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[4], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[5], aLine );
    aLine.append( " c\n" );
    m_aPages.back().appendPoint( aPoints[6], aLine );
    aLine.append( " l " );
    m_aPages.back().appendPoint( aPoints[7], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[8], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[9], aLine );
    aLine.append( " c\n" );
    m_aPages.back().appendPoint( aPoints[10], aLine );
    aLine.append( " l " );
    m_aPages.back().appendPoint( aPoints[11], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[12], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[13], aLine );
    aLine.append( " c\n" );
    m_aPages.back().appendPoint( aPoints[14], aLine );
    aLine.append( " l " );
    m_aPages.back().appendPoint( aPoints[15], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[0], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[1], aLine );
    aLine.append( " c " );

    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor != Color( COL_TRANSPARENT ) )
        aLine.append( "b*\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( "s\n" );
    else
        aLine.append( "f*\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawEllipse( const Rectangle& rRect )
{
    MARK( "drawEllipse" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

    Point aPoints[12];
    const double kappa = 0.5522847498;
    const sal_uInt32 kx = (sal_uInt32)((kappa*(double)rRect.GetWidth()/2.0)+0.5);
    const sal_uInt32 ky = (sal_uInt32)((kappa*(double)rRect.GetHeight()/2.0)+0.5);

    aPoints[1]  = Point( rRect.TopLeft().X() + rRect.GetWidth()/2, rRect.TopLeft().Y() );
    aPoints[0]  = Point( aPoints[1].X() - kx, aPoints[1].Y() );
    aPoints[2]  = Point( aPoints[1].X() + kx, aPoints[1].Y() );

    aPoints[4]  = Point( rRect.TopRight().X()+1, rRect.TopRight().Y() + rRect.GetHeight()/2 );
    aPoints[3]  = Point( aPoints[4].X(), aPoints[4].Y() - ky );
    aPoints[5]  = Point( aPoints[4].X(), aPoints[4].Y() + ky );

    aPoints[7]  = Point( rRect.BottomLeft().X() + rRect.GetWidth()/2, rRect.BottomLeft().Y()+1 );
    aPoints[6]  = Point( aPoints[7].X() + kx, aPoints[7].Y() );
    aPoints[8]  = Point( aPoints[7].X() - kx, aPoints[7].Y() );

    aPoints[10] = Point( rRect.TopLeft().X(), rRect.TopLeft().Y() + rRect.GetHeight()/2 );
    aPoints[9]  = Point( aPoints[10].X(), aPoints[10].Y() + ky );
    aPoints[11] = Point( aPoints[10].X(), aPoints[10].Y() - ky );

    OStringBuffer aLine( 80 );
    m_aPages.back().appendPoint( aPoints[1], aLine );
    aLine.append( " m " );
    m_aPages.back().appendPoint( aPoints[2], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[3], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[4], aLine );
    aLine.append( " c\n" );
    m_aPages.back().appendPoint( aPoints[5], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[6], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[7], aLine );
    aLine.append( " c\n" );
    m_aPages.back().appendPoint( aPoints[8], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[9], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[10], aLine );
    aLine.append( " c\n" );
    m_aPages.back().appendPoint( aPoints[11], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[0], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[1], aLine );
    aLine.append( " c " );

    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor != Color( COL_TRANSPARENT ) )
        aLine.append( "b*\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( "s\n" );
    else
        aLine.append( "f*\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

static double calcAngle( const Rectangle& rRect, const Point& rPoint )
{
    Point aOrigin((rRect.Left()+rRect.Right()+1)/2,
                  (rRect.Top()+rRect.Bottom()+1)/2);
    Point aPoint = rPoint - aOrigin;

    double fX = (double)aPoint.X();
    double fY = (double)-aPoint.Y();

    if( rRect.GetWidth() > rRect.GetHeight() )
        fY = fY*((double)rRect.GetWidth()/(double)rRect.GetHeight());
    else if( rRect.GetHeight() > rRect.GetWidth() )
        fX = fX*((double)rRect.GetHeight()/(double)rRect.GetWidth());
    return atan2( fY, fX );
}

void PDFWriterImpl::drawArc( const Rectangle& rRect, const Point& rStart, const Point& rStop, bool bWithPie, bool bWithChord )
{
    MARK( "drawArc" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

    // calculate start and stop angles
    double fStartAngle = calcAngle( rRect, rStart );
    double fStopAngle  = calcAngle( rRect, rStop );
    while( fStopAngle < fStartAngle )
        fStopAngle += 2.0*M_PI;
    int nFragments = (int)((fStopAngle-fStartAngle)/(M_PI/2.0))+1;
    double fFragmentDelta = (fStopAngle-fStartAngle)/(double)nFragments;
    double kappa = fabs( 4.0 * (1.0-cos(fFragmentDelta/2.0))/sin(fFragmentDelta/2.0) / 3.0);
    double halfWidth = (double)rRect.GetWidth()/2.0;
    double halfHeight = (double)rRect.GetHeight()/2.0;

    Point aCenter( (rRect.Left()+rRect.Right()+1)/2,
                   (rRect.Top()+rRect.Bottom()+1)/2 );

    OStringBuffer aLine( 30*nFragments );
    Point aPoint( (int)(halfWidth * cos(fStartAngle) ),
                  -(int)(halfHeight * sin(fStartAngle) ) );
    aPoint += aCenter;
    m_aPages.back().appendPoint( aPoint, aLine );
    aLine.append( " m " );
    for( int i = 0; i < nFragments; i++ )
    {
        double fStartFragment = fStartAngle + (double)i*fFragmentDelta;
        double fStopFragment = fStartFragment + fFragmentDelta;
        aPoint = Point( (int)(halfWidth * (cos(fStartFragment) - kappa*sin(fStartFragment) ) ),
                        -(int)(halfHeight * (sin(fStartFragment) + kappa*cos(fStartFragment) ) ) );
        aPoint += aCenter;
        m_aPages.back().appendPoint( aPoint, aLine );
        aLine.append( ' ' );

        aPoint = Point( (int)(halfWidth * (cos(fStopFragment) + kappa*sin(fStopFragment) ) ),
                        -(int)(halfHeight * (sin(fStopFragment) - kappa*cos(fStopFragment) ) ) );
        aPoint += aCenter;
        m_aPages.back().appendPoint( aPoint, aLine );
        aLine.append( ' ' );

        aPoint = Point( (int)(halfWidth * cos(fStopFragment) ),
                        -(int)(halfHeight * sin(fStopFragment) ) );
        aPoint += aCenter;
        m_aPages.back().appendPoint( aPoint, aLine );
        aLine.append( " c\n" );
    }
    if( bWithChord || bWithPie )
    {
        if( bWithPie )
        {
            m_aPages.back().appendPoint( aCenter, aLine );
            aLine.append( " l " );
        }
        aLine.append( "h " );
    }
    if( ! bWithChord && ! bWithPie )
        aLine.append( "S\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor != Color( COL_TRANSPARENT ) )
        aLine.append( "B*\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( "S\n" );
    else
        aLine.append( "f*\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawPolyLine( const Polygon& rPoly )
{
    MARK( "drawPolyLine" );

    USHORT nPoints = rPoly.GetSize();
    if( nPoints < 2 )
        return;

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) )
        return;

    OStringBuffer aLine( 20 * nPoints );
    m_aPages.back().appendPolygon( rPoly, aLine, rPoly[0] == rPoly[nPoints-1] );
    aLine.append( "S\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawPolyLine( const Polygon& rPoly, const LineInfo& rInfo )
{
    MARK( "drawPolyLine with LineInfo" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) )
        return;

    OStringBuffer aLine;
    aLine.append( "q " );
    if( m_aPages.back().appendLineInfo( rInfo, aLine ) )
    {
        writeBuffer( aLine.getStr(), aLine.getLength() );
        drawPolyLine( rPoly );
        writeBuffer( "Q\n", 2 );
    }
    else
    {
        PDFWriter::ExtLineInfo aInfo;
        convertLineInfoToExtLineInfo( rInfo, aInfo );
        drawPolyLine( rPoly, aInfo );
    }
}

void PDFWriterImpl::convertLineInfoToExtLineInfo( const LineInfo& rIn, PDFWriter::ExtLineInfo& rOut )
{
    DBG_ASSERT( rIn.GetStyle() == LINE_DASH, "invalid conversion" );
    rOut.m_fLineWidth           = rIn.GetWidth();
    rOut.m_fTransparency        = 0.0;
    rOut.m_eCap                 = PDFWriter::capButt;
    rOut.m_eJoin                = PDFWriter::joinMiter;
    rOut.m_fMiterLimit          = 10;
    rOut.m_aDashArray.clear();

    int nDashes     = rIn.GetDashCount();
    int nDashLen    = rIn.GetDashLen();
    int nDistance   = rIn.GetDistance();
    for( int n  = 0; n < nDashes; n++ )
    {
        rOut.m_aDashArray.push_back( nDashLen );
        rOut.m_aDashArray.push_back( nDistance );
    }
    int nDots       = rIn.GetDotCount();
    int nDotLen     = rIn.GetDotLen();
    for( int n  = 0; n < nDots; n++ )
    {
        rOut.m_aDashArray.push_back( nDotLen );
        rOut.m_aDashArray.push_back( nDistance );
    }
}

void PDFWriterImpl::drawPolyLine( const Polygon& rPoly, const PDFWriter::ExtLineInfo& rInfo )
{
    MARK( "drawPolyLine with ExtLineInfo" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) )
        return;

    if( rInfo.m_fTransparency >= 1.0 )
        return;

    if( rInfo.m_fTransparency != 0.0 )
        beginTransparencyGroup();

    OStringBuffer aLine;
    aLine.append( "q " );
    m_aPages.back().appendMappedLength( rInfo.m_fLineWidth, aLine );
    aLine.append( " w" );
    if( rInfo.m_aDashArray.size() < 10 ) // implmentation limit of acrobat reader
    {
        switch( rInfo.m_eCap )
        {
            default:
            case PDFWriter::capButt:   aLine.append( " 0 J" );break;
            case PDFWriter::capRound:  aLine.append( " 1 J" );break;
            case PDFWriter::capSquare: aLine.append( " 2 J" );break;
        }
        switch( rInfo.m_eJoin )
        {
            default:
            case PDFWriter::joinMiter:
            {
                double fLimit = rInfo.m_fMiterLimit;
                if( rInfo.m_fLineWidth < rInfo.m_fMiterLimit )
                    fLimit = fLimit / rInfo.m_fLineWidth;
                if( fLimit < 1.0 )
                    fLimit = 1.0;
                aLine.append( " 0 j " );
                appendDouble( fLimit, aLine );
                aLine.append( " M" );
            }
            break;
            case PDFWriter::joinRound:  aLine.append( " 1 j" );break;
            case PDFWriter::joinBevel:  aLine.append( " 2 j" );break;
        }
        if( rInfo.m_aDashArray.size() > 0 )
        {
            aLine.append( " [ " );
            for( std::vector<double>::const_iterator it = rInfo.m_aDashArray.begin();
                 it != rInfo.m_aDashArray.end(); ++it )
            {
                m_aPages.back().appendMappedLength( *it, aLine );
                aLine.append( ' ' );
            }
            aLine.append( "] 0 d" );
        }
        aLine.append( "\n" );
        writeBuffer( aLine.getStr(), aLine.getLength() );
        drawPolyLine( rPoly );
    }
    else
    {
        basegfx::B2DPolygon aPoly( rPoly.getB2DPolygon() );
        basegfx::B2DPolyPolygon aPolyPoly( basegfx::tools::applyLineDashing( aPoly, rInfo.m_aDashArray ) );
        sal_uInt32 nPolygons = aPolyPoly.count();
        for( sal_uInt32 nPoly = 0; nPoly < nPolygons; nPoly++ )
        {
            aLine.append( (nPoly != 0 && (nPoly & 7) == 0) ? "\n" : " " );
            aPoly = aPolyPoly.getB2DPolygon( nPoly );
            DBG_ASSERT( aPoly.count() != 2, "erroneous sub polygon" );
            basegfx::B2DPoint aStart = aPoly.getB2DPoint( 0 );
            basegfx::B2DPoint aStop  = aPoly.getB2DPoint( 1 );
            m_aPages.back().appendPoint( Point( FRound(aStart.getX()),
                                                FRound(aStart.getY()) ),
                                         aLine );
            aLine.append( " m " );
            m_aPages.back().appendPoint( Point( FRound(aStop.getX()),
                                                FRound(aStop.getY()) ),
                                         aLine );
            aLine.append( " l" );
        }
        aLine.append( " S " );
        writeBuffer( aLine.getStr(), aLine.getLength() );
    }
    writeBuffer( "Q\n", 2 );

    if( rInfo.m_fTransparency != 0.0 )
    {
        // FIXME: actually this may be incorrect with bezier polygons
        Rectangle aBoundRect( rPoly.GetBoundRect() );
        // avoid clipping with thick lines
        if( rInfo.m_fLineWidth > 0.0 )
        {
            sal_Int32 nLW = sal_Int32(rInfo.m_fLineWidth);
            aBoundRect.Top()    -= nLW;
            aBoundRect.Left()   -= nLW;
            aBoundRect.Right()  += nLW;
            aBoundRect.Bottom() += nLW;
        }
        endTransparencyGroup( aBoundRect, (USHORT)(100.0*rInfo.m_fTransparency) );
    }
}

void PDFWriterImpl::drawPixel( const Point& rPoint, const Color& rColor )
{
    MARK( "drawPixel" );

    Color aColor = ( rColor == Color( COL_TRANSPARENT ) ? m_aGraphicsStack.front().m_aLineColor : rColor );

    if( aColor == Color( COL_TRANSPARENT ) )
        return;

    // pixels are drawn in line color, so have to set
    // the nonstroking color to line color
    Color aOldFillColor = m_aGraphicsStack.front().m_aFillColor;
    setFillColor( aColor );

    updateGraphicsState();

    OStringBuffer aLine( 20 );
    m_aPages.back().appendPoint( rPoint, aLine );
    aLine.append( ' ' );
    appendDouble( 1.0/double(getReferenceDevice()->ImplGetDPIX()), aLine );
    aLine.append( ' ' );
    appendDouble( 1.0/double(getReferenceDevice()->ImplGetDPIY()), aLine );
    aLine.append( " re f\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );

    setFillColor( aOldFillColor );
}

void PDFWriterImpl::drawPixel( const Polygon& rPoints, const Color* pColors )
{
    MARK( "drawPixel with Polygon" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) && ! pColors )
        return;

    USHORT nPoints = rPoints.GetSize();
    OStringBuffer aLine( nPoints*40 );
    aLine.append( "q " );
    if( ! pColors )
    {
        appendNonStrokingColor( m_aGraphicsStack.front().m_aLineColor, aLine );
        aLine.append( ' ' );
    }

    OStringBuffer aPixel(32);
    aPixel.append( ' ' );
    appendDouble( 1.0/double(getReferenceDevice()->ImplGetDPIX()), aPixel );
    aPixel.append( ' ' );
    appendDouble( 1.0/double(getReferenceDevice()->ImplGetDPIY()), aPixel );
    OString aPixelStr = aPixel.makeStringAndClear();
    for( USHORT i = 0; i < nPoints; i++ )
    {
        if( pColors )
        {
            if( pColors[i] == Color( COL_TRANSPARENT ) )
                continue;

            appendNonStrokingColor( pColors[i], aLine );
            aLine.append( ' ' );
        }
        m_aPages.back().appendPoint( rPoints[i], aLine );
        aLine.append( aPixelStr );
        aLine.append( " re f\n" );
    }
    aLine.append( "Q\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

class AccessReleaser
{
    BitmapReadAccess* m_pAccess;
public:
    AccessReleaser( BitmapReadAccess* pAccess ) : m_pAccess( pAccess ){}
    ~AccessReleaser() { delete m_pAccess; }
};

bool PDFWriterImpl::writeTransparentObject( TransparencyEmit& rObject )
{
    CHECK_RETURN( updateObject( rObject.m_nObject ) );

    bool bFlateFilter = compressStream( rObject.m_pContentStream );
    rObject.m_pContentStream->Seek( STREAM_SEEK_TO_END );
    ULONG nSize = rObject.m_pContentStream->Tell();
    rObject.m_pContentStream->Seek( STREAM_SEEK_TO_BEGIN );

    OStringBuffer aLine( 512 );
    CHECK_RETURN( updateObject( rObject.m_nObject ) );
    aLine.append( rObject.m_nObject );
    aLine.append( " 0 obj\n"
                  "<</Type/XObject\n"
                  "/Subtype/Form\n"
                  "/BBox[ " );
    appendFixedInt( rObject.m_aBoundRect.Left(), aLine );
    aLine.append( ' ' );
    appendFixedInt( rObject.m_aBoundRect.Top(), aLine );
    aLine.append( ' ' );
    appendFixedInt( rObject.m_aBoundRect.Right(), aLine );
    aLine.append( ' ' );
    appendFixedInt( rObject.m_aBoundRect.Bottom()+1, aLine );
    aLine.append( " ]\n" );
    /* #i42884# the PDF reference recommends that each Form XObject
    *  should have a resource dict; alas if that is the same object
    *  as the one of the page it triggers an endless recursion in
    *  acroread 5 (6 and up have that fixed). Since we have only one
    *  resource dict anyway, let's use the one from the page by NOT
    *  emitting a Resources entry.
    */
    #if 0
    aLine.append( "   /Resources " );
    aLine.append( getResourceDictObj() );
    aLine.append( " 0 R\n" );
    #endif

    aLine.append( "/Length " );
    aLine.append( (sal_Int32)(nSize) );
    aLine.append( "\n" );
    if( bFlateFilter )
        aLine.append( "/Filter/FlateDecode\n" );
    aLine.append( ">>\n"
                  "stream\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    CHECK_RETURN( writeBuffer( rObject.m_pContentStream->GetData(), nSize ) );
    aLine.setLength( 0 );
    aLine.append( "\n"
                  "endstream\n"
                  "endobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    // write ExtGState dict for this XObject
    aLine.setLength( 0 );
    aLine.append( rObject.m_nExtGStateObject );
    aLine.append( " 0 obj\n"
                  "<<" );
    if( ! rObject.m_pSoftMaskStream )
    {
        aLine.append(  "/CA " );
        appendDouble( rObject.m_fAlpha, aLine );
        aLine.append( "\n"
                  "   /ca " );
        appendDouble( rObject.m_fAlpha, aLine );
        aLine.append( "\n" );
    }
    else
    {
        rObject.m_pSoftMaskStream->Seek( STREAM_SEEK_TO_END );
        sal_Int32 nMaskSize = (sal_Int32)rObject.m_pSoftMaskStream->Tell();
        rObject.m_pSoftMaskStream->Seek( STREAM_SEEK_TO_BEGIN );
        sal_Int32 nMaskObject = createObject();
        aLine.append( "/SMask<</Type/Mask/S/Luminosity/G " );
        aLine.append( nMaskObject );
        aLine.append( " 0 R>>\n" );

        OStringBuffer aMask;
        aMask.append( nMaskObject );
        aMask.append( " 0 obj\n"
                      "<</Type/XObject\n"
                      "/Subtype/Form\n"
                      "/BBox[" );
        appendFixedInt( rObject.m_aBoundRect.Left(), aMask );
        aMask.append( ' ' );
        appendFixedInt( rObject.m_aBoundRect.Top(), aMask );
        aMask.append( ' ' );
        appendFixedInt( rObject.m_aBoundRect.Right(), aMask );
        aMask.append( ' ' );
        appendFixedInt( rObject.m_aBoundRect.Bottom()+1, aMask );
        aMask.append( "]\n" );

        /* #i42884# see above */
        #if 0
        aLine.append( "/Resources " );
        aMask.append( getResourceDictObj() );
        aMask.append( " 0 R\n" );
        #endif

        aMask.append( "/Group<</S/Transparency/CS/DeviceRGB>>\n" );
        aMask.append( "/Length " );
        aMask.append( nMaskSize );
        aMask.append( ">>\n"
                      "stream\n" );
        CHECK_RETURN( updateObject( nMaskObject ) );
        CHECK_RETURN( writeBuffer( aMask.getStr(), aMask.getLength() ) );
        CHECK_RETURN( writeBuffer( rObject.m_pSoftMaskStream->GetData(), nMaskSize ) );
        aMask.setLength( 0 );
        aMask.append( "\nendstream\n"
                      "endobj\n\n" );
        CHECK_RETURN( writeBuffer( aMask.getStr(), aMask.getLength() ) );
    }
    aLine.append( ">>\n"
                  "endobj\n\n" );
    CHECK_RETURN( updateObject( rObject.m_nExtGStateObject ) );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    return true;
}

bool PDFWriterImpl::writeGradientFunction( GradientEmit& rObject )
{
    sal_Int32 nFunctionObject = createObject();
    CHECK_RETURN( updateObject( nFunctionObject ) );

    OutputDevice* pRefDevice = getReferenceDevice();
    pRefDevice->Push( PUSH_ALL );
    if( rObject.m_aSize.Width() > pRefDevice->GetOutputSizePixel().Width() )
        rObject.m_aSize.Width() = pRefDevice->GetOutputSizePixel().Width();
    if( rObject.m_aSize.Height() > pRefDevice->GetOutputSizePixel().Height() )
        rObject.m_aSize.Height() = pRefDevice->GetOutputSizePixel().Height();
    pRefDevice->SetMapMode( MapMode( MAP_PIXEL ) );
    pRefDevice->DrawGradient( Rectangle( Point( 0, 0 ), rObject.m_aSize ), rObject.m_aGradient );

    Bitmap aSample = pRefDevice->GetBitmap( Point( 0, 0 ), rObject.m_aSize );
    BitmapReadAccess* pAccess = aSample.AcquireReadAccess();
    AccessReleaser aReleaser( pAccess );

    Size aSize = aSample.GetSizePixel();

    sal_Int32 nStreamLengthObject = createObject();
    OStringBuffer aLine( 120 );
    aLine.append( nFunctionObject );
    aLine.append( " 0 obj\n"
                  "<</FunctionType 0\n"
                  "/Domain[ 0 1 0 1 ]\n"
                  "/Size[ " );
    aLine.append( (sal_Int32)aSize.Width() );
    aLine.append( ' ' );
    aLine.append( (sal_Int32)aSize.Height() );
    aLine.append( " ]\n"
                  "/BitsPerSample 8\n"
                  "/Range[ 0 1 0 1 0 1 ]\n"
                  "/Length " );
    aLine.append( nStreamLengthObject );
    aLine.append( " 0 R\n"
#ifndef DEBUG_DISABLE_PDFCOMPRESSION
                  "/Filter/FlateDecode"
#endif
                  ">>\n"
                  "stream\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    sal_uInt64 nStartStreamPos = 0;
    CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nStartStreamPos )) );

    beginCompression();
    for( int y = 0; y < aSize.Height(); y++ )
    {
        for( int x = 0; x < aSize.Width(); x++ )
        {
            sal_uInt8 aCol[3];
            BitmapColor aColor = pAccess->GetColor( y, x );
            aCol[0] = aColor.GetRed();
            aCol[1] = aColor.GetGreen();
            aCol[2] = aColor.GetBlue();
            CHECK_RETURN( writeBuffer( aCol, 3 ) );
        }
    }
    endCompression();

    sal_uInt64 nEndStreamPos = 0;
    CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nEndStreamPos )) );

    aLine.setLength( 0 );
    aLine.append( "\nendstream\nendobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    // write stream length
    CHECK_RETURN( updateObject( nStreamLengthObject ) );
    aLine.setLength( 0 );
    aLine.append( nStreamLengthObject );
    aLine.append( " 0 obj\n" );
    aLine.append( (sal_Int64)(nEndStreamPos-nStartStreamPos) );
    aLine.append( "\nendobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    CHECK_RETURN( updateObject( rObject.m_nObject ) );
    aLine.setLength( 0 );
    aLine.append( rObject.m_nObject );
    aLine.append( " 0 obj\n"
                  "<</ShadingType 1\n"
                  "/ColorSpace/DeviceRGB\n"
                  "/AntiAlias true\n"
                  "/Domain[ 0 1 0 1 ]\n"
                  "/Matrix[ " );
    aLine.append( (sal_Int32)aSize.Width() );
    aLine.append( " 0 0 " );
    aLine.append( (sal_Int32)aSize.Height() );
    aLine.append( " 0 0 ]\n"
                  "/Function " );
    aLine.append( nFunctionObject );
    aLine.append( " 0 R\n"
                  ">>\n"
                  "endobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    pRefDevice->Pop();

    return true;
}

bool PDFWriterImpl::writeJPG( JPGEmit& rObject )
{
    CHECK_RETURN( rObject.m_pStream );
    CHECK_RETURN( updateObject( rObject.m_nObject ) );

    sal_Int32 nLength = 0;
    rObject.m_pStream->Seek( STREAM_SEEK_TO_END );
    nLength = rObject.m_pStream->Tell();
    rObject.m_pStream->Seek( STREAM_SEEK_TO_BEGIN );

    sal_Int32 nMaskObject = 0;
    if( !!rObject.m_aMask )
    {
        if( rObject.m_aMask.GetBitCount() == 1 ||
            ( rObject.m_aMask.GetBitCount() == 8 && m_aContext.Version >= PDFWriter::PDF_1_4 )
            )
            nMaskObject = createObject();
    }

    OStringBuffer aLine(200);
    aLine.append( rObject.m_nObject );
    aLine.append( " 0 obj\n"
                  "<</Type/XObject/Subtype/Image/Width " );
    aLine.append( (sal_Int32)rObject.m_aID.m_aPixelSize.Width() );
    aLine.append( " /Height " );
    aLine.append( (sal_Int32)rObject.m_aID.m_aPixelSize.Height() );
    aLine.append( " /BitsPerComponent 8 " );
    if( rObject.m_bTrueColor )
        aLine.append( "/ColorSpace/DeviceRGB" );
    else
        aLine.append( "/ColorSpace/DeviceGray" );
    aLine.append( "/Filter/DCTDecode/Length " );
    aLine.append( nLength );
    if( nMaskObject )
    {
        aLine.append( rObject.m_aMask.GetBitCount() == 1 ? " /Mask " : " /SMask " );
        aLine.append( nMaskObject );
        aLine.append( " 0 R " );
    }
    aLine.append( ">>\nstream\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    CHECK_RETURN( writeBuffer( rObject.m_pStream->GetData(), nLength ) );

    aLine.setLength( 0 );
    aLine.append( "\nendstream\nendobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    if( nMaskObject )
    {
        BitmapEmit aEmit;
        aEmit.m_nObject = nMaskObject;
        if( rObject.m_aMask.GetBitCount() == 1 )
            aEmit.m_aBitmap = BitmapEx( rObject.m_aMask, rObject.m_aMask );
        else if( rObject.m_aMask.GetBitCount() == 8 )
            aEmit.m_aBitmap = BitmapEx( rObject.m_aMask, AlphaMask( rObject.m_aMask ) );
        writeBitmapObject( aEmit, true );
    }

    return true;
}

bool PDFWriterImpl::writeBitmapObject( BitmapEmit& rObject, bool bMask )
{
    CHECK_RETURN( updateObject( rObject.m_nObject ) );

    Bitmap  aBitmap;
    Color   aTransparentColor( COL_TRANSPARENT );
    bool    bWriteMask = false;
    if( ! bMask )
    {
        aBitmap = rObject.m_aBitmap.GetBitmap();
        if( rObject.m_aBitmap.IsAlpha() )
        {
            if( m_aContext.Version >= PDFWriter::PDF_1_4 )
                bWriteMask = true;
            // else draw without alpha channel
        }
        else
        {
            switch( rObject.m_aBitmap.GetTransparentType() )
            {
                case TRANSPARENT_NONE:
                    // comes from drawMask function
                    if( aBitmap.GetBitCount() == 1 && rObject.m_bDrawMask )
                        bMask = true;
                    break;
                case TRANSPARENT_COLOR:
                    aTransparentColor = rObject.m_aBitmap.GetTransparentColor();
                    break;
                case TRANSPARENT_BITMAP:
                    bWriteMask = true;
                    break;
            }
        }
    }
    else
    {
        if( m_aContext.Version < PDFWriter::PDF_1_4 || ! rObject.m_aBitmap.IsAlpha() )
        {
            aBitmap = rObject.m_aBitmap.GetMask();
            aBitmap.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
            DBG_ASSERT( aBitmap.GetBitCount() == 1, "mask conversion failed" );
        }
        else if( aBitmap.GetBitCount() != 8 )
        {
            aBitmap = rObject.m_aBitmap.GetAlpha().GetBitmap();
            aBitmap.Convert( BMP_CONVERSION_8BIT_GREYS );
            DBG_ASSERT( aBitmap.GetBitCount() == 8, "alpha mask conversion failed" );
        }
    }

    BitmapReadAccess* pAccess = aBitmap.AcquireReadAccess();
    AccessReleaser aReleaser( pAccess );

    bool bTrueColor;
    sal_Int32 nBitsPerComponent;
    switch( aBitmap.GetBitCount() )
    {
        case 1:
        case 2:
        case 4:
        case 8:
            bTrueColor = false;
            nBitsPerComponent = aBitmap.GetBitCount();
            break;
        default:
            bTrueColor = true;
            nBitsPerComponent = 8;
            break;
    }

    sal_Int32 nStreamLengthObject   = createObject();
    sal_Int32 nMaskObject           = 0;

    OStringBuffer aLine(1024);
    aLine.append( rObject.m_nObject );
    aLine.append( " 0 obj\n"
                  "<</Type/XObject/Subtype/Image/Width " );
    aLine.append( (sal_Int32)aBitmap.GetSizePixel().Width() );
    aLine.append( " /Height " );
    aLine.append( (sal_Int32)aBitmap.GetSizePixel().Height() );
    aLine.append( " /BitsPerComponent " );
    aLine.append( nBitsPerComponent );
    aLine.append( " /Length " );
    aLine.append( nStreamLengthObject );
    aLine.append( " 0 R\n" );
#ifndef DEBUG_DISABLE_PDFCOMPRESSION
    aLine.append( "/Filter/FlateDecode" );
#endif
    if( ! bMask )
    {
        aLine.append( "/ColorSpace" );
        if( bTrueColor )
            aLine.append( "/DeviceRGB\n" );
        else if( aBitmap.HasGreyPalette() )
        {
            aLine.append( "/DeviceGray\n" );
            if( aBitmap.GetBitCount() == 1 )
            {
                // #i47395# 1 bit bitmaps occasionally have an inverted grey palette
                sal_Int32 nBlackIndex = pAccess->GetBestPaletteIndex( BitmapColor( Color( COL_BLACK ) ) );
                DBG_ASSERT( nBlackIndex == 0 || nBlackIndex == 1, "wrong black index" );
                if( nBlackIndex == 1 )
                    aLine.append( "/Decode[1 0]\n" );
            }
        }
        else
        {
            aLine.append( "[ /Indexed/DeviceRGB " );
            aLine.append( (sal_Int32)(pAccess->GetPaletteEntryCount()-1) );
            aLine.append( " <\n" );
            for( USHORT i = 0; i < pAccess->GetPaletteEntryCount(); i++ )
            {
                const BitmapColor& rColor = pAccess->GetPaletteColor( i );
                appendHex( rColor.GetRed(), aLine );
                appendHex( rColor.GetGreen(), aLine );
                appendHex( rColor.GetBlue(), aLine );
                if( (i+1) & 15 )
                    aLine.append( ' ' );
                else
                    aLine.append( "\n" );
            }
            aLine.append( "> ]\n" );
        }
    }
    else
    {
        if( aBitmap.GetBitCount() == 1 )
        {
            aLine.append( " /ImageMask true\n" );
            sal_Int32 nBlackIndex = pAccess->GetBestPaletteIndex( BitmapColor( Color( COL_BLACK ) ) );
            DBG_ASSERT( nBlackIndex == 0 || nBlackIndex == 1, "wrong black index" );
            if( nBlackIndex )
                aLine.append( "/Decode[ 1 0 ]\n" );
            else
                aLine.append( "/Decode[ 0 1 ]\n" );
        }
        else if( aBitmap.GetBitCount() == 8 )
        {
            aLine.append( "/ColorSpace/DeviceGray\n"
                          "/Decode [ 1 0 ]\n" );
        }
    }

    if( ! bMask && m_aContext.Version > PDFWriter::PDF_1_2 )
    {
        if( bWriteMask )
        {
            nMaskObject = createObject();
            if( rObject.m_aBitmap.IsAlpha() && m_aContext.Version > PDFWriter::PDF_1_3 )
                aLine.append( "/SMask " );
            else
                aLine.append( "/Mask " );
            aLine.append( nMaskObject );
            aLine.append( " 0 R\n" );
        }
        else if( aTransparentColor != Color( COL_TRANSPARENT ) )
        {
            aLine.append( "/Mask[ " );
            if( bTrueColor )
            {
                aLine.append( (sal_Int32)aTransparentColor.GetRed() );
                aLine.append( ' ' );
                aLine.append( (sal_Int32)aTransparentColor.GetRed() );
                aLine.append( ' ' );
                aLine.append( (sal_Int32)aTransparentColor.GetGreen() );
                aLine.append( ' ' );
                aLine.append( (sal_Int32)aTransparentColor.GetGreen() );
                aLine.append( ' ' );
                aLine.append( (sal_Int32)aTransparentColor.GetBlue() );
                aLine.append( ' ' );
                aLine.append( (sal_Int32)aTransparentColor.GetBlue() );
            }
            else
            {
                sal_Int32 nIndex = pAccess->GetBestPaletteIndex( BitmapColor( aTransparentColor ) );
                aLine.append( nIndex );
            }
            aLine.append( " ]\n" );
        }
    }
    aLine.append( ">>\n"
                  "stream\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    sal_uInt64 nStartPos = 0;
    CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nStartPos )) );

    beginCompression();
    if( ! bTrueColor || pAccess->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_RGB )
    {
        const int nScanLineBytes = 1 + ( pAccess->GetBitCount() * ( pAccess->Width() - 1 ) / 8U );

        for( int i = 0; i < pAccess->Height(); i++ )
        {
            CHECK_RETURN( writeBuffer( pAccess->GetScanline( i ), nScanLineBytes ) );
        }
    }
    else
    {
        const int nScanLineBytes = pAccess->Width()*3;
        sal_uInt8 *pCol = (sal_uInt8*)rtl_allocateMemory( nScanLineBytes );
        for( int y = 0; y < pAccess->Height(); y++ )
        {
            for( int x = 0; x < pAccess->Width(); x++ )
            {
                BitmapColor aColor = pAccess->GetColor( y, x );
                pCol[3*x+0] = aColor.GetRed();
                pCol[3*x+1] = aColor.GetGreen();
                pCol[3*x+2] = aColor.GetBlue();
            }
            CHECK_RETURN( writeBuffer( pCol, nScanLineBytes ) );
        }
        rtl_freeMemory( pCol );
    }
    endCompression();

    sal_uInt64 nEndPos = 0;
    CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nEndPos )) );
    aLine.setLength( 0 );
    aLine.append( "\nendstream\nendobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    CHECK_RETURN( updateObject( nStreamLengthObject ) );
    aLine.setLength( 0 );
    aLine.append( nStreamLengthObject );
    aLine.append( " 0 obj\n" );
    aLine.append( (sal_Int64)(nEndPos-nStartPos) );
    aLine.append( "\nendobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    if( nMaskObject )
    {
        BitmapEmit aEmit;
        aEmit.m_nObject             = nMaskObject;
        aEmit.m_aBitmap             = rObject.m_aBitmap;
        return writeBitmapObject( aEmit, true );
    }

    return true;
}

void PDFWriterImpl::drawJPGBitmap( SvStream& rDCTData, bool bIsTrueColor, const Size& rSizePixel, const Rectangle& rTargetArea, const Bitmap& rMask )
{
    MARK( "drawJPGBitmap" );

    OStringBuffer aLine( 80 );
    updateGraphicsState();

    // #i40055# sanity check
    if( ! (rTargetArea.GetWidth() && rTargetArea.GetHeight() ) )
        return;
    if( ! (rSizePixel.Width() && rSizePixel.Height()) )
        return;

    SvMemoryStream* pStream = new SvMemoryStream;
    rDCTData.Seek( 0 );
    *pStream << rDCTData;
    pStream->Seek( STREAM_SEEK_TO_END );

    BitmapID aID;
    aID.m_aPixelSize    = rSizePixel;
    aID.m_nSize         = pStream->Tell();
    pStream->Seek( STREAM_SEEK_TO_BEGIN );
    aID.m_nChecksum     = rtl_crc32( 0, pStream->GetData(), aID.m_nSize );
    if( ! rMask.IsEmpty() )
        aID.m_nMaskChecksum = rMask.GetChecksum();

    std::list< JPGEmit >::const_iterator it;
    for( it = m_aJPGs.begin(); it != m_aJPGs.end() && ! (aID == it->m_aID); ++it )
        ;
    if( it == m_aJPGs.end() )
    {
        m_aJPGs.push_front( JPGEmit() );
        JPGEmit& rEmit = m_aJPGs.front();
        rEmit.m_nObject     = createObject();
        rEmit.m_aID         = aID;
        rEmit.m_pStream     = pStream;
        rEmit.m_bTrueColor  = bIsTrueColor;
        if( !! rMask && rMask.GetSizePixel() == rSizePixel )
            rEmit.m_aMask   = rMask;

        it = m_aJPGs.begin();
    }
    else
        delete pStream;

    aLine.append( "q " );
    m_aPages.back().appendMappedLength( (sal_Int32)rTargetArea.GetWidth(), aLine, false );
    aLine.append( " 0 0 " );
    m_aPages.back().appendMappedLength( (sal_Int32)rTargetArea.GetHeight(), aLine, true );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( rTargetArea.BottomLeft(), aLine );
    aLine.append( " cm\n/Im" );
    aLine.append( it->m_nObject );
    aLine.append( " Do Q\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawBitmap( const Point& rDestPoint, const Size& rDestSize, const BitmapEmit& rBitmap, const Color& rFillColor )
{
    OStringBuffer aLine( 80 );
    updateGraphicsState();

    aLine.append( "q " );
    if( rFillColor != Color( COL_TRANSPARENT ) )
    {
        appendNonStrokingColor( rFillColor, aLine );
        aLine.append( ' ' );
    }
    m_aPages.back().appendMappedLength( (sal_Int32)rDestSize.Width(), aLine, false );
    aLine.append( " 0 0 " );
    m_aPages.back().appendMappedLength( (sal_Int32)rDestSize.Height(), aLine, true );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( rDestPoint + Point( 0, rDestSize.Height()-1 ), aLine );
    aLine.append( " cm\n/Im" );
    aLine.append( rBitmap.m_nObject );
    aLine.append( " Do Q\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

const PDFWriterImpl::BitmapEmit& PDFWriterImpl::createBitmapEmit( const BitmapEx& rBitmap, bool bDrawMask )
{
    BitmapID aID;
    aID.m_aPixelSize        = rBitmap.GetSizePixel();
    aID.m_nSize             = rBitmap.GetBitCount();
    aID.m_nChecksum         = rBitmap.GetBitmap().GetChecksum();
    aID.m_nMaskChecksum     = 0;
    if( rBitmap.IsAlpha() )
        aID.m_nMaskChecksum = rBitmap.GetAlpha().GetChecksum();
    else
    {
        Bitmap aMask = rBitmap.GetMask();
        if( ! aMask.IsEmpty() )
            aID.m_nMaskChecksum = aMask.GetChecksum();
    }
    for( std::list< BitmapEmit >::const_iterator it = m_aBitmaps.begin(); it != m_aBitmaps.end(); ++it )
    {
        if( aID == it->m_aID )
            return *it;
    }
    m_aBitmaps.push_front( BitmapEmit() );
    m_aBitmaps.front().m_aID        = aID;
    m_aBitmaps.front().m_aBitmap    = rBitmap;
    m_aBitmaps.front().m_nObject    = createObject();
    m_aBitmaps.front().m_bDrawMask  = bDrawMask;

    return m_aBitmaps.front();
}

void PDFWriterImpl::drawBitmap( const Point& rDestPoint, const Size& rDestSize, const Bitmap& rBitmap )
{
    MARK( "drawBitmap (Bitmap)" );

    // #i40055# sanity check
    if( ! (rDestSize.Width() && rDestSize.Height()) )
        return;

    const BitmapEmit& rEmit = createBitmapEmit( BitmapEx( rBitmap ) );
    drawBitmap( rDestPoint, rDestSize, rEmit, Color( COL_TRANSPARENT ) );
}

void PDFWriterImpl::drawBitmap( const Point& rDestPoint, const Size& rDestSize, const BitmapEx& rBitmap )
{
    MARK( "drawBitmap (BitmapEx)" );

    // #i40055# sanity check
    if( ! (rDestSize.Width() && rDestSize.Height()) )
        return;

    const BitmapEmit& rEmit = createBitmapEmit( rBitmap );
    drawBitmap( rDestPoint, rDestSize, rEmit, Color( COL_TRANSPARENT ) );
}

void PDFWriterImpl::drawMask( const Point& rDestPoint, const Size& rDestSize, const Bitmap& rBitmap, const Color& rFillColor )
{
    MARK( "drawMask" );

    // #i40055# sanity check
    if( ! (rDestSize.Width() && rDestSize.Height()) )
        return;

    Bitmap aBitmap( rBitmap );
    if( aBitmap.GetBitCount() > 1 )
        aBitmap.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
    DBG_ASSERT( aBitmap.GetBitCount() == 1, "mask conversion failed" );

    const BitmapEmit& rEmit = createBitmapEmit( BitmapEx( aBitmap ), true );
    drawBitmap( rDestPoint, rDestSize, rEmit, rFillColor );
}

sal_Int32 PDFWriterImpl::createGradient( const Gradient& rGradient, const Size& rSize )
{
    Size aPtSize( lcl_convert( m_aGraphicsStack.front().m_aMapMode,
                               MapMode( MAP_POINT ),
                               getReferenceDevice(),
                               rSize ) );
    // check if we already have this gradient
    for( std::list<GradientEmit>::iterator it = m_aGradients.begin(); it != m_aGradients.end(); ++it )
    {
        if( it->m_aGradient == rGradient )
        {
            if( it->m_aSize.Width() < aPtSize.Width() )
                it->m_aSize.Width() = aPtSize.Width();
            if( it->m_aSize.Height() <= aPtSize.Height() )
                it->m_aSize.Height() = aPtSize.Height();
            return it->m_nObject;
        }
    }

    m_aGradients.push_back( GradientEmit() );
    m_aGradients.back().m_aGradient = rGradient;
    m_aGradients.back().m_nObject   = createObject();
    m_aGradients.back().m_aSize     = aPtSize;
    return m_aGradients.back().m_nObject;
}

void PDFWriterImpl::drawGradient( const Rectangle& rRect, const Gradient& rGradient )
{
    MARK( "drawGradient (Rectangle)" );

    if( m_aContext.Version == PDFWriter::PDF_1_2 )
    {
        drawRectangle( rRect );
        return;
    }

    sal_Int32 nGradient = createGradient( rGradient, rRect.GetSize() );

    Point aTranslate( rRect.BottomLeft() );
    aTranslate += Point( 0, 1 );

    updateGraphicsState();

    OStringBuffer aLine( 80 );
    aLine.append( "q 1 0 0 1 " );
    m_aPages.back().appendPoint( aTranslate, aLine );
    aLine.append( " cm " );
    // if a stroke is appended reset the clip region before stroke
    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( "q " );
    aLine.append( "0 0 " );
    m_aPages.back().appendMappedLength( (sal_Int32)rRect.GetWidth(), aLine, false );
    aLine.append( ' ' );
    m_aPages.back().appendMappedLength( (sal_Int32)rRect.GetHeight(), aLine, true );
    aLine.append( " re W n\n" );

    aLine.append( "/P" );
    aLine.append( nGradient );
    aLine.append( " sh " );
    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
    {
        aLine.append( "Q 0 0 " );
        m_aPages.back().appendMappedLength( (sal_Int32)rRect.GetWidth(), aLine, false );
        aLine.append( ' ' );
        m_aPages.back().appendMappedLength( (sal_Int32)rRect.GetHeight(), aLine, true );
        aLine.append( " re S " );
    }
    aLine.append( "Q\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawGradient( const PolyPolygon& rPolyPoly, const Gradient& rGradient )
{
    MARK( "drawGradient (PolyPolygon)" );

    if( m_aContext.Version == PDFWriter::PDF_1_2 )
    {
        drawPolyPolygon( rPolyPoly );
        return;
    }

    sal_Int32 nGradient = createGradient( rGradient, rPolyPoly.GetBoundRect().GetSize() );

    updateGraphicsState();

    Rectangle aBoundRect = rPolyPoly.GetBoundRect();
    Point aTranslate = aBoundRect.BottomLeft() + Point( 0, 1 );
    int nPolygons = rPolyPoly.Count();

    OStringBuffer aLine( 80*nPolygons );
    aLine.append( "q " );
    // set PolyPolygon as clip path
    m_aPages.back().appendPolyPolygon( rPolyPoly, aLine );
    aLine.append( "W* n\n" );
    aLine.append( "1 0 0 1 " );
    m_aPages.back().appendPoint( aTranslate, aLine );
    aLine.append( " cm\n" );
    aLine.append( "/P" );
    aLine.append( nGradient );
    aLine.append( " sh Q\n" );
    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
    {
        // and draw the surrounding path
        m_aPages.back().appendPolyPolygon( rPolyPoly, aLine );
        aLine.append( "S\n" );
    }
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawHatch( const PolyPolygon& rPolyPoly, const Hatch& rHatch )
{
    MARK( "drawHatch" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;
    if( rPolyPoly.Count() )
    {
        PolyPolygon     aPolyPoly( rPolyPoly );

        aPolyPoly.Optimize( POLY_OPTIMIZE_NO_SAME );
        push( PUSH_LINECOLOR );
        setLineColor( rHatch.GetColor() );
        getReferenceDevice()->ImplDrawHatch( aPolyPoly, rHatch, FALSE );
        pop();
    }
}

void PDFWriterImpl::drawWallpaper( const Rectangle& rRect, const Wallpaper& rWall )
{
    MARK( "drawWallpaper" );

    bool bDrawColor         = false;
    bool bDrawGradient      = false;
    bool bDrawBitmap        = false;

    BitmapEx aBitmap;
    Point aBmpPos = rRect.TopLeft();
    Size aBmpSize;
    if( rWall.IsBitmap() )
    {
        aBitmap = rWall.GetBitmap();
        aBmpSize = lcl_convert( aBitmap.GetPrefMapMode(),
                                getMapMode(),
                                getReferenceDevice(),
                                aBitmap.GetPrefSize() );
        Rectangle aRect( rRect );
        if( rWall.IsRect() )
        {
            aRect = rWall.GetRect();
            aBmpPos = aRect.TopLeft();
            aBmpSize = aRect.GetSize();
        }
        if( rWall.GetStyle() != WALLPAPER_SCALE )
        {
            if( rWall.GetStyle() != WALLPAPER_TILE )
            {
                bDrawBitmap     = true;
                if( rWall.IsGradient() )
                    bDrawGradient = true;
                else
                    bDrawColor = true;
                switch( rWall.GetStyle() )
                {
                    case WALLPAPER_TOPLEFT:
                        break;
                    case WALLPAPER_TOP:
                        aBmpPos.X() += (aRect.GetWidth()-aBmpSize.Width())/2;
                        break;
                    case WALLPAPER_LEFT:
                        aBmpPos.Y() += (aRect.GetHeight()-aBmpSize.Height())/2;
                        break;
                    case WALLPAPER_TOPRIGHT:
                        aBmpPos.X() += aRect.GetWidth()-aBmpSize.Width();
                        break;
                    case WALLPAPER_CENTER:
                        aBmpPos.X() += (aRect.GetWidth()-aBmpSize.Width())/2;
                        aBmpPos.Y() += (aRect.GetHeight()-aBmpSize.Height())/2;
                        break;
                    case WALLPAPER_RIGHT:
                        aBmpPos.X() += aRect.GetWidth()-aBmpSize.Width();
                        aBmpPos.Y() += (aRect.GetHeight()-aBmpSize.Height())/2;
                        break;
                    case WALLPAPER_BOTTOMLEFT:
                        aBmpPos.Y() += aRect.GetHeight()-aBmpSize.Height();
                        break;
                    case WALLPAPER_BOTTOM:
                        aBmpPos.X() += (aRect.GetWidth()-aBmpSize.Width())/2;
                        aBmpPos.Y() += aRect.GetHeight()-aBmpSize.Height();
                        break;
                    case WALLPAPER_BOTTOMRIGHT:
                        aBmpPos.X() += aRect.GetWidth()-aBmpSize.Width();
                        aBmpPos.Y() += aRect.GetHeight()-aBmpSize.Height();
                        break;
                    default: ;
                }
            }
            else
            {
                // push the bitmap
                const BitmapEmit& rEmit = createBitmapEmit( BitmapEx( aBitmap ) );

                // convert to page coordinates; this needs to be done here
                // since the emit does not know the page anymore
                Rectangle aConvertRect( aBmpPos, aBmpSize );
                m_aPages.back().convertRect( aConvertRect );

                // push the pattern
                m_aTilings.push_back( BitmapPatternEmit() );
                m_aTilings.back().m_nObject         = createObject();
                m_aTilings.back().m_nBitmapObject   = rEmit.m_nObject;
                m_aTilings.back().m_aRectangle      = aConvertRect;

                updateGraphicsState();

                // fill a rRect with the pattern
                OStringBuffer aLine( 100 );
                aLine.append( "q /Pattern cs /P" );
                aLine.append( m_aTilings.back().m_nObject );
                aLine.append( " scn " );
                m_aPages.back().appendRect( rRect, aLine );
                aLine.append( " f Q\n" );
                writeBuffer( aLine.getStr(), aLine.getLength() );
            }
        }
        else
        {
            aBmpPos     = aRect.TopLeft();
            aBmpSize    = aRect.GetSize();
            bDrawBitmap = true;
        }

        if( aBitmap.IsTransparent() )
        {
            if( rWall.IsGradient() )
                bDrawGradient = true;
            else
                bDrawColor = true;
        }
    }
    else if( rWall.IsGradient() )
        bDrawGradient = true;
    else
        bDrawColor = true;

    if( bDrawGradient )
    {
        drawGradient( rRect, rWall.GetGradient() );
    }
    if( bDrawColor )
    {
        Color aOldLineColor = m_aGraphicsStack.front().m_aLineColor;
        Color aOldFillColor = m_aGraphicsStack.front().m_aFillColor;
        setLineColor( Color( COL_TRANSPARENT ) );
        setFillColor( rWall.GetColor() );
        drawRectangle( rRect );
        setLineColor( aOldLineColor );
        setFillColor( aOldFillColor );
    }
    if( bDrawBitmap )
    {
        // set temporary clip region since aBmpPos and aBmpSize
        // may be outside rRect
        OStringBuffer aLine( 20 );
        aLine.append( "q " );
        m_aPages.back().appendRect( rRect, aLine );
        aLine.append( " W n\n" );
        writeBuffer( aLine.getStr(), aLine.getLength() );
        drawBitmap( aBmpPos, aBmpSize, aBitmap );
        writeBuffer( "Q\n", 2 );
    }
}

void PDFWriterImpl::updateGraphicsState()
{
    OStringBuffer aLine( 256 );
    GraphicsState& rNewState = m_aGraphicsStack.front();
    // first set clip region since it might invalidate everything else

    if( (rNewState.m_nUpdateFlags & GraphicsState::updateClipRegion) )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsState::updateClipRegion;

        Region& rNewClip = rNewState.m_aClipRegion;

        /*  #103137# equality operator is not implemented
        *  const as API promises but may change Region
        *  from Polygon to rectangles. Arrrgghh !!!!
        */
        Region aLeft = m_aCurrentPDFState.m_aClipRegion;
        Region aRight = rNewClip;
        if( aLeft != aRight )
        {
            if( ! m_aCurrentPDFState.m_aClipRegion.IsEmpty() &&
                ! m_aCurrentPDFState.m_aClipRegion.IsNull() )
            {
                aLine.append( "Q " );
                // invalidate everything but the clip region
                m_aCurrentPDFState = GraphicsState();
                rNewState.m_nUpdateFlags = sal::static_int_cast<sal_uInt16>(~GraphicsState::updateClipRegion);
            }
            if( ! rNewClip.IsEmpty() && ! rNewClip.IsNull() )
            {
                // clip region is always stored in private PDF mapmode
                MapMode aNewMapMode = rNewState.m_aMapMode;
                rNewState.m_aMapMode = m_aMapMode;
                getReferenceDevice()->SetMapMode( rNewState.m_aMapMode );
                m_aCurrentPDFState.m_aMapMode = rNewState.m_aMapMode;

                aLine.append( "q " );
                if( rNewClip.HasPolyPolygon() )
                {
                    m_aPages.back().appendPolyPolygon( rNewClip.GetPolyPolygon(), aLine );
                    aLine.append( "W* n\n" );
                }
                else
                {
                    // need to clip all rectangles
                    RegionHandle aHandle = rNewClip.BeginEnumRects();
                    Rectangle aRect;
                    while( rNewClip.GetNextEnumRect( aHandle, aRect ) )
                    {
                        m_aPages.back().appendRect( aRect, aLine );
                        if( aLine.getLength() > 80 )
                        {
                            aLine.append( "\n" );
                            writeBuffer( aLine.getStr(), aLine.getLength() );
                            aLine.setLength( 0 );
                        }
                        else
                            aLine.append( ' ' );
                    }
                    rNewClip.EndEnumRects( aHandle );
                    aLine.append( "W* n\n" );
                }

                rNewState.m_aMapMode = aNewMapMode;
                getReferenceDevice()->SetMapMode( rNewState.m_aMapMode );
                m_aCurrentPDFState.m_aMapMode = rNewState.m_aMapMode;
            }
        }
    }

    if( (rNewState.m_nUpdateFlags & GraphicsState::updateMapMode) )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsState::updateMapMode;
        getReferenceDevice()->SetMapMode( rNewState.m_aMapMode );
    }

    if( (rNewState.m_nUpdateFlags & GraphicsState::updateFont) )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsState::updateFont;
        getReferenceDevice()->SetFont( rNewState.m_aFont );
        getReferenceDevice()->ImplNewFont();
    }

    if( (rNewState.m_nUpdateFlags & GraphicsState::updateLayoutMode) )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsState::updateLayoutMode;
        getReferenceDevice()->SetLayoutMode( rNewState.m_nLayoutMode );
    }

    if( (rNewState.m_nUpdateFlags & GraphicsState::updateDigitLanguage) )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsState::updateDigitLanguage;
        getReferenceDevice()->SetDigitLanguage( rNewState.m_aDigitLanguage );
    }

    if( (rNewState.m_nUpdateFlags & GraphicsState::updateLineColor) )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsState::updateLineColor;
        if( m_aCurrentPDFState.m_aLineColor != rNewState.m_aLineColor &&
            rNewState.m_aLineColor != Color( COL_TRANSPARENT ) )
        {
            appendStrokingColor( rNewState.m_aLineColor, aLine );
            aLine.append( "\n" );
        }
    }

    if( (rNewState.m_nUpdateFlags & GraphicsState::updateFillColor) )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsState::updateFillColor;
        if( m_aCurrentPDFState.m_aFillColor != rNewState.m_aFillColor &&
            rNewState.m_aFillColor != Color( COL_TRANSPARENT ) )
        {
            appendNonStrokingColor( rNewState.m_aFillColor, aLine );
            aLine.append( "\n" );
        }
    }

    if( (rNewState.m_nUpdateFlags & GraphicsState::updateTransparentPercent) )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsState::updateTransparentPercent;
        if( m_aContext.Version >= PDFWriter::PDF_1_4 && m_aCurrentPDFState.m_nTransparentPercent != rNewState.m_nTransparentPercent )
        {
            // TODO: switch extended graphicsstate
        }
    }

    // everything is up to date now
    m_aCurrentPDFState = m_aGraphicsStack.front();
    if( aLine.getLength() )
        writeBuffer( aLine.getStr(), aLine.getLength() );
}

/* #i47544# imitate OutputDevice behaviour:
*  if a font with a nontransparent color is set, it overwrites the current
*  text color. OTOH setting the text color will overwrite the color of the font.
*/
void PDFWriterImpl::setFont( const Font& rFont )
{
    Color aColor = rFont.GetColor();
    if( aColor == Color( COL_TRANSPARENT ) )
        aColor = m_aGraphicsStack.front().m_aFont.GetColor();
    m_aGraphicsStack.front().m_aFont = rFont;
    m_aGraphicsStack.front().m_aFont.SetColor( aColor );
    m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateFont;
}

void PDFWriterImpl::push( sal_uInt16 nFlags )
{
    m_aGraphicsStack.push_front( m_aGraphicsStack.front() );
    m_aGraphicsStack.front().m_nFlags = nFlags;
}

void PDFWriterImpl::pop()
{
    GraphicsState aState = m_aGraphicsStack.front();
    m_aGraphicsStack.pop_front();
    GraphicsState& rOld = m_aGraphicsStack.front();

    // move those parameters back that were not pushed
    // in the first place
    if( ! (aState.m_nFlags & PUSH_LINECOLOR) )
        setLineColor( aState.m_aLineColor );
    if( ! (aState.m_nFlags & PUSH_FILLCOLOR) )
        setFillColor( aState.m_aFillColor );
    if( ! (aState.m_nFlags & PUSH_FONT) )
        setFont( aState.m_aFont );
    if( ! (aState.m_nFlags & PUSH_TEXTCOLOR) )
        setTextColor( aState.m_aFont.GetColor() );
    if( ! (aState.m_nFlags & PUSH_MAPMODE) )
        setMapMode( aState.m_aMapMode );
    if( ! (aState.m_nFlags & PUSH_CLIPREGION) )
        // do not use setClipRegion here
        // it would convert again assuming the current mapmode
        rOld.m_aClipRegion = aState.m_aClipRegion;
    if( ! (aState.m_nFlags & PUSH_TEXTLINECOLOR ) )
        setTextLineColor( aState.m_aTextLineColor );
    if( ! (aState.m_nFlags & PUSH_TEXTALIGN ) )
        setTextAlign( aState.m_aFont.GetAlign() );
    if( ! (aState.m_nFlags & PUSH_TEXTFILLCOLOR) )
        setTextFillColor( aState.m_aFont.GetFillColor() );
    if( ! (aState.m_nFlags & PUSH_REFPOINT) )
    {
        // what ?
    }
    // invalidate graphics state
    m_aGraphicsStack.front().m_nUpdateFlags = sal::static_int_cast<sal_uInt16>(~0U);
}

void PDFWriterImpl::setMapMode( const MapMode& rMapMode )
{
    m_aGraphicsStack.front().m_aMapMode = rMapMode;
    getReferenceDevice()->SetMapMode( rMapMode );
    m_aCurrentPDFState.m_aMapMode = rMapMode;
}

void PDFWriterImpl::setClipRegion( const Region& rRegion )
{
    Region aRegion = getReferenceDevice()->LogicToPixel( rRegion, m_aGraphicsStack.front().m_aMapMode );
    aRegion = getReferenceDevice()->PixelToLogic( aRegion, m_aMapMode );
    m_aGraphicsStack.front().m_aClipRegion = aRegion;
    m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateClipRegion;
}

void PDFWriterImpl::moveClipRegion( sal_Int32 nX, sal_Int32 nY )
{
    Point aPoint( lcl_convert( m_aGraphicsStack.front().m_aMapMode,
                               m_aMapMode,
                               getReferenceDevice(),
                               Point( nX, nY ) ) );
    aPoint -= lcl_convert( m_aGraphicsStack.front().m_aMapMode,
                           m_aMapMode,
                           getReferenceDevice(),
                           Point() );
    m_aGraphicsStack.front().m_aClipRegion.Move( aPoint.X(), aPoint.Y() );
    m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateClipRegion;
}

bool PDFWriterImpl::intersectClipRegion( const Rectangle& rRect )
{
    Rectangle aRect( lcl_convert( m_aGraphicsStack.front().m_aMapMode,
                                  m_aMapMode,
                                  getReferenceDevice(),
                                  rRect ) );
    m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateClipRegion;
    return m_aGraphicsStack.front().m_aClipRegion.Intersect( aRect );
}


bool PDFWriterImpl::intersectClipRegion( const Region& rRegion )
{
    Region aRegion = getReferenceDevice()->LogicToPixel( rRegion, m_aGraphicsStack.front().m_aMapMode );
    aRegion = getReferenceDevice()->PixelToLogic( aRegion, m_aMapMode );
    m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateClipRegion;
    return m_aGraphicsStack.front().m_aClipRegion.Intersect( aRegion );
}

void PDFWriterImpl::createNote( const Rectangle& rRect, const PDFNote& rNote, sal_Int32 nPageNr )
{
    if( nPageNr < 0 )
        nPageNr = m_nCurrentPage;

    if( nPageNr < 0 || nPageNr >= (sal_Int32)m_aPages.size() )
        return;

    m_aNotes.push_back( PDFNoteEntry() );
    m_aNotes.back().m_nObject       = createObject();
    m_aNotes.back().m_aContents     = rNote;
    m_aNotes.back().m_aRect         = rRect;
    // convert to default user space now, since the mapmode may change
    m_aPages[nPageNr].convertRect( m_aNotes.back().m_aRect );

    // insert note to page's annotation list
    m_aPages[ nPageNr ].m_aAnnotations.push_back( m_aNotes.back().m_nObject );
}

sal_Int32 PDFWriterImpl::createLink( const Rectangle& rRect, sal_Int32 nPageNr )
{
    if( nPageNr < 0 )
        nPageNr = m_nCurrentPage;

    if( nPageNr < 0 || nPageNr >= (sal_Int32)m_aPages.size() )
        return -1;

    sal_Int32 nRet = m_aLinks.size();

    m_aLinks.push_back( PDFLink() );
    m_aLinks.back().m_nObject   = createObject();
    m_aLinks.back().m_nPage     = nPageNr;
    m_aLinks.back().m_aRect     = rRect;
    // convert to default user space now, since the mapmode may change
    m_aPages[nPageNr].convertRect( m_aLinks.back().m_aRect );

    // insert link to page's annotation list
    m_aPages[ nPageNr ].m_aAnnotations.push_back( m_aLinks.back().m_nObject );

    return nRet;
}

sal_Int32 PDFWriterImpl::createDest( const Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType )
{
    if( nPageNr < 0 )
        nPageNr = m_nCurrentPage;

    if( nPageNr < 0 || nPageNr >= (sal_Int32)m_aPages.size() )
        return -1;

    sal_Int32 nRet = m_aDests.size();

    m_aDests.push_back( PDFDest() );
    m_aDests.back().m_nPage = nPageNr;
    m_aDests.back().m_eType = eType;
    m_aDests.back().m_aRect = rRect;
    // convert to default user space now, since the mapmode may change
    m_aPages[nPageNr].convertRect( m_aDests.back().m_aRect );

    return nRet;
}

sal_Int32 PDFWriterImpl::setLinkDest( sal_Int32 nLinkId, sal_Int32 nDestId )
{
    if( nLinkId < 0 || nLinkId >= (sal_Int32)m_aLinks.size() )
        return -1;
    if( nDestId < 0 || nDestId >= (sal_Int32)m_aDests.size() )
        return -2;

    m_aLinks[ nLinkId ].m_nDest = nDestId;

    return 0;
}

static OUString escapeStringLiteral( const OUString& rStr )
{
    OUStringBuffer aBuf( rStr.getLength()*2 );
    const sal_Unicode* pUni = rStr.getStr();
    int nLen = rStr.getLength();
    for( ; nLen; nLen--, pUni++ )
    {
        switch( *pUni )
        {
            case sal_Unicode(')'):
            case sal_Unicode('('):
            case sal_Unicode('\\'):
                aBuf.append( sal_Unicode( '\\' ) );
            default:
                aBuf.append( *pUni );
                break;
        }
    }
    return aBuf.makeStringAndClear();
}

sal_Int32 PDFWriterImpl::setLinkURL( sal_Int32 nLinkId, const OUString& rURL )
{
    if( nLinkId < 0 || nLinkId >= (sal_Int32)m_aLinks.size() )
        return -1;

    m_aLinks[ nLinkId ].m_nDest = -1;
    m_aLinks[ nLinkId ].m_aURL  = escapeStringLiteral( rURL );

    return 0;
}

void PDFWriterImpl::setLinkPropertyId( sal_Int32 nLinkId, sal_Int32 nPropertyId )
{
    m_aLinkPropertyMap[ nPropertyId ] = nLinkId;
}

sal_Int32 PDFWriterImpl::createOutlineItem( sal_Int32 nParent, const OUString& rText, sal_Int32 nDestID )
{
    // create new item
    sal_Int32 nNewItem = m_aOutline.size();
    m_aOutline.push_back( PDFOutlineEntry() );

    // set item attributes
    setOutlineItemParent( nNewItem, nParent );
    setOutlineItemText( nNewItem, rText );
    setOutlineItemDest( nNewItem, nDestID );

    return nNewItem;
}

sal_Int32 PDFWriterImpl::setOutlineItemParent( sal_Int32 nItem, sal_Int32 nNewParent )
{
    if( nItem < 1 || nItem >= (sal_Int32)m_aOutline.size() )
        return -1;

    int nRet = 0;

    if( nNewParent < 0 || nNewParent >= (sal_Int32)m_aOutline.size() || nNewParent == nItem )
    {
        nNewParent = 0;
        nRet = -2;
    }
    // remove item from previous parent
    sal_Int32 nParentID = m_aOutline[ nItem ].m_nParentID;
    if( nParentID >= 0 && nParentID < (sal_Int32)m_aOutline.size() )
    {
        PDFOutlineEntry& rParent = m_aOutline[ nParentID ];

        for( std::vector<sal_Int32>::iterator it = rParent.m_aChildren.begin();
             it != rParent.m_aChildren.end(); ++it )
        {
            if( *it == nItem )
            {
                rParent.m_aChildren.erase( it );
                break;
            }
        }
    }

    // insert item to new parent's list of children
    m_aOutline[ nNewParent ].m_aChildren.push_back( nItem );

    return nRet;
}

sal_Int32 PDFWriterImpl::setOutlineItemText( sal_Int32 nItem, const OUString& rText )
{
    if( nItem < 1 || nItem >= (sal_Int32)m_aOutline.size() )
        return -1;

    m_aOutline[ nItem ].m_aTitle = rText;
    return 0;
}

sal_Int32 PDFWriterImpl::setOutlineItemDest( sal_Int32 nItem, sal_Int32 nDestID )
{
    if( nItem < 1 || nItem >= (sal_Int32)m_aOutline.size() ) // item does not exist
        return -1;
    if( nDestID < 0 || nDestID >= (sal_Int32)m_aDests.size() ) // dest does not exist
        return -2;
    m_aOutline[nItem].m_nDestID = nDestID;
    return 0;
}

const sal_Char* PDFWriterImpl::getStructureTag( PDFWriter::StructElement eType )
{
    static std::map< PDFWriter::StructElement, const char* > aTagStrings;
    if( aTagStrings.empty() )
    {
        aTagStrings[ PDFWriter::NonStructElement] = "NonStruct";
        aTagStrings[ PDFWriter::Document ]      = "Document";
        aTagStrings[ PDFWriter::Part ]          = "Part";
        aTagStrings[ PDFWriter::Article ]       = "Art";
        aTagStrings[ PDFWriter::Section ]       = "Sect";
        aTagStrings[ PDFWriter::Division ]      = "Div";
        aTagStrings[ PDFWriter::BlockQuote ]    = "BlockQuote";
        aTagStrings[ PDFWriter::Caption ]       = "Caption";
        aTagStrings[ PDFWriter::TOC ]           = "TOC";
        aTagStrings[ PDFWriter::TOCI ]          = "TOCI";
        aTagStrings[ PDFWriter::Index ]         = "Index";
        aTagStrings[ PDFWriter::Paragraph ]     = "P";
        aTagStrings[ PDFWriter::Heading ]       = "H";
        aTagStrings[ PDFWriter::H1 ]            = "H1";
        aTagStrings[ PDFWriter::H2 ]            = "H2";
        aTagStrings[ PDFWriter::H3 ]            = "H3";
        aTagStrings[ PDFWriter::H4 ]            = "H4";
        aTagStrings[ PDFWriter::H5 ]            = "H5";
        aTagStrings[ PDFWriter::H6 ]            = "H6";
        aTagStrings[ PDFWriter::List ]          = "L";
        aTagStrings[ PDFWriter::ListItem ]      = "LI";
        aTagStrings[ PDFWriter::LILabel ]       = "Lbl";
        aTagStrings[ PDFWriter::LIBody ]        = "LBody";
        aTagStrings[ PDFWriter::Table ]         = "Table";
        aTagStrings[ PDFWriter::TableRow ]      = "TR";
        aTagStrings[ PDFWriter::TableHeader ]   = "TH";
        aTagStrings[ PDFWriter::TableData ]     = "TD";
        aTagStrings[ PDFWriter::Span ]          = "Span";
        aTagStrings[ PDFWriter::Quote ]         = "Quote";
        aTagStrings[ PDFWriter::Note ]          = "Note";
        aTagStrings[ PDFWriter::Reference ]     = "Reference";
        aTagStrings[ PDFWriter::BibEntry ]      = "BibEntry";
        aTagStrings[ PDFWriter::Code ]          = "Code";
        aTagStrings[ PDFWriter::Link ]          = "Link";
        aTagStrings[ PDFWriter::Figure ]        = "Figure";
        aTagStrings[ PDFWriter::Formula ]       = "Formula";
        aTagStrings[ PDFWriter::Form ]          = "Form";
    }

    std::map< PDFWriter::StructElement, const char* >::const_iterator it = aTagStrings.find( eType );

    return it != aTagStrings.end() ? it->second : "Div";
}

void PDFWriterImpl::beginStructureElementMCSeq()
{
    if( m_bEmitStructure &&
        m_nCurrentStructElement > 0 && // StructTreeRoot
        ! m_aStructure[ m_nCurrentStructElement ].m_bOpenMCSeq // already opened sequence
        )
    {
        PDFStructureElement& rEle = m_aStructure[ m_nCurrentStructElement ];
        OStringBuffer aLine( 128 );
        sal_Int32 nMCID = m_aPages[ m_nCurrentPage ].m_aMCIDParents.size();
        aLine.append( "/" );
        aLine.append( getStructureTag( rEle.m_eType ) );
        aLine.append( "<</MCID " );
        aLine.append( nMCID );
        aLine.append( ">>BDC\n" );
        writeBuffer( aLine.getStr(), aLine.getLength() );

        // update the element's content list
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "beginning marked content id %ld on page object %ld, structure first page = %ld\n",
                 nMCID,
                 m_aPages[ m_nCurrentPage ].m_nPageObject,
                 rEle.m_nFirstPageObject );
#endif
        rEle.m_aKids.push_back( PDFStructureElementKid( nMCID, m_aPages[m_nCurrentPage].m_nPageObject ) );
        // update the page's mcid parent list
        m_aPages[ m_nCurrentPage ].m_aMCIDParents.push_back( rEle.m_nObject );
        // mark element MC sequence as open
        rEle.m_bOpenMCSeq = true;
    }
}

void PDFWriterImpl::endStructureElementMCSeq()
{
    if( m_bEmitStructure &&
        m_nCurrentStructElement > 0 && // StructTreeRoot
        m_aStructure[ m_nCurrentStructElement ].m_bOpenMCSeq // must have an opened MC sequence
        )
    {
        writeBuffer( "EMC\n", 4 );
        m_aStructure[ m_nCurrentStructElement ].m_bOpenMCSeq = false;
    }
}

bool PDFWriterImpl::checkEmitStructure()
{
    bool bEmit = false;
    if( m_aContext.Tagged )
    {
        bEmit = true;
        sal_Int32 nEle = m_nCurrentStructElement;
        while( nEle > 0 && nEle < sal_Int32(m_aStructure.size()) )
        {
            if( m_aStructure[ nEle ].m_eType == PDFWriter::NonStructElement )
            {
                bEmit = false;
                break;
            }
            nEle = m_aStructure[ nEle ].m_nParentElement;
        }
    }
    return bEmit;
}

sal_Int32 PDFWriterImpl::beginStructureElement( PDFWriter::StructElement eType )
{
    if( m_nCurrentPage < 0 )
        return -1;

    if( ! m_aContext.Tagged )
        return -1;

    // close eventual current MC sequence
    endStructureElementMCSeq();

    if( m_nCurrentStructElement == 0 &&
        eType != PDFWriter::Document && eType != PDFWriter::NonStructElement )
    {
        // struct tree root hit, but not beginning document
        // this might happen with setCurrentStructureElement
        // silently insert structure into document again if one properly exists
        if( ! m_aStructure[ 0 ].m_aChildren.empty() )
        {
            PDFWriter::StructElement childType = PDFWriter::NonStructElement;
            sal_Int32 nNewCurElement = 0;
            const std::list< sal_Int32 >& rRootChildren = m_aStructure[0].m_aChildren;
            for( std::list< sal_Int32 >::const_iterator it = rRootChildren.begin();
                 childType != PDFWriter::Document && it != rRootChildren.end(); ++it )
            {
                nNewCurElement = *it;
                childType = m_aStructure[ nNewCurElement ].m_eType;
            }
            if( childType == PDFWriter::Document )
            {
                m_nCurrentStructElement = nNewCurElement;
                DBG_ASSERT( 0, "Structure element inserted to StructTreeRoot that is not a document" );
            }
            else
                DBG_ERROR( "document structure in disorder !" );
        }
        else
            DBG_ERROR( "PDF document structure MUST be contained in a Document element" );
    }

#if OSL_DEBUG_LEVEL > 1
    if( m_bEmitStructure )
    {
        OStringBuffer aLine( "beginStructureElement " );
        aLine.append( sal_Int32(m_aStructure.size() ) );
        aLine.append( ": " );
        aLine.append( getStructureTag( eType ) );
        emitComment( aLine.getStr() );
    }
#endif

    sal_Int32 nNewId = sal_Int32(m_aStructure.size());
    m_aStructure.push_back( PDFStructureElement() );
    PDFStructureElement& rEle = m_aStructure.back();
    rEle.m_eType            = eType;
    rEle.m_nOwnElement      = nNewId;
    rEle.m_nParentElement   = m_nCurrentStructElement;
    rEle.m_nFirstPageObject = m_aPages[ m_nCurrentPage ].m_nPageObject;
    m_aStructure[ m_nCurrentStructElement ].m_aChildren.push_back( nNewId );
    m_nCurrentStructElement = nNewId;

    // check whether to emit structure henceforth
    m_bEmitStructure = checkEmitStructure();

    if( m_bEmitStructure ) // don't create nonexistant objects
    {
        rEle.m_nObject      = createObject();
        // update parent's kids list
        m_aStructure[ rEle.m_nParentElement ].m_aKids.push_back( rEle.m_nObject );
    }
    return nNewId;
}

void PDFWriterImpl::endStructureElement()
{
    if( m_nCurrentPage < 0 )
        return;

    if( ! m_aContext.Tagged )
        return;

    if( m_nCurrentStructElement == 0 )
    {
        // hit the struct tree root, that means there is an endStructureElement
        // without corresponding beginStructureElement
        return;
    }

    // end the marked content sequence
    endStructureElementMCSeq();

#if OSL_DEBUG_LEVEL > 1
    OStringBuffer aLine( "endStructureElement " );
    aLine.append( m_nCurrentStructElement );
    aLine.append( ": " );
    aLine.append( getStructureTag( m_aStructure[ m_nCurrentStructElement ].m_eType ) );
#endif

    // "end" the structure element, the parent becomes current element
    m_nCurrentStructElement = m_aStructure[ m_nCurrentStructElement ].m_nParentElement;

    // check whether to emit structure henceforth
    m_bEmitStructure = checkEmitStructure();

#if OSL_DEBUG_LEVEL > 1
    if( m_bEmitStructure )
        emitComment( aLine.getStr() );
#endif
}

bool PDFWriterImpl::setCurrentStructureElement( sal_Int32 nEle )
{
    bool bSuccess = false;

    if( m_aContext.Tagged && nEle >= 0 && nEle < sal_Int32(m_aStructure.size()) )
    {
        // end eventual previous marked content sequence
        endStructureElementMCSeq();

        m_nCurrentStructElement = nEle;
        m_bEmitStructure = checkEmitStructure();
#if OSL_DEBUG_LEVEL > 1
        OStringBuffer aLine( "setCurrentStructureElement " );
        aLine.append( m_nCurrentStructElement );
        aLine.append( ": " );
        aLine.append( getStructureTag( m_aStructure[ m_nCurrentStructElement ].m_eType ) );
        if( ! m_bEmitStructure )
            aLine.append( " (inside NonStruct)" );
        emitComment( aLine.getStr() );
#endif
        bSuccess = true;
    }

    return bSuccess;
}

sal_Int32 PDFWriterImpl::getCurrentStructureElement()
{
    return m_nCurrentStructElement;
}

bool PDFWriterImpl::setStructureAttribute( enum PDFWriter::StructAttribute eAttr, enum PDFWriter::StructAttributeValue eVal )
{
    if( !m_aContext.Tagged )
        return false;

    bool bInsert = false;
    if( m_nCurrentStructElement > 0 && m_bEmitStructure )
    {
        PDFWriter::StructElement eType = m_aStructure[ m_nCurrentStructElement ].m_eType;
        switch( eAttr )
        {
            case PDFWriter::Placement:
                if( eVal == PDFWriter::Block        ||
                    eVal == PDFWriter::Inline       ||
                    eVal == PDFWriter::Before       ||
                    eVal == PDFWriter::Start        ||
                    eVal == PDFWriter::End )
                    bInsert = true;
                break;
            case PDFWriter::WritingMode:
                if( eVal == PDFWriter::LrTb         ||
                    eVal == PDFWriter::RlTb         ||
                    eVal == PDFWriter::TbRl )
                {
                    bInsert = true;
                }
                break;
            case PDFWriter::TextAlign:
                if( eVal == PDFWriter::Start        ||
                    eVal == PDFWriter::Center       ||
                    eVal == PDFWriter::End          ||
                    eVal == PDFWriter::Justify )
                {
                    if( eType == PDFWriter::Paragraph   ||
                        eType == PDFWriter::Heading     ||
                        eType == PDFWriter::H1          ||
                        eType == PDFWriter::H2          ||
                        eType == PDFWriter::H3          ||
                        eType == PDFWriter::H4          ||
                        eType == PDFWriter::H5          ||
                        eType == PDFWriter::H6          ||
                        eType == PDFWriter::List        ||
                        eType == PDFWriter::ListItem    ||
                        eType == PDFWriter::LILabel     ||
                        eType == PDFWriter::LIBody      ||
                        eType == PDFWriter::Table       ||
                        eType == PDFWriter::TableRow    ||
                        eType == PDFWriter::TableHeader ||
                        eType == PDFWriter::TableData )
                    {
                        bInsert = true;
                    }
                }
                break;
            case PDFWriter::Width:
            case PDFWriter::Height:
                if( eVal == PDFWriter::Auto )
                {
                    if( eType == PDFWriter::Figure      ||
                        eType == PDFWriter::Formula     ||
                        eType == PDFWriter::Form        ||
                        eType == PDFWriter::Table       ||
                        eType == PDFWriter::TableHeader ||
                        eType == PDFWriter::TableData )
                    {
                        bInsert = true;
                    }
                }
                break;
            case PDFWriter::BlockAlign:
                if( eVal == PDFWriter::Before       ||
                    eVal == PDFWriter::Middle       ||
                    eVal == PDFWriter::After        ||
                    eVal == PDFWriter::Justify )
                {
                    if( eType == PDFWriter::TableHeader ||
                        eType == PDFWriter::TableData )
                    {
                        bInsert = true;
                    }
                }
                break;
            case PDFWriter::InlineAlign:
                if( eVal == PDFWriter::Start        ||
                    eVal == PDFWriter::Center       ||
                    eVal == PDFWriter::End )
                {
                    if( eType == PDFWriter::TableHeader ||
                        eType == PDFWriter::TableData )
                    {
                        bInsert = true;
                    }
                }
                break;
            case PDFWriter::LineHeight:
                if( eVal == PDFWriter::Normal       ||
                    eVal == PDFWriter::Auto )
                {
                    // only for ILSE and BLSE
                    if( eType == PDFWriter::Paragraph   ||
                        eType == PDFWriter::Heading     ||
                        eType == PDFWriter::H1          ||
                        eType == PDFWriter::H2          ||
                        eType == PDFWriter::H3          ||
                        eType == PDFWriter::H4          ||
                        eType == PDFWriter::H5          ||
                        eType == PDFWriter::H6          ||
                        eType == PDFWriter::List        ||
                        eType == PDFWriter::ListItem    ||
                        eType == PDFWriter::LILabel     ||
                        eType == PDFWriter::LIBody      ||
                        eType == PDFWriter::Table       ||
                        eType == PDFWriter::TableRow    ||
                        eType == PDFWriter::TableHeader ||
                        eType == PDFWriter::TableData   ||
                        eType == PDFWriter::Span        ||
                        eType == PDFWriter::Quote       ||
                        eType == PDFWriter::Note        ||
                        eType == PDFWriter::Reference   ||
                        eType == PDFWriter::BibEntry    ||
                        eType == PDFWriter::Code        ||
                        eType == PDFWriter::Link )
                    {
                        bInsert = true;
                    }
                }
                break;
            case PDFWriter::TextDecorationType:
                if( eVal == PDFWriter::NONE         ||
                    eVal == PDFWriter::Underline    ||
                    eVal == PDFWriter::Overline     ||
                    eVal == PDFWriter::LineThrough )
                {
                    // only for ILSE and BLSE
                    if( eType == PDFWriter::Paragraph   ||
                        eType == PDFWriter::Heading     ||
                        eType == PDFWriter::H1          ||
                        eType == PDFWriter::H2          ||
                        eType == PDFWriter::H3          ||
                        eType == PDFWriter::H4          ||
                        eType == PDFWriter::H5          ||
                        eType == PDFWriter::H6          ||
                        eType == PDFWriter::List        ||
                        eType == PDFWriter::ListItem    ||
                        eType == PDFWriter::LILabel     ||
                        eType == PDFWriter::LIBody      ||
                        eType == PDFWriter::Table       ||
                        eType == PDFWriter::TableRow    ||
                        eType == PDFWriter::TableHeader ||
                        eType == PDFWriter::TableData   ||
                        eType == PDFWriter::Span        ||
                        eType == PDFWriter::Quote       ||
                        eType == PDFWriter::Note        ||
                        eType == PDFWriter::Reference   ||
                        eType == PDFWriter::BibEntry    ||
                        eType == PDFWriter::Code        ||
                        eType == PDFWriter::Link )
                    {
                        bInsert = true;
                    }
                }
                break;
            case PDFWriter::ListNumbering:
                if( eVal == PDFWriter::NONE         ||
                    eVal == PDFWriter::Disc         ||
                    eVal == PDFWriter::Circle       ||
                    eVal == PDFWriter::Square       ||
                    eVal == PDFWriter::Decimal      ||
                    eVal == PDFWriter::UpperRoman   ||
                    eVal == PDFWriter::LowerRoman   ||
                    eVal == PDFWriter::UpperAlpha   ||
                    eVal == PDFWriter::LowerAlpha )
                {
                    if( eType == PDFWriter::List )
                        bInsert = true;
                }
                break;
            default: break;
        }
    }

    if( bInsert )
        m_aStructure[ m_nCurrentStructElement ].m_aAttributes[ eAttr ] = PDFStructureAttribute( eVal );
#if OSL_DEBUG_LEVEL > 1
    else if( m_nCurrentStructElement > 0 && m_bEmitStructure )
        fprintf( stderr, "rejecting setStructureAttribute( %s, %s ) on %s element\n",
                 getAttributeTag( eAttr ),
                 getAttributeValueTag( eVal ),
                 getStructureTag( m_aStructure[ m_nCurrentStructElement ].m_eType ) );
#endif

    return bInsert;
}

bool PDFWriterImpl::setStructureAttributeNumerical( enum PDFWriter::StructAttribute eAttr, sal_Int32 nValue )
{
    if( ! m_aContext.Tagged )
        return false;

    bool bInsert = false;
    if( m_nCurrentStructElement > 0 && m_bEmitStructure )
    {
        PDFWriter::StructElement eType = m_aStructure[ m_nCurrentStructElement ].m_eType;
        switch( eAttr )
        {
            case PDFWriter::SpaceBefore:
            case PDFWriter::SpaceAfter:
            case PDFWriter::StartIndent:
            case PDFWriter::EndIndent:
                // just for BLSE
                if( eType == PDFWriter::Paragraph   ||
                    eType == PDFWriter::Heading     ||
                    eType == PDFWriter::H1          ||
                    eType == PDFWriter::H2          ||
                    eType == PDFWriter::H3          ||
                    eType == PDFWriter::H4          ||
                    eType == PDFWriter::H5          ||
                    eType == PDFWriter::H6          ||
                    eType == PDFWriter::List        ||
                    eType == PDFWriter::ListItem    ||
                    eType == PDFWriter::LILabel     ||
                    eType == PDFWriter::LIBody      ||
                    eType == PDFWriter::Table       ||
                    eType == PDFWriter::TableRow    ||
                    eType == PDFWriter::TableHeader ||
                    eType == PDFWriter::TableData )
                {
                    bInsert = true;
                }
                break;
            case PDFWriter::TextIndent:
                // paragraph like BLSE and additional elements
                if( eType == PDFWriter::Paragraph   ||
                    eType == PDFWriter::Heading     ||
                    eType == PDFWriter::H1          ||
                    eType == PDFWriter::H2          ||
                    eType == PDFWriter::H3          ||
                    eType == PDFWriter::H4          ||
                    eType == PDFWriter::H5          ||
                    eType == PDFWriter::H6          ||
                    eType == PDFWriter::LILabel     ||
                    eType == PDFWriter::LIBody      ||
                    eType == PDFWriter::TableHeader ||
                    eType == PDFWriter::TableData )
                {
                    bInsert = true;
                }
                break;
            case PDFWriter::Width:
            case PDFWriter::Height:
                if( eType == PDFWriter::Figure      ||
                    eType == PDFWriter::Formula     ||
                    eType == PDFWriter::Form        ||
                    eType == PDFWriter::Table       ||
                    eType == PDFWriter::TableHeader ||
                    eType == PDFWriter::TableData )
                {
                    bInsert = true;
                }
                break;
            case PDFWriter::LineHeight:
            case PDFWriter::BaselineShift:
                // only for ILSE and BLSE
                if( eType == PDFWriter::Paragraph   ||
                    eType == PDFWriter::Heading     ||
                    eType == PDFWriter::H1          ||
                    eType == PDFWriter::H2          ||
                    eType == PDFWriter::H3          ||
                    eType == PDFWriter::H4          ||
                    eType == PDFWriter::H5          ||
                    eType == PDFWriter::H6          ||
                    eType == PDFWriter::List        ||
                    eType == PDFWriter::ListItem    ||
                    eType == PDFWriter::LILabel     ||
                    eType == PDFWriter::LIBody      ||
                    eType == PDFWriter::Table       ||
                    eType == PDFWriter::TableRow    ||
                    eType == PDFWriter::TableHeader ||
                    eType == PDFWriter::TableData   ||
                    eType == PDFWriter::Span        ||
                    eType == PDFWriter::Quote       ||
                    eType == PDFWriter::Note        ||
                    eType == PDFWriter::Reference   ||
                    eType == PDFWriter::BibEntry    ||
                    eType == PDFWriter::Code        ||
                    eType == PDFWriter::Link )
                {
                        bInsert = true;
                }
                break;
            case PDFWriter::RowSpan:
            case PDFWriter::ColSpan:
                // only for table cells
                if( eType == PDFWriter::TableHeader ||
                    eType == PDFWriter::TableData )
                {
                    bInsert = true;
                }
                break;
            case PDFWriter::LinkAnnotation:
                if( eType == PDFWriter::Link )
                    bInsert = true;
                break;
            default: break;
        }
    }

    if( bInsert )
        m_aStructure[ m_nCurrentStructElement ].m_aAttributes[ eAttr ] = PDFStructureAttribute( nValue );
#if OSL_DEBUG_LEVEL > 1
    else if( m_nCurrentStructElement > 0 && m_bEmitStructure )
        fprintf( stderr, "rejecting setStructureAttributeNumerical( %s, %d ) on %s element\n",
                 getAttributeTag( eAttr ),
                 (int)nValue,
                 getStructureTag( m_aStructure[ m_nCurrentStructElement ].m_eType ) );
#endif

    return bInsert;
}

void PDFWriterImpl::setStructureBoundingBox( const Rectangle& rRect )
{
    sal_Int32 nPageNr = m_nCurrentPage;
    if( nPageNr < 0 || nPageNr >= (sal_Int32)m_aPages.size() || !m_aContext.Tagged )
        return;

    if( m_nCurrentStructElement > 0 && m_bEmitStructure )
    {
        PDFWriter::StructElement eType = m_aStructure[ m_nCurrentStructElement ].m_eType;
        if( eType == PDFWriter::Figure      ||
            eType == PDFWriter::Formula     ||
            eType == PDFWriter::Form        ||
            eType == PDFWriter::Table )
        {
            m_aStructure[ m_nCurrentStructElement ].m_aBBox = rRect;
            // convert to default user space now, since the mapmode may change
            m_aPages[nPageNr].convertRect( m_aStructure[ m_nCurrentStructElement ].m_aBBox );
        }
    }
}

void PDFWriterImpl::setActualText( const String& rText )
{
    if( m_aContext.Tagged && m_nCurrentStructElement > 0 && m_bEmitStructure )
    {
        m_aStructure[ m_nCurrentStructElement ].m_aActualText = rText;
    }
}

void PDFWriterImpl::setAlternateText( const String& rText )
{
    if( m_aContext.Tagged && m_nCurrentStructElement > 0 && m_bEmitStructure )
    {
        m_aStructure[ m_nCurrentStructElement ].m_aAltText = rText;
    }
}

void PDFWriterImpl::setAutoAdvanceTime( sal_uInt32 nSeconds, sal_Int32 nPageNr )
{
    if( nPageNr < 0 )
        nPageNr = m_nCurrentPage;

    if( nPageNr < 0 || nPageNr >= (sal_Int32)m_aPages.size() )
        return;

    m_aPages[ nPageNr ].m_nDuration = nSeconds;
}

void PDFWriterImpl::setPageTransition( PDFWriter::PageTransition eType, sal_uInt32 nMilliSec, sal_Int32 nPageNr )
{
    if( nPageNr < 0 )
        nPageNr = m_nCurrentPage;

    if( nPageNr < 0 || nPageNr >= (sal_Int32)m_aPages.size() )
        return;

    m_aPages[ nPageNr ].m_eTransition   = eType;
    m_aPages[ nPageNr ].m_nTransTime    = nMilliSec;
}

void PDFWriterImpl::ensureUniqueRadioOnValues()
{
    // loop over radio groups
    for( std::map<sal_Int32,sal_Int32>::const_iterator group = m_aRadioGroupWidgets.begin();
         group != m_aRadioGroupWidgets.end(); ++group )
    {
        PDFWidget& rGroupWidget = m_aWidgets[ group->second ];
        // check whether all kids have a unique OnValue
        std::hash_map< OUString, sal_Int32, OUStringHash > aOnValues;
        int nChildren = rGroupWidget.m_aKidsIndex.size();
        bool bIsUnique = true;
        for( int nKid = 0; nKid < nChildren && bIsUnique; nKid++ )
        {
            int nKidIndex = rGroupWidget.m_aKidsIndex[nKid];
            const OUString& rVal = m_aWidgets[nKidIndex].m_aOnValue;
            #if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "OnValue: %s\n", OUStringToOString( rVal, RTL_TEXTENCODING_UTF8 ).getStr() );
            #endif
            if( aOnValues.find( rVal ) == aOnValues.end() )
            {
                aOnValues[ rVal ] = 1;
            }
            else
            {
                bIsUnique = false;
            }
        }
        if( ! bIsUnique )
        {
            #if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "enforcing unique OnValues\n" );
            #endif
            // make unique by using ascending OnValues
            for( int nKid = 0; nKid < nChildren; nKid++ )
            {
                int nKidIndex = rGroupWidget.m_aKidsIndex[nKid];
                PDFWidget& rKid = m_aWidgets[nKidIndex];
                rKid.m_aOnValue = OUString::valueOf( sal_Int32(nKid+1) );
                if( ! rKid.m_aValue.equalsAscii( "Off" ) )
                    rKid.m_aValue = rKid.m_aOnValue;
            }
        }
        // finally move the "Yes" appearance to the OnValue appearance
        for( int nKid = 0; nKid < nChildren; nKid++ )
        {
            int nKidIndex = rGroupWidget.m_aKidsIndex[nKid];
            PDFWidget& rKid = m_aWidgets[nKidIndex];
            PDFAppearanceMap::iterator app_it = rKid.m_aAppearances.find( "N" );
            if( app_it != rKid.m_aAppearances.end() )
            {
                PDFAppearanceStreams::iterator stream_it = app_it->second.find( "Yes" );
                if( stream_it != app_it->second.end() )
                {
                    SvMemoryStream* pStream = stream_it->second;
                    app_it->second.erase( stream_it );
                    OStringBuffer aBuf( rKid.m_aOnValue.getLength()*2 );
                    appendName( rKid.m_aOnValue, aBuf );
                    (app_it->second)[ aBuf.makeStringAndClear() ] = pStream;
                }
                #if OSL_DEBUG_LEVEL > 1
                else
                    fprintf( stderr, "error: RadioButton without \"Yes\" stream\n" );
                #endif
            }
            // update selected radio button
            if( ! rKid.m_aValue.equalsAscii( "Off" ) )
            {
                rGroupWidget.m_aValue = rKid.m_aValue;
            }
        }
    }
}

sal_Int32 PDFWriterImpl::findRadioGroupWidget( const PDFWriter::RadioButtonWidget& rBtn )
{
    sal_Int32 nRadioGroupWidget = -1;

    std::map< sal_Int32, sal_Int32 >::const_iterator it = m_aRadioGroupWidgets.find( rBtn.RadioGroup );

    if( it == m_aRadioGroupWidgets.end() )
    {
        m_aRadioGroupWidgets[ rBtn.RadioGroup ] = nRadioGroupWidget =
            sal_Int32(m_aWidgets.size());

        // new group, insert the radiobutton
        m_aWidgets.push_back( PDFWidget() );
        m_aWidgets.back().m_nObject     = createObject();
        m_aWidgets.back().m_nPage       = m_nCurrentPage;
        m_aWidgets.back().m_eType       = PDFWriter::RadioButton;
        m_aWidgets.back().m_nRadioGroup = rBtn.RadioGroup;
        m_aWidgets.back().m_nFlags |= 0x00008000;

        // create radio button field name
        const rtl::OUString& rName = (m_aContext.Version > PDFWriter::PDF_1_2) ?
                                     rBtn.Name : rBtn.Text;
        if( rName.getLength() )
        {
            m_aWidgets.back().m_aName   = convertWidgetFieldName( rName );
        }
        else
        {
            m_aWidgets.back().m_aName   = "RadioGroup";
            m_aWidgets.back().m_aName  += OString::valueOf( rBtn.RadioGroup );
        }
    }
    else
        nRadioGroupWidget = it->second;

    return nRadioGroupWidget;
}

sal_Int32 PDFWriterImpl::createControl( const PDFWriter::AnyWidget& rControl, sal_Int32 nPageNr )
{
    if( nPageNr < 0 )
        nPageNr = m_nCurrentPage;

    if( nPageNr < 0 || nPageNr >= (sal_Int32)m_aPages.size() )
        return -1;

    m_aWidgets.push_back( PDFWidget() );
    sal_Int32 nNewWidget = m_aWidgets.size()-1;

    // create eventual radio button before getting any references
    // from m_aWidgets as the push_back operation potentially assigns new
    // memory to the vector and thereby invalidates the reference
    int nRadioGroupWidget = -1;
    if( rControl.getType() == PDFWriter::RadioButton )
        nRadioGroupWidget = findRadioGroupWidget( static_cast<const PDFWriter::RadioButtonWidget&>(rControl) );

    PDFWidget& rNewWidget           = m_aWidgets[nNewWidget];
    rNewWidget.m_nObject            = createObject();
    rNewWidget.m_aRect              = rControl.Location;
    rNewWidget.m_nPage              = nPageNr;
    rNewWidget.m_eType              = rControl.getType();

    // for unknown reasons the radio buttons of a radio group must not have a
    // field name, else the buttons are in fact check boxes -
    // that is multiple buttons of the radio group can be selected
    if( rControl.getType() != PDFWriter::RadioButton )
    {
        // acrobat reader since 3.0 does not support unicode text
        // strings for the field name; so we need to encode unicodes
        // larger than 255
        rNewWidget.m_aName          =
            convertWidgetFieldName( (m_aContext.Version > PDFWriter::PDF_1_2) ?
                                    rControl.Name : rControl.Text );
    }
    rNewWidget.m_aDescription       = rControl.Description;
    rNewWidget.m_aText              = rControl.Text;
    rNewWidget.m_nTextStyle         = rControl.TextStyle &
        (  TEXT_DRAW_LEFT | TEXT_DRAW_CENTER | TEXT_DRAW_RIGHT | TEXT_DRAW_TOP |
           TEXT_DRAW_VCENTER | TEXT_DRAW_BOTTOM |
           TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK  );
    rNewWidget.m_nTabOrder          = rControl.TabOrder;

    // various properties are set via the flags (/Ff) property of the field dict
    if( rControl.ReadOnly )
        rNewWidget.m_nFlags |= 1;
    if( rControl.getType() == PDFWriter::PushButton )
    {
        const PDFWriter::PushButtonWidget& rBtn = static_cast<const PDFWriter::PushButtonWidget&>(rControl);
        if( rNewWidget.m_nTextStyle == 0 )
            rNewWidget.m_nTextStyle =
                TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER |
                TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK;

        rNewWidget.m_nFlags |= 0x00010000;
        if( rBtn.URL.getLength() )
            rNewWidget.m_aListEntries.push_back( rBtn.URL );
        rNewWidget.m_bSubmit = rBtn.Submit;
        rNewWidget.m_nDest   = rBtn.Dest;
        createDefaultPushButtonAppearance( rNewWidget, rBtn );
    }
    else if( rControl.getType() == PDFWriter::RadioButton )
    {
        const PDFWriter::RadioButtonWidget& rBtn = static_cast<const PDFWriter::RadioButtonWidget&>(rControl);
        if( rNewWidget.m_nTextStyle == 0 )
            rNewWidget.m_nTextStyle =
                TEXT_DRAW_VCENTER | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK;
        /*  PDF sees a RadioButton group as one radio button with
         *  children which are in turn check boxes
         *
         *  so we need to create a radio button on demand for a new group
         *  and insert a checkbox for each RadioButtonWidget as its child
         */
        rNewWidget.m_eType          = PDFWriter::CheckBox;
        rNewWidget.m_nRadioGroup    = rBtn.RadioGroup;

        DBG_ASSERT( nRadioGroupWidget >= 0 && nRadioGroupWidget < (sal_Int32)m_aWidgets.size(), "no radio group parent" );

        PDFWidget& rRadioButton = m_aWidgets[nRadioGroupWidget];
        rRadioButton.m_aKids.push_back( rNewWidget.m_nObject );
        rRadioButton.m_aKidsIndex.push_back( nNewWidget );
        rNewWidget.m_nParent = rRadioButton.m_nObject;

        rNewWidget.m_aValue     = OUString( RTL_CONSTASCII_USTRINGPARAM( "Off" ) );
        rNewWidget.m_aOnValue   = rBtn.OnValue;
        if( ! rRadioButton.m_aValue.getLength() && rBtn.Selected )
        {
            rNewWidget.m_aValue     = rNewWidget.m_aOnValue;
            rRadioButton.m_aValue   = rNewWidget.m_aOnValue;
        }
        createDefaultRadioButtonAppearance( rNewWidget, rBtn );

        // union rect of radio group
        Rectangle aRect = rNewWidget.m_aRect;
        m_aPages[ nPageNr ].convertRect( aRect );
        rRadioButton.m_aRect.Union( aRect );
    }
    else if( rControl.getType() == PDFWriter::CheckBox )
    {
        const PDFWriter::CheckBoxWidget& rBox = static_cast<const PDFWriter::CheckBoxWidget&>(rControl);
        if( rNewWidget.m_nTextStyle == 0 )
            rNewWidget.m_nTextStyle =
                TEXT_DRAW_VCENTER | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK;

        rNewWidget.m_aValue = OUString::createFromAscii( rBox.Checked ? "Yes" : "Off" );
        // create default appearance before m_aRect gets transformed
        createDefaultCheckBoxAppearance( rNewWidget, rBox );
    }
    else if( rControl.getType() == PDFWriter::ListBox )
    {
        if( rNewWidget.m_nTextStyle == 0 )
            rNewWidget.m_nTextStyle = TEXT_DRAW_VCENTER;

        const PDFWriter::ListBoxWidget& rLstBox = static_cast<const PDFWriter::ListBoxWidget&>(rControl);
        rNewWidget.m_aListEntries   = rLstBox.Entries;
        rNewWidget.m_aValue         = rLstBox.Text;
        if( rLstBox.DropDown )
            rNewWidget.m_nFlags |= 0x00020000;
        if( rLstBox.Sort )
        {
            rNewWidget.m_nFlags |= 0x00080000;
            rNewWidget.m_aListEntries.sort();
        }
        if( rLstBox.MultiSelect && !rLstBox.DropDown )
            rNewWidget.m_nFlags |= 0x00200000;

        createDefaultListBoxAppearance( rNewWidget, rLstBox );
    }
    else if( rControl.getType() == PDFWriter::ComboBox )
    {
        if( rNewWidget.m_nTextStyle == 0 )
            rNewWidget.m_nTextStyle = TEXT_DRAW_VCENTER;

        const PDFWriter::ComboBoxWidget& rBox = static_cast<const PDFWriter::ComboBoxWidget&>(rControl);
        rNewWidget.m_aValue         = rBox.Text;
        rNewWidget.m_aListEntries   = rBox.Entries;
        rNewWidget.m_nFlags |= 0x00060000; // combo and edit flag
        if( rBox.Sort )
        {
            rNewWidget.m_nFlags |= 0x00080000;
            rNewWidget.m_aListEntries.sort();
        }

        PDFWriter::ListBoxWidget aLBox;
        aLBox.Name              = rBox.Name;
        aLBox.Description       = rBox.Description;
        aLBox.Text              = rBox.Text;
        aLBox.TextStyle         = rBox.TextStyle;
        aLBox.ReadOnly          = rBox.ReadOnly;
        aLBox.Border            = rBox.Border;
        aLBox.BorderColor       = rBox.BorderColor;
        aLBox.Background        = rBox.Background;
        aLBox.BackgroundColor   = rBox.BackgroundColor;
        aLBox.TextFont          = rBox.TextFont;
        aLBox.TextColor         = rBox.TextColor;
        aLBox.DropDown          = true;
        aLBox.Sort              = rBox.Sort;
        aLBox.MultiSelect       = false;
        aLBox.Entries           = rBox.Entries;

        createDefaultListBoxAppearance( rNewWidget, aLBox );
    }
    else if( rControl.getType() == PDFWriter::Edit )
    {
        if( rNewWidget.m_nTextStyle == 0 )
            rNewWidget.m_nTextStyle = TEXT_DRAW_LEFT | TEXT_DRAW_VCENTER;

        const PDFWriter::EditWidget& rEdit = static_cast<const  PDFWriter::EditWidget&>(rControl);
        if( rEdit.MultiLine )
        {
            rNewWidget.m_nFlags |= 0x00001000;
            rNewWidget.m_nTextStyle |= TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK;
        }
        if( rEdit.Password )
            rNewWidget.m_nFlags |= 0x00002000;
        if( rEdit.FileSelect && m_aContext.Version > PDFWriter::PDF_1_3 )
            rNewWidget.m_nFlags |= 0x00100000;
        rNewWidget.m_nMaxLen = rEdit.MaxLen;
        rNewWidget.m_aValue = rEdit.Text;

        createDefaultEditAppearance( rNewWidget, rEdit );
    }

    // convert to default user space now, since the mapmode may change
    // note: create default appearances before m_aRect gets transformed
    m_aPages[ nPageNr ].convertRect( rNewWidget.m_aRect );

    // insert widget to page's annotation list
    m_aPages[ nPageNr ].m_aAnnotations.push_back( rNewWidget.m_nObject );

    // mark page as having widgets
    m_aPages[ nPageNr ].m_bHasWidgets = true;

    return nNewWidget;
}

void PDFWriterImpl::beginControlAppearance( sal_Int32 nControl )
{
    if( nControl < 0 || nControl >= (sal_Int32)m_aWidgets.size() )
        return;

    PDFWidget& rWidget = m_aWidgets[ nControl ];
    m_nCurrentControl = nControl;

    SvMemoryStream* pControlStream = new SvMemoryStream( 1024, 1024 );
    // back conversion of control rect to current MapMode; necessary because
    // MapMode between createControl and beginControlAppearance
    // could have changed; therefore the widget rectangle is
    // already converted
    Rectangle aBack( Point( rWidget.m_aRect.Left(), pointToPixel(m_aPages[m_nCurrentPage].getHeight()) - rWidget.m_aRect.Top() - rWidget.m_aRect.GetHeight() ),
                     rWidget.m_aRect.GetSize() );
    aBack = lcl_convert( m_aMapMode,
                         m_aGraphicsStack.front().m_aMapMode,
                         getReferenceDevice(),
                         aBack );
    beginRedirect( pControlStream, aBack );
    writeBuffer( "/Tx BMC\n", 8 );
}

bool PDFWriterImpl::endControlAppearance( PDFWriter::WidgetState eState )
{
    bool bRet = false;
    if( ! m_aOutputStreams.empty() )
        writeBuffer( "\nEMC\n", 5 );
    SvMemoryStream* pAppearance = static_cast<SvMemoryStream*>(endRedirect());
    if( pAppearance && m_nCurrentControl >= 0 && m_nCurrentControl < (sal_Int32)m_aWidgets.size() )
    {
        PDFWidget& rWidget = m_aWidgets[ m_nCurrentControl ];
        OString aState, aStyle;
        switch( rWidget.m_eType )
        {
            case PDFWriter::PushButton:
                if( eState == PDFWriter::Up || eState == PDFWriter::Down )
                {
                    aState = (eState == PDFWriter::Up) ? "N" : "D";
                    aStyle = "Standard";
                }
                break;
            case PDFWriter::CheckBox:
                if( eState == PDFWriter::Up || eState == PDFWriter::Down )
                {
                    aState = "N";
                    aStyle = (eState == PDFWriter::Up) ? "Off" : "Yes";
                    /* cf PDFReference 3rd ed. V1.4 p539:
                       recommended name for on state is "Yes",
                       recommended name for off state is "Off"
                     */
                }
                break;
            case PDFWriter::RadioButton:
                if( eState == PDFWriter::Up || eState == PDFWriter::Down )
                {
                    aState = "N";
                    if( eState == PDFWriter::Up )
                        aStyle = "Off";
                    else
                    {
                        OStringBuffer aBuf( rWidget.m_aOnValue.getLength()*2 );
                        appendName( rWidget.m_aOnValue, aBuf );
                        aStyle = aBuf.makeStringAndClear();
                    }
                }
                break;
            case PDFWriter::Edit:
                aState = "N";
                aStyle = "Standard";
                break;
            case PDFWriter::ListBox:
            case PDFWriter::ComboBox:
                break;
        }
        if( aState.getLength() && aStyle.getLength() )
        {
            // delete eventual existing stream
            PDFAppearanceStreams::iterator it =
                rWidget.m_aAppearances[ aState ].find( aStyle );
            if( it != rWidget.m_aAppearances[ aState ].end() )
                delete it->second;
            rWidget.m_aAppearances[ aState ][ aStyle ] = pAppearance;
            bRet = true;
        }
    }

    if( ! bRet )
        delete pAppearance;

    m_nCurrentControl = -1;

    return bRet;
}
