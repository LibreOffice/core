/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#define BASEGFX_STATICLIBRARY

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
#include <gtest/gtest.h>
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
#include <hash_map>


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
            ASSERT_TRUE(m_aPageSize.Width == 79400 && m_aPageSize.Height == 59500) << "A4 page size (in 100th of points)";
            ASSERT_TRUE(m_bPageEnded) << "endPage() called";
            ASSERT_TRUE(m_nNumPages == 1) << "Num pages equal one";
            ASSERT_TRUE(rtl::math::approxEqual(m_aHyperlinkBounds.X1,34.7 ) &&
                                    rtl::math::approxEqual(m_aHyperlinkBounds.Y1,386.0) &&
                                    rtl::math::approxEqual(m_aHyperlinkBounds.X2,166.7) &&
                                    rtl::math::approxEqual(m_aHyperlinkBounds.Y2,406.2)) << "Correct hyperlink bounding box";
            ASSERT_TRUE(m_aURI == ::rtl::OUString::createFromAscii( "http://download.openoffice.org/" )) << "Correct hyperlink URI";

            const char* sText = " \n \nThis is a testtext\nNew paragraph,\nnew line\n"
                "Hyperlink, this is\n?\nThis is more text\noutline mode\n?\nNew paragraph\n";
            ::rtl::OString aTmp;
            m_aTextOut.makeStringAndClear().convertToString( &aTmp,
                                                             RTL_TEXTENCODING_ASCII_US,
                                                             OUSTRING_TO_OSTRING_CVTFLAGS );
            ASSERT_TRUE( sText == aTmp ) << "Imported text is \"This is a testtext New paragraph, new line"
                                    " Hyperlink, this is * This is more text outline mode * New paragraph\"";

            ASSERT_TRUE(m_bRedCircleSeen) << "red circle seen in input";
            ASSERT_TRUE(m_bGreenStrokeSeen) << "green stroke seen in input";
            ASSERT_TRUE(m_bDashedLineSeen) << "dashed line seen in input";
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
            ASSERT_TRUE(start == 0.0) << "line dashing start offset";
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
                ASSERT_TRUE(rContext.LineColor.Alpha == 1.0 &&
                                        rContext.LineColor.Red == 0.0 &&
                                        rContext.LineColor.Green == 1.0 &&
                                        rContext.LineColor.Blue == 0.0) << "Line color is green";

                ASSERT_TRUE(rtl::math::approxEqual(rContext.LineWidth, 28.3)) << "Line width is 0";

                const char* sExportString = "m53570 7650-35430 24100";
                ASSERT_TRUE(basegfx::tools::exportToSvgD( aPath, true, true, false ).compareToAscii(sExportString) == 0) << "Stroke is m535.7 518.5-354.3-241";

                m_bGreenStrokeSeen = true;
            }
            else
            {
                ASSERT_TRUE(rContext.DashArray.size() == 4 &&
                                        rtl::math::approxEqual(rContext.DashArray[0],14.3764) &&
                                        rContext.DashArray[0] == rContext.DashArray[1] &&
                                        rContext.DashArray[1] == rContext.DashArray[2] &&
                                        rContext.DashArray[2] == rContext.DashArray[3]) << "Dash array cons  ists of four entries";

                ASSERT_TRUE(rContext.LineColor.Alpha == 1.0 &&
                                        rContext.LineColor.Red == 0.0 &&
                                        rContext.LineColor.Green == 0.0 &&
                                        rContext.LineColor.Blue == 0.0) << "Line color is black";

                ASSERT_TRUE(rContext.LineWidth == 0) << "Line width is 0";

                const char* sExportString = "m49890 5670.00000000001-35430 24090";
                ASSERT_TRUE(basegfx::tools::exportToSvgD( aPath, true, true, false ).compareToAscii(sExportString) == 0) << "Stroke is m49890 5670.00000000001-35430 24090";

                m_bDashedLineSeen = true;
            }
            ASSERT_TRUE(rContext.BlendMode == rendering::BlendMode::NORMAL) << "Blend mode is normal";
            ASSERT_TRUE(rContext.LineJoin == rendering::PathJoinType::ROUND) << "Join type is round";
            ASSERT_TRUE(rContext.LineCap == rendering::PathCapType::BUTT) << "Cap type is butt";
            ASSERT_TRUE(rContext.MiterLimit == 10) << "Line miter limit is 10";
            ASSERT_TRUE(rContext.Flatness == 1) << "Flatness is 0";
            ASSERT_TRUE(rContext.FontId == 0) << "Font id is 0";
        }

        virtual void fillPath( const uno::Reference<rendering::XPolyPolygon2D>& rPath )
        {
            GraphicsContext& rContext( getCurrentContext() );
            basegfx::B2DPolyPolygon aPath = basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rPath);
            aPath.transform( rContext.Transformation );

            ASSERT_TRUE(rContext.FillColor.Alpha == 1.0 &&
                                    rContext.FillColor.Red == 0.0 &&
                                    rContext.FillColor.Green == 0.0 &&
                                    rContext.FillColor.Blue == 0.0) << "Fill color is black";
            ASSERT_TRUE(rContext.BlendMode == rendering::BlendMode::NORMAL) << "Blend mode is normal";
            ASSERT_TRUE(rContext.Flatness == 10) << "Flatness is 10";
            ASSERT_TRUE(rContext.FontId == 0) << "Font id is 0";
        }

        virtual void eoFillPath( const uno::Reference<rendering::XPolyPolygon2D>& rPath )
        {
            GraphicsContext& rContext( getCurrentContext() );
            basegfx::B2DPolyPolygon aPath = basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rPath);
            aPath.transform( rContext.Transformation );

            ASSERT_TRUE(rContext.FillColor.Alpha == 1.0 &&
                                    rContext.FillColor.Red == 1.0 &&
                                    rContext.FillColor.Green == 0.0 &&
                                    rContext.FillColor.Blue == 0.0) << "Fill color is black";
            ASSERT_TRUE(rContext.BlendMode == rendering::BlendMode::NORMAL) << "Blend mode is normal";
            ASSERT_TRUE(rContext.Flatness == 1) << "Flatness is 0";
            ASSERT_TRUE(rContext.FontId == 0) << "Font id is 0";

            const char* sExportString = "m12050 49610c-4310 0-7800-3490-7800-7800 0-4300 "
                "3490-7790 7800-7790 4300 0 7790 3490 7790 7790 0 4310-3490 7800-7790 7800z";
            ASSERT_TRUE(basegfx::tools::exportToSvgD( aPath, true, true, false ).compareToAscii(sExportString) == 0) << "Stroke is a 4-bezier circle";

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
            m_aTextOut.append( ::rtl::OUString::createFromAscii("\n") );
        }

        virtual void drawMask(const uno::Sequence<beans::PropertyValue>& xBitmap,
                              bool                                       /*bInvert*/ )
        {
            ASSERT_TRUE(xBitmap.getLength()==3) << "drawMask received two properties";
            ASSERT_TRUE(xBitmap[0].Name.compareToAscii( "URL" ) == 0) << "drawMask got URL param";
            ASSERT_TRUE(xBitmap[1].Name.compareToAscii( "InputStream" ) == 0) << "drawMask got InputStream param";
        }

        virtual void drawImage(const uno::Sequence<beans::PropertyValue>& xBitmap )
        {
            ASSERT_TRUE(xBitmap.getLength()==3) << "drawImage received two properties";
            ASSERT_TRUE(xBitmap[0].Name.compareToAscii( "URL" ) == 0) << "drawImage got URL param";
            ASSERT_TRUE(xBitmap[1].Name.compareToAscii( "InputStream" ) == 0) << "drawImage got InputStream param";
        }

        virtual void drawColorMaskedImage(const uno::Sequence<beans::PropertyValue>& xBitmap,
                                          const uno::Sequence<uno::Any>&             /*xMaskColors*/ )
        {
            ASSERT_TRUE(xBitmap.getLength()==3) << "drawColorMaskedImage received two properties";
            ASSERT_TRUE(xBitmap[0].Name.compareToAscii( "URL" ) == 0) << "drawColorMaskedImage got URL param";
            ASSERT_TRUE(xBitmap[1].Name.compareToAscii( "InputStream" ) == 0) << "drawColorMaskedImage got InputStream param";
        }

        virtual void drawMaskedImage(const uno::Sequence<beans::PropertyValue>& xBitmap,
                                     const uno::Sequence<beans::PropertyValue>& xMask,
                                     bool                                       /*bInvertMask*/)
        {
            ASSERT_TRUE(xBitmap.getLength()==3) << "drawMaskedImage received two properties #1";
            ASSERT_TRUE(xBitmap[0].Name.compareToAscii( "URL" ) == 0) << "drawMaskedImage got URL param #1";
            ASSERT_TRUE(xBitmap[1].Name.compareToAscii( "InputStream" ) == 0) << "drawMaskedImage got InputStream param #1";

            ASSERT_TRUE(xMask.getLength()==3) << "drawMaskedImage received two properties #2";
            ASSERT_TRUE(xMask[0].Name.compareToAscii( "URL" ) == 0) << "drawMaskedImage got URL param #2";
            ASSERT_TRUE(xMask[1].Name.compareToAscii( "InputStream" ) == 0) << "drawMaskedImage got InputStream param #2";
        }

        virtual void drawAlphaMaskedImage(const uno::Sequence<beans::PropertyValue>& xBitmap,
                                          const uno::Sequence<beans::PropertyValue>& xMask)
        {
            ASSERT_TRUE(xBitmap.getLength()==3) << "drawAlphaMaskedImage received two properties #1";
            ASSERT_TRUE(xBitmap[0].Name.compareToAscii( "URL" ) == 0) << "drawAlphaMaskedImage got URL param #1";
            ASSERT_TRUE(xBitmap[1].Name.compareToAscii( "InputStream" ) == 0) << "drawAlphaMaskedImage got InputStream param #1";

            ASSERT_TRUE(xMask.getLength()==3) << "drawAlphaMaskedImage received two properties #2";
            ASSERT_TRUE(xMask[0].Name.compareToAscii( "URL" ) == 0) << "drawAlphaMaskedImage got URL param #2";
            ASSERT_TRUE(xMask[1].Name.compareToAscii( "InputStream" ) == 0) << "drawAlphaMaskedImage got InputStream param #2";
        }

        virtual void setTextRenderMode( sal_Int32 )
        {
        }

        typedef std::hash_map<sal_Int32,FontAttributes> IdToFontMap;
        typedef std::hash_map<FontAttributes,sal_Int32,FontAttrHash> FontToIdMap;

        typedef std::hash_map<sal_Int32,GraphicsContext> IdToGCMap;
        typedef std::hash_map<GraphicsContext,sal_Int32,GraphicsContextHash> GCToIdMap;

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

    class PDFITest : public ::testing::Test
    {
    protected:
        uno::Reference<uno::XComponentContext> mxCtx;
        rtl::OUString                          msBaseDir;
        bool                                   mbUnoInitialized;

    public:
        PDFITest() : mxCtx(),msBaseDir(),mbUnoInitialized(false)
        {}

        void SetUp()
        {
            if( !mbUnoInitialized )
            {
                const char* pArgs( getenv("TESTS_FORWARD_STRING") );
                ASSERT_TRUE(pArgs) << "Test file parameter";

                msBaseDir = rtl::OUString::createFromAscii(pArgs);

                // bootstrap UNO
                try
                {
                    ::rtl::OUString aIniUrl;
                    ASSERT_TRUE(
                        osl_getFileURLFromSystemPath(
                            (msBaseDir+rtl::OUString::createFromAscii("pdfi_unittest_test.ini")).pData,
                            &aIniUrl.pData ) == osl_File_E_None )
                        << "Converting ini file to URL";

                    mxCtx = ::cppu::defaultBootstrap_InitialComponentContext(aIniUrl);
                    ASSERT_TRUE(mxCtx.is()) << "Getting component context";
                }
                catch( uno::Exception& )
                {
                    FAIL() << "Bootstrapping UNO";
                }

                mbUnoInitialized = true;
            }
        }
        void TearDown()
        {
        }
    };

    TEST_F(PDFITest, testXPDFParser)
    {
        pdfi::ContentSinkSharedPtr pSink( new TestSink() );
        pdfi::xpdf_ImportFromFile( msBaseDir + rtl::OUString::createFromAscii("pdfi_unittest_test.pdf"),
                                   pSink,
                                   uno::Reference< task::XInteractionHandler >(),
                                   rtl::OUString(),
                                   mxCtx );

        // make destruction explicit, a bunch of things are
        // checked in the destructor
        pSink.reset();
    }

    TEST_F(PDFITest, testOdfDrawExport)
    {
        pdfi::PDFIRawAdaptor aAdaptor( mxCtx );
        aAdaptor.setTreeVisitorFactory( createDrawTreeVisitorFactory() );

        ::rtl::OUString aURL, aAbsURL, aBaseURL;
        osl_getFileURLFromSystemPath( (msBaseDir + rtl::OUString::createFromAscii("pdfi_unittest_draw.xml")).pData,
                                      &aURL.pData );
        osl_getProcessWorkingDir(&aBaseURL.pData);
        osl_getAbsoluteFileURL(aBaseURL.pData,aURL.pData,&aAbsURL.pData);
        ASSERT_TRUE(aAdaptor.odfConvert( msBaseDir + rtl::OUString::createFromAscii("pdfi_unittest_test.pdf"),
                                         new OutputWrap(aAbsURL),
                                         NULL )) << "Exporting to ODF";
    }

    TEST_F(PDFITest, testOdfWriterExport)
    {
        pdfi::PDFIRawAdaptor aAdaptor( mxCtx );
        aAdaptor.setTreeVisitorFactory( createWriterTreeVisitorFactory() );

        ::rtl::OUString aURL, aAbsURL, aBaseURL;
        osl_getFileURLFromSystemPath( (msBaseDir + rtl::OUString::createFromAscii("pdfi_unittest_writer.xml")).pData,
                                      &aURL.pData );
        osl_getProcessWorkingDir(&aBaseURL.pData);
        osl_getAbsoluteFileURL(aBaseURL.pData,aURL.pData,&aAbsURL.pData);
        ASSERT_TRUE(aAdaptor.odfConvert( msBaseDir + rtl::OUString::createFromAscii("pdfi_unittest_test.pdf"),
                                         new OutputWrap(aAbsURL),
                                         NULL )) << "Exporting to ODF";
    }

}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
