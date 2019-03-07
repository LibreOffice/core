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
#include <zlib.h>

#include "outputwrap.hxx"
#include <contentsink.hxx>
#include <pdfihelper.hxx>
#include <wrapper.hxx>
#include <pdfparse.hxx>
#include "../pdfiadaptor.hxx"

#include <rtl/math.hxx>
#include <osl/file.hxx>
#include <comphelper/sequence.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <test/bootstrapfixture.hxx>

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XColorSpace.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/BlendMode.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>

#include <unordered_map>
#include <vector>

#include <cassert>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>

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

        void check()
        {
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "A4 page size (in 100th of points): Width", 79400, m_aPageSize.Width, 0.00000001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "A4 page size (in 100th of points): Height", 59500, m_aPageSize.Height, 0.0000001 );
            CPPUNIT_ASSERT_MESSAGE( "endPage() called", m_bPageEnded );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Num pages equal one", sal_Int32(1), m_nNumPages );
            CPPUNIT_ASSERT_MESSAGE( "Correct hyperlink bounding box",
                                    rtl::math::approxEqual(m_aHyperlinkBounds.X1,34.7 ) &&
                                    rtl::math::approxEqual(m_aHyperlinkBounds.Y1,386.0) &&
                                    rtl::math::approxEqual(m_aHyperlinkBounds.X2,166.7) &&
                                    rtl::math::approxEqual(m_aHyperlinkBounds.Y2,406.2) );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Correct hyperlink URI", OUString("http://download.openoffice.org/"), m_aURI );

            const char* const sText = " \n \nThis is a testtext\nNew paragraph,\nnew line\n"
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
        virtual void setPageNum( sal_Int32 nNumPages ) override
        {
            m_nNumPages = nNumPages;
        }

        virtual void startPage( const geometry::RealSize2D& rSize ) override
        {
            m_aPageSize = rSize;
        }

        virtual void endPage() override
        {
            m_bPageEnded = true;
        }

        virtual void hyperLink( const geometry::RealRectangle2D& rBounds,
                                const OUString&             rURI ) override
        {
            m_aHyperlinkBounds = rBounds;
            m_aURI = rURI;
        }

        virtual void pushState() override
        {
            GraphicsContextStack::value_type const a(m_aGCStack.back());
            m_aGCStack.push_back(a);
        }

        virtual void popState() override
        {
            m_aGCStack.pop_back();
        }

        virtual void setTransformation( const geometry::AffineMatrix2D& rMatrix ) override
        {
            basegfx::unotools::homMatrixFromAffineMatrix(
                getCurrentContext().Transformation,
                rMatrix );
        }

        virtual void setLineDash( const uno::Sequence<double>& dashes,
                                  double                       start ) override
        {
            GraphicsContext& rContext( getCurrentContext() );
            if( dashes.getLength() )
                comphelper::sequenceToContainer(rContext.DashArray,dashes);
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "line dashing start offset", 0.0, start, 0.000000001 );
        }

        virtual void setFlatness( double nFlatness ) override
        {
            getCurrentContext().Flatness = nFlatness;
        }

        virtual void setLineJoin(sal_Int8 nJoin) override
        {
            getCurrentContext().LineJoin = nJoin;
        }

        virtual void setLineCap(sal_Int8 nCap) override
        {
            getCurrentContext().LineCap = nCap;
        }

        virtual void setMiterLimit(double nVal) override
        {
            getCurrentContext().MiterLimit = nVal;
        }

        virtual void setLineWidth(double nVal) override
        {
            getCurrentContext().LineWidth = nVal;
        }

        virtual void setFillColor( const rendering::ARGBColor& rColor ) override
        {
            getCurrentContext().FillColor = rColor;
        }

        virtual void setStrokeColor( const rendering::ARGBColor& rColor ) override
        {
            getCurrentContext().LineColor = rColor;
        }

        virtual void setFont( const FontAttributes& rFont ) override
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

        virtual void strokePath( const uno::Reference<rendering::XPolyPolygon2D>& rPath ) override
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

                const char sExportString[] = "m53570 7650-35430 24100";
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Stroke is m535.7 518.5-354.3-241",
                                        OUString(sExportString), basegfx::utils::exportToSvgD( aPath, true, true, false ) );

                m_bGreenStrokeSeen = true;
            }
            else
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Dash array consists of four entries", std::vector<double>::size_type(4), rContext.DashArray.size());
                CPPUNIT_ASSERT_DOUBLES_EQUAL( 14.3764, rContext.DashArray[0], 1E-12 );
                CPPUNIT_ASSERT_DOUBLES_EQUAL( rContext.DashArray[0], rContext.DashArray[1], 1E-12 );
                CPPUNIT_ASSERT_DOUBLES_EQUAL( rContext.DashArray[1], rContext.DashArray[2], 1E-12 );
                CPPUNIT_ASSERT_DOUBLES_EQUAL( rContext.DashArray[2], rContext.DashArray[3], 1E-12 );

                CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 1.0, rContext.LineColor.Alpha, 0.00000001);
                CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 0.0, rContext.LineColor.Blue, 0.00000001);
                CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 0.0, rContext.LineColor.Green, 0.00000001);
                CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 0.0, rContext.LineColor.Red, 0.00000001);

                CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line width is 0",
                                        0, rContext.LineWidth, 0.0000001 );

                const char sExportString[] = "m49890 5670.00000000001-35430 24090";
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Stroke is m49890 5670.00000000001-35430 24090",
                                        OUString(sExportString), basegfx::utils::exportToSvgD( aPath, true, true, false ) );

                m_bDashedLineSeen = true;
            }
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Blend mode is normal",
                                    rendering::BlendMode::NORMAL, rContext.BlendMode );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Join type is round",
                                    rendering::PathJoinType::ROUND, rContext.LineJoin );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Cap type is butt",
                                    rendering::PathCapType::BUTT, rContext.LineCap );
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line miter limit is 10",
                                    10, rContext.MiterLimit, 0.0000001 );
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Flatness is 0",
                                    1, rContext.Flatness, 0.00000001 );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Font id is 0",
                                    sal_Int32(0), rContext.FontId );
        }

        virtual void fillPath( const uno::Reference<rendering::XPolyPolygon2D>& rPath ) override
        {
            GraphicsContext& rContext( getCurrentContext() );
            basegfx::B2DPolyPolygon aPath = basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rPath);
            aPath.transform( rContext.Transformation );

            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 1.0, rContext.LineColor.Alpha, 0.00000001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 0.0, rContext.LineColor.Blue, 0.00000001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 0.0, rContext.LineColor.Green, 0.00000001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 0.0, rContext.LineColor.Red, 0.00000001);

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Blend mode is normal",
                                    rendering::BlendMode::NORMAL, rContext.BlendMode );
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Flatness is 10",
                                    10, rContext.Flatness, 0.00000001 );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Font id is 0",
                                    sal_Int32(0), rContext.FontId );
        }

        virtual void eoFillPath( const uno::Reference<rendering::XPolyPolygon2D>& rPath ) override
        {
            GraphicsContext& rContext( getCurrentContext() );
            basegfx::B2DPolyPolygon aPath = basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rPath);
            aPath.transform( rContext.Transformation );

            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 1.0, rContext.LineColor.Alpha, 0.00000001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 0.0, rContext.LineColor.Blue, 0.00000001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 0.0, rContext.LineColor.Green, 0.00000001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Line color is black", 0.0, rContext.LineColor.Red, 0.00000001);

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Blend mode is normal",
                                    rendering::BlendMode::NORMAL, rContext.BlendMode );
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Flatness is 0",
                                    1, rContext.Flatness, 0.00000001 );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Font id is 0",
                                    sal_Int32(0), rContext.FontId );

            const char sExportString[] = "m12050 49610c-4310 0-7800-3490-7800-7800 0-4300 "
                "3490-7790 7800-7790 4300 0 7790 3490 7790 7790 0 4310-3490 7800-7790 7800z";
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Stroke is a 4-bezier circle",
                                    OUString(sExportString), basegfx::utils::exportToSvgD( aPath, true, true, false ) );

            m_bRedCircleSeen = true;
        }

        virtual void intersectClip(const uno::Reference<rendering::XPolyPolygon2D>& rPath) override
        {
            basegfx::B2DPolyPolygon aNewClip = basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rPath);
            basegfx::B2DPolyPolygon aCurClip = getCurrentContext().Clip;

            if( aCurClip.count() )  // #i92985# adapted API from (..., false, false) to (..., true, false)
                aNewClip = basegfx::utils::clipPolyPolygonOnPolyPolygon( aCurClip, aNewClip, true, false );

            getCurrentContext().Clip = aNewClip;
        }

        virtual void intersectEoClip(const uno::Reference<rendering::XPolyPolygon2D>& rPath) override
        {
            basegfx::B2DPolyPolygon aNewClip = basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rPath);
            basegfx::B2DPolyPolygon aCurClip = getCurrentContext().Clip;

            if( aCurClip.count() )  // #i92985# adapted API from (..., false, false) to (..., true, false)
                aNewClip = basegfx::utils::clipPolyPolygonOnPolyPolygon( aCurClip, aNewClip, true, false );

            getCurrentContext().Clip = aNewClip;
        }

        virtual void drawGlyphs( const OUString&             rGlyphs,
                                 const geometry::RealRectangle2D& /*rRect*/,
                                 const geometry::Matrix2D&        /*rFontMatrix*/,
                                 double /*fontSize*/) override
        {
            m_aTextOut.append(rGlyphs);
        }

        virtual void endText() override
        {
            m_aTextOut.append( "\n" );
        }

        virtual void drawMask(const uno::Sequence<beans::PropertyValue>& xBitmap,
                              bool                                       /*bInvert*/ ) override
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawMask received two properties",
                                    sal_Int32(3), xBitmap.getLength() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawMask got URL param",
                                    OUString("URL"), xBitmap[0].Name );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawMask got InputStream param",
                                    OUString("InputStream"), xBitmap[1].Name );
        }

        virtual void drawImage(const uno::Sequence<beans::PropertyValue>& xBitmap ) override
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawImage received two properties",
                                    sal_Int32(3), xBitmap.getLength() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawImage got URL param",
                                    OUString("URL"), xBitmap[0].Name );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawImage got InputStream param",
                                    OUString("InputStream"), xBitmap[1].Name );
            m_bImageSeen = true;
        }

        virtual void drawColorMaskedImage(const uno::Sequence<beans::PropertyValue>& xBitmap,
                                          const uno::Sequence<uno::Any>&             /*xMaskColors*/ ) override
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawColorMaskedImage received two properties",
                                    sal_Int32(3), xBitmap.getLength() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawColorMaskedImage got URL param",
                                    OUString("URL"), xBitmap[0].Name );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawColorMaskedImage got InputStream param",
                                    OUString("InputStream"), xBitmap[1].Name );
        }

        virtual void drawMaskedImage(const uno::Sequence<beans::PropertyValue>& xBitmap,
                                     const uno::Sequence<beans::PropertyValue>& xMask,
                                     bool                                       /*bInvertMask*/) override
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawMaskedImage received two properties #1",
                                    sal_Int32(3), xBitmap.getLength() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawMaskedImage got URL param #1",
                                    OUString("URL"), xBitmap[0].Name );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawMaskedImage got InputStream param #1",
                                    OUString("InputStream"), xBitmap[1].Name );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawMaskedImage received two properties #2",
                                    sal_Int32(3), xMask.getLength() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawMaskedImage got URL param #2",
                                    OUString("URL"), xMask[0].Name );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawMaskedImage got InputStream param #2",
                                    OUString("InputStream"), xMask[1].Name );
        }

        virtual void drawAlphaMaskedImage(const uno::Sequence<beans::PropertyValue>& xBitmap,
                                          const uno::Sequence<beans::PropertyValue>& xMask) override
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawAlphaMaskedImage received two properties #1",
                                    sal_Int32(3), xBitmap.getLength() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawAlphaMaskedImage got URL param #1",
                                    OUString("URL"), xBitmap[0].Name );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawAlphaMaskedImage got InputStream param #1",
                                    OUString("InputStream"), xBitmap[1].Name );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawAlphaMaskedImage received two properties #2",
                                    sal_Int32(3), xMask.getLength() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawAlphaMaskedImage got URL param #2",
                                    OUString("URL"), xMask[0].Name );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "drawAlphaMaskedImage got InputStream param #2",
                                    OUString("InputStream"), xMask[1].Name );
        }

        virtual void setTextRenderMode( sal_Int32 ) override
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
#if HAVE_FEATURE_POPPLER
            std::shared_ptr<TestSink> pSink( new TestSink() );
            CPPUNIT_ASSERT(
                pdfi::xpdf_ImportFromFile(
                    m_directories.getURLFromSrc("/sdext/source/pdfimport/test/testinput.pdf"),
                    pSink,
                    uno::Reference< task::XInteractionHandler >(),
                    OUString(),
                    getComponentContext(), "" ) );
            pSink->check();
