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

#include <zlib.h>

#include "outputwrap.hxx"
#include "contentsink.hxx"
#include "pdfihelper.hxx"
#include "wrapper.hxx"
#include "pdfparse.hxx"
#include "../pdfiadaptor.hxx"

#include <rtl/math.hxx>
#include <osl/file.hxx>
#include <comphelper/sequence.hxx>

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include <test/bootstrapfixture.hxx>

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XColorSpace.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/BlendMode.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>

#include <unordered_map>
#include <vector>

#include <cassert>
#include <rtl/ustring.hxx>

using namespace ::pdfparse;
using namespace ::pdfi;
using namespace ::com::sun::star;

namespace
{

    class TestSink : public ContentSink
    {
    public:
        TestSink() :
            m_nNextFontId( 1 ),
            m_aIdToFont(),
            m_aFontToId(),
            m_aGCStack(1),
            m_aPageSize(),
            m_aHyperlinkBounds(),
            m_aURI(),
            m_aTextOut(),
            m_nNumPages(0),
            m_bPageEnded(false),
            m_bRedCircleSeen(false),
            m_bGreenStrokeSeen(false),
            m_bDashedLineSeen(false),
            m_bImageSeen(false)
        {}

        virtual ~TestSink() {}

        void check()
        {
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "A4 page size (in 100th of points): Width", 79400, m_aPageSize.Width, 0.00000001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "A4 page size (in 100th of points): Height", 59500, m_aPageSize.Height, 0.0000001 );
            CPPUNIT_ASSERT_MESSAGE( "endPage() called", m_bPageEnded );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Num pages equal one", (sal_Int32) 1, m_nNumPages );
            CPPUNIT_ASSERT_MESSAGE( "Correct hyperlink bounding box",
                                    rtl::math::approxEqual(m_aHyperlinkBounds.X1,34.7 ) &&
                                    rtl::math::approxEqual(m_aHyperlinkBounds.Y1,386.0) &&
                                    rtl::math::approxEqual(m_aHyperlinkBounds.X2,166.7) &&
                                    rtl::math::approxEqual(m_aHyperlinkBounds.Y2,406.2) );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Correct hyperlink URI", OUString("http://download.openoffice.org/"), m_aURI );

            const char* sText = " \n \nThis is a testtext\nNew paragraph,\nnew line\n"
                "Hyperlink, this is\n?\nThis is more text\noutline mode\n?\nNew paragraph\n";
            OString aTmp;
            m_aTextOut.makeStringAndClear().convertToString( &aTmp,
                                                             RTL_TEXTENCODING_ASCII_US,
                                                             OUSTRING_TO_OSTRING_CVTFLAGS );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Imported text is \"This is a testtext New paragraph, new line"
                                    " Hyperlink, this is * This is more text outline mode * New paragraph\"",
                                    aTmp, OString(sText) );

