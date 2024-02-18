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

#include <outputwrap.hxx>
#include <contentsink.hxx>
#include <pdfihelper.hxx>
#include <wrapper.hxx>
#include <pdfparse.hxx>
#include <../pdfiadaptor.hxx>

#include <rtl/math.hxx>
#include <osl/file.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/string.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <test/bootstrapfixture.hxx>
#include <test/xmltesttools.hxx>

#include <com/sun/star/geometry/RealRectangle2D.hpp>
#include <com/sun/star/geometry/RealSize2D.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/BlendMode.hpp>

#include <basegfx/utils/canvastools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>

#include <unordered_map>
#include <vector>

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
                                    rtl::math::approxEqual(m_aHyperlinkBounds.X1,34.7 ) );
            CPPUNIT_ASSERT_MESSAGE( "Correct hyperlink bounding box",
                                    rtl::math::approxEqual(m_aHyperlinkBounds.Y1,386.0) );
            CPPUNIT_ASSERT_MESSAGE( "Correct hyperlink bounding box",
                                    rtl::math::approxEqual(m_aHyperlinkBounds.X2,166.7) );
            CPPUNIT_ASSERT_MESSAGE( "Correct hyperlink bounding box",
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
            if( dashes.hasElements() )
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

                static constexpr OUString sExportString = u"m53570 7650-35430 24100"_ustr;
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Stroke is m535.7 518.5-354.3-241",
                                        sExportString, basegfx::utils::exportToSvgD( aPath, true, true, false ) );

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

                static constexpr OUString sExportString = u"m49890 5670.00000000001-35430 24090"_ustr;
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Stroke is m49890 5670.00000000001-35430 24090",
                                        sExportString, basegfx::utils::exportToSvgD( aPath, true, true, false ) );

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

            static constexpr OUString sExportString
                = u"m12050 49610c-4310 0-7800-3490-7800-7800 0-4300 "
                "3490-7790 7800-7790 4300 0 7790 3490 7790 7790 0 4310-3490 7800-7790 7800z"_ustr;
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Stroke is a 4-bezier circle",
                                    sExportString, basegfx::utils::exportToSvgD( aPath, true, true, false ) );

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

        virtual void tilingPatternFill(int, int, int, int,
                                       double, double,
                                       int,
                                       css::geometry::AffineMatrix2D&,
                                       const css::uno::Sequence<css::beans::PropertyValue>&) override
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

    class PDFITest : public test::BootstrapFixture, public XmlTestTools
    {
    public:
        void testXPDFParser()
        {
#if HAVE_FEATURE_POPPLER
            auto pSink = std::make_shared<TestSink>();
            CPPUNIT_ASSERT(
                pdfi::xpdf_ImportFromFile(
                    m_directories.getURLFromSrc(u"/sdext/source/pdfimport/test/testinput.pdf"),
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
                                   xAdaptor->odfConvert( m_directories.getURLFromSrc(u"/sdext/source/pdfimport/test/testinput.pdf"),
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
                                   xAdaptor->odfConvert( m_directories.getURLFromSrc(u"/sdext/source/pdfimport/test/testinput.pdf"),
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
                xAdaptor->odfConvert(m_directories.getURLFromSrc(u"/sdext/source/pdfimport/test/testTdf96993.pdf"),
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
                xAdaptor->odfConvert(m_directories.getURLFromSrc(u"/sdext/source/pdfimport/test/testTdf96993.pdf"),
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
                xAdaptor->odfConvert(m_directories.getURLFromSrc(u"/sdext/source/pdfimport/test/testTdf105536.pdf"),
                new OutputWrapString(aOutput),
                nullptr));
            // This ensures that the imported image arrives properly flipped
            CPPUNIT_ASSERT(aOutput.indexOf("draw:transform=\"matrix(-21488.4 0 0 -27978.1 21488.4 27978.1)\"") != -1);
#endif
        }

        void testTdf141709_chinesechar()
        {
// this test crashes on the windows jenkins boxes, but no-one can catch it locally
#if HAVE_FEATURE_POPPLER
            rtl::Reference<pdfi::PDFIRawAdaptor> xAdaptor(new pdfi::PDFIRawAdaptor(OUString(), getComponentContext()));
            xAdaptor->setTreeVisitorFactory(createDrawTreeVisitorFactory());

            OString aOutput;
            CPPUNIT_ASSERT_MESSAGE("Exporting to ODF",
                xAdaptor->odfConvert(m_directories.getURLFromSrc(u"/sdext/qa/unit/data/testTdf141709_chinesechar.pdf"),
                new OutputWrapString(aOutput),
                nullptr));
            xmlDocUniquePtr pXmlDoc(xmlParseDoc(reinterpret_cast<xmlChar const *>(aOutput.getStr())));
            // This ensures that the imported text contains all of the characters
            OString xpath = "//draw:frame[@draw:z-index='3'][1]/draw:text-box/text:p/text:span[1]"_ostr;
            OUString  sContent = getXPathContent(pXmlDoc, xpath).replaceAll("\n", "");
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aOutput.getStr(), u"敏捷的狐狸跨过慵懒的"_ustr, sContent);
            xpath = "//draw:frame[@draw:z-index='4'][1]/draw:text-box/text:p/text:span[1]"_ostr;
            sContent = getXPathContent(pXmlDoc, xpath).replaceAll("\n", "");
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aOutput.getStr(), u"狗。"_ustr, sContent);
#endif
        }

        void testTdf78427_FontFeatures()
        {
#if HAVE_FEATURE_POPPLER
            rtl::Reference<pdfi::PDFIRawAdaptor> xAdaptor(new pdfi::PDFIRawAdaptor(OUString(), getComponentContext()));
            xAdaptor->setTreeVisitorFactory(createDrawTreeVisitorFactory());

            OString aOutput;
            CPPUNIT_ASSERT_MESSAGE("Converting PDF to ODF XML",
                xAdaptor->odfConvert( m_directories.getURLFromSrc(
                    u"/sdext/qa/unit/data/tdf78427-testFontFeatures.pdf"),
                new OutputWrapString(aOutput),
                nullptr ));
            // Un-comment the following debug line to see the content of generated XML content in
            // workdir/CppunitTest/sdext_pdfimport.test.log after running "make CppunitTest_sdext_pdfimport".
            //std::cout << aOutput << std::endl;
            xmlDocUniquePtr pXmlDoc(xmlParseDoc(reinterpret_cast<xmlChar const *>(aOutput.getStr())));
            //CPPUNIT_ASSERT(pXmlDoc);

            /* Test for the 1st paragraph */
            OUString styleName = getXPath(pXmlDoc, "//draw:frame[1]//text:span[1]"_ostr, "style-name"_ostr);
            OString xpath = "//office:automatic-styles/style:style[@style:name=\"" +
                OUStringToOString(styleName,  RTL_TEXTENCODING_UTF8) +
                "\"]/style:text-properties";
            // the font-weight and font-style should be normal
            assertXPath(pXmlDoc, xpath, "font-weight"_ostr, "normal");
            assertXPathNoAttribute(pXmlDoc, xpath, "font-style"_ostr);

            /* Test for the 2nd paragraph */
            styleName = getXPath(pXmlDoc, "//draw:frame[2]//text:span[1]"_ostr, "style-name"_ostr);
            xpath = "//office:automatic-styles/style:style[@style:name=\"" +
                OUStringToOString(styleName,  RTL_TEXTENCODING_UTF8) +
                "\"]/style:text-properties";
            // there should be a font-weight="bold", but no font-style italic
            assertXPath(pXmlDoc, xpath, "font-weight"_ostr, "bold");
            assertXPathNoAttribute(pXmlDoc, xpath, "font-style"_ostr);

            /* Test for the 3rd paragraph */
            styleName = getXPath(pXmlDoc, "//draw:frame[3]//text:span[1]"_ostr, "style-name"_ostr);
            xpath = "//office:automatic-styles/style:style[@style:name=\"" +
                OUStringToOString(styleName,  RTL_TEXTENCODING_UTF8) +
                "\"]/style:text-properties";
            // there should be a font-style="italic", but no font-weight bold
            assertXPath(pXmlDoc, xpath, "font-weight"_ostr, "normal");
            assertXPath(pXmlDoc, xpath, "font-style"_ostr, "italic");

            /* Test for the 4th paragraph */
            styleName = getXPath(pXmlDoc, "//draw:frame[4]//text:span[1]"_ostr, "style-name"_ostr);
            xpath = "//office:automatic-styles/style:style[@style:name=\"" +
                OUStringToOString(styleName,  RTL_TEXTENCODING_UTF8) +
                "\"]/style:text-properties";
            // there should be both font-style="italic" and font-weight="bold"
            assertXPath(pXmlDoc, xpath, "font-weight"_ostr, "bold");
            assertXPath(pXmlDoc, xpath, "font-style"_ostr, "italic");

            /* Test for the 5th paragraph */
            styleName = getXPath(pXmlDoc, "//draw:frame[5]//text:span[1]"_ostr, "style-name"_ostr);
            xpath = "//office:automatic-styles/style:style[@style:name=\"" +
                OUStringToOString(styleName,  RTL_TEXTENCODING_UTF8) +
                "\"]/style:text-properties";
            // the font should be Arial and font-weight="bold", no font-style
            assertXPath(pXmlDoc, xpath, "font-family"_ostr, "Arial");
            assertXPath(pXmlDoc, xpath, "font-weight"_ostr, "bold");
            assertXPathNoAttribute(pXmlDoc, xpath, "font-style"_ostr);

            /* Test for the 6th paragraph */
            styleName = getXPath(pXmlDoc, "//draw:frame[6]//text:span[1]"_ostr, "style-name"_ostr);
            xpath = "//office:automatic-styles/style:style[@style:name=\"" +
                OUStringToOString(styleName,  RTL_TEXTENCODING_UTF8) +
                "\"]/style:text-properties";
            // the font should be Arial without font-weight and font-style
            assertXPath(pXmlDoc, xpath, "font-family"_ostr, "Arial");
            assertXPath(pXmlDoc, xpath, "font-weight"_ostr, "normal");
            assertXPathNoAttribute(pXmlDoc, xpath, "font-style"_ostr);

            /* Test for the 7th paragraph */
            styleName = getXPath(pXmlDoc, "//draw:frame[7]//text:span[1]"_ostr, "style-name"_ostr);
            xpath = "//office:automatic-styles/style:style[@style:name=\"" +
                OUStringToOString(styleName,  RTL_TEXTENCODING_UTF8) +
                "\"]/style:text-properties";
            // the font should be SimSun without font-weight and font-style
            assertXPath(pXmlDoc, xpath, "font-family"_ostr, "SimSun"); // TODO: tdf#143095 use localized font name rather than PS name
            assertXPath(pXmlDoc, xpath, "font-weight"_ostr, "normal");
            assertXPathNoAttribute(pXmlDoc, xpath, "font-style"_ostr);

            /* Test for the 8th paragraph */
            styleName = getXPath(pXmlDoc, "//draw:frame[8]//text:span[1]"_ostr, "style-name"_ostr);
            xpath = "//office:automatic-styles/style:style[@style:name=\"" +
                OUStringToOString(styleName,  RTL_TEXTENCODING_UTF8) +
                "\"]/style:text-properties";
            // the font should be SimSun and font-weight="bold", no font-style italic
            assertXPath(pXmlDoc, xpath, "font-family"_ostr, "SimSun");
            assertXPath(pXmlDoc, xpath, "font-weight"_ostr, "bold");
            assertXPathNoAttribute(pXmlDoc, xpath, "font-style"_ostr);

            /* Test for the 9th paragraph */
            styleName = getXPath(pXmlDoc, "//draw:frame[9]//text:span[1]"_ostr, "style-name"_ostr);
            xpath = "//office:automatic-styles/style:style[@style:name=\"" +
                OUStringToOString(styleName,  RTL_TEXTENCODING_UTF8) +
                "\"]/style:text-properties";
            // the font should be SimSun, font-weight should be "normal", font-style="italic"
            assertXPath(pXmlDoc, xpath, "font-family"_ostr, "SimSun");
            assertXPath(pXmlDoc, xpath, "font-weight"_ostr, "normal");
            // FIXME and remove the below comment:
            // the chinese chars are shown in pdf as faux italic (fake italic). It is currencly imported wrongly as normal font style.
            // See tdf#78427 for how the faux bold problem was handled. Faux italic may be handled using the transformation pattern.
            // assertXPath(pXmlDoc, xpath, "font-style", "italic");

            /* Test for the 10th paragraph */
            styleName = getXPath(pXmlDoc, "//draw:frame[10]//text:span[1]"_ostr, "style-name"_ostr);
            xpath = "//office:automatic-styles/style:style[@style:name=\"" +
                OUStringToOString(styleName,  RTL_TEXTENCODING_UTF8) +
                "\"]/style:text-properties";
            // the font should be SimSun font-weight="bold" and font-style="italic"
            assertXPath(pXmlDoc, xpath, "font-family"_ostr, "SimSun");
            assertXPath(pXmlDoc, xpath, "font-weight"_ostr, "bold");
            // FIXME: faux italic, see above
            // assertXPath(pXmlDoc, xpath, "font-style", "italic");

            /* Test for the 11th paragraph */
            styleName = getXPath(pXmlDoc, "//draw:frame[11]//text:span[1]"_ostr, "style-name"_ostr);
            xpath = "//office:automatic-styles/style:style[@style:name=\"" +
                OUStringToOString(styleName,  RTL_TEXTENCODING_UTF8) +
                "\"]/style:text-properties";
            // the font should be SimSun and there should be style:text-outline="true"
            // (i.e., the real "outline" font rather than faux bold / fake bold)
            assertXPath(pXmlDoc, xpath, "font-family"_ostr, "SimSun");
            assertXPath(pXmlDoc, xpath, "font-weight"_ostr, "normal");
            assertXPathNoAttribute(pXmlDoc, xpath, "font-style"_ostr);
            assertXPath(pXmlDoc, xpath, "text-outline"_ostr, "true");
#endif
        }

        void testTdf78427_FontWeight_MyraidProSemibold() // Related to attachment 155937.
        {
#if HAVE_FEATURE_POPPLER
            rtl::Reference<pdfi::PDFIRawAdaptor> xAdaptor(new pdfi::PDFIRawAdaptor(OUString(), getComponentContext()));
            xAdaptor->setTreeVisitorFactory(createDrawTreeVisitorFactory());

            OString aOutput;
            CPPUNIT_ASSERT_MESSAGE("Converting PDF to ODF XML",
                xAdaptor->odfConvert( m_directories.getURLFromSrc(
                    u"/sdext/qa/unit/data/tdf78427-MyraidPro-Semibold-Light.pdf"),
                new OutputWrapString(aOutput),
                nullptr ));
            //std::cout << aOutput << std::endl;

            xmlDocUniquePtr pXmlDoc(xmlParseDoc(reinterpret_cast<xmlChar const *>(aOutput.getStr())));
            //CPPUNIT_ASSERT(pXmlDoc);

            // The for the 1st frame */
            OUString styleName = getXPath(pXmlDoc, "//draw:frame[1]//text:span[1]"_ostr, "style-name"_ostr);
            OString xpath = "//office:automatic-styles/style:style[@style:name=\"" +
                OUStringToOString(styleName,  RTL_TEXTENCODING_UTF8) +
                "\"]/style:text-properties";
            // the font-weight and font-style should be 600 (Semibold)
            assertXPath(pXmlDoc, xpath, "font-weight"_ostr, "600");

            // The for the 2nd frame */
            styleName = getXPath(pXmlDoc, "//draw:frame[2]//text:span[1]"_ostr, "style-name"_ostr);
            xpath = "//office:automatic-styles/style:style[@style:name=\"" +
                OUStringToOString(styleName,  RTL_TEXTENCODING_UTF8) +
                "\"]/style:text-properties";
            // the font-weight and font-style should be 300 (Light)
            assertXPath(pXmlDoc, xpath, "font-weight"_ostr, "300");
#endif
        }

        void testTdf143959_nameFromFontFile()
        {
#if HAVE_FEATURE_POPPLER
            rtl::Reference<pdfi::PDFIRawAdaptor> xAdaptor(new pdfi::PDFIRawAdaptor(OUString(), getComponentContext()));
            xAdaptor->setTreeVisitorFactory(createDrawTreeVisitorFactory());

            OString aOutput;
            CPPUNIT_ASSERT_MESSAGE("Converting PDF to ODF XML",
                                   xAdaptor->odfConvert( m_directories.getURLFromSrc(u"/sdext/qa/unit/data/testTdf143959.pdf"),
                                                        new OutputWrapString(aOutput),
                                                        nullptr ));

            //std::cout << aOutput << std::endl;
            xmlDocUniquePtr pXmlDoc(xmlParseDoc(reinterpret_cast<xmlChar const *>(aOutput.getStr())));

            /* Test for the 1st text paragraph */
            OUString styleName = getXPath(pXmlDoc, "//draw:frame[2]//text:span[1]"_ostr, "style-name"_ostr);
            OString xpath = "//office:automatic-styles/style:style[@style:name=\"" +
                OUStringToOString(styleName,  RTL_TEXTENCODING_UTF8) +
                "\"]/style:text-properties";
            CPPUNIT_ASSERT_EQUAL(OUString("TimesNewRoman"),
                                 getXPath(pXmlDoc, xpath, "font-family"_ostr).replaceAll(u" ", u""));

            /* Test for the "TOTAL ESTA HOJA USD" paragraph" */
            styleName = getXPath(pXmlDoc, "//draw:frame[last()-1]//text:span[1]"_ostr, "style-name"_ostr);
            xpath = "//office:automatic-styles/style:style[@style:name=\"" +
                OUStringToOString(styleName,  RTL_TEXTENCODING_UTF8) +
                "\"]/style:text-properties";
            CPPUNIT_ASSERT_EQUAL(OUString("TimesNewRoman"),
                                 getXPath(pXmlDoc, xpath, "font-family"_ostr).replaceAll(u" ", u""));
            CPPUNIT_ASSERT_EQUAL(OUString("bold"),
                                 getXPath(pXmlDoc, xpath, "font-weight"_ostr));
#endif
        }

        void testTdf104597_textrun()
        {
#if HAVE_FEATURE_POPPLER
            rtl::Reference<pdfi::PDFIRawAdaptor> xAdaptor(new pdfi::PDFIRawAdaptor(OUString(), getComponentContext()));
            xAdaptor->setTreeVisitorFactory(createDrawTreeVisitorFactory());

            OString aOutput;
            CPPUNIT_ASSERT_MESSAGE("Converting PDF to ODF XML",
                xAdaptor->odfConvert(m_directories.getURLFromSrc(u"/sdext/qa/unit/data/tdf104597_textrun.pdf"),
                    new OutputWrapString(aOutput),
                    nullptr));

            xmlDocUniquePtr pXmlDoc(xmlParseDoc(reinterpret_cast<xmlChar const *>(aOutput.getStr())));

            // Test for امُ عَلَيْكَ
            OString xpath = "string(//draw:frame[@draw:transform='matrix(917.222222222222 0 0 917.222222222222 14821.9583333333 2159.23861112778)']/draw:text-box/text:p/text:span)"_ostr;
            OUString sContent = getXPathContent(pXmlDoc, xpath);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aOutput.getStr(), u"امُ عَلَيَْك"_ustr, sContent.replaceAll("\n\n", " ").replaceAll("\n", ""));

            // Test for ٱلسََّل . It appears in the 3rd frame, i.e. after the امُ عَلَيَْك which is in the 2nd frame (from left to right)
            // thus these two frames together appear as ٱلسََّل امُ عَلَيْكَ in Draw‬.
            // FIXME: Should be ٱلسَّلَامُ عَلَيْكَ (i.e. the two text frames should be merged into one so that the ل and the ا will show as لَا rather than ل ا)
            xpath = "string(//draw:frame[@draw:transform='matrix(917.222222222222 0 0 917.222222222222 17420.1666666667 2159.23861112778)']/draw:text-box/text:p/text:span)"_ostr;
            sContent = getXPathContent(pXmlDoc, xpath);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aOutput.getStr(), u"ٱلسََّل"_ustr, sContent.replaceAll("\n\n", " ").replaceAll("\n", ""));

            // Test for "LibreOffice RTL"
            xpath = "string(//draw:frame[@draw:transform='matrix(917.222222222222 0 0 917.222222222222 12779.375 5121.79583335)']/draw:text-box/text:p/text:span)"_ostr;
            sContent = getXPathContent(pXmlDoc, xpath);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aOutput.getStr(), u"LibreOffice RTL"_ustr, sContent.replaceAll("\n\n", " ").replaceAll("\n", ""));

            // Test for "LibreOffice LTR (test)"
            xpath = "string(//draw:frame[last()-1]/draw:text-box/text:p/text:span[last()])"_ostr;
            sContent = getXPathContent(pXmlDoc, xpath);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aOutput.getStr(), u"LibreOffice LTR (test)"_ustr, sContent.replaceAll("\n\n", " ").replaceAll("\n", ""));

            /* Test for Chinese characters */
            // Use last() instead of matrix below, because the matrix may be different on different OS due to fallback of Chinese fonts.
            xpath = "string(//draw:frame[last()]/draw:text-box/text:p/text:span)"_ostr;
            sContent = getXPathContent(pXmlDoc, xpath);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aOutput.getStr(), u"中文测试，中文"_ustr, sContent.replaceAll("\n\n", " ").replaceAll("\n", ""));

            // Test pdf text run in the Writer PDF import filter
            xAdaptor->setTreeVisitorFactory(createWriterTreeVisitorFactory());
            OString aOutput2;
            xAdaptor->odfConvert(m_directories.getURLFromSrc(u"/sdext/qa/unit/data/tdf104597_textrun.pdf"),
                    new OutputWrapString(aOutput2),
                    nullptr);
            xmlDocUniquePtr pXmlDoc2(xmlParseDoc(reinterpret_cast<xmlChar const *>(aOutput2.getStr())));
            xpath = "string(//draw:frame[@draw:z-index='3'][1]/draw:text-box/text:p/text:span)"_ostr;
            sContent = getXPathContent(pXmlDoc2, xpath).replaceAll("\n\n", " ").replaceAll("\n", "");
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aOutput2.getStr(), u"ٱلسََّل"_ustr, sContent);
            xpath = "string(//draw:frame[@draw:z-index='2'][1]/draw:text-box/text:p/text:span)"_ostr;
            sContent = getXPathContent(pXmlDoc2, xpath).replaceAll("\n\n", " ").replaceAll("\n", "");
            CPPUNIT_ASSERT_EQUAL(u"امُ عَلَيَْك"_ustr, sContent);
            xpath = "string(//draw:frame[last()]/draw:text-box/text:p/text:span)"_ostr;
            sContent = getXPathContent(pXmlDoc2, xpath).replaceAll("\n\n", " ").replaceAll("\n", "");
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aOutput2.getStr(), u"中文测试，中文"_ustr, sContent);
#endif
        }

        void testSpaces()
        {
#if HAVE_FEATURE_POPPLER
            rtl::Reference<pdfi::PDFIRawAdaptor> xAdaptor(new pdfi::PDFIRawAdaptor(OUString(), getComponentContext()));
            xAdaptor->setTreeVisitorFactory(createWriterTreeVisitorFactory());

            OString aOutput;
            xAdaptor->odfConvert(m_directories.getURLFromSrc(u"/sdext/qa/unit/data/testSpace.pdf"),
                    new OutputWrapString(aOutput),
                    nullptr);
            xmlDocUniquePtr pXmlDoc(xmlParseDoc(reinterpret_cast<xmlChar const *>(aOutput.getStr())));

            // Space test: there are 10 spaces, each space is expressed as a <text:s text:c="1" ...>,
            // thus the 10th text:s should exist and the attribute "text:c" should be "1".
            OString xpath = "//draw:frame[@draw:z-index='1'][1]/draw:text-box/text:p/text:span/text:s[10]"_ostr;
            OUString  sContent = getXPath(pXmlDoc, xpath, "c"_ostr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aOutput.getStr(), OUString("1"), sContent);

            // Tab test: there are 10 tabs. Text before and after the tabs are shown in different draw frames.
            // With the Liberation Serif font, the horizontal position of the first frame is 20.03mm and the
            // second frame is 94.12mm.
            xpath = "//draw:frame[@draw:z-index='2'][1]"_ostr;
            sContent = getXPath(pXmlDoc, xpath, "transform"_ostr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aOutput.getStr(), OUString("translate( 20.03mm 25.05mm )"), sContent);
            xpath = "//draw:frame[@draw:z-index='3'][1]"_ostr;
            sContent = getXPath(pXmlDoc, xpath, "transform"_ostr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aOutput.getStr(), OUString("translate( 94.12mm 25.05mm )"), sContent);

            // Non-breaking space test: there are 10 NBSpaces, which are treated as the same as normal space in PDF,
            // thus each is expressed as a <text:s text:c="1" ...>.
            // The 10th text:s should exist and the attribute "text:c" should be "1".
            xpath = "//draw:frame[@draw:z-index='4'][1]/draw:text-box/text:p/text:span/text:s[10]"_ostr;
            sContent = getXPath(pXmlDoc, xpath, "c"_ostr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aOutput.getStr(), OUString("1"), sContent);
#endif
        }

        CPPUNIT_TEST_SUITE(PDFITest);
        CPPUNIT_TEST(testXPDFParser);
        CPPUNIT_TEST(testOdfWriterExport);
        CPPUNIT_TEST(testOdfDrawExport);
        CPPUNIT_TEST(testTdf96993);
        CPPUNIT_TEST(testTdf98421);
        CPPUNIT_TEST(testTdf105536);
        CPPUNIT_TEST(testTdf141709_chinesechar);
        CPPUNIT_TEST(testTdf78427_FontFeatures);
        CPPUNIT_TEST(testTdf78427_FontWeight_MyraidProSemibold);
        CPPUNIT_TEST(testTdf143959_nameFromFontFile);
        CPPUNIT_TEST(testTdf104597_textrun);
        CPPUNIT_TEST(testSpaces);
        CPPUNIT_TEST_SUITE_END();
    };

}

CPPUNIT_TEST_SUITE_REGISTRATION(PDFITest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
