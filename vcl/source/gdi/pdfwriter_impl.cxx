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

#include <config_features.h>

#include <sal/types.h>

#include <math.h>
#include <algorithm>

#if defined __GNUC__ && __cplusplus > 201402L
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wregister"
#endif
#include <lcms2.h>
#if defined __GNUC__ && __cplusplus > 201402L
#pragma GCC diagnostic pop
#endif

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <memory>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <cppuhelper/implbase.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <o3tl/numeric.hxx>
#include <o3tl/make_unique.hxx>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <rtl/crc.h>
#include <rtl/digest.h>
#include <rtl/ustrbuf.hxx>
#include <svl/urihelper.hxx>
#include <tools/debug.hxx>
#include <tools/fract.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <tools/zcodec.hxx>
#include <svl/cryptosign.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/image.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/metric.hxx>
#include <vcl/settings.hxx>
#include <vcl/strhelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/filter/pdfdocument.hxx>
#include <comphelper/hash.hxx>

#include <fontsubset.hxx>
#include <outdev.h>
#include <PhysicalFontFace.hxx>
#include <salgdi.hxx>
#include <sallayout.hxx>
#include <textlayout.hxx>
#include <textlineinfo.hxx>
#include <bitmapwriteaccess.hxx>

#include "pdfwriter_impl.hxx"

#ifdef _WIN32
// WinCrypt headers for PDF signing
// Note: this uses Windows 7 APIs and requires the relevant data types
#include <prewin.h>
#include <wincrypt.h>
#include <postwin.h>
#endif

#include <config_eot.h>

#if ENABLE_EOT
#include <libeot/libeot.h>
#endif

using namespace vcl;
using namespace::com::sun::star;

static bool g_bDebugDisableCompression = getenv("VCL_DEBUG_DISABLE_PDFCOMPRESSION");

#if HAVE_FEATURE_NSS
// Is this length truly the maximum possible, or just a number that
// seemed large enough when the author tested this (with some type of
// certificates)? I suspect the latter.

// Used to be 0x4000 = 16384, but a sample signed PDF (produced by
// some other software) provided by the customer has a signature
// content that is 30000 bytes. The SampleSignedPDFDocument.pdf from
// Adobe has one that is 21942 bytes. So let's be careful. Pity this
// can't be dynamic, at least not without restructuring the code. Also
// note that the checks in the code for this being too small
// apparently are broken, if this overflows you end up with an invalid
// PDF. Need to fix that.

#define MAX_SIGNATURE_CONTENT_LENGTH 50000
#endif

#ifdef DO_TEST_PDF
class PDFTestOutputStream : public PDFOutputStream
{
    public:
    virtual ~PDFTestOutputStream();
    virtual void write( const css::uno::Reference< css::io::XOutputStream >& xStream );
};

PDFTestOutputStream::~PDFTestOutputStream()
{
}

void PDFTestOutputStream::write( const css::uno::Reference< css::io::XOutputStream >& xStream )
{
    OString aStr( "lalala\ntest\ntest\ntest" );
    css::uno::Sequence< sal_Int8 > aData( aStr.getLength() );
    memcpy( aData.getArray(), aStr.getStr(), aStr.getLength() );
    xStream->writeBytes( aData );
}

// this test code cannot be used to test PDF/A-1 because it forces
// control item (widgets) to bypass the structure controlling
// the embedding of such elements in actual run
void doTestCode()
{
    static const char* pHome = getenv( "HOME"  );
    OUString aTestFile( "file://"  );
    aTestFile += OUString( pHome, strlen( pHome ), RTL_TEXTENCODING_MS_1252 );
    aTestFile += "/pdf_export_test.pdf";

    PDFWriter::PDFWriterContext aContext;
    aContext.URL            = aTestFile;
    aContext.Version        = PDFWriter::PDF_1_4;
    aContext.Tagged         = true;
    aContext.InitialPage    = 2;
    aContext.DocumentInfo.Title = "PDF export test document";
    aContext.DocumentInfo.Producer = "VCL";

    aContext.SignPDF        = true;
    aContext.SignLocation   = "Burdur";
    aContext.SignReason     = "Some valid reason to sign";
    aContext.SignContact    = "signer@example.com";

    css::uno::Reference< css::beans::XMaterialHolder > xEnc;
    PDFWriter aWriter( aContext, xEnc );
    aWriter.NewPage( 595, 842 );
    aWriter.BeginStructureElement( PDFWriter::Document );
    // set duration of 3 sec for first page
    aWriter.SetAutoAdvanceTime( 3 );
    aWriter.SetMapMode( MapMode( MapUnit::Map100thMM ) );

    aWriter.SetFillColor( COL_LIGHTRED );
    aWriter.SetLineColor( COL_LIGHTGREEN );
    aWriter.DrawRect( Rectangle( Point( 2000, 200 ), Size( 8000, 3000 ) ), 5000, 2000 );

    aWriter.SetFont( Font( OUString( "Times" ), Size( 0, 500 ) ) );
    aWriter.SetTextColor( COL_BLACK );
    aWriter.SetLineColor( COL_BLACK );
    aWriter.SetFillColor( COL_LIGHTBLUE );

    Rectangle aRect( Point( 5000, 5000 ), Size( 6000, 3000 ) );
    aWriter.DrawRect( aRect );
    aWriter.DrawText( aRect, OUString( "Link annot 1" ) );
    sal_Int32 nFirstLink = aWriter.CreateLink( aRect );
    PDFNote aNote;
    aNote.Title = "A small test note";
    aNote.Contents = "There is no business like show business like no business i know. Everything about it is appealing.";
    aWriter.CreateNote( Rectangle( Point( aRect.Right(), aRect.Top() ), Size( 6000, 3000 ) ), aNote );

    Rectangle aTargetRect( Point( 3000, 23000 ), Size( 12000, 6000 ) );
    aWriter.SetFillColor( COL_LIGHTGREEN );
    aWriter.DrawRect( aTargetRect );
    aWriter.DrawText( aTargetRect, "Dest second link" );
    sal_Int32 nSecondDest = aWriter.CreateDest( aTargetRect );

    aWriter.BeginStructureElement( PDFWriter::Section );
    aWriter.BeginStructureElement( PDFWriter::Heading );
    aWriter.DrawText( Point(4500, 9000), "A small structure test" );
    aWriter.EndStructureElement();
    aWriter.BeginStructureElement( PDFWriter::Paragraph );
    aWriter.SetStructureAttribute( PDFWriter::WritingMode, PDFWriter::LrTb );
    aWriter.SetStructureAttribute( PDFWriter::TextDecorationType, PDFWriter::Underline );
    aWriter.DrawText( Rectangle( Point( 4500, 10000 ), Size( 12000, 6000 ) ),
                     "It was the best of PDF, it was the worst of PDF ... or so. This is a pretty nonsensical text to denote a paragraph. I suggest you stop reading it. Because if you read on you might get bored. So continue on your on risk. Hey, you're still here ? Why do you continue to read this as it is of no use at all ? OK, it's your time, but still... . Woah, i even get bored writing this, so let's end this here and now.",
                      DrawTextFlags::MultiLine | DrawTextFlags::WordBreak
                      );
    aWriter.SetActualText( "It was the best of PDF, it was the worst of PDF ... or so. This is a pretty nonsensical text to denote a paragraph. I suggest you stop reading it. Because if you read on you might get bored. So continue on your on risk. Hey, you're still here ? Why do you continue to read this as it is of no use at all ? OK, it's your time, but still... . Woah, i even get bored writing this, so let's end this here and now." );
    aWriter.SetAlternateText( "This paragraph contains some lengthy nonsense to test structural element emission of PDFWriter." );
    aWriter.EndStructureElement();
    aWriter.BeginStructureElement( PDFWriter::Paragraph );
    aWriter.SetStructureAttribute( PDFWriter::WritingMode, PDFWriter::LrTb );
    aWriter.DrawText( Rectangle( Point( 4500, 19000 ), Size( 12000, 1000 ) ),
                      "This paragraph is nothing special either but ends on the next page structurewise",
                      DrawTextFlags::MultiLine | DrawTextFlags::WordBreak
                      );

    aWriter.NewPage( 595, 842 );
    // test AddStream interface
    aWriter.AddStream( "text/plain", new PDFTestOutputStream(), true );
    // set transitional mode
    aWriter.SetPageTransition( PDFWriter::WipeRightToLeft, 1500 );
    aWriter.SetMapMode( MapMode( MapUnit::Map100thMM ) );
    aWriter.SetTextColor( COL_BLACK );
    aWriter.SetFont( Font( OUString( "Times" ), Size( 0, 500 ) ) );
    aWriter.DrawText( Rectangle( Point( 4500, 1500 ), Size( 12000, 3000 ) ),
                      "Here's where all things come to an end ... well at least the paragraph from the last page.",
                      DrawTextFlags::MultiLine | DrawTextFlags::WordBreak
                      );
    aWriter.EndStructureElement();

    aWriter.SetFillColor( COL_LIGHTBLUE );
    // disable structure
    aWriter.BeginStructureElement( PDFWriter::NonStructElement );
    aWriter.DrawRect( aRect );
    aWriter.BeginStructureElement( PDFWriter::Paragraph );
    aWriter.DrawText( aRect, "Link annot 2" );
    sal_Int32 nSecondLink = aWriter.CreateLink( aRect );

    aWriter.SetFillColor( COL_LIGHTGREEN );
    aWriter.BeginStructureElement( PDFWriter::ListItem );
    aWriter.DrawRect( aTargetRect );
    aWriter.DrawText( aTargetRect, "Dest first link" );
    sal_Int32 nFirstDest = aWriter.CreateDest( aTargetRect );
    // enable structure
    aWriter.EndStructureElement();

    aWriter.EndStructureElement();
    aWriter.EndStructureElement();
    aWriter.BeginStructureElement( PDFWriter::Figure );
    aWriter.BeginStructureElement( PDFWriter::Caption );
    aWriter.DrawText( Point( 4500, 9000 ), "Some drawing stuff inside the structure" );
    aWriter.EndStructureElement();

    // test clipping
    basegfx::B2DPolyPolygon aClip;
    basegfx::B2DPolygon aClipPoly;
    aClipPoly.append( basegfx::B2DPoint( 8250, 9600 ) );
    aClipPoly.append( basegfx::B2DPoint( 16500, 11100 ) );
    aClipPoly.append( basegfx::B2DPoint( 8250, 12600 ) );
    aClipPoly.append( basegfx::B2DPoint( 4500, 11100 ) );
    aClipPoly.setClosed( true );
    aClip.append( aClipPoly );

    aWriter.Push( PushFlags::CLIPREGION | PushFlags::FILLCOLOR );
    aWriter.SetClipRegion( aClip );
    aWriter.DrawEllipse( Rectangle( Point( 4500, 9600 ), Size( 12000, 3000 ) ) );
    aWriter.MoveClipRegion( 1000, 500 );
    aWriter.SetFillColor( COL_RED );
    aWriter.DrawEllipse( Rectangle( Point( 4500, 9600 ), Size( 12000, 3000 ) ) );
    aWriter.Pop();
    // test transparency
    // draw background
    Rectangle aTranspRect( Point( 7500, 13500 ), Size( 9000, 6000 ) );
    aWriter.SetFillColor( COL_LIGHTRED );
    aWriter.DrawRect( aTranspRect );
    aWriter.BeginTransparencyGroup();

    aWriter.SetFillColor( COL_LIGHTGREEN );
    aWriter.DrawEllipse( aTranspRect );
    aWriter.SetTextColor( COL_LIGHTBLUE );
    aWriter.DrawText( aTranspRect,
                      "Some transparent text",
                      DrawTextFlags::Center | DrawTextFlags::VCenter | DrawTextFlags::MultiLine | DrawTextFlags::WordBreak );

    aWriter.EndTransparencyGroup( aTranspRect, 50 );

    // prepare an alpha mask
    Bitmap aTransMask( Size( 256, 256 ), 8, &Bitmap::GetGreyPalette( 256 ) );
    BitmapScopedWriteAccess pAcc(aTransMask);
    for( int nX = 0; nX < 256; nX++ )
        for( int nY = 0; nY < 256; nY++ )
            pAcc->SetPixel( nX, nY, BitmapColor( (sal_uInt8)((nX+nY)/2) ) );
    pAcc.reset();
    aTransMask.SetPrefMapMode( MapUnit::MapMM );
    aTransMask.SetPrefSize( Size( 10, 10 ) );

    aWriter.DrawBitmap( Point( 600, 13500 ), Size( 3000, 3000 ), aTransMask );

    aTranspRect = Rectangle( Point( 4200, 13500 ), Size( 3000, 3000 ) );
    aWriter.SetFillColor( COL_LIGHTRED );
    aWriter.DrawRect( aTranspRect );
    aWriter.SetFillColor( COL_LIGHTGREEN );
    aWriter.DrawEllipse( aTranspRect );
    aWriter.SetTextColor( COL_LIGHTBLUE );
    aWriter.DrawText( aTranspRect,
                      "Some transparent text",
                      DrawTextFlags::Center | DrawTextFlags::VCenter | DrawTextFlags::MultiLine | DrawTextFlags::WordBreak );
    aTranspRect = Rectangle( Point( 1500, 16500 ), Size( 4800, 3000 ) );
    aWriter.SetFillColor( COL_LIGHTRED );
    aWriter.DrawRect( aTranspRect );

    Bitmap aImageBmp( Size( 256, 256 ), 24 );
    pAcc = BitmapScopedWriteAccess(aImageBmp);
    pAcc->SetFillColor( Color( 0xff, 0, 0xff ) );
    pAcc->FillRect( Rectangle( Point( 0, 0 ), Size( 256, 256 ) ) );
    pAcc.reset();
    BitmapEx aBmpEx( aImageBmp, AlphaMask( aTransMask ) );
    aWriter.DrawBitmapEx( Point( 1500, 19500 ), Size( 4800, 3000 ), aBmpEx );

    aWriter.EndStructureElement();
    aWriter.EndStructureElement();

    LineInfo aLI( LineStyle::Dash, 3 );
    aLI.SetDashCount( 2 );
    aLI.SetDashLen( 50 );
    aLI.SetDotCount( 2 );
    aLI.SetDotLen( 25 );
    aLI.SetDistance( 15 );
    Point aLIPoints[] = { Point( 4000, 10000 ),
                          Point( 8000, 12000 ),
                          Point( 3000, 19000 ) };
    tools::Polygon aLIPoly( 3, aLIPoints );
    aWriter.SetLineColor( COL_BLUE );
    aWriter.SetFillColor();
    aWriter.DrawPolyLine( aLIPoly, aLI );

    aLI.SetDashCount( 4 );
    aLIPoly.Move( 1000, 1000 );
    aWriter.DrawPolyLine( aLIPoly, aLI );

    aWriter.NewPage( 595, 842 );
    aWriter.SetMapMode( MapMode( MapUnit::Map100thMM ) );
    Wallpaper aWall( aTransMask );
    aWall.SetStyle( WallpaperStyle::Tile );
    aWriter.DrawWallpaper( Rectangle( Point( 4400, 4200 ), Size( 10200, 6300 ) ), aWall );

    aWriter.NewPage( 595, 842 );
    aWriter.SetMapMode( MapMode( MapUnit::Map100thMM ) );
    aWriter.SetFont( Font( OUString( "Times" ), Size( 0, 500 ) ) );
    aWriter.SetTextColor( COL_BLACK );
    aRect = Rectangle( Point( 4500, 6000 ), Size( 6000, 1500 ) );
    aWriter.DrawRect( aRect );
    aWriter.DrawText( aRect, "www.heise.de" );
    sal_Int32 nURILink = aWriter.CreateLink( aRect );
    aWriter.SetLinkURL( nURILink, OUString( "http://www.heise.de" ) );

    aWriter.SetLinkDest( nFirstLink, nFirstDest );
    aWriter.SetLinkDest( nSecondLink, nSecondDest );

    // include a button
    PDFWriter::PushButtonWidget aBtn;
    aBtn.Name = "testButton";
    aBtn.Description = "A test button";
    aBtn.Text = "hit me";
    aBtn.Location = Rectangle( Point( 4500, 9000 ), Size( 4500, 3000 ) );
    aBtn.Border = aBtn.Background = true;
    aWriter.CreateControl( aBtn );

    // include a uri button
    PDFWriter::PushButtonWidget aUriBtn;
    aUriBtn.Name = "wwwButton";
    aUriBtn.Description = "A URI button";
    aUriBtn.Text = "to www";
    aUriBtn.Location = Rectangle( Point( 9500, 9000 ), Size( 4500, 3000 ) );
    aUriBtn.Border = aUriBtn.Background = true;
    aUriBtn.URL = "http://www.heise.de";
    aWriter.CreateControl( aUriBtn );

    // include a dest button
    PDFWriter::PushButtonWidget aDstBtn;
    aDstBtn.Name = "destButton";
    aDstBtn.Description = "A Dest button";
    aDstBtn.Text = "to paragraph";
    aDstBtn.Location = Rectangle( Point( 14500, 9000 ), Size( 4500, 3000 ) );
    aDstBtn.Border = aDstBtn.Background = true;
    aDstBtn.Dest = nFirstDest;
    aWriter.CreateControl( aDstBtn );

    PDFWriter::CheckBoxWidget aCBox;
    aCBox.Name = "textCheckBox";
    aCBox.Description = "A test check box";
    aCBox.Text = "check me";
    aCBox.Location = Rectangle( Point( 4500, 13500 ), Size( 3000, 750 ) );
    aCBox.Checked = true;
    aCBox.Border = aCBox.Background = false;
    aWriter.CreateControl( aCBox );

    PDFWriter::CheckBoxWidget aCBox2;
    aCBox2.Name = "textCheckBox2";
    aCBox2.Description = "Another test check box";
    aCBox2.Text = "check me right";
    aCBox2.Location = Rectangle( Point( 4500, 14250 ), Size( 3000, 750 ) );
    aCBox2.Checked = true;
    aCBox2.Border = aCBox2.Background = false;
    aCBox2.ButtonIsLeft = false;
    aWriter.CreateControl( aCBox2 );

    PDFWriter::RadioButtonWidget aRB1;
    aRB1.Name = "rb1_1";
    aRB1.Description = "radio 1 button 1";
    aRB1.Text = "Despair";
    aRB1.Location = Rectangle( Point( 4500, 15000 ), Size( 6000, 1000 ) );
    aRB1.Selected = true;
    aRB1.RadioGroup = 1;
    aRB1.Border = aRB1.Background = true;
    aRB1.ButtonIsLeft = false;
    aRB1.BorderColor = COL_LIGHTGREEN;
    aRB1.BackgroundColor = COL_LIGHTBLUE;
    aRB1.TextColor = COL_LIGHTRED;
    aRB1.TextFont = Font( OUString( "Courier" ), Size( 0, 800 ) );
    aWriter.CreateControl( aRB1 );

    PDFWriter::RadioButtonWidget aRB2;
    aRB2.Name = "rb2_1";
    aRB2.Description = "radio 2 button 1";
    aRB2.Text = "Joy";
    aRB2.Location = Rectangle( Point( 10500, 15000 ), Size( 3000, 1000 ) );
    aRB2.Selected = true;
    aRB2.RadioGroup = 2;
    aWriter.CreateControl( aRB2 );

    PDFWriter::RadioButtonWidget aRB3;
    aRB3.Name = "rb1_2";
    aRB3.Description = "radio 1 button 2";
    aRB3.Text = "Desperation";
    aRB3.Location = Rectangle( Point( 4500, 16000 ), Size( 3000, 1000 ) );
    aRB3.Selected = true;
    aRB3.RadioGroup = 1;
    aWriter.CreateControl( aRB3 );

    PDFWriter::EditWidget aEditBox;
    aEditBox.Name = "testEdit";
    aEditBox.Description = "A test edit field";
    aEditBox.Text = "A little test text";
    aEditBox.TextStyle = DrawTextFlags::Left | DrawTextFlags::VCenter;
    aEditBox.Location = Rectangle( Point( 10000, 18000 ), Size( 5000, 1500 ) );
    aEditBox.MaxLen = 100;
    aEditBox.Border = aEditBox.Background = true;
    aEditBox.BorderColor = COL_BLACK;
    aWriter.CreateControl( aEditBox );

    // normal list box
    PDFWriter::ListBoxWidget aLstBox;
    aLstBox.Name = "testListBox";
    aLstBox.Text = "One";
    aLstBox.Description = "select me";
    aLstBox.Location = Rectangle( Point( 4500, 18000 ), Size( 3000, 1500 ) );
    aLstBox.Sort = true;
    aLstBox.MultiSelect = true;
    aLstBox.Border = aLstBox.Background = true;
    aLstBox.BorderColor = COL_BLACK;
    aLstBox.Entries.push_back( OUString( "One"  ) );
    aLstBox.Entries.push_back( OUString( "Two"  ) );
    aLstBox.Entries.push_back( OUString( "Three"  ) );
    aLstBox.Entries.push_back( OUString( "Four"  ) );
    aLstBox.SelectedEntries.push_back( 1 );
    aLstBox.SelectedEntries.push_back( 2 );
    aWriter.CreateControl( aLstBox );

    // dropdown list box
    aLstBox.Name = "testDropDownListBox";
    aLstBox.DropDown = true;
    aLstBox.Location = Rectangle( Point( 4500, 19500 ), Size( 3000, 500 ) );
    aWriter.CreateControl( aLstBox );

    // combo box
    PDFWriter::ComboBoxWidget aComboBox;
    aComboBox.Name = "testComboBox";
    aComboBox.Text = "test a combobox";
    aComboBox.Entries.push_back( OUString( "Larry"  ) );
    aComboBox.Entries.push_back( OUString( "Curly"  ) );
    aComboBox.Entries.push_back( OUString( "Moe"  ) );
    aComboBox.Location = Rectangle( Point( 4500, 20000 ), Size( 3000, 500 ) );
    aWriter.CreateControl( aComboBox );

    // test outlines
    sal_Int32 nPage1OL = aWriter.CreateOutlineItem();
    aWriter.SetOutlineItemText( nPage1OL, OUString( "Page 1"  ) );
    aWriter.SetOutlineItemDest( nPage1OL, nSecondDest );
    aWriter.CreateOutlineItem( nPage1OL, OUString( "Dest 2"  ), nSecondDest );
    aWriter.CreateOutlineItem( nPage1OL, OUString( "Dest 2 revisited"  ), nSecondDest );
    aWriter.CreateOutlineItem( nPage1OL, OUString( "Dest 2 again"  ), nSecondDest );
    sal_Int32 nPage2OL = aWriter.CreateOutlineItem();
    aWriter.SetOutlineItemText( nPage2OL, OUString( "Page 2"  ) );
    aWriter.CreateOutlineItem( nPage2OL, OUString( "Dest 1"  ), nFirstDest );

    aWriter.EndStructureElement(); // close document

    aWriter.Emit();
}
#endif

static const sal_Int32 nLog10Divisor = 3;
static const double fDivisor = 1000.0;

static inline double pixelToPoint( double px ) { return px/fDivisor; }
static inline sal_Int32 pointToPixel( double pt ) { return sal_Int32(pt*fDivisor); }

const sal_uInt8 PDFWriterImpl::s_nPadString[32] =
{
    0x28, 0xBF, 0x4E, 0x5E, 0x4E, 0x75, 0x8A, 0x41, 0x64, 0x00, 0x4E, 0x56, 0xFF, 0xFA, 0x01, 0x08,
    0x2E, 0x2E, 0x00, 0xB6, 0xD0, 0x68, 0x3E, 0x80, 0x2F, 0x0C, 0xA9, 0xFE, 0x64, 0x53, 0x69, 0x7A
};

static void appendHex( sal_Int8 nInt, OStringBuffer& rBuffer )
{
    static const sal_Char pHexDigits[] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                           '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    rBuffer.append( pHexDigits[ (nInt >> 4) & 15 ] );
    rBuffer.append( pHexDigits[ nInt & 15 ] );
}

static void appendName( const OUString& rStr, OStringBuffer& rBuffer )
{
// FIXME i59651 add a check for max length of 127 chars? Per PDF spec 1.4, appendix C.1
// I guess than when reading the #xx sequence it will count for a single character.
    OString aStr( OUStringToOString( rStr, RTL_TEXTENCODING_UTF8 ) );
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
        if( (aStr[i] >= 'A' && aStr[i] <= 'Z' ) ||
            (aStr[i] >= 'a' && aStr[i] <= 'z' ) ||
            (aStr[i] >= '0' && aStr[i] <= '9' ) ||
            aStr[i] == '-' )
        {
            rBuffer.append( aStr[i] );
        }
        else
        {
            rBuffer.append( '#' );
            appendHex( static_cast<sal_Int8>(aStr[i]), rBuffer );
        }
    }
}

static void appendName( const sal_Char* pStr, OStringBuffer& rBuffer )
{
    // FIXME i59651 see above
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
            appendHex( static_cast<sal_Int8>(*pStr), rBuffer );
        }
        pStr++;
    }
}

//used only to emit encoded passwords
static void appendLiteralString( const sal_Char* pStr, sal_Int32 nLength, OStringBuffer& rBuffer )
{
    while( nLength )
    {
        switch( *pStr )
        {
        case '\n' :
            rBuffer.append( "\\n" );
            break;
        case '\r' :
            rBuffer.append( "\\r" );
            break;
        case '\t' :
            rBuffer.append( "\\t" );
            break;
        case '\b' :
            rBuffer.append( "\\b" );
            break;
        case '\f' :
            rBuffer.append( "\\f" );
            break;
        case '(' :
        case ')' :
        case '\\' :
            rBuffer.append( "\\" );
            rBuffer.append( static_cast<sal_Char>(*pStr) );
            break;
        default:
            rBuffer.append( static_cast<sal_Char>(*pStr) );
            break;
        }
        pStr++;
        nLength--;
    }
}

/*
 * Convert a string before using it.
 *
 * This string conversion function is needed because the destination name
 * in a PDF file seen through an Internet browser should be
 * specially crafted, in order to be used directly by the browser.
 * In this way the fragment part of a hyperlink to a PDF file (e.g. something
 * as 'test1/test2/a-file.pdf\#thefragment) will be (hopefully) interpreted by the
 * PDF reader (currently only Adobe Reader plug-in seems to be working that way) called
 * from inside the Internet browser as: 'open the file test1/test2/a-file.pdf
 * and go to named destination thefragment using default zoom'.
 * The conversion is needed because in case of a fragment in the form: Slide%201
 * (meaning Slide 1) as it is converted obeying the Inet rules, it will become Slide25201
 * using this conversion, in both the generated named destinations, fragment and GoToR
 * destination.
 *
 * The names for destinations are name objects and so they don't need to be encrypted
 * even though they expose the content of PDF file (e.g. guessing the PDF content from the
 * destination name).
 *
 * Further limitation: it is advisable to use standard ASCII characters for
 * OOo bookmarks.
*/
static void appendDestinationName( const OUString& rString, OStringBuffer& rBuffer )
{
    const sal_Unicode* pStr = rString.getStr();
    sal_Int32 nLen = rString.getLength();
    for( int i = 0; i < nLen; i++ )
    {
        sal_Unicode aChar = pStr[i];
        if( (aChar >= '0' && aChar <= '9' ) ||
            (aChar >= 'a' && aChar <= 'z' ) ||
            (aChar >= 'A' && aChar <= 'Z' ) ||
            aChar == '-' )
        {
            rBuffer.append(static_cast<sal_Char>(aChar));
        }
        else
        {
            sal_Int8 aValueHigh = sal_Int8(aChar >> 8);
            if(aValueHigh > 0)
                appendHex( aValueHigh, rBuffer );
            appendHex( static_cast<sal_Int8>(aChar & 255 ), rBuffer );
        }
    }
}

void PDFWriter::AppendUnicodeTextString(const OUString& rString, OStringBuffer& rBuffer)
{
    rBuffer.append( "FEFF" );
    const sal_Unicode* pStr = rString.getStr();
    sal_Int32 nLen = rString.getLength();
    for( int i = 0; i < nLen; i++ )
    {
        sal_Unicode aChar = pStr[i];
        appendHex( static_cast<sal_Int8>(aChar >> 8), rBuffer );
        appendHex( static_cast<sal_Int8>(aChar & 255 ), rBuffer );
    }
}

void PDFWriterImpl::createWidgetFieldName( sal_Int32 i_nWidgetIndex, const PDFWriter::AnyWidget& i_rControl )
{
    /* #i80258# previously we use appendName here
       however we need a slightly different coding scheme than the normal
       name encoding for field names
    */
    const OUString& rName = (m_aContext.Version > PDFWriter::PDFVersion::PDF_1_2) ? i_rControl.Name : i_rControl.Text;
    OString aStr( OUStringToOString( rName, RTL_TEXTENCODING_UTF8 ) );
    int nLen = aStr.getLength();

    OStringBuffer aBuffer( rName.getLength()+64 );
    for( int i = 0; i < nLen; i++ )
    {
        /*  #i16920# PDF recommendation: output UTF8, any byte
         *  outside the interval [32(=ASCII' ');126(=ASCII'~')]
         *  should be escaped hexadecimal
         */
        if( aStr[i] >= 32 && aStr[i] <= 126 )
            aBuffer.append( aStr[i] );
        else
        {
            aBuffer.append( '#' );
            appendHex( static_cast<sal_Int8>(aStr[i]), aBuffer );
        }
    }

    OString aFullName( aBuffer.makeStringAndClear() );

    /* #i82785# create hierarchical fields down to the for each dot in i_rName */
    sal_Int32 nTokenIndex = 0, nLastTokenIndex = 0;
    OString aPartialName;
    OString aDomain;
    do
    {
        nLastTokenIndex = nTokenIndex;
        aPartialName = aFullName.getToken( 0, '.', nTokenIndex );
        if( nTokenIndex != -1 )
        {
            // find or create a hierarchical field
            // first find the fully qualified name up to this field
            aDomain = aFullName.copy( 0, nTokenIndex-1 );
            std::unordered_map< OString, sal_Int32 >::const_iterator it = m_aFieldNameMap.find( aDomain );
            if( it == m_aFieldNameMap.end() )
            {
                 // create new hierarchy field
                sal_Int32 nNewWidget = m_aWidgets.size();
                m_aWidgets.emplace_back( );
                m_aWidgets[nNewWidget].m_nObject = createObject();
                m_aWidgets[nNewWidget].m_eType = PDFWriter::Hierarchy;
                m_aWidgets[nNewWidget].m_aName = aPartialName;
                m_aWidgets[i_nWidgetIndex].m_nParent = m_aWidgets[nNewWidget].m_nObject;
                m_aFieldNameMap[aDomain] = nNewWidget;
                m_aWidgets[i_nWidgetIndex].m_nParent = m_aWidgets[nNewWidget].m_nObject;
                if( nLastTokenIndex > 0 )
                {
                    // this field is not a root field and
                    // needs to be inserted to its parent
                    OString aParentDomain( aDomain.copy( 0, nLastTokenIndex-1 ) );
                    it = m_aFieldNameMap.find( aParentDomain );
                    OSL_ENSURE( it != m_aFieldNameMap.end(), "field name not found" );
                    if( it != m_aFieldNameMap.end()  )
                    {
                        OSL_ENSURE( it->second < sal_Int32(m_aWidgets.size()), "invalid field number entry" );
                        if( it->second < sal_Int32(m_aWidgets.size()) )
                        {
                            PDFWidget& rParentField( m_aWidgets[it->second] );
                            rParentField.m_aKids.push_back( m_aWidgets[nNewWidget].m_nObject );
                            rParentField.m_aKidsIndex.push_back( nNewWidget );
                            m_aWidgets[nNewWidget].m_nParent = rParentField.m_nObject;
                        }
                    }
                }
            }
            else if( m_aWidgets[it->second].m_eType != PDFWriter::Hierarchy )
            {
                // this is invalid, someone tries to have a terminal field as parent
                // example: a button with the name foo.bar exists and
                // another button is named foo.bar.no
                // workaround: put the second terminal field as much up in the hierarchy as
                // necessary to have a non-terminal field as parent (or none at all)
                // since it->second already is terminal, we just need to use its parent
                aDomain.clear();
                aPartialName = aFullName.copy( aFullName.lastIndexOf( '.' )+1 );
                if( nLastTokenIndex > 0 )
                {
                    aDomain = aFullName.copy( 0, nLastTokenIndex-1 );
                    OStringBuffer aBuf( aDomain.getLength() + 1 + aPartialName.getLength() );
                    aBuf.append( aDomain );
                    aBuf.append( '.' );
                    aBuf.append( aPartialName );
                    aFullName = aBuf.makeStringAndClear();
                }
                else
                    aFullName = aPartialName;
                break;
            }
        }
    } while( nTokenIndex != -1 );

    // insert widget into its hierarchy field
    if( !aDomain.isEmpty() )
    {
        std::unordered_map< OString, sal_Int32 >::const_iterator it = m_aFieldNameMap.find( aDomain );
        if( it != m_aFieldNameMap.end() )
        {
            OSL_ENSURE( it->second >= 0 && it->second < sal_Int32( m_aWidgets.size() ), "invalid field index" );
            if( it->second >= 0 && it->second < sal_Int32(m_aWidgets.size()) )
            {
                m_aWidgets[i_nWidgetIndex].m_nParent = m_aWidgets[it->second].m_nObject;
                m_aWidgets[it->second].m_aKids.push_back( m_aWidgets[i_nWidgetIndex].m_nObject);
                m_aWidgets[it->second].m_aKidsIndex.push_back( i_nWidgetIndex );
            }
        }
    }

    if( aPartialName.isEmpty() )
    {
        // how funny, an empty field name
        if( i_rControl.getType() == PDFWriter::RadioButton )
        {
            aPartialName  = "RadioGroup";
            aPartialName += OString::number( static_cast<const PDFWriter::RadioButtonWidget&>(i_rControl).RadioGroup );
        }
        else
            aPartialName = OString( "Widget" );
    }

    if( ! m_aContext.AllowDuplicateFieldNames )
    {
        std::unordered_map<OString, sal_Int32>::iterator it = m_aFieldNameMap.find( aFullName );

        if( it != m_aFieldNameMap.end() ) // not unique
        {
            std::unordered_map< OString, sal_Int32 >::const_iterator check_it;
            OString aTry;
            sal_Int32 nTry = 2;
            do
            {
                OStringBuffer aUnique( aFullName.getLength() + 16 );
                aUnique.append( aFullName );
                aUnique.append( '_' );
                aUnique.append( nTry++ );
                aTry = aUnique.makeStringAndClear();
                check_it = m_aFieldNameMap.find( aTry );
            } while( check_it != m_aFieldNameMap.end() );
            aFullName = aTry;
            m_aFieldNameMap[ aFullName ] = i_nWidgetIndex;
            aPartialName = aFullName.copy( aFullName.lastIndexOf( '.' )+1 );
        }
        else
            m_aFieldNameMap[ aFullName ] = i_nWidgetIndex;
    }

    // finally
    m_aWidgets[i_nWidgetIndex].m_aName = aPartialName;
}

static void appendFixedInt( sal_Int32 nValue, OStringBuffer& rBuffer )
{
    if( nValue < 0 )
    {
        rBuffer.append( '-' );
        nValue = -nValue;
    }
    sal_Int32 nFactor = 1, nDiv = nLog10Divisor;
    while( nDiv-- )
        nFactor *= 10;

    sal_Int32 nInt = nValue / nFactor;
    rBuffer.append( nInt );
    if (nFactor > 1 && nValue % nFactor)
    {
        rBuffer.append( '.' );
        do
        {
            nFactor /= 10;
            rBuffer.append((nValue / nFactor) % 10);
        }
        while (nFactor > 1 && nValue % nFactor); // omit trailing zeros
    }
}

// appends a double. PDF does not accept exponential format, only fixed point
static void appendDouble( double fValue, OStringBuffer& rBuffer, sal_Int32 nPrecision = 5 )
{
    bool bNeg = false;
    if( fValue < 0.0 )
    {
        bNeg = true;
        fValue=-fValue;
    }

    sal_Int64 nInt = static_cast<sal_Int64>(fValue);
    fValue -= static_cast<double>(nInt);
    // optimizing hardware may lead to a value of 1.0 after the subtraction
    if( rtl::math::approxEqual(fValue, 1.0) || log10( 1.0-fValue ) <= -nPrecision )
    {
        nInt++;
        fValue = 0.0;
    }
    sal_Int64 nFrac = 0;
    if( fValue )
    {
        fValue *= pow( 10.0, static_cast<double>(nPrecision) );
        nFrac = static_cast<sal_Int64>(fValue);
    }
    if( bNeg && ( nInt || nFrac ) )
        rBuffer.append( '-' );
    rBuffer.append( nInt );
    if( nFrac )
    {
        int i;
        rBuffer.append( '.' );
        sal_Int64 nBound = static_cast<sal_Int64>(pow( 10.0, nPrecision - 1.0 )+0.5);
        for ( i = 0; ( i < nPrecision ) && nFrac; i++ )
        {
            sal_Int64 nNumb = nFrac / nBound;
            nFrac -= nNumb * nBound;
            rBuffer.append( nNumb );
            nBound /= 10;
        }
    }
}

static void appendColor( const Color& rColor, OStringBuffer& rBuffer, bool bConvertToGrey )
{

    if( rColor != COL_TRANSPARENT )
    {
        if( bConvertToGrey )
        {
            sal_uInt8 cByte = rColor.GetLuminance();
            appendDouble( static_cast<double>(cByte) / 255.0, rBuffer );
        }
        else
        {
            appendDouble( static_cast<double>(rColor.GetRed()) / 255.0, rBuffer );
            rBuffer.append( ' ' );
            appendDouble( static_cast<double>(rColor.GetGreen()) / 255.0, rBuffer );
            rBuffer.append( ' ' );
            appendDouble( static_cast<double>(rColor.GetBlue()) / 255.0, rBuffer );
        }
    }
}

void PDFWriterImpl::appendStrokingColor( const Color& rColor, OStringBuffer& rBuffer )
{
    if( rColor != COL_TRANSPARENT )
    {
        bool bGrey = m_aContext.ColorMode == PDFWriter::DrawGreyscale;
        appendColor( rColor, rBuffer, bGrey );
        rBuffer.append( bGrey ? " G" : " RG" );
    }
}

void PDFWriterImpl::appendNonStrokingColor( const Color& rColor, OStringBuffer& rBuffer )
{
    if( rColor != COL_TRANSPARENT )
    {
        bool bGrey = m_aContext.ColorMode == PDFWriter::DrawGreyscale;
        appendColor( rColor, rBuffer, bGrey );
        rBuffer.append( bGrey ? " g" : " rg" );
    }
}

// matrix helper class
// TODO: use basegfx matrix class instead or derive from it
namespace vcl // TODO: use anonymous namespace to keep this class local
{
/*  for sparse matrices of the form (2D linear transformations)
 *  f[0] f[1] 0
 *  f[2] f[3] 0
 *  f[4] f[5] 1
 */
class Matrix3
{
    double f[6];

    void set( const double *pn ) { for( int i = 0 ; i < 6; i++ ) f[i] = pn[i]; }
public:
    Matrix3();

    void skew( double alpha, double beta );
    void scale( double sx, double sy );
    void rotate( double angle );
    void translate( double tx, double ty );
    void invert();

    void append( PDFWriterImpl::PDFPage const & rPage, OStringBuffer& rBuffer );

    Point transform( const Point& rPoint ) const;
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

Point Matrix3::transform( const Point& rOrig ) const
{
    double x = static_cast<double>(rOrig.X()), y = static_cast<double>(rOrig.Y());
    return Point( static_cast<int>(x*f[0] + y*f[2] + f[4]), static_cast<int>(x*f[1] + y*f[3] + f[5]) );
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

void Matrix3::invert()
{
    // short circuit trivial cases
    if( f[1]==f[2] && f[1]==0.0 && f[0]==f[3] && f[0]==1.0 )
    {
        f[4] = -f[4];
        f[5] = -f[5];
        return;
    }

    // check determinant
    const double fDet = f[0]*f[3]-f[1]*f[2];
    if( fDet == 0.0 )
        return;

    // invert the matrix
    double fn[6];
    fn[0] = +f[3] / fDet;
    fn[1] = -f[1] / fDet;
    fn[2] = -f[2] / fDet;
    fn[3] = +f[0] / fDet;

    // apply inversion to translation
    fn[4] = -(f[4]*fn[0] + f[5]*fn[2]);
    fn[5] = -(f[4]*fn[1] + f[5]*fn[3]);

    set( fn );
}

void Matrix3::append( PDFWriterImpl::PDFPage const & rPage, OStringBuffer& rBuffer )
{
    appendDouble( f[0], rBuffer );
    rBuffer.append( ' ' );
    appendDouble( f[1], rBuffer );
    rBuffer.append( ' ' );
    appendDouble( f[2], rBuffer );
    rBuffer.append( ' ' );
    appendDouble( f[3], rBuffer );
    rBuffer.append( ' ' );
    rPage.appendPoint( Point( static_cast<long>(f[4]), static_cast<long>(f[5]) ), rBuffer );
}

static void appendResourceMap( OStringBuffer& rBuf, const char* pPrefix, const PDFWriterImpl::ResourceMap& rList )
{
    if( rList.empty() )
        return;
    rBuf.append( '/' );
    rBuf.append( pPrefix );
    rBuf.append( "<<" );
    int ni = 0;
    for (auto const& item : rList)
    {
        if( !item.first.isEmpty() && item.second > 0 )
        {
            rBuf.append( '/' );
            rBuf.append( item.first );
            rBuf.append( ' ' );
            rBuf.append( item.second );
            rBuf.append( " 0 R" );
            if( ((++ni) & 7) == 0 )
                rBuf.append( '\n' );
        }
    }
    rBuf.append( ">>\n" );
}

void PDFWriterImpl::ResourceDict::append( OStringBuffer& rBuf, sal_Int32 nFontDictObject )
{
    rBuf.append( "<</Font " );
    rBuf.append( nFontDictObject );
    rBuf.append( " 0 R\n" );
    appendResourceMap( rBuf, "XObject", m_aXObjects );
    appendResourceMap( rBuf, "ExtGState", m_aExtGStates );
    appendResourceMap( rBuf, "Shading", m_aShadings );
    appendResourceMap( rBuf, "Pattern", m_aPatterns );
    rBuf.append( "/ProcSet[/PDF/Text" );
    if( !m_aXObjects.empty() )
        rBuf.append( "/ImageC/ImageI/ImageB" );
    rBuf.append( "]\n>>\n" );
};

PDFWriterImpl::PDFPage::PDFPage( PDFWriterImpl* pWriter, double nPageWidth, double nPageHeight, PDFWriter::Orientation eOrientation )
        :
        m_pWriter( pWriter ),
        m_nPageWidth( nPageWidth ),
        m_nPageHeight( nPageHeight ),
        m_eOrientation( eOrientation ),
        m_nPageObject( 0 ),  // invalid object number
        m_nStreamLengthObject( 0 ),
        m_nBeginStreamPos( 0 ),
        m_eTransition( PDFWriter::PageTransition::Regular ),
        m_nTransTime( 0 )
{
    // object ref must be only ever updated in emit()
    m_nPageObject = m_pWriter->createObject();
}

PDFWriterImpl::PDFPage::~PDFPage()
{
}

void PDFWriterImpl::PDFPage::beginStream()
{
    if (g_bDebugDisableCompression)
    {
        m_pWriter->emitComment("PDFWriterImpl::PDFPage::beginStream, +");
    }
    m_aStreamObjects.push_back(m_pWriter->createObject());
    if( ! m_pWriter->updateObject( m_aStreamObjects.back() ) )
        return;

    m_nStreamLengthObject = m_pWriter->createObject();
    // write content stream header
    OStringBuffer aLine;
    aLine.append( m_aStreamObjects.back() );
    aLine.append( " 0 obj\n<</Length " );
    aLine.append( m_nStreamLengthObject );
    aLine.append( " 0 R" );
    if (!g_bDebugDisableCompression)
        aLine.append( "/Filter/FlateDecode" );
    aLine.append( ">>\nstream\n" );
    if( ! m_pWriter->writeBuffer( aLine.getStr(), aLine.getLength() ) )
        return;
    if (osl::File::E_None != m_pWriter->m_aFile.getPos(m_nBeginStreamPos))
    {
        m_pWriter->m_aFile.close();
        m_pWriter->m_bOpen = false;
    }
    if (!g_bDebugDisableCompression)
        m_pWriter->beginCompression();
    m_pWriter->checkAndEnableStreamEncryption( m_aStreamObjects.back() );
}

void PDFWriterImpl::PDFPage::endStream()
{
    if (!g_bDebugDisableCompression)
        m_pWriter->endCompression();
    sal_uInt64 nEndStreamPos;
    if (osl::File::E_None != m_pWriter->m_aFile.getPos(nEndStreamPos))
    {
        m_pWriter->m_aFile.close();
        m_pWriter->m_bOpen = false;
        return;
    }
    m_pWriter->disableStreamEncryption();
    if( ! m_pWriter->writeBuffer( "\nendstream\nendobj\n\n", 19 ) )
        return;
    // emit stream length object
    if( ! m_pWriter->updateObject( m_nStreamLengthObject ) )
        return;
    OStringBuffer aLine;
    aLine.append( m_nStreamLengthObject );
    aLine.append( " 0 obj\n" );
    aLine.append( static_cast<sal_Int64>(nEndStreamPos-m_nBeginStreamPos) );
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
        case PDFWriter::Orientation::Portrait: aLine.append( "/Rotate 0\n" );break;
        case PDFWriter::Orientation::Inherit:  break;
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
    if( m_eTransition != PDFWriter::PageTransition::Regular && m_nTransTime > 0 )
    {
        // transition duration
        aLine.append( "/Trans<</D " );
        appendDouble( static_cast<double>(m_nTransTime)/1000.0, aLine, 3 );
        aLine.append( "\n" );
        const char *pStyle = nullptr, *pDm = nullptr, *pM = nullptr, *pDi = nullptr;
        switch( m_eTransition )
        {
            case PDFWriter::PageTransition::SplitHorizontalInward:
                pStyle = "Split"; pDm = "H"; pM = "I"; break;
            case PDFWriter::PageTransition::SplitHorizontalOutward:
                pStyle = "Split"; pDm = "H"; pM = "O"; break;
            case PDFWriter::PageTransition::SplitVerticalInward:
                pStyle = "Split"; pDm = "V"; pM = "I"; break;
            case PDFWriter::PageTransition::SplitVerticalOutward:
                pStyle = "Split"; pDm = "V"; pM = "O"; break;
            case PDFWriter::PageTransition::BlindsHorizontal:
                pStyle = "Blinds"; pDm = "H"; break;
            case PDFWriter::PageTransition::BlindsVertical:
                pStyle = "Blinds"; pDm = "V"; break;
            case PDFWriter::PageTransition::BoxInward:
                pStyle = "Box"; pM = "I"; break;
            case PDFWriter::PageTransition::BoxOutward:
                pStyle = "Box"; pM = "O"; break;
            case PDFWriter::PageTransition::WipeLeftToRight:
                pStyle = "Wipe"; pDi = "0"; break;
            case PDFWriter::PageTransition::WipeBottomToTop:
                pStyle = "Wipe"; pDi = "90"; break;
            case PDFWriter::PageTransition::WipeRightToLeft:
                pStyle = "Wipe"; pDi = "180"; break;
            case PDFWriter::PageTransition::WipeTopToBottom:
                pStyle = "Wipe"; pDi = "270"; break;
            case PDFWriter::PageTransition::Dissolve:
                pStyle = "Dissolve"; break;
            case PDFWriter::PageTransition::Regular:
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
    if( m_pWriter->getVersion() > PDFWriter::PDFVersion::PDF_1_3 && ! m_pWriter->m_bIsPDF_A1 )
    {
        aLine.append( "/Group<</S/Transparency/CS/DeviceRGB/I true>>" );
    }
    aLine.append( "/Contents" );
    unsigned int nStreamObjects = m_aStreamObjects.size();
    if( nStreamObjects > 1 )
        aLine.append( '[' );
    for(sal_Int32 i : m_aStreamObjects)
    {
        aLine.append( ' ' );
        aLine.append( i );
        aLine.append( " 0 R" );
    }
    if( nStreamObjects > 1 )
        aLine.append( ']' );
    aLine.append( ">>\nendobj\n\n" );
    return m_pWriter->writeBuffer( aLine.getStr(), aLine.getLength() );
}

namespace vcl
{
template < class GEOMETRY >
GEOMETRY lcl_convert( const MapMode& _rSource, const MapMode& _rDest, OutputDevice* _pPixelConversion, const GEOMETRY& _rObject )
{
    GEOMETRY aPoint;
    if ( MapUnit::MapPixel == _rSource.GetMapUnit() )
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

void PDFWriterImpl::PDFPage::appendPoint( const Point& rPoint, OStringBuffer& rBuffer ) const
{
    Point aPoint( lcl_convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                               m_pWriter->m_aMapMode,
                               m_pWriter->getReferenceDevice(),
                               rPoint ) );

    sal_Int32 nValue    = aPoint.X();

    appendFixedInt( nValue, rBuffer );

    rBuffer.append( ' ' );

    nValue      = pointToPixel(getHeight()) - aPoint.Y();

    appendFixedInt( nValue, rBuffer );
}

void PDFWriterImpl::PDFPage::appendPixelPoint( const basegfx::B2DPoint& rPoint, OStringBuffer& rBuffer ) const
{
    double fValue   = pixelToPoint(rPoint.getX());

    appendDouble( fValue, rBuffer, nLog10Divisor );
    rBuffer.append( ' ' );
    fValue      = getHeight() - pixelToPoint(rPoint.getY());
    appendDouble( fValue, rBuffer, nLog10Divisor );
}

void PDFWriterImpl::PDFPage::appendRect( const tools::Rectangle& rRect, OStringBuffer& rBuffer ) const
{
    appendPoint( rRect.BottomLeft() + Point( 0, 1 ), rBuffer );
    rBuffer.append( ' ' );
    appendMappedLength( static_cast<sal_Int32>(rRect.GetWidth()), rBuffer, false );
    rBuffer.append( ' ' );
    appendMappedLength( static_cast<sal_Int32>(rRect.GetHeight()), rBuffer );
    rBuffer.append( " re" );
}

void PDFWriterImpl::PDFPage::convertRect( tools::Rectangle& rRect ) const
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
    rRect.SetLeft( aLL.X() );
    rRect.SetRight( aLL.X() + aSize.Width() );
    rRect.SetTop( pointToPixel(getHeight()) - aLL.Y() );
    rRect.SetBottom( rRect.Top() + aSize.Height() );
}

void PDFWriterImpl::PDFPage::appendPolygon( const tools::Polygon& rPoly, OStringBuffer& rBuffer, bool bClose ) const
{
    sal_uInt16 nPoints = rPoly.GetSize();
    /*
     *  #108582# applications do weird things
     */
    sal_uInt32 nBufLen = rBuffer.getLength();
    if( nPoints > 0 )
    {
        const PolyFlags* pFlagArray = rPoly.GetConstFlagAry();
        appendPoint( rPoly[0], rBuffer );
        rBuffer.append( " m\n" );
        for( sal_uInt16 i = 1; i < nPoints; i++ )
        {
            if( pFlagArray && pFlagArray[i] == PolyFlags::Control && nPoints-i > 2 )
            {
                // bezier
                SAL_WARN_IF( pFlagArray[i+1] != PolyFlags::Control || pFlagArray[i+2] == PolyFlags::Control, "vcl.pdfwriter", "unexpected sequence of control points" );
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

void PDFWriterImpl::PDFPage::appendPolygon( const basegfx::B2DPolygon& rPoly, OStringBuffer& rBuffer ) const
{
    basegfx::B2DPolygon aPoly( lcl_convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                                            m_pWriter->m_aMapMode,
                                            m_pWriter->getReferenceDevice(),
                                            rPoly ) );

    if( basegfx::utils::isRectangle( aPoly ) )
    {
        basegfx::B2DRange aRange( aPoly.getB2DRange() );
        basegfx::B2DPoint aBL( aRange.getMinX(), aRange.getMaxY() );
        appendPixelPoint( aBL, rBuffer );
        rBuffer.append( ' ' );
        appendMappedLength( aRange.getWidth(), rBuffer, false, nLog10Divisor );
        rBuffer.append( ' ' );
        appendMappedLength( aRange.getHeight(), rBuffer, true, nLog10Divisor );
        rBuffer.append( " re\n" );
        return;
    }
    sal_uInt32 nPoints = aPoly.count();
    if( nPoints > 0 )
    {
        sal_uInt32 nBufLen = rBuffer.getLength();
        basegfx::B2DPoint aLastPoint( aPoly.getB2DPoint( 0 ) );
        appendPixelPoint( aLastPoint, rBuffer );
        rBuffer.append( " m\n" );
        for( sal_uInt32 i = 1; i <= nPoints; i++ )
        {
            if( i != nPoints || aPoly.isClosed() )
            {
                sal_uInt32 nCurPoint  = i % nPoints;
                sal_uInt32 nLastPoint = i-1;
                basegfx::B2DPoint aPoint( aPoly.getB2DPoint( nCurPoint ) );
                if( aPoly.isNextControlPointUsed( nLastPoint ) &&
                    aPoly.isPrevControlPointUsed( nCurPoint ) )
                {
                    appendPixelPoint( aPoly.getNextControlPoint( nLastPoint ), rBuffer );
                    rBuffer.append( ' ' );
                    appendPixelPoint( aPoly.getPrevControlPoint( nCurPoint ), rBuffer );
                    rBuffer.append( ' ' );
                    appendPixelPoint( aPoint, rBuffer );
                    rBuffer.append( " c" );
                }
                else if( aPoly.isNextControlPointUsed( nLastPoint ) )
                {
                    appendPixelPoint( aPoly.getNextControlPoint( nLastPoint ), rBuffer );
                    rBuffer.append( ' ' );
                    appendPixelPoint( aPoint, rBuffer );
                    rBuffer.append( " y" );
                }
                else if( aPoly.isPrevControlPointUsed( nCurPoint ) )
                {
                    appendPixelPoint( aPoly.getPrevControlPoint( nCurPoint ), rBuffer );
                    rBuffer.append( ' ' );
                    appendPixelPoint( aPoint, rBuffer );
                    rBuffer.append( " v" );
                }
                else
                {
                    appendPixelPoint( aPoint, rBuffer );
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
        }
        rBuffer.append( "h\n" );
    }
}

void PDFWriterImpl::PDFPage::appendPolyPolygon( const tools::PolyPolygon& rPolyPoly, OStringBuffer& rBuffer ) const
{
    sal_uInt16 nPolygons = rPolyPoly.Count();
    for( sal_uInt16 n = 0; n < nPolygons; n++ )
        appendPolygon( rPolyPoly[n], rBuffer );
}

void PDFWriterImpl::PDFPage::appendPolyPolygon( const basegfx::B2DPolyPolygon& rPolyPoly, OStringBuffer& rBuffer ) const
{
    sal_uInt32 nPolygons = rPolyPoly.count();
    for( sal_uInt32 n = 0; n < nPolygons; n++ )
        appendPolygon( rPolyPoly.getB2DPolygon( n ), rBuffer );
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

    appendFixedInt( nValue, rBuffer );
}

void PDFWriterImpl::PDFPage::appendMappedLength( double fLength, OStringBuffer& rBuffer, bool bVertical, sal_Int32 nPrecision ) const
{
    Size aSize( lcl_convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                             m_pWriter->m_aMapMode,
                             m_pWriter->getReferenceDevice(),
                             Size( 1000, 1000 ) ) );
    fLength *= pixelToPoint(static_cast<double>(bVertical ? aSize.Height() : aSize.Width()) / 1000.0);
    appendDouble( fLength, rBuffer, nPrecision );
}

bool PDFWriterImpl::PDFPage::appendLineInfo( const LineInfo& rInfo, OStringBuffer& rBuffer ) const
{
    if(LineStyle::Dash == rInfo.GetStyle() && rInfo.GetDashLen() != rInfo.GetDotLen())
    {
        // dashed and non-degraded case, check for implementation limits of dash array
        // in PDF reader apps (e.g. acroread)
        if(2 * (rInfo.GetDashCount() + rInfo.GetDotCount()) > 10)
        {
            return false;
        }
    }

    if(basegfx::B2DLineJoin::NONE != rInfo.GetLineJoin())
    {
        // LineJoin used, ExtLineInfo required
        return false;
    }

    if(css::drawing::LineCap_BUTT != rInfo.GetLineCap())
    {
        // LineCap used, ExtLineInfo required
        return false;
    }

    if( rInfo.GetStyle() == LineStyle::Dash )
    {
        rBuffer.append( "[ " );
        if( rInfo.GetDashLen() == rInfo.GetDotLen() ) // degraded case
        {
            appendMappedLength( static_cast<sal_Int32>(rInfo.GetDashLen()), rBuffer );
            rBuffer.append( ' ' );
            appendMappedLength( static_cast<sal_Int32>(rInfo.GetDistance()), rBuffer );
            rBuffer.append( ' ' );
        }
        else
        {
            for( int n = 0; n < rInfo.GetDashCount(); n++ )
            {
                appendMappedLength( static_cast<sal_Int32>(rInfo.GetDashLen()), rBuffer );
                rBuffer.append( ' ' );
                appendMappedLength( static_cast<sal_Int32>(rInfo.GetDistance()), rBuffer );
                rBuffer.append( ' ' );
            }
            for( int m = 0; m < rInfo.GetDotCount(); m++ )
            {
                appendMappedLength( static_cast<sal_Int32>(rInfo.GetDotLen()), rBuffer );
                rBuffer.append( ' ' );
                appendMappedLength( static_cast<sal_Int32>(rInfo.GetDistance()), rBuffer );
                rBuffer.append( ' ' );
            }
        }
        rBuffer.append( "] 0 d\n" );
    }

    if( rInfo.GetWidth() > 1 )
    {
        appendMappedLength( static_cast<sal_Int32>(rInfo.GetWidth()), rBuffer );
        rBuffer.append( " w\n" );
    }
    else if( rInfo.GetWidth() == 0 )
    {
        // "pixel" line
        appendDouble( 72.0/double(m_pWriter->getReferenceDevice()->GetDPIX()), rBuffer );
        rBuffer.append( " w\n" );
    }

    return true;
}

void PDFWriterImpl::PDFPage::appendWaveLine( sal_Int32 nWidth, sal_Int32 nY, sal_Int32 nDelta, OStringBuffer& rBuffer ) const
{
    if( nWidth <= 0 )
        return;
    if( nDelta < 1 )
        nDelta = 1;

    rBuffer.append( "0 " );
    appendMappedLength( nY, rBuffer );
    rBuffer.append( " m\n" );
    for( sal_Int32 n = 0; n < nWidth; )
    {
        n += nDelta;
        appendMappedLength( n, rBuffer, false );
        rBuffer.append( ' ' );
        appendMappedLength( nDelta+nY, rBuffer );
        rBuffer.append( ' ' );
        n += nDelta;
        appendMappedLength( n, rBuffer, false );
        rBuffer.append( ' ' );
        appendMappedLength( nY, rBuffer );
        rBuffer.append( " v " );
        if( n < nWidth )
        {
            n += nDelta;
            appendMappedLength( n, rBuffer, false );
            rBuffer.append( ' ' );
            appendMappedLength( nY-nDelta, rBuffer );
            rBuffer.append( ' ' );
            n += nDelta;
            appendMappedLength( n, rBuffer, false );
            rBuffer.append( ' ' );
            appendMappedLength( nY, rBuffer );
            rBuffer.append( " v\n" );
        }
    }
    rBuffer.append( "S\n" );
}

 PDFWriterImpl::PDFWriterImpl( const PDFWriter::PDFWriterContext& rContext,
                               const css::uno::Reference< css::beans::XMaterialHolder >& xEnc,
                               PDFWriter& i_rOuterFace)
        :
        m_pReferenceDevice( nullptr ),
        m_aMapMode( MapUnit::MapPoint, Point(), Fraction( 1, pointToPixel(1) ), Fraction( 1, pointToPixel(1) ) ),
        m_nCurrentStructElement( 0 ),
        m_bEmitStructure( true ),
        m_nNextFID( 1 ),
        m_nInheritedPageWidth( 595 ),  // default A4
        m_nInheritedPageHeight( 842 ), // default A4
        m_nCurrentPage( -1 ),
        m_nCatalogObject(0),
        m_nSignatureObject( -1 ),
        m_nSignatureContentOffset( 0 ),
        m_nSignatureLastByteRangeNoOffset( 0 ),
        m_nResourceDict( -1 ),
        m_nFontDictObject( -1 ),
        m_aContext(rContext),
        m_aFile(m_aContext.URL),
        m_bOpen(false),
        m_DocDigest(::comphelper::HashType::MD5),
        m_aCipher( nullptr ),
        m_nKeyLength(0),
        m_nRC4KeyLength(0),
        m_bEncryptThisStream( false ),
        m_nAccessPermissions(0),
        m_pEncryptionBuffer( nullptr ),
        m_nEncryptionBufferSize( 0 ),
        m_bIsPDF_A1( false ),
        m_rOuterFace( i_rOuterFace )
{
#ifdef DO_TEST_PDF
    static bool bOnce = true;
    if( bOnce )
    {
        bOnce = false;
        doTestCode();
    }
#endif
    m_aStructure.emplace_back( );
    m_aStructure[0].m_nOwnElement       = 0;
    m_aStructure[0].m_nParentElement    = 0;

    Font aFont;
    aFont.SetFamilyName( "Times" );
    aFont.SetFontSize( Size( 0, 12 ) );

    GraphicsState aState;
    aState.m_aMapMode       = m_aMapMode;
    aState.m_aFont          = aFont;
    m_aGraphicsStack.push_front( aState );

    osl::File::RC aError = m_aFile.open(osl_File_OpenFlag_Write | osl_File_OpenFlag_Create);
    if (aError != osl::File::E_None)
    {
        if (aError == osl::File::E_EXIST)
        {
            aError = m_aFile.open(osl_File_OpenFlag_Write);
            if (aError == osl::File::E_None)
                aError = m_aFile.setSize(0);
        }
    }
    if (aError != osl::File::E_None)
        return;

    m_bOpen = true;

    // setup DocInfo
    setupDocInfo();

    /* prepare the cypher engine, can be done in CTOR, free in DTOR */
    m_aCipher = rtl_cipher_createARCFOUR( rtl_Cipher_ModeStream );

    /* the size of the Codec default maximum */
    /* is this 0x4000 required to be the same as MAX_SIGNATURE_CONTENT_LENGTH or just coincidentally the same at the moment? */
    if (!checkEncryptionBufferSize(0x4000))
    {
        m_aFile.close();
        m_bOpen = false;
        return;
    }

    if( xEnc.is() )
        prepareEncryption( xEnc );

    if( m_aContext.Encryption.Encrypt() )
    {
        // sanity check
        if( m_aContext.Encryption.OValue.size() != ENCRYPTED_PWD_SIZE ||
            m_aContext.Encryption.UValue.size() != ENCRYPTED_PWD_SIZE ||
            m_aContext.Encryption.EncryptionKey.size() != MAXIMUM_RC4_KEY_LENGTH
           )
        {
            // the field lengths are invalid ? This was not setup by initEncryption.
            // do not encrypt after all
            m_aContext.Encryption.OValue.clear();
            m_aContext.Encryption.UValue.clear();
            OSL_ENSURE( false, "encryption data failed sanity check, encryption disabled" );
        }
        else // setup key lengths
            m_nAccessPermissions = computeAccessPermissions( m_aContext.Encryption, m_nKeyLength, m_nRC4KeyLength );
    }

    // write header
    OStringBuffer aBuffer( 20 );
    aBuffer.append( "%PDF-" );
    switch( m_aContext.Version )
    {
        case PDFWriter::PDFVersion::PDF_1_2: aBuffer.append( "1.2" );break;
        case PDFWriter::PDFVersion::PDF_1_3: aBuffer.append( "1.3" );break;
        case PDFWriter::PDFVersion::PDF_A_1:
        case PDFWriter::PDFVersion::PDF_1_4: aBuffer.append( "1.4" );break;
        default:
        case PDFWriter::PDFVersion::PDF_1_5: aBuffer.append( "1.5" );break;
    }
    // append something binary as comment (suggested in PDF Reference)
    aBuffer.append( "\n%\303\244\303\274\303\266\303\237\n" );
    if( !writeBuffer( aBuffer.getStr(), aBuffer.getLength() ) )
    {
        m_aFile.close();
        m_bOpen = false;
        return;
    }

    // insert outline root
    m_aOutline.emplace_back( );

    m_bIsPDF_A1 = (m_aContext.Version == PDFWriter::PDFVersion::PDF_A_1);
    if( m_bIsPDF_A1 )
        m_aContext.Version = PDFWriter::PDFVersion::PDF_1_4; //meaning we need PDF 1.4, PDF/A flavour
}

PDFWriterImpl::~PDFWriterImpl()
{
    m_pReferenceDevice.disposeAndClear();

    if( m_aCipher )
        rtl_cipher_destroyARCFOUR( m_aCipher );

    rtl_freeMemory( m_pEncryptionBuffer );
}

void PDFWriterImpl::setupDocInfo()
{
    std::vector< sal_uInt8 > aId;
    m_aCreationDateString = PDFWriter::GetDateTime();
    computeDocumentIdentifier( aId, m_aContext.DocumentInfo, m_aCreationDateString, m_aCreationMetaDateString );
    if( m_aContext.Encryption.DocumentIdentifier.empty() )
        m_aContext.Encryption.DocumentIdentifier = aId;
}

OString PDFWriter::GetDateTime()
{
    OStringBuffer aRet;

    TimeValue aTVal, aGMT;
    oslDateTime aDT;
    osl_getSystemTime(&aGMT);
    osl_getLocalTimeFromSystemTime(&aGMT, &aTVal);
    osl_getDateTimeFromTimeValue(&aTVal, &aDT);
    aRet.append("D:");
    aRet.append(static_cast<sal_Char>('0' + ((aDT.Year / 1000) % 10)));
    aRet.append(static_cast<sal_Char>('0' + ((aDT.Year / 100) % 10)));
    aRet.append(static_cast<sal_Char>('0' + ((aDT.Year / 10) % 10)));
    aRet.append(static_cast<sal_Char>('0' + (aDT.Year % 10)));
    aRet.append(static_cast<sal_Char>('0' + ((aDT.Month / 10) % 10)));
    aRet.append(static_cast<sal_Char>('0' + (aDT.Month % 10)));
    aRet.append(static_cast<sal_Char>('0' + ((aDT.Day / 10) % 10)));
    aRet.append(static_cast<sal_Char>('0' + (aDT.Day % 10)));
    aRet.append(static_cast<sal_Char>('0' + ((aDT.Hours / 10) % 10)));
    aRet.append(static_cast<sal_Char>('0' + (aDT.Hours % 10)));
    aRet.append(static_cast<sal_Char>('0' + ((aDT.Minutes / 10) % 10)));
    aRet.append(static_cast<sal_Char>('0' + (aDT.Minutes % 10)));
    aRet.append(static_cast<sal_Char>('0' + ((aDT.Seconds / 10) % 10)));
    aRet.append(static_cast<sal_Char>('0' + (aDT.Seconds % 10)));

    sal_uInt32 nDelta = 0;
    if (aGMT.Seconds > aTVal.Seconds)
    {
        aRet.append("-");
        nDelta = aGMT.Seconds-aTVal.Seconds;
    }
    else if (aGMT.Seconds < aTVal.Seconds)
    {
        aRet.append("+");
        nDelta = aTVal.Seconds-aGMT.Seconds;
    }
    else
        aRet.append("Z");

    if (nDelta)
    {
        aRet.append(static_cast<sal_Char>('0' + ((nDelta / 36000) % 10)));
        aRet.append(static_cast<sal_Char>('0' + ((nDelta / 3600) % 10)));
        aRet.append("'");
        aRet.append(static_cast<sal_Char>('0' + ((nDelta / 600) % 6)));
        aRet.append(static_cast<sal_Char>('0' + ((nDelta / 60) % 10)));
    }
    aRet.append( "'" );

    return aRet.makeStringAndClear();
}

void PDFWriterImpl::computeDocumentIdentifier( std::vector< sal_uInt8 >& o_rIdentifier,
                                               const vcl::PDFWriter::PDFDocInfo& i_rDocInfo,
                                               const OString& i_rCString1,
                                               OString& o_rCString2
                                               )
{
    o_rIdentifier.clear();

    //build the document id
    OString aInfoValuesOut;
    OStringBuffer aID( 1024 );
    if( !i_rDocInfo.Title.isEmpty() )
        PDFWriter::AppendUnicodeTextString(i_rDocInfo.Title, aID);
    if( !i_rDocInfo.Author.isEmpty() )
        PDFWriter::AppendUnicodeTextString(i_rDocInfo.Author, aID);
    if( !i_rDocInfo.Subject.isEmpty() )
        PDFWriter::AppendUnicodeTextString(i_rDocInfo.Subject, aID);
    if( !i_rDocInfo.Keywords.isEmpty() )
        PDFWriter::AppendUnicodeTextString(i_rDocInfo.Keywords, aID);
    if( !i_rDocInfo.Creator.isEmpty() )
        PDFWriter::AppendUnicodeTextString(i_rDocInfo.Creator, aID);
    if( !i_rDocInfo.Producer.isEmpty() )
        PDFWriter::AppendUnicodeTextString(i_rDocInfo.Producer, aID);

    TimeValue aTVal, aGMT;
    oslDateTime aDT;
    osl_getSystemTime( &aGMT );
    osl_getLocalTimeFromSystemTime( &aGMT, &aTVal );
    osl_getDateTimeFromTimeValue( &aTVal, &aDT );
    OStringBuffer aCreationMetaDateString(64);

    // i59651: we fill the Metadata date string as well, if PDF/A is requested
    // according to ISO 19005-1:2005 6.7.3 the date is corrected for
    // local time zone offset UTC only, whereas Acrobat 8 seems
    // to use the localtime notation only
    // according to a recommendation in XMP Specification (Jan 2004, page 75)
    // the Acrobat way seems the right approach
    aCreationMetaDateString.append( static_cast<sal_Char>('0' + ((aDT.Year/1000)%10)) );
    aCreationMetaDateString.append( static_cast<sal_Char>('0' + ((aDT.Year/100)%10)) );
    aCreationMetaDateString.append( static_cast<sal_Char>('0' + ((aDT.Year/10)%10)) );
    aCreationMetaDateString.append( static_cast<sal_Char>('0' + ((aDT.Year)%10)) );
    aCreationMetaDateString.append( "-" );
    aCreationMetaDateString.append( static_cast<sal_Char>('0' + ((aDT.Month/10)%10)) );
    aCreationMetaDateString.append( static_cast<sal_Char>('0' + ((aDT.Month)%10)) );
    aCreationMetaDateString.append( "-" );
    aCreationMetaDateString.append( static_cast<sal_Char>('0' + ((aDT.Day/10)%10)) );
    aCreationMetaDateString.append( static_cast<sal_Char>('0' + ((aDT.Day)%10)) );
    aCreationMetaDateString.append( "T" );
    aCreationMetaDateString.append( static_cast<sal_Char>('0' + ((aDT.Hours/10)%10)) );
    aCreationMetaDateString.append( static_cast<sal_Char>('0' + ((aDT.Hours)%10)) );
    aCreationMetaDateString.append( ":" );
    aCreationMetaDateString.append( static_cast<sal_Char>('0' + ((aDT.Minutes/10)%10)) );
    aCreationMetaDateString.append( static_cast<sal_Char>('0' + ((aDT.Minutes)%10)) );
    aCreationMetaDateString.append( ":" );
    aCreationMetaDateString.append( static_cast<sal_Char>('0' + ((aDT.Seconds/10)%10)) );
    aCreationMetaDateString.append( static_cast<sal_Char>('0' + ((aDT.Seconds)%10)) );

    sal_uInt32 nDelta = 0;
    if( aGMT.Seconds > aTVal.Seconds )
    {
        nDelta = aGMT.Seconds-aTVal.Seconds;
        aCreationMetaDateString.append( "-" );
    }
    else if( aGMT.Seconds < aTVal.Seconds )
    {
        nDelta = aTVal.Seconds-aGMT.Seconds;
        aCreationMetaDateString.append( "+" );
    }
    else
    {
        aCreationMetaDateString.append( "Z" );

    }
    if( nDelta )
    {
        aCreationMetaDateString.append( static_cast<sal_Char>('0' + ((nDelta/36000)%10)) );
        aCreationMetaDateString.append( static_cast<sal_Char>('0' + ((nDelta/3600)%10)) );
        aCreationMetaDateString.append( ":" );
        aCreationMetaDateString.append( static_cast<sal_Char>('0' + ((nDelta/600)%6)) );
        aCreationMetaDateString.append( static_cast<sal_Char>('0' + ((nDelta/60)%10)) );
    }
    aID.append( i_rCString1.getStr(), i_rCString1.getLength() );

    aInfoValuesOut = aID.makeStringAndClear();
    o_rCString2 = aCreationMetaDateString.makeStringAndClear();

    ::comphelper::Hash aDigest(::comphelper::HashType::MD5);
    aDigest.update(reinterpret_cast<unsigned char const*>(&aGMT), sizeof(aGMT));
    aDigest.update(reinterpret_cast<unsigned char const*>(aInfoValuesOut.getStr()), aInfoValuesOut.getLength());
    //the binary form of the doc id is needed for encryption stuff
    o_rIdentifier = aDigest.finalize();
}

/* i12626 methods */
/*
check if the Unicode string must be encrypted or not, perform the requested task,
append the string as unicode hex, encrypted if needed
 */
inline void PDFWriterImpl::appendUnicodeTextStringEncrypt( const OUString& rInString, const sal_Int32 nInObjectNumber, OStringBuffer& rOutBuffer )
{
    rOutBuffer.append( "<" );
    if( m_aContext.Encryption.Encrypt() )
    {
        const sal_Unicode* pStr = rInString.getStr();
        sal_Int32 nLen = rInString.getLength();
        //prepare a unicode string, encrypt it
        if( checkEncryptionBufferSize( nLen*2 ) )
        {
            enableStringEncryption( nInObjectNumber );
            sal_uInt8 *pCopy = m_pEncryptionBuffer;
            sal_Int32 nChars = 2;
            *pCopy++ = 0xFE;
            *pCopy++ = 0xFF;
            // we need to prepare a byte stream from the unicode string buffer
            for( int i = 0; i < nLen; i++ )
            {
                sal_Unicode aUnChar = pStr[i];
                *pCopy++ = static_cast<sal_uInt8>( aUnChar >> 8 );
                *pCopy++ = static_cast<sal_uInt8>( aUnChar & 255 );
                nChars += 2;
            }
            //encrypt in place
            rtl_cipher_encodeARCFOUR( m_aCipher, m_pEncryptionBuffer, nChars, m_pEncryptionBuffer, nChars );
            //now append, hexadecimal (appendHex), the encrypted result
            for(int i = 0; i < nChars; i++)
                appendHex( m_pEncryptionBuffer[i], rOutBuffer );
        }
    }
    else
        PDFWriter::AppendUnicodeTextString(rInString, rOutBuffer);
    rOutBuffer.append( ">" );
}

inline void PDFWriterImpl::appendLiteralStringEncrypt( OStringBuffer const & rInString, const sal_Int32 nInObjectNumber, OStringBuffer& rOutBuffer )
{
    rOutBuffer.append( "(" );
    sal_Int32 nChars = rInString.getLength();
    //check for encryption, if ok, encrypt the string, then convert with appndLiteralString
    if( m_aContext.Encryption.Encrypt() && checkEncryptionBufferSize( nChars ) )
    {
        //encrypt the string in a buffer, then append it
        enableStringEncryption( nInObjectNumber );
        rtl_cipher_encodeARCFOUR( m_aCipher, rInString.getStr(), nChars, m_pEncryptionBuffer, nChars );
        appendLiteralString( reinterpret_cast<sal_Char*>(m_pEncryptionBuffer), nChars, rOutBuffer );
    }
    else
        appendLiteralString( rInString.getStr(), nChars , rOutBuffer );
    rOutBuffer.append( ")" );
}

inline void PDFWriterImpl::appendLiteralStringEncrypt( const OString& rInString, const sal_Int32 nInObjectNumber, OStringBuffer& rOutBuffer )
{
    OStringBuffer aBufferString( rInString );
    appendLiteralStringEncrypt( aBufferString, nInObjectNumber, rOutBuffer);
}

void PDFWriterImpl::appendLiteralStringEncrypt( const OUString& rInString, const sal_Int32 nInObjectNumber, OStringBuffer& rOutBuffer, rtl_TextEncoding nEnc )
{
    OString aBufferString( OUStringToOString( rInString, nEnc ) );
    sal_Int32 nLen = aBufferString.getLength();
    OStringBuffer aBuf( nLen );
    const sal_Char* pT = aBufferString.getStr();

    for( sal_Int32 i = 0; i < nLen; i++, pT++ )
    {
        if( (*pT & 0x80) == 0 )
            aBuf.append( *pT );
        else
        {
            aBuf.append( '<' );
            appendHex( *pT, aBuf );
            aBuf.append( '>' );
        }
    }
    aBufferString = aBuf.makeStringAndClear();
    appendLiteralStringEncrypt( aBufferString, nInObjectNumber, rOutBuffer);
}

/* end i12626 methods */

void PDFWriterImpl::emitComment( const char* pComment )
{
    OStringBuffer aLine( 64 );
    aLine.append( "% " );
    aLine.append( pComment );
    aLine.append( "\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

bool PDFWriterImpl::compressStream( SvMemoryStream* pStream )
{
    if (!g_bDebugDisableCompression)
    {
        pStream->Seek( STREAM_SEEK_TO_END );
        sal_uLong nEndPos = pStream->Tell();
        pStream->Seek( STREAM_SEEK_TO_BEGIN );
        ZCodec aCodec( 0x4000, 0x4000 );
        SvMemoryStream aStream;
        aCodec.BeginCompression();
        aCodec.Write( aStream, static_cast<const sal_uInt8*>(pStream->GetData()), nEndPos );
        aCodec.EndCompression();
        nEndPos = aStream.Tell();
        pStream->Seek( STREAM_SEEK_TO_BEGIN );
        aStream.Seek( STREAM_SEEK_TO_BEGIN );
        pStream->SetStreamSize( nEndPos );
        pStream->WriteBytes( aStream.GetData(), nEndPos );
        return true;
    }
    else
        return false;
}

void PDFWriterImpl::beginCompression()
{
    if (!g_bDebugDisableCompression)
    {
        m_pCodec = o3tl::make_unique<ZCodec>( 0x4000, 0x4000 );
        m_pMemStream = o3tl::make_unique<SvMemoryStream>();
        m_pCodec->BeginCompression();
    }
}

void PDFWriterImpl::endCompression()
{
    if (!g_bDebugDisableCompression && m_pCodec)
    {
        m_pCodec->EndCompression();
        m_pCodec.reset();
        sal_uInt64 nLen = m_pMemStream->Tell();
        m_pMemStream->Seek( 0 );
        writeBuffer( m_pMemStream->GetData(), nLen );
        m_pMemStream.reset();
    }
}

bool PDFWriterImpl::writeBuffer( const void* pBuffer, sal_uInt64 nBytes )
{
    if( ! m_bOpen ) // we are already down the drain
        return false;

    if( ! nBytes ) // huh ?
        return true;

    if( !m_aOutputStreams.empty() )
    {
        m_aOutputStreams.front().m_pStream->Seek( STREAM_SEEK_TO_END );
        m_aOutputStreams.front().m_pStream->WriteBytes(
                pBuffer, sal::static_int_cast<std::size_t>(nBytes));
        return true;
    }

    sal_uInt64 nWritten;
    if( m_pCodec )
    {
        m_pCodec->Write( *m_pMemStream, static_cast<const sal_uInt8*>(pBuffer), static_cast<sal_uLong>(nBytes) );
        nWritten = nBytes;
    }
    else
    {
        bool  buffOK = true;
        if( m_bEncryptThisStream )
        {
            /* implement the encryption part of the PDF spec encryption algorithm 3.1 */
            buffOK = checkEncryptionBufferSize( static_cast<sal_Int32>(nBytes) );
            if( buffOK )
                rtl_cipher_encodeARCFOUR( m_aCipher,
                                          pBuffer, static_cast<sal_Size>(nBytes),
                                          m_pEncryptionBuffer, static_cast<sal_Size>(nBytes) );
        }

        const void* pWriteBuffer = ( m_bEncryptThisStream && buffOK ) ? m_pEncryptionBuffer  : pBuffer;
        m_DocDigest.update(static_cast<unsigned char const*>(pWriteBuffer), static_cast<sal_uInt32>(nBytes));

        if (m_aFile.write(pWriteBuffer, nBytes, nWritten) != osl::File::E_None)
            nWritten = 0;

        if( nWritten != nBytes )
        {
            m_aFile.close();
            m_bOpen = false;
        }
    }

    return nWritten == nBytes;
}

OutputDevice* PDFWriterImpl::getReferenceDevice()
{
    if( ! m_pReferenceDevice )
    {
        VclPtrInstance<VirtualDevice> pVDev(DeviceFormat::DEFAULT);

        m_pReferenceDevice = pVDev;

        if( m_aContext.DPIx == 0 || m_aContext.DPIy == 0 )
            pVDev->SetReferenceDevice( VirtualDevice::RefDevMode::PDF1 );
        else
            pVDev->SetReferenceDevice( m_aContext.DPIx, m_aContext.DPIy );

        pVDev->SetOutputSizePixel( Size( 640, 480 ) );
        pVDev->SetMapMode(MapMode(MapUnit::MapMM));

        m_pReferenceDevice->mpPDFWriter = this;
        m_pReferenceDevice->ImplUpdateFontData();
    }
    return m_pReferenceDevice;
}

static FontAttributes GetDevFontAttributes( const PDFWriterImpl::BuiltinFont& rBuiltin )
{
    FontAttributes aDFA;
    aDFA.SetFamilyName( OUString::createFromAscii( rBuiltin.m_pName ) );
    aDFA.SetStyleName( OUString::createFromAscii( rBuiltin.m_pStyleName ) );
    aDFA.SetFamilyType( rBuiltin.m_eFamily );
    aDFA.SetSymbolFlag( rBuiltin.m_eCharSet != RTL_TEXTENCODING_MS_1252 );
    aDFA.SetPitch( rBuiltin.m_ePitch );
    aDFA.SetWeight( rBuiltin.m_eWeight );
    aDFA.SetItalic( rBuiltin.m_eItalic );
    aDFA.SetWidthType( rBuiltin.m_eWidthType );

    aDFA.SetQuality( 50000 );
    return aDFA;
}

PdfBuiltinFontFace::PdfBuiltinFontFace( const PDFWriterImpl::BuiltinFont& rBuiltin )
:   PhysicalFontFace( GetDevFontAttributes(rBuiltin) ),
    mrBuiltin( rBuiltin )
{}

void PDFWriterImpl::newPage( double nPageWidth, double nPageHeight, PDFWriter::Orientation eOrientation )
{
    endPage();
    m_nCurrentPage = m_aPages.size();
    m_aPages.emplace_back(this, nPageWidth, nPageHeight, eOrientation );
    m_aPages.back().beginStream();

    // setup global graphics state
    // linewidth is "1 pixel" by default
    OStringBuffer aBuf( 16 );
    appendDouble( 72.0/double(getReferenceDevice()->GetDPIX()), aBuf );
    aBuf.append( " w\n" );
    writeBuffer( aBuf.getStr(), aBuf.getLength() );
}

void PDFWriterImpl::endPage()
{
    if( m_aPages.empty() )
        return;

    // close eventual MC sequence
    endStructureElementMCSeq();

    // sanity check
    if( !m_aOutputStreams.empty() )
    {
        OSL_FAIL( "redirection across pages !!!" );
        m_aOutputStreams.clear(); // leak !
        m_aMapMode.SetOrigin( Point() );
    }

    m_aGraphicsStack.clear();
    m_aGraphicsStack.emplace_back( );

    // this should pop the PDF graphics stack if necessary
    updateGraphicsState();

    m_aPages.back().endStream();

    // reset the default font
    Font aFont;
    aFont.SetFamilyName( "Times" );
    aFont.SetFontSize( Size( 0, 12 ) );

    m_aCurrentPDFState = m_aGraphicsStack.front();
    m_aGraphicsStack.front().m_aFont =  aFont;

    for (auto & bitmap : m_aBitmaps)
    {
        if( ! bitmap.m_aBitmap.IsEmpty() )
        {
            writeBitmapObject(bitmap);
            bitmap.m_aBitmap = BitmapEx();
        }
    }
    for (auto & jpeg : m_aJPGs)
    {
        if( jpeg.m_pStream )
        {
            writeJPG( jpeg );
            jpeg.m_pStream.reset();
            jpeg.m_aMask = Bitmap();
        }
    }
    for (auto & item : m_aTransparentObjects)
    {
        if( item.m_pContentStream )
        {
            writeTransparentObject(item);
            item.m_pContentStream.reset();
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
    osl::File::RC aError = m_aFile.getPos(nOffset);
    SAL_WARN_IF( aError != osl::File::E_None, "vcl.pdfwriter", "could not register object" );
    if (aError != osl::File::E_None)
    {
        m_aFile.close();
        m_bOpen = false;
    }
    m_aObjects[ n-1 ] = nOffset;
    return aError == osl::File::E_None;
}

#define CHECK_RETURN( x ) if( !(x) ) return 0
#define CHECK_RETURN2( x ) if( !(x) ) return

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

    if( it == aAttributeStrings.end() )
        SAL_INFO("vcl.pdfwriter", "invalid PDFWriter::StructAttribute " << eAttr);

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

    if( it == aValueStrings.end() )
        SAL_INFO("vcl.pdfwriter", "invalid PDFWriter::StructAttributeValue " << eVal);

    return it != aValueStrings.end() ? it->second : "";
}

static void appendStructureAttributeLine( PDFWriter::StructAttribute i_eAttr, const PDFWriterImpl::PDFStructureAttribute& i_rVal, OStringBuffer& o_rLine, bool i_bIsFixedInt )
{
    o_rLine.append( "/" );
    o_rLine.append( PDFWriterImpl::getAttributeTag( i_eAttr ) );

    if( i_rVal.eValue != PDFWriter::Invalid )
    {
        o_rLine.append( "/" );
        o_rLine.append( PDFWriterImpl::getAttributeValueTag( i_rVal.eValue ) );
    }
    else
    {
        // numerical value
        o_rLine.append( " " );
        if( i_bIsFixedInt )
            appendFixedInt( i_rVal.nValue, o_rLine );
        else
            o_rLine.append( i_rVal.nValue );
    }
    o_rLine.append( "\n" );
}

OString PDFWriterImpl::emitStructureAttributes( PDFStructureElement& i_rEle )
{
    // create layout, list and table attribute sets
    OStringBuffer aLayout(256), aList(64), aTable(64);
    for (auto const& attribute : i_rEle.m_aAttributes)
    {
        if( attribute.first == PDFWriter::ListNumbering )
            appendStructureAttributeLine( attribute.first, attribute.second, aList, true );
        else if( attribute.first == PDFWriter::RowSpan ||
                 attribute.first == PDFWriter::ColSpan )
            appendStructureAttributeLine( attribute.first, attribute.second, aTable, false );
        else if( attribute.first == PDFWriter::LinkAnnotation )
        {
            sal_Int32 nLink = attribute.second.nValue;
            std::map< sal_Int32, sal_Int32 >::const_iterator link_it =
                m_aLinkPropertyMap.find( nLink );
            if( link_it != m_aLinkPropertyMap.end() )
                nLink = link_it->second;
            if( nLink >= 0 && nLink < static_cast<sal_Int32>(m_aLinks.size()) )
            {
                // update struct parent of link
                OStringBuffer aStructParentEntry( 32 );
                aStructParentEntry.append( i_rEle.m_nObject );
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
                if (updateObject(nRefObject))
                {
                    writeBuffer( aRef.getStr(), aRef.getLength() );
                }

                i_rEle.m_aKids.emplace_back( nRefObject );
            }
            else
            {
                OSL_FAIL( "unresolved link id for Link structure" );
                SAL_INFO("vcl.pdfwriter", "unresolved link id " << nLink << " for Link structure");
                if (g_bDebugDisableCompression)
                {
                    OStringBuffer aLine( "unresolved link id " );
                    aLine.append( nLink );
                    aLine.append( " for Link structure" );
                    emitComment( aLine.getStr() );
                }
            }
        }
        else
            appendStructureAttributeLine( attribute.first, attribute.second, aLayout, true );
    }
    if( ! i_rEle.m_aBBox.IsEmpty() )
    {
        aLayout.append( "/BBox[" );
        appendFixedInt( i_rEle.m_aBBox.Left(), aLayout );
        aLayout.append( " " );
        appendFixedInt( i_rEle.m_aBBox.Top(), aLayout );
        aLayout.append( " " );
        appendFixedInt( i_rEle.m_aBBox.Right(), aLayout );
        aLayout.append( " " );
        appendFixedInt( i_rEle.m_aBBox.Bottom(), aLayout );
        aLayout.append( "]\n" );
    }

    std::vector< sal_Int32 > aAttribObjects;
    if( !aLayout.isEmpty() )
    {
        aAttribObjects.push_back( createObject() );
        if (updateObject( aAttribObjects.back() ))
        {
            OStringBuffer aObj( 64 );
            aObj.append( aAttribObjects.back() );
            aObj.append( " 0 obj\n"
                         "<</O/Layout\n" );
            aLayout.append( ">>\nendobj\n\n" );
            writeBuffer( aObj.getStr(), aObj.getLength() );
            writeBuffer( aLayout.getStr(), aLayout.getLength() );
        }
    }
    if( !aList.isEmpty() )
    {
        aAttribObjects.push_back( createObject() );
        if (updateObject( aAttribObjects.back() ))
        {
            OStringBuffer aObj( 64 );
            aObj.append( aAttribObjects.back() );
            aObj.append( " 0 obj\n"
                         "<</O/List\n" );
            aList.append( ">>\nendobj\n\n" );
            writeBuffer( aObj.getStr(), aObj.getLength() );
            writeBuffer( aList.getStr(), aList.getLength() );
        }
    }
    if( !aTable.isEmpty() )
    {
        aAttribObjects.push_back( createObject() );
        if (updateObject( aAttribObjects.back() ))
        {
            OStringBuffer aObj( 64 );
            aObj.append( aAttribObjects.back() );
            aObj.append( " 0 obj\n"
                         "<</O/Table\n" );
            aTable.append( ">>\nendobj\n\n" );
            writeBuffer( aObj.getStr(), aObj.getLength() );
            writeBuffer( aTable.getStr(), aTable.getLength() );
        }
    }

    OStringBuffer aRet( 64 );
    if( aAttribObjects.size() > 1 )
        aRet.append( " [" );
    for (auto const& attrib : aAttribObjects)
    {
        aRet.append( " " );
        aRet.append( attrib );
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

    for (auto const& child : rEle.m_aChildren)
    {
        if( child > 0 && child < sal_Int32(m_aStructure.size()) )
        {
            PDFStructureElement& rChild = m_aStructure[ child ];
            if( rChild.m_eType != PDFWriter::NonStructElement )
            {
                if( rChild.m_nParentElement == rEle.m_nOwnElement )
                    emitStructure( rChild );
                else
                {
                    OSL_FAIL( "PDFWriterImpl::emitStructure: invalid child structure element" );
                    SAL_INFO("vcl.pdfwriter", "PDFWriterImpl::emitStructure: invalid child structure element with id " << child);
                }
            }
        }
        else
        {
            OSL_FAIL( "PDFWriterImpl::emitStructure: invalid child structure id" );
            SAL_INFO("vcl.pdfwriter", "PDFWriterImpl::emitStructure: invalid child structure id " << child);
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
        if( ! m_aRoleMap.empty() )
        {
            aLine.append( "/RoleMap<<" );
            for (auto const& role : m_aRoleMap)
            {
                aLine.append( '/' );
                aLine.append(role.first);
                aLine.append( '/' );
                aLine.append( role.second );
                aLine.append( '\n' );
            }
            aLine.append( ">>\n" );
        }
    }
    else
    {
        aLine.append( "/StructElem\n"
                      "/S/" );
        if( !rEle.m_aAlias.isEmpty() )
            aLine.append( rEle.m_aAlias );
        else
            aLine.append( getStructureTag( rEle.m_eType ) );
        aLine.append( "\n"
                      "/P " );
        aLine.append( m_aStructure[ rEle.m_nParentElement ].m_nObject );
        aLine.append( " 0 R\n"
                      "/Pg " );
        aLine.append( rEle.m_nFirstPageObject );
        aLine.append( " 0 R\n" );
        if( !rEle.m_aActualText.isEmpty() )
        {
            aLine.append( "/ActualText" );
            appendUnicodeTextStringEncrypt( rEle.m_aActualText, rEle.m_nObject, aLine );
            aLine.append( "\n" );
        }
        if( !rEle.m_aAltText.isEmpty() )
        {
            aLine.append( "/Alt" );
            appendUnicodeTextStringEncrypt( rEle.m_aAltText, rEle.m_nObject, aLine );
            aLine.append( "\n" );
        }
    }
    if( (! rEle.m_aBBox.IsEmpty()) || (! rEle.m_aAttributes.empty()) )
    {
        OString aAttribs =  emitStructureAttributes( rEle );
        if( !aAttribs.isEmpty() )
        {
            aLine.append( "/A" );
            aLine.append( aAttribs );
            aLine.append( "\n" );
        }
    }
    if( !rEle.m_aLocale.Language.isEmpty() )
    {
        /* PDF allows only RFC 3066, which is only partly BCP 47 and does not
         * include script tags and others.
         * http://pdf.editme.com/pdfua-naturalLanguageSpecification
         * http://partners.adobe.com/public/developer/en/pdf/PDFReference16.pdf#page=886
         * https://www.adobe.com/content/dam/Adobe/en/devnet/pdf/pdfs/PDF32000_2008.pdf#M13.9.19332.1Heading.97.Natural.Language.Specification
         * */
        LanguageTag aLanguageTag( rEle.m_aLocale);
        OUString aLanguage, aScript, aCountry;
        aLanguageTag.getIsoLanguageScriptCountry( aLanguage, aScript, aCountry);
        if (!aLanguage.isEmpty())
        {
            OUStringBuffer aLocBuf( 16 );
            aLocBuf.append( aLanguage );
            if( !aCountry.isEmpty() )
            {
                aLocBuf.append( '-' );
                aLocBuf.append( aCountry );
            }
            aLine.append( "/Lang" );
            appendLiteralStringEncrypt( aLocBuf.makeStringAndClear(), rEle.m_nObject, aLine );
            aLine.append( "\n" );
        }
    }
    if( ! rEle.m_aKids.empty() )
    {
        unsigned int i = 0;
        aLine.append( "/K[" );
        for (auto const& kid : rEle.m_aKids)
        {
            if( kid.nMCID == -1 )
            {
                aLine.append( kid.nObject );
                aLine.append( " 0 R" );
                aLine.append( ( (i & 15) == 15 ) ? "\n" : " " );
            }
            else
            {
                if( kid.nObject == rEle.m_nFirstPageObject )
                {
                    aLine.append( kid.nMCID );
                    aLine.append( " " );
                }
                else
                {
                    aLine.append( "<</Type/MCR/Pg " );
                    aLine.append( kid.nObject );
                    aLine.append( " 0 R /MCID " );
                    aLine.append( kid.nMCID );
                    aLine.append( ">>\n" );
                }
            }
            ++i;
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
    for (auto const& gradient : m_aGradients)
    {
        if ( !writeGradientFunction( gradient ) ) return false;
    }
    return true;
}

bool PDFWriterImpl::emitTilings()
{
    OStringBuffer aTilingObj( 1024 );

    for (auto & tiling : m_aTilings)
    {
        SAL_WARN_IF( !tiling.m_pTilingStream, "vcl.pdfwriter", "tiling without stream" );
        if( ! tiling.m_pTilingStream )
            continue;

        aTilingObj.setLength( 0 );

        if (g_bDebugDisableCompression)
        {
            emitComment( "PDFWriterImpl::emitTilings" );
        }

        sal_Int32 nX = static_cast<sal_Int32>(tiling.m_aRectangle.Left());
        sal_Int32 nY = static_cast<sal_Int32>(tiling.m_aRectangle.Top());
        sal_Int32 nW = static_cast<sal_Int32>(tiling.m_aRectangle.GetWidth());
        sal_Int32 nH = static_cast<sal_Int32>(tiling.m_aRectangle.GetHeight());
        if( tiling.m_aCellSize.Width() == 0 )
            tiling.m_aCellSize.setWidth( nW );
        if( tiling.m_aCellSize.Height() == 0 )
            tiling.m_aCellSize.setHeight( nH );

        bool bDeflate = compressStream( tiling.m_pTilingStream );
        tiling.m_pTilingStream->Seek( STREAM_SEEK_TO_END );
        sal_uInt64 const nTilingStreamSize = tiling.m_pTilingStream->Tell();
        tiling.m_pTilingStream->Seek( STREAM_SEEK_TO_BEGIN );

        // write pattern object
        aTilingObj.append( tiling.m_nObject );
        aTilingObj.append( " 0 obj\n" );
        aTilingObj.append( "<</Type/Pattern/PatternType 1\n"
                           "/PaintType 1\n"
                           "/TilingType 2\n"
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
        appendFixedInt( tiling.m_aCellSize.Width(), aTilingObj );
        aTilingObj.append( "\n"
                           "/YStep " );
        appendFixedInt( tiling.m_aCellSize.Height(), aTilingObj );
        aTilingObj.append( "\n" );
        if( tiling.m_aTransform.matrix[0] != 1.0 ||
            tiling.m_aTransform.matrix[1] != 0.0 ||
            tiling.m_aTransform.matrix[3] != 0.0 ||
            tiling.m_aTransform.matrix[4] != 1.0 ||
            tiling.m_aTransform.matrix[2] != 0.0 ||
            tiling.m_aTransform.matrix[5] != 0.0 )
        {
            aTilingObj.append( "/Matrix [" );
            // TODO: scaling, mirroring on y, etc
            appendDouble( tiling.m_aTransform.matrix[0], aTilingObj );
            aTilingObj.append( ' ' );
            appendDouble( tiling.m_aTransform.matrix[1], aTilingObj );
            aTilingObj.append( ' ' );
            appendDouble( tiling.m_aTransform.matrix[3], aTilingObj );
            aTilingObj.append( ' ' );
            appendDouble( tiling.m_aTransform.matrix[4], aTilingObj );
            aTilingObj.append( ' ' );
            appendDouble( tiling.m_aTransform.matrix[2], aTilingObj );
            aTilingObj.append( ' ' );
            appendDouble( tiling.m_aTransform.matrix[5], aTilingObj );
            aTilingObj.append( "]\n" );
        }
        aTilingObj.append( "/Resources" );
        tiling.m_aResources.append( aTilingObj, getFontDictObject() );
        if( bDeflate )
            aTilingObj.append( "/Filter/FlateDecode" );
        aTilingObj.append( "/Length " );
        aTilingObj.append( static_cast<sal_Int32>(nTilingStreamSize) );
        aTilingObj.append( ">>\nstream\n" );
        if ( !updateObject( tiling.m_nObject ) ) return false;
        if ( !writeBuffer( aTilingObj.getStr(), aTilingObj.getLength() ) ) return false;
        checkAndEnableStreamEncryption( tiling.m_nObject );
        bool written = writeBuffer( tiling.m_pTilingStream->GetData(), nTilingStreamSize );
        delete tiling.m_pTilingStream;
        tiling.m_pTilingStream = nullptr;
        if( !written )
            return false;
        disableStreamEncryption();
        aTilingObj.setLength( 0 );
        aTilingObj.append( "\nendstream\nendobj\n\n" );
        if ( !writeBuffer( aTilingObj.getStr(), aTilingObj.getLength() ) ) return false;
    }
    return true;
}

sal_Int32 PDFWriterImpl::emitBuiltinFont( const PdfBuiltinFontFace* pFD, sal_Int32 nFontObject )
{
    if( !pFD )
        return 0;
    const BuiltinFont& rBuiltinFont = pFD->GetBuiltinFont();

    OStringBuffer aLine( 1024 );

    if( nFontObject <= 0 )
        nFontObject = createObject();
    CHECK_RETURN( updateObject( nFontObject ) );
    aLine.append( nFontObject );
    aLine.append( " 0 obj\n"
                  "<</Type/Font/Subtype/Type1/BaseFont/" );
    appendName( rBuiltinFont.m_pPSName, aLine );
    aLine.append( "\n" );
    if( rBuiltinFont.m_eCharSet == RTL_TEXTENCODING_MS_1252 )
         aLine.append( "/Encoding/WinAnsiEncoding\n" );
    aLine.append( ">>\nendobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    return nFontObject;
}

std::map< sal_Int32, sal_Int32 > PDFWriterImpl::emitSystemFont( const PhysicalFontFace* pFont, EmbedFont const & rEmbed )
{
    std::map< sal_Int32, sal_Int32 > aRet;

    sal_Int32 nFontDescriptor = 0;
    OString aSubType( "/Type1" );
    FontSubsetInfo aInfo;
    // fill in dummy values
    aInfo.m_nAscent = 1000;
    aInfo.m_nDescent = 200;
    aInfo.m_nCapHeight = 1000;
    aInfo.m_aFontBBox = tools::Rectangle( Point( -200, -200 ), Size( 1700, 1700 ) );
    aInfo.m_aPSName = pFont->GetFamilyName();
    sal_Int32 pWidths[256];
    memset( pWidths, 0, sizeof(pWidths) );

    SalGraphics *pGraphics = m_pReferenceDevice->GetGraphics();

    assert(pGraphics);

    aSubType = OString( "/TrueType" );
    std::vector< sal_Int32 > aGlyphWidths;
    Ucs2UIntMap aUnicodeMap;
    pGraphics->GetGlyphWidths( pFont, false, aGlyphWidths, aUnicodeMap );

    OUString aTmpName;
    osl_createTempFile( nullptr, nullptr, &aTmpName.pData );
    sal_GlyphId aGlyphIds[ 256 ];
    sal_uInt8 pEncoding[ 256 ];
    sal_Int32 pDuWidths[ 256 ];

    memset( aGlyphIds, 0, sizeof( aGlyphIds ) );
    memset( pEncoding, 0, sizeof( pEncoding ) );
    memset( pDuWidths, 0, sizeof( pDuWidths ) );

    for( sal_Ucs c = 32; c < 256; c++ )
    {
        pEncoding[c] = c;
        aGlyphIds[c] = 0;
        if( aUnicodeMap.find( c ) != aUnicodeMap.end() )
            pWidths[ c ] = aGlyphWidths[ aUnicodeMap[ c ] ];
    }
    //TODO: surely this is utterly broken because aGlyphIds is just all zeros, if we
    //had the right glyphids here then I imagine we could replace pDuWidths with
    //pWidths and remove pWidths assignment above. i.e. start with the glyph ids
    //and map those to unicode rather than try and reverse map them ?
    pGraphics->CreateFontSubset( aTmpName, pFont, aGlyphIds, pEncoding, pDuWidths, 256, aInfo );
    osl_removeFile( aTmpName.pData );

    // write font descriptor
    nFontDescriptor = emitFontDescriptor( pFont, aInfo, 0, 0 );
    if( nFontDescriptor )
    {
        // write font object
        sal_Int32 nObject = createObject();
        if( updateObject( nObject ) )
        {
            OStringBuffer aLine( 1024 );
            aLine.append( nObject );
            aLine.append( " 0 obj\n"
                          "<</Type/Font/Subtype" );
            aLine.append( aSubType );
            aLine.append( "/BaseFont/" );
            appendName( aInfo.m_aPSName, aLine );
            aLine.append( "\n" );
            if( !pFont->IsSymbolFont() )
                aLine.append( "/Encoding/WinAnsiEncoding\n" );
            aLine.append( "/FirstChar 32 /LastChar 255\n"
                          "/Widths[" );
            for( int i = 32; i < 256; i++ )
            {
                aLine.append( pWidths[i] );
                aLine.append( ((i&15) == 15) ? "\n" : " " );
            }
            aLine.append( "]\n"
                          "/FontDescriptor " );
            aLine.append( nFontDescriptor );
            aLine.append( " 0 R>>\n"
                          "endobj\n\n" );
            writeBuffer( aLine.getStr(), aLine.getLength() );

            aRet[ rEmbed.m_nNormalFontID ] = nObject;
        }
    }

    return aRet;
}

typedef int ThreeInts[3];
static bool getPfbSegmentLengths( const unsigned char* pFontBytes, int nByteLen,
    ThreeInts& rSegmentLengths )
{
    if( !pFontBytes || (nByteLen < 0) )
        return false;
    const unsigned char* pPtr = pFontBytes;
    const unsigned char* pEnd = pFontBytes + nByteLen;

    for(int & rSegmentLength : rSegmentLengths) {
        // read segment1 header
        if( pPtr+6 >= pEnd )
            return false;
        if( (pPtr[0] != 0x80) || (pPtr[1] >= 0x03) )
            return false;
        const int nLen = (pPtr[5]<<24) + (pPtr[4]<<16) + (pPtr[3]<<8) + pPtr[2];
        if( nLen <= 0)
            return false;
        rSegmentLength = nLen;
        pPtr += nLen + 6;
    }

    // read segment-end header
    if( pPtr+2 >= pEnd )
        return false;
    if( (pPtr[0] != 0x80) || (pPtr[1] != 0x03) )
        return false;

    return true;
}

static void appendSubsetName( int nSubsetID, const OUString& rPSName, OStringBuffer& rBuffer )
{
    if( nSubsetID )
    {
        for( int i = 0; i < 6; i++ )
        {
            int nOffset = (nSubsetID % 26);
            nSubsetID /= 26;
            rBuffer.append( static_cast<sal_Char>('A'+nOffset) );
        }
        rBuffer.append( '+' );
    }
    appendName( rPSName, rBuffer );
}

sal_Int32 PDFWriterImpl::createToUnicodeCMap( sal_uInt8 const * pEncoding,
                                              const sal_Ucs* pCodeUnits,
                                              const sal_Int32* pCodeUnitsPerGlyph,
                                              const sal_Int32* pEncToUnicodeIndex,
                                              int nGlyphs )
{
    int nMapped = 0;
    for (int n = 0; n < nGlyphs; ++n)
        if( pCodeUnits[pEncToUnicodeIndex[n]] && pCodeUnitsPerGlyph[n] )
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
    for (int n = 0; n < nGlyphs; ++n)
    {
        if( pCodeUnits[pEncToUnicodeIndex[n]] && pCodeUnitsPerGlyph[n] )
        {
            if( (nCount % 100) == 0 )
            {
                if( nCount )
                    aContents.append( "endbfchar\n" );
                aContents.append( static_cast<sal_Int32>(std::min(nMapped-nCount, 100)) );
                aContents.append( " beginbfchar\n" );
            }
            aContents.append( '<' );
            appendHex( static_cast<sal_Int8>(pEncoding[n]), aContents );
            aContents.append( "> <" );
            // TODO: handle code points>U+FFFF
            sal_Int32 nIndex = pEncToUnicodeIndex[n];
            for( sal_Int32 j = 0; j < pCodeUnitsPerGlyph[n]; j++ )
            {
                appendHex( static_cast<sal_Int8>(pCodeUnits[nIndex + j] / 256), aContents );
                appendHex( static_cast<sal_Int8>(pCodeUnits[nIndex + j] & 255), aContents );
            }
            aContents.append( ">\n" );
            nCount++;
        }
    }
    aContents.append( "endbfchar\n"
                      "endcmap\n"
                      "CMapName currentdict /CMap defineresource pop\n"
                      "end\n"
                      "end\n" );
    SvMemoryStream aStream;
    if (!g_bDebugDisableCompression)
    {
        ZCodec aCodec( 0x4000, 0x4000 );
        aCodec.BeginCompression();
        aCodec.Write( aStream, reinterpret_cast<const sal_uInt8*>(aContents.getStr()), aContents.getLength() );
        aCodec.EndCompression();
    }

    if (g_bDebugDisableCompression)
    {
        emitComment( "PDFWriterImpl::createToUnicodeCMap" );
    }
    OStringBuffer aLine( 40 );

    aLine.append( nStream );
    aLine.append( " 0 obj\n<</Length " );
    sal_Int32 nLen = 0;
    if (!g_bDebugDisableCompression)
    {
        nLen = static_cast<sal_Int32>(aStream.Tell());
        aStream.Seek( 0 );
        aLine.append( nLen );
        aLine.append( "/Filter/FlateDecode" );
    }
    else
        aLine.append( aContents.getLength() );
    aLine.append( ">>\nstream\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    checkAndEnableStreamEncryption( nStream );
    if (!g_bDebugDisableCompression)
    {
        CHECK_RETURN( writeBuffer( aStream.GetData(), nLen ) );
    }
    else
    {
        CHECK_RETURN( writeBuffer( aContents.getStr(), aContents.getLength() ) );
    }
    disableStreamEncryption();
    aLine.setLength( 0 );
    aLine.append( "\nendstream\n"
                  "endobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    return nStream;
}

sal_Int32 PDFWriterImpl::emitFontDescriptor( const PhysicalFontFace* pFont, FontSubsetInfo const & rInfo, sal_Int32 nSubsetID, sal_Int32 nFontStream )
{
    OStringBuffer aLine( 1024 );
    // get font flags, see PDF reference 1.4 p. 358
    // possibly characters outside Adobe standard encoding
    // so set Symbolic flag
    sal_Int32 nFontFlags = (1<<2);
    if( pFont->GetItalic() == ITALIC_NORMAL || pFont->GetItalic() == ITALIC_OBLIQUE )
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
    aLine.append( static_cast<sal_Int32>(rInfo.m_aFontBBox.TopLeft().X()) );
    aLine.append( ' ' );
    aLine.append( static_cast<sal_Int32>(rInfo.m_aFontBBox.TopLeft().Y()) );
    aLine.append( ' ' );
    aLine.append( static_cast<sal_Int32>(rInfo.m_aFontBBox.BottomRight().X()) );
    aLine.append( ' ' );
    aLine.append( static_cast<sal_Int32>(rInfo.m_aFontBBox.BottomRight().Y()+1) );
    aLine.append( "]/ItalicAngle " );
    if( pFont->GetItalic() == ITALIC_OBLIQUE || pFont->GetItalic() == ITALIC_NORMAL )
        aLine.append( "-30" );
    else
        aLine.append( "0" );
    aLine.append( "\n"
                  "/Ascent " );
    aLine.append( static_cast<sal_Int32>(rInfo.m_nAscent) );
    aLine.append( "\n"
                  "/Descent " );
    aLine.append( static_cast<sal_Int32>(-rInfo.m_nDescent) );
    aLine.append( "\n"
                  "/CapHeight " );
    aLine.append( static_cast<sal_Int32>(rInfo.m_nCapHeight) );
    // According to PDF reference 1.4 StemV is required
    // seems a tad strange to me, but well ...
    aLine.append( "\n"
                  "/StemV 80\n" );
    if( nFontStream )
    {
        aLine.append( "/FontFile" );
        switch( rInfo.m_nFontType )
        {
            case FontType::SFNT_TTF:
                aLine.append( '2' );
                break;
            case FontType::TYPE1_PFA:
            case FontType::TYPE1_PFB:
            case FontType::ANY_TYPE1:
                break;
            default:
                OSL_FAIL( "unknown fonttype in PDF font descriptor" );
                return 0;
        }
        aLine.append( ' ' );
        aLine.append( nFontStream );
        aLine.append( " 0 R\n" );
    }
    aLine.append( ">>\n"
                  "endobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    return nFontDescriptor;
}

void PDFWriterImpl::appendBuiltinFontsToDict( OStringBuffer& rDict ) const
{
    for (auto const& item : m_aBuiltinFontToObjectMap)
    {
        rDict.append( m_aBuiltinFonts[item.first].getNameObject() );
        rDict.append( ' ' );
        rDict.append( item.second );
        rDict.append( " 0 R" );
    }
}

bool PDFWriterImpl::emitFonts()
{
    SalGraphics *pGraphics = m_pReferenceDevice->GetGraphics();

    if (!pGraphics)
        return false;

    OStringBuffer aLine( 1024 );

    std::map< sal_Int32, sal_Int32 > aFontIDToObject;

    OUString aTmpName;
    osl_createTempFile( nullptr, nullptr, &aTmpName.pData );
    for (auto & subset : m_aSubsets)
    {
        for (auto & s_subset :subset.second.m_aSubsets)
        {
            sal_GlyphId aGlyphIds[ 256 ];
            sal_Int32 pWidths[ 256 ];
            sal_uInt8 pEncoding[ 256 ];
            sal_Int32 pEncToUnicodeIndex[ 256 ];
            sal_Int32 pCodeUnitsPerGlyph[ 256 ];
            std::vector<sal_Ucs> aCodeUnits;
            aCodeUnits.reserve( 256 );
            int nGlyphs = 1;
            // fill arrays and prepare encoding index map
            sal_Int32 nToUnicodeStream = 0;

            memset( aGlyphIds, 0, sizeof( aGlyphIds ) );
            memset( pEncoding, 0, sizeof( pEncoding ) );
            memset( pCodeUnitsPerGlyph, 0, sizeof( pCodeUnitsPerGlyph ) );
            memset( pEncToUnicodeIndex, 0, sizeof( pEncToUnicodeIndex ) );
            for (auto const& item : s_subset.m_aMapping)
            {
                sal_uInt8 nEnc = item.second.getGlyphId();

                SAL_WARN_IF( aGlyphIds[nEnc] != 0 || pEncoding[nEnc] != 0, "vcl.pdfwriter", "duplicate glyph" );
                SAL_WARN_IF( nEnc > s_subset.m_aMapping.size(), "vcl.pdfwriter", "invalid glyph encoding" );

                aGlyphIds[ nEnc ] = item.first;
                pEncoding[ nEnc ] = nEnc;
                pEncToUnicodeIndex[ nEnc ] = static_cast<sal_Int32>(aCodeUnits.size());
                pCodeUnitsPerGlyph[ nEnc ] = item.second.countCodes();
                for( sal_Int32 n = 0; n < pCodeUnitsPerGlyph[ nEnc ]; n++ )
                    aCodeUnits.push_back( item.second.getCode( n ) );
                if( item.second.getCode(0) )
                    nToUnicodeStream = 1;
                if( nGlyphs < 256 )
                    nGlyphs++;
                else
                {
                    OSL_FAIL( "too many glyphs for subset" );
                }
            }
            FontSubsetInfo aSubsetInfo;
            if( pGraphics->CreateFontSubset( aTmpName, subset.first, aGlyphIds, pEncoding, pWidths, nGlyphs, aSubsetInfo ) )
            {
                // create font stream
                osl::File aFontFile(aTmpName);
                if (osl::File::E_None != aFontFile.open(osl_File_OpenFlag_Read)) return false;
                // get file size
                sal_uInt64 nLength1;
                if ( osl::File::E_None != aFontFile.setPos(osl_Pos_End, 0) ) return false;
                if ( osl::File::E_None != aFontFile.getPos(nLength1) ) return false;
                if ( osl::File::E_None != aFontFile.setPos(osl_Pos_Absolut, 0) ) return false;

                if (g_bDebugDisableCompression)
                {
                    emitComment( "PDFWriterImpl::emitFonts" );
                }
                sal_Int32 nFontStream = createObject();
                sal_Int32 nStreamLengthObject = createObject();
                if ( !updateObject( nFontStream ) ) return false;
                aLine.setLength( 0 );
                aLine.append( nFontStream );
                aLine.append( " 0 obj\n"
                             "<</Length " );
                aLine.append( nStreamLengthObject );
                if (!g_bDebugDisableCompression)
                    aLine.append( " 0 R"
                                 "/Filter/FlateDecode"
                                 "/Length1 " );
                else
                    aLine.append( " 0 R"
                                 "/Length1 " );

                sal_uInt64 nStartPos = 0;
                if( aSubsetInfo.m_nFontType == FontType::SFNT_TTF )
                {
                    aLine.append( static_cast<sal_Int32>(nLength1) );

                    aLine.append( ">>\n"
                                 "stream\n" );
                    if ( !writeBuffer( aLine.getStr(), aLine.getLength() ) ) return false;
                    if ( osl::File::E_None != m_aFile.getPos(nStartPos) ) return false;

                    // copy font file
                    beginCompression();
                    checkAndEnableStreamEncryption( nFontStream );
                    sal_Bool bEOF = false;
                    do
                    {
                        char buf[8192];
                        sal_uInt64 nRead;
                        if ( osl::File::E_None != aFontFile.read(buf, sizeof(buf), nRead) ) return false;
                        if ( !writeBuffer( buf, nRead ) ) return false;
                        if ( osl::File::E_None != aFontFile.isEndOfFile(&bEOF) ) return false;
                    } while( ! bEOF );
                }
                else if( aSubsetInfo.m_nFontType & FontType::CFF_FONT)
                {
                    // TODO: implement
                    OSL_FAIL( "PDFWriterImpl does not support CFF-font subsets yet!" );
                }
                else if( aSubsetInfo.m_nFontType & FontType::TYPE1_PFB) // TODO: also support PFA?
                {
                    std::unique_ptr<unsigned char[]> xBuffer(new unsigned char[nLength1]);

                    sal_uInt64 nBytesRead = 0;
                    if ( osl::File::E_None != aFontFile.read(xBuffer.get(), nLength1, nBytesRead) ) return false;
                    SAL_WARN_IF( nBytesRead!=nLength1, "vcl.pdfwriter", "PDF-FontSubset read incomplete!" );
                    if ( osl::File::E_None != aFontFile.setPos(osl_Pos_Absolut, 0) ) return false;
                    // get the PFB-segment lengths
                    ThreeInts aSegmentLengths = {0,0,0};
                    getPfbSegmentLengths(xBuffer.get(), static_cast<int>(nBytesRead), aSegmentLengths);
                    // the lengths below are mandatory for PDF-exported Type1 fonts
                    // because the PFB segment headers get stripped! WhyOhWhy.
                    aLine.append( static_cast<sal_Int32>(aSegmentLengths[0]) );
                    aLine.append( "/Length2 " );
                    aLine.append( static_cast<sal_Int32>(aSegmentLengths[1]) );
                    aLine.append( "/Length3 " );
                    aLine.append( static_cast<sal_Int32>(aSegmentLengths[2]) );

                    aLine.append( ">>\n"
                                 "stream\n" );
                    if ( !writeBuffer( aLine.getStr(), aLine.getLength() ) ) return false;
                    if ( osl::File::E_None != m_aFile.getPos(nStartPos) ) return false;

                    // emit PFB-sections without section headers
                    beginCompression();
                    checkAndEnableStreamEncryption( nFontStream );
                    if ( !writeBuffer( &xBuffer[6], aSegmentLengths[0] ) ) return false;
                    if ( !writeBuffer( &xBuffer[12] + aSegmentLengths[0], aSegmentLengths[1] ) ) return false;
                    if ( !writeBuffer( &xBuffer[18] + aSegmentLengths[0] + aSegmentLengths[1], aSegmentLengths[2] ) ) return false;
                }
                else
                {
                    SAL_INFO("vcl.pdfwriter", "PDF: CreateFontSubset result in not yet supported format=" << static_cast<int>(aSubsetInfo.m_nFontType));
                    aLine.append( "0 >>\nstream\n" );
                }

                endCompression();
                disableStreamEncryption();
                // close the file
                aFontFile.close();

                sal_uInt64 nEndPos = 0;
                if ( osl::File::E_None != m_aFile.getPos(nEndPos) ) return false;
                // end the stream
                aLine.setLength( 0 );
                aLine.append( "\nendstream\nendobj\n\n" );
                if ( !writeBuffer( aLine.getStr(), aLine.getLength() ) ) return false;

                // emit stream length object
                if ( !updateObject( nStreamLengthObject ) ) return false;
                aLine.setLength( 0 );
                aLine.append( nStreamLengthObject );
                aLine.append( " 0 obj\n" );
                aLine.append( static_cast<sal_Int64>(nEndPos-nStartPos) );
                aLine.append( "\nendobj\n\n" );
                if ( !writeBuffer( aLine.getStr(), aLine.getLength() ) ) return false;

                // write font descriptor
                sal_Int32 nFontDescriptor = emitFontDescriptor( subset.first, aSubsetInfo, s_subset.m_nFontID, nFontStream );

                if( nToUnicodeStream )
                    nToUnicodeStream = createToUnicodeCMap( pEncoding, &aCodeUnits[0], pCodeUnitsPerGlyph, pEncToUnicodeIndex, nGlyphs );

                sal_Int32 nFontObject = createObject();
                if ( !updateObject( nFontObject ) ) return false;
                aLine.setLength( 0 );
                aLine.append( nFontObject );

                aLine.append( " 0 obj\n" );
                aLine.append( (aSubsetInfo.m_nFontType & FontType::ANY_TYPE1) ?
                             "<</Type/Font/Subtype/Type1/BaseFont/" :
                             "<</Type/Font/Subtype/TrueType/BaseFont/" );
                appendSubsetName( s_subset.m_nFontID, aSubsetInfo.m_aPSName, aLine );
                aLine.append( "\n"
                             "/FirstChar 0\n"
                             "/LastChar " );
                aLine.append( static_cast<sal_Int32>(nGlyphs-1) );
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
                if ( !writeBuffer( aLine.getStr(), aLine.getLength() ) ) return false;

                aFontIDToObject[ s_subset.m_nFontID ] = nFontObject;
            }
            else
            {
                const PhysicalFontFace* pFont = subset.first;
                OStringBuffer aErrorComment( 256 );
                aErrorComment.append( "CreateFontSubset failed for font \"" );
                aErrorComment.append( OUStringToOString( pFont->GetFamilyName(), RTL_TEXTENCODING_UTF8 ) );
                aErrorComment.append( '\"' );
                if( pFont->GetItalic() == ITALIC_NORMAL )
                    aErrorComment.append( " italic" );
                else if( pFont->GetItalic() == ITALIC_OBLIQUE )
                    aErrorComment.append( " oblique" );
                aErrorComment.append( " weight=" );
                aErrorComment.append( sal_Int32(pFont->GetWeight()) );
                emitComment( aErrorComment.getStr() );
            }
        }
    }
    osl_removeFile( aTmpName.pData );

    // emit system fonts
    for (auto const& systemFont : m_aSystemFonts)
    {
        std::map< sal_Int32, sal_Int32 > aObjects = emitSystemFont( systemFont.first, systemFont.second );
        for (auto const& item : aObjects)
        {
            if ( !item.second ) return false;
            aFontIDToObject[ item.first ] = item.second;
        }
    }

    OStringBuffer aFontDict( 1024 );
    aFontDict.append( getFontDictObject() );
    aFontDict.append( " 0 obj\n"
                     "<<" );
    int ni = 0;
    for (auto const& itemMap : aFontIDToObject)
    {
        aFontDict.append( "/F" );
        aFontDict.append( itemMap.first );
        aFontDict.append( ' ' );
        aFontDict.append( itemMap.second );
        aFontDict.append( " 0 R" );
        if( ((++ni) & 7) == 0 )
            aFontDict.append( '\n' );
    }
    // emit builtin font for widget appearances / variable text
    for (auto & item : m_aBuiltinFontToObjectMap)
    {
        rtl::Reference<PdfBuiltinFontFace> aData(new PdfBuiltinFontFace(m_aBuiltinFonts[item.first]));
        item.second = emitBuiltinFont( aData.get(), item.second );
    }
    appendBuiltinFontsToDict( aFontDict );
    aFontDict.append( "\n>>\nendobj\n\n" );

    if ( !updateObject( getFontDictObject() ) ) return false;
    if ( !writeBuffer( aFontDict.getStr(), aFontDict.getLength() ) ) return false;
    return true;
}

sal_Int32 PDFWriterImpl::emitResources()
{
    // emit shadings
    if( ! m_aGradients.empty() )
        CHECK_RETURN( emitGradients() );
    // emit tilings
    if( ! m_aTilings.empty() )
        CHECK_RETURN( emitTilings() );

    // emit font dict
    CHECK_RETURN( emitFonts() );

    // emit Resource dict
    OStringBuffer aLine( 512 );
    sal_Int32 nResourceDict = getResourceDictObj();
    CHECK_RETURN( updateObject( nResourceDict ) );
    aLine.setLength( 0 );
    aLine.append( nResourceDict );
    aLine.append( " 0 obj\n" );
    m_aGlobalResourceDict.append( aLine, getFontDictObject() );
    aLine.append( "endobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    return nResourceDict;
}

sal_Int32 PDFWriterImpl::updateOutlineItemCount( std::vector< sal_Int32 >& rCounts,
                                                 sal_Int32 nItemLevel,
                                                 sal_Int32 nCurrentItemId )
{
    /* The /Count number of an item is
       positive: the number of visible subitems
       negative: the negative number of subitems that will become visible if
                 the item gets opened
       see PDF ref 1.4 p 478
    */

    sal_Int32 nCount = 0;

    if( m_aContext.OpenBookmarkLevels < 0           || // all levels are visible
        m_aContext.OpenBookmarkLevels >= nItemLevel    // this level is visible
      )
    {
        PDFOutlineEntry& rItem = m_aOutline[ nCurrentItemId ];
        sal_Int32 nChildren = rItem.m_aChildren.size();
        for( sal_Int32 i = 0; i < nChildren; i++ )
            nCount += updateOutlineItemCount( rCounts, nItemLevel+1, rItem.m_aChildren[i] );
        rCounts[nCurrentItemId] = nCount;
        // return 1 (this item) + visible sub items
        if( nCount < 0 )
            nCount = 0;
        nCount++;
    }
    else
    {
        // this bookmark level is invisible
        PDFOutlineEntry& rItem = m_aOutline[ nCurrentItemId ];
        sal_Int32 nChildren = rItem.m_aChildren.size();
        rCounts[ nCurrentItemId ] = -sal_Int32(rItem.m_aChildren.size());
        for( sal_Int32 i = 0; i < nChildren; i++ )
            updateOutlineItemCount( rCounts, nItemLevel+1, rItem.m_aChildren[i] );
        nCount = -1;
    }

    return nCount;
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

    // calculate Count entries for all items
    std::vector< sal_Int32 > aCounts( nItems );
    updateOutlineItemCount( aCounts, 0, 0 );

    // emit hierarchy
    for( i = 0; i < nItems; ++i )
    {
        PDFOutlineEntry& rItem = m_aOutline[i];
        OStringBuffer aLine( 1024 );

        CHECK_RETURN( updateObject( rItem.m_nObject ) );
        aLine.append( rItem.m_nObject );
        aLine.append( " 0 obj\n" );
        aLine.append( "<<" );
        // number of visible children (all levels)
        if( i > 0 || aCounts[0] > 0 )
        {
            aLine.append( "/Count " );
            aLine.append( aCounts[i] );
        }
        if( ! rItem.m_aChildren.empty() )
        {
            // children list: First, Last
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
            appendUnicodeTextStringEncrypt( rItem.m_aTitle, rItem.m_nObject, aLine );
            aLine.append( "\n" );
            // Dest is not required
            if( rItem.m_nDestID >= 0 && rItem.m_nDestID < static_cast<sal_Int32>(m_aDests.size()) )
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
    if( nDestID < 0 || nDestID >= static_cast<sal_Int32>(m_aDests.size()) )
    {
        SAL_INFO("vcl.pdfwriter", "ERROR: invalid dest " << static_cast<int>(nDestID) << " requested");
        return false;
    }

    const PDFDest& rDest        = m_aDests[ nDestID ];
    const PDFPage& rDestPage    = m_aPages[ rDest.m_nPage ];

    rBuffer.append( '[' );
    rBuffer.append( rDestPage.m_nPageObject );
    rBuffer.append( " 0 R" );

    switch( rDest.m_eType )
    {
        case PDFWriter::DestAreaType::XYZ:
        default:
            rBuffer.append( "/XYZ " );
            appendFixedInt( rDest.m_aRect.Left(), rBuffer );
            rBuffer.append( ' ' );
            appendFixedInt( rDest.m_aRect.Bottom(), rBuffer );
            rBuffer.append( " 0" );
            break;
        case PDFWriter::DestAreaType::FitRectangle:
            rBuffer.append( "/FitR " );
            appendFixedInt( rDest.m_aRect.Left(), rBuffer );
            rBuffer.append( ' ' );
            appendFixedInt( rDest.m_aRect.Top(), rBuffer );
            rBuffer.append( ' ' );
            appendFixedInt( rDest.m_aRect.Right(), rBuffer );
            rBuffer.append( ' ' );
            appendFixedInt( rDest.m_aRect.Bottom(), rBuffer );
            break;
    }
    rBuffer.append( ']' );

    return true;
}

bool PDFWriterImpl::emitScreenAnnotations()
{
    int nAnnots = m_aScreens.size();
    for (int i = 0; i < nAnnots; i++)
    {
        const PDFScreen& rScreen = m_aScreens[i];

        OStringBuffer aLine;
        bool bEmbed = false;
        if (!rScreen.m_aTempFileURL.isEmpty())
        {
            bEmbed = true;
            if (!updateObject(rScreen.m_nTempFileObject))
                continue;

            SvFileStream aFileStream(rScreen.m_aTempFileURL, StreamMode::READ);
            SvMemoryStream aMemoryStream;
            aMemoryStream.WriteStream(aFileStream);

            aLine.append(rScreen.m_nTempFileObject);
            aLine.append(" 0 obj\n");
            aLine.append("<< /Type /EmbeddedFile /Length ");
            aLine.append(static_cast<sal_Int64>(aMemoryStream.GetSize()));
            aLine.append(" >>\nstream\n");
            CHECK_RETURN(writeBuffer(aLine.getStr(), aLine.getLength()));
            aLine.setLength(0);

            CHECK_RETURN(writeBuffer(aMemoryStream.GetData(), aMemoryStream.GetSize()));

            aLine.append("\nendstream\nendobj\n\n");
            CHECK_RETURN(writeBuffer(aLine.getStr(), aLine.getLength()));
            aLine.setLength(0);
        }

        if (!updateObject(rScreen.m_nObject))
            continue;

        // Annot dictionary.
        aLine.append(rScreen.m_nObject);
        aLine.append(" 0 obj\n");
        aLine.append("<</Type/Annot");
        aLine.append("/Subtype/Screen/Rect[");
        appendFixedInt(rScreen.m_aRect.Left(), aLine);
        aLine.append(' ');
        appendFixedInt(rScreen.m_aRect.Top(), aLine);
        aLine.append(' ');
        appendFixedInt(rScreen.m_aRect.Right(), aLine);
        aLine.append(' ');
        appendFixedInt(rScreen.m_aRect.Bottom(), aLine);
        aLine.append("]");

        // Action dictionary.
        aLine.append("/A<</Type/Action /S/Rendition /AN ");
        aLine.append(rScreen.m_nObject);
        aLine.append(" 0 R ");

        // Rendition dictionary.
        aLine.append("/R<</Type/Rendition /S/MR ");

        // MediaClip dictionary.
        aLine.append("/C<</Type/MediaClip /S/MCD ");
        if (bEmbed)
        {
            aLine.append("/D << /Type /Filespec /F (<embedded file>) /EF << /F ");
            aLine.append(rScreen.m_nTempFileObject);
            aLine.append(" 0 R >> >>");
        }
        else
        {
            // Linked.
            aLine.append("/D << /Type /Filespec /FS /URL /F ");
            appendLiteralStringEncrypt(rScreen.m_aURL, rScreen.m_nObject, aLine, osl_getThreadTextEncoding());
            aLine.append(" >>");
        }
        // Allow playing the video via a tempfile.
        aLine.append("/P <</TF (TEMPACCESS)>>");
        // Until the real MIME type (instead of application/vnd.sun.star.media) is available here.
        aLine.append("/CT (video/mp4)");
        aLine.append(">>");

        // End Rendition dictionary by requesting play/pause/stop controls.
        aLine.append("/P<</BE<</C true >>>>");
        aLine.append(">>");

        // End Action dictionary.
        aLine.append("/OP 0 >>");

        // End Annot dictionary.
        aLine.append("/P ");
        aLine.append(m_aPages[rScreen.m_nPage].m_nPageObject);
        aLine.append(" 0 R\n>>\nendobj\n\n");
        CHECK_RETURN(writeBuffer(aLine.getStr(), aLine.getLength()));
    }

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
// i59651: key /F set bits Print to 1 rest to 0. We don't set NoZoom NoRotate to 1, since it's a 'should'
// see PDF 8.4.2 and ISO 19005-1:2005 6.5.3
        aLine.append( "<</Type/Annot" );
        if( m_bIsPDF_A1 )
            aLine.append( "/F 4" );
        aLine.append( "/Subtype/Link/Border[0 0 0]/Rect[" );

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
/*
destination is external to the document, so
we check in the following sequence:

 if target type is neither .pdf, nor .od[tpgs], then
          check if relative or absolute and act accordingly (use URI or 'launch application' as requested)
                             end processing
 else if target is .od[tpgs]: then
      if conversion of type from od[tpgs]  to pdf is requested, convert it and this becomes the new target file
      processing continue

 if (new)target is .pdf : then
     if GotToR is requested, then
           convert the target in GoToR where the fragment of the URI is
           considered the named destination in the target file, set relative or absolute as requested
     else strip the fragment from URL and then set URI or 'launch application' as requested
*/

// FIXME: check if the decode mechanisms for URL processing throughout this implementation
// are the correct one!!

// extract target file type
            auto url(URIHelper::resolveIdnaHost(rLink.m_aURL));

            INetURLObject aDocumentURL( m_aContext.BaseURL );
            INetURLObject aTargetURL( url );
            bool bSetGoToRMode = false;
            bool    bTargetHasPDFExtension = false;
            INetProtocol eTargetProtocol = aTargetURL.GetProtocol();
            bool    bIsUNCPath = false;

            // check if the protocol is a known one, or if there is no protocol at all (on target only)
            // if there is no protocol, make the target relative to the current document directory
            // getting the needed URL information from the current document path
            if( eTargetProtocol == INetProtocol::NotValid )
            {
                if( url.getLength() > 4 && url.startsWith("\\\\\\\\"))
                {
                    bIsUNCPath = true;
                }
                else
                {
                    INetURLObject aNewBase( aDocumentURL );//duplicate document URL
                    aNewBase.removeSegment(); //remove last segment from it, obtaining the base URL of the
                                              //target document
                    aNewBase.insertName( url );
                    aTargetURL = aNewBase;//reassign the new target URL
                    //recompute the target protocol, with the new URL
                    //normal URL processing resumes
                    eTargetProtocol = aTargetURL.GetProtocol();
                }
            }

            OUString aFileExtension = aTargetURL.GetFileExtension();

            // Check if the URL ends in '/': if yes it's a directory,
            // it will be forced to a URI link.
            // possibly a malformed URI, leave it as it is, force as URI
            if( aTargetURL.hasFinalSlash() )
                m_aContext.DefaultLinkAction = PDFWriter::URIAction;

            if( !aFileExtension.isEmpty() )
            {
                if( m_aContext.ConvertOOoTargetToPDFTarget )
                {
                    bool bChangeFileExtensionToPDF = false;
                    //examine the file type (.odm .odt. .odp, odg, ods)
                    if( aFileExtension.equalsIgnoreAsciiCase( "odm" ) )
                        bChangeFileExtensionToPDF = true;
                    if( aFileExtension.equalsIgnoreAsciiCase( "odt" ) )
                        bChangeFileExtensionToPDF = true;
                    else if( aFileExtension.equalsIgnoreAsciiCase( "odp" ) )
                        bChangeFileExtensionToPDF = true;
                    else if( aFileExtension.equalsIgnoreAsciiCase( "odg" ) )
                        bChangeFileExtensionToPDF = true;
                    else if( aFileExtension.equalsIgnoreAsciiCase( "ods" ) )
                        bChangeFileExtensionToPDF = true;
                    if( bChangeFileExtensionToPDF )
                        aTargetURL.setExtension("pdf" );
                }
                //check if extension is pdf, see if GoToR should be forced
                bTargetHasPDFExtension = aTargetURL.GetFileExtension().equalsIgnoreAsciiCase( "pdf" );
                if( m_aContext.ForcePDFAction && bTargetHasPDFExtension )
                    bSetGoToRMode = true;
            }
            //prepare the URL, if relative or not
            INetProtocol eBaseProtocol = aDocumentURL.GetProtocol();
            //queue the string common to all types of actions
            aLine.append( "/A<</Type/Action/S");
            if( bIsUNCPath ) // handle Win UNC paths
            {
                aLine.append( "/Launch/Win<</F" );
                // INetURLObject is not good with UNC paths, use original path
                appendLiteralStringEncrypt( url, rLink.m_nObject, aLine, osl_getThreadTextEncoding() );
                aLine.append( ">>" );
            }
            else
            {
                bool bSetRelative = false;
                bool bFileSpec = false;
                //check if relative file link is requested and if the protocol is 'file://'
                if( m_aContext.RelFsys && eBaseProtocol == eTargetProtocol && eTargetProtocol == INetProtocol::File )
                    bSetRelative = true;

                OUString aFragment = aTargetURL.GetMark( INetURLObject::DecodeMechanism::NONE /*DecodeMechanism::WithCharset*/ ); //fragment as is,
                if( !bSetGoToRMode )
                {
                    switch( m_aContext.DefaultLinkAction )
                    {
                    default:
                    case PDFWriter::URIAction :
                    case PDFWriter::URIActionDestination :
                        aLine.append( "/URI/URI" );
                        break;
                    case PDFWriter::LaunchAction:
                        // now:
                        // if a launch action is requested and the hyperlink target has a fragment
                        // and the target file does not have a pdf extension, or it's not a 'file:://'
                        // protocol then force the uri action on it
                        // This code will permit the correct opening of application on web pages,
                        // the one that normally have fragments (but I may be wrong...)
                        // and will force the use of URI when the protocol is not file:
                        if( (!aFragment.isEmpty() && !bTargetHasPDFExtension) ||
                                        eTargetProtocol != INetProtocol::File )
                        {
                            aLine.append( "/URI/URI" );
                        }
                        else
                        {
                            aLine.append( "/Launch/F" );
                            bFileSpec = true;
                        }
                        break;
                    }
                }

                //fragment are encoded in the same way as in the named destination processing
                if( bSetGoToRMode )
                {
                    //add the fragment
                    OUString aURLNoMark = aTargetURL.GetURLNoMark( INetURLObject::DecodeMechanism::WithCharset );
                    aLine.append("/GoToR");
                    aLine.append("/F");
                    appendLiteralStringEncrypt( bSetRelative ? INetURLObject::GetRelURL( m_aContext.BaseURL, aURLNoMark,
                                                                                         INetURLObject::EncodeMechanism::WasEncoded,
                                                                                         INetURLObject::DecodeMechanism::WithCharset ) :
                                                                   aURLNoMark, rLink.m_nObject, aLine, osl_getThreadTextEncoding() );
                    if( !aFragment.isEmpty() )
                    {
                        aLine.append("/D/");
                        appendDestinationName( aFragment , aLine );
                    }
                }
                else
                {
                    // change the fragment to accommodate the bookmark (only if the file extension
                    // is PDF and the requested action is of the correct type)
                    if(m_aContext.DefaultLinkAction == PDFWriter::URIActionDestination &&
                               bTargetHasPDFExtension && !aFragment.isEmpty() )
                    {
                        OStringBuffer aLineLoc( 1024 );
                        appendDestinationName( aFragment , aLineLoc );
                        //substitute the fragment
                        aTargetURL.SetMark( OStringToOUString(aLineLoc.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US) );
                    }
                    OUString aURL = aTargetURL.GetMainURL( bFileSpec ? INetURLObject::DecodeMechanism::WithCharset : INetURLObject::DecodeMechanism::NONE );
                    appendLiteralStringEncrypt(bSetRelative ? INetURLObject::GetRelURL( m_aContext.BaseURL, aURL,
                                                                                        INetURLObject::EncodeMechanism::WasEncoded,
                                                                                            bFileSpec ? INetURLObject::DecodeMechanism::WithCharset : INetURLObject::DecodeMechanism::NONE
                                                                                            ) :
                                                                               aURL , rLink.m_nObject, aLine, osl_getThreadTextEncoding() );
                }
            }
            aLine.append( ">>\n" );
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
// i59651: key /F set bits Print to 1 rest to 0. We don't set NoZoom NoRotate to 1, since it's a 'should'
// see PDF 8.4.2 and ISO 19005-1:2005 6.5.3
        aLine.append( "<</Type/Annot" );
        if( m_bIsPDF_A1 )
            aLine.append( "/F 4" );
        aLine.append( "/Subtype/Text/Rect[" );

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
        appendUnicodeTextStringEncrypt( rNote.m_aContents.Contents, rNote.m_nObject, aLine );
        aLine.append( "\n" );

        // optional title
        if( !rNote.m_aContents.Title.isEmpty() )
        {
            aLine.append( "/T" );
            appendUnicodeTextStringEncrypt( rNote.m_aContents.Title, rNote.m_nObject, aLine );
            aLine.append( "\n" );
        }

        aLine.append( ">>\nendobj\n\n" );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }
    return true;
}

Font PDFWriterImpl::replaceFont( const vcl::Font& rControlFont, const vcl::Font&  rAppSetFont )
{
    bool bAdjustSize = false;

    Font aFont( rControlFont );
    if( aFont.GetFamilyName().isEmpty() )
    {
        aFont = rAppSetFont;
        if( rControlFont.GetFontHeight() )
            aFont.SetFontSize( Size( 0, rControlFont.GetFontHeight() ) );
        else
            bAdjustSize = true;
        if( rControlFont.GetItalic() != ITALIC_DONTKNOW )
            aFont.SetItalic( rControlFont.GetItalic() );
        if( rControlFont.GetWeight() != WEIGHT_DONTKNOW )
            aFont.SetWeight( rControlFont.GetWeight() );
    }
    else if( ! aFont.GetFontHeight() )
    {
        aFont.SetFontSize( rAppSetFont.GetFontSize() );
        bAdjustSize = true;
    }
    if( bAdjustSize )
    {
        Size aFontSize = aFont.GetFontSize();
        OutputDevice* pDefDev = Application::GetDefaultDevice();
        aFontSize = OutputDevice::LogicToLogic( aFontSize, pDefDev->GetMapMode(), getMapMode() );
        aFont.SetFontSize( aFontSize );
    }
    return aFont;
}

sal_Int32 PDFWriterImpl::getBestBuiltinFont( const vcl::Font& rFont )
{
    sal_Int32 nBest = 4; // default to Helvetica
    OUString aFontName( rFont.GetFamilyName() );
    aFontName = aFontName.toAsciiLowerCase();

    if( aFontName.indexOf( "times" ) != -1 )
        nBest = 8;
    else if( aFontName.indexOf( "courier" ) != -1 )
        nBest = 0;
    else if( aFontName.indexOf( "dingbats" ) != -1 )
        nBest = 13;
    else if( aFontName.indexOf( "symbol" ) != -1 )
        nBest = 12;
    if( nBest < 12 )
    {
        if( rFont.GetItalic() == ITALIC_OBLIQUE || rFont.GetItalic() == ITALIC_NORMAL )
            nBest += 1;
        if( rFont.GetWeight() > WEIGHT_MEDIUM )
            nBest += 2;
    }

    if( m_aBuiltinFontToObjectMap.find( nBest ) == m_aBuiltinFontToObjectMap.end() )
        m_aBuiltinFontToObjectMap[ nBest ] = createObject();

    return nBest;
}

static inline const Color& replaceColor( const Color& rCol1, const Color& rCol2 )
{
    return (rCol1 == COL_TRANSPARENT) ? rCol2 : rCol1;
}

void PDFWriterImpl::createDefaultPushButtonAppearance( PDFWidget& rButton, const PDFWriter::PushButtonWidget& rWidget )
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();

    // save graphics state
    push( PushFlags::ALL );

    // transform relative to control's coordinates since an
    // appearance stream is a form XObject
    // this relies on the m_aRect member of rButton NOT already being transformed
    // to default user space
    if( rWidget.Background || rWidget.Border )
    {
        setLineColor( rWidget.Border ? replaceColor( rWidget.BorderColor, rSettings.GetLightColor() ) : COL_TRANSPARENT );
        setFillColor( rWidget.Background ? replaceColor( rWidget.BackgroundColor, rSettings.GetDialogColor() ) : COL_TRANSPARENT );
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
    Font aDummyFont( "Helvetica", aFont.GetFontSize() );
    sal_Int32 nDummyBuiltin = getBestBuiltinFont( aDummyFont );
    aDA.append( ' ' );
    aDA.append( m_aBuiltinFonts[nDummyBuiltin].getNameObject() );
    aDA.append( ' ' );
    m_aPages[m_nCurrentPage].appendMappedLength( sal_Int32( aFont.GetFontHeight() ), aDA );
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
       filled with the background color but nor text. Urgh.
    */
    rButton.m_aMKDict = "/BC [] /BG [] /CA";
    rButton.m_aMKDictCAString = "";
}

Font PDFWriterImpl::drawFieldBorder( PDFWidget& rIntern,
                                     const PDFWriter::AnyWidget& rWidget,
                                     const StyleSettings& rSettings )
{
    Font aFont = replaceFont( rWidget.TextFont, rSettings.GetFieldFont() );

    if( rWidget.Background || rWidget.Border )
    {
        if( rWidget.Border && rWidget.BorderColor == COL_TRANSPARENT )
        {
            sal_Int32 nDelta = getReferenceDevice()->GetDPIX() / 500;
            if( nDelta < 1 )
                nDelta = 1;
            setLineColor( COL_TRANSPARENT );
            tools::Rectangle aRect = rIntern.m_aRect;
            setFillColor( rSettings.GetLightBorderColor() );
            drawRectangle( aRect );
            aRect.AdjustLeft(nDelta ); aRect.AdjustTop(nDelta );
            aRect.AdjustRight( -nDelta ); aRect.AdjustBottom( -nDelta );
            setFillColor( rSettings.GetFieldColor() );
            drawRectangle( aRect );
            setFillColor( rSettings.GetLightColor() );
            drawRectangle( tools::Rectangle( Point( aRect.Left(), aRect.Bottom()-nDelta ), aRect.BottomRight() ) );
            drawRectangle( tools::Rectangle( Point( aRect.Right()-nDelta, aRect.Top() ), aRect.BottomRight() ) );
            setFillColor( rSettings.GetDarkShadowColor() );
            drawRectangle( tools::Rectangle( aRect.TopLeft(), Point( aRect.Left()+nDelta, aRect.Bottom() ) ) );
            drawRectangle( tools::Rectangle( aRect.TopLeft(), Point( aRect.Right(), aRect.Top()+nDelta ) ) );
        }
        else
        {
            setLineColor( rWidget.Border ? replaceColor( rWidget.BorderColor, rSettings.GetShadowColor() ) : COL_TRANSPARENT );
            setFillColor( rWidget.Background ? replaceColor( rWidget.BackgroundColor, rSettings.GetFieldColor() ) : COL_TRANSPARENT );
            drawRectangle( rIntern.m_aRect );
        }

        if( rWidget.Border )
        {
            // adjust edit area accounting for border
            sal_Int32 nDelta = aFont.GetFontHeight()/4;
            if( nDelta < 1 )
                nDelta = 1;
            rIntern.m_aRect.AdjustLeft(nDelta );
            rIntern.m_aRect.AdjustTop(nDelta );
            rIntern.m_aRect.AdjustRight( -nDelta );
            rIntern.m_aRect.AdjustBottom( -nDelta );
        }
    }
    return aFont;
}

void PDFWriterImpl::createDefaultEditAppearance( PDFWidget& rEdit, const PDFWriter::EditWidget& rWidget )
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    SvMemoryStream* pEditStream = new SvMemoryStream( 1024, 1024 );

    push( PushFlags::ALL );

    // prepare font to use, draw field border
    Font aFont = drawFieldBorder( rEdit, rWidget, rSettings );
    sal_Int32 nBest = getSystemFont( aFont );

    // prepare DA string
    OStringBuffer aDA( 32 );
    appendNonStrokingColor( replaceColor( rWidget.TextColor, rSettings.GetFieldTextColor() ), aDA );
    aDA.append( ' ' );
    aDA.append( "/F" );
    aDA.append( nBest );

    OStringBuffer aDR( 32 );
    aDR.append( "/Font " );
    aDR.append( getFontDictObject() );
    aDR.append( " 0 R" );
    rEdit.m_aDRDict = aDR.makeStringAndClear();
    aDA.append( ' ' );
    m_aPages[ m_nCurrentPage ].appendMappedLength( sal_Int32( aFont.GetFontHeight() ), aDA );
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

    push( PushFlags::ALL );

    // prepare font to use, draw field border
    Font aFont = drawFieldBorder( rBox, rWidget, rSettings );
    sal_Int32 nBest = getSystemFont( aFont );

    beginRedirect( pListBoxStream, rBox.m_aRect );
    OStringBuffer aAppearance( 64 );

    setLineColor( COL_TRANSPARENT );
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
    // prepare DA string
    appendNonStrokingColor( replaceColor( rWidget.TextColor, rSettings.GetFieldTextColor() ), aDA );
    aDA.append( ' ' );
    aDA.append( "/F" );
    aDA.append( nBest );

    OStringBuffer aDR( 32 );
    aDR.append( "/Font " );
    aDR.append( getFontDictObject() );
    aDR.append( " 0 R" );
    rBox.m_aDRDict = aDR.makeStringAndClear();
    aDA.append( ' ' );
    m_aPages[ m_nCurrentPage ].appendMappedLength( sal_Int32( aFont.GetFontHeight() ), aDA );
    aDA.append( " Tf" );
    rBox.m_aDAString = aDA.makeStringAndClear();
}

void PDFWriterImpl::createDefaultCheckBoxAppearance( PDFWidget& rBox, const PDFWriter::CheckBoxWidget& rWidget )
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();

    // save graphics state
    push( PushFlags::ALL );

    if( rWidget.Background || rWidget.Border )
    {
        setLineColor( rWidget.Border ? replaceColor( rWidget.BorderColor, rSettings.GetCheckedColor() ) : COL_TRANSPARENT );
        setFillColor( rWidget.Background ? replaceColor( rWidget.BackgroundColor, rSettings.GetFieldColor() ) : COL_TRANSPARENT );
        drawRectangle( rBox.m_aRect );
    }

    Font aFont = replaceFont( rWidget.TextFont, rSettings.GetRadioCheckFont() );
    setFont( aFont );
    Size aFontSize = aFont.GetFontSize();
    if( aFontSize.Height() > rBox.m_aRect.GetHeight() )
        aFontSize.setHeight( rBox.m_aRect.GetHeight() );
    sal_Int32 nDelta = aFontSize.Height()/10;
    if( nDelta < 1 )
        nDelta = 1;

    tools::Rectangle aCheckRect, aTextRect;
    {
        aCheckRect.SetLeft( rBox.m_aRect.Left() + nDelta );
        aCheckRect.SetTop( rBox.m_aRect.Top() + (rBox.m_aRect.GetHeight()-aFontSize.Height())/2 );
        aCheckRect.SetRight( aCheckRect.Left() + aFontSize.Height() );
        aCheckRect.SetBottom( aCheckRect.Top() + aFontSize.Height() );

        // #i74206# handle small controls without text area
        while( aCheckRect.GetWidth() > rBox.m_aRect.GetWidth() && aCheckRect.GetWidth() > nDelta )
        {
            aCheckRect.AdjustRight( -nDelta );
            aCheckRect.AdjustTop(nDelta/2 );
            aCheckRect.AdjustBottom( -(nDelta - (nDelta/2)) );
        }

        aTextRect.SetLeft( rBox.m_aRect.Left() + aCheckRect.GetWidth()+5*nDelta );
        aTextRect.SetTop( rBox.m_aRect.Top() );
        aTextRect.SetRight( aTextRect.Left() + rBox.m_aRect.GetWidth() - aCheckRect.GetWidth()-6*nDelta );
        aTextRect.SetBottom( rBox.m_aRect.Bottom() );
    }
    setLineColor( COL_BLACK );
    setFillColor( COL_TRANSPARENT );
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
    sal_Int32 nBest = getBestBuiltinFont( Font( "ZapfDingbats", aFont.GetFontSize() ) );
    aDA.append( ' ' );
    aDA.append( m_aBuiltinFonts[nBest].getNameObject() );
    aDA.append( " 0 Tf" );
    rBox.m_aDAString = aDA.makeStringAndClear();
    rBox.m_aMKDict = "/CA";
    rBox.m_aMKDictCAString = "8";
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
    aDA.append( ' ' );
    aDA.append( m_aBuiltinFonts[nBest].getNameObject() );
    aDA.append( ' ' );
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
    push( PushFlags::ALL );

    if( rWidget.Background || rWidget.Border )
    {
        setLineColor( rWidget.Border ? replaceColor( rWidget.BorderColor, rSettings.GetCheckedColor() ) : COL_TRANSPARENT );
        setFillColor( rWidget.Background ? replaceColor( rWidget.BackgroundColor, rSettings.GetFieldColor() ) : COL_TRANSPARENT );
        drawRectangle( rBox.m_aRect );
    }

    Font aFont = replaceFont( rWidget.TextFont, rSettings.GetRadioCheckFont() );
    setFont( aFont );
    Size aFontSize = aFont.GetFontSize();
    if( aFontSize.Height() > rBox.m_aRect.GetHeight() )
        aFontSize.setHeight( rBox.m_aRect.GetHeight() );
    sal_Int32 nDelta = aFontSize.Height()/10;
    if( nDelta < 1 )
        nDelta = 1;

    tools::Rectangle aCheckRect, aTextRect;
    {
        aCheckRect.SetLeft( rBox.m_aRect.Left() + nDelta );
        aCheckRect.SetTop( rBox.m_aRect.Top() + (rBox.m_aRect.GetHeight()-aFontSize.Height())/2 );
        aCheckRect.SetRight( aCheckRect.Left() + aFontSize.Height() );
        aCheckRect.SetBottom( aCheckRect.Top() + aFontSize.Height() );

        // #i74206# handle small controls without text area
        while( aCheckRect.GetWidth() > rBox.m_aRect.GetWidth() && aCheckRect.GetWidth() > nDelta )
        {
            aCheckRect.AdjustRight( -nDelta );
            aCheckRect.AdjustTop(nDelta/2 );
            aCheckRect.AdjustBottom( -(nDelta - (nDelta/2)) );
        }

        aTextRect.SetLeft( rBox.m_aRect.Left() + aCheckRect.GetWidth()+5*nDelta );
        aTextRect.SetTop( rBox.m_aRect.Top() );
        aTextRect.SetRight( aTextRect.Left() + rBox.m_aRect.GetWidth() - aCheckRect.GetWidth()-6*nDelta );
        aTextRect.SetBottom( rBox.m_aRect.Bottom() );
    }
    setLineColor( COL_BLACK );
    setFillColor( COL_TRANSPARENT );
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
    sal_Int32 nBest = getBestBuiltinFont( Font( "ZapfDingbats", aFont.GetFontSize() ) );
    aDA.append( ' ' );
    aDA.append( m_aBuiltinFonts[nBest].getNameObject() );
    aDA.append( " 0 Tf" );
    rBox.m_aDAString = aDA.makeStringAndClear();
    //to encrypt this (el)
    rBox.m_aMKDict = "/CA";
    //after this assignment, to m_aMKDic cannot be added anything
    rBox.m_aMKDictCAString = "l";

    rBox.m_aRect = aCheckRect;

    // create appearance streams
    push( PushFlags::ALL);
    SvMemoryStream* pCheckStream = new SvMemoryStream( 256, 256 );

    beginRedirect( pCheckStream, aCheckRect );
    aDA.append( "/Tx BMC\nq BT\n" );
    appendNonStrokingColor( replaceColor( rWidget.TextColor, rSettings.GetRadioCheckTextColor() ), aDA );
    aDA.append( ' ' );
    aDA.append( m_aBuiltinFonts[nBest].getNameObject() );
    aDA.append( ' ' );
    m_aPages[m_nCurrentPage].appendMappedLength( sal_Int32( aCheckRect.GetHeight() ), aDA );
    aDA.append( " Tf\n0 0 Td\nET\nQ\n" );
    writeBuffer( aDA.getStr(), aDA.getLength() );
    setFillColor( replaceColor( rWidget.TextColor, rSettings.GetRadioCheckTextColor() ) );
    setLineColor( COL_TRANSPARENT );
    aCheckRect.AdjustLeft(3*nDelta );
    aCheckRect.AdjustTop(3*nDelta );
    aCheckRect.AdjustBottom( -(3*nDelta) );
    aCheckRect.AdjustRight( -(3*nDelta) );
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
    OString aStandardAppearance;
    switch( rWidget.m_eType )
    {
        case PDFWriter::CheckBox:
            aStandardAppearance = OUStringToOString( rWidget.m_aValue, RTL_TEXTENCODING_ASCII_US );
            break;
        default:
            break;
    }

    if( !rWidget.m_aAppearances.empty() )
    {
        rAnnotDict.append( "/AP<<\n" );
        for (auto & dict_item : rWidget.m_aAppearances)
        {
            rAnnotDict.append( "/" );
            rAnnotDict.append( dict_item.first );
            bool bUseSubDict = (dict_item.second.size() > 1);
            rAnnotDict.append( bUseSubDict ? "<<" : " " );

            for (auto const& stream_item : dict_item.second)
            {
                SvMemoryStream* pApppearanceStream = stream_item.second;
                dict_item.second[ stream_item.first ] = nullptr;

                bool bDeflate = compressStream( pApppearanceStream );

                pApppearanceStream->Seek( STREAM_SEEK_TO_END );
                sal_Int64 nStreamLen = pApppearanceStream->Tell();
                pApppearanceStream->Seek( STREAM_SEEK_TO_BEGIN );
                sal_Int32 nObject = createObject();
                CHECK_RETURN( updateObject( nObject ) );
                if (g_bDebugDisableCompression)
                {
                    emitComment( "PDFWriterImpl::emitAppearances" );
                }
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
                checkAndEnableStreamEncryption( nObject );
                CHECK_RETURN( writeBuffer( pApppearanceStream->GetData(), nStreamLen ) );
                disableStreamEncryption();
                CHECK_RETURN( writeBuffer( "\nendstream\nendobj\n\n", 19 ) );

                if( bUseSubDict )
                {
                    rAnnotDict.append( " /" );
                    rAnnotDict.append( stream_item.first );
                    rAnnotDict.append( " " );
                }
                rAnnotDict.append( nObject );
                rAnnotDict.append( " 0 R" );

                delete pApppearanceStream;
            }

            rAnnotDict.append( bUseSubDict ? ">>\n" : "\n" );
        }
        rAnnotDict.append( ">>\n" );
        if( !aStandardAppearance.isEmpty() )
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
        if( rWidget.m_eType != PDFWriter::Hierarchy )
        {
            // emit widget annotation only for terminal fields
            if( rWidget.m_aKids.empty() )
            {
                int iRectMargin;

                aLine.append( "/Type/Annot/Subtype/Widget/F " );

                if (rWidget.m_eType == PDFWriter::Signature)
                {
                    aLine.append( "132\n" ); // Print & Locked
                    iRectMargin = 0;
                }
                else
                {
                    aLine.append( "4\n" );
                    iRectMargin = 1;
                }

                aLine.append("/Rect[" );
                appendFixedInt( rWidget.m_aRect.Left()-iRectMargin, aLine );
                aLine.append( ' ' );
                appendFixedInt( rWidget.m_aRect.Top()+iRectMargin, aLine );
                aLine.append( ' ' );
                appendFixedInt( rWidget.m_aRect.Right()+iRectMargin, aLine );
                aLine.append( ' ' );
                appendFixedInt( rWidget.m_aRect.Bottom()-iRectMargin, aLine );
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
                    // of course real check boxes (not belonging to a radio group)
                    // need their values, too
                    if( rWidget.m_eType == PDFWriter::RadioButton || rWidget.m_nRadioGroup < 0 )
                    {
                        aValue.append( "/" );
                        // check for radio group with all buttons unpressed
                        if( rWidget.m_aValue.isEmpty() )
                            aValue.append( "Off" );
                        else
                            appendName( rWidget.m_aValue, aValue );
                    }
                    SAL_FALLTHROUGH;
                case PDFWriter::PushButton:
                    aLine.append( "Btn" );
                    break;
                case PDFWriter::ListBox:
                    if( rWidget.m_nFlags & 0x200000 ) // multiselect
                    {
                        aValue.append( "[" );
                        for( size_t i = 0; i < rWidget.m_aSelectedEntries.size(); i++ )
                        {
                            sal_Int32 nEntry = rWidget.m_aSelectedEntries[i];
                            if( nEntry >= 0 && nEntry < sal_Int32(rWidget.m_aListEntries.size()) )
                                appendUnicodeTextStringEncrypt( rWidget.m_aListEntries[ nEntry ], rWidget.m_nObject, aValue );
                        }
                        aValue.append( "]" );
                    }
                    else if( rWidget.m_aSelectedEntries.size() > 0 &&
                             rWidget.m_aSelectedEntries[0] >= 0 &&
                             rWidget.m_aSelectedEntries[0] < sal_Int32(rWidget.m_aListEntries.size()) )
                    {
                        appendUnicodeTextStringEncrypt( rWidget.m_aListEntries[ rWidget.m_aSelectedEntries[0] ], rWidget.m_nObject, aValue );
                    }
                    else
                        appendUnicodeTextStringEncrypt( OUString(), rWidget.m_nObject, aValue );
                    aLine.append( "Ch" );
                    break;
                case PDFWriter::ComboBox:
                    appendUnicodeTextStringEncrypt( rWidget.m_aValue, rWidget.m_nObject, aValue );
                    aLine.append( "Ch" );
                    break;
                case PDFWriter::Edit:
                    aLine.append( "Tx" );
                    appendUnicodeTextStringEncrypt( rWidget.m_aValue, rWidget.m_nObject, aValue );
                    break;
                case PDFWriter::Signature:
                    aLine.append( "Sig" );
                    aValue.append(OUStringToOString(rWidget.m_aValue, RTL_TEXTENCODING_ASCII_US));
                    break;
                case PDFWriter::Hierarchy: // make the compiler happy
                    break;
            }
            aLine.append( "\n" );
            aLine.append( "/P " );
            aLine.append( m_aPages[ rWidget.m_nPage ].m_nPageObject );
            aLine.append( " 0 R\n" );
        }
        if( rWidget.m_nParent )
        {
            aLine.append( "/Parent " );
            aLine.append( rWidget.m_nParent );
            aLine.append( " 0 R\n" );
        }
        if( rWidget.m_aKids.size() )
        {
            aLine.append( "/Kids[" );
            for( size_t i = 0; i < rWidget.m_aKids.size(); i++ )
            {
                aLine.append( rWidget.m_aKids[i] );
                aLine.append( " 0 R" );
                aLine.append( ( (i&15) == 15 ) ? "\n" : " " );
            }
            aLine.append( "]\n" );
        }
        if( !rWidget.m_aName.isEmpty() )
        {
            aLine.append( "/T" );
            appendLiteralStringEncrypt( rWidget.m_aName, rWidget.m_nObject, aLine );
            aLine.append( "\n" );
        }
        if( m_aContext.Version > PDFWriter::PDFVersion::PDF_1_2 && !rWidget.m_aDescription.isEmpty() )
        {
            // the alternate field name should be unicode able since it is
            // supposed to be used in UI
            aLine.append( "/TU" );
            appendUnicodeTextStringEncrypt( rWidget.m_aDescription, rWidget.m_nObject, aLine );
            aLine.append( "\n" );
        }

        if( rWidget.m_nFlags )
        {
            aLine.append( "/Ff " );
            aLine.append( rWidget.m_nFlags );
            aLine.append( "\n" );
        }
        if( !aValue.isEmpty() )
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
            for (auto const& entry : rWidget.m_aListEntries)
            {
                appendUnicodeTextStringEncrypt( entry, rWidget.m_nObject, aLine );
                aLine.append( "\n" );
                if( entry == rWidget.m_aValue )
                    nTI = i;
                ++i;
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
            if(!m_bIsPDF_A1)
            {
                OStringBuffer aDest;
                if( rWidget.m_nDest != -1 && appendDest( m_aDestinationIdTranslation[ rWidget.m_nDest ], aDest ) )
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
                    aLine.append( "/AA<</D<</Type/Action/S/SubmitForm/F" );
                    appendLiteralStringEncrypt( rWidget.m_aListEntries.front(), rWidget.m_nObject, aLine, osl_getThreadTextEncoding() );
                    aLine.append( "/Flags " );

                    sal_Int32 nFlags = 0;
                    switch( m_aContext.SubmitFormat )
                    {
                    case PDFWriter::HTML:
                        nFlags |= 4;
                        break;
                    case PDFWriter::XML:
                        if( m_aContext.Version > PDFWriter::PDFVersion::PDF_1_3 )
                            nFlags |= 32;
                        break;
                    case PDFWriter::PDF:
                        if( m_aContext.Version > PDFWriter::PDFVersion::PDF_1_3 )
                            nFlags |= 256;
                        break;
                    case PDFWriter::FDF:
                    default:
                        break;
                    }
                    if( rWidget.m_bSubmitGet )
                        nFlags |= 8;
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
            else
                m_aErrors.insert( PDFWriter::Warning_FormAction_Omitted_PDFA );
        }
        if( !rWidget.m_aDAString.isEmpty() )
        {
            if( !rWidget.m_aDRDict.isEmpty() )
            {
                aLine.append( "/DR<<" );
                aLine.append( rWidget.m_aDRDict );
                aLine.append( ">>\n" );
            }
            else
            {
                aLine.append( "/DR<</Font<<" );
                appendBuiltinFontsToDict( aLine );
                aLine.append( ">>>>\n" );
            }
            aLine.append( "/DA" );
            appendLiteralStringEncrypt( rWidget.m_aDAString, rWidget.m_nObject, aLine );
            aLine.append( "\n" );
            if( rWidget.m_nTextStyle & DrawTextFlags::Center )
                aLine.append( "/Q 1\n" );
            else if( rWidget.m_nTextStyle & DrawTextFlags::Right )
                aLine.append( "/Q 2\n" );
        }
        // appearance characteristics for terminal fields
        // which are supposed to have an appearance constructed
        // by the viewer application
        if( !rWidget.m_aMKDict.isEmpty() )
        {
            aLine.append( "/MK<<" );
            aLine.append( rWidget.m_aMKDict );
            //add the CA string, encrypting it
            appendLiteralStringEncrypt(rWidget.m_aMKDictCAString, rWidget.m_nObject, aLine);
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
    CHECK_RETURN(emitScreenAnnotations());
    CHECK_RETURN( emitNoteAnnotations() );
    CHECK_RETURN( emitWidgetAnnotations() );

    return true;
}

bool PDFWriterImpl::emitEmbeddedFiles()
{
    for (auto& rEmbeddedFile : m_aEmbeddedFiles)
    {
        if (!updateObject(rEmbeddedFile.m_nObject))
            continue;

        OStringBuffer aLine;
        aLine.append(rEmbeddedFile.m_nObject);
        aLine.append(" 0 obj\n");
        aLine.append("<< /Type /EmbeddedFile /Length ");
        aLine.append(static_cast<sal_Int64>(rEmbeddedFile.m_aData.getLength()));
        aLine.append(" >>\nstream\n");
        CHECK_RETURN(writeBuffer(aLine.getStr(), aLine.getLength()));
        aLine.setLength(0);

        CHECK_RETURN(writeBuffer(rEmbeddedFile.m_aData.getArray(), rEmbeddedFile.m_aData.getLength()));

        aLine.append("\nendstream\nendobj\n\n");
        CHECK_RETURN(writeBuffer(aLine.getStr(), aLine.getLength()));
    }
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
    for (auto & page : m_aPages)
        if( ! page.emit( nTreeNode ) )
            return false;

    sal_Int32 nNamedDestinationsDictionary = emitNamedDestinations();

    sal_Int32 nOutlineDict = emitOutline();

    // emit Output intent
    sal_Int32 nOutputIntentObject = emitOutputIntent();

    // emit metadata
    sal_Int32 nMetadataObject = emitDocumentMetadata();

    sal_Int32 nStructureDict = 0;
    if(m_aStructure.size() > 1)
    {
        // check if dummy structure containers are needed
        addInternalStructureContainer(m_aStructure[0]);
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

    sal_Int32 nMediaBoxWidth = 0;
    sal_Int32 nMediaBoxHeight = 0;
    if( m_aPages.empty() ) // sanity check, this should not happen
    {
        nMediaBoxWidth = m_nInheritedPageWidth;
        nMediaBoxHeight = m_nInheritedPageHeight;
    }
    else
    {
        for (auto const& page : m_aPages)
        {
            if( page.m_nPageWidth > nMediaBoxWidth )
                nMediaBoxWidth = page.m_nPageWidth;
            if( page.m_nPageHeight > nMediaBoxHeight )
                nMediaBoxHeight = page.m_nPageHeight;
        }
    }
    aLine.append( "/MediaBox[ 0 0 " );
    aLine.append( nMediaBoxWidth );
    aLine.append( ' ' );
    aLine.append( nMediaBoxHeight );
    aLine.append( " ]\n"
                  "/Kids[ " );
    unsigned int i = 0;
    for (auto & page : m_aPages)
    {
        aLine.append( page.m_nPageObject );
        aLine.append( " 0 R" );
        aLine.append( ( (i&15) == 15 ) ? "\n" : " " );
        ++i;
    }
    aLine.append( "]\n"
                  "/Count " );
    aLine.append( static_cast<sal_Int32>(m_aPages.size()) );
    aLine.append( ">>\n"
                  "endobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    // emit annotation objects
    CHECK_RETURN( emitAnnotations() );
    CHECK_RETURN( emitEmbeddedFiles() );

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

    // check if there are named destinations to emit (root must be inside the catalog)
    if( nNamedDestinationsDictionary )
    {
        aLine.append("/Dests ");
        aLine.append( nNamedDestinationsDictionary );
        aLine.append( " 0 R\n" );
    }

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
            // the flag m_aContext.FirstPageLeft below is used to set the page on the left side
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

    OStringBuffer aInitPageRef;
    if( m_aContext.InitialPage >= 0 && m_aContext.InitialPage < static_cast<sal_Int32>(m_aPages.size()) )
    {
        aInitPageRef.append( m_aPages[m_aContext.InitialPage].m_nPageObject );
        aInitPageRef.append( " 0 R" );
    }
    else
        aInitPageRef.append( "0" );

    switch( m_aContext.PDFDocumentAction )
    {
    case PDFWriter::ActionDefault :     //do nothing, this is the Acrobat default
    default:
        if( aInitPageRef.getLength() > 1 )
        {
            aLine.append( "/OpenAction[" );
            aLine.append( aInitPageRef.makeStringAndClear() );
            aLine.append( " /XYZ null null 0]\n" );
        }
        break;
    case PDFWriter::FitInWindow :
        aLine.append( "/OpenAction[" );
        aLine.append( aInitPageRef.makeStringAndClear() );
        aLine.append( " /Fit]\n" ); //Open fit page
        break;
    case PDFWriter::FitWidth :
        aLine.append( "/OpenAction[" );
        aLine.append( aInitPageRef.makeStringAndClear() );
        aLine.append( " /FitH " );
        aLine.append( m_nInheritedPageHeight );//Open fit width
        aLine.append( "]\n" );
        break;
    case PDFWriter::FitVisible :
        aLine.append( "/OpenAction[" );
        aLine.append( aInitPageRef.makeStringAndClear() );
        aLine.append( " /FitBH " );
        aLine.append( m_nInheritedPageHeight );//Open fit visible
        aLine.append( "]\n" );
        break;
    case PDFWriter::ActionZoom :
        aLine.append( "/OpenAction[" );
        aLine.append( aInitPageRef.makeStringAndClear() );
        aLine.append( " /XYZ null null " );
        if( m_aContext.Zoom >= 50 && m_aContext.Zoom <= 1600 )
            aLine.append( static_cast<double>(m_aContext.Zoom)/100.0 );
        else
            aLine.append( "0" );
        aLine.append( "]\n" );
        break;
    }

    // viewer preferences, if we had some, then emit
    if( m_aContext.HideViewerToolbar ||
        ( m_aContext.Version > PDFWriter::PDFVersion::PDF_1_3 && !m_aContext.DocumentInfo.Title.isEmpty() && m_aContext.DisplayPDFDocumentTitle ) ||
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
            aLine.append( "/FitWindow true\n" );
        if( m_aContext.CenterWindow )
            aLine.append( "/CenterWindow true\n" );
        if( m_aContext.Version > PDFWriter::PDFVersion::PDF_1_3 && !m_aContext.DocumentInfo.Title.isEmpty() && m_aContext.DisplayPDFDocumentTitle )
            aLine.append( "/DisplayDocTitle true\n" );
        if( m_aContext.FirstPageLeft &&  m_aContext.PageLayout == PDFWriter::ContinuousFacing )
            aLine.append( "/Direction/R2L\n" );
        if( m_aContext.OpenInFullScreenMode )
            switch( m_aContext.PDFDocumentMode )
            {
            default :
            case PDFWriter::ModeDefault :
                aLine.append( "/NonFullScreenPageMode/UseNone\n" );
                break;
            case PDFWriter::UseOutlines :
                aLine.append( "/NonFullScreenPageMode/UseOutlines\n" );
                break;
            case PDFWriter::UseThumbs :
                aLine.append( "/NonFullScreenPageMode/UseThumbs\n" );
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
    if( !m_aContext.DocumentLocale.Language.isEmpty() )
    {
        /* PDF allows only RFC 3066, see above in emitStructure(). */
        LanguageTag aLanguageTag( m_aContext.DocumentLocale);
        OUString aLanguage, aScript, aCountry;
        aLanguageTag.getIsoLanguageScriptCountry( aLanguage, aScript, aCountry);
        if (!aLanguage.isEmpty())
        {
            OUStringBuffer aLocBuf( 16 );
            aLocBuf.append( aLanguage );
            if( !aCountry.isEmpty() )
            {
                aLocBuf.append( '-' );
                aLocBuf.append( aCountry );
            }
            aLine.append( "/Lang" );
            appendLiteralStringEncrypt( aLocBuf.makeStringAndClear(), m_nCatalogObject, aLine );
            aLine.append( "\n" );
        }
    }
    if( m_aContext.Tagged && m_aContext.Version > PDFWriter::PDFVersion::PDF_1_3 )
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
        aLine.append( "\n]" );

#if HAVE_FEATURE_NSS
        if (m_nSignatureObject != -1)
            aLine.append( "/SigFlags 3");
#endif

        aLine.append( "/DR " );
        aLine.append( getResourceDictObj() );
        aLine.append( " 0 R" );
        // NeedAppearances must not be used if PDF is signed
        if( m_bIsPDF_A1
#if HAVE_FEATURE_NSS
            || ( m_nSignatureObject != -1 )
#endif
            )
            aLine.append( ">>\n" );
        else
            aLine.append( "/NeedAppearances true>>\n" );
    }

    //check if there is a Metadata object
    if( nOutputIntentObject )
    {
        aLine.append("/OutputIntents[");
        aLine.append( nOutputIntentObject );
        aLine.append( " 0 R]" );
    }

    if( nMetadataObject )
    {
        aLine.append("/Metadata ");
        aLine.append( nMetadataObject );
        aLine.append( " 0 R" );
    }

    aLine.append( ">>\n"
                  "endobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    return true;
}

#if HAVE_FEATURE_NSS

bool PDFWriterImpl::emitSignature()
{
    if( !updateObject( m_nSignatureObject ) )
        return false;

    OStringBuffer aLine( 0x5000 );
    aLine.append( m_nSignatureObject );
    aLine.append( " 0 obj\n" );
    aLine.append("<</Contents <" );

    sal_uInt64 nOffset = ~0U;
    CHECK_RETURN( (osl::File::E_None == m_aFile.getPos(nOffset) ) );

    m_nSignatureContentOffset = nOffset + aLine.getLength();

    // reserve some space for the PKCS#7 object
    OStringBuffer aContentFiller( MAX_SIGNATURE_CONTENT_LENGTH );
    comphelper::string::padToLength(aContentFiller, MAX_SIGNATURE_CONTENT_LENGTH, '0');
    aLine.append( aContentFiller.makeStringAndClear() );
    aLine.append( ">\n/Type/Sig/SubFilter/adbe.pkcs7.detached");

    if( !m_aContext.DocumentInfo.Author.isEmpty() )
    {
        aLine.append( "/Name" );
        appendUnicodeTextStringEncrypt( m_aContext.DocumentInfo.Author, m_nSignatureObject, aLine );
    }

    aLine.append( " /M ");
    appendLiteralStringEncrypt( m_aCreationDateString, m_nSignatureObject, aLine );

    aLine.append( " /ByteRange [ 0 ");
    aLine.append( m_nSignatureContentOffset - 1 );
    aLine.append( " " );
    aLine.append( m_nSignatureContentOffset + MAX_SIGNATURE_CONTENT_LENGTH + 1 );
    aLine.append( " " );

    m_nSignatureLastByteRangeNoOffset = nOffset + aLine.getLength();

    // mark the last ByteRange no and add some space. Now, we don't know
    // how many bytes we need for this ByteRange value
    // The real value will be overwritten in the finalizeSignature method
    OStringBuffer aByteRangeFiller( 100  );
    comphelper::string::padToLength(aByteRangeFiller, 100, ' ');
    aLine.append( aByteRangeFiller.makeStringAndClear() );
    aLine.append("  /Filter/Adobe.PPKMS");

    //emit reason, location and contactinfo
    if ( !m_aContext.SignReason.isEmpty() )
    {
        aLine.append("/Reason");
        appendUnicodeTextStringEncrypt( m_aContext.SignReason, m_nSignatureObject, aLine );
    }

    if ( !m_aContext.SignLocation.isEmpty() )
    {
        aLine.append("/Location");
        appendUnicodeTextStringEncrypt( m_aContext.SignLocation, m_nSignatureObject, aLine );
    }

    if ( !m_aContext.SignContact.isEmpty() )
    {
        aLine.append("/ContactInfo");
        appendUnicodeTextStringEncrypt( m_aContext.SignContact, m_nSignatureObject, aLine );
    }

    aLine.append(" >>\nendobj\n\n" );

    if (!writeBuffer( aLine.getStr(), aLine.getLength() ))
        return false;

    return true;
}

bool PDFWriterImpl::finalizeSignature()
{
    if (!m_aContext.SignCertificate.is())
        return false;

    // 1- calculate last ByteRange value
    sal_uInt64 nOffset = ~0U;
    CHECK_RETURN( (osl::File::E_None == m_aFile.getPos(nOffset) ) );

    sal_Int64 nLastByteRangeNo = nOffset - (m_nSignatureContentOffset + MAX_SIGNATURE_CONTENT_LENGTH + 1);

    // 2- overwrite the value to the m_nSignatureLastByteRangeNoOffset position
    sal_uInt64 nWritten = 0;
    CHECK_RETURN( (osl::File::E_None == m_aFile.setPos(osl_Pos_Absolut, m_nSignatureLastByteRangeNoOffset) ) );
    OStringBuffer aByteRangeNo( 256 );
    aByteRangeNo.append( nLastByteRangeNo );
    aByteRangeNo.append( " ]" );

    if (m_aFile.write(aByteRangeNo.getStr(), aByteRangeNo.getLength(), nWritten) != osl::File::E_None)
    {
        CHECK_RETURN( (osl::File::E_None == m_aFile.setPos(osl_Pos_Absolut, nOffset)) );
        return false;
    }

    // 3- create the PKCS#7 object using NSS

    // Prepare buffer and calculate PDF file digest
    CHECK_RETURN( (osl::File::E_None == m_aFile.setPos(osl_Pos_Absolut, 0)) );

    std::unique_ptr<char[]> buffer1(new char[m_nSignatureContentOffset + 1]);
    sal_uInt64 bytesRead1;

    //FIXME: Check if hash is calculated from the correct byterange
    if (osl::File::E_None != m_aFile.read(buffer1.get(), m_nSignatureContentOffset - 1 , bytesRead1) ||
        bytesRead1 != static_cast<sal_uInt64>(m_nSignatureContentOffset) - 1)
    {
        SAL_WARN("vcl.pdfwriter", "First buffer read failed");
        return false;
    }

    std::unique_ptr<char[]> buffer2(new char[nLastByteRangeNo + 1]);
    sal_uInt64 bytesRead2;

    if (osl::File::E_None != m_aFile.setPos(osl_Pos_Absolut, m_nSignatureContentOffset + MAX_SIGNATURE_CONTENT_LENGTH + 1) ||
        osl::File::E_None != m_aFile.read(buffer2.get(), nLastByteRangeNo, bytesRead2) ||
        bytesRead2 != static_cast<sal_uInt64>(nLastByteRangeNo))
    {
        SAL_WARN("vcl.pdfwriter", "Second buffer read failed");
        return false;
    }

    OStringBuffer aCMSHexBuffer;
    svl::crypto::Signing aSigning(m_aContext.SignCertificate);
    aSigning.AddDataRange(buffer1.get(), bytesRead1);
    aSigning.AddDataRange(buffer2.get(), bytesRead2);
    aSigning.SetSignTSA(m_aContext.SignTSA);
    aSigning.SetSignPassword(m_aContext.SignPassword);
    if (!aSigning.Sign(aCMSHexBuffer))
    {
        SAL_WARN("vcl.pdfwriter", "PDFWriter::Sign() failed");
        return false;
    }

    assert(aCMSHexBuffer.getLength() <= MAX_SIGNATURE_CONTENT_LENGTH);

    // Set file pointer to the m_nSignatureContentOffset, we're ready to overwrite PKCS7 object
    nWritten = 0;
    CHECK_RETURN( (osl::File::E_None == m_aFile.setPos(osl_Pos_Absolut, m_nSignatureContentOffset)) );
    m_aFile.write(aCMSHexBuffer.getStr(), aCMSHexBuffer.getLength(), nWritten);

    CHECK_RETURN( (osl::File::E_None == m_aFile.setPos(osl_Pos_Absolut, nOffset)) );
    return true;
}

#endif //HAVE_FEATURE_NSS

sal_Int32 PDFWriterImpl::emitInfoDict( )
{
    sal_Int32 nObject = createObject();

    if( updateObject( nObject ) )
    {
        OStringBuffer aLine( 1024 );
        aLine.append( nObject );
        aLine.append( " 0 obj\n"
                      "<<" );
        if( !m_aContext.DocumentInfo.Title.isEmpty() )
        {
            aLine.append( "/Title" );
            appendUnicodeTextStringEncrypt( m_aContext.DocumentInfo.Title, nObject, aLine );
            aLine.append( "\n" );
        }
        if( !m_aContext.DocumentInfo.Author.isEmpty() )
        {
            aLine.append( "/Author" );
            appendUnicodeTextStringEncrypt( m_aContext.DocumentInfo.Author, nObject, aLine );
            aLine.append( "\n" );
        }
        if( !m_aContext.DocumentInfo.Subject.isEmpty() )
        {
            aLine.append( "/Subject" );
            appendUnicodeTextStringEncrypt( m_aContext.DocumentInfo.Subject, nObject, aLine );
            aLine.append( "\n" );
        }
        if( !m_aContext.DocumentInfo.Keywords.isEmpty() )
        {
            aLine.append( "/Keywords" );
            appendUnicodeTextStringEncrypt( m_aContext.DocumentInfo.Keywords, nObject, aLine );
            aLine.append( "\n" );
        }
        if( !m_aContext.DocumentInfo.Creator.isEmpty() )
        {
            aLine.append( "/Creator" );
            appendUnicodeTextStringEncrypt( m_aContext.DocumentInfo.Creator, nObject, aLine );
            aLine.append( "\n" );
        }
        if( !m_aContext.DocumentInfo.Producer.isEmpty() )
        {
            aLine.append( "/Producer" );
            appendUnicodeTextStringEncrypt( m_aContext.DocumentInfo.Producer, nObject, aLine );
            aLine.append( "\n" );
        }

         aLine.append( "/CreationDate" );
         appendLiteralStringEncrypt( m_aCreationDateString, nObject, aLine );
        aLine.append( ">>\nendobj\n\n" );
        if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
            nObject = 0;
    }
    else
        nObject = 0;

    return nObject;
}

// Part of this function may be shared with method appendDest.
sal_Int32 PDFWriterImpl::emitNamedDestinations()
{
    sal_Int32  nCount = m_aNamedDests.size();
    if( nCount <= 0 )
        return 0;//define internal error

    //get the object number for all the destinations
    sal_Int32 nObject = createObject();

    if( updateObject( nObject ) )
    {
        //emit the dictionary
        OStringBuffer aLine( 1024 );
        aLine.append( nObject );
        aLine.append( " 0 obj\n"
                      "<<" );

        sal_Int32  nDestID;
        for( nDestID = 0; nDestID < nCount; nDestID++ )
        {
            const PDFNamedDest& rDest   = m_aNamedDests[ nDestID ];
            // In order to correctly function both under an Internet browser and
            // directly with a reader (provided the reader has the feature) we
            // need to set the name of the destination the same way it will be encoded
            // in an Internet link
            INetURLObject aLocalURL( "http://ahost.ax" ); //dummy location, won't be used
            aLocalURL.SetMark( rDest.m_aDestName );

            const OUString aName   = aLocalURL.GetMark( INetURLObject::DecodeMechanism::NONE ); //same coding as
            // in link creation ( see PDFWriterImpl::emitLinkAnnotations )
            const PDFPage& rDestPage    = m_aPages[ rDest.m_nPage ];

            aLine.append( '/' );
            appendDestinationName( aName, aLine ); // this conversion must be done when forming the link to target ( see in emitCatalog )
            aLine.append( '[' ); // the '[' can be emitted immediately, because the appendDestinationName function
                                 //maps the preceding character properly
            aLine.append( rDestPage.m_nPageObject );
            aLine.append( " 0 R" );

            switch( rDest.m_eType )
            {
            case PDFWriter::DestAreaType::XYZ:
            default:
                aLine.append( "/XYZ " );
                appendFixedInt( rDest.m_aRect.Left(), aLine );
                aLine.append( ' ' );
                appendFixedInt( rDest.m_aRect.Bottom(), aLine );
                aLine.append( " 0" );
                break;
            case PDFWriter::DestAreaType::FitRectangle:
                aLine.append( "/FitR " );
                appendFixedInt( rDest.m_aRect.Left(), aLine );
                aLine.append( ' ' );
                appendFixedInt( rDest.m_aRect.Top(), aLine );
                aLine.append( ' ' );
                appendFixedInt( rDest.m_aRect.Right(), aLine );
                aLine.append( ' ' );
                appendFixedInt( rDest.m_aRect.Bottom(), aLine );
                break;
            }
            aLine.append( "]\n" );
        }

        //close
        aLine.append( ">>\nendobj\n\n" );
        if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
            nObject = 0;
    }
    else
        nObject = 0;

    return nObject;
}

// emits the output intent dictionary
sal_Int32 PDFWriterImpl::emitOutputIntent()
{
    if( !m_bIsPDF_A1 )
        return 0;

    //emit the sRGB standard profile, in ICC format, in a stream, per IEC61966-2.1

    OStringBuffer aLine( 1024 );
    sal_Int32 nICCObject = createObject();
    sal_Int32 nStreamLengthObject = createObject();

    aLine.append( nICCObject );
// sRGB has 3 colors, hence /N 3 below (PDF 1.4 table 4.16)
    aLine.append( " 0 obj\n<</N 3/Length " );
    aLine.append( nStreamLengthObject );
    aLine.append( " 0 R" );
    if (!g_bDebugDisableCompression)
        aLine.append( "/Filter/FlateDecode" );
    aLine.append( ">>\nstream\n" );
    if ( !updateObject( nICCObject ) ) return 0;
    if ( !writeBuffer( aLine.getStr(), aLine.getLength() ) ) return 0;
    //get file position
    sal_uInt64 nBeginStreamPos = 0;
    m_aFile.getPos(nBeginStreamPos);
    beginCompression();
    checkAndEnableStreamEncryption( nICCObject );
    cmsHPROFILE hProfile = cmsCreate_sRGBProfile();
    //force ICC profile version 2.1
    cmsSetProfileVersion(hProfile, 2.1);
    cmsUInt32Number nBytesNeeded = 0;
    cmsSaveProfileToMem(hProfile, nullptr, &nBytesNeeded);
    if (!nBytesNeeded)
      return 0;
    std::vector<unsigned char> aBuffer(nBytesNeeded);
    cmsSaveProfileToMem(hProfile, &aBuffer[0], &nBytesNeeded);
    cmsCloseProfile(hProfile);
    bool written = writeBuffer( &aBuffer[0], static_cast<sal_Int32>(aBuffer.size()) );
    disableStreamEncryption();
    endCompression();
    sal_uInt64 nEndStreamPos = 0;
    m_aFile.getPos(nEndStreamPos);

    if( !written )
        return 0;
    if( ! writeBuffer( "\nendstream\nendobj\n\n", 19 ) )
        return 0 ;
    aLine.setLength( 0 );

    //emit the stream length   object
    if ( !updateObject( nStreamLengthObject ) ) return 0;
    aLine.setLength( 0 );
    aLine.append( nStreamLengthObject );
    aLine.append( " 0 obj\n" );
    aLine.append( static_cast<sal_Int64>(nEndStreamPos-nBeginStreamPos) );
    aLine.append( "\nendobj\n\n" );
    if ( !writeBuffer( aLine.getStr(), aLine.getLength() ) ) return 0;
    aLine.setLength( 0 );

    //emit the OutputIntent dictionary
    sal_Int32 nOIObject = createObject();
    if ( !updateObject( nOIObject ) ) return 0;
    aLine.append( nOIObject );
    aLine.append( " 0 obj\n"
                  "<</Type/OutputIntent/S/GTS_PDFA1/OutputConditionIdentifier");

    OUString const aComment( "sRGB IEC61966-2.1"  );
    appendLiteralStringEncrypt( aComment ,nOIObject, aLine );
    aLine.append("/DestOutputProfile ");
    aLine.append( nICCObject );
    aLine.append( " 0 R>>\nendobj\n\n" );
    if ( !writeBuffer( aLine.getStr(), aLine.getLength() ) ) return 0;

    return nOIObject;
}

// formats the string for the XML stream
static void escapeStringXML( const OUString& rStr, OUString &rValue)
{
    const sal_Unicode* pUni = rStr.getStr();
    int nLen = rStr.getLength();
    for( ; nLen; nLen--, pUni++ )
    {
        switch( *pUni )
        {
        case u'&':
            rValue += "&amp;";
        break;
        case u'<':
            rValue += "&lt;";
        break;
        case u'>':
            rValue += "&gt;";
        break;
        case u'\'':
            rValue += "&apos;";
        break;
        case u'"':
            rValue += "&quot;";
        break;
        default:
            rValue += OUStringLiteral1( *pUni );
            break;
        }
    }
}

// emits the document metadata
sal_Int32 PDFWriterImpl::emitDocumentMetadata()
{
    if( !m_bIsPDF_A1 )
        return 0;

    //get the object number for all the destinations
    sal_Int32 nObject = createObject();

    if( updateObject( nObject ) )
    {
        // the following string are written in UTF-8 unicode
        OStringBuffer aMetadataStream( 8192 );

        aMetadataStream.append( "<?xpacket begin=\"" );
        // these lines write Unicode "zero width non-breaking space character" (U+FEFF)
        // (aka byte-order mark ) used as a byte-order marker.
        aMetadataStream.append( OUStringToOString( OUString( u'\xFEFF' ), RTL_TEXTENCODING_UTF8 ) );
        aMetadataStream.append( "\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>\n" );
        aMetadataStream.append( "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\">\n" );
        aMetadataStream.append( " <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n" );
        //PDF/A part ( ISO 19005-1:2005 - 6.7.11 )
        aMetadataStream.append( "  <rdf:Description rdf:about=\"\"\n" );
        aMetadataStream.append( "      xmlns:pdfaid=\"http://www.aiim.org/pdfa/ns/id/\">\n" );
        aMetadataStream.append( "   <pdfaid:part>1</pdfaid:part>\n" );
        aMetadataStream.append( "   <pdfaid:conformance>A</pdfaid:conformance>\n" );
        aMetadataStream.append( "  </rdf:Description>\n" );
        //... Dublin Core properties go here
        if( !m_aContext.DocumentInfo.Title.isEmpty() ||
            !m_aContext.DocumentInfo.Author.isEmpty() ||
            !m_aContext.DocumentInfo.Subject.isEmpty() )
        {
            aMetadataStream.append( "  <rdf:Description rdf:about=\"\"\n" );
            aMetadataStream.append( "      xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n" );
            if( !m_aContext.DocumentInfo.Title.isEmpty() )
            {
                // this is according to PDF/A-1, technical corrigendum 1 (2007-04-01)
                aMetadataStream.append( "   <dc:title>\n" );
                aMetadataStream.append( "    <rdf:Alt>\n" );
                aMetadataStream.append( "     <rdf:li xml:lang=\"x-default\">" );
                OUString aTitle;
                escapeStringXML( m_aContext.DocumentInfo.Title, aTitle );
                aMetadataStream.append( OUStringToOString( aTitle, RTL_TEXTENCODING_UTF8 )  );
                aMetadataStream.append( "</rdf:li>\n" );
                aMetadataStream.append( "    </rdf:Alt>\n" );
                aMetadataStream.append( "   </dc:title>\n" );
            }
            if( !m_aContext.DocumentInfo.Author.isEmpty() )
            {
                aMetadataStream.append( "   <dc:creator>\n" );
                aMetadataStream.append( "    <rdf:Seq>\n" );
                aMetadataStream.append( "     <rdf:li>" );
                OUString aAuthor;
                escapeStringXML( m_aContext.DocumentInfo.Author, aAuthor );
                aMetadataStream.append( OUStringToOString( aAuthor , RTL_TEXTENCODING_UTF8 )  );
                aMetadataStream.append( "</rdf:li>\n" );
                aMetadataStream.append( "    </rdf:Seq>\n" );
                aMetadataStream.append( "   </dc:creator>\n" );
            }
            if( !m_aContext.DocumentInfo.Subject.isEmpty() )
            {
                // this is according to PDF/A-1, technical corrigendum 1 (2007-04-01)
                aMetadataStream.append( "   <dc:description>\n" );
                aMetadataStream.append( "    <rdf:Alt>\n" );
                aMetadataStream.append( "     <rdf:li xml:lang=\"x-default\">" );
                OUString aSubject;
                escapeStringXML( m_aContext.DocumentInfo.Subject, aSubject );
                aMetadataStream.append( OUStringToOString( aSubject , RTL_TEXTENCODING_UTF8 )  );
                aMetadataStream.append( "</rdf:li>\n" );
                aMetadataStream.append( "    </rdf:Alt>\n" );
                aMetadataStream.append( "   </dc:description>\n" );
            }
            aMetadataStream.append( "  </rdf:Description>\n" );
        }

        //... PDF properties go here
        if( !m_aContext.DocumentInfo.Producer.isEmpty() ||
            !m_aContext.DocumentInfo.Keywords.isEmpty() )
        {
            aMetadataStream.append( "  <rdf:Description rdf:about=\"\"\n" );
            aMetadataStream.append( "     xmlns:pdf=\"http://ns.adobe.com/pdf/1.3/\">\n" );
            if( !m_aContext.DocumentInfo.Producer.isEmpty() )
            {
                aMetadataStream.append( "   <pdf:Producer>" );
                OUString aProducer;
                escapeStringXML( m_aContext.DocumentInfo.Producer, aProducer );
                aMetadataStream.append( OUStringToOString( aProducer , RTL_TEXTENCODING_UTF8 )  );
                aMetadataStream.append( "</pdf:Producer>\n" );
            }
            if( !m_aContext.DocumentInfo.Keywords.isEmpty() )
            {
                aMetadataStream.append( "   <pdf:Keywords>" );
                OUString aKeywords;
                escapeStringXML( m_aContext.DocumentInfo.Keywords, aKeywords );
                aMetadataStream.append( OUStringToOString( aKeywords , RTL_TEXTENCODING_UTF8 )  );
                aMetadataStream.append( "</pdf:Keywords>\n" );
            }
            aMetadataStream.append( "  </rdf:Description>\n" );
        }

        aMetadataStream.append( "  <rdf:Description rdf:about=\"\"\n" );
        aMetadataStream.append( "    xmlns:xmp=\"http://ns.adobe.com/xap/1.0/\">\n" );
        if( !m_aContext.DocumentInfo.Creator.isEmpty() )
        {
            aMetadataStream.append( "   <xmp:CreatorTool>" );
            OUString aCreator;
            escapeStringXML( m_aContext.DocumentInfo.Creator, aCreator );
            aMetadataStream.append( OUStringToOString( aCreator , RTL_TEXTENCODING_UTF8 )  );
            aMetadataStream.append( "</xmp:CreatorTool>\n" );
        }
        //creation date
        aMetadataStream.append( "   <xmp:CreateDate>" );
        aMetadataStream.append( m_aCreationMetaDateString );
        aMetadataStream.append( "</xmp:CreateDate>\n" );

        aMetadataStream.append( "  </rdf:Description>\n" );
        aMetadataStream.append( " </rdf:RDF>\n" );
        aMetadataStream.append( "</x:xmpmeta>\n" );

        //add the padding
        for( sal_Int32 nSpaces = 1; nSpaces <= 2100; nSpaces++ )
        {
            aMetadataStream.append( " " );
            if( nSpaces % 100 == 0 )
                aMetadataStream.append( "\n" );
        }

        aMetadataStream.append( "<?xpacket end=\"w\"?>\n" );

        OStringBuffer aMetadataObj( 1024 );

        aMetadataObj.append( nObject );
        aMetadataObj.append( " 0 obj\n" );

        aMetadataObj.append( "<</Type/Metadata/Subtype/XML/Length " );

        aMetadataObj.append( aMetadataStream.getLength() );
        aMetadataObj.append( ">>\nstream\n" );
        if ( !writeBuffer( aMetadataObj.getStr(), aMetadataObj.getLength() ) )
            return 0;
        //emit the stream
        if ( !writeBuffer( aMetadataStream.getStr(), aMetadataStream.getLength() ) )
            return 0;

        aMetadataObj.setLength( 0 );
        aMetadataObj.append( "\nendstream\nendobj\n\n" );
        if( ! writeBuffer( aMetadataObj.getStr(), aMetadataObj.getLength() ) )
            nObject = 0;
    }
    else
        nObject = 0;

    return nObject;
}

bool PDFWriterImpl::emitTrailer()
{
    // emit doc info
    sal_Int32 nDocInfoObject = emitInfoDict( );

    sal_Int32 nSecObject = 0;

    if( m_aContext.Encryption.Encrypt() )
    {
        //emit the security information
        //must be emitted as indirect dictionary object, since
        //Acrobat Reader 5 works only with this kind of implementation
        nSecObject = createObject();

        if( updateObject( nSecObject ) )
        {
            OStringBuffer aLineS( 1024 );
            aLineS.append( nSecObject );
            aLineS.append( " 0 obj\n"
                           "<</Filter/Standard/V " );
            // check the version
            aLineS.append( "2/Length 128/R 3" );

            // emit the owner password, must not be encrypted
            aLineS.append( "/O(" );
            appendLiteralString( reinterpret_cast<char*>(&m_aContext.Encryption.OValue[0]), sal_Int32(m_aContext.Encryption.OValue.size()), aLineS );
            aLineS.append( ")/U(" );
            appendLiteralString( reinterpret_cast<char*>(&m_aContext.Encryption.UValue[0]), sal_Int32(m_aContext.Encryption.UValue.size()), aLineS );
            aLineS.append( ")/P " );// the permission set
            aLineS.append( m_nAccessPermissions );
            aLineS.append( ">>\nendobj\n\n" );
            if( !writeBuffer( aLineS.getStr(), aLineS.getLength() ) )
                nSecObject = 0;
        }
        else
            nSecObject = 0;
    }
    // emit xref table
    // remember start
    sal_uInt64 nXRefOffset = 0;
    CHECK_RETURN( (osl::File::E_None == m_aFile.getPos(nXRefOffset )) );
    CHECK_RETURN( writeBuffer( "xref\n", 5 ) );

    sal_Int32 nObjects = m_aObjects.size();
    OStringBuffer aLine;
    aLine.append( "0 " );
    aLine.append( static_cast<sal_Int32>(nObjects+1) );
    aLine.append( "\n" );
    aLine.append( "0000000000 65535 f \n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    for( sal_Int32 i = 0; i < nObjects; i++ )
    {
        aLine.setLength( 0 );
        OString aOffset = OString::number( m_aObjects[i] );
        for( sal_Int32 j = 0; j < (10-aOffset.getLength()); j++ )
            aLine.append( '0' );
        aLine.append( aOffset );
        aLine.append( " 00000 n \n" );
        SAL_WARN_IF( aLine.getLength() != 20, "vcl.pdfwriter", "invalid xref entry" );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }

    // prepare document checksum
    OStringBuffer aDocChecksum( 2*RTL_DIGEST_LENGTH_MD5+1 );
    ::std::vector<unsigned char> const nMD5Sum(m_DocDigest.finalize());
    for (sal_uInt8 i : nMD5Sum)
        appendHex( i, aDocChecksum );
    // document id set in setDocInfo method
    // emit trailer
    aLine.setLength( 0 );
    aLine.append( "trailer\n"
                  "<</Size " );
    aLine.append( static_cast<sal_Int32>(nObjects+1) );
    aLine.append( "/Root " );
    aLine.append( m_nCatalogObject );
    aLine.append( " 0 R\n" );
    if( nSecObject )
    {
        aLine.append( "/Encrypt ");
        aLine.append( nSecObject );
        aLine.append( " 0 R\n" );
    }
    if( nDocInfoObject )
    {
        aLine.append( "/Info " );
        aLine.append( nDocInfoObject );
        aLine.append( " 0 R\n" );
    }
    if( ! m_aContext.Encryption.DocumentIdentifier.empty() )
    {
        aLine.append( "/ID [ <" );
        for (auto const& item : m_aContext.Encryption.DocumentIdentifier)
        {
            appendHex( sal_Int8(item), aLine );
        }
        aLine.append( ">\n"
                      "<" );
        for (auto const& item : m_aContext.Encryption.DocumentIdentifier)
        {
            appendHex( sal_Int8(item), aLine );
        }
        aLine.append( "> ]\n" );
    }
    if( !aDocChecksum.isEmpty() )
    {
        aLine.append( "/DocChecksum /" );
        aLine.append( aDocChecksum.makeStringAndClear() );
        aLine.append( "\n" );
    }
    if( m_aAdditionalStreams.size() > 0 )
    {
        aLine.append( "/AdditionalStreams [" );
        for(const PDFAddStream & rAdditionalStream : m_aAdditionalStreams)
        {
            aLine.append( "/" );
            appendName( rAdditionalStream.m_aMimeType, aLine );
            aLine.append( " " );
            aLine.append( rAdditionalStream.m_nStreamObject );
            aLine.append( " 0 R\n" );
        }
        aLine.append( "]\n" );
    }
    aLine.append( ">>\n"
                  "startxref\n" );
    aLine.append( static_cast<sal_Int64>(nXRefOffset) );
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

    explicit AnnotSorterLess( std::vector< PDFWriterImpl::PDFWidget >& rWidgets ) : m_rWidgets( rWidgets ) {}

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
    std::unordered_map< sal_Int32, AnnotSortContainer > sorted;
    int nWidgets = m_aWidgets.size();
    for( int nW = 0; nW < nWidgets; nW++ )
    {
        const PDFWidget& rWidget = m_aWidgets[nW];
        if( rWidget.m_nPage >= 0 )
        {
            AnnotSortContainer& rCont = sorted[ rWidget.m_nPage ];
            // optimize vector allocation
            if( rCont.aSortedAnnots.empty() )
                rCont.aSortedAnnots.reserve( m_aPages[ rWidget.m_nPage ].m_aAnnotations.size() );
            // insert widget to tab sorter
            // RadioButtons are not page annotations, only their individual check boxes are
            if( rWidget.m_eType != PDFWriter::RadioButton )
            {
                rCont.aObjects.insert( rWidget.m_nObject );
                rCont.aSortedAnnots.emplace_back( rWidget.m_nTabOrder, rWidget.m_nObject, nW );
            }
        }
    }
    for (auto & item : sorted)
    {
        // append entries for non widget annotations
        PDFPage& rPage = m_aPages[ item.first ];
        unsigned int nAnnots = rPage.m_aAnnotations.size();
        for( unsigned int nA = 0; nA < nAnnots; nA++ )
            if( item.second.aObjects.find( rPage.m_aAnnotations[nA] ) == item.second.aObjects.end())
                item.second.aSortedAnnots.emplace_back( 10000, rPage.m_aAnnotations[nA], -1 );

        AnnotSorterLess aLess( m_aWidgets );
        std::stable_sort( item.second.aSortedAnnots.begin(), item.second.aSortedAnnots.end(), aLess );
        // sanity check
        if( item.second.aSortedAnnots.size() == nAnnots)
        {
            for( unsigned int nA = 0; nA < nAnnots; nA++ )
                rPage.m_aAnnotations[nA] = item.second.aSortedAnnots[nA].nObject;
        }
        else
        {
            SAL_WARN( "vcl.pdfwriter", "wrong number of sorted annotations" );
            SAL_INFO("vcl.pdfwriter", "PDFWriterImpl::sortWidgets(): wrong number of sorted assertions "
                     "on page nr " << item.first << ", " <<
                     static_cast<long int>(item.second.aSortedAnnots.size()) << " sorted and " <<
                     static_cast<long int>(nAnnots) << " unsorted");
        }
    }

    // FIXME: implement tab order in structure tree for PDF 1.5
}

namespace vcl {
class PDFStreamIf :
        public cppu::WeakImplHelper< css::io::XOutputStream >
{
    PDFWriterImpl*  m_pWriter;
    bool            m_bWrite;
    public:
    explicit PDFStreamIf( PDFWriterImpl* pWriter ) : m_pWriter( pWriter ), m_bWrite( true ) {}

    virtual void SAL_CALL writeBytes( const css::uno::Sequence< sal_Int8 >& aData ) override;
    virtual void SAL_CALL flush() override;
    virtual void SAL_CALL closeOutput() override;
};
}

void SAL_CALL  PDFStreamIf::writeBytes( const css::uno::Sequence< sal_Int8 >& aData )
{
    if( m_bWrite && aData.getLength() )
    {
        sal_Int32 nBytes = aData.getLength();
        m_pWriter->writeBuffer( aData.getConstArray(), nBytes );
    }
}

void SAL_CALL PDFStreamIf::flush()
{
}

void SAL_CALL PDFStreamIf::closeOutput()
{
    m_bWrite = false;
}

bool PDFWriterImpl::emitAdditionalStreams()
{
    unsigned int nStreams = m_aAdditionalStreams.size();
    for( unsigned int i = 0; i < nStreams; i++ )
    {
        PDFAddStream& rStream = m_aAdditionalStreams[i];
        rStream.m_nStreamObject = createObject();
        sal_Int32 nSizeObject = createObject();

        if( ! updateObject( rStream.m_nStreamObject ) )
            return false;

        OStringBuffer aLine;
        aLine.append( rStream.m_nStreamObject );
        aLine.append( " 0 obj\n<</Length " );
        aLine.append( nSizeObject );
        aLine.append( " 0 R" );
        if( rStream.m_bCompress )
            aLine.append( "/Filter/FlateDecode" );
        aLine.append( ">>\nstream\n" );
        if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
            return false;
        sal_uInt64 nBeginStreamPos = 0, nEndStreamPos = 0;
        if( osl::File::E_None != m_aFile.getPos(nBeginStreamPos) )
        {
            m_aFile.close();
            m_bOpen = false;
        }
        if( rStream.m_bCompress )
            beginCompression();

        checkAndEnableStreamEncryption( rStream.m_nStreamObject );
        css::uno::Reference< css::io::XOutputStream > xStream( new PDFStreamIf( this ) );
        assert(rStream.m_pStream);
        if (!rStream.m_pStream)
            return false;
        rStream.m_pStream->write( xStream );
        xStream.clear();
        delete rStream.m_pStream;
        rStream.m_pStream = nullptr;
        disableStreamEncryption();

        if( rStream.m_bCompress )
            endCompression();

        if (osl::File::E_None != m_aFile.getPos(nEndStreamPos))
        {
            m_aFile.close();
            m_bOpen = false;
            return false;
        }
        if( ! writeBuffer( "\nendstream\nendobj\n\n", 19 ) )
            return false ;
        // emit stream length object
        if( ! updateObject( nSizeObject ) )
            return false;
        aLine.setLength( 0 );
        aLine.append( nSizeObject );
        aLine.append( " 0 obj\n" );
        aLine.append( static_cast<sal_Int64>(nEndStreamPos-nBeginStreamPos) );
        aLine.append( "\nendobj\n\n" );
        if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
            return false;
    }
    return true;
}

bool PDFWriterImpl::emit()
{
    endPage();

    // resort structure tree and annotations if necessary
    // needed for widget tab order
    sortWidgets();

#if HAVE_FEATURE_NSS
    if( m_aContext.SignPDF )
    {
        // sign the document
        PDFWriter::SignatureWidget aSignature;
        aSignature.Name = "Signature1";
        createControl( aSignature, 0 );
    }
#endif

    // emit additional streams
    CHECK_RETURN( emitAdditionalStreams() );

    // emit catalog
    CHECK_RETURN( emitCatalog() );

#if HAVE_FEATURE_NSS
    if (m_nSignatureObject != -1) // if document is signed, emit sigdict
    {
        if( !emitSignature() )
        {
            m_aErrors.insert( PDFWriter::Error_Signature_Failed );
            return false;
        }
    }
#endif

    // emit trailer
    CHECK_RETURN( emitTrailer() );

#if HAVE_FEATURE_NSS
    if (m_nSignatureObject != -1) // finalize the signature
    {
        if( !finalizeSignature() )
        {
            m_aErrors.insert( PDFWriter::Error_Signature_Failed );
            return false;
        }
    }
#endif

    m_aFile.close();
    m_bOpen = false;

    return true;
}


sal_Int32 PDFWriterImpl::getSystemFont( const vcl::Font& i_rFont )
{
    getReferenceDevice()->Push();
    getReferenceDevice()->SetFont( i_rFont );
    getReferenceDevice()->ImplNewFont();

    const PhysicalFontFace* pDevFont = m_pReferenceDevice->mpFontInstance->GetFontFace();
    sal_Int32 nFontID = 0;
    FontEmbedData::iterator it = m_aSystemFonts.find( pDevFont );
    if( it != m_aSystemFonts.end() )
        nFontID = it->second.m_nNormalFontID;
    else
    {
        nFontID = m_nNextFID++;
        m_aSystemFonts[ pDevFont ] = EmbedFont();
        m_aSystemFonts[ pDevFont ].m_nNormalFontID = nFontID;
    }

    getReferenceDevice()->Pop();
    getReferenceDevice()->ImplNewFont();

    return nFontID;
}

void PDFWriterImpl::registerGlyph(const GlyphItem* pGlyph,
                                  const PhysicalFontFace* pFont,
                                  const std::vector<sal_Ucs>& rCodeUnits,
                                  sal_uInt8& nMappedGlyph,
                                  sal_Int32& nMappedFontObject)
{
    const int nFontGlyphId = pGlyph->maGlyphId;
    FontSubset& rSubset = m_aSubsets[ pFont ];
    // search for font specific glyphID
    FontMapping::iterator it = rSubset.m_aMapping.find( nFontGlyphId );
    if( it != rSubset.m_aMapping.end() )
    {
        nMappedFontObject = it->second.m_nFontID;
        nMappedGlyph = it->second.m_nSubsetGlyphID;
    }
    else
    {
        // create new subset if necessary
        if( rSubset.m_aSubsets.empty()
        || (rSubset.m_aSubsets.back().m_aMapping.size() > 254) )
        {
            rSubset.m_aSubsets.emplace_back( m_nNextFID++ );
        }

        // copy font id
        nMappedFontObject = rSubset.m_aSubsets.back().m_nFontID;
        // create new glyph in subset
        sal_uInt8 nNewId = sal::static_int_cast<sal_uInt8>(rSubset.m_aSubsets.back().m_aMapping.size()+1);
        nMappedGlyph = nNewId;

        // add new glyph to emitted font subset
        GlyphEmit& rNewGlyphEmit = rSubset.m_aSubsets.back().m_aMapping[ nFontGlyphId ];
        rNewGlyphEmit.setGlyphId( nNewId );
        for (const auto nCode : rCodeUnits)
            rNewGlyphEmit.addCode(nCode);

        // add new glyph to font mapping
        Glyph& rNewGlyph = rSubset.m_aMapping[ nFontGlyphId ];
        rNewGlyph.m_nFontID = nMappedFontObject;
        rNewGlyph.m_nSubsetGlyphID = nNewId;
    }
}

void PDFWriterImpl::drawRelief( SalLayout& rLayout, const OUString& rText, bool bTextLines )
{
    push( PushFlags::ALL );

    FontRelief eRelief = m_aCurrentPDFState.m_aFont.GetRelief();

    Color aTextColor = m_aCurrentPDFState.m_aFont.GetColor();
    Color aTextLineColor = m_aCurrentPDFState.m_aTextLineColor;
    Color aOverlineColor = m_aCurrentPDFState.m_aOverlineColor;
    Color aReliefColor( COL_LIGHTGRAY );
    if( aTextColor == COL_BLACK )
        aTextColor = COL_WHITE;
    if( aTextLineColor == COL_BLACK )
        aTextLineColor = COL_WHITE;
    if( aOverlineColor == COL_BLACK )
        aOverlineColor = COL_WHITE;
    if( aTextColor == COL_WHITE )
        aReliefColor = COL_BLACK;

    Font aSetFont = m_aCurrentPDFState.m_aFont;
    aSetFont.SetRelief( FontRelief::NONE );
    aSetFont.SetShadow( false );

    aSetFont.SetColor( aReliefColor );
    setTextLineColor( aReliefColor );
    setOverlineColor( aReliefColor );
    setFont( aSetFont );
    long nOff = 1 + getReferenceDevice()->mnDPIX/300;
    if( eRelief == FontRelief::Engraved )
        nOff = -nOff;

    rLayout.DrawOffset() += Point( nOff, nOff );
    updateGraphicsState();
    drawLayout( rLayout, rText, bTextLines );

    rLayout.DrawOffset() -= Point( nOff, nOff );
    setTextLineColor( aTextLineColor );
    setOverlineColor( aOverlineColor );
    aSetFont.SetColor( aTextColor );
    setFont( aSetFont );
    updateGraphicsState();
    drawLayout( rLayout, rText, bTextLines );

    // clean up the mess
    pop();
}

void PDFWriterImpl::drawShadow( SalLayout& rLayout, const OUString& rText, bool bTextLines )
{
    Font aSaveFont = m_aCurrentPDFState.m_aFont;
    Color aSaveTextLineColor = m_aCurrentPDFState.m_aTextLineColor;
    Color aSaveOverlineColor = m_aCurrentPDFState.m_aOverlineColor;

    Font& rFont = m_aCurrentPDFState.m_aFont;
    if( rFont.GetColor() == COL_BLACK || rFont.GetColor().GetLuminance() < 8 )
        rFont.SetColor( COL_LIGHTGRAY );
    else
        rFont.SetColor( COL_BLACK );
    rFont.SetShadow( false );
    rFont.SetOutline( false );
    setFont( rFont );
    setTextLineColor( rFont.GetColor() );
    setOverlineColor( rFont.GetColor() );
    updateGraphicsState();

    long nOff = 1 + ((m_pReferenceDevice->mpFontInstance->mnLineHeight-24)/24);
    if( rFont.IsOutline() )
        nOff++;
    rLayout.DrawBase() += Point( nOff, nOff );
    drawLayout( rLayout, rText, bTextLines );
    rLayout.DrawBase() -= Point( nOff, nOff );

    setFont( aSaveFont );
    setTextLineColor( aSaveTextLineColor );
    setOverlineColor( aSaveOverlineColor );
    updateGraphicsState();
}

void PDFWriterImpl::drawVerticalGlyphs(
        const std::vector<PDFWriterImpl::PDFGlyph>& rGlyphs,
        OStringBuffer& rLine,
        const Point& rAlignOffset,
        const Matrix3& rRotScale,
        double fAngle,
        double fXScale,
        double fSkew,
        sal_Int32 nFontHeight )
{
    long nXOffset = 0;
    Point aCurPos( rGlyphs[0].m_aPos );
    aCurPos = m_pReferenceDevice->PixelToLogic( aCurPos );
    aCurPos += rAlignOffset;
    for( size_t i = 0; i < rGlyphs.size(); i++ )
    {
        // have to emit each glyph on its own
        double fDeltaAngle = 0.0;
        double fYScale = 1.0;
        double fTempXScale = fXScale;
        double fSkewB = fSkew;
        double fSkewA = 0.0;

        Point aDeltaPos;
        if (rGlyphs[i].m_pGlyph->IsVertical())
        {
            fDeltaAngle = M_PI/2.0;
            aDeltaPos.setX( m_pReferenceDevice->GetFontMetric().GetAscent() );
            aDeltaPos.setY( static_cast<int>(static_cast<double>(m_pReferenceDevice->GetFontMetric().GetDescent()) * fXScale) );
            fYScale = fXScale;
            fTempXScale = 1.0;
            fSkewA = -fSkewB;
            fSkewB = 0.0;
        }
        aDeltaPos += (m_pReferenceDevice->PixelToLogic( Point( static_cast<int>(static_cast<double>(nXOffset)/fXScale), 0 ) ) - m_pReferenceDevice->PixelToLogic( Point() ) );
        if( i < rGlyphs.size()-1 )
        // #i120627# the text on the Y axis is reversed when export ppt file to PDF format
        {
            long nOffsetX = rGlyphs[i+1].m_aPos.X() - rGlyphs[i].m_aPos.X();
            long nOffsetY = rGlyphs[i+1].m_aPos.Y() - rGlyphs[i].m_aPos.Y();
            nXOffset += static_cast<int>(sqrt(double(nOffsetX*nOffsetX + nOffsetY*nOffsetY)));
        }
        if( ! rGlyphs[i].m_pGlyph->maGlyphId )
            continue;

        aDeltaPos = rRotScale.transform( aDeltaPos );

        Matrix3 aMat;
        if( fSkewB != 0.0 || fSkewA != 0.0 )
            aMat.skew( fSkewA, fSkewB );
        aMat.scale( fTempXScale, fYScale );
        aMat.rotate( fAngle+fDeltaAngle );
        aMat.translate( aCurPos.X()+aDeltaPos.X(), aCurPos.Y()+aDeltaPos.Y() );
        aMat.append( m_aPages.back(), rLine );
        rLine.append( " Tm" );
        if( i == 0 || rGlyphs[i-1].m_nMappedFontId != rGlyphs[i].m_nMappedFontId )
        {
            rLine.append( " /F" );
            rLine.append( rGlyphs[i].m_nMappedFontId );
            rLine.append( ' ' );
            m_aPages.back().appendMappedLength( nFontHeight, rLine );
            rLine.append( " Tf" );
        }
        rLine.append( "<" );
        appendHex( rGlyphs[i].m_nMappedGlyphId, rLine );
        rLine.append( ">Tj\n" );
    }
}

void PDFWriterImpl::drawHorizontalGlyphs(
        const std::vector<PDFWriterImpl::PDFGlyph>& rGlyphs,
        OStringBuffer& rLine,
        const Point& rAlignOffset,
        bool bFirst,
        double fAngle,
        double fXScale,
        double fSkew,
        sal_Int32 nFontHeight,
        sal_Int32 nPixelFontHeight
        )
{
    // horizontal (= normal) case

    // fill in  run end indices
    // end is marked by index of the first glyph of the next run
    // a run is marked by same mapped font id and same Y position
    std::vector< sal_uInt32 > aRunEnds;
    aRunEnds.reserve( rGlyphs.size() );
    for( size_t i = 1; i < rGlyphs.size(); i++ )
    {
        if( rGlyphs[i].m_nMappedFontId != rGlyphs[i-1].m_nMappedFontId ||
            rGlyphs[i].m_aPos.Y() != rGlyphs[i-1].m_aPos.Y() )
        {
            aRunEnds.push_back(i);
        }
    }
    // last run ends at last glyph
    aRunEnds.push_back( rGlyphs.size() );

    // loop over runs of the same font
    sal_uInt32 nBeginRun = 0;
    for( size_t nRun = 0; nRun < aRunEnds.size(); nRun++ )
    {
        // setup text matrix
        Point aCurPos = rGlyphs[nBeginRun].m_aPos;
        // back transformation to current coordinate system
        aCurPos = m_pReferenceDevice->PixelToLogic( aCurPos );
        aCurPos += rAlignOffset;
        // the first run can be set with "Td" operator
        // subsequent use of that operator would move
        // the textline matrix relative to what was set before
        // making use of that would drive us into rounding issues
        Matrix3 aMat;
        if( bFirst && nRun == 0 && fAngle == 0.0 && fXScale == 1.0 && fSkew == 0.0 )
        {
            m_aPages.back().appendPoint( aCurPos, rLine );
            rLine.append( " Td " );
        }
        else
        {
            if( fSkew != 0.0 )
                aMat.skew( 0.0, fSkew );
            aMat.scale( fXScale, 1.0 );
            aMat.rotate( fAngle );
            aMat.translate( aCurPos.X(), aCurPos.Y() );
            aMat.append( m_aPages.back(), rLine );
            rLine.append( " Tm\n" );
        }
        // set up correct font
        rLine.append( "/F" );
        rLine.append( rGlyphs[nBeginRun].m_nMappedFontId );
        rLine.append( ' ' );
        m_aPages.back().appendMappedLength( nFontHeight, rLine );
        rLine.append( " Tf" );

        // output glyphs using Tj or TJ
        OStringBuffer aKernedLine( 256 ), aUnkernedLine( 256 );
        aKernedLine.append( "[<" );
        aUnkernedLine.append( '<' );
        appendHex( rGlyphs[nBeginRun].m_nMappedGlyphId, aKernedLine );
        appendHex( rGlyphs[nBeginRun].m_nMappedGlyphId, aUnkernedLine );

        aMat.invert();
        bool bNeedKern = false;
        for( sal_uInt32 nPos = nBeginRun+1; nPos < aRunEnds[nRun]; nPos++ )
        {
            appendHex( rGlyphs[nPos].m_nMappedGlyphId, aUnkernedLine );
            // check if default glyph positioning is sufficient
            const Point aThisPos = aMat.transform( rGlyphs[nPos].m_aPos );
            const Point aPrevPos = aMat.transform( rGlyphs[nPos-1].m_aPos );
            double fAdvance = aThisPos.X() - aPrevPos.X();
            fAdvance *= 1000.0 / nPixelFontHeight;
            const sal_Int32 nAdjustment = static_cast<sal_Int32>(rGlyphs[nPos-1].m_nNativeWidth - fAdvance + 0.5);
            if( nAdjustment != 0 )
            {
                // apply individual glyph positioning
                bNeedKern = true;
                aKernedLine.append( ">" );
                aKernedLine.append( nAdjustment );
                aKernedLine.append( "<" );
            }
            appendHex( rGlyphs[nPos].m_nMappedGlyphId, aKernedLine );
        }
        aKernedLine.append( ">]TJ\n" );
        aUnkernedLine.append( ">Tj\n" );
        rLine.append(
            (bNeedKern ? aKernedLine : aUnkernedLine).makeStringAndClear() );

        // set beginning of next run
        nBeginRun = aRunEnds[nRun];
    }
}

void PDFWriterImpl::drawLayout( SalLayout& rLayout, const OUString& rText, bool bTextLines )
{
    // relief takes precedence over shadow (see outdev3.cxx)
    if(  m_aCurrentPDFState.m_aFont.GetRelief() != FontRelief::NONE )
    {
        drawRelief( rLayout, rText, bTextLines );
        return;
    }
    else if( m_aCurrentPDFState.m_aFont.IsShadow() )
        drawShadow( rLayout, rText, bTextLines );

    OStringBuffer aLine( 512 );

    const int nMaxGlyphs = 256;

    const GlyphItem* pGlyph = nullptr;
    const PhysicalFontFace* pFallbackFont = nullptr;
    std::vector<sal_Ucs> aCodeUnits;
    bool bVertical = m_aCurrentPDFState.m_aFont.IsVertical();
    int nIndex = 0;
    double fXScale = 1.0;
    double fSkew = 0.0;
    sal_Int32 nPixelFontHeight = m_pReferenceDevice->mpFontInstance->GetFontSelectPattern().mnHeight;
    TextAlign eAlign = m_aCurrentPDFState.m_aFont.GetAlignment();

    // transform font height back to current units
    // note: the layout calculates in outdevs device pixel !!
    sal_Int32 nFontHeight = m_pReferenceDevice->ImplDevicePixelToLogicHeight( nPixelFontHeight );
    if( m_aCurrentPDFState.m_aFont.GetAverageFontWidth() )
    {
        Font aFont( m_aCurrentPDFState.m_aFont );
        aFont.SetAverageFontWidth( 0 );
        FontMetric aMetric = m_pReferenceDevice->GetFontMetric( aFont );
        if( aMetric.GetAverageFontWidth() != m_aCurrentPDFState.m_aFont.GetAverageFontWidth() )
        {
            fXScale =
                static_cast<double>(m_aCurrentPDFState.m_aFont.GetAverageFontWidth()) /
                static_cast<double>(aMetric.GetAverageFontWidth());
        }
        // force state before GetFontMetric
        m_pReferenceDevice->ImplNewFont();
    }

    // perform artificial italics if necessary
    if( ( m_aCurrentPDFState.m_aFont.GetItalic() == ITALIC_NORMAL ||
          m_aCurrentPDFState.m_aFont.GetItalic() == ITALIC_OBLIQUE ) &&
        !( m_pReferenceDevice->mpFontInstance->GetFontFace()->GetItalic() == ITALIC_NORMAL ||
           m_pReferenceDevice->mpFontInstance->GetFontFace()->GetItalic() == ITALIC_OBLIQUE )
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
    double fAngle = static_cast<double>(nAngle) * M_PI / 1800.0;

    Matrix3 aRotScale;
    aRotScale.scale( fXScale, 1.0 );
    if( fAngle != 0.0 )
        aRotScale.rotate( -fAngle );

    bool bPop = false;
    bool bABold = false;
    // artificial bold necessary ?
    if( m_pReferenceDevice->mpFontInstance->GetFontFace()->GetWeight() <= WEIGHT_MEDIUM &&
        m_pReferenceDevice->mpFontInstance->GetFontSelectPattern().GetWeight() > WEIGHT_MEDIUM )
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
        aNonStrokeColor = COL_WHITE;
    }
    else
        aNonStrokeColor = m_aCurrentPDFState.m_aFont.GetColor();
    if( bABold )
        aStrokeColor = m_aCurrentPDFState.m_aFont.GetColor();

    if( aStrokeColor != COL_TRANSPARENT && aStrokeColor != m_aCurrentPDFState.m_aLineColor )
    {
        if( ! bPop )
            aLine.append( "q " );
        bPop = true;
        appendStrokingColor( aStrokeColor, aLine );
        aLine.append( "\n" );
    }
    if( aNonStrokeColor != COL_TRANSPARENT && aNonStrokeColor != m_aCurrentPDFState.m_aFillColor )
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
            double fW = static_cast<double>(m_aCurrentPDFState.m_aFont.GetFontHeight()) / 30.0;
            m_aPages.back().appendMappedLength( fW, aLine );
            aLine.append ( " w\n" );
        }
    }

    FontMetric aRefDevFontMetric = m_pReferenceDevice->GetFontMetric();
    const PhysicalFontFace* pDevFont = m_pReferenceDevice->mpFontInstance->GetFontFace();

    // collect the glyphs into a single array
    std::vector< PDFGlyph > aGlyphs;
    aGlyphs.reserve( nMaxGlyphs );
    // first get all the glyphs and register them; coordinates still in Pixel
    Point aPos;
    while (rLayout.GetNextGlyph(&pGlyph, aPos, nIndex, &pFallbackFont))
    {
        const auto* pFont = pFallbackFont ? pFallbackFont : pDevFont;

        aCodeUnits.clear();

        // tdf#66597, tdf#115117
        //
        // Here is how we embed textual content in PDF files, to allow for
        // better text extraction for complex and typography-rich text.
        //
        // * If there is many to one or many to many mapping, use an
        //   ActualText span embedding the original string, since ToUnicode
        //   can’t handle these.
        // * If the one glyph is used for several Unicode code points, also
        //   use ActualText since ToUnicode can map each glyph in the font
        //   only once.
        // * Limit ActualText to single cluster at a time, since using it
        //   for whole words or sentences breaks text selection and
        //   highlighting in PDF viewers (there will be no way to tell
        //   which glyphs belong to which characters).
        // * Keep generating (now) redundant ToUnicode entries for
        //   compatibility with old tools not supporting ActualText.

        assert(pGlyph->mnCharCount >= 0);
        for (int n = 0; n < pGlyph->mnCharCount; n++)
            aCodeUnits.push_back(rText[pGlyph->mnCharPos + n]);

        bool bUseActualText = false;

        // If this is a start of complex cluster, use ActualText.
        if (pGlyph->IsClusterStart())
            bUseActualText = true;

        // Or part of a complex cluster, will be handled by the ActualText
        // of its cluster start.
        if (pGlyph->IsInCluster())
            assert(aCodeUnits.empty());

        // A glyph can’t have more than one ToUnicode entry, use ActualText
        // instead.
        if (!aCodeUnits.empty() && !bUseActualText)
        {
            for (const auto& rSubset : m_aSubsets[pFont].m_aSubsets)
            {
                const auto& it = rSubset.m_aMapping.find(pGlyph->maGlyphId);
                if (it != rSubset.m_aMapping.cend() && it->second.codes() != aCodeUnits)
                {
                    bUseActualText = true;
                    aCodeUnits.clear();
                }
            }
        }

        assert(!aCodeUnits.empty() || bUseActualText || pGlyph->IsInCluster());

        sal_uInt8 nMappedGlyph;
        sal_Int32 nMappedFontObject;
        registerGlyph(pGlyph, pFont, aCodeUnits, nMappedGlyph, nMappedFontObject);

        sal_Int32 nGlyphWidth = 0;
        if (m_pReferenceDevice->AcquireGraphics())
        {
            SalGraphics *pGraphics = m_pReferenceDevice->GetGraphics();
            if (pGraphics)
                nGlyphWidth = m_aFontCache.getGlyphWidth(pFont,
                                                         pGlyph->maGlyphId,
                                                         pGlyph->IsVertical(),
                                                         pGraphics);
        }

        int nCharPos = -1;
        if (bUseActualText || pGlyph->IsInCluster())
            nCharPos = pGlyph->mnCharPos;

        aGlyphs.emplace_back(aPos,
                             pGlyph,
                             nGlyphWidth,
                             nMappedFontObject,
                             nMappedGlyph,
                             nCharPos);
    }

    // Avoid fill color when map mode is in pixels, the below code assumes
    // logic map mode.
    bool bPixel = m_aCurrentPDFState.m_aMapMode.GetMapUnit() == MapUnit::MapPixel;
    if (m_aCurrentPDFState.m_aFont.GetFillColor() != COL_TRANSPARENT && !bPixel)
    {
        // PDF doesn't have a text fill color, so draw a rectangle before
        // drawing the actual text.
        push(PushFlags::FILLCOLOR | PushFlags::LINECOLOR);
        setFillColor(m_aCurrentPDFState.m_aFont.GetFillColor());
        // Avoid border around the rectangle for Writer shape text.
        setLineColor(COL_TRANSPARENT);

        // The rectangle is the bounding box of the text, but also includes
        // ascent / descent to match the on-screen rendering.
        tools::Rectangle aRectangle;
        // This is the top left of the text without ascent / descent.
        aRectangle.SetPos(m_pReferenceDevice->PixelToLogic(rLayout.GetDrawPosition()));
        aRectangle.setY(aRectangle.getY() - aRefDevFontMetric.GetAscent());
        aRectangle.SetSize(m_pReferenceDevice->PixelToLogic(Size(rLayout.GetTextWidth(), 0)));
        // This includes ascent / descent.
        aRectangle.setHeight(aRefDevFontMetric.GetLineHeight());

        LogicalFontInstance* pFontInstance = m_pReferenceDevice->mpFontInstance.get();
        if (pFontInstance->mnOrientation)
        {
            // Adapt rectangle for rotated text.
            tools::Polygon aPolygon(aRectangle);
            aPolygon.Rotate(m_pReferenceDevice->PixelToLogic(rLayout.GetDrawPosition()), pFontInstance->mnOrientation);
            drawPolygon(aPolygon);
        }
        else
            drawRectangle(aRectangle);

        pop();
    }

    Point aAlignOffset;
    if ( eAlign == ALIGN_BOTTOM )
        aAlignOffset.AdjustY( -(aRefDevFontMetric.GetDescent()) );
    else if ( eAlign == ALIGN_TOP )
        aAlignOffset.AdjustY(aRefDevFontMetric.GetAscent() );
    if( aAlignOffset.X() || aAlignOffset.Y() )
        aAlignOffset = aRotScale.transform( aAlignOffset );

    /* #159153# do not emit an empty glyph vector; this can happen if e.g. the original
       string contained only one of the UTF16 BOMs
    */
    if( ! aGlyphs.empty() )
    {
        size_t nStart = 0;
        size_t nEnd = 0;
        while (nStart < aGlyphs.size())
        {
            while (nEnd < aGlyphs.size() && aGlyphs[nEnd].m_nCharPos == aGlyphs[nStart].m_nCharPos)
                nEnd++;

            std::vector<PDFGlyph> aRun(aGlyphs.begin() + nStart, aGlyphs.begin() + nEnd);

            int nCharPos, nCharCount;
            if (!aRun.front().m_pGlyph->IsRTLGlyph())
            {
                nCharPos = aRun.front().m_nCharPos;
                nCharCount = aRun.front().m_pGlyph->mnCharCount;
            }
            else
            {
                nCharPos = aRun.back().m_nCharPos;
                nCharCount = aRun.back().m_pGlyph->mnCharCount;
            }

            if (nCharPos >= 0 && nCharCount)
            {
                aLine.append("/Span<</ActualText<FEFF");
                for (int i = 0; i < nCharCount; i++)
                {
                    sal_Unicode aChar = rText[nCharPos + i];
                    appendHex(static_cast<sal_Int8>(aChar >> 8), aLine);
                    appendHex(static_cast<sal_Int8>(aChar & 255), aLine);
                }
                aLine.append( ">>>\nBDC\n" );
            }

            if (bVertical)
                drawVerticalGlyphs(aRun, aLine, aAlignOffset, aRotScale, fAngle, fXScale, fSkew, nFontHeight);
            else
                drawHorizontalGlyphs(aRun, aLine, aAlignOffset, nStart == 0, fAngle, fXScale, fSkew, nFontHeight, nPixelFontHeight);

            if (nCharPos >= 0 && nCharCount)
                aLine.append( "EMC\n" );

            nStart = nEnd;
        }
    }

    // end textobject
    aLine.append( "ET\n" );
    if( bPop )
        aLine.append( "Q\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );

    // draw eventual textlines
    FontStrikeout eStrikeout = m_aCurrentPDFState.m_aFont.GetStrikeout();
    FontLineStyle eUnderline = m_aCurrentPDFState.m_aFont.GetUnderline();
    FontLineStyle eOverline  = m_aCurrentPDFState.m_aFont.GetOverline();
    if( bTextLines &&
        (
         ( eUnderline != LINESTYLE_NONE && eUnderline != LINESTYLE_DONTKNOW ) ||
         ( eOverline  != LINESTYLE_NONE && eOverline  != LINESTYLE_DONTKNOW ) ||
         ( eStrikeout != STRIKEOUT_NONE && eStrikeout != STRIKEOUT_DONTKNOW )
         )
        )
    {
        bool bUnderlineAbove = OutputDevice::ImplIsUnderlineAbove( m_aCurrentPDFState.m_aFont );
        if( m_aCurrentPDFState.m_aFont.IsWordLineMode() )
        {
            Point aStartPt;
            sal_Int32 nWidth = 0;
            nIndex = 0;
            while (rLayout.GetNextGlyph(&pGlyph, aPos, nIndex))
            {
                if (!pGlyph->IsSpacing())
                {
                    if( !nWidth )
                        aStartPt = aPos;

                    nWidth += pGlyph->mnNewWidth;
                }
                else if( nWidth > 0 )
                {
                    drawTextLine( m_pReferenceDevice->PixelToLogic( aStartPt ),
                                  m_pReferenceDevice->ImplDevicePixelToLogicWidth( nWidth ),
                                  eStrikeout, eUnderline, eOverline, bUnderlineAbove );
                    nWidth = 0;
                }
            }

            if( nWidth > 0 )
            {
                drawTextLine( m_pReferenceDevice->PixelToLogic( aStartPt ),
                              m_pReferenceDevice->ImplDevicePixelToLogicWidth( nWidth ),
                              eStrikeout, eUnderline, eOverline, bUnderlineAbove );
            }
        }
        else
        {
            Point aStartPt = rLayout.GetDrawPosition();
            int nWidth = rLayout.GetTextWidth() / rLayout.GetUnitsPerPixel();
            drawTextLine( m_pReferenceDevice->PixelToLogic( aStartPt ),
                          m_pReferenceDevice->ImplDevicePixelToLogicWidth( nWidth ),
                          eStrikeout, eUnderline, eOverline, bUnderlineAbove );
        }
    }

    // write eventual emphasis marks
    if( !(m_aCurrentPDFState.m_aFont.GetEmphasisMark() & FontEmphasisMark::Style) )
        return;

    tools::PolyPolygon             aEmphPoly;
    tools::Rectangle               aEmphRect1;
    tools::Rectangle               aEmphRect2;
    long                    nEmphYOff;
    long                    nEmphWidth;
    long                    nEmphHeight;
    bool                    bEmphPolyLine;
    FontEmphasisMark        nEmphMark;

    push( PushFlags::ALL );

    aLine.setLength( 0 );
    aLine.append( "q\n" );

    nEmphMark = OutputDevice::ImplGetEmphasisMarkStyle( m_aCurrentPDFState.m_aFont );
    if ( nEmphMark & FontEmphasisMark::PosBelow )
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
                                             m_pReferenceDevice->ImplDevicePixelToLogicWidth(nEmphHeight) );
    if ( bEmphPolyLine )
    {
        setLineColor( m_aCurrentPDFState.m_aFont.GetColor() );
        setFillColor( COL_TRANSPARENT );
    }
    else
    {
        setFillColor( m_aCurrentPDFState.m_aFont.GetColor() );
        setLineColor( COL_TRANSPARENT );
    }
    writeBuffer( aLine.getStr(), aLine.getLength() );

    Point aOffset = Point(0,0);

    if ( nEmphMark & FontEmphasisMark::PosBelow )
        aOffset.AdjustY(m_pReferenceDevice->mpFontInstance->mxFontMetric->GetDescent() + nEmphYOff );
    else
        aOffset.AdjustY( -(m_pReferenceDevice->mpFontInstance->mxFontMetric->GetAscent() + nEmphYOff) );

    long nEmphWidth2     = nEmphWidth / 2;
    long nEmphHeight2    = nEmphHeight / 2;
    aOffset += Point( nEmphWidth2, nEmphHeight2 );

    if ( eAlign == ALIGN_BOTTOM )
        aOffset.AdjustY( -(m_pReferenceDevice->mpFontInstance->mxFontMetric->GetDescent()) );
    else if ( eAlign == ALIGN_TOP )
        aOffset.AdjustY(m_pReferenceDevice->mpFontInstance->mxFontMetric->GetAscent() );

    nIndex = 0;
    while (rLayout.GetNextGlyph(&pGlyph, aPos, nIndex))
    {
        if (pGlyph->IsSpacing())
        {
            Point aAdjOffset = aOffset;
            aAdjOffset.AdjustX((pGlyph->mnNewWidth - nEmphWidth) / 2 );
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

void PDFWriterImpl::drawEmphasisMark( long nX, long nY,
                                      const tools::PolyPolygon& rPolyPoly, bool bPolyLine,
                                      const tools::Rectangle& rRect1, const tools::Rectangle& rRect2 )
{
    // TODO: pass nWidth as width of this mark
    // long nWidth = 0;

    if ( rPolyPoly.Count() )
    {
        if ( bPolyLine )
        {
            tools::Polygon aPoly = rPolyPoly.GetObject( 0 );
            aPoly.Move( nX, nY );
            drawPolyLine( aPoly );
        }
        else
        {
            tools::PolyPolygon aPolyPoly = rPolyPoly;
            aPolyPoly.Move( nX, nY );
            drawPolyPolygon( aPolyPoly );
        }
    }

    if ( !rRect1.IsEmpty() )
    {
        tools::Rectangle aRect( Point( nX+rRect1.Left(),
                                nY+rRect1.Top() ), rRect1.GetSize() );
        drawRectangle( aRect );
    }

    if ( !rRect2.IsEmpty() )
    {
        tools::Rectangle aRect( Point( nX+rRect2.Left(),
                                nY+rRect2.Top() ), rRect2.GetSize() );

        drawRectangle( aRect );
    }
}

void PDFWriterImpl::drawText( const Point& rPos, const OUString& rText, sal_Int32 nIndex, sal_Int32 nLen, bool bTextLines )
{
    MARK( "drawText" );

    updateGraphicsState();

    // get a layout from the OutputDevice's SalGraphics
    // this also enforces font substitution and sets the font on SalGraphics
    std::unique_ptr<SalLayout> pLayout = m_pReferenceDevice->ImplLayout( rText, nIndex, nLen, rPos );
    if( pLayout )
    {
        drawLayout( *pLayout, rText, bTextLines );
    }
}

void PDFWriterImpl::drawTextArray( const Point& rPos, const OUString& rText, const long* pDXArray, sal_Int32 nIndex, sal_Int32 nLen )
{
    MARK( "drawText with array" );

    updateGraphicsState();

    // get a layout from the OutputDevice's SalGraphics
    // this also enforces font substitution and sets the font on SalGraphics
    std::unique_ptr<SalLayout> pLayout = m_pReferenceDevice->ImplLayout( rText, nIndex, nLen, rPos, 0, pDXArray );
    if( pLayout )
    {
        drawLayout( *pLayout, rText, true );
    }
}

void PDFWriterImpl::drawStretchText( const Point& rPos, sal_uLong nWidth, const OUString& rText, sal_Int32 nIndex, sal_Int32 nLen )
{
    MARK( "drawStretchText" );

    updateGraphicsState();

    // get a layout from the OutputDevice's SalGraphics
    // this also enforces font substitution and sets the font on SalGraphics
    std::unique_ptr<SalLayout> pLayout = m_pReferenceDevice->ImplLayout( rText, nIndex, nLen, rPos, nWidth );
    if( pLayout )
    {
        drawLayout( *pLayout, rText, true );
    }
}

void PDFWriterImpl::drawText( const tools::Rectangle& rRect, const OUString& rOrigStr, DrawTextFlags nStyle )
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
    sal_Int32   nMnemonicPos    = -1;

    OUString aStr = rOrigStr;
    if ( nStyle & DrawTextFlags::Mnemonic )
        aStr = OutputDevice::GetNonMnemonicString( aStr, nMnemonicPos );

    // multiline text
    if ( nStyle & DrawTextFlags::MultiLine )
    {
        OUString           aLastLine;
        ImplMultiTextLineInfo   aMultiLineInfo;
        ImplTextLineInfo*       pLineInfo;
        sal_Int32               i;
        sal_Int32               nLines;
        sal_Int32               nFormatLines;

        if ( nTextHeight )
        {
            vcl::DefaultTextLayout aLayout( *m_pReferenceDevice );
            OutputDevice::ImplGetTextLines( aMultiLineInfo, nWidth, aStr, nStyle, aLayout );
            nLines = nHeight/nTextHeight;
            nFormatLines = aMultiLineInfo.Count();
            if ( !nLines )
                nLines = 1;
            if ( nFormatLines > nLines )
            {
                if ( nStyle & DrawTextFlags::EndEllipsis )
                {
                    // handle last line
                    nFormatLines = nLines-1;

                    pLineInfo = aMultiLineInfo.GetLine( nFormatLines );
                    aLastLine = convertLineEnd(aStr.copy(pLineInfo->GetIndex()), LINEEND_LF);
                    // replace line feed by space
                    aLastLine = aLastLine.replace('\n', ' ');
                    aLastLine = m_pReferenceDevice->GetEllipsisString( aLastLine, nWidth, nStyle );
                    nStyle &= ~DrawTextFlags(DrawTextFlags::VCenter | DrawTextFlags::Bottom);
                    nStyle |= DrawTextFlags::Top;
                }
            }

            // vertical alignment
            if ( nStyle & DrawTextFlags::Bottom )
                aPos.AdjustY(nHeight-(nFormatLines*nTextHeight) );
            else if ( nStyle & DrawTextFlags::VCenter )
                aPos.AdjustY((nHeight-(nFormatLines*nTextHeight))/2 );

            // draw all lines excluding the last
            for ( i = 0; i < nFormatLines; i++ )
            {
                pLineInfo = aMultiLineInfo.GetLine( i );
                if ( nStyle & DrawTextFlags::Right )
                    aPos.AdjustX(nWidth-pLineInfo->GetWidth() );
                else if ( nStyle & DrawTextFlags::Center )
                    aPos.AdjustX((nWidth-pLineInfo->GetWidth())/2 );
                sal_Int32 nIndex = pLineInfo->GetIndex();
                sal_Int32 nLineLen = pLineInfo->GetLen();
                drawText( aPos, aStr, nIndex, nLineLen );
                // mnemonics should not appear in documents,
                // if the need arises, put them in here
                aPos.AdjustY(nTextHeight );
                aPos.setX( rRect.Left() );
            }

            // output last line left adjusted since it was shortened
            if (!aLastLine.isEmpty())
                drawText( aPos, aLastLine, 0, aLastLine.getLength() );
        }
    }
    else
    {
        long nTextWidth = m_pReferenceDevice->GetTextWidth( aStr );

        // Evt. Text kuerzen
        if ( nTextWidth > nWidth )
        {
            if ( nStyle & (DrawTextFlags::EndEllipsis | DrawTextFlags::PathEllipsis | DrawTextFlags::NewsEllipsis) )
            {
                aStr = m_pReferenceDevice->GetEllipsisString( aStr, nWidth, nStyle );
                nStyle &= ~DrawTextFlags(DrawTextFlags::Center | DrawTextFlags::Right);
                nStyle |= DrawTextFlags::Left;
                nTextWidth = m_pReferenceDevice->GetTextWidth( aStr );
            }
        }

        // vertical alignment
        if ( nStyle & DrawTextFlags::Right )
            aPos.AdjustX(nWidth-nTextWidth );
        else if ( nStyle & DrawTextFlags::Center )
            aPos.AdjustX((nWidth-nTextWidth)/2 );

        if ( nStyle & DrawTextFlags::Bottom )
            aPos.AdjustY(nHeight-nTextHeight );
        else if ( nStyle & DrawTextFlags::VCenter )
            aPos.AdjustY((nHeight-nTextHeight)/2 );

        // mnemonics should be inserted here if the need arises

        // draw the actual text
        drawText( aPos, aStr, 0, aStr.getLength() );
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

    if( m_aGraphicsStack.front().m_aLineColor == COL_TRANSPARENT )
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

    if( m_aGraphicsStack.front().m_aLineColor == COL_TRANSPARENT )
        return;

    if( rInfo.GetStyle() == LineStyle::Solid && rInfo.GetWidth() < 2 )
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
        tools::Polygon aPoly( 2, aPolyPoints );
        drawPolyLine( aPoly, aInfo );
    }
}

#define HCONV( x ) m_pReferenceDevice->ImplDevicePixelToLogicHeight( x )

void PDFWriterImpl::drawWaveTextLine( OStringBuffer& aLine, long nWidth, FontLineStyle eTextLine, Color aColor, bool bIsAbove )
{
    // note: units in pFontInstance are ref device pixel
    LogicalFontInstance*  pFontInstance = m_pReferenceDevice->mpFontInstance.get();
    long            nLineHeight = 0;
    long            nLinePos = 0;

    appendStrokingColor( aColor, aLine );
    aLine.append( "\n" );

    if ( bIsAbove )
    {
        if ( !pFontInstance->mxFontMetric->GetAboveWavelineUnderlineSize() )
            m_pReferenceDevice->ImplInitAboveTextLineSize();
        nLineHeight = HCONV( pFontInstance->mxFontMetric->GetAboveWavelineUnderlineSize() );
        nLinePos = HCONV( pFontInstance->mxFontMetric->GetAboveWavelineUnderlineOffset() );
    }
    else
    {
        if ( !pFontInstance->mxFontMetric->GetWavelineUnderlineSize() )
            m_pReferenceDevice->ImplInitTextLineSize();
        nLineHeight = HCONV( pFontInstance->mxFontMetric->GetWavelineUnderlineSize() );
        nLinePos = HCONV( pFontInstance->mxFontMetric->GetWavelineUnderlineOffset() );
    }
    if ( (eTextLine == LINESTYLE_SMALLWAVE) && (nLineHeight > 3) )
        nLineHeight = 3;

    long nLineWidth = getReferenceDevice()->mnDPIX/450;
    if ( ! nLineWidth )
        nLineWidth = 1;

    if ( eTextLine == LINESTYLE_BOLDWAVE )
        nLineWidth = 3*nLineWidth;

    m_aPages.back().appendMappedLength( static_cast<sal_Int32>(nLineWidth), aLine );
    aLine.append( " w " );

    if ( eTextLine == LINESTYLE_DOUBLEWAVE )
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
        if ( eTextLine != LINESTYLE_BOLDWAVE )
            nLinePos -= nLineWidth/2;
        m_aPages.back().appendWaveLine( nWidth, -nLinePos, nLineHeight, aLine );
    }
}

void PDFWriterImpl::drawStraightTextLine( OStringBuffer& aLine, long nWidth, FontLineStyle eTextLine, Color aColor, bool bIsAbove )
{
    // note: units in pFontInstance are ref device pixel
    LogicalFontInstance*  pFontInstance = m_pReferenceDevice->mpFontInstance.get();
    long            nLineHeight = 0;
    long            nLinePos  = 0;
    long            nLinePos2 = 0;

    if ( eTextLine > LINESTYLE_BOLDWAVE )
        eTextLine = LINESTYLE_SINGLE;

    switch ( eTextLine )
    {
        case LINESTYLE_SINGLE:
        case LINESTYLE_DOTTED:
        case LINESTYLE_DASH:
        case LINESTYLE_LONGDASH:
        case LINESTYLE_DASHDOT:
        case LINESTYLE_DASHDOTDOT:
            if ( bIsAbove )
            {
                if ( !pFontInstance->mxFontMetric->GetAboveUnderlineSize() )
                    m_pReferenceDevice->ImplInitAboveTextLineSize();
                nLineHeight = HCONV( pFontInstance->mxFontMetric->GetAboveUnderlineSize() );
                nLinePos    = HCONV( pFontInstance->mxFontMetric->GetAboveUnderlineOffset() );
            }
            else
            {
                if ( !pFontInstance->mxFontMetric->GetUnderlineSize() )
                    m_pReferenceDevice->ImplInitTextLineSize();
                nLineHeight = HCONV( pFontInstance->mxFontMetric->GetUnderlineSize() );
                nLinePos    = HCONV( pFontInstance->mxFontMetric->GetUnderlineOffset() );
            }
            break;
        case LINESTYLE_BOLD:
        case LINESTYLE_BOLDDOTTED:
        case LINESTYLE_BOLDDASH:
        case LINESTYLE_BOLDLONGDASH:
        case LINESTYLE_BOLDDASHDOT:
        case LINESTYLE_BOLDDASHDOTDOT:
            if ( bIsAbove )
            {
                if ( !pFontInstance->mxFontMetric->GetAboveBoldUnderlineSize() )
                    m_pReferenceDevice->ImplInitAboveTextLineSize();
                nLineHeight = HCONV( pFontInstance->mxFontMetric->GetAboveBoldUnderlineSize() );
                nLinePos    = HCONV( pFontInstance->mxFontMetric->GetAboveBoldUnderlineOffset() );
            }
            else
            {
                if ( !pFontInstance->mxFontMetric->GetBoldUnderlineSize() )
                    m_pReferenceDevice->ImplInitTextLineSize();
                nLineHeight = HCONV( pFontInstance->mxFontMetric->GetBoldUnderlineSize() );
                nLinePos    = HCONV( pFontInstance->mxFontMetric->GetBoldUnderlineOffset() );
                nLinePos += nLineHeight/2;
            }
            break;
        case LINESTYLE_DOUBLE:
            if ( bIsAbove )
            {
                if ( !pFontInstance->mxFontMetric->GetAboveDoubleUnderlineSize() )
                    m_pReferenceDevice->ImplInitAboveTextLineSize();
                nLineHeight = HCONV( pFontInstance->mxFontMetric->GetAboveDoubleUnderlineSize() );
                nLinePos    = HCONV( pFontInstance->mxFontMetric->GetAboveDoubleUnderlineOffset1() );
                nLinePos2   = HCONV( pFontInstance->mxFontMetric->GetAboveDoubleUnderlineOffset2() );
            }
            else
            {
                if ( !pFontInstance->mxFontMetric->GetDoubleUnderlineSize() )
                    m_pReferenceDevice->ImplInitTextLineSize();
                nLineHeight = HCONV( pFontInstance->mxFontMetric->GetDoubleUnderlineSize() );
                nLinePos    = HCONV( pFontInstance->mxFontMetric->GetDoubleUnderlineOffset1() );
                nLinePos2   = HCONV( pFontInstance->mxFontMetric->GetDoubleUnderlineOffset2() );
            }
            break;
        default:
            break;
    }

    if ( !nLineHeight )
        return;

    m_aPages.back().appendMappedLength( static_cast<sal_Int32>(nLineHeight), aLine );
    aLine.append( " w " );
    appendStrokingColor( aColor, aLine );
    aLine.append( "\n" );

    switch ( eTextLine )
    {
        case LINESTYLE_DOTTED:
        case LINESTYLE_BOLDDOTTED:
            aLine.append( "[ " );
            m_aPages.back().appendMappedLength( static_cast<sal_Int32>(nLineHeight), aLine, false );
            aLine.append( " ] 0 d\n" );
            break;
        case LINESTYLE_DASH:
        case LINESTYLE_LONGDASH:
        case LINESTYLE_BOLDDASH:
        case LINESTYLE_BOLDLONGDASH:
            {
                sal_Int32 nDashLength = 4*nLineHeight;
                sal_Int32 nVoidLength = 2*nLineHeight;
                if ( ( eTextLine == LINESTYLE_LONGDASH ) || ( eTextLine == LINESTYLE_BOLDLONGDASH ) )
                    nDashLength = 8*nLineHeight;

                aLine.append( "[ " );
                m_aPages.back().appendMappedLength( nDashLength, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                aLine.append( " ] 0 d\n" );
            }
            break;
        case LINESTYLE_DASHDOT:
        case LINESTYLE_BOLDDASHDOT:
            {
                sal_Int32 nDashLength = 4*nLineHeight;
                sal_Int32 nVoidLength = 2*nLineHeight;
                aLine.append( "[ " );
                m_aPages.back().appendMappedLength( nDashLength, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( static_cast<sal_Int32>(nLineHeight), aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                aLine.append( " ] 0 d\n" );
            }
            break;
        case LINESTYLE_DASHDOTDOT:
        case LINESTYLE_BOLDDASHDOTDOT:
            {
                sal_Int32 nDashLength = 4*nLineHeight;
                sal_Int32 nVoidLength = 2*nLineHeight;
                aLine.append( "[ " );
                m_aPages.back().appendMappedLength( nDashLength, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( static_cast<sal_Int32>(nLineHeight), aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( static_cast<sal_Int32>(nLineHeight), aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                aLine.append( " ] 0 d\n" );
            }
            break;
        default:
            break;
    }

    aLine.append( "0 " );
    m_aPages.back().appendMappedLength( static_cast<sal_Int32>(-nLinePos), aLine );
    aLine.append( " m " );
    m_aPages.back().appendMappedLength( static_cast<sal_Int32>(nWidth), aLine, false );
    aLine.append( ' ' );
    m_aPages.back().appendMappedLength( static_cast<sal_Int32>(-nLinePos), aLine );
    aLine.append( " l S\n" );
    if ( eTextLine == LINESTYLE_DOUBLE )
    {
        aLine.append( "0 " );
        m_aPages.back().appendMappedLength( static_cast<sal_Int32>(-nLinePos2-nLineHeight), aLine );
        aLine.append( " m " );
        m_aPages.back().appendMappedLength( static_cast<sal_Int32>(nWidth), aLine, false );
        aLine.append( ' ' );
        m_aPages.back().appendMappedLength( static_cast<sal_Int32>(-nLinePos2-nLineHeight), aLine );
        aLine.append( " l S\n" );
    }

}

void PDFWriterImpl::drawStrikeoutLine( OStringBuffer& aLine, long nWidth, FontStrikeout eStrikeout, Color aColor )
{
    // note: units in pFontInstance are ref device pixel
    LogicalFontInstance*  pFontInstance = m_pReferenceDevice->mpFontInstance.get();
    long            nLineHeight = 0;
    long            nLinePos  = 0;
    long            nLinePos2 = 0;

    if ( eStrikeout > STRIKEOUT_X )
        eStrikeout = STRIKEOUT_SINGLE;

    switch ( eStrikeout )
    {
        case STRIKEOUT_SINGLE:
            if ( !pFontInstance->mxFontMetric->GetStrikeoutSize() )
                m_pReferenceDevice->ImplInitTextLineSize();
            nLineHeight = HCONV( pFontInstance->mxFontMetric->GetStrikeoutSize() );
            nLinePos    = HCONV( pFontInstance->mxFontMetric->GetStrikeoutOffset() );
            break;
        case STRIKEOUT_BOLD:
            if ( !pFontInstance->mxFontMetric->GetBoldStrikeoutSize() )
                m_pReferenceDevice->ImplInitTextLineSize();
            nLineHeight = HCONV( pFontInstance->mxFontMetric->GetBoldStrikeoutSize() );
            nLinePos    = HCONV( pFontInstance->mxFontMetric->GetBoldStrikeoutOffset() );
            break;
        case STRIKEOUT_DOUBLE:
            if ( !pFontInstance->mxFontMetric->GetDoubleStrikeoutSize() )
                m_pReferenceDevice->ImplInitTextLineSize();
            nLineHeight = HCONV( pFontInstance->mxFontMetric->GetDoubleStrikeoutSize() );
            nLinePos    = HCONV( pFontInstance->mxFontMetric->GetDoubleStrikeoutOffset1() );
            nLinePos2   = HCONV( pFontInstance->mxFontMetric->GetDoubleStrikeoutOffset2() );
            break;
        default:
            break;
    }

    if ( !nLineHeight )
        return;

    m_aPages.back().appendMappedLength( static_cast<sal_Int32>(nLineHeight), aLine );
    aLine.append( " w " );
    appendStrokingColor( aColor, aLine );
    aLine.append( "\n" );

    aLine.append( "0 " );
    m_aPages.back().appendMappedLength( static_cast<sal_Int32>(-nLinePos), aLine );
    aLine.append( " m " );
    m_aPages.back().appendMappedLength( static_cast<sal_Int32>(nWidth), aLine );
    aLine.append( ' ' );
    m_aPages.back().appendMappedLength( static_cast<sal_Int32>(-nLinePos), aLine );
    aLine.append( " l S\n" );

    if ( eStrikeout == STRIKEOUT_DOUBLE )
    {
        aLine.append( "0 " );
        m_aPages.back().appendMappedLength( static_cast<sal_Int32>(-nLinePos2-nLineHeight), aLine );
        aLine.append( " m " );
        m_aPages.back().appendMappedLength( static_cast<sal_Int32>(nWidth), aLine );
        aLine.append( ' ' );
        m_aPages.back().appendMappedLength( static_cast<sal_Int32>(-nLinePos2-nLineHeight), aLine );
        aLine.append( " l S\n" );
    }

}

void PDFWriterImpl::drawStrikeoutChar( const Point& rPos, long nWidth, FontStrikeout eStrikeout )
{
    //See qadevOOo/testdocs/StrikeThrough.odt for examples if you need
    //to tweak this

    OUString aStrikeoutChar = eStrikeout == STRIKEOUT_SLASH ? OUString( "/" ) : OUString( "X" );
    OUString aStrikeout = aStrikeoutChar;
    while( m_pReferenceDevice->GetTextWidth( aStrikeout ) < nWidth )
        aStrikeout += aStrikeout;

    // do not get broader than nWidth modulo 1 character
    while( m_pReferenceDevice->GetTextWidth( aStrikeout ) >= nWidth )
        aStrikeout = aStrikeout.replaceAt( 0, 1, "" );
    aStrikeout += aStrikeoutChar;
    bool bShadow = m_aCurrentPDFState.m_aFont.IsShadow();
    if ( bShadow )
    {
        Font aFont = m_aCurrentPDFState.m_aFont;
        aFont.SetShadow( false );
        setFont( aFont );
        updateGraphicsState();
    }

    // strikeout string is left aligned non-CTL text
    ComplexTextLayoutFlags nOrigTLM = m_pReferenceDevice->GetLayoutMode();
    m_pReferenceDevice->SetLayoutMode(ComplexTextLayoutFlags::BiDiStrong);

    push( PushFlags::CLIPREGION );
    FontMetric aRefDevFontMetric = m_pReferenceDevice->GetFontMetric();
    tools::Rectangle aRect;
    aRect.SetLeft( rPos.X() );
    aRect.SetRight( aRect.Left()+nWidth );
    aRect.SetBottom( rPos.Y()+aRefDevFontMetric.GetDescent() );
    aRect.SetTop( rPos.Y()-aRefDevFontMetric.GetAscent() );

    LogicalFontInstance* pFontInstance = m_pReferenceDevice->mpFontInstance.get();
    if (pFontInstance->mnOrientation)
    {
        tools::Polygon aPoly( aRect );
        aPoly.Rotate( rPos, pFontInstance->mnOrientation);
        aRect = aPoly.GetBoundRect();
    }

    intersectClipRegion( aRect );
    drawText( rPos, aStrikeout, 0, aStrikeout.getLength(), false );
    pop();

    m_pReferenceDevice->SetLayoutMode( nOrigTLM );

    if ( bShadow )
    {
        Font aFont = m_aCurrentPDFState.m_aFont;
        aFont.SetShadow( true );
        setFont( aFont );
        updateGraphicsState();
    }
}

void PDFWriterImpl::drawTextLine( const Point& rPos, long nWidth, FontStrikeout eStrikeout, FontLineStyle eUnderline, FontLineStyle eOverline, bool bUnderlineAbove )
{
    if ( !nWidth ||
         ( ((eStrikeout == STRIKEOUT_NONE)||(eStrikeout == STRIKEOUT_DONTKNOW)) &&
           ((eUnderline == LINESTYLE_NONE)||(eUnderline == LINESTYLE_DONTKNOW)) &&
           ((eOverline  == LINESTYLE_NONE)||(eOverline  == LINESTYLE_DONTKNOW)) ) )
        return;

    MARK( "drawTextLine" );
    updateGraphicsState();

    // note: units in pFontInstance are ref device pixel
    LogicalFontInstance* pFontInstance = m_pReferenceDevice->mpFontInstance.get();
    Color           aUnderlineColor = m_aCurrentPDFState.m_aTextLineColor;
    Color           aOverlineColor  = m_aCurrentPDFState.m_aOverlineColor;
    Color           aStrikeoutColor = m_aCurrentPDFState.m_aFont.GetColor();
    bool            bStrikeoutDone = false;
    bool            bUnderlineDone = false;
    bool            bOverlineDone  = false;

    if ( (eStrikeout == STRIKEOUT_SLASH) || (eStrikeout == STRIKEOUT_X) )
    {
        drawStrikeoutChar( rPos, nWidth, eStrikeout );
        bStrikeoutDone = true;
    }

    Point aPos( rPos );
    TextAlign eAlign = m_aCurrentPDFState.m_aFont.GetAlignment();
    if( eAlign == ALIGN_TOP )
        aPos.AdjustY(HCONV( pFontInstance->mxFontMetric->GetAscent() ));
    else if( eAlign == ALIGN_BOTTOM )
        aPos.AdjustY( -HCONV( pFontInstance->mxFontMetric->GetDescent() ) );

    OStringBuffer aLine( 512 );
    // save GS
    aLine.append( "q " );

    // rotate and translate matrix
    double fAngle = static_cast<double>(m_aCurrentPDFState.m_aFont.GetOrientation()) * M_PI / 1800.0;
    Matrix3 aMat;
    aMat.rotate( fAngle );
    aMat.translate( aPos.X(), aPos.Y() );
    aMat.append( m_aPages.back(), aLine );
    aLine.append( " cm\n" );

    if ( aUnderlineColor.GetTransparency() != 0 )
        aUnderlineColor = aStrikeoutColor;

    if ( (eUnderline == LINESTYLE_SMALLWAVE) ||
         (eUnderline == LINESTYLE_WAVE) ||
         (eUnderline == LINESTYLE_DOUBLEWAVE) ||
         (eUnderline == LINESTYLE_BOLDWAVE) )
    {
        drawWaveTextLine( aLine, nWidth, eUnderline, aUnderlineColor, bUnderlineAbove );
        bUnderlineDone = true;
    }

    if ( (eOverline == LINESTYLE_SMALLWAVE) ||
         (eOverline == LINESTYLE_WAVE) ||
         (eOverline == LINESTYLE_DOUBLEWAVE) ||
         (eOverline == LINESTYLE_BOLDWAVE) )
    {
        drawWaveTextLine( aLine, nWidth, eOverline, aOverlineColor, true );
        bOverlineDone = true;
    }

    if ( !bUnderlineDone )
    {
        drawStraightTextLine( aLine, nWidth, eUnderline, aUnderlineColor, bUnderlineAbove );
    }

    if ( !bOverlineDone )
    {
        drawStraightTextLine( aLine, nWidth, eOverline, aOverlineColor, true );
    }

    if ( !bStrikeoutDone )
    {
        drawStrikeoutLine( aLine, nWidth, eStrikeout, aStrikeoutColor );
    }

    aLine.append( "Q\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawPolygon( const tools::Polygon& rPoly )
{
    MARK( "drawPolygon" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == COL_TRANSPARENT &&
        m_aGraphicsStack.front().m_aFillColor == COL_TRANSPARENT )
        return;

    int nPoints = rPoly.GetSize();
    OStringBuffer aLine( 20 * nPoints );
    m_aPages.back().appendPolygon( rPoly, aLine );
    if( m_aGraphicsStack.front().m_aLineColor != COL_TRANSPARENT &&
        m_aGraphicsStack.front().m_aFillColor != COL_TRANSPARENT )
        aLine.append( "B*\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != COL_TRANSPARENT )
        aLine.append( "S\n" );
    else
        aLine.append( "f*\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawPolyPolygon( const tools::PolyPolygon& rPolyPoly )
{
    MARK( "drawPolyPolygon" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == COL_TRANSPARENT &&
        m_aGraphicsStack.front().m_aFillColor == COL_TRANSPARENT )
        return;

    int nPolygons = rPolyPoly.Count();

    OStringBuffer aLine( 40 * nPolygons );
    m_aPages.back().appendPolyPolygon( rPolyPoly, aLine );
    if( m_aGraphicsStack.front().m_aLineColor != COL_TRANSPARENT &&
        m_aGraphicsStack.front().m_aFillColor != COL_TRANSPARENT )
        aLine.append( "B*\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != COL_TRANSPARENT )
        aLine.append( "S\n" );
    else
        aLine.append( "f*\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawTransparent( const tools::PolyPolygon& rPolyPoly, sal_uInt32 nTransparentPercent )
{
    SAL_WARN_IF( nTransparentPercent > 100, "vcl.pdfwriter", "invalid alpha value" );
    nTransparentPercent = nTransparentPercent % 100;

    MARK( "drawTransparent" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == COL_TRANSPARENT &&
        m_aGraphicsStack.front().m_aFillColor == COL_TRANSPARENT )
        return;

    if( m_bIsPDF_A1 || m_aContext.Version < PDFWriter::PDFVersion::PDF_1_4 )
    {
        m_aErrors.insert( m_bIsPDF_A1 ?
                          PDFWriter::Warning_Transparency_Omitted_PDFA :
                          PDFWriter::Warning_Transparency_Omitted_PDF13 );

        drawPolyPolygon( rPolyPoly );
        return;
    }

    // create XObject
    m_aTransparentObjects.emplace_back( );
    // FIXME: polygons with beziers may yield incorrect bound rect
    m_aTransparentObjects.back().m_aBoundRect     = rPolyPoly.GetBoundRect();
    // convert rectangle to default user space
    m_aPages.back().convertRect( m_aTransparentObjects.back().m_aBoundRect );
    m_aTransparentObjects.back().m_nObject          = createObject();
    m_aTransparentObjects.back().m_nExtGStateObject = createObject();
    m_aTransparentObjects.back().m_fAlpha           = static_cast<double>(100-nTransparentPercent) / 100.0;
    m_aTransparentObjects.back().m_pContentStream.reset(new SvMemoryStream( 256, 256 ));
    // create XObject's content stream
    OStringBuffer aContent( 256 );
    m_aPages.back().appendPolyPolygon( rPolyPoly, aContent );
    if( m_aCurrentPDFState.m_aLineColor != COL_TRANSPARENT &&
        m_aCurrentPDFState.m_aFillColor != COL_TRANSPARENT )
        aContent.append( " B*\n" );
    else if( m_aCurrentPDFState.m_aLineColor != COL_TRANSPARENT )
        aContent.append( " S\n" );
    else
        aContent.append( " f*\n" );
    m_aTransparentObjects.back().m_pContentStream->WriteBytes(
        aContent.getStr(), aContent.getLength() );

    OStringBuffer aObjName( 16 );
    aObjName.append( "Tr" );
    aObjName.append( m_aTransparentObjects.back().m_nObject );
    OString aTrName( aObjName.makeStringAndClear() );
    aObjName.append( "EGS" );
    aObjName.append( m_aTransparentObjects.back().m_nExtGStateObject );
    OString aExtName( aObjName.makeStringAndClear() );

    OStringBuffer aLine( 80 );
    // insert XObject
    aLine.append( "q /" );
    aLine.append( aExtName );
    aLine.append( " gs /" );
    aLine.append( aTrName );
    aLine.append( " Do Q\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );

    pushResource( ResXObject, aTrName, m_aTransparentObjects.back().m_nObject );
    pushResource( ResExtGState, aExtName, m_aTransparentObjects.back().m_nExtGStateObject );
}

void PDFWriterImpl::pushResource( ResourceKind eKind, const OString& rResource, sal_Int32 nObject )
{
    if( nObject >= 0 )
    {
        switch( eKind )
        {
            case ResXObject:
                m_aGlobalResourceDict.m_aXObjects[ rResource ] = nObject;
                if( ! m_aOutputStreams.empty() )
                    m_aOutputStreams.front().m_aResourceDict.m_aXObjects[ rResource ] = nObject;
                break;
            case ResExtGState:
                m_aGlobalResourceDict.m_aExtGStates[ rResource ] = nObject;
                if( ! m_aOutputStreams.empty() )
                    m_aOutputStreams.front().m_aResourceDict.m_aExtGStates[ rResource ] = nObject;
                break;
            case ResShading:
                m_aGlobalResourceDict.m_aShadings[ rResource ] = nObject;
                if( ! m_aOutputStreams.empty() )
                    m_aOutputStreams.front().m_aResourceDict.m_aShadings[ rResource ] = nObject;
                break;
            case ResPattern:
                m_aGlobalResourceDict.m_aPatterns[ rResource ] = nObject;
                if( ! m_aOutputStreams.empty() )
                    m_aOutputStreams.front().m_aResourceDict.m_aPatterns[ rResource ] = nObject;
                break;
        }
    }
}

void PDFWriterImpl::beginRedirect( SvStream* pStream, const tools::Rectangle& rTargetRect )
{
    push( PushFlags::ALL );

    // force reemitting clip region inside the new stream, and
    // prevent emitting an unbalanced "Q" at the start
    clearClipRegion();
    // this is needed to point m_aCurrentPDFState at the pushed state
    // ... but it's pointless to actually write into the "outer" stream here!
    updateGraphicsState(NOWRITE);

    m_aOutputStreams.push_front( StreamRedirect() );
    m_aOutputStreams.front().m_pStream = pStream;
    m_aOutputStreams.front().m_aMapMode = m_aMapMode;

    if( !rTargetRect.IsEmpty() )
    {
        m_aOutputStreams.front().m_aTargetRect =
            lcl_convert( m_aGraphicsStack.front().m_aMapMode,
                         m_aMapMode,
                         getReferenceDevice(),
                         rTargetRect );
        Point aDelta = m_aOutputStreams.front().m_aTargetRect.BottomLeft();
        long nPageHeight = pointToPixel(m_aPages[m_nCurrentPage].getHeight());
        aDelta.setY( -(nPageHeight - m_aOutputStreams.front().m_aTargetRect.Bottom()) );
        m_aMapMode.SetOrigin( m_aMapMode.GetOrigin() + aDelta );
    }

    // setup graphics state for independent object stream

    // force reemitting colors
    m_aCurrentPDFState.m_aLineColor = COL_TRANSPARENT;
    m_aCurrentPDFState.m_aFillColor = COL_TRANSPARENT;
}

SvStream* PDFWriterImpl::endRedirect()
{
    SvStream* pStream = nullptr;
    if( ! m_aOutputStreams.empty() )
    {
        pStream     = m_aOutputStreams.front().m_pStream;
        m_aMapMode  = m_aOutputStreams.front().m_aMapMode;
        m_aOutputStreams.pop_front();
    }

    pop();

    m_aCurrentPDFState.m_aLineColor = COL_TRANSPARENT;
    m_aCurrentPDFState.m_aFillColor = COL_TRANSPARENT;

    // needed after pop() to set m_aCurrentPDFState
    updateGraphicsState(NOWRITE);

    return pStream;
}

void PDFWriterImpl::beginTransparencyGroup()
{
    updateGraphicsState();
    if( m_aContext.Version >= PDFWriter::PDFVersion::PDF_1_4 )
        beginRedirect( new SvMemoryStream( 1024, 1024 ), tools::Rectangle() );
}

void PDFWriterImpl::endTransparencyGroup( const tools::Rectangle& rBoundingBox, sal_uInt32 nTransparentPercent )
{
    SAL_WARN_IF( nTransparentPercent > 100, "vcl.pdfwriter", "invalid alpha value" );
    nTransparentPercent = nTransparentPercent % 100;

    if( m_aContext.Version < PDFWriter::PDFVersion::PDF_1_4 )
        return;

    // create XObject
    m_aTransparentObjects.emplace_back( );
    m_aTransparentObjects.back().m_aBoundRect   = rBoundingBox;
    // convert rectangle to default user space
    m_aPages.back().convertRect( m_aTransparentObjects.back().m_aBoundRect );
    m_aTransparentObjects.back().m_nObject      = createObject();
    m_aTransparentObjects.back().m_fAlpha       = static_cast<double>(100-nTransparentPercent) / 100.0;
    // get XObject's content stream
    m_aTransparentObjects.back().m_pContentStream.reset( static_cast<SvMemoryStream*>(endRedirect()) );
    m_aTransparentObjects.back().m_nExtGStateObject = createObject();

    OStringBuffer aObjName( 16 );
    aObjName.append( "Tr" );
    aObjName.append( m_aTransparentObjects.back().m_nObject );
    OString aTrName( aObjName.makeStringAndClear() );
    aObjName.append( "EGS" );
    aObjName.append( m_aTransparentObjects.back().m_nExtGStateObject );
    OString aExtName( aObjName.makeStringAndClear() );

    OStringBuffer aLine( 80 );
    // insert XObject
    aLine.append( "q /" );
    aLine.append( aExtName );
    aLine.append( " gs /" );
    aLine.append( aTrName );
    aLine.append( " Do Q\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );

    pushResource( ResXObject, aTrName, m_aTransparentObjects.back().m_nObject );
    pushResource( ResExtGState, aExtName, m_aTransparentObjects.back().m_nExtGStateObject );

}

void PDFWriterImpl::drawRectangle( const tools::Rectangle& rRect )
{
    MARK( "drawRectangle" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == COL_TRANSPARENT &&
        m_aGraphicsStack.front().m_aFillColor == COL_TRANSPARENT )
        return;

    OStringBuffer aLine( 40 );
    m_aPages.back().appendRect( rRect, aLine );

    if( m_aGraphicsStack.front().m_aLineColor != COL_TRANSPARENT &&
        m_aGraphicsStack.front().m_aFillColor != COL_TRANSPARENT )
        aLine.append( " B*\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != COL_TRANSPARENT )
        aLine.append( " S\n" );
    else
        aLine.append( " f*\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawRectangle( const tools::Rectangle& rRect, sal_uInt32 nHorzRound, sal_uInt32 nVertRound )
{
    MARK( "drawRectangle with rounded edges" );

    if( !nHorzRound && !nVertRound )
        drawRectangle( rRect );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == COL_TRANSPARENT &&
        m_aGraphicsStack.front().m_aFillColor == COL_TRANSPARENT )
        return;

    if( nHorzRound > static_cast<sal_uInt32>(rRect.GetWidth())/2 )
        nHorzRound = rRect.GetWidth()/2;
    if( nVertRound > static_cast<sal_uInt32>(rRect.GetHeight())/2 )
        nVertRound = rRect.GetHeight()/2;

    Point aPoints[16];
    const double kappa = 0.5522847498;
    const sal_uInt32 kx = static_cast<sal_uInt32>((kappa*static_cast<double>(nHorzRound))+0.5);
    const sal_uInt32 ky = static_cast<sal_uInt32>((kappa*static_cast<double>(nVertRound))+0.5);

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

    if( m_aGraphicsStack.front().m_aLineColor != COL_TRANSPARENT &&
        m_aGraphicsStack.front().m_aFillColor != COL_TRANSPARENT )
        aLine.append( "b*\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != COL_TRANSPARENT )
        aLine.append( "s\n" );
    else
        aLine.append( "f*\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawEllipse( const tools::Rectangle& rRect )
{
    MARK( "drawEllipse" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == COL_TRANSPARENT &&
        m_aGraphicsStack.front().m_aFillColor == COL_TRANSPARENT )
        return;

    Point aPoints[12];
    const double kappa = 0.5522847498;
    const sal_uInt32 kx = static_cast<sal_uInt32>((kappa*static_cast<double>(rRect.GetWidth())/2.0)+0.5);
    const sal_uInt32 ky = static_cast<sal_uInt32>((kappa*static_cast<double>(rRect.GetHeight())/2.0)+0.5);

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

    if( m_aGraphicsStack.front().m_aLineColor != COL_TRANSPARENT &&
        m_aGraphicsStack.front().m_aFillColor != COL_TRANSPARENT )
        aLine.append( "b*\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != COL_TRANSPARENT )
        aLine.append( "s\n" );
    else
        aLine.append( "f*\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

static double calcAngle( const tools::Rectangle& rRect, const Point& rPoint )
{
    Point aOrigin((rRect.Left()+rRect.Right()+1)/2,
                  (rRect.Top()+rRect.Bottom()+1)/2);
    Point aPoint = rPoint - aOrigin;

    double fX = static_cast<double>(aPoint.X());
    double fY = static_cast<double>(-aPoint.Y());

    if ((rRect.GetHeight() == 0) || (rRect.GetWidth() == 0))
        throw o3tl::divide_by_zero();

    if( rRect.GetWidth() > rRect.GetHeight() )
        fY = fY*(static_cast<double>(rRect.GetWidth())/static_cast<double>(rRect.GetHeight()));
    else if( rRect.GetHeight() > rRect.GetWidth() )
        fX = fX*(static_cast<double>(rRect.GetHeight())/static_cast<double>(rRect.GetWidth()));
    return atan2( fY, fX );
}

void PDFWriterImpl::drawArc( const tools::Rectangle& rRect, const Point& rStart, const Point& rStop, bool bWithPie, bool bWithChord )
{
    MARK( "drawArc" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == COL_TRANSPARENT &&
        m_aGraphicsStack.front().m_aFillColor == COL_TRANSPARENT )
        return;

    // calculate start and stop angles
    const double fStartAngle = calcAngle( rRect, rStart );
    double fStopAngle  = calcAngle( rRect, rStop );
    while( fStopAngle < fStartAngle )
        fStopAngle += 2.0*M_PI;
    const int nFragments = static_cast<int>((fStopAngle-fStartAngle)/(M_PI/2.0))+1;
    const double fFragmentDelta = (fStopAngle-fStartAngle)/static_cast<double>(nFragments);
    const double kappa = fabs( 4.0 * (1.0-cos(fFragmentDelta/2.0))/sin(fFragmentDelta/2.0) / 3.0);
    const double halfWidth = static_cast<double>(rRect.GetWidth())/2.0;
    const double halfHeight = static_cast<double>(rRect.GetHeight())/2.0;

    const Point aCenter( (rRect.Left()+rRect.Right()+1)/2,
                         (rRect.Top()+rRect.Bottom()+1)/2 );

    OStringBuffer aLine( 30*nFragments );
    Point aPoint( static_cast<int>(halfWidth * cos(fStartAngle) ),
                  -static_cast<int>(halfHeight * sin(fStartAngle) ) );
    aPoint += aCenter;
    m_aPages.back().appendPoint( aPoint, aLine );
    aLine.append( " m " );
    if( !basegfx::fTools::equal(fStartAngle, fStopAngle) )
    {
        for( int i = 0; i < nFragments; i++ )
        {
            const double fStartFragment = fStartAngle + static_cast<double>(i)*fFragmentDelta;
            const double fStopFragment = fStartFragment + fFragmentDelta;
            aPoint = Point( static_cast<int>(halfWidth * (cos(fStartFragment) - kappa*sin(fStartFragment) ) ),
                            -static_cast<int>(halfHeight * (sin(fStartFragment) + kappa*cos(fStartFragment) ) ) );
            aPoint += aCenter;
            m_aPages.back().appendPoint( aPoint, aLine );
            aLine.append( ' ' );

            aPoint = Point( static_cast<int>(halfWidth * (cos(fStopFragment) + kappa*sin(fStopFragment) ) ),
                            -static_cast<int>(halfHeight * (sin(fStopFragment) - kappa*cos(fStopFragment) ) ) );
            aPoint += aCenter;
            m_aPages.back().appendPoint( aPoint, aLine );
            aLine.append( ' ' );

            aPoint = Point( static_cast<int>(halfWidth * cos(fStopFragment) ),
                            -static_cast<int>(halfHeight * sin(fStopFragment) ) );
            aPoint += aCenter;
            m_aPages.back().appendPoint( aPoint, aLine );
            aLine.append( " c\n" );
        }
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
    else if( m_aGraphicsStack.front().m_aLineColor != COL_TRANSPARENT &&
        m_aGraphicsStack.front().m_aFillColor != COL_TRANSPARENT )
        aLine.append( "B*\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != COL_TRANSPARENT )
        aLine.append( "S\n" );
    else
        aLine.append( "f*\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawPolyLine( const tools::Polygon& rPoly )
{
    MARK( "drawPolyLine" );

    sal_uInt16 nPoints = rPoly.GetSize();
    if( nPoints < 2 )
        return;

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == COL_TRANSPARENT )
        return;

    OStringBuffer aLine( 20 * nPoints );
    m_aPages.back().appendPolygon( rPoly, aLine, rPoly[0] == rPoly[nPoints-1] );
    aLine.append( "S\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawPolyLine( const tools::Polygon& rPoly, const LineInfo& rInfo )
{
    MARK( "drawPolyLine with LineInfo" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == COL_TRANSPARENT )
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
    SAL_WARN_IF( rIn.GetStyle() != LineStyle::Dash, "vcl.pdfwriter", "invalid conversion" );
    rOut.m_fLineWidth           = rIn.GetWidth();
    rOut.m_fTransparency        = 0.0;
    rOut.m_eCap                 = PDFWriter::capButt;
    rOut.m_eJoin                = PDFWriter::joinMiter;
    rOut.m_fMiterLimit          = 10;
    rOut.m_aDashArray.clear();

    // add DashDot to DashArray
    const int nDashes   = rIn.GetDashCount();
    const int nDashLen  = rIn.GetDashLen();
    const int nDistance = rIn.GetDistance();

    for( int n  = 0; n < nDashes; n++ )
    {
        rOut.m_aDashArray.push_back( nDashLen );
        rOut.m_aDashArray.push_back( nDistance );
    }
    const int nDots   = rIn.GetDotCount();
    const int nDotLen = rIn.GetDotLen();

    for( int n  = 0; n < nDots; n++ )
    {
        rOut.m_aDashArray.push_back( nDotLen );
        rOut.m_aDashArray.push_back( nDistance );
    }

    // add LineJoin
    switch(rIn.GetLineJoin())
    {
        case basegfx::B2DLineJoin::Bevel :
        {
            rOut.m_eJoin = PDFWriter::joinBevel;
            break;
        }
        // Pdf has no 'none' lineJoin, default is miter
        case basegfx::B2DLineJoin::NONE :
        case basegfx::B2DLineJoin::Miter :
        {
            rOut.m_eJoin = PDFWriter::joinMiter;
            break;
        }
        case basegfx::B2DLineJoin::Round :
        {
            rOut.m_eJoin = PDFWriter::joinRound;
            break;
        }
    }

    // add LineCap
    switch(rIn.GetLineCap())
    {
        default: /* css::drawing::LineCap_BUTT */
        {
            rOut.m_eCap = PDFWriter::capButt;
            break;
        }
        case css::drawing::LineCap_ROUND:
        {
            rOut.m_eCap = PDFWriter::capRound;
            break;
        }
        case css::drawing::LineCap_SQUARE:
        {
            rOut.m_eCap = PDFWriter::capSquare;
            break;
        }
    }
}

void PDFWriterImpl::drawPolyLine( const tools::Polygon& rPoly, const PDFWriter::ExtLineInfo& rInfo )
{
    MARK( "drawPolyLine with ExtLineInfo" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == COL_TRANSPARENT )
        return;

    if( rInfo.m_fTransparency >= 1.0 )
        return;

    if( rInfo.m_fTransparency != 0.0 )
        beginTransparencyGroup();

    OStringBuffer aLine;
    aLine.append( "q " );
    m_aPages.back().appendMappedLength( rInfo.m_fLineWidth, aLine );
    aLine.append( " w" );
    if( rInfo.m_aDashArray.size() < 10 ) // implementation limit of acrobat reader
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
            for (auto const& dash : rInfo.m_aDashArray)
            {
                m_aPages.back().appendMappedLength( dash, aLine );
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
        basegfx::B2DPolygon aPoly(rPoly.getB2DPolygon());
        basegfx::B2DPolyPolygon aPolyPoly;

        basegfx::utils::applyLineDashing(aPoly, rInfo.m_aDashArray, &aPolyPoly);

        // Old applyLineDashing subdivided the polygon. New one will create bezier curve segments.
        // To mimic old behaviour, apply subdivide here. If beziers shall be written (better quality)
        // this line needs to be removed and the loop below adapted accordingly
        aPolyPoly = basegfx::utils::adaptiveSubdivideByAngle(aPolyPoly);

        const sal_uInt32 nPolygonCount(aPolyPoly.count());

        for( sal_uInt32 nPoly = 0; nPoly < nPolygonCount; nPoly++ )
        {
            aLine.append( (nPoly != 0 && (nPoly & 7) == 0) ? "\n" : " " );
            aPoly = aPolyPoly.getB2DPolygon( nPoly );
            const sal_uInt32 nPointCount(aPoly.count());

            if(nPointCount)
            {
                const sal_uInt32 nEdgeCount(aPoly.isClosed() ? nPointCount : nPointCount - 1);
                basegfx::B2DPoint aCurrent(aPoly.getB2DPoint(0));

                for(sal_uInt32 a(0); a < nEdgeCount; a++)
                {
                    if( a > 0 )
                        aLine.append( " " );
                    const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                    const basegfx::B2DPoint aNext(aPoly.getB2DPoint(nNextIndex));

                    m_aPages.back().appendPoint( Point( FRound(aCurrent.getX()),
                                                        FRound(aCurrent.getY()) ),
                                                 aLine );
                    aLine.append( " m " );
                    m_aPages.back().appendPoint( Point( FRound(aNext.getX()),
                                                        FRound(aNext.getY()) ),
                                                 aLine );
                    aLine.append( " l" );

                    // prepare next edge
                    aCurrent = aNext;
                }
            }
        }
        aLine.append( " S " );
        writeBuffer( aLine.getStr(), aLine.getLength() );
    }
    writeBuffer( "Q\n", 2 );

    if( rInfo.m_fTransparency != 0.0 )
    {
        // FIXME: actually this may be incorrect with bezier polygons
        tools::Rectangle aBoundRect( rPoly.GetBoundRect() );
        // avoid clipping with thick lines
        if( rInfo.m_fLineWidth > 0.0 )
        {
            sal_Int32 nLW = sal_Int32(rInfo.m_fLineWidth);
            aBoundRect.AdjustTop( -nLW );
            aBoundRect.AdjustLeft( -nLW );
            aBoundRect.AdjustRight(nLW );
            aBoundRect.AdjustBottom(nLW );
        }
        endTransparencyGroup( aBoundRect, static_cast<sal_uInt16>(100.0*rInfo.m_fTransparency) );
    }
}

void PDFWriterImpl::drawPixel( const Point& rPoint, const Color& rColor )
{
    MARK( "drawPixel" );

    Color aColor = ( rColor == COL_TRANSPARENT ? m_aGraphicsStack.front().m_aLineColor : rColor );

    if( aColor == COL_TRANSPARENT )
        return;

    // pixels are drawn in line color, so have to set
    // the nonstroking color to line color
    Color aOldFillColor = m_aGraphicsStack.front().m_aFillColor;
    setFillColor( aColor );

    updateGraphicsState();

    OStringBuffer aLine( 20 );
    m_aPages.back().appendPoint( rPoint, aLine );
    aLine.append( ' ' );
    appendDouble( 1.0/double(getReferenceDevice()->GetDPIX()), aLine );
    aLine.append( ' ' );
    appendDouble( 1.0/double(getReferenceDevice()->GetDPIY()), aLine );
    aLine.append( " re f\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );

    setFillColor( aOldFillColor );
}

void PDFWriterImpl::writeTransparentObject( TransparencyEmit& rObject )
{
    CHECK_RETURN2( updateObject( rObject.m_nObject ) );

    bool bFlateFilter = compressStream( rObject.m_pContentStream.get() );
    rObject.m_pContentStream->Seek( STREAM_SEEK_TO_END );
    sal_uLong nSize = rObject.m_pContentStream->Tell();
    rObject.m_pContentStream->Seek( STREAM_SEEK_TO_BEGIN );
    if (g_bDebugDisableCompression)
    {
        emitComment( "PDFWriterImpl::writeTransparentObject" );
    }
    OStringBuffer aLine( 512 );
    CHECK_RETURN2( updateObject( rObject.m_nObject ) );
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
    if( ! rObject.m_pSoftMaskStream )
    {
        if( ! m_bIsPDF_A1 )
        {
            aLine.append( "/Group<</S/Transparency/CS/DeviceRGB/K true>>\n" );
        }
    }
    /* #i42884# the PDF reference recommends that each Form XObject
    *  should have a resource dict; alas if that is the same object
    *  as the one of the page it triggers an endless recursion in
    *  acroread 5 (6 and up have that fixed). Since we have only one
    *  resource dict anyway, let's use the one from the page by NOT
    *  emitting a Resources entry.
    */

    aLine.append( "/Length " );
    aLine.append( static_cast<sal_Int32>(nSize) );
    aLine.append( "\n" );
    if( bFlateFilter )
        aLine.append( "/Filter/FlateDecode\n" );
    aLine.append( ">>\n"
                  "stream\n" );
    CHECK_RETURN2( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    checkAndEnableStreamEncryption( rObject.m_nObject );
    CHECK_RETURN2( writeBuffer( rObject.m_pContentStream->GetData(), nSize ) );
    disableStreamEncryption();
    aLine.setLength( 0 );
    aLine.append( "\n"
                  "endstream\n"
                  "endobj\n\n" );
    CHECK_RETURN2( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    // write ExtGState dict for this XObject
    aLine.setLength( 0 );
    aLine.append( rObject.m_nExtGStateObject );
    aLine.append( " 0 obj\n"
                  "<<" );
    if( ! rObject.m_pSoftMaskStream )
    {
        if( m_bIsPDF_A1 )
        {
            aLine.append( "/CA 1.0/ca 1.0" );
            m_aErrors.insert( PDFWriter::Warning_Transparency_Omitted_PDFA );
        }
        else
        {
            aLine.append(  "/CA " );
            appendDouble( rObject.m_fAlpha, aLine );
            aLine.append( "\n"
                          "   /ca " );
            appendDouble( rObject.m_fAlpha, aLine );
        }
        aLine.append( "\n" );
    }
    else
    {
        if( m_bIsPDF_A1 )
        {
            aLine.append( "/SMask/None" );
            m_aErrors.insert( PDFWriter::Warning_Transparency_Omitted_PDFA );
        }
        else
        {
            rObject.m_pSoftMaskStream->Seek( STREAM_SEEK_TO_END );
            sal_Int32 nMaskSize = static_cast<sal_Int32>(rObject.m_pSoftMaskStream->Tell());
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
            aMask.append( "/Group<</S/Transparency/CS/DeviceRGB>>\n" );
            aMask.append( "/Length " );
            aMask.append( nMaskSize );
            aMask.append( ">>\n"
                          "stream\n" );
            CHECK_RETURN2( updateObject( nMaskObject ) );
            checkAndEnableStreamEncryption(  nMaskObject );
            CHECK_RETURN2( writeBuffer( aMask.getStr(), aMask.getLength() ) );
            CHECK_RETURN2( writeBuffer( rObject.m_pSoftMaskStream->GetData(), nMaskSize ) );
            disableStreamEncryption();
            aMask.setLength( 0 );
            aMask.append( "\nendstream\n"
                          "endobj\n\n" );
            CHECK_RETURN2( writeBuffer( aMask.getStr(), aMask.getLength() ) );
        }
    }
    aLine.append( ">>\n"
                  "endobj\n\n" );
    CHECK_RETURN2( updateObject( rObject.m_nExtGStateObject ) );
    CHECK_RETURN2( writeBuffer( aLine.getStr(), aLine.getLength() ) );
}

bool PDFWriterImpl::writeGradientFunction( GradientEmit const & rObject )
{
    // LO internal gradient -> PDF shading type:
    //  * GradientStyle::Linear: axial shading, using sampled-function with 2 samples
    //                          [t=0:colorStart, t=1:colorEnd]
    //  * GradientStyle::Axial: axial shading, using sampled-function with 3 samples
    //                          [t=0:colorEnd, t=0.5:colorStart, t=1:colorEnd]
    //  * other styles: function shading with aSize.Width() * aSize.Height() samples
    sal_Int32 nFunctionObject = createObject();
    CHECK_RETURN( updateObject( nFunctionObject ) );

    ScopedVclPtrInstance< VirtualDevice > aDev;
    aDev->SetOutputSizePixel( rObject.m_aSize );
    aDev->SetMapMode( MapMode( MapUnit::MapPixel ) );
    if( m_aContext.ColorMode == PDFWriter::DrawGreyscale )
        aDev->SetDrawMode( aDev->GetDrawMode() |
                          ( DrawModeFlags::GrayLine | DrawModeFlags::GrayFill | DrawModeFlags::GrayText |
                            DrawModeFlags::GrayBitmap | DrawModeFlags::GrayGradient ) );
    aDev->DrawGradient( tools::Rectangle( Point( 0, 0 ), rObject.m_aSize ), rObject.m_aGradient );

    Bitmap aSample = aDev->GetBitmap( Point( 0, 0 ), rObject.m_aSize );
    Bitmap::ScopedReadAccess pAccess(aSample);

    Size aSize = aSample.GetSizePixel();

    sal_Int32 nStreamLengthObject = createObject();
    if (g_bDebugDisableCompression)
    {
        emitComment( "PDFWriterImpl::writeGradientFunction" );
    }
    OStringBuffer aLine( 120 );
    aLine.append( nFunctionObject );
    aLine.append( " 0 obj\n"
                  "<</FunctionType 0\n");
    switch (rObject.m_aGradient.GetStyle())
    {
        case GradientStyle::Linear:
        case GradientStyle::Axial:
            aLine.append("/Domain[ 0 1]\n");
            break;
        default:
            aLine.append("/Domain[ 0 1 0 1]\n");
    }
    aLine.append("/Size[ " );
    switch (rObject.m_aGradient.GetStyle())
    {
        case GradientStyle::Linear:
            aLine.append('2');
            break;
        case GradientStyle::Axial:
            aLine.append('3');
            break;
        default:
            aLine.append( static_cast<sal_Int32>(aSize.Width()) );
            aLine.append( ' ' );
            aLine.append( static_cast<sal_Int32>(aSize.Height()) );
    }
    aLine.append( " ]\n"
                  "/BitsPerSample 8\n"
                  "/Range[ 0 1 0 1 0 1 ]\n"
                  "/Order 3\n"
                  "/Length " );
    aLine.append( nStreamLengthObject );
    if (!g_bDebugDisableCompression)
        aLine.append( " 0 R\n"
                      "/Filter/FlateDecode"
                      ">>\n"
                      "stream\n" );
    else
        aLine.append( " 0 R\n"
                      ">>\n"
                      "stream\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    sal_uInt64 nStartStreamPos = 0;
    CHECK_RETURN( (osl::File::E_None == m_aFile.getPos(nStartStreamPos)) );

    checkAndEnableStreamEncryption( nFunctionObject );
    beginCompression();
    sal_uInt8 aCol[3];
    switch (rObject.m_aGradient.GetStyle())
    {
        case GradientStyle::Axial:
            aCol[0] = rObject.m_aGradient.GetEndColor().GetRed();
            aCol[1] = rObject.m_aGradient.GetEndColor().GetGreen();
            aCol[2] = rObject.m_aGradient.GetEndColor().GetBlue();
            CHECK_RETURN( writeBuffer( aCol, 3 ) );
            SAL_FALLTHROUGH;
        case GradientStyle::Linear:
        {
            aCol[0] = rObject.m_aGradient.GetStartColor().GetRed();
            aCol[1] = rObject.m_aGradient.GetStartColor().GetGreen();
            aCol[2] = rObject.m_aGradient.GetStartColor().GetBlue();
            CHECK_RETURN( writeBuffer( aCol, 3 ) );

            aCol[0] = rObject.m_aGradient.GetEndColor().GetRed();
            aCol[1] = rObject.m_aGradient.GetEndColor().GetGreen();
            aCol[2] = rObject.m_aGradient.GetEndColor().GetBlue();
            CHECK_RETURN( writeBuffer( aCol, 3 ) );
            break;
        }
        default:
            for( int y = aSize.Height()-1; y >= 0; y-- )
            {
                for( long x = 0; x < aSize.Width(); x++ )
                {
                    BitmapColor aColor = pAccess->GetColor( y, x );
                    aCol[0] = aColor.GetRed();
                    aCol[1] = aColor.GetGreen();
                    aCol[2] = aColor.GetBlue();
                    CHECK_RETURN( writeBuffer( aCol, 3 ) );
                }
            }
    }
    endCompression();
    disableStreamEncryption();

    sal_uInt64 nEndStreamPos = 0;
    CHECK_RETURN( (osl::File::E_None == m_aFile.getPos(nEndStreamPos)) );

    aLine.setLength( 0 );
    aLine.append( "\nendstream\nendobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    // write stream length
    CHECK_RETURN( updateObject( nStreamLengthObject ) );
    aLine.setLength( 0 );
    aLine.append( nStreamLengthObject );
    aLine.append( " 0 obj\n" );
    aLine.append( static_cast<sal_Int64>(nEndStreamPos-nStartStreamPos) );
    aLine.append( "\nendobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    CHECK_RETURN( updateObject( rObject.m_nObject ) );
    aLine.setLength( 0 );
    aLine.append( rObject.m_nObject );
    aLine.append( " 0 obj\n");
    switch (rObject.m_aGradient.GetStyle())
    {
        case GradientStyle::Linear:
        case GradientStyle::Axial:
            aLine.append("<</ShadingType 2\n");
            break;
        default:
            aLine.append("<</ShadingType 1\n");
    }
    aLine.append("/ColorSpace/DeviceRGB\n"
                  "/AntiAlias true\n");

    // Determination of shading axis
    // See: OutputDevice::ImplDrawLinearGradient for reference
    tools::Rectangle aRect;
    aRect.SetLeft(0);
    aRect.SetTop(0);
    aRect.SetRight( aSize.Width() );
    aRect.SetBottom( aSize.Height() );

    tools::Rectangle aBoundRect;
    Point     aCenter;
    sal_uInt16    nAngle = rObject.m_aGradient.GetAngle() % 3600;
    rObject.m_aGradient.GetBoundRect( aRect, aBoundRect, aCenter );

    const bool bLinear = (rObject.m_aGradient.GetStyle() == GradientStyle::Linear);
    double fBorder = aBoundRect.GetHeight() * rObject.m_aGradient.GetBorder() / 100.0;
    if ( !bLinear )
    {
        fBorder /= 2.0;
    }

    aBoundRect.AdjustBottom( -fBorder );
    if (!bLinear)
    {
        aBoundRect.AdjustTop(fBorder );
    }

    switch (rObject.m_aGradient.GetStyle())
    {
        case GradientStyle::Linear:
        case GradientStyle::Axial:
        {
            aLine.append("/Domain[ 0 1 ]\n"
                    "/Coords[ " );
            tools::Polygon aPoly( 2 );
            aPoly[0] = aBoundRect.BottomCenter();
            aPoly[1] = aBoundRect.TopCenter();
            aPoly.Rotate( aCenter, 3600 - nAngle );

            aLine.append( static_cast<sal_Int32>(aPoly[0].X()) );
            aLine.append( " " );
            aLine.append( static_cast<sal_Int32>(aPoly[0].Y()) );
            aLine.append( " " );
            aLine.append( static_cast<sal_Int32>(aPoly[1].X()));
            aLine.append( " ");
            aLine.append( static_cast<sal_Int32>(aPoly[1].Y()));
            aLine.append( " ]\n");
            aLine.append("/Extend [true true]\n");
            break;
        }
        default:
            aLine.append("/Domain[ 0 1 0 1 ]\n"
                    "/Matrix[ " );
            aLine.append( static_cast<sal_Int32>(aSize.Width()) );
            aLine.append( " 0 0 " );
            aLine.append( static_cast<sal_Int32>(aSize.Height()) );
            aLine.append( " 0 0 ]\n");
    }
    aLine.append("/Function " );
    aLine.append( nFunctionObject );
    aLine.append( " 0 R\n"
                  ">>\n"
                  "endobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    return true;
}

void PDFWriterImpl::writeJPG( JPGEmit& rObject )
{
    if (rObject.m_aReferenceXObject.m_aPDFData.hasElements() && !m_aContext.UseReferenceXObject)
    {
        writeReferenceXObject(rObject.m_aReferenceXObject);
        return;
    }

    CHECK_RETURN2( rObject.m_pStream );
    CHECK_RETURN2( updateObject( rObject.m_nObject ) );

    sal_Int32 nLength = 0;
    rObject.m_pStream->Seek( STREAM_SEEK_TO_END );
    nLength = rObject.m_pStream->Tell();
    rObject.m_pStream->Seek( STREAM_SEEK_TO_BEGIN );

    sal_Int32 nMaskObject = 0;
    if( !!rObject.m_aMask )
    {
        if( rObject.m_aMask.GetBitCount() == 1 ||
            ( rObject.m_aMask.GetBitCount() == 8 && m_aContext.Version >= PDFWriter::PDFVersion::PDF_1_4 && !m_bIsPDF_A1 )
            )
        {
            nMaskObject = createObject();
        }
        else if( m_bIsPDF_A1 )
            m_aErrors.insert( PDFWriter::Warning_Transparency_Omitted_PDFA );
        else if( m_aContext.Version < PDFWriter::PDFVersion::PDF_1_4 )
            m_aErrors.insert( PDFWriter::Warning_Transparency_Omitted_PDF13 );

    }
    if (g_bDebugDisableCompression)
    {
        emitComment( "PDFWriterImpl::writeJPG" );
    }

    OStringBuffer aLine(200);
    aLine.append( rObject.m_nObject );
    aLine.append( " 0 obj\n"
                  "<</Type/XObject/Subtype/Image/Width " );
    aLine.append( static_cast<sal_Int32>(rObject.m_aID.m_aPixelSize.Width()) );
    aLine.append( " /Height " );
    aLine.append( static_cast<sal_Int32>(rObject.m_aID.m_aPixelSize.Height()) );
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
    CHECK_RETURN2( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    checkAndEnableStreamEncryption( rObject.m_nObject );
    CHECK_RETURN2( writeBuffer( rObject.m_pStream->GetData(), nLength ) );
    disableStreamEncryption();

    aLine.setLength( 0 );
    aLine.append( "\nendstream\nendobj\n\n" );
    CHECK_RETURN2( writeBuffer( aLine.getStr(), aLine.getLength() ) );

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

    writeReferenceXObject(rObject.m_aReferenceXObject);
}

sal_Int32 PDFWriterImpl::copyExternalResource(SvMemoryStream& rDocBuffer, filter::PDFObjectElement& rObject, std::map<sal_Int32, sal_Int32>& rCopiedResources)
{
    auto it = rCopiedResources.find(rObject.GetObjectValue());
    if (it != rCopiedResources.end())
        // This resource was already copied once, nothing to do.
        return it->second;

    sal_Int32 nObject = createObject();
    // Remember what is the ID of this object in our output.
    rCopiedResources[rObject.GetObjectValue()] = nObject;
    SAL_INFO("vcl.pdfwriter", "PDFWriterImpl::copyExternalResource: " << rObject.GetObjectValue() << " -> " << nObject);

    OStringBuffer aLine;
    aLine.append(nObject);
    aLine.append(" 0 obj\n");
    if (rObject.GetDictionary())
    {
        aLine.append("<<");

        // Complex case: can't copy the dictionary byte array as is, as it may contain references.
        bool bDone = false;
        sal_uInt64 nCopyStart = 0;
        for (auto pReference : rObject.GetDictionaryReferences())
        {
            if (pReference)
            {
                filter::PDFObjectElement* pReferenced = pReference->LookupObject();
                if (pReferenced)
                {
                    // Copy the referenced object.
                    sal_Int32 nRef = copyExternalResource(rDocBuffer, *pReferenced, rCopiedResources);

                    sal_uInt64 nReferenceStart = pReference->GetObjectElement().GetLocation();
                    sal_uInt64 nReferenceEnd = pReference->GetOffset();
                    sal_uInt64 nOffset = 0;
                    if (nCopyStart == 0)
                        // Dict start -> reference start.
                        nOffset = rObject.GetDictionaryOffset();
                    else
                        // Previous reference end -> reference start.
                        nOffset = nCopyStart;
                    aLine.append(static_cast<const sal_Char*>(rDocBuffer.GetData()) + nOffset, nReferenceStart - nOffset);
                    // Write the updated reference.
                    aLine.append(" ");
                    aLine.append(nRef);
                    aLine.append(" 0 R");
                    // Start copying here next time.
                    nCopyStart = nReferenceEnd;

                    bDone = true;
                }
            }
        }

        if (bDone)
        {
            // Copy the last part here, in the complex case.
            sal_uInt64 nDictEnd = rObject.GetDictionaryOffset() + rObject.GetDictionaryLength();
            aLine.append(static_cast<const sal_Char*>(rDocBuffer.GetData()) + nCopyStart, nDictEnd - nCopyStart);
        }
        else
            // Can copy it as-is.
            aLine.append(static_cast<const sal_Char*>(rDocBuffer.GetData()) + rObject.GetDictionaryOffset(), rObject.GetDictionaryLength());

        aLine.append(">>\n");
    }

    if (filter::PDFStreamElement* pStream = rObject.GetStream())
    {
        aLine.append("stream\n");
        SvMemoryStream& rStream = pStream->GetMemory();
        aLine.append(static_cast<const sal_Char*>(rStream.GetData()), rStream.GetSize());
        aLine.append("\nendstream\n");
    }

    if (filter::PDFArrayElement* pArray = rObject.GetArray())
    {
        aLine.append("[");

        const std::vector<filter::PDFElement*>& rElements = pArray->GetElements();
        bool bDone = false;
        // Complex case: can't copy the array byte array as is, as it may contain references.
        sal_uInt64 nCopyStart = 0;
        for (const auto pElement : rElements)
        {
            auto pReference = dynamic_cast<filter::PDFReferenceElement*>(pElement);
            if (pReference)
            {
                filter::PDFObjectElement* pReferenced = pReference->LookupObject();
                if (pReferenced)
                {
                    // Copy the referenced object.
                    sal_Int32 nRef = copyExternalResource(rDocBuffer, *pReferenced, rCopiedResources);

                    sal_uInt64 nReferenceStart = pReference->GetObjectElement().GetLocation();
                    sal_uInt64 nReferenceEnd = pReference->GetOffset();
                    sal_uInt64 nOffset = 0;
                    if (nCopyStart == 0)
                        // Array start -> reference start.
                        nOffset = rObject.GetArrayOffset();
                    else
                        // Previous reference end -> reference start.
                        nOffset = nCopyStart;
                    aLine.append(static_cast<const sal_Char*>(rDocBuffer.GetData()) + nOffset, nReferenceStart - nOffset);

                    // Write the updated reference.
                    aLine.append(" ");
                    aLine.append(nRef);
                    aLine.append(" 0 R");
                    // Start copying here next time.
                    nCopyStart = nReferenceEnd;

                    bDone = true;
                }
            }
        }

        if (bDone)
        {
            // Copy the last part here, in the complex case.
            sal_uInt64 nArrEnd = rObject.GetArrayOffset() + rObject.GetArrayLength();
            aLine.append(static_cast<const sal_Char*>(rDocBuffer.GetData()) + nCopyStart, nArrEnd - nCopyStart);
        }
        else
            // Can copy it as-is.
            aLine.append(static_cast<const sal_Char*>(rDocBuffer.GetData()) + rObject.GetArrayOffset(), rObject.GetArrayLength());

        aLine.append("]\n");
    }

    // If the object has a number element outside a dictionary or array, copy that.
    if (filter::PDFNumberElement* pNumber = rObject.GetNumberElement())
    {
        aLine.append(static_cast<const sal_Char*>(rDocBuffer.GetData()) + pNumber->GetLocation(), pNumber->GetLength());
        aLine.append("\n");
    }


    aLine.append("endobj\n\n");

    // We have the whole object, now write it to the output.
    if (!updateObject(nObject))
        return -1;
    if (!writeBuffer(aLine.getStr(), aLine.getLength()))
        return -1;

    return nObject;
}

OString PDFWriterImpl::copyExternalResources(filter::PDFObjectElement& rPage, const OString& rKind, std::map<sal_Int32, sal_Int32>& rCopiedResources)
{
    // A name - object ID map, IDs as they appear in our output, not the
    // original ones.
    std::map<OString, sal_Int32> aRet;

    // Get the rKind subset of the resource dictionary.
    std::map<OString, filter::PDFElement*> aItems;
    if (auto pResources = dynamic_cast<filter::PDFDictionaryElement*>(rPage.Lookup("Resources")))
    {
        // Resources is a direct dictionary.
        if (auto pDictionary = dynamic_cast<filter::PDFDictionaryElement*>(pResources->LookupElement(rKind)))
            aItems = pDictionary->GetItems();
    }
    else if (filter::PDFObjectElement* pPageResources = rPage.LookupObject("Resources"))
    {
        // Resources is an indirect object.
        filter::PDFElement* pValue = pPageResources->Lookup(rKind);
        if (auto pDictionary = dynamic_cast<filter::PDFDictionaryElement*>(pValue))
            // Kind is a direct dictionary.
            aItems = pDictionary->GetItems();
        else if (filter::PDFObjectElement* pObject = pPageResources->LookupObject(rKind))
            // Kind is an indirect object.
            aItems = pObject->GetDictionaryItems();
    }
    if (aItems.empty())
        return OString();

    SvMemoryStream& rDocBuffer = rPage.GetDocument().GetEditBuffer();

    for (const auto& rItem : aItems)
    {
        // For each item copy it over to our output then insert it into aRet.
        auto pReference = dynamic_cast<filter::PDFReferenceElement*>(rItem.second);
        if (!pReference)
            continue;

        filter::PDFObjectElement* pValue = pReference->LookupObject();
        if (!pValue)
            continue;

        // Then copying over an object copy its dictionary and its stream.
        sal_Int32 nObject = copyExternalResource(rDocBuffer, *pValue, rCopiedResources);
        aRet[rItem.first] = nObject;
    }

    // Build the dictionary entry string.
    OString sRet = "/" + rKind + "<<";
    for (const auto& rPair : aRet)
    {
        sRet += "/" + rPair.first + " " + OString::number(rPair.second) + " 0 R";
    }
    sRet += ">>";

    return sRet;
}

void PDFWriterImpl::writeReferenceXObject(ReferenceXObjectEmit& rEmit)
{
    if (rEmit.m_nFormObject <= 0)
        return;

    // Count /Matrix and /BBox.
    // vcl::ImportPDF() works with 96 DPI so use the same values here, too.
    sal_Int32 nOldDPIX = getReferenceDevice()->GetDPIX();
    getReferenceDevice()->SetDPIX(96);
    sal_Int32 nOldDPIY = getReferenceDevice()->GetDPIY();
    getReferenceDevice()->SetDPIY(96);
    Size aSize = getReferenceDevice()->PixelToLogic(rEmit.m_aPixelSize, MapMode(m_aMapMode.GetMapUnit()));
    getReferenceDevice()->SetDPIX(nOldDPIX);
    getReferenceDevice()->SetDPIY(nOldDPIY);
    double fScaleX = 1.0 / aSize.Width();
    double fScaleY = 1.0 / aSize.Height();

    sal_Int32 nWrappedFormObject = 0;
    if (!m_aContext.UseReferenceXObject)
    {
        // Parse the PDF data, we need that to write the PDF dictionary of our
        // object.
        SvMemoryStream aPDFStream;
        aPDFStream.WriteBytes(rEmit.m_aPDFData.getArray(), rEmit.m_aPDFData.getLength());
        aPDFStream.Seek(0);
        filter::PDFDocument aPDFDocument;
        if (!aPDFDocument.Read(aPDFStream))
        {
            SAL_WARN("vcl.pdfwriter", "PDFWriterImpl::writeReferenceXObject: reading the PDF document failed");
            return;
        }
        std::vector<filter::PDFObjectElement*> aPages = aPDFDocument.GetPages();
        if (aPages.empty())
        {
            SAL_WARN("vcl.pdfwriter", "PDFWriterImpl::writeReferenceXObject: no pages");
            return;
        }

        filter::PDFObjectElement* pPage = aPages[0];
        if (!pPage)
        {
            SAL_WARN("vcl.pdfwriter", "PDFWriterImpl::writeReferenceXObject: no page");
            return;
        }

        std::vector<filter::PDFObjectElement*> aContentStreams;
        if (filter::PDFObjectElement* pContentStream = pPage->LookupObject("Contents"))
            aContentStreams.push_back(pContentStream);
        else if (auto pArray = dynamic_cast<filter::PDFArrayElement*>(pPage->Lookup("Contents")))
        {
            for (const auto pElement : pArray->GetElements())
            {
                auto pReference = dynamic_cast<filter::PDFReferenceElement*>(pElement);
                if (!pReference)
                    continue;

                filter::PDFObjectElement* pObject = pReference->LookupObject();
                if (!pObject)
                    continue;

                aContentStreams.push_back(pObject);
            }
        }

        if (aContentStreams.empty())
        {
            SAL_WARN("vcl.pdfwriter", "PDFWriterImpl::writeReferenceXObject: no content stream");
            return;
        }

        // Maps from source object id (PDF image) to target object id (export result).
        std::map<sal_Int32, sal_Int32> aCopiedResources;

        nWrappedFormObject = createObject();
        // Write the form XObject wrapped below. This is a separate object from
        // the wrapper, this way there is no need to alter the stream contents.

        OStringBuffer aLine;
        aLine.append(nWrappedFormObject);
        aLine.append(" 0 obj\n");
        aLine.append("<< /Type /XObject");
        aLine.append(" /Subtype /Form");
        aLine.append(" /Resources <<");
        static const std::initializer_list<OString> aKeys =
        {
            "ColorSpace",
            "ExtGState",
            "Font",
            "XObject",
            "Shading"
        };
        for (const auto& rKey : aKeys)
            aLine.append(copyExternalResources(*pPage, rKey, aCopiedResources));
        aLine.append(">>");
        aLine.append(" /BBox [ 0 0 ");
        aLine.append(aSize.Width());
        aLine.append(" ");
        aLine.append(aSize.Height());
        aLine.append(" ]");

        if (!g_bDebugDisableCompression)
            aLine.append(" /Filter/FlateDecode");
        aLine.append(" /Length ");

        SvMemoryStream aStream;
        for (auto pContent : aContentStreams)
        {
            filter::PDFStreamElement* pPageStream = pContent->GetStream();
            if (!pPageStream)
            {
                SAL_WARN("vcl.pdfwriter", "PDFWriterImpl::writeReferenceXObject: contents has no stream");
                continue;
            }

            SvMemoryStream& rPageStream = pPageStream->GetMemory();

            auto pFilter = dynamic_cast<filter::PDFNameElement*>(pContent->Lookup("Filter"));
            if (pFilter)
            {
                if (pFilter->GetValue() != "FlateDecode")
                    continue;

                SvMemoryStream aMemoryStream;
                ZCodec aZCodec;
                rPageStream.Seek(0);
                aZCodec.BeginCompression();
                aZCodec.Decompress(rPageStream, aMemoryStream);
                if (!aZCodec.EndCompression())
                {
                    SAL_WARN("vcl.pdfwriter", "PDFWriterImpl::writeReferenceXObject: decompression failed");
                    continue;
                }

                aStream.WriteBytes(aMemoryStream.GetData(), aMemoryStream.GetSize());
            }
            else
                aStream.WriteBytes(rPageStream.GetData(), rPageStream.GetSize());
        }

        compressStream(&aStream);
        sal_Int32 nLength = aStream.Tell();
        aLine.append(nLength);

        aLine.append(">>\nstream\n");
        // Copy the original page streams to the form XObject stream.
        aLine.append(static_cast<const sal_Char*>(aStream.GetData()), aStream.GetSize());
        aLine.append("\nendstream\nendobj\n\n");
        if (!updateObject(nWrappedFormObject))
            return;
        if (!writeBuffer(aLine.getStr(), aLine.getLength()))
            return;
    }

    OStringBuffer aLine;
    if (!updateObject(rEmit.m_nFormObject))
        return;

    // Now have all the info to write the form XObject.
    aLine.append(rEmit.m_nFormObject);
    aLine.append(" 0 obj\n");
    aLine.append("<< /Type /XObject");
    aLine.append(" /Subtype /Form");
    aLine.append(" /Resources << /XObject<<");

    sal_Int32 nObject = m_aContext.UseReferenceXObject ? rEmit.m_nBitmapObject : nWrappedFormObject;
    aLine.append(" /Im");
    aLine.append(nObject);
    aLine.append(" ");
    aLine.append(nObject);
    aLine.append(" 0 R");

    aLine.append(">> >>");
    aLine.append(" /Matrix [ ");
    appendDouble(fScaleX, aLine);
    aLine.append(" 0 0 ");
    appendDouble(fScaleY, aLine);
    aLine.append(" 0 0 ]");
    aLine.append(" /BBox [ 0 0 ");
    aLine.append(aSize.Width());
    aLine.append(" ");
    aLine.append(aSize.Height());
    aLine.append(" ]\n");

    if (m_aContext.UseReferenceXObject && rEmit.m_nEmbeddedObject > 0)
    {
        // Write the reference dictionary.
        aLine.append("/Ref<< /F << /Type /Filespec /F (<embedded file>) /EF << /F ");
        aLine.append(rEmit.m_nEmbeddedObject);
        aLine.append(" 0 R >> >> /Page 0 >>\n");
    }

    aLine.append("/Length ");

    OStringBuffer aStream;
    aStream.append("q ");
    if (m_aContext.UseReferenceXObject)
    {
        // Reference XObject markup is used, just refer to the fallback bitmap
        // here.
        aStream.append(aSize.Width());
        aStream.append(" 0 0 ");
        aStream.append(aSize.Height());
        aStream.append(" 0 0 cm\n");
        aStream.append("/Im");
        aStream.append(rEmit.m_nBitmapObject);
        aStream.append(" Do\n");
    }
    else
    {
        // Reset line width to the default.
        aStream.append(" 1 w\n");

        // No reference XObject, draw the form XObject containing the original
        // page streams.
        aStream.append("/Im");
        aStream.append(nWrappedFormObject);
        aStream.append(" Do\n");
    }
    aStream.append("Q");
    aLine.append(aStream.getLength());

    aLine.append(">>\nstream\n");
    aLine.append(aStream.getStr());
    aLine.append("\nendstream\nendobj\n\n");
    CHECK_RETURN2(writeBuffer(aLine.getStr(), aLine.getLength()));
}

namespace
{
    unsigned char reverseByte(unsigned char b)
    {
        b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
        b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
        b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
        return b;
    }

    //tdf#103051 convert any N1BitLsbPal to N1BitMsbPal
    Bitmap getExportBitmap(const Bitmap &rBitmap)
    {
        Bitmap::ScopedReadAccess pAccess(const_cast<Bitmap&>(rBitmap));
        const ScanlineFormat eFormat = pAccess->GetScanlineFormat();
        if (eFormat != ScanlineFormat::N1BitLsbPal)
            return rBitmap;
        Bitmap aNewBmp(rBitmap);
        BitmapScopedWriteAccess xWriteAcc(aNewBmp);
        const int nScanLineBytes = (pAccess->Width() + 7U) / 8U;
        for (long nY = 0L; nY < xWriteAcc->Height(); ++nY)
        {
            Scanline pBitSwap = xWriteAcc->GetScanline(nY);
            for (int x = 0; x < nScanLineBytes; ++x)
                pBitSwap[x] = reverseByte(pBitSwap[x]);
        }
        return aNewBmp;
    }
}

bool PDFWriterImpl::writeBitmapObject( BitmapEmit& rObject, bool bMask )
{
    if (rObject.m_aReferenceXObject.m_aPDFData.hasElements() && !m_aContext.UseReferenceXObject)
    {
        writeReferenceXObject(rObject.m_aReferenceXObject);
        return true;
    }

    CHECK_RETURN( updateObject( rObject.m_nObject ) );

    Bitmap  aBitmap;
    Color   aTransparentColor( COL_TRANSPARENT );
    bool    bWriteMask = false;
    if( ! bMask )
    {
        aBitmap = getExportBitmap(rObject.m_aBitmap.GetBitmap());
        if( rObject.m_aBitmap.IsAlpha() )
        {
            if( m_aContext.Version >= PDFWriter::PDFVersion::PDF_1_4 )
                bWriteMask = true;
            // else draw without alpha channel
        }
        else
        {
            switch( rObject.m_aBitmap.GetTransparentType() )
            {
                case TransparentType::NONE:
                    break;
                case TransparentType::Color:
                    aTransparentColor = rObject.m_aBitmap.GetTransparentColor();
                    break;
                case TransparentType::Bitmap:
                    bWriteMask = true;
                    break;
            }
        }
    }
    else
    {
        if( m_aContext.Version < PDFWriter::PDFVersion::PDF_1_4 || ! rObject.m_aBitmap.IsAlpha() )
        {
            aBitmap = getExportBitmap(rObject.m_aBitmap.GetMask());
            aBitmap.Convert( BmpConversion::N1BitThreshold );
            SAL_WARN_IF( aBitmap.GetBitCount() != 1, "vcl.pdfwriter", "mask conversion failed" );
        }
        else if( aBitmap.GetBitCount() != 8 )
        {
            aBitmap = getExportBitmap(rObject.m_aBitmap.GetAlpha().GetBitmap());
            aBitmap.Convert( BmpConversion::N8BitGreys );
            SAL_WARN_IF( aBitmap.GetBitCount() != 8, "vcl.pdfwriter", "alpha mask conversion failed" );
        }
    }

    Bitmap::ScopedReadAccess pAccess(aBitmap);

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

    if (g_bDebugDisableCompression)
    {
        emitComment( "PDFWriterImpl::writeBitmapObject" );
    }
    OStringBuffer aLine(1024);
    aLine.append( rObject.m_nObject );
    aLine.append( " 0 obj\n"
                  "<</Type/XObject/Subtype/Image/Width " );
    aLine.append( static_cast<sal_Int32>(aBitmap.GetSizePixel().Width()) );
    aLine.append( "/Height " );
    aLine.append( static_cast<sal_Int32>(aBitmap.GetSizePixel().Height()) );
    aLine.append( "/BitsPerComponent " );
    aLine.append( nBitsPerComponent );
    aLine.append( "/Length " );
    aLine.append( nStreamLengthObject );
    aLine.append( " 0 R\n" );
    if (!g_bDebugDisableCompression)
    {
        if( nBitsPerComponent != 1 )
        {
            aLine.append( "/Filter/FlateDecode" );
        }
        else
        {
            aLine.append( "/Filter/CCITTFaxDecode/DecodeParms<</K -1/BlackIs1 true/Columns " );
            aLine.append( static_cast<sal_Int32>(aBitmap.GetSizePixel().Width()) );
            aLine.append( ">>\n" );
        }
    }
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
                sal_uInt16 nBlackIndex = pAccess->GetBestPaletteIndex( BitmapColor( COL_BLACK ) );
                assert( nBlackIndex == 0 || nBlackIndex == 1);
                sal_uInt16 nWhiteIndex = pAccess->GetBestPaletteIndex( BitmapColor( COL_WHITE ) );
                if( pAccess->GetPalette()[nBlackIndex] == BitmapColor( COL_BLACK ) &&
                    pAccess->GetPalette()[nWhiteIndex] == BitmapColor( COL_WHITE ) )
                {
                    // It is black and white
                    if( nBlackIndex == 1 )
                        aLine.append( "/Decode[1 0]\n" );
                }
                else
                {
                    // It is two levels of grey
                    aLine.append( "/Decode[" );
                    assert( pAccess->GetPalette()[0].GetRed() == pAccess->GetPalette()[0].GetGreen() &&
                            pAccess->GetPalette()[0].GetRed() == pAccess->GetPalette()[0].GetBlue() &&
                            pAccess->GetPalette()[1].GetRed() == pAccess->GetPalette()[1].GetGreen() &&
                            pAccess->GetPalette()[1].GetRed() == pAccess->GetPalette()[1].GetBlue() );
                    aLine.append( pAccess->GetPalette()[0].GetRed() / 255.0 );
                    aLine.append( " " );
                    aLine.append( pAccess->GetPalette()[1].GetRed() / 255.0 );
                    aLine.append( "]\n" );
                }
            }
        }
        else
        {
            aLine.append( "[ /Indexed/DeviceRGB " );
            aLine.append( static_cast<sal_Int32>(pAccess->GetPaletteEntryCount()-1) );
            aLine.append( "\n<" );
            if( m_aContext.Encryption.Encrypt() )
            {
                enableStringEncryption( rObject.m_nObject );
                //check encryption buffer size
                if( checkEncryptionBufferSize( pAccess->GetPaletteEntryCount()*3 ) )
                {
                    int nChar = 0;
                    //fill the encryption buffer
                    for( sal_uInt16 i = 0; i < pAccess->GetPaletteEntryCount(); i++ )
                    {
                        const BitmapColor& rColor = pAccess->GetPaletteColor( i );
                        m_pEncryptionBuffer[nChar++] = rColor.GetRed();
                        m_pEncryptionBuffer[nChar++] = rColor.GetGreen();
                        m_pEncryptionBuffer[nChar++] = rColor.GetBlue();
                    }
                    //encrypt the colorspace lookup table
                    rtl_cipher_encodeARCFOUR( m_aCipher, m_pEncryptionBuffer, nChar, m_pEncryptionBuffer, nChar );
                    //now queue the data for output
                    nChar = 0;
                    for( sal_uInt16 i = 0; i < pAccess->GetPaletteEntryCount(); i++ )
                    {
                        appendHex(m_pEncryptionBuffer[nChar++], aLine );
                        appendHex(m_pEncryptionBuffer[nChar++], aLine );
                        appendHex(m_pEncryptionBuffer[nChar++], aLine );
                    }
                }
            }
            else //no encryption requested (PDF/A-1a program flow drops here)
            {
                for( sal_uInt16 i = 0; i < pAccess->GetPaletteEntryCount(); i++ )
                {
                    const BitmapColor& rColor = pAccess->GetPaletteColor( i );
                    appendHex( rColor.GetRed(), aLine );
                    appendHex( rColor.GetGreen(), aLine );
                    appendHex( rColor.GetBlue(), aLine );
                }
            }
            aLine.append( ">\n]\n" );
        }
    }
    else
    {
        if( aBitmap.GetBitCount() == 1 )
        {
            aLine.append( "/ImageMask true\n" );
            sal_Int32 nBlackIndex = pAccess->GetBestPaletteIndex( BitmapColor( COL_BLACK ) );
            SAL_WARN_IF( nBlackIndex != 0 && nBlackIndex != 1, "vcl.pdfwriter", "wrong black index" );
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

    if( ! bMask && m_aContext.Version > PDFWriter::PDFVersion::PDF_1_2 && !m_bIsPDF_A1 )
    {
        if( bWriteMask )
        {
            nMaskObject = createObject();
            if( rObject.m_aBitmap.IsAlpha() && m_aContext.Version > PDFWriter::PDFVersion::PDF_1_3 )
                aLine.append( "/SMask " );
            else
                aLine.append( "/Mask " );
            aLine.append( nMaskObject );
            aLine.append( " 0 R\n" );
        }
        else if( aTransparentColor != COL_TRANSPARENT )
        {
            aLine.append( "/Mask[ " );
            if( bTrueColor )
            {
                aLine.append( static_cast<sal_Int32>(aTransparentColor.GetRed()) );
                aLine.append( ' ' );
                aLine.append( static_cast<sal_Int32>(aTransparentColor.GetRed()) );
                aLine.append( ' ' );
                aLine.append( static_cast<sal_Int32>(aTransparentColor.GetGreen()) );
                aLine.append( ' ' );
                aLine.append( static_cast<sal_Int32>(aTransparentColor.GetGreen()) );
                aLine.append( ' ' );
                aLine.append( static_cast<sal_Int32>(aTransparentColor.GetBlue()) );
                aLine.append( ' ' );
                aLine.append( static_cast<sal_Int32>(aTransparentColor.GetBlue()) );
            }
            else
            {
                sal_Int32 nIndex = pAccess->GetBestPaletteIndex( BitmapColor( aTransparentColor ) );
                aLine.append( nIndex );
            }
            aLine.append( " ]\n" );
        }
    }
    else if( m_bIsPDF_A1 && (bWriteMask || aTransparentColor != COL_TRANSPARENT) )
        m_aErrors.insert( PDFWriter::Warning_Transparency_Omitted_PDFA );

    aLine.append( ">>\n"
                  "stream\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    sal_uInt64 nStartPos = 0;
    CHECK_RETURN( (osl::File::E_None == m_aFile.getPos(nStartPos)) );

    checkAndEnableStreamEncryption( rObject.m_nObject );
    if (!g_bDebugDisableCompression && nBitsPerComponent == 1)
    {
        writeG4Stream(pAccess.get());
    }
    else
    {
        beginCompression();
        if( ! bTrueColor || pAccess->GetScanlineFormat() == ScanlineFormat::N24BitTcRgb )
        {
            //With PDF bitmaps, each row is padded to a BYTE boundary (multiple of 8 bits).
            const int nScanLineBytes = ((pAccess->GetBitCount() * pAccess->Width()) + 7U) / 8U;

            for( long i = 0; i < pAccess->Height(); i++ )
            {
                CHECK_RETURN( writeBuffer( pAccess->GetScanline( i ), nScanLineBytes ) );
            }
        }
        else
        {
            const int nScanLineBytes = pAccess->Width()*3;
            std::unique_ptr<sal_uInt8[]> xCol(new sal_uInt8[nScanLineBytes]);
            for( long y = 0; y < pAccess->Height(); y++ )
            {
                for( long x = 0; x < pAccess->Width(); x++ )
                {
                    BitmapColor aColor = pAccess->GetColor( y, x );
                    xCol[3*x+0] = aColor.GetRed();
                    xCol[3*x+1] = aColor.GetGreen();
                    xCol[3*x+2] = aColor.GetBlue();
                }
                CHECK_RETURN(writeBuffer(xCol.get(), nScanLineBytes));
            }
        }
        endCompression();
    }
    disableStreamEncryption();

    sal_uInt64 nEndPos = 0;
    CHECK_RETURN( (osl::File::E_None == m_aFile.getPos(nEndPos)) );
    aLine.setLength( 0 );
    aLine.append( "\nendstream\nendobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    CHECK_RETURN( updateObject( nStreamLengthObject ) );
    aLine.setLength( 0 );
    aLine.append( nStreamLengthObject );
    aLine.append( " 0 obj\n" );
    aLine.append( static_cast<sal_Int64>(nEndPos-nStartPos) );
    aLine.append( "\nendobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    if( nMaskObject )
    {
        BitmapEmit aEmit;
        aEmit.m_nObject             = nMaskObject;
        aEmit.m_aBitmap             = rObject.m_aBitmap;
        return writeBitmapObject( aEmit, true );
    }

    writeReferenceXObject(rObject.m_aReferenceXObject);

    return true;
}

void PDFWriterImpl::createEmbeddedFile(const Graphic& rGraphic, ReferenceXObjectEmit& rEmit, sal_Int32 nBitmapObject)
{
    // The bitmap object is always a valid identifier, even if the graphic has
    // no pdf data.
    rEmit.m_nBitmapObject = nBitmapObject;

    if (!rGraphic.hasPdfData())
        return;

    if (m_aContext.UseReferenceXObject)
    {
        // Store the original PDF data as an embedded file.
        m_aEmbeddedFiles.emplace_back();
        m_aEmbeddedFiles.back().m_nObject = createObject();
        m_aEmbeddedFiles.back().m_aData = *rGraphic.getPdfData();

        rEmit.m_nEmbeddedObject = m_aEmbeddedFiles.back().m_nObject;
    }
    else
        rEmit.m_aPDFData = *rGraphic.getPdfData();

    rEmit.m_nFormObject = createObject();
    rEmit.m_aPixelSize = rGraphic.GetPrefSize();
}

void PDFWriterImpl::drawJPGBitmap( SvStream& rDCTData, bool bIsTrueColor, const Size& rSizePixel, const tools::Rectangle& rTargetArea, const Bitmap& rMask, const Graphic& rGraphic )
{
    MARK( "drawJPGBitmap" );

    OStringBuffer aLine( 80 );
    updateGraphicsState();

    // #i40055# sanity check
    if( ! (rTargetArea.GetWidth() && rTargetArea.GetHeight() ) )
        return;
    if( ! (rSizePixel.Width() && rSizePixel.Height()) )
        return;

    rDCTData.Seek( 0 );
    if( bIsTrueColor && m_aContext.ColorMode == PDFWriter::DrawGreyscale )
    {
        // need to convert to grayscale;
        // load stream to bitmap and draw the bitmap instead
        Graphic aGraphic;
        GraphicConverter::Import( rDCTData, aGraphic, ConvertDataFormat::JPG );
        if( !!rMask && rMask.GetSizePixel() == aGraphic.GetSizePixel() )
        {
            Bitmap aBmp( aGraphic.GetBitmapEx().GetBitmap() );
            BitmapEx aBmpEx( aBmp, rMask );
            drawBitmap( rTargetArea.TopLeft(), rTargetArea.GetSize(), aBmpEx );
        }
        else
            drawBitmap( rTargetArea.TopLeft(), rTargetArea.GetSize(), aGraphic.GetBitmapEx() );
        return;
    }

    SvMemoryStream* pStream = new SvMemoryStream;
    pStream->WriteStream( rDCTData );
    pStream->Seek( STREAM_SEEK_TO_END );

    BitmapID aID;
    aID.m_aPixelSize    = rSizePixel;
    aID.m_nSize         = pStream->Tell();
    pStream->Seek( STREAM_SEEK_TO_BEGIN );
    aID.m_nChecksum     = vcl_get_checksum( 0, pStream->GetData(), aID.m_nSize );
    if( ! rMask.IsEmpty() )
        aID.m_nMaskChecksum = rMask.GetChecksum();

    std::vector< JPGEmit >::const_iterator it = std::find_if(m_aJPGs.begin(), m_aJPGs.end(),
                                             [&](const JPGEmit& arg) { return aID == arg.m_aID; });
    if( it == m_aJPGs.end() )
    {
        m_aJPGs.emplace( m_aJPGs.begin() );
        JPGEmit& rEmit = m_aJPGs.front();
        if (!rGraphic.hasPdfData() || m_aContext.UseReferenceXObject)
            rEmit.m_nObject = createObject();
        rEmit.m_aID         = aID;
        rEmit.m_pStream.reset( pStream );
        rEmit.m_bTrueColor  = bIsTrueColor;
        if( !! rMask && rMask.GetSizePixel() == rSizePixel )
            rEmit.m_aMask   = rMask;
        createEmbeddedFile(rGraphic, rEmit.m_aReferenceXObject, rEmit.m_nObject);

        it = m_aJPGs.begin();
    }
    else
        delete pStream;

    aLine.append( "q " );
    sal_Int32 nCheckWidth = 0;
    m_aPages.back().appendMappedLength( static_cast<sal_Int32>(rTargetArea.GetWidth()), aLine, false, &nCheckWidth );
    aLine.append( " 0 0 " );
    sal_Int32 nCheckHeight = 0;
    m_aPages.back().appendMappedLength( static_cast<sal_Int32>(rTargetArea.GetHeight()), aLine, true, &nCheckHeight );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( rTargetArea.BottomLeft(), aLine );
    aLine.append( " cm\n/Im" );
    sal_Int32 nObject = it->m_aReferenceXObject.getObject();
    aLine.append(nObject);
    aLine.append( " Do Q\n" );
    if( nCheckWidth == 0 || nCheckHeight == 0 )
    {
        // #i97512# avoid invalid current matrix
        aLine.setLength( 0 );
        aLine.append( "\n%jpeg image /Im" );
        aLine.append( it->m_nObject );
        aLine.append( " scaled to zero size, omitted\n" );
    }
    writeBuffer( aLine.getStr(), aLine.getLength() );

    OStringBuffer aObjName( 16 );
    aObjName.append( "Im" );
    aObjName.append(nObject);
    pushResource( ResXObject, aObjName.makeStringAndClear(), nObject );

}

void PDFWriterImpl::drawBitmap( const Point& rDestPoint, const Size& rDestSize, const BitmapEmit& rBitmap, const Color& rFillColor )
{
    OStringBuffer aLine( 80 );
    updateGraphicsState();

    aLine.append( "q " );
    if( rFillColor != COL_TRANSPARENT )
    {
        appendNonStrokingColor( rFillColor, aLine );
        aLine.append( ' ' );
    }
    sal_Int32 nCheckWidth = 0;
    m_aPages.back().appendMappedLength( static_cast<sal_Int32>(rDestSize.Width()), aLine, false, &nCheckWidth );
    aLine.append( " 0 0 " );
    sal_Int32 nCheckHeight = 0;
    m_aPages.back().appendMappedLength( static_cast<sal_Int32>(rDestSize.Height()), aLine, true, &nCheckHeight );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( rDestPoint + Point( 0, rDestSize.Height()-1 ), aLine );
    aLine.append( " cm\n/Im" );
    sal_Int32 nObject = rBitmap.m_aReferenceXObject.getObject();
    aLine.append(nObject);
    aLine.append( " Do Q\n" );
    if( nCheckWidth == 0 || nCheckHeight == 0 )
    {
        // #i97512# avoid invalid current matrix
        aLine.setLength( 0 );
        aLine.append( "\n%bitmap image /Im" );
        aLine.append( rBitmap.m_nObject );
        aLine.append( " scaled to zero size, omitted\n" );
    }
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

const PDFWriterImpl::BitmapEmit& PDFWriterImpl::createBitmapEmit( const BitmapEx& i_rBitmap, const Graphic& rGraphic )
{
    BitmapEx aBitmap( i_rBitmap );
    if( m_aContext.ColorMode == PDFWriter::DrawGreyscale )
    {
        BmpConversion eConv = BmpConversion::N8BitGreys;
        int nDepth = aBitmap.GetBitmap().GetBitCount();
        if( nDepth <= 4 )
            eConv = BmpConversion::N4BitGreys;
        if( nDepth > 1 )
            aBitmap.Convert( eConv );
    }
    BitmapID aID;
    aID.m_aPixelSize        = aBitmap.GetSizePixel();
    aID.m_nSize             = aBitmap.GetBitCount();
    aID.m_nChecksum         = aBitmap.GetBitmap().GetChecksum();
    aID.m_nMaskChecksum     = 0;
    if( aBitmap.IsAlpha() )
        aID.m_nMaskChecksum = aBitmap.GetAlpha().GetChecksum();
    else
    {
        Bitmap aMask = aBitmap.GetMask();
        if( ! aMask.IsEmpty() )
            aID.m_nMaskChecksum = aMask.GetChecksum();
    }
    std::list< BitmapEmit >::const_iterator it = std::find_if(m_aBitmaps.begin(), m_aBitmaps.end(),
                                             [&](const BitmapEmit& arg) { return aID == arg.m_aID; });
    if( it == m_aBitmaps.end() )
    {
        m_aBitmaps.push_front( BitmapEmit() );
        m_aBitmaps.front().m_aID        = aID;
        m_aBitmaps.front().m_aBitmap    = aBitmap;
        if (!rGraphic.hasPdfData() || m_aContext.UseReferenceXObject)
            m_aBitmaps.front().m_nObject = createObject();
        createEmbeddedFile(rGraphic, m_aBitmaps.front().m_aReferenceXObject, m_aBitmaps.front().m_nObject);
        it = m_aBitmaps.begin();
    }

    OStringBuffer aObjName( 16 );
    aObjName.append( "Im" );
    sal_Int32 nObject = it->m_aReferenceXObject.getObject();
    aObjName.append(nObject);
    pushResource( ResXObject, aObjName.makeStringAndClear(), nObject );

    return *it;
}

void PDFWriterImpl::drawBitmap( const Point& rDestPoint, const Size& rDestSize, const Bitmap& rBitmap, const Graphic& rGraphic )
{
    MARK( "drawBitmap (Bitmap)" );

    // #i40055# sanity check
    if( ! (rDestSize.Width() && rDestSize.Height()) )
        return;

    const BitmapEmit& rEmit = createBitmapEmit( BitmapEx( rBitmap ), rGraphic );
    drawBitmap( rDestPoint, rDestSize, rEmit, COL_TRANSPARENT );
}

void PDFWriterImpl::drawBitmap( const Point& rDestPoint, const Size& rDestSize, const BitmapEx& rBitmap )
{
    MARK( "drawBitmap (BitmapEx)" );

    // #i40055# sanity check
    if( ! (rDestSize.Width() && rDestSize.Height()) )
        return;

    const BitmapEmit& rEmit = createBitmapEmit( rBitmap, Graphic() );
    drawBitmap( rDestPoint, rDestSize, rEmit, COL_TRANSPARENT );
}

sal_Int32 PDFWriterImpl::createGradient( const Gradient& rGradient, const Size& rSize )
{
    Size aPtSize( lcl_convert( m_aGraphicsStack.front().m_aMapMode,
                               MapMode( MapUnit::MapPoint ),
                               getReferenceDevice(),
                               rSize ) );
    // check if we already have this gradient
    // rounding to point will generally lose some pixels
    // round up to point boundary
    aPtSize.AdjustWidth( 1 );
    aPtSize.AdjustHeight( 1 );
    std::list< GradientEmit >::const_iterator it = std::find_if(m_aGradients.begin(), m_aGradients.end(),
                                             [&](const GradientEmit& arg) { return ((rGradient == arg.m_aGradient) && (aPtSize == arg.m_aSize) ); });

    if( it == m_aGradients.end() )
    {
        m_aGradients.push_front( GradientEmit() );
        m_aGradients.front().m_aGradient    = rGradient;
        m_aGradients.front().m_nObject      = createObject();
        m_aGradients.front().m_aSize        = aPtSize;
        it = m_aGradients.begin();
    }

    OStringBuffer aObjName( 16 );
    aObjName.append( 'P' );
    aObjName.append( it->m_nObject );
    pushResource( ResShading, aObjName.makeStringAndClear(), it->m_nObject );

    return it->m_nObject;
}

void PDFWriterImpl::drawGradient( const tools::Rectangle& rRect, const Gradient& rGradient )
{
    MARK( "drawGradient (Rectangle)" );

    if( m_aContext.Version == PDFWriter::PDFVersion::PDF_1_2 )
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
    if( m_aGraphicsStack.front().m_aLineColor != COL_TRANSPARENT )
        aLine.append( "q " );
    aLine.append( "0 0 " );
    m_aPages.back().appendMappedLength( static_cast<sal_Int32>(rRect.GetWidth()), aLine, false );
    aLine.append( ' ' );
    m_aPages.back().appendMappedLength( static_cast<sal_Int32>(rRect.GetHeight()), aLine );
    aLine.append( " re W n\n" );

    aLine.append( "/P" );
    aLine.append( nGradient );
    aLine.append( " sh " );
    if( m_aGraphicsStack.front().m_aLineColor != COL_TRANSPARENT )
    {
        aLine.append( "Q 0 0 " );
        m_aPages.back().appendMappedLength( static_cast<sal_Int32>(rRect.GetWidth()), aLine, false );
        aLine.append( ' ' );
        m_aPages.back().appendMappedLength( static_cast<sal_Int32>(rRect.GetHeight()), aLine );
        aLine.append( " re S " );
    }
    aLine.append( "Q\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawHatch( const tools::PolyPolygon& rPolyPoly, const Hatch& rHatch )
{
    MARK( "drawHatch" );

    updateGraphicsState();

    if( rPolyPoly.Count() )
    {
        tools::PolyPolygon     aPolyPoly( rPolyPoly );

        aPolyPoly.Optimize( PolyOptimizeFlags::NO_SAME );
        push( PushFlags::LINECOLOR );
        setLineColor( rHatch.GetColor() );
        getReferenceDevice()->DrawHatch( aPolyPoly, rHatch, false );
        pop();
    }
}

void PDFWriterImpl::drawWallpaper( const tools::Rectangle& rRect, const Wallpaper& rWall )
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
        tools::Rectangle aRect( rRect );
        if( rWall.IsRect() )
        {
            aRect = rWall.GetRect();
            aBmpPos = aRect.TopLeft();
            aBmpSize = aRect.GetSize();
        }
        if( rWall.GetStyle() != WallpaperStyle::Scale )
        {
            if( rWall.GetStyle() != WallpaperStyle::Tile )
            {
                bDrawBitmap     = true;
                if( rWall.IsGradient() )
                    bDrawGradient = true;
                else
                    bDrawColor = true;
                switch( rWall.GetStyle() )
                {
                    case WallpaperStyle::TopLeft:
                        break;
                    case WallpaperStyle::Top:
                        aBmpPos.AdjustX((aRect.GetWidth()-aBmpSize.Width())/2 );
                        break;
                    case WallpaperStyle::Left:
                        aBmpPos.AdjustY((aRect.GetHeight()-aBmpSize.Height())/2 );
                        break;
                    case WallpaperStyle::TopRight:
                        aBmpPos.AdjustX(aRect.GetWidth()-aBmpSize.Width() );
                        break;
                    case WallpaperStyle::Center:
                        aBmpPos.AdjustX((aRect.GetWidth()-aBmpSize.Width())/2 );
                        aBmpPos.AdjustY((aRect.GetHeight()-aBmpSize.Height())/2 );
                        break;
                    case WallpaperStyle::Right:
                        aBmpPos.AdjustX(aRect.GetWidth()-aBmpSize.Width() );
                        aBmpPos.AdjustY((aRect.GetHeight()-aBmpSize.Height())/2 );
                        break;
                    case WallpaperStyle::BottomLeft:
                        aBmpPos.AdjustY(aRect.GetHeight()-aBmpSize.Height() );
                        break;
                    case WallpaperStyle::Bottom:
                        aBmpPos.AdjustX((aRect.GetWidth()-aBmpSize.Width())/2 );
                        aBmpPos.AdjustY(aRect.GetHeight()-aBmpSize.Height() );
                        break;
                    case WallpaperStyle::BottomRight:
                        aBmpPos.AdjustX(aRect.GetWidth()-aBmpSize.Width() );
                        aBmpPos.AdjustY(aRect.GetHeight()-aBmpSize.Height() );
                        break;
                    default: ;
                }
            }
            else
            {
                // push the bitmap
                const BitmapEmit& rEmit = createBitmapEmit( aBitmap, Graphic() );

                // convert to page coordinates; this needs to be done here
                // since the emit does not know the page anymore
                tools::Rectangle aConvertRect( aBmpPos, aBmpSize );
                m_aPages.back().convertRect( aConvertRect );

                OStringBuffer aNameBuf(16);
                aNameBuf.append( "Im" );
                aNameBuf.append( rEmit.m_nObject );
                OString aImageName( aNameBuf.makeStringAndClear() );

                // push the pattern
                OStringBuffer aTilingStream( 32 );
                appendFixedInt( aConvertRect.GetWidth(), aTilingStream );
                aTilingStream.append( " 0 0 " );
                appendFixedInt( aConvertRect.GetHeight(), aTilingStream );
                aTilingStream.append( " 0 0 cm\n/" );
                aTilingStream.append( aImageName );
                aTilingStream.append( " Do\n" );

                m_aTilings.emplace_back( );
                m_aTilings.back().m_nObject         = createObject();
                m_aTilings.back().m_aRectangle      = tools::Rectangle( Point( 0, 0 ), aConvertRect.GetSize() );
                m_aTilings.back().m_pTilingStream   = new SvMemoryStream();
                m_aTilings.back().m_pTilingStream->WriteBytes(
                    aTilingStream.getStr(), aTilingStream.getLength() );
                // phase the tiling so wallpaper begins on upper left
                if ((aConvertRect.GetWidth() == 0) || (aConvertRect.GetHeight() == 0))
                    throw o3tl::divide_by_zero();
                m_aTilings.back().m_aTransform.matrix[2] = double(aConvertRect.Left() % aConvertRect.GetWidth()) / fDivisor;
                m_aTilings.back().m_aTransform.matrix[5] = double(aConvertRect.Top() % aConvertRect.GetHeight()) / fDivisor;
                m_aTilings.back().m_aResources.m_aXObjects[aImageName] = rEmit.m_nObject;

                updateGraphicsState();

                OStringBuffer aObjName( 16 );
                aObjName.append( 'P' );
                aObjName.append( m_aTilings.back().m_nObject );
                OString aPatternName( aObjName.makeStringAndClear() );
                pushResource( ResPattern, aPatternName, m_aTilings.back().m_nObject );

                // fill a rRect with the pattern
                OStringBuffer aLine( 100 );
                aLine.append( "q /Pattern cs /" );
                aLine.append( aPatternName );
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
        setLineColor( COL_TRANSPARENT );
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

void PDFWriterImpl::updateGraphicsState(Mode const mode)
{
    OStringBuffer aLine( 256 );
    GraphicsState& rNewState = m_aGraphicsStack.front();
    // first set clip region since it might invalidate everything else

    if( rNewState.m_nUpdateFlags & GraphicsStateUpdateFlags::ClipRegion )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsStateUpdateFlags::ClipRegion;

        if( m_aCurrentPDFState.m_bClipRegion != rNewState.m_bClipRegion ||
            ( rNewState.m_bClipRegion && m_aCurrentPDFState.m_aClipRegion != rNewState.m_aClipRegion ) )
        {
            if( m_aCurrentPDFState.m_bClipRegion )
            {
                aLine.append( "Q " );
                // invalidate everything but the clip region
                m_aCurrentPDFState = GraphicsState();
                rNewState.m_nUpdateFlags = ~GraphicsStateUpdateFlags::ClipRegion;
            }
            if( rNewState.m_bClipRegion )
            {
                // clip region is always stored in private PDF mapmode
                MapMode aNewMapMode = rNewState.m_aMapMode;
                rNewState.m_aMapMode = m_aMapMode;
                getReferenceDevice()->SetMapMode( rNewState.m_aMapMode );
                m_aCurrentPDFState.m_aMapMode = rNewState.m_aMapMode;

                aLine.append("q ");
                if ( rNewState.m_aClipRegion.count() )
                {
                    m_aPages.back().appendPolyPolygon( rNewState.m_aClipRegion, aLine );
                    aLine.append( "W* n\n" );
                }

                rNewState.m_aMapMode = aNewMapMode;
                getReferenceDevice()->SetMapMode( rNewState.m_aMapMode );
                m_aCurrentPDFState.m_aMapMode = rNewState.m_aMapMode;
            }
        }
    }

    if( rNewState.m_nUpdateFlags & GraphicsStateUpdateFlags::MapMode )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsStateUpdateFlags::MapMode;
        getReferenceDevice()->SetMapMode( rNewState.m_aMapMode );
    }

    if( rNewState.m_nUpdateFlags & GraphicsStateUpdateFlags::Font )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsStateUpdateFlags::Font;
        getReferenceDevice()->SetFont( rNewState.m_aFont );
        getReferenceDevice()->ImplNewFont();
    }

    if( rNewState.m_nUpdateFlags & GraphicsStateUpdateFlags::LayoutMode )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsStateUpdateFlags::LayoutMode;
        getReferenceDevice()->SetLayoutMode( rNewState.m_nLayoutMode );
    }

    if( rNewState.m_nUpdateFlags & GraphicsStateUpdateFlags::DigitLanguage )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsStateUpdateFlags::DigitLanguage;
        getReferenceDevice()->SetDigitLanguage( rNewState.m_aDigitLanguage );
    }

    if( rNewState.m_nUpdateFlags & GraphicsStateUpdateFlags::LineColor )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsStateUpdateFlags::LineColor;
        if( m_aCurrentPDFState.m_aLineColor != rNewState.m_aLineColor &&
            rNewState.m_aLineColor != COL_TRANSPARENT )
        {
            appendStrokingColor( rNewState.m_aLineColor, aLine );
            aLine.append( "\n" );
        }
    }

    if( rNewState.m_nUpdateFlags & GraphicsStateUpdateFlags::FillColor )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsStateUpdateFlags::FillColor;
        if( m_aCurrentPDFState.m_aFillColor != rNewState.m_aFillColor &&
            rNewState.m_aFillColor != COL_TRANSPARENT )
        {
            appendNonStrokingColor( rNewState.m_aFillColor, aLine );
            aLine.append( "\n" );
        }
    }

    if( rNewState.m_nUpdateFlags & GraphicsStateUpdateFlags::TransparentPercent )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsStateUpdateFlags::TransparentPercent;
        if( m_aContext.Version >= PDFWriter::PDFVersion::PDF_1_4 )
        {
            // TODO: switch extended graphicsstate
        }
    }

    // everything is up to date now
    m_aCurrentPDFState = m_aGraphicsStack.front();
    if ((mode != NOWRITE) &&  !aLine.isEmpty())
        writeBuffer( aLine.getStr(), aLine.getLength() );
}

/* #i47544# imitate OutputDevice behaviour:
*  if a font with a nontransparent color is set, it overwrites the current
*  text color. OTOH setting the text color will overwrite the color of the font.
*/
void PDFWriterImpl::setFont( const vcl::Font& rFont )
{
    Color aColor = rFont.GetColor();
    if( aColor == COL_TRANSPARENT )
        aColor = m_aGraphicsStack.front().m_aFont.GetColor();
    m_aGraphicsStack.front().m_aFont = rFont;
    m_aGraphicsStack.front().m_aFont.SetColor( aColor );
    m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsStateUpdateFlags::Font;
}

void PDFWriterImpl::push( PushFlags nFlags )
{
    OSL_ENSURE( !m_aGraphicsStack.empty(), "invalid graphics stack" );
    m_aGraphicsStack.push_front( m_aGraphicsStack.front() );
    m_aGraphicsStack.front().m_nFlags = nFlags;
}

void PDFWriterImpl::pop()
{
    OSL_ENSURE( m_aGraphicsStack.size() > 1, "pop without push" );
    if( m_aGraphicsStack.size() < 2 )
        return;

    GraphicsState aState = m_aGraphicsStack.front();
    m_aGraphicsStack.pop_front();
    GraphicsState& rOld = m_aGraphicsStack.front();

    // move those parameters back that were not pushed
    // in the first place
    if( ! (aState.m_nFlags & PushFlags::LINECOLOR) )
        setLineColor( aState.m_aLineColor );
    if( ! (aState.m_nFlags & PushFlags::FILLCOLOR) )
        setFillColor( aState.m_aFillColor );
    if( ! (aState.m_nFlags & PushFlags::FONT) )
        setFont( aState.m_aFont );
    if( ! (aState.m_nFlags & PushFlags::TEXTCOLOR) )
        setTextColor( aState.m_aFont.GetColor() );
    if( ! (aState.m_nFlags & PushFlags::MAPMODE) )
        setMapMode( aState.m_aMapMode );
    if( ! (aState.m_nFlags & PushFlags::CLIPREGION) )
    {
        // do not use setClipRegion here
        // it would convert again assuming the current mapmode
        rOld.m_aClipRegion = aState.m_aClipRegion;
        rOld.m_bClipRegion = aState.m_bClipRegion;
    }
    if( ! (aState.m_nFlags & PushFlags::TEXTLINECOLOR ) )
        setTextLineColor( aState.m_aTextLineColor );
    if( ! (aState.m_nFlags & PushFlags::OVERLINECOLOR ) )
        setOverlineColor( aState.m_aOverlineColor );
    if( ! (aState.m_nFlags & PushFlags::TEXTALIGN ) )
        setTextAlign( aState.m_aFont.GetAlignment() );
    if( ! (aState.m_nFlags & PushFlags::TEXTFILLCOLOR) )
        setTextFillColor( aState.m_aFont.GetFillColor() );
    if( ! (aState.m_nFlags & PushFlags::REFPOINT) )
    {
        // what ?
    }
    // invalidate graphics state
    m_aGraphicsStack.front().m_nUpdateFlags = GraphicsStateUpdateFlags::All;
}

void PDFWriterImpl::setMapMode( const MapMode& rMapMode )
{
    m_aGraphicsStack.front().m_aMapMode = rMapMode;
    getReferenceDevice()->SetMapMode( rMapMode );
    m_aCurrentPDFState.m_aMapMode = rMapMode;
}

void PDFWriterImpl::setClipRegion( const basegfx::B2DPolyPolygon& rRegion )
{
    basegfx::B2DPolyPolygon aRegion = getReferenceDevice()->LogicToPixel( rRegion, m_aGraphicsStack.front().m_aMapMode );
    aRegion = getReferenceDevice()->PixelToLogic( aRegion, m_aMapMode );
    m_aGraphicsStack.front().m_aClipRegion = aRegion;
    m_aGraphicsStack.front().m_bClipRegion = true;
    m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsStateUpdateFlags::ClipRegion;
}

void PDFWriterImpl::moveClipRegion( sal_Int32 nX, sal_Int32 nY )
{
    if( m_aGraphicsStack.front().m_bClipRegion && m_aGraphicsStack.front().m_aClipRegion.count() )
    {
        Point aPoint( lcl_convert( m_aGraphicsStack.front().m_aMapMode,
                                   m_aMapMode,
                                   getReferenceDevice(),
                                   Point( nX, nY ) ) );
        aPoint -= lcl_convert( m_aGraphicsStack.front().m_aMapMode,
                               m_aMapMode,
                               getReferenceDevice(),
                               Point() );
        basegfx::B2DHomMatrix aMat;
        aMat.translate( aPoint.X(), aPoint.Y() );
        m_aGraphicsStack.front().m_aClipRegion.transform( aMat );
        m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsStateUpdateFlags::ClipRegion;
    }
}

void PDFWriterImpl::intersectClipRegion( const tools::Rectangle& rRect )
{
    basegfx::B2DPolyPolygon aRect( basegfx::utils::createPolygonFromRect(
        basegfx::B2DRectangle( rRect.Left(), rRect.Top(), rRect.Right(), rRect.Bottom() ) ) );
    intersectClipRegion( aRect );
}

void PDFWriterImpl::intersectClipRegion( const basegfx::B2DPolyPolygon& rRegion )
{
    basegfx::B2DPolyPolygon aRegion( getReferenceDevice()->LogicToPixel( rRegion, m_aGraphicsStack.front().m_aMapMode ) );
    aRegion = getReferenceDevice()->PixelToLogic( aRegion, m_aMapMode );
    m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsStateUpdateFlags::ClipRegion;
    if( m_aGraphicsStack.front().m_bClipRegion )
    {
        basegfx::B2DPolyPolygon aOld( basegfx::utils::prepareForPolygonOperation( m_aGraphicsStack.front().m_aClipRegion ) );
        aRegion = basegfx::utils::prepareForPolygonOperation( aRegion );
        m_aGraphicsStack.front().m_aClipRegion = basegfx::utils::solvePolygonOperationAnd( aOld, aRegion );
    }
    else
    {
        m_aGraphicsStack.front().m_aClipRegion = aRegion;
        m_aGraphicsStack.front().m_bClipRegion = true;
    }
}

void PDFWriterImpl::createNote( const tools::Rectangle& rRect, const PDFNote& rNote, sal_Int32 nPageNr )
{
    if( nPageNr < 0 )
        nPageNr = m_nCurrentPage;

    if( nPageNr < 0 || nPageNr >= static_cast<sal_Int32>(m_aPages.size()) )
        return;

    m_aNotes.emplace_back( );
    m_aNotes.back().m_nObject       = createObject();
    m_aNotes.back().m_aContents     = rNote;
    m_aNotes.back().m_aRect         = rRect;
    // convert to default user space now, since the mapmode may change
    m_aPages[nPageNr].convertRect( m_aNotes.back().m_aRect );

    // insert note to page's annotation list
    m_aPages[ nPageNr ].m_aAnnotations.push_back( m_aNotes.back().m_nObject );
}

sal_Int32 PDFWriterImpl::createLink( const tools::Rectangle& rRect, sal_Int32 nPageNr )
{
    if( nPageNr < 0 )
        nPageNr = m_nCurrentPage;

    if( nPageNr < 0 || nPageNr >= static_cast<sal_Int32>(m_aPages.size()) )
        return -1;

    sal_Int32 nRet = m_aLinks.size();

    m_aLinks.emplace_back( );
    m_aLinks.back().m_nObject   = createObject();
    m_aLinks.back().m_nPage     = nPageNr;
    m_aLinks.back().m_aRect     = rRect;
    // convert to default user space now, since the mapmode may change
    m_aPages[nPageNr].convertRect( m_aLinks.back().m_aRect );

    // insert link to page's annotation list
    m_aPages[ nPageNr ].m_aAnnotations.push_back( m_aLinks.back().m_nObject );

    return nRet;
}

sal_Int32 PDFWriterImpl::createScreen(const tools::Rectangle& rRect, sal_Int32 nPageNr)
{
    if (nPageNr < 0)
        nPageNr = m_nCurrentPage;

    if (nPageNr < 0 || nPageNr >= static_cast<sal_Int32>(m_aPages.size()))
        return -1;

    sal_Int32 nRet = m_aScreens.size();

    m_aScreens.emplace_back();
    m_aScreens.back().m_nObject = createObject();
    m_aScreens.back().m_nPage = nPageNr;
    m_aScreens.back().m_aRect = rRect;
    // Convert to default user space now, since the mapmode may change.
    m_aPages[nPageNr].convertRect(m_aScreens.back().m_aRect);

    // Insert link to page's annotation list.
    m_aPages[nPageNr].m_aAnnotations.push_back(m_aScreens.back().m_nObject);

    return nRet;
}

sal_Int32 PDFWriterImpl::createNamedDest( const OUString& sDestName, const tools::Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType )
{
    if( nPageNr < 0 )
        nPageNr = m_nCurrentPage;

    if( nPageNr < 0 || nPageNr >= static_cast<sal_Int32>(m_aPages.size()) )
        return -1;

    sal_Int32 nRet = m_aNamedDests.size();

    m_aNamedDests.emplace_back( );
    m_aNamedDests.back().m_aDestName = sDestName;
    m_aNamedDests.back().m_nPage = nPageNr;
    m_aNamedDests.back().m_eType = eType;
    m_aNamedDests.back().m_aRect = rRect;
    // convert to default user space now, since the mapmode may change
    m_aPages[nPageNr].convertRect( m_aNamedDests.back().m_aRect );

    return nRet;
}

sal_Int32 PDFWriterImpl::createDest( const tools::Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType )
{
    if( nPageNr < 0 )
        nPageNr = m_nCurrentPage;

    if( nPageNr < 0 || nPageNr >= static_cast<sal_Int32>(m_aPages.size()) )
        return -1;

    sal_Int32 nRet = m_aDests.size();

    m_aDests.emplace_back( );
    m_aDests.back().m_nPage = nPageNr;
    m_aDests.back().m_eType = eType;
    m_aDests.back().m_aRect = rRect;
    // convert to default user space now, since the mapmode may change
    m_aPages[nPageNr].convertRect( m_aDests.back().m_aRect );

    return nRet;
}

sal_Int32 PDFWriterImpl::registerDestReference( sal_Int32 nDestId, const tools::Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType )
{
    return m_aDestinationIdTranslation[ nDestId ] = createDest( rRect, nPageNr, eType );
}

void PDFWriterImpl::setLinkDest( sal_Int32 nLinkId, sal_Int32 nDestId )
{
    if( nLinkId < 0 || nLinkId >= static_cast<sal_Int32>(m_aLinks.size()) )
        return;
    if( nDestId < 0 || nDestId >= static_cast<sal_Int32>(m_aDests.size()) )
        return;

    m_aLinks[ nLinkId ].m_nDest = nDestId;
}

void PDFWriterImpl::setLinkURL( sal_Int32 nLinkId, const OUString& rURL )
{
    if( nLinkId < 0 || nLinkId >= static_cast<sal_Int32>(m_aLinks.size()) )
        return;

    m_aLinks[ nLinkId ].m_nDest = -1;

    using namespace ::com::sun::star;

    if (!m_xTrans.is())
    {
        uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
        m_xTrans = util::URLTransformer::create(xContext);
    }

    util::URL aURL;
    aURL.Complete = rURL;

    m_xTrans->parseStrict( aURL );

    m_aLinks[ nLinkId ].m_aURL  = aURL.Complete;
}

void PDFWriterImpl::setScreenURL(sal_Int32 nScreenId, const OUString& rURL)
{
    if (nScreenId < 0 || nScreenId >= static_cast<sal_Int32>(m_aScreens.size()))
        return;

    m_aScreens[nScreenId].m_aURL = rURL;
}

void PDFWriterImpl::setScreenStream(sal_Int32 nScreenId, const OUString& rURL)
{
    if (nScreenId < 0 || nScreenId >= static_cast<sal_Int32>(m_aScreens.size()))
        return;

    m_aScreens[nScreenId].m_aTempFileURL = rURL;
    m_aScreens[nScreenId].m_nTempFileObject = createObject();
}

void PDFWriterImpl::setLinkPropertyId( sal_Int32 nLinkId, sal_Int32 nPropertyId )
{
    m_aLinkPropertyMap[ nPropertyId ] = nLinkId;
}

sal_Int32 PDFWriterImpl::createOutlineItem( sal_Int32 nParent, const OUString& rText, sal_Int32 nDestID )
{
    // create new item
    sal_Int32 nNewItem = m_aOutline.size();
    m_aOutline.emplace_back( );

    // set item attributes
    setOutlineItemParent( nNewItem, nParent );
    setOutlineItemText( nNewItem, rText );
    setOutlineItemDest( nNewItem, nDestID );

    return nNewItem;
}

void PDFWriterImpl::setOutlineItemParent( sal_Int32 nItem, sal_Int32 nNewParent )
{
    if( nItem < 1 || nItem >= static_cast<sal_Int32>(m_aOutline.size()) )
        return;

    if( nNewParent < 0 || nNewParent >= static_cast<sal_Int32>(m_aOutline.size()) || nNewParent == nItem )
    {
        nNewParent = 0;
    }
    // insert item to new parent's list of children
    m_aOutline[ nNewParent ].m_aChildren.push_back( nItem );
}

void PDFWriterImpl::setOutlineItemText( sal_Int32 nItem, const OUString& rText )
{
    if( nItem < 1 || nItem >= static_cast<sal_Int32>(m_aOutline.size()) )
        return;

    m_aOutline[ nItem ].m_aTitle = psp::WhitespaceToSpace( rText );
}

void PDFWriterImpl::setOutlineItemDest( sal_Int32 nItem, sal_Int32 nDestID )
{
    if( nItem < 1 || nItem >= static_cast<sal_Int32>(m_aOutline.size()) ) // item does not exist
        return;
    if( nDestID < 0 || nDestID >= static_cast<sal_Int32>(m_aDests.size()) ) // dest does not exist
        return;
    m_aOutline[nItem].m_nDestID = nDestID;
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
        if( !rEle.m_aAlias.isEmpty() )
            aLine.append( rEle.m_aAlias );
        else
            aLine.append( getStructureTag( rEle.m_eType ) );
        aLine.append( "<</MCID " );
        aLine.append( nMCID );
        aLine.append( ">>BDC\n" );
        writeBuffer( aLine.getStr(), aLine.getLength() );

        // update the element's content list
        SAL_INFO("vcl.pdfwriter", "beginning marked content id " << nMCID << " on page object "
                 << m_aPages[ m_nCurrentPage ].m_nPageObject << ", structure first page = "
                 << rEle.m_nFirstPageObject);
        rEle.m_aKids.emplace_back( nMCID, m_aPages[m_nCurrentPage].m_nPageObject );
        // update the page's mcid parent list
        m_aPages[ m_nCurrentPage ].m_aMCIDParents.push_back( rEle.m_nObject );
        // mark element MC sequence as open
        rEle.m_bOpenMCSeq = true;
    }
    // handle artifacts
    else if( ! m_bEmitStructure && m_aContext.Tagged &&
               m_nCurrentStructElement > 0 &&
               m_aStructure[ m_nCurrentStructElement ].m_eType == PDFWriter::NonStructElement &&
             ! m_aStructure[ m_nCurrentStructElement ].m_bOpenMCSeq // already opened sequence
             )
    {
        OStringBuffer aLine( 128 );
        aLine.append( "/Artifact BMC\n" );
        writeBuffer( aLine.getStr(), aLine.getLength() );
        // mark element MC sequence as open
        m_aStructure[ m_nCurrentStructElement ].m_bOpenMCSeq = true;
    }
}

void PDFWriterImpl::endStructureElementMCSeq()
{
    if( m_nCurrentStructElement > 0 && // StructTreeRoot
        ( m_bEmitStructure || m_aStructure[ m_nCurrentStructElement ].m_eType == PDFWriter::NonStructElement ) &&
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

sal_Int32 PDFWriterImpl::beginStructureElement( PDFWriter::StructElement eType, const OUString& rAlias )
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
                SAL_WARN( "vcl.pdfwriter", "Structure element inserted to StructTreeRoot that is not a document" );
            }
            else {
                OSL_FAIL( "document structure in disorder !" );
            }
        }
        else {
            OSL_FAIL( "PDF document structure MUST be contained in a Document element" );
        }
    }

    sal_Int32 nNewId = sal_Int32(m_aStructure.size());
    m_aStructure.emplace_back( );
    PDFStructureElement& rEle = m_aStructure.back();
    rEle.m_eType            = eType;
    rEle.m_nOwnElement      = nNewId;
    rEle.m_nParentElement   = m_nCurrentStructElement;
    rEle.m_nFirstPageObject = m_aPages[ m_nCurrentPage ].m_nPageObject;
    m_aStructure[ m_nCurrentStructElement ].m_aChildren.push_back( nNewId );
    m_nCurrentStructElement = nNewId;

    // handle alias names
    if( !rAlias.isEmpty() && eType != PDFWriter::NonStructElement )
    {
        OStringBuffer aNameBuf( rAlias.getLength() );
        appendName( rAlias, aNameBuf );
        OString aAliasName( aNameBuf.makeStringAndClear() );
        rEle.m_aAlias = aAliasName;
        m_aRoleMap[ aAliasName ] = getStructureTag( eType );
    }

    if (g_bDebugDisableCompression)
    {
        OStringBuffer aLine( "beginStructureElement " );
        aLine.append( m_nCurrentStructElement );
        aLine.append( ": " );
        aLine.append( getStructureTag( eType ) );
        if( !rEle.m_aAlias.isEmpty() )
        {
            aLine.append( " aliased as \"" );
            aLine.append( rEle.m_aAlias );
            aLine.append( '\"' );
        }
        emitComment( aLine.getStr() );
    }

    // check whether to emit structure henceforth
    m_bEmitStructure = checkEmitStructure();

    if( m_bEmitStructure ) // don't create nonexistent objects
    {
        rEle.m_nObject      = createObject();
        // update parent's kids list
        m_aStructure[ rEle.m_nParentElement ].m_aKids.emplace_back(rEle.m_nObject);
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

    OStringBuffer aLine;
    if (g_bDebugDisableCompression)
    {
        aLine.append( "endStructureElement " );
        aLine.append( m_nCurrentStructElement );
        aLine.append( ": " );
        aLine.append( getStructureTag( m_aStructure[ m_nCurrentStructElement ].m_eType ) );
        if( !m_aStructure[ m_nCurrentStructElement ].m_aAlias.isEmpty() )
        {
            aLine.append( " aliased as \"" );
            aLine.append( m_aStructure[ m_nCurrentStructElement ].m_aAlias );
            aLine.append( '\"' );
        }
    }

    // "end" the structure element, the parent becomes current element
    m_nCurrentStructElement = m_aStructure[ m_nCurrentStructElement ].m_nParentElement;

    // check whether to emit structure henceforth
    m_bEmitStructure = checkEmitStructure();

    if (g_bDebugDisableCompression && m_bEmitStructure)
    {
        emitComment( aLine.getStr() );
    }
}

/*
 * This function adds an internal structure list container to overcome the 8191 elements array limitation
 * in kids element emission.
 * Recursive function
 *
 */
void PDFWriterImpl::addInternalStructureContainer( PDFStructureElement& rEle )
{
    if( rEle.m_eType == PDFWriter::NonStructElement &&
        rEle.m_nOwnElement != rEle.m_nParentElement )
        return;

    for (auto const& child : rEle.m_aChildren)
    {
        if( child > 0 && child < sal_Int32(m_aStructure.size()) )
        {
            PDFStructureElement& rChild = m_aStructure[ child ];
            if( rChild.m_eType != PDFWriter::NonStructElement )
            {
                //triggered when a child of the rEle element is found
                if( rChild.m_nParentElement == rEle.m_nOwnElement )
                    addInternalStructureContainer( rChild );//examine the child
                else
                {
                    OSL_FAIL( "PDFWriterImpl::addInternalStructureContainer: invalid child structure element" );
                    SAL_INFO("vcl.pdfwriter", "PDFWriterImpl::addInternalStructureContainer: invalid child structure element with id " << child );
                }
            }
        }
        else
        {
            OSL_FAIL( "PDFWriterImpl::emitStructure: invalid child structure id" );
            SAL_INFO("vcl.pdfwriter", "PDFWriterImpl::addInternalStructureContainer: invalid child structure id " << child );
        }
    }

    if( rEle.m_nOwnElement != rEle.m_nParentElement )
    {
        if( !rEle.m_aKids.empty() )
        {
            if( rEle.m_aKids.size() > ncMaxPDFArraySize ) {
                //then we need to add the containers for the kids elements
                // a list to be used for the new kid element
                std::list< PDFStructureElementKid > aNewKids;
                std::list< sal_Int32 > aNewChildren;

                // add Div in RoleMap, in case no one else did (TODO: is it needed? Is it dangerous?)
                OString aAliasName( "Div" );
                m_aRoleMap[ aAliasName ] = getStructureTag( PDFWriter::Division );

                while( rEle.m_aKids.size() > ncMaxPDFArraySize )
                {
                    sal_Int32 nCurrentStructElement = rEle.m_nOwnElement;
                    sal_Int32 nNewId = sal_Int32(m_aStructure.size());
                    m_aStructure.emplace_back( );
                    PDFStructureElement& rEleNew = m_aStructure.back();
                    rEleNew.m_aAlias            = aAliasName;
                    rEleNew.m_eType             = PDFWriter::Division; // a new Div type container
                    rEleNew.m_nOwnElement       = nNewId;
                    rEleNew.m_nParentElement    = nCurrentStructElement;
                    //inherit the same page as the first child to be reparented
                    rEleNew.m_nFirstPageObject  = m_aStructure[ rEle.m_aChildren.front() ].m_nFirstPageObject;
                    rEleNew.m_nObject           = createObject();//assign a PDF object number
                    //add the object to the kid list of the parent
                    aNewKids.emplace_back( rEleNew.m_nObject );
                    aNewChildren.push_back( nNewId );

                    std::list< sal_Int32 >::iterator aChildEndIt( rEle.m_aChildren.begin() );
                    std::list< PDFStructureElementKid >::iterator aKidEndIt( rEle.m_aKids.begin() );
                    advance( aChildEndIt, ncMaxPDFArraySize );
                    advance( aKidEndIt, ncMaxPDFArraySize );

                    rEleNew.m_aKids.splice( rEleNew.m_aKids.begin(),
                                            rEle.m_aKids,
                                            rEle.m_aKids.begin(),
                                            aKidEndIt );
                    rEleNew.m_aChildren.splice( rEleNew.m_aChildren.begin(),
                                                rEle.m_aChildren,
                                                rEle.m_aChildren.begin(),
                                                aChildEndIt );
                    // set the kid's new parent
                    for (auto const& child : rEleNew.m_aChildren)
                    {
                        m_aStructure[ child ].m_nParentElement = nNewId;
                    }
                }
                //finally add the new kids resulting from the container added
                rEle.m_aKids.insert( rEle.m_aKids.begin(), aNewKids.begin(), aNewKids.end() );
                rEle.m_aChildren.insert( rEle.m_aChildren.begin(), aNewChildren.begin(), aNewChildren.end() );
            }
        }
    }
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
        if (g_bDebugDisableCompression)
        {
            OStringBuffer aLine( "setCurrentStructureElement " );
            aLine.append( m_nCurrentStructElement );
            aLine.append( ": " );
            aLine.append( getStructureTag( m_aStructure[ m_nCurrentStructElement ].m_eType ) );
            if( !m_aStructure[ m_nCurrentStructElement ].m_aAlias.isEmpty() )
            {
                aLine.append( " aliased as \"" );
                aLine.append( m_aStructure[ m_nCurrentStructElement ].m_aAlias );
                aLine.append( '\"' );
            }
            if( ! m_bEmitStructure )
                aLine.append( " (inside NonStruct)" );
            emitComment( aLine.getStr() );
        }
        bSuccess = true;
    }

    return bSuccess;
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
    else if( m_nCurrentStructElement > 0 && m_bEmitStructure )
        SAL_INFO("vcl.pdfwriter",
                 "rejecting setStructureAttribute( " << getAttributeTag( eAttr )
                 << ", " << getAttributeValueTag( eVal )
                 << " ) on " << getStructureTag( m_aStructure[ m_nCurrentStructElement ].m_eType )
                 << " (" << m_aStructure[ m_nCurrentStructElement ].m_aAlias.getStr()
                 << ") element");

    return bInsert;
}

bool PDFWriterImpl::setStructureAttributeNumerical( enum PDFWriter::StructAttribute eAttr, sal_Int32 nValue )
{
    if( ! m_aContext.Tagged )
        return false;

    bool bInsert = false;
    if( m_nCurrentStructElement > 0 && m_bEmitStructure )
    {
        if( eAttr == PDFWriter::Language )
        {
            m_aStructure[ m_nCurrentStructElement ].m_aLocale = LanguageTag( LanguageType(nValue) ).getLocale();
            return true;
        }

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
    else if( m_nCurrentStructElement > 0 && m_bEmitStructure )
        SAL_INFO("vcl.pdfwriter",
                 "rejecting setStructureAttributeNumerical( " << getAttributeTag( eAttr )
                 << ", " << static_cast<int>(nValue)
                 << " ) on " << getStructureTag( m_aStructure[ m_nCurrentStructElement ].m_eType )
                 << " (" << m_aStructure[ m_nCurrentStructElement ].m_aAlias.getStr()
                 << ") element");

    return bInsert;
}

void PDFWriterImpl::setStructureBoundingBox( const tools::Rectangle& rRect )
{
    sal_Int32 nPageNr = m_nCurrentPage;
    if( nPageNr < 0 || nPageNr >= static_cast<sal_Int32>(m_aPages.size()) || !m_aContext.Tagged )
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

void PDFWriterImpl::setActualText( const OUString& rText )
{
    if( m_aContext.Tagged && m_nCurrentStructElement > 0 && m_bEmitStructure )
    {
        m_aStructure[ m_nCurrentStructElement ].m_aActualText = rText;
    }
}

void PDFWriterImpl::setAlternateText( const OUString& rText )
{
    if( m_aContext.Tagged && m_nCurrentStructElement > 0 && m_bEmitStructure )
    {
        m_aStructure[ m_nCurrentStructElement ].m_aAltText = rText;
    }
}

void PDFWriterImpl::setPageTransition( PDFWriter::PageTransition eType, sal_uInt32 nMilliSec, sal_Int32 nPageNr )
{
    if( nPageNr < 0 )
        nPageNr = m_nCurrentPage;

    if( nPageNr < 0 || nPageNr >= static_cast<sal_Int32>(m_aPages.size()) )
        return;

    m_aPages[ nPageNr ].m_eTransition   = eType;
    m_aPages[ nPageNr ].m_nTransTime    = nMilliSec;
}

void PDFWriterImpl::ensureUniqueRadioOnValues()
{
    // loop over radio groups
    for (auto const& group : m_aRadioGroupWidgets)
    {
        PDFWidget& rGroupWidget = m_aWidgets[ group.second ];
        // check whether all kids have a unique OnValue
        std::unordered_map< OUString, sal_Int32 > aOnValues;
        bool bIsUnique = true;
        for (auto const& nKidIndex : rGroupWidget.m_aKidsIndex)
        {
            const OUString& rVal = m_aWidgets[nKidIndex].m_aOnValue;
            SAL_INFO("vcl.pdfwriter", "OnValue: " << rVal);
            if( aOnValues.find( rVal ) == aOnValues.end() )
            {
                aOnValues[ rVal ] = 1;
            }
            else
            {
                bIsUnique = false;
                break;
            }
        }
        if( ! bIsUnique )
        {
            SAL_INFO("vcl.pdfwriter", "enforcing unique OnValues" );
            // make unique by using ascending OnValues
            int nKid = 0;
            for (auto const& nKidIndex : rGroupWidget.m_aKidsIndex)
            {
                PDFWidget& rKid = m_aWidgets[nKidIndex];
                rKid.m_aOnValue = OUString::number( nKid+1 );
                if( rKid.m_aValue != "Off" )
                    rKid.m_aValue = rKid.m_aOnValue;
                ++nKid;
            }
        }
        // finally move the "Yes" appearance to the OnValue appearance
        for (auto const& nKidIndex : rGroupWidget.m_aKidsIndex)
        {
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
                else
                    SAL_INFO("vcl.pdfwriter", "error: RadioButton without \"Yes\" stream" );
            }
            // update selected radio button
            if( rKid.m_aValue != "Off" )
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
        m_aWidgets.emplace_back( );
        m_aWidgets.back().m_nObject     = createObject();
        m_aWidgets.back().m_nPage       = m_nCurrentPage;
        m_aWidgets.back().m_eType       = PDFWriter::RadioButton;
        m_aWidgets.back().m_nRadioGroup = rBtn.RadioGroup;
        m_aWidgets.back().m_nFlags |= 0x0000C000;   // NoToggleToOff and Radio bits

        createWidgetFieldName( sal_Int32(m_aWidgets.size()-1), rBtn );
    }
    else
        nRadioGroupWidget = it->second;

    return nRadioGroupWidget;
}

sal_Int32 PDFWriterImpl::createControl( const PDFWriter::AnyWidget& rControl, sal_Int32 nPageNr )
{
    if( nPageNr < 0 )
        nPageNr = m_nCurrentPage;

    if( nPageNr < 0 || nPageNr >= static_cast<sal_Int32>(m_aPages.size()) )
        return -1;

    bool sigHidden(true);
    sal_Int32 nNewWidget = m_aWidgets.size();
    m_aWidgets.emplace_back( );

    m_aWidgets.back().m_nObject         = createObject();
    m_aWidgets.back().m_aRect           = rControl.Location;
    m_aWidgets.back().m_nPage           = nPageNr;
    m_aWidgets.back().m_eType           = rControl.getType();

    sal_Int32 nRadioGroupWidget = -1;
    // for unknown reasons the radio buttons of a radio group must not have a
    // field name, else the buttons are in fact check boxes -
    // that is multiple buttons of the radio group can be selected
    if( rControl.getType() == PDFWriter::RadioButton )
        nRadioGroupWidget = findRadioGroupWidget( static_cast<const PDFWriter::RadioButtonWidget&>(rControl) );
    else
    {
        createWidgetFieldName( nNewWidget, rControl );
    }

    // caution: m_aWidgets must not be changed after here or rNewWidget may be invalid
    PDFWidget& rNewWidget           = m_aWidgets[nNewWidget];
    rNewWidget.m_aDescription       = rControl.Description;
    rNewWidget.m_aText              = rControl.Text;
    rNewWidget.m_nTextStyle         = rControl.TextStyle &
        (  DrawTextFlags::Left | DrawTextFlags::Center | DrawTextFlags::Right | DrawTextFlags::Top |
           DrawTextFlags::VCenter | DrawTextFlags::Bottom |
           DrawTextFlags::MultiLine | DrawTextFlags::WordBreak  );
    rNewWidget.m_nTabOrder          = rControl.TabOrder;

    // various properties are set via the flags (/Ff) property of the field dict
    if( rControl.ReadOnly )
        rNewWidget.m_nFlags |= 1;
    if( rControl.getType() == PDFWriter::PushButton )
    {
        const PDFWriter::PushButtonWidget& rBtn = static_cast<const PDFWriter::PushButtonWidget&>(rControl);
        if( rNewWidget.m_nTextStyle == DrawTextFlags::NONE )
            rNewWidget.m_nTextStyle =
                DrawTextFlags::Center | DrawTextFlags::VCenter |
                DrawTextFlags::MultiLine | DrawTextFlags::WordBreak;

        rNewWidget.m_nFlags |= 0x00010000;
        if( !rBtn.URL.isEmpty() )
            rNewWidget.m_aListEntries.push_back( rBtn.URL );
        rNewWidget.m_bSubmit    = rBtn.Submit;
        rNewWidget.m_bSubmitGet = rBtn.SubmitGet;
        rNewWidget.m_nDest      = rBtn.Dest;
        createDefaultPushButtonAppearance( rNewWidget, rBtn );
    }
    else if( rControl.getType() == PDFWriter::RadioButton )
    {
        const PDFWriter::RadioButtonWidget& rBtn = static_cast<const PDFWriter::RadioButtonWidget&>(rControl);
        if( rNewWidget.m_nTextStyle == DrawTextFlags::NONE )
            rNewWidget.m_nTextStyle =
                DrawTextFlags::VCenter | DrawTextFlags::MultiLine | DrawTextFlags::WordBreak;
        /*  PDF sees a RadioButton group as one radio button with
         *  children which are in turn check boxes
         *
         *  so we need to create a radio button on demand for a new group
         *  and insert a checkbox for each RadioButtonWidget as its child
         */
        rNewWidget.m_eType          = PDFWriter::CheckBox;
        rNewWidget.m_nRadioGroup    = rBtn.RadioGroup;

        SAL_WARN_IF( nRadioGroupWidget < 0 || nRadioGroupWidget >= static_cast<sal_Int32>(m_aWidgets.size()), "vcl.pdfwriter", "no radio group parent" );

        PDFWidget& rRadioButton = m_aWidgets[nRadioGroupWidget];
        rRadioButton.m_aKids.push_back( rNewWidget.m_nObject );
        rRadioButton.m_aKidsIndex.push_back( nNewWidget );
        rNewWidget.m_nParent = rRadioButton.m_nObject;

        rNewWidget.m_aValue     = "Off";
        rNewWidget.m_aOnValue   = rBtn.OnValue;
        if( rRadioButton.m_aValue.isEmpty() && rBtn.Selected )
        {
            rNewWidget.m_aValue     = rNewWidget.m_aOnValue;
            rRadioButton.m_aValue   = rNewWidget.m_aOnValue;
        }
        createDefaultRadioButtonAppearance( rNewWidget, rBtn );

        // union rect of radio group
        tools::Rectangle aRect = rNewWidget.m_aRect;
        m_aPages[ nPageNr ].convertRect( aRect );
        rRadioButton.m_aRect.Union( aRect );
    }
    else if( rControl.getType() == PDFWriter::CheckBox )
    {
        const PDFWriter::CheckBoxWidget& rBox = static_cast<const PDFWriter::CheckBoxWidget&>(rControl);
        if( rNewWidget.m_nTextStyle == DrawTextFlags::NONE )
            rNewWidget.m_nTextStyle =
                DrawTextFlags::VCenter | DrawTextFlags::MultiLine | DrawTextFlags::WordBreak;

        rNewWidget.m_aValue = rBox.Checked ? OUString("Yes") : OUString("Off" );
        // create default appearance before m_aRect gets transformed
        createDefaultCheckBoxAppearance( rNewWidget, rBox );
    }
    else if( rControl.getType() == PDFWriter::ListBox )
    {
        if( rNewWidget.m_nTextStyle == DrawTextFlags::NONE )
            rNewWidget.m_nTextStyle = DrawTextFlags::VCenter;

        const PDFWriter::ListBoxWidget& rLstBox = static_cast<const PDFWriter::ListBoxWidget&>(rControl);
        rNewWidget.m_aListEntries     = rLstBox.Entries;
        rNewWidget.m_aSelectedEntries = rLstBox.SelectedEntries;
        rNewWidget.m_aValue           = rLstBox.Text;
        if( rLstBox.DropDown )
            rNewWidget.m_nFlags |= 0x00020000;
        if( rLstBox.MultiSelect && !rLstBox.DropDown && m_aContext.Version > PDFWriter::PDFVersion::PDF_1_3 )
            rNewWidget.m_nFlags |= 0x00200000;

        createDefaultListBoxAppearance( rNewWidget, rLstBox );
    }
    else if( rControl.getType() == PDFWriter::ComboBox )
    {
        if( rNewWidget.m_nTextStyle == DrawTextFlags::NONE )
            rNewWidget.m_nTextStyle = DrawTextFlags::VCenter;

        const PDFWriter::ComboBoxWidget& rBox = static_cast<const PDFWriter::ComboBoxWidget&>(rControl);
        rNewWidget.m_aValue         = rBox.Text;
        rNewWidget.m_aListEntries   = rBox.Entries;
        rNewWidget.m_nFlags |= 0x00060000; // combo and edit flag

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
        aLBox.MultiSelect       = false;
        aLBox.Entries           = rBox.Entries;

        createDefaultListBoxAppearance( rNewWidget, aLBox );
    }
    else if( rControl.getType() == PDFWriter::Edit )
    {
        if( rNewWidget.m_nTextStyle == DrawTextFlags::NONE )
            rNewWidget.m_nTextStyle = DrawTextFlags::Left | DrawTextFlags::VCenter;

        const PDFWriter::EditWidget& rEdit = static_cast<const  PDFWriter::EditWidget&>(rControl);
        if( rEdit.MultiLine )
        {
            rNewWidget.m_nFlags |= 0x00001000;
            rNewWidget.m_nTextStyle |= DrawTextFlags::MultiLine | DrawTextFlags::WordBreak;
        }
        if( rEdit.Password )
            rNewWidget.m_nFlags |= 0x00002000;
        if( rEdit.FileSelect && m_aContext.Version > PDFWriter::PDFVersion::PDF_1_3 )
            rNewWidget.m_nFlags |= 0x00100000;
        rNewWidget.m_nMaxLen = rEdit.MaxLen;
        rNewWidget.m_aValue = rEdit.Text;

        createDefaultEditAppearance( rNewWidget, rEdit );
    }
#if HAVE_FEATURE_NSS
    else if( rControl.getType() == PDFWriter::Signature)
    {
        sigHidden = true;

        rNewWidget.m_aRect = tools::Rectangle(0, 0, 0, 0);

        m_nSignatureObject = createObject();
        rNewWidget.m_aValue = OUString::number( m_nSignatureObject );
        rNewWidget.m_aValue += " 0 R";
        // let's add a fake appearance
        rNewWidget.m_aAppearances[ "N" ][ "Standard" ] = new SvMemoryStream();
    }
#endif

    // if control is a hidden signature, do not convert coordinates since we
    // need /Rect [ 0 0 0 0 ]
    if ( ! ( ( rControl.getType() == PDFWriter::Signature ) && sigHidden ) )
    {
        // convert to default user space now, since the mapmode may change
        // note: create default appearances before m_aRect gets transformed
        m_aPages[ nPageNr ].convertRect( rNewWidget.m_aRect );
    }

    // insert widget to page's annotation list
    m_aPages[ nPageNr ].m_aAnnotations.push_back( rNewWidget.m_nObject );

    return nNewWidget;
}

void PDFWriterImpl::addStream( const OUString& rMimeType, PDFOutputStream* pStream )
{
    if( pStream )
    {
        m_aAdditionalStreams.emplace_back( );
        PDFAddStream& rStream = m_aAdditionalStreams.back();
        rStream.m_aMimeType = !rMimeType.isEmpty()
                              ? rMimeType
                              : OUString( "application/octet-stream"  );
        rStream.m_pStream = pStream;
        rStream.m_bCompress = false;
    }
}

void PDFWriterImpl::MARK( const char* pString )
{
    beginStructureElementMCSeq();
    if (g_bDebugDisableCompression)
        emitComment( pString );
}

sal_Int32 PDFWriterImpl::ReferenceXObjectEmit::getObject() const
{
    if (m_nFormObject > 0)
        return m_nFormObject;
    else
        return m_nBitmapObject;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
