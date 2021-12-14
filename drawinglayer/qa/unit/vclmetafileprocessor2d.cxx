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
#include <vcl/canvastools.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>
#include <tools/stream.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/PolygonStrokePrimitive2D.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <cppcanvas/vclfactory.hxx>

#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>

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
            BitmapEx aBitmapEx(device->GetBitmapEx(Point(0, 0), device->GetOutputSizePixel()));
            SvFileStream aStream(filename, StreamMode::WRITE | StreamMode::TRUNC);
            GraphicFilter::GetGraphicFilter().compressAsPNG(aBitmapEx, aStream);
        }
    }

public:
    VclMetaFileProcessor2DTest()
        : BootstrapFixture(true, false)
    {
    }

    virtual void tearDown() override
    {
        mVclDevice.clear();
        mCanvas = uno::Reference<rendering::XCanvas>();
        BootstrapFixture::tearDown();
    }

    void setupCanvas(const Size& size, Color backgroundColor = COL_WHITE, bool alpha = false)
    {
        mVclDevice
            = alpha ? VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT, DeviceFormat::DEFAULT)
                    : VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
        mVclDevice->SetOutputSizePixel(size);
        mVclDevice->SetBackground(Wallpaper(backgroundColor));
        mVclDevice->Erase();
        mCanvas = mVclDevice->GetCanvas();
        CPPUNIT_ASSERT(mCanvas.is());
    }

    // Test drawing a dotted line in Impress presentation mode.
    void testTdf136957()
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
            processor2d::createBaseProcessor2DFromOutputDevice(*metadevice, view));
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
        primitives.push_back(primitive2d::Primitive2DReference(strokePrimitive));
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
        exportDevice("test-tdf136957", mVclDevice);
        Bitmap bitmap = mVclDevice->GetBitmap(Point(), Size(1920, 1080));
        Bitmap::ScopedReadAccess access(bitmap);
        // There should be a dotted line, without the fix it wouldn't be there, so check
        // there's a sufficient amount of non-white pixels and that's the line.
        int nonWhiteCount = 0;
        for (tools::Long y = 193; y <= 524; ++y)
            for (tools::Long x = 883; x <= 885; ++x)
                if (access->GetColor(y, x) != COL_WHITE)
                    ++nonWhiteCount;
        CPPUNIT_ASSERT_GREATER(100, nonWhiteCount);
    }

    CPPUNIT_TEST_SUITE(VclMetaFileProcessor2DTest);
    CPPUNIT_TEST(testTdf136957);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(VclMetaFileProcessor2DTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
