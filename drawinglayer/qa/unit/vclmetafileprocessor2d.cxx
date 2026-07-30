/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <test/bootstrapfixture.hxx>

#include <vcl/virdev.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/metaact.hxx>
#include <vcl/metaactiontypes.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <tools/mapunit.hxx>
#include <tools/stream.hxx>
#include <unotools/tempfile.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/PolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <cppcanvas/vclfactory.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <comphelper/string.hxx>

#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>

using namespace drawinglayer;
using namespace com::sun::star;

class VclMetaFileProcessor2DTest : public test::BootstrapFixture
{
    VclPtr<VirtualDevice> mVclDevice;
    uno::Reference<rendering::XCanvas> mCanvas;

    // if enabled - check the result images with:
    // "xdg-open ./workdir/CppunitTest/drawinglayer_processors.test.core/"
    static constexpr const bool mbExportBitmap = false;

    void exportDevice(const OUString& filename, const VclPtr<VirtualDevice>& device)
    {
        if (mbExportBitmap)
        {
            Bitmap aBitmap(device->GetBitmap(Point(0, 0), device->GetOutputSizePixel()));
            SvFileStream aStream(filename, StreamMode::WRITE | StreamMode::TRUNC);
            GraphicFilter::GetGraphicFilter().compressAsPNG(aBitmap, aStream);
        }
    }

public:
    VclMetaFileProcessor2DTest()
        : BootstrapFixture(true, false)
    {
    }

    virtual void tearDown() override
    {
        mVclDevice.reset();
        mCanvas = uno::Reference<rendering::XCanvas>();
        BootstrapFixture::tearDown();
    }

    void setupCanvas(const Size& size, Color backgroundColor = COL_WHITE, bool alpha = false)
    {
        mVclDevice = alpha ? VclPtr<VirtualDevice>::Create(DeviceFormat::WITH_ALPHA)
                           : VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
        mVclDevice->SetOutputSizePixel(size);
        mVclDevice->SetBackground(Wallpaper(backgroundColor));
        mVclDevice->Erase();
        mCanvas = mVclDevice->GetCanvas();
        CPPUNIT_ASSERT(mCanvas.is());
    }

