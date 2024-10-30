/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_cairo_canvas.h>

#include <sal/config.h>

#include <vcl/wrkwin.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/virdev.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>

#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>

#include <cppcanvas/vclfactory.hxx>

using namespace ::com::sun::star;

static std::ostream& operator<<(std::ostream& rStream, const KernArray& rArray)
{
    if (rArray.empty())
    {
        rStream << "{ }";
        return rStream;
    }

    rStream << "{ ";
    for (size_t i = 0; i < rArray.size() - 1; i++)
        rStream << rArray[i] << ", ";
    rStream << rArray.back();
    rStream << " }";
    return rStream;
}

static std::ostream& operator<<(std::ostream& rStream, const std::vector<sal_Bool>& rVec)
{
    if (rVec.empty())
    {
        rStream << "{ }";
        return rStream;
    }

    rStream << "{ ";
    for (size_t i = 0; i < rVec.size() - 1; i++)
        rStream << std::boolalpha << bool(rVec[i]) << ", ";
    rStream << std::boolalpha << bool(rVec.back());
    rStream << " }";
    return rStream;
}

#include <test/bootstrapfixture.hxx>

class CanvasTest : public test::BootstrapFixture
{
public:
    CanvasTest()
        : BootstrapFixture(true, false)
    {
    }
};

CPPUNIT_TEST_FIXTURE(CanvasTest, testComposite)
{
    ScopedVclPtrInstance<WorkWindow> pWin(nullptr, WB_STDWORK);

    uno::Reference<rendering::XCanvas> xCanvas = pWin->GetOutDev()->GetCanvas();
    CPPUNIT_ASSERT(xCanvas.is());

    // a huge canvas ...
    Size aSize(1, 1);
    uno::Reference<rendering::XBitmap> xBitmap = xCanvas->getDevice()->createCompatibleAlphaBitmap(
        vcl::unotools::integerSize2DFromSize(aSize));
    CPPUNIT_ASSERT(xBitmap.is());

    uno::Reference<rendering::XBitmapCanvas> xBitmapCanvas(xBitmap, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xBitmapCanvas.is());

    BitmapEx aBitmapEx;
    {
        // clear the canvas and basic sanity check ...
        xBitmapCanvas->clear();
        CPPUNIT_ASSERT(aBitmapEx.Create(xBitmapCanvas, aSize));
        CPPUNIT_ASSERT(aBitmapEx.IsAlpha());
        CPPUNIT_ASSERT(!aBitmapEx.GetAlphaMask().IsEmpty());
    }

    {
        // render something
        rendering::RenderState aDefaultState;
        uno::Sequence<double> aRedTransparent{ 1.0, // R
                                               0.0, // G
                                               0.0, // B
                                               0.5 }; // A
        aDefaultState.DeviceColor = aRedTransparent;
#if 0
        // words fail me to describe the sheer beauty of allocating a UNO
        // object to represent a polygon, and manually handling the ViewState
        // and there being no public helper for this - to render ... a rectangle.
        XCachedPrimitive    fillPolyPolygon( [in] XPolyPolygon2D xPolyPolygon, [in] ViewState aViewState, [in] RenderState aRenderState )
#endif
    }
}

CPPUNIT_TEST_FIXTURE(CanvasTest, testTdf155810)
{
    GDIMetaFile aInputMetaFile, aOutputMetaFile;
    KernArray aDXArray;
    std::vector<sal_Bool> aKashidaArray;

    // First create a meta file with a text array action that has Kashida adjustments.
    {
        ScopedVclPtrInstance<VirtualDevice> pDev;

        vcl::Font aFont(u"Noto Naskh Arabic"_ustr, u"Regular"_ustr, Size(0, 72));
        pDev->SetFont(aFont);

        aInputMetaFile.Record(pDev.get());

        OUString aText(u"خالد"_ustr);
        pDev->GetTextArray(aText, &aDXArray);

        auto nKashida = 200;
        aDXArray[0] += nKashida;
        aDXArray[2] += nKashida;
        aKashidaArray = { true, false, true, false };

        pDev->DrawTextArray(Point(0, 0), aText, aDXArray, aKashidaArray, 0, -1);

        aInputMetaFile.Stop();
    }

    // Then draw it using canvas
    {
        ScopedVclPtrInstance<VirtualDevice> pDev;

        aOutputMetaFile.Record(pDev.get());

        auto xCanvas = pDev->GetCanvas();
        CPPUNIT_ASSERT(xCanvas.is());
        auto pCanvas = cppcanvas::VCLFactory::createCanvas(xCanvas);

        auto pRenderer = cppcanvas::VCLFactory::createRenderer(pCanvas, aInputMetaFile, {});
        pRenderer->draw();

        aOutputMetaFile.Stop();
    }

    // Then check that the text array drawn by the canvas renderer didn’t loose
    // the Kashida insertion positions.
    {
        auto pInputAction = aInputMetaFile.GetAction(aInputMetaFile.GetActionSize() - 1);
        auto pOutputAction = aOutputMetaFile.GetAction(aOutputMetaFile.GetActionSize() - 2);

        CPPUNIT_ASSERT_EQUAL(MetaActionType::TEXTARRAY, pInputAction->GetType());
        CPPUNIT_ASSERT_EQUAL(pInputAction->GetType(), pOutputAction->GetType());

        MetaTextArrayAction* pInputTextAction = static_cast<MetaTextArrayAction*>(pInputAction);
        MetaTextArrayAction* pOutputTextAction = static_cast<MetaTextArrayAction*>(pOutputAction);

        CPPUNIT_ASSERT_EQUAL(pInputTextAction->GetDXArray(), aDXArray);
        CPPUNIT_ASSERT_EQUAL(pInputTextAction->GetDXArray(), pOutputTextAction->GetDXArray());

        CPPUNIT_ASSERT_EQUAL(pInputTextAction->GetKashidaArray(), aKashidaArray);
        CPPUNIT_ASSERT_EQUAL(pInputTextAction->GetKashidaArray(),
                             pOutputTextAction->GetKashidaArray());
    }

    // Now, test drawSubset
    {
        ScopedVclPtrInstance<VirtualDevice> pDev;

        aOutputMetaFile.Clear();
        aOutputMetaFile.Record(pDev.get());

        auto xCanvas = pDev->GetCanvas();
        CPPUNIT_ASSERT(xCanvas.is());
        auto pCanvas = cppcanvas::VCLFactory::createCanvas(xCanvas);

        auto pRenderer = cppcanvas::VCLFactory::createRenderer(pCanvas, aInputMetaFile, {});
        pRenderer->drawSubset(1, 3);

        aOutputMetaFile.Stop();
    }

    {
        auto pOutputAction = aOutputMetaFile.GetAction(aOutputMetaFile.GetActionSize() - 2);

        CPPUNIT_ASSERT_EQUAL(MetaActionType::TEXTARRAY, pOutputAction->GetType());

        MetaTextArrayAction* pOutputTextAction = static_cast<MetaTextArrayAction*>(pOutputAction);
        std::vector<sal_Bool> aSubsetKashidaArray({ false, true });

        CPPUNIT_ASSERT_EQUAL(aSubsetKashidaArray, pOutputTextAction->GetKashidaArray());
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
