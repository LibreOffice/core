/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <vcl/GraphicObject.hxx>

#include <test/bootstrapfixture.hxx>

#include <tools/stream.hxx>

#include <vcl/image.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <officecfg/Office/Common.hxx>
#include <unotest/macros_test.hxx>
#include <comphelper/processfactory.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include <ndgrf.hxx>

using namespace css;

namespace
{

class GraphicObjectTest: public test::BootstrapFixture, public unotest::MacrosTest
{

public:
    void testSwap();
    void testTdf88836();
    void testPdf();


    virtual void setUp() override
    {
        test::BootstrapFixture::setUp();

        mxDesktop.set(css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
    }

private:
    DECL_LINK(getLinkStream, const GraphicObject*, SvStream*);

private:
    CPPUNIT_TEST_SUITE(GraphicObjectTest);
    CPPUNIT_TEST(testSwap);
    CPPUNIT_TEST(testTdf88836);
    CPPUNIT_TEST(testPdf);
    CPPUNIT_TEST_SUITE_END();
};

static const char aGraphicFile[] = "/svtools/qa/unit/data/graphic.png";
static const sal_uLong nGraphicSizeBytes = 4800;

const Graphic lcl_loadGraphic(const rtl::OUString &rUrl)
{
    const Image aImage(rUrl);
    return Graphic(aImage.GetBitmapEx());
}

IMPL_LINK_NOARG(GraphicObjectTest, getLinkStream, const GraphicObject*, SvStream*)
{
    return GRFMGR_AUTOSWAPSTREAM_LINK;
}

void GraphicObjectTest::testSwap()
{
    // simple non-linked case
    {
        GraphicObject aGraphObj(lcl_loadGraphic(m_directories.getURLFromSrc(aGraphicFile)));
        CPPUNIT_ASSERT(!aGraphObj.HasSwapStreamHdl());
        CPPUNIT_ASSERT(!aGraphObj.IsSwappedOut());
        CPPUNIT_ASSERT_EQUAL(nGraphicSizeBytes, aGraphObj.GetGraphic().GetSizeBytes());
        // swap out
        CPPUNIT_ASSERT(aGraphObj.SwapOut());
        CPPUNIT_ASSERT(aGraphObj.IsSwappedOut());
        // swap in
        CPPUNIT_ASSERT(aGraphObj.SwapIn());
        CPPUNIT_ASSERT(!aGraphObj.IsSwappedOut());
        // the data are still there
        CPPUNIT_ASSERT_EQUAL(nGraphicSizeBytes, aGraphObj.GetGraphic().GetSizeBytes());
    }

    // linked case
    {
        GraphicObject aGraphObj(lcl_loadGraphic(m_directories.getURLFromSrc(aGraphicFile)));
        aGraphObj.SetSwapStreamHdl(LINK(this, GraphicObjectTest, getLinkStream));

        CPPUNIT_ASSERT(aGraphObj.HasSwapStreamHdl());
        CPPUNIT_ASSERT(!aGraphObj.IsSwappedOut());
        CPPUNIT_ASSERT_EQUAL(nGraphicSizeBytes, aGraphObj.GetGraphic().GetSizeBytes());
        // swap out
        CPPUNIT_ASSERT(aGraphObj.SwapOut());
        CPPUNIT_ASSERT(aGraphObj.IsSwappedOut());
        // swap in
        CPPUNIT_ASSERT(aGraphObj.SwapIn());
        CPPUNIT_ASSERT(!aGraphObj.IsSwappedOut());
        // the data are still there
        CPPUNIT_ASSERT_EQUAL(nGraphicSizeBytes, aGraphObj.GetGraphic().GetSizeBytes());
    }

    // combination of two GraphicObjects
    {
        GraphicObject aGraphObj(lcl_loadGraphic(m_directories.getURLFromSrc(aGraphicFile)));

        GraphicObject aGraphObj2(aGraphObj);
        aGraphObj2.SetSwapStreamHdl(LINK(this, GraphicObjectTest, getLinkStream));

        CPPUNIT_ASSERT(!aGraphObj.IsSwappedOut());
        CPPUNIT_ASSERT(!aGraphObj2.IsSwappedOut());
        CPPUNIT_ASSERT_EQUAL(nGraphicSizeBytes, aGraphObj.GetGraphic().GetSizeBytes());
        CPPUNIT_ASSERT_EQUAL(nGraphicSizeBytes, aGraphObj2.GetGraphic().GetSizeBytes());

        // GraphicObjects never share the same Graphic. A new one is created as one step during
        // registration of the GraphicObject at GraphicManager.

        // swap out
        CPPUNIT_ASSERT(aGraphObj.SwapOut());
        CPPUNIT_ASSERT(aGraphObj.IsSwappedOut());
        CPPUNIT_ASSERT(!aGraphObj2.IsSwappedOut());
        CPPUNIT_ASSERT(aGraphObj2.SwapOut());
        CPPUNIT_ASSERT(aGraphObj2.IsSwappedOut());
        // swap in
        CPPUNIT_ASSERT(aGraphObj2.SwapIn());
        CPPUNIT_ASSERT(!aGraphObj2.IsSwappedOut());
        CPPUNIT_ASSERT(aGraphObj.IsSwappedOut());
        CPPUNIT_ASSERT(aGraphObj.SwapIn());
        CPPUNIT_ASSERT(!aGraphObj.IsSwappedOut());
        // the data are still there
        CPPUNIT_ASSERT_EQUAL(nGraphicSizeBytes, aGraphObj.GetGraphic().GetSizeBytes());
        CPPUNIT_ASSERT_EQUAL(nGraphicSizeBytes, aGraphObj2.GetGraphic().GetSizeBytes());
    }
}

void GraphicObjectTest::testTdf88836()
{
    // Construction with empty bitmap -> type should be GraphicType::NONE
    Graphic aGraphic = Bitmap();
    CPPUNIT_ASSERT_EQUAL(int(GraphicType::NONE), int(aGraphic.GetType()));
    aGraphic = Graphic(BitmapEx());
    CPPUNIT_ASSERT_EQUAL(int(GraphicType::NONE), int(aGraphic.GetType()));
}

void GraphicObjectTest::testPdf()
{
#if HAVE_FEATURE_PDFIUM
    uno::Reference<lang::XComponent> xComponent = loadFromDesktop(m_directories.getURLFromSrc("svtools/qa/unit/data/pdf.odt"), "com.sun.star.text.TextDocument");
    SwXTextDocument* pTxtDoc = dynamic_cast<SwXTextDocument*>(xComponent.get());
    CPPUNIT_ASSERT(pTxtDoc);
    SwDoc* pDoc = pTxtDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);
    SwNodes& aNodes = pDoc->GetNodes();

    // Find images
    GraphicObject* pGraphicObject = nullptr;
    for( sal_uLong nIndex = 0; nIndex < aNodes.Count(); ++nIndex)
    {
        if( aNodes[nIndex]->IsGrfNode() )
        {
            SwGrfNode* pGrfNode = aNodes[nIndex]->GetGrfNode();
            CPPUNIT_ASSERT(pGrfNode);
            pGraphicObject = const_cast<GraphicObject*>(&pGrfNode->GetGrfObj());
            break;
        }
    }
    CPPUNIT_ASSERT_MESSAGE("Missing image", pGraphicObject);

    CPPUNIT_ASSERT(pGraphicObject->GetGraphic().getPdfData().hasElements());
    CPPUNIT_ASSERT(pGraphicObject->SwapOut());
    CPPUNIT_ASSERT(pGraphicObject->SwapIn());
    // This failed, swap out + swap in lost the PDF data.
    CPPUNIT_ASSERT(pGraphicObject->GetGraphic().getPdfData().hasElements());

    xComponent->dispose();
#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(GraphicObjectTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