    // Test drawing a dotted line in Impress presentation mode.
    void tdf136957_draw_impress_dotted_line()
    {
        // Impress presentation mode first draws the slide to a metafile.
        GDIMetaFile metafile;
        // I got these values by adding debug output to cppcanvas::internal::ImplRenderer::ImplRenderer().
        metafile.SetPrefMapMode(MapMode(MapUnit::Map100thMM));
        metafile.SetPrefSize(Size(14548, 3350));
        ScopedVclPtrInstance<VirtualDevice> metadevice;
        metafile.Record(metadevice);
        drawinglayer::geometry::ViewInformation2D view;
        std::unique_ptr<processor2d::BaseProcessor2D> processor(
            processor2d::createProcessor2DFromOutputDevice(*metadevice, view));
        CPPUNIT_ASSERT(processor);
        // Match the values Impress uses.
        basegfx::B2DPolygon polygon = { { 15601, 0 }, { 15602, 5832 } };
        attribute::LineAttribute lineAttributes(
            basegfx::BColor(0.047058823529411764, 0.19607843137254902, 0.17254901960784313), 35,
            basegfx::B2DLineJoin::Miter, css::drawing::LineCap_ROUND);
        attribute::StrokeAttribute strokeAttributes({ 0.35, 69.65 });
        rtl::Reference<primitive2d::PolygonStrokePrimitive2D> strokePrimitive(
            new primitive2d::PolygonStrokePrimitive2D(polygon, lineAttributes, strokeAttributes));
        primitive2d::Primitive2DContainer primitives;
        primitives.push_back(strokePrimitive);
        processor->process(primitives);
        metafile.Stop();
        metafile.WindStart();

        // Now verify that the metafile has the one PolyLine action with the right dashing.
        int lineActionCount = 0;
        for (std::size_t i = 0; i < metafile.GetActionSize(); ++i)
        {
            const MetaAction* metaAction = metafile.GetAction(i);
            if (metaAction->GetType() == MetaActionType::POLYLINE)
            {
                const MetaPolyLineAction* action
                    = static_cast<const MetaPolyLineAction*>(metaAction);

                CPPUNIT_ASSERT_EQUAL(35.0, action->GetLineInfo().GetWidth());
                CPPUNIT_ASSERT_EQUAL(LineStyle::Dash, action->GetLineInfo().GetStyle());
                CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), action->GetLineInfo().GetDashCount());
                CPPUNIT_ASSERT_EQUAL(0.35, action->GetLineInfo().GetDashLen());
                CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), action->GetLineInfo().GetDotCount());
                CPPUNIT_ASSERT_EQUAL(0.0, action->GetLineInfo().GetDotLen());
                CPPUNIT_ASSERT_EQUAL(69.65, action->GetLineInfo().GetDistance());
                lineActionCount++;
            }
        }
        CPPUNIT_ASSERT_EQUAL(1, lineActionCount);

        // Now draw the metafile using canvas and verify that the line is drawn.
        setupCanvas(Size(1920, 1080));
        cppcanvas::CanvasSharedPtr cppCanvas = cppcanvas::VCLFactory::createCanvas(mCanvas);
        // I got these matrices from a breakpoint in drawing the polyline, and walking up
        // the stack to the canvas code.
        cppCanvas->setTransformation(
            basegfx::B2DHomMatrix(0.056662828121770453, 0, 0, 0, 0.056640419947506564, 0));
        cppcanvas::RendererSharedPtr renderer = cppcanvas::VCLFactory::createRenderer(
            cppCanvas, metafile, cppcanvas::Renderer::Parameters());
        renderer->setTransformation(basegfx::B2DHomMatrix(14548, 0, -2, 0, 3350, 3431));
        CPPUNIT_ASSERT(renderer->draw());
        exportDevice(u"test-tdf136957"_ustr, mVclDevice);
        Bitmap bitmap = mVclDevice->GetBitmap(Point(), Size(1920, 1080));
        BitmapScopedReadAccess access(bitmap);
        // There should be a dotted line, without the fix it wouldn't be there, so check
        // there's a sufficient amount of non-white pixels and that's the line.
        int nonWhiteCount = 0;
        for (tools::Long y = 193; y <= 524; ++y)
            for (tools::Long x = 883; x <= 885; ++x)
                if (access->GetColor(y, x) != COL_WHITE)
                    ++nonWhiteCount;
        CPPUNIT_ASSERT_GREATER(100, nonWhiteCount);
    }

    // Test that a link's clickable bounding box is correctly transformed
    void tdf169919_link_bounding_box_transform()
    {
        // Impress presentation mode first draws the slide to a metafile.
        GDIMetaFile metafile;
        // I got these values by adding debug output to cppcanvas::internal::ImplRenderer::ImplRenderer().
        metafile.SetPrefMapMode(MapMode(MapUnit::Map100thMM));
        metafile.SetPrefSize(Size(14548, 3350));
        ScopedVclPtrInstance<VirtualDevice> metadevice;

        // Clickable links are only created during PDF export
        vcl::PDFExtOutDevData aPDFExtOutDevData(*metadevice);
        aPDFExtOutDevData.SetIsExportBookmarks(true);
        aPDFExtOutDevData.SetIsExportTaggedPDF(true);
        metadevice->SetExtOutDevData(&aPDFExtOutDevData);

        vcl::pdf::PDFWriter::PDFWriterContext aContext;
        aContext.Version = vcl::pdf::PDFWriter::PDFVersion::PDF_1_7;
        aContext.Tagged = true;
        aContext.PDFDocumentMode = vcl::pdf::PDFWriter::ModeDefault;
        aContext.PDFDocumentAction = vcl::pdf::PDFWriter::ActionDefault;
        aContext.PageLayout = vcl::pdf::PDFWriter::DefaultLayout;

        // Create a temp file to store the written PDF
        utl::TempFileNamed aTempFile;
        aTempFile.EnableKillingFile();
        aContext.URL = aTempFile.GetURL();

        rtl::Reference<beans::XMaterialHolder> xEnc;
        vcl::pdf::PDFWriter aPDFWriter(aContext, xEnc);
        aPDFWriter.NewPage(14548, 3350);

        metafile.Record(metadevice);
        drawinglayer::geometry::ViewInformation2D view;

        // Set a transform to ensure processed link honors transforms
        const basegfx::B2DHomMatrix aMappingTransform(
            basegfx::utils::createTranslateB2DHomMatrix(500, 500));
        view.setObjectTransformation(aMappingTransform);

        std::unique_ptr<processor2d::BaseProcessor2D> processor(
            processor2d::createProcessor2DFromOutputDevice(*metadevice, view));
        CPPUNIT_ASSERT(processor);

        // Create a child primitive2d that holds the bounding box for the link
        drawinglayer::primitive2d::Primitive2DContainer aSeq(1);
        attribute::LineAttribute lineAttributes(
            basegfx::BColor(0.047058823529411764, 0.19607843137254902, 0.17254901960784313), 35,
            basegfx::B2DLineJoin::Miter, css::drawing::LineCap_ROUND);
        basegfx::B2DPolygon aPolygon = { { -10, 65 }, { 539, 368 } };
        aSeq[0] = new drawinglayer::primitive2d::PolygonStrokePrimitive2D(aPolygon, lineAttributes);

        // The primitive2d for the link itself
        std::vector<std::pair<OUString, OUString>> meValues;
#define LINK_URL "http://libreoffice.org"
        meValues.emplace_back("URL", OUString(LINK_URL));
        meValues.emplace_back("AltText", "link");
        rtl::Reference<primitive2d::TextHierarchyFieldPrimitive2D> fieldPrimitive(
            new primitive2d::TextHierarchyFieldPrimitive2D(
                std::move(aSeq), drawinglayer::primitive2d::FIELD_TYPE_URL, &meValues));

        primitive2d::Primitive2DContainer primitives;
        primitives.push_back(fieldPrimitive);

        processor->process(primitives);

        metafile.Stop();
        metafile.WindStart();

        // Match bookmarks with their link URL; usually done by each module's
        // rendering code, eg ScModelObj::render() and such
        std::vector<vcl::PDFExtOutDevBookmarkEntry>& rBookmarks = aPDFExtOutDevData.GetBookmarks();
        CPPUNIT_ASSERT(!rBookmarks.empty());
        for (const auto& rBookmark : rBookmarks)
            aPDFExtOutDevData.SetLinkURL(rBookmark.nLinkId, rBookmark.aBookmark);

        aPDFExtOutDevData.PlayGlobalActions(aPDFWriter);
        CPPUNIT_ASSERT(aPDFWriter.Emit());
        CPPUNIT_ASSERT(aPDFWriter.GetErrors().empty());

        SvStream* pStream = aTempFile.GetStream(StreamMode::READ);
        CPPUNIT_ASSERT(pStream->TellEnd() > 5000);

        bool found = false;
        OString sLine;
        while (pStream->ReadLine(sLine))
        {
            if (sLine.isEmpty())
                continue;

            // Look for a line like
            // <</Type/Annot/Subtype/Link/Border[0 0 0]/Rect[23.593 1630.65 52.907 1647.6]/A<</Type/Action/S/URI/URI(http://libreoffice.org/)>>
            // and parse out the Rect[] bits
            sal_Int32 nLinkIdx = sLine.indexOf(LINK_URL);
            if (nLinkIdx > 0 && sLine.startsWith("<</Type/Annot/Subtype/Link/Border[0 0 0]/Rect["))
            {
#define RECT_START "/Rect["
                sal_Int32 nStartIdx = sLine.indexOf(RECT_START);
                CPPUNIT_ASSERT_EQUAL(sal_Int32(40), nStartIdx);
                nStartIdx += strlen(RECT_START);
                sal_Int32 nEndIdx = sLine.indexOf(']', nStartIdx);
                CPPUNIT_ASSERT_GREATER(nStartIdx + 15, nEndIdx);

                OString aStr(sLine.subView(nStartIdx, nEndIdx - nStartIdx - 1));
                auto aCoords = comphelper::string::split(aStr, ' ');
                CPPUNIT_ASSERT_EQUAL(size_t(4), aCoords.size());

                // Assert that the Link's Rect is within the expected range, which depends
                // on VclMetafileProcessor2D::processTextHierarchyFieldPrimitive2D()
                // correctly doing the view transformation
                CPPUNIT_ASSERT_GREATER(20.0, aCoords[0].toDouble());
                CPPUNIT_ASSERT_LESS(25.0, aCoords[0].toDouble());

                CPPUNIT_ASSERT_GREATER(1628.0, aCoords[1].toDouble());
                CPPUNIT_ASSERT_LESS(1632.0, aCoords[1].toDouble());

                CPPUNIT_ASSERT_GREATER(50.0, aCoords[2].toDouble());
                CPPUNIT_ASSERT_LESS(55.0, aCoords[2].toDouble());

                CPPUNIT_ASSERT_GREATER(1645.0, aCoords[3].toDouble());
                CPPUNIT_ASSERT_LESS(1650.0, aCoords[3].toDouble());

                found = true;
                break;
            }
        }
        CPPUNIT_ASSERT(found);
    }

    CPPUNIT_TEST_SUITE(VclMetaFileProcessor2DTest);
    CPPUNIT_TEST(tdf136957_draw_impress_dotted_line);
    CPPUNIT_TEST(tdf169919_link_bounding_box_transform);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(VclMetaFileProcessor2DTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
