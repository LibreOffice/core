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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#ifdef SYSTEM_ZLIB
#include "zlib.h"
#else
#include <zlib/zlib.h>
#endif

#include "outputwrap.hxx"
#include "contentsink.hxx"
#include "pdfihelper.hxx"
#include "wrapper.hxx"
#include "pdfparse.hxx"
#include "../pdfiadaptor.hxx"

#include <rtl/math.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <testshl/simpleheader.hxx>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/sequence.hxx>


#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XColorSpace.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/BlendMode.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>

#include <vector>
#include <boost/unordered_map.hpp>


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
            m_bDashedLineSeen(false)
        {}

        ~TestSink()
        {
            CPPUNIT_ASSERT_MESSAGE( "A4 page size (in 100th of points)",
                                    m_aPageSize.Width == 79400 && m_aPageSize.Height == 59500 );
            CPPUNIT_ASSERT_MESSAGE( "endPage() called", m_bPageEnded );
            CPPUNIT_ASSERT_MESSAGE( "Num pages equal one", m_nNumPages == 1 );
            CPPUNIT_ASSERT_MESSAGE( "Correct hyperlink bounding box",
                                    rtl::math::approxEqual(m_aHyperlinkBounds.X1,34.7 ) &&
                                    rtl::math::approxEqual(m_aHyperlinkBounds.Y1,386.0) &&
                                    rtl::math::approxEqual(m_aHyperlinkBounds.X2,166.7) &&
                                    rtl::math::approxEqual(m_aHyperlinkBounds.Y2,406.2) );
            CPPUNIT_ASSERT_MESSAGE( "Correct hyperlink URI",
                                    m_aURI == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("http://download.openoffice.org/")) );

            const char* sText = " \n \nThis is a testtext\nNew paragraph,\nnew line\n"
                "Hyperlink, this is\n?\nThis is more text\noutline mode\n?\nNew paragraph\n";
            ::rtl::OString aTmp;
            m_aTextOut.makeStringAndClear().convertToString( &aTmp,
                                                             RTL_TEXTENCODING_ASCII_US,
                                                             OUSTRING_TO_OSTRING_CVTFLAGS );
            CPPUNIT_ASSERT_MESSAGE( "Imported text is \"This is a testtext New paragraph, new line"
                                    " Hyperlink, this is * This is more text outline mode * New paragraph\"",
                                    sText == aTmp );

            CPPUNIT_ASSERT_MESSAGE( "red circle seen in input", m_bRedCircleSeen );
            CPPUNIT_ASSERT_MESSAGE( "green stroke seen in input", m_bGreenStrokeSeen );
            CPPUNIT_ASSERT_MESSAGE( "dashed line seen in input", m_bDashedLineSeen );
        }

    private:
        GraphicsContext& getCurrentContext() { return m_aGCStack.back(); }

        // ContentSink interface implementation
        virtual void setPageNum( sal_Int32 nNumPages )
        {
            m_nNumPages = nNumPages;
        }

        virtual void startPage( const geometry::RealSize2D& rSize )
        {
            m_aPageSize = rSize;
        }

        virtual void endPage()
        {
            m_bPageEnded = true;
        }

        virtual void hyperLink( const geometry::RealRectangle2D& rBounds,
                                const ::rtl::OUString&             rURI )
        {
            m_aHyperlinkBounds = rBounds;
            m_aURI = rURI;
        }

        virtual void pushState()
        {
            m_aGCStack.push_back( m_aGCStack.back() );
        }

        virtual void popState()
        {
            m_aGCStack.pop_back();
        }

        virtual void setTransformation( const geometry::AffineMatrix2D& rMatrix )
        {
            basegfx::unotools::homMatrixFromAffineMatrix(
                getCurrentContext().Transformation,
                rMatrix );
        }

        virtual void setLineDash( const uno::Sequence<double>& dashes,
                                  double                       start )
        {
            GraphicsContext& rContext( getCurrentContext() );
            if( dashes.getLength() )
                comphelper::sequenceToContainer(rContext.DashArray,dashes);
            CPPUNIT_ASSERT_MESSAGE( "line dashing start offset", start == 0.0 );
        }

        virtual void setFlatness( double nFlatness )
        {
            getCurrentContext().Flatness = nFlatness;
        }

        virtual void setLineJoin(sal_Int8 nJoin)
        {
            getCurrentContext().LineJoin = nJoin;
        }

        virtual void setLineCap(sal_Int8 nCap)
        {
            getCurrentContext().LineCap = nCap;
        }

        virtual void setMiterLimit(double nVal)
        {
            getCurrentContext().MiterLimit = nVal;
        }

        virtual void setLineWidth(double nVal)
        {
            getCurrentContext().LineWidth = nVal;
        }

        virtual void setFillColor( const rendering::ARGBColor& rColor )
        {
            getCurrentContext().FillColor = rColor;
        }

        virtual void setStrokeColor( const rendering::ARGBColor& rColor )
        {
            getCurrentContext().LineColor = rColor;
        }

        virtual void setBlendMode(sal_Int8 nMode)
        {
            getCurrentContext().BlendMode = nMode;
        }

        virtual void setFont( const FontAttributes& rFont )
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

        virtual void strokePath( const uno::Reference<rendering::XPolyPolygon2D>& rPath )
        {
            GraphicsContext& rContext( getCurrentContext() );
            basegfx::B2DPolyPolygon aPath = basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rPath);
            aPath.transform( rContext.Transformation );

            if( rContext.DashArray.empty() )
            {
                CPPUNIT_ASSERT_MESSAGE( "Line color is green",
                                        rContext.LineColor.Alpha == 1.0 &&
                                        rContext.LineColor.Red == 0.0 &&
                                        rContext.LineColor.Green == 1.0 &&
                                        rContext.LineColor.Blue == 0.0 );

                CPPUNIT_ASSERT_MESSAGE( "Line width is 0",
                                        rtl::math::approxEqual(rContext.LineWidth, 28.3) );

                const char* sExportString = "m53570 7650-35430 24100";
                CPPUNIT_ASSERT_MESSAGE( "Stroke is m535.7 518.5-354.3-241",
                                        basegfx::tools::exportToSvgD( aPath ).compareToAscii(sExportString) == 0 );

                m_bGreenStrokeSeen = true;
            }
            else
            {
                CPPUNIT_ASSERT_MESSAGE( "Dash array cons  ists of four entries",
                                        rContext.DashArray.size() == 4 &&
                                        rtl::math::approxEqual(rContext.DashArray[0],14.3764) &&
                                        rContext.DashArray[0] == rContext.DashArray[1] &&
                                        rContext.DashArray[1] == rContext.DashArray[2] &&
                                        rContext.DashArray[2] == rContext.DashArray[3] );

                CPPUNIT_ASSERT_MESSAGE( "Line color is black",
                                        rContext.LineColor.Alpha == 1.0 &&
                                        rContext.LineColor.Red == 0.0 &&
                                        rContext.LineColor.Green == 0.0 &&
                                        rContext.LineColor.Blue == 0.0 );

                CPPUNIT_ASSERT_MESSAGE( "Line width is 0",
                                        rContext.LineWidth == 0 );

                const char* sExportString = "m49890 5670.00000000001-35430 24090";
                CPPUNIT_ASSERT_MESSAGE( "Stroke is m49890 5670.00000000001-35430 24090",
                                        basegfx::tools::exportToSvgD( aPath ).compareToAscii(sExportString) == 0 );

                m_bDashedLineSeen = true;
            }
            CPPUNIT_ASSERT_MESSAGE( "Blend mode is normal",
                                    rContext.BlendMode == rendering::BlendMode::NORMAL );
            CPPUNIT_ASSERT_MESSAGE( "Join type is round",
                                    rContext.LineJoin == rendering::PathJoinType::ROUND );
            CPPUNIT_ASSERT_MESSAGE( "Cap type is butt",
                                    rContext.LineCap == rendering::PathCapType::BUTT );
            CPPUNIT_ASSERT_MESSAGE( "Line miter limit is 10",
                                    rContext.MiterLimit == 10 );
            CPPUNIT_ASSERT_MESSAGE( "Flatness is 0",
                                    rContext.Flatness == 1 );
            CPPUNIT_ASSERT_MESSAGE( "Font id is 0",
                                    rContext.FontId == 0 );
        }

        virtual void fillPath( const uno::Reference<rendering::XPolyPolygon2D>& rPath )
        {
            GraphicsContext& rContext( getCurrentContext() );
            basegfx::B2DPolyPolygon aPath = basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rPath);
            aPath.transform( rContext.Transformation );

            CPPUNIT_ASSERT_MESSAGE( "Fill color is black",
                                    rContext.FillColor.Alpha == 1.0 &&
                                    rContext.FillColor.Red == 0.0 &&
                                    rContext.FillColor.Green == 0.0 &&
                                    rContext.FillColor.Blue == 0.0 );
            CPPUNIT_ASSERT_MESSAGE( "Blend mode is normal",
                                    rContext.BlendMode == rendering::BlendMode::NORMAL );
            CPPUNIT_ASSERT_MESSAGE( "Flatness is 10",
                                    rContext.Flatness == 10 );
            CPPUNIT_ASSERT_MESSAGE( "Font id is 0",
                                    rContext.FontId == 0 );
        }

        virtual void eoFillPath( const uno::Reference<rendering::XPolyPolygon2D>& rPath )
        {
            GraphicsContext& rContext( getCurrentContext() );
            basegfx::B2DPolyPolygon aPath = basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rPath);
            aPath.transform( rContext.Transformation );

            CPPUNIT_ASSERT_MESSAGE( "Fill color is black",
                                    rContext.FillColor.Alpha == 1.0 &&
                                    rContext.FillColor.Red == 1.0 &&
                                    rContext.FillColor.Green == 0.0 &&
                                    rContext.FillColor.Blue == 0.0 );
            CPPUNIT_ASSERT_MESSAGE( "Blend mode is normal",
                                    rContext.BlendMode == rendering::BlendMode::NORMAL );
            CPPUNIT_ASSERT_MESSAGE( "Flatness is 0",
                                    rContext.Flatness == 1 );
            CPPUNIT_ASSERT_MESSAGE( "Font id is 0",
                                    rContext.FontId == 0 );

            const char* sExportString = "m12050 49610c-4310 0-7800-3490-7800-7800 0-4300 "
                "3490-7790 7800-7790 4300 0 7790 3490 7790 7790 0 4310-3490 7800-7790 7800z";
            CPPUNIT_ASSERT_MESSAGE( "Stroke is a 4-bezier circle",
                                    basegfx::tools::exportToSvgD( aPath ).compareToAscii(sExportString) == 0 );

            m_bRedCircleSeen = true;
        }

        virtual void intersectClip(const uno::Reference<rendering::XPolyPolygon2D>& rPath)
        {
            basegfx::B2DPolyPolygon aNewClip = basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rPath);
            basegfx::B2DPolyPolygon aCurClip = getCurrentContext().Clip;

            if( aCurClip.count() )  // #i92985# adapted API from (..., false, false) to (..., true, false)
                aNewClip = basegfx::tools::clipPolyPolygonOnPolyPolygon( aCurClip, aNewClip, true, false );

            getCurrentContext().Clip = aNewClip;
        }

        virtual void intersectEoClip(const uno::Reference<rendering::XPolyPolygon2D>& rPath)
        {
            basegfx::B2DPolyPolygon aNewClip = basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rPath);
            basegfx::B2DPolyPolygon aCurClip = getCurrentContext().Clip;

            if( aCurClip.count() )  // #i92985# adapted API from (..., false, false) to (..., true, false)
                aNewClip = basegfx::tools::clipPolyPolygonOnPolyPolygon( aCurClip, aNewClip, true, false );

            getCurrentContext().Clip = aNewClip;
        }

        virtual void drawGlyphs( const rtl::OUString&             rGlyphs,
                                 const geometry::RealRectangle2D& /*rRect*/,
                                 const geometry::Matrix2D&        /*rFontMatrix*/ )
        {
            m_aTextOut.append(rGlyphs);
        }

        virtual void endText()
        {
            m_aTextOut.append( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n")) );
        }

        virtual void drawMask(const uno::Sequence<beans::PropertyValue>& xBitmap,
                              bool                                       /*bInvert*/ )
        {
            CPPUNIT_ASSERT_MESSAGE( "drawMask received two properties",
                                    xBitmap.getLength()==3 );
            CPPUNIT_ASSERT_MESSAGE( "drawMask got URL param",
                                    xBitmap[0].Name.compareToAscii( "URL" ) == 0 );
            CPPUNIT_ASSERT_MESSAGE( "drawMask got InputStream param",
                                    xBitmap[1].Name.compareToAscii( "InputStream" ) == 0 );
        }

        virtual void drawImage(const uno::Sequence<beans::PropertyValue>& xBitmap )
        {
            CPPUNIT_ASSERT_MESSAGE( "drawImage received two properties",
                                    xBitmap.getLength()==3 );
            CPPUNIT_ASSERT_MESSAGE( "drawImage got URL param",
                                    xBitmap[0].Name.compareToAscii( "URL" ) == 0 );
            CPPUNIT_ASSERT_MESSAGE( "drawImage got InputStream param",
                                    xBitmap[1].Name.compareToAscii( "InputStream" ) == 0 );
        }

        virtual void drawColorMaskedImage(const uno::Sequence<beans::PropertyValue>& xBitmap,
                                          const uno::Sequence<uno::Any>&             /*xMaskColors*/ )
        {
            CPPUNIT_ASSERT_MESSAGE( "drawColorMaskedImage received two properties",
                                    xBitmap.getLength()==3 );
            CPPUNIT_ASSERT_MESSAGE( "drawColorMaskedImage got URL param",
                                    xBitmap[0].Name.compareToAscii( "URL" ) == 0 );
            CPPUNIT_ASSERT_MESSAGE( "drawColorMaskedImage got InputStream param",
                                    xBitmap[1].Name.compareToAscii( "InputStream" ) == 0 );
        }

        virtual void drawMaskedImage(const uno::Sequence<beans::PropertyValue>& xBitmap,
                                     const uno::Sequence<beans::PropertyValue>& xMask,
                                     bool                                       /*bInvertMask*/)
        {
            CPPUNIT_ASSERT_MESSAGE( "drawMaskedImage received two properties #1",
                                    xBitmap.getLength()==3 );
            CPPUNIT_ASSERT_MESSAGE( "drawMaskedImage got URL param #1",
                                    xBitmap[0].Name.compareToAscii( "URL" ) == 0 );
            CPPUNIT_ASSERT_MESSAGE( "drawMaskedImage got InputStream param #1",
                                    xBitmap[1].Name.compareToAscii( "InputStream" ) == 0 );

            CPPUNIT_ASSERT_MESSAGE( "drawMaskedImage received two properties #2",
                                    xMask.getLength()==3 );
            CPPUNIT_ASSERT_MESSAGE( "drawMaskedImage got URL param #2",
                                    xMask[0].Name.compareToAscii( "URL" ) == 0 );
            CPPUNIT_ASSERT_MESSAGE( "drawMaskedImage got InputStream param #2",
                                    xMask[1].Name.compareToAscii( "InputStream" ) == 0 );
        }

        virtual void drawAlphaMaskedImage(const uno::Sequence<beans::PropertyValue>& xBitmap,
                                          const uno::Sequence<beans::PropertyValue>& xMask)
        {
            CPPUNIT_ASSERT_MESSAGE( "drawAlphaMaskedImage received two properties #1",
                                    xBitmap.getLength()==3 );
            CPPUNIT_ASSERT_MESSAGE( "drawAlphaMaskedImage got URL param #1",
                                    xBitmap[0].Name.compareToAscii( "URL" ) == 0 );
            CPPUNIT_ASSERT_MESSAGE( "drawAlphaMaskedImage got InputStream param #1",
                                    xBitmap[1].Name.compareToAscii( "InputStream" ) == 0 );

            CPPUNIT_ASSERT_MESSAGE( "drawAlphaMaskedImage received two properties #2",
                                    xMask.getLength()==3 );
            CPPUNIT_ASSERT_MESSAGE( "drawAlphaMaskedImage got URL param #2",
                                    xMask[0].Name.compareToAscii( "URL" ) == 0 );
            CPPUNIT_ASSERT_MESSAGE( "drawAlphaMaskedImage got InputStream param #2",
                                    xMask[1].Name.compareToAscii( "InputStream" ) == 0 );
        }

        virtual void setTextRenderMode( sal_Int32 )
        {
        }

        typedef boost::unordered_map<sal_Int32,FontAttributes> IdToFontMap;
        typedef boost::unordered_map<FontAttributes,sal_Int32,FontAttrHash> FontToIdMap;

        typedef boost::unordered_map<sal_Int32,GraphicsContext> IdToGCMap;
        typedef boost::unordered_map<GraphicsContext,sal_Int32,GraphicsContextHash> GCToIdMap;

        typedef std::vector<GraphicsContext> GraphicsContextStack;

        sal_Int32                 m_nNextFontId;
        IdToFontMap               m_aIdToFont;
        FontToIdMap               m_aFontToId;

        GraphicsContextStack      m_aGCStack;
        geometry::RealSize2D      m_aPageSize;
        geometry::RealRectangle2D m_aHyperlinkBounds;
        ::rtl::OUString           m_aURI;
        ::rtl::OUStringBuffer     m_aTextOut;
        sal_Int32                 m_nNumPages;
        bool                      m_bPageEnded;
        bool                      m_bRedCircleSeen;
        bool                      m_bGreenStrokeSeen;
        bool                      m_bDashedLineSeen;
    };

    class PDFITest : public CppUnit::TestFixture
    {
        uno::Reference<uno::XComponentContext> mxCtx;
        rtl::OUString                          msBaseDir;
        bool                                   mbUnoInitialized;

    public:
        PDFITest() : mxCtx(),msBaseDir(),mbUnoInitialized(false)
        {}

        void setUp()
        {
            if( !mbUnoInitialized )
            {
                const char* pArgs( getForwardString() );
                CPPUNIT_ASSERT_MESSAGE("Test file parameter", pArgs);

                msBaseDir = rtl::OUString::createFromAscii(pArgs);

                // bootstrap UNO
                try
                {
                    ::rtl::OUString aIniUrl;
                    CPPUNIT_ASSERT_MESSAGE(
                        "Converting ini file to URL",
                        osl_getFileURLFromSystemPath(
                            (msBaseDir+rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("pdfi_unittest_test.ini"))).pData,
                            &aIniUrl.pData ) == osl_File_E_None );

                    mxCtx = ::cppu::defaultBootstrap_InitialComponentContext(aIniUrl);
                    CPPUNIT_ASSERT_MESSAGE("Getting component context", mxCtx.is());
                }
                catch( uno::Exception& )
                {
                    CPPUNIT_ASSERT_MESSAGE("Bootstrapping UNO", false);
                }

                mbUnoInitialized = true;
            }
        }
        void tearDown()
        {
        }

        void testXPDFParser()
        {
            pdfi::ContentSinkSharedPtr pSink( new TestSink() );
            pdfi::xpdf_ImportFromFile( msBaseDir + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("pdfi_unittest_test.pdf")),
                                       pSink,
                                       uno::Reference< task::XInteractionHandler >(),
                                       rtl::OUString(),
                                       mxCtx );

            // make destruction explicit, a bunch of things are
            // checked in the destructor
            pSink.reset();
        }

        void testOdfDrawExport()
        {
            pdfi::PDFIRawAdaptor aAdaptor( mxCtx );
            aAdaptor.setTreeVisitorFactory( createDrawTreeVisitorFactory() );

            ::rtl::OUString aURL, aAbsURL, aBaseURL;
            osl_getFileURLFromSystemPath( (msBaseDir + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("pdfi_unittest_draw.xml"))).pData,
                                          &aURL.pData );
            osl_getProcessWorkingDir(&aBaseURL.pData);
            osl_getAbsoluteFileURL(aBaseURL.pData,aURL.pData,&aAbsURL.pData);
            CPPUNIT_ASSERT_MESSAGE("Exporting to ODF",
                                   aAdaptor.odfConvert( msBaseDir + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("pdfi_unittest_test.pdf")),
                                                        new OutputWrap(aAbsURL),
                                                        NULL ));
        }

        void testOdfWriterExport()
        {
            pdfi::PDFIRawAdaptor aAdaptor( mxCtx );
            aAdaptor.setTreeVisitorFactory( createWriterTreeVisitorFactory() );

            ::rtl::OUString aURL, aAbsURL, aBaseURL;
            osl_getFileURLFromSystemPath( (msBaseDir + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("pdfi_unittest_writer.xml"))).pData,
                                          &aURL.pData );
            osl_getProcessWorkingDir(&aBaseURL.pData);
            osl_getAbsoluteFileURL(aBaseURL.pData,aURL.pData,&aAbsURL.pData);
            CPPUNIT_ASSERT_MESSAGE("Exporting to ODF",
                                   aAdaptor.odfConvert( msBaseDir + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("pdfi_unittest_test.pdf")),
                                                        new OutputWrap(aAbsURL),
                                                        NULL ));
        }

        CPPUNIT_TEST_SUITE(PDFITest);
        CPPUNIT_TEST(testXPDFParser);
        CPPUNIT_TEST(testOdfWriterExport);
        CPPUNIT_TEST(testOdfDrawExport);
        CPPUNIT_TEST_SUITE_END();
    };

}

// =======================================================================

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(PDFITest, "PDFITest");


// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
NOADDITIONAL;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