#endif
        }

        void testOdfDrawExport()
        {
#if HAVE_FEATURE_POPPLER
            rtl::Reference<pdfi::PDFIRawAdaptor> xAdaptor( new pdfi::PDFIRawAdaptor(OUString(), getComponentContext()) );
            xAdaptor->setTreeVisitorFactory( createDrawTreeVisitorFactory() );

            OUString tempFileURL;
            CPPUNIT_ASSERT_EQUAL( osl::File::E_None, osl::File::createTempFile( nullptr, nullptr, &tempFileURL ) );
            osl::File::remove( tempFileURL ); // FIXME the below apparently fails silently if the file already exists
            CPPUNIT_ASSERT_MESSAGE("Exporting to ODF",
                                   xAdaptor->odfConvert( m_directories.getURLFromSrc("/sdext/source/pdfimport/test/testinput.pdf"),
                                                        new OutputWrap(tempFileURL),
                                                        nullptr ));
            osl::File::remove( tempFileURL );
#endif
        }

        void testOdfWriterExport()
        {
#if HAVE_FEATURE_POPPLER
            rtl::Reference<pdfi::PDFIRawAdaptor> xAdaptor( new pdfi::PDFIRawAdaptor(OUString(), getComponentContext()) );
            xAdaptor->setTreeVisitorFactory( createWriterTreeVisitorFactory() );

            OUString tempFileURL;
            CPPUNIT_ASSERT_EQUAL( osl::File::E_None, osl::File::createTempFile( nullptr, nullptr, &tempFileURL ) );
            osl::File::remove( tempFileURL ); // FIXME the below apparently fails silently if the file already exists
            CPPUNIT_ASSERT_MESSAGE("Exporting to ODF",
                                   xAdaptor->odfConvert( m_directories.getURLFromSrc("/sdext/source/pdfimport/test/testinput.pdf"),
                                                        new OutputWrap(tempFileURL),
                                                        nullptr ));
            osl::File::remove( tempFileURL );
#endif
        }

        void testTdf96993()
        {
#if HAVE_FEATURE_POPPLER
            rtl::Reference<pdfi::PDFIRawAdaptor> xAdaptor(new pdfi::PDFIRawAdaptor(OUString(), getComponentContext()));
            xAdaptor->setTreeVisitorFactory(createDrawTreeVisitorFactory());

            OString aOutput;
            CPPUNIT_ASSERT_MESSAGE("Exporting to ODF",
                xAdaptor->odfConvert(m_directories.getURLFromSrc("/sdext/source/pdfimport/test/testTdf96993.pdf"),
                new OutputWrapString(aOutput),
                nullptr));
            // This ensures that the imported image arrives properly flipped
            CPPUNIT_ASSERT(aOutput.indexOf("draw:transform=\"matrix(18520.8333333333 0 0 26281.9444444444 0 0)\"") != -1);
#endif
        }

        void testTdf98421()
        {
#if HAVE_FEATURE_POPPLER
            rtl::Reference<pdfi::PDFIRawAdaptor> xAdaptor(new pdfi::PDFIRawAdaptor(OUString(), getComponentContext()));
            xAdaptor->setTreeVisitorFactory(createWriterTreeVisitorFactory());

            OString aOutput;
            CPPUNIT_ASSERT_MESSAGE("Exporting to ODF",
                xAdaptor->odfConvert(m_directories.getURLFromSrc("/sdext/source/pdfimport/test/testTdf96993.pdf"),
                new OutputWrapString(aOutput),
                nullptr));
            // This ensures that the imported image arrives properly flipped
            CPPUNIT_ASSERT(aOutput.indexOf("draw:transform=\"scale( 1.0 -1.0 ) translate( 0mm 0mm )\"") != -1);
            CPPUNIT_ASSERT(aOutput.indexOf("svg:height=\"-262.82mm\"") != -1);
#endif
        }

        void testTdf105536()
        {
#if HAVE_FEATURE_POPPLER
            rtl::Reference<pdfi::PDFIRawAdaptor> xAdaptor(new pdfi::PDFIRawAdaptor(OUString(), getComponentContext()));
            xAdaptor->setTreeVisitorFactory(createDrawTreeVisitorFactory());

            OString aOutput;
            CPPUNIT_ASSERT_MESSAGE("Exporting to ODF",
                xAdaptor->odfConvert(m_directories.getURLFromSrc("/sdext/source/pdfimport/test/testTdf105536.pdf"),
                new OutputWrapString(aOutput),
                nullptr));
            // This ensures that the imported image arrives properly flipped
            CPPUNIT_ASSERT(aOutput.indexOf("draw:transform=\"matrix(-21488.4 0 0 -27978.1 21488.4 27978.1)\"") != -1);
#endif
        }

        CPPUNIT_TEST_SUITE(PDFITest);
        CPPUNIT_TEST(testXPDFParser);
        CPPUNIT_TEST(testOdfWriterExport);
        CPPUNIT_TEST(testOdfDrawExport);
        CPPUNIT_TEST(testTdf96993);
        CPPUNIT_TEST(testTdf98421);
        CPPUNIT_TEST(testTdf105536);
        CPPUNIT_TEST_SUITE_END();
    };

}

CPPUNIT_TEST_SUITE_REGISTRATION(PDFITest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