            CPPUNIT_ASSERT_MESSAGE( "red circle seen in input", m_bRedCircleSeen );
            CPPUNIT_ASSERT_MESSAGE( "green stroke seen in input", m_bGreenStrokeSeen );
            CPPUNIT_ASSERT_MESSAGE( "dashed line seen in input", m_bDashedLineSeen );
            CPPUNIT_ASSERT_MESSAGE( "image seen in input", m_bImageSeen );
        }

    private:
        GraphicsContext& getCurrentContext() { return m_aGCStack.back(); }

        // ContentSink interface implementation
        virtual void setPageNum( sal_Int32 nNumPages ) SAL_OVERRIDE
        {
            m_nNumPages = nNumPages;
        }

        virtual void startPage( const geometry::RealSize2D& rSize ) SAL_OVERRIDE
        {
            m_aPageSize = rSize;
        }

        virtual void endPage() SAL_OVERRIDE
        {
            m_bPageEnded = true;
        }

        virtual void hyperLink( const geometry::RealRectangle2D& rBounds,
                                const OUString&             rURI ) SAL_OVERRIDE
        {
            m_aHyperlinkBounds = rBounds;
            m_aURI = rURI;
        }

        virtual void pushState() SAL_OVERRIDE
        {
            GraphicsContextStack::value_type const a(m_aGCStack.back());
            m_aGCStack.push_back(a);
        }

        virtual void popState() SAL_OVERRIDE
        {
            m_aGCStack.pop_back();
        }

        virtual void setTransformation( const geometry::AffineMatrix2D& rMatrix ) SAL_OVERRIDE
        {
            basegfx::unotools::homMatrixFromAffineMatrix(
                getCurrentContext().Transformation,
                rMatrix );
        }

        virtual void setLineDash( const uno::Sequence<double>& dashes,
                                  double                       start ) SAL_OVERRIDE
        {
            GraphicsContext& rContext( getCurrentContext() );
            if( dashes.getLength() )
                comphelper::sequenceToContainer(rContext.DashArray,dashes);
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "line dashing start offset", 0.0, start, 0.000000001 );
        }

        virtual void setFlatness( double nFlatness ) SAL_OVERRIDE
        {
            getCurrentContext().Flatness = nFlatness;
        }

        virtual void setLineJoin(sal_Int8 nJoin) SAL_OVERRIDE
        {
            getCurrentContext().LineJoin = nJoin;
        }

        virtual void setLineCap(sal_Int8 nCap) SAL_OVERRIDE
        {
            getCurrentContext().LineCap = nCap;
        }

        virtual void setMiterLimit(double nVal) SAL_OVERRIDE
        {
            getCurrentContext().MiterLimit = nVal;
        }

        virtual void setLineWidth(double nVal) SAL_OVERRIDE
        {
            getCurrentContext().LineWidth = nVal;
        }

        virtual void setFillColor( const rendering::ARGBColor& rColor ) SAL_OVERRIDE
        {
            getCurrentContext().FillColor = rColor;
        }

        virtual void setStrokeColor( const rendering::ARGBColor& rColor ) SAL_OVERRIDE
        {
            getCurrentContext().LineColor = rColor;
        }

        virtual void setFont( const FontAttributes& rFont ) SAL_OVERRIDE
        {
            FontToIdMap::const_iterator it = m_aFontToId.find( rFont );
            if( it != m_aFontToId.end() )
                getCurrentContext().FontId = it->second;
            else
            {
                m_aFontToId[ rFont ] = m_nNextFontId;
                m_aIdToFont[ m_nNextFontId ] = rFont;
                getCurrentContext().FontId = m_nNextFontId;
                m_nNextFontId++;
            }
        }

        virtual void strokePath( const uno::Reference<rendering::XPolyPolygon2D>& rPath ) SAL_OVERRIDE
        {
            GraphicsContext& rContext( getCurrentContext() );
            basegfx::B2DPolyPolygon aPath = basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rPath);
            aPath.transform( rContext.Transformation );

            if( rContext.DashArray.empty() )
            {
                CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is green", 1.0, rContext.LineColor.Alpha, 0.00000001);
                CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is green", 0.0, rContext.LineColor.Blue, 0.00000001);
                CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is green", 1.0, rContext.LineColor.Green, 0.00000001);
                CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is green", 0.0, rContext.LineColor.Red, 0.00000001);

                CPPUNIT_ASSERT_MESSAGE( "Line width is 0",
                                        rtl::math::approxEqual(rContext.LineWidth, 28.3) );

                const char* sExportString = "m53570 7650-35430 24100";
                CPPUNIT_ASSERT_MESSAGE( "Stroke is m535.7 518.5-354.3-241",
                                        basegfx::tools::exportToSvgD( aPath, true, true, false ).equalsAscii(sExportString) );

                m_bGreenStrokeSeen = true;
            }
            else
            {
                CPPUNIT_ASSERT_MESSAGE( "Dash array consists of four entries",
                                        rContext.DashArray.size() == 4 &&
                                        rtl::math::approxEqual(rContext.DashArray[0],14.3764) &&
                                        rContext.DashArray[0] == rContext.DashArray[1] &&
                                        rContext.DashArray[1] == rContext.DashArray[2] &&
                                        rContext.DashArray[2] == rContext.DashArray[3] );

                CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 1.0, rContext.LineColor.Alpha, 0.00000001);
                CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 0.0, rContext.LineColor.Blue, 0.00000001);
                CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 0.0, rContext.LineColor.Green, 0.00000001);
                CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 0.0, rContext.LineColor.Red, 0.00000001);

                CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line width is 0",
                                        0, rContext.LineWidth, 0.0000001 );

                const char* sExportString = "m49890 5670.00000000001-35430 24090";
                CPPUNIT_ASSERT_MESSAGE( "Stroke is m49890 5670.00000000001-35430 24090",
                                        basegfx::tools::exportToSvgD( aPath, true, true, false ).equalsAscii(sExportString) );

                m_bDashedLineSeen = true;
            }
            CPPUNIT_ASSERT_MESSAGE( "Blend mode is normal",
                                    rContext.BlendMode == rendering::BlendMode::NORMAL );
            CPPUNIT_ASSERT_MESSAGE( "Join type is round",
                                    rContext.LineJoin == rendering::PathJoinType::ROUND );
            CPPUNIT_ASSERT_MESSAGE( "Cap type is butt",
                                    rContext.LineCap == rendering::PathCapType::BUTT );
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line miter limit is 10",
                                    10, rContext.MiterLimit, 0.0000001 );
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Flatness is 0",
                                    1, rContext.Flatness, 0.00000001 );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Font id is 0",
                                    (sal_Int32) 0, rContext.FontId );
        }

        virtual void fillPath( const uno::Reference<rendering::XPolyPolygon2D>& rPath ) SAL_OVERRIDE
        {
            GraphicsContext& rContext( getCurrentContext() );
            basegfx::B2DPolyPolygon aPath = basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rPath);
            aPath.transform( rContext.Transformation );

            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 1.0, rContext.LineColor.Alpha, 0.00000001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 0.0, rContext.LineColor.Blue, 0.00000001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 0.0, rContext.LineColor.Green, 0.00000001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 0.0, rContext.LineColor.Red, 0.00000001);

            CPPUNIT_ASSERT_MESSAGE( "Blend mode is normal",
                                    rContext.BlendMode == rendering::BlendMode::NORMAL );
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Flatness is 10",
                                    10, rContext.Flatness, 0.00000001 );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Font id is 0",
                                    (sal_Int32) 0, rContext.FontId );
        }

        virtual void eoFillPath( const uno::Reference<rendering::XPolyPolygon2D>& rPath ) SAL_OVERRIDE
        {
            GraphicsContext& rContext( getCurrentContext() );
            basegfx::B2DPolyPolygon aPath = basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rPath);
            aPath.transform( rContext.Transformation );

            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 1.0, rContext.LineColor.Alpha, 0.00000001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 0.0, rContext.LineColor.Blue, 0.00000001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 0.0, rContext.LineColor.Green, 0.00000001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 0.0, rContext.LineColor.Red, 0.00000001);

            CPPUNIT_ASSERT_MESSAGE( "Blend mode is normal",
                                    rContext.BlendMode == rendering::BlendMode::NORMAL );
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Flatness is 0",
                                    1, rContext.Flatness, 0.00000001 );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Font id is 0",
                                    (sal_Int32) 0, rContext.FontId );

            const char* sExportString = "m12050 49610c-4310 0-7800-3490-7800-7800 0-4300 "
                "3490-7790 7800-7790 4300 0 7790 3490 7790 7790 0 4310-3490 7800-7790 7800z";
            CPPUNIT_ASSERT_MESSAGE( "Stroke is a 4-bezier circle",
                                    basegfx::tools::exportToSvgD( aPath, true, true, false ).equalsAscii(sExportString) );

            m_bRedCircleSeen = true;
        }

        virtual void intersectClip(const uno::Reference<rendering::XPolyPolygon2D>& rPath) SAL_OVERRIDE
        {
            basegfx::B2DPolyPolygon aNewClip = basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rPath);
            basegfx::B2DPolyPolygon aCurClip = getCurrentContext().Clip;

            if( aCurClip.count() )  // #i92985# adapted API from (..., false, false) to (..., true, false)
                aNewClip = basegfx::tools::clipPolyPolygonOnPolyPolygon( aCurClip, aNewClip, true, false );

            getCurrentContext().Clip = aNewClip;
        }

        virtual void intersectEoClip(const uno::Reference<rendering::XPolyPolygon2D>& rPath) SAL_OVERRIDE
        {
            basegfx::B2DPolyPolygon aNewClip = basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rPath);
            basegfx::B2DPolyPolygon aCurClip = getCurrentContext().Clip;

            if( aCurClip.count() )  // #i92985# adapted API from (..., false, false) to (..., true, false)
                aNewClip = basegfx::tools::clipPolyPolygonOnPolyPolygon( aCurClip, aNewClip, true, false );

            getCurrentContext().Clip = aNewClip;
        }

        virtual void drawGlyphs( const OUString&             rGlyphs,
                                 const geometry::RealRectangle2D& /*rRect*/,
                                 const geometry::Matrix2D&        /*rFontMatrix*/,
                                 double /*fontSize*/) SAL_OVERRIDE
        {
            m_aTextOut.append(rGlyphs);
        }

        virtual void endText() SAL_OVERRIDE
        {
            m_aTextOut.append( "\n" );
        }

        virtual void drawMask(const uno::Sequence<beans::PropertyValue>& xBitmap,
                              bool                                       /*bInvert*/ ) SAL_OVERRIDE
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawMask received two properties",
                                    (sal_Int32) 3, xBitmap.getLength() );
            CPPUNIT_ASSERT_MESSAGE( "drawMask got URL param",
                                    xBitmap[0].Name == "URL" );
            CPPUNIT_ASSERT_MESSAGE( "drawMask got InputStream param",
                                    xBitmap[1].Name == "InputStream" );
        }

        virtual void drawImage(const uno::Sequence<beans::PropertyValue>& xBitmap ) SAL_OVERRIDE
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawImage received two properties",
                                    (sal_Int32) 3, xBitmap.getLength() );
            CPPUNIT_ASSERT_MESSAGE( "drawImage got URL param",
                                    xBitmap[0].Name == "URL" );
            CPPUNIT_ASSERT_MESSAGE( "drawImage got InputStream param",
                                    xBitmap[1].Name == "InputStream" );
            m_bImageSeen = true;
        }

        virtual void drawColorMaskedImage(const uno::Sequence<beans::PropertyValue>& xBitmap,
                                          const uno::Sequence<uno::Any>&             /*xMaskColors*/ ) SAL_OVERRIDE
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawColorMaskedImage received two properties",
                                    (sal_Int32) 3, xBitmap.getLength() );
            CPPUNIT_ASSERT_MESSAGE( "drawColorMaskedImage got URL param",
                                    xBitmap[0].Name == "URL" );
            CPPUNIT_ASSERT_MESSAGE( "drawColorMaskedImage got InputStream param",
                                    xBitmap[1].Name == "InputStream" );
        }

        virtual void drawMaskedImage(const uno::Sequence<beans::PropertyValue>& xBitmap,
                                     const uno::Sequence<beans::PropertyValue>& xMask,
                                     bool                                       /*bInvertMask*/) SAL_OVERRIDE
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawMaskedImage received two properties #1",
                                    (sal_Int32) 3, xBitmap.getLength() );
            CPPUNIT_ASSERT_MESSAGE( "drawMaskedImage got URL param #1",
                                    xBitmap[0].Name == "URL" );
            CPPUNIT_ASSERT_MESSAGE( "drawMaskedImage got InputStream param #1",
                                    xBitmap[1].Name == "InputStream" );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawMaskedImage received two properties #2",
                                    (sal_Int32) 3, xMask.getLength() );
            CPPUNIT_ASSERT_MESSAGE( "drawMaskedImage got URL param #2",
                                    xMask[0].Name == "URL" );
            CPPUNIT_ASSERT_MESSAGE( "drawMaskedImage got InputStream param #2",
                                    xMask[1].Name == "InputStream" );
        }

        virtual void drawAlphaMaskedImage(const uno::Sequence<beans::PropertyValue>& xBitmap,
                                          const uno::Sequence<beans::PropertyValue>& xMask) SAL_OVERRIDE
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawAlphaMaskedImage received two properties #1",
                                    (sal_Int32) 3, xBitmap.getLength() );
            CPPUNIT_ASSERT_MESSAGE( "drawAlphaMaskedImage got URL param #1",
                                    xBitmap[0].Name == "URL" );
            CPPUNIT_ASSERT_MESSAGE( "drawAlphaMaskedImage got InputStream param #1",
                                    xBitmap[1].Name == "InputStream" );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawAlphaMaskedImage received two properties #2",
                                    (sal_Int32) 3, xMask.getLength() );
            CPPUNIT_ASSERT_MESSAGE( "drawAlphaMaskedImage got URL param #2",
                                    xMask[0].Name == "URL" );
            CPPUNIT_ASSERT_MESSAGE( "drawAlphaMaskedImage got InputStream param #2",
                                    xMask[1].Name == "InputStream" );
        }

        virtual void setTextRenderMode( sal_Int32 ) SAL_OVERRIDE
        {
        }

        typedef std::unordered_map<sal_Int32,FontAttributes> IdToFontMap;
        typedef std::unordered_map<FontAttributes,sal_Int32,FontAttrHash> FontToIdMap;

        typedef std::vector<GraphicsContext> GraphicsContextStack;

        sal_Int32                 m_nNextFontId;
        IdToFontMap               m_aIdToFont;
        FontToIdMap               m_aFontToId;

        GraphicsContextStack      m_aGCStack;
        geometry::RealSize2D      m_aPageSize;
        geometry::RealRectangle2D m_aHyperlinkBounds;
        OUString           m_aURI;
        OUStringBuffer     m_aTextOut;
        sal_Int32                 m_nNumPages;
        bool                      m_bPageEnded;
        bool                      m_bRedCircleSeen;
        bool                      m_bGreenStrokeSeen;
        bool                      m_bDashedLineSeen;
        bool                      m_bImageSeen;
    };

    class PDFITest : public test::BootstrapFixture
    {
    public:
        void testXPDFParser()
        {
            std::shared_ptr<TestSink> pSink( new TestSink() );
            CPPUNIT_ASSERT(
                pdfi::xpdf_ImportFromFile(
                    getURLFromSrc("/sdext/source/pdfimport/test/testinput.pdf"),
                    pSink,
                    uno::Reference< task::XInteractionHandler >(),
                    OUString(),
                    getComponentContext() ) );
            pSink->check();
        }

        void testOdfDrawExport()
        {
            uno::Reference<pdfi::PDFIRawAdaptor> xAdaptor( new pdfi::PDFIRawAdaptor(OUString(), getComponentContext()) );
            xAdaptor->setTreeVisitorFactory( createDrawTreeVisitorFactory() );

            OUString tempFileURL;
            CPPUNIT_ASSERT( osl::File::createTempFile( NULL, NULL, &tempFileURL ) == osl::File::E_None );
            osl::File::remove( tempFileURL ); // FIXME the below apparently fails silently if the file already exists
            CPPUNIT_ASSERT_MESSAGE("Exporting to ODF",
                                   xAdaptor->odfConvert( getURLFromSrc("/sdext/source/pdfimport/test/testinput.pdf"),
                                                        new OutputWrap(tempFileURL),
                                                        NULL ));
            osl::File::remove( tempFileURL );
        }

        void testOdfWriterExport()
        {
            uno::Reference<pdfi::PDFIRawAdaptor> xAdaptor( new pdfi::PDFIRawAdaptor(OUString(), getComponentContext()) );
            xAdaptor->setTreeVisitorFactory( createWriterTreeVisitorFactory() );

            OUString tempFileURL;
            CPPUNIT_ASSERT( osl::File::createTempFile( NULL, NULL, &tempFileURL ) == osl::File::E_None );
            osl::File::remove( tempFileURL ); // FIXME the below apparently fails silently if the file already exists
            CPPUNIT_ASSERT_MESSAGE("Exporting to ODF",
                                   xAdaptor->odfConvert( getURLFromSrc("/sdext/source/pdfimport/test/testinput.pdf"),
                                                        new OutputWrap(tempFileURL),
                                                        NULL ));
            osl::File::remove( tempFileURL );
        }

        CPPUNIT_TEST_SUITE(PDFITest);
        CPPUNIT_TEST(testXPDFParser);
        CPPUNIT_TEST(testOdfWriterExport);
        CPPUNIT_TEST(testOdfDrawExport);
        CPPUNIT_TEST_SUITE_END();
    };

}

CPPUNIT_TEST_SUITE_REGISTRATION(PDFITest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
