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
#include "cppunit/plugin/TestPlugIn.h"

#include <svtools/grfmgr.hxx>

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
#include <boost/shared_ptr.hpp>

using namespace css;

namespace
{

class GraphicObjectTest: public test::BootstrapFixture, public unotest::MacrosTest
{

public:
    void testSwap();
    void testSizeBasedAutoSwap();
    void testTdf88836();


    virtual void setUp() SAL_OVERRIDE
    {
        test::BootstrapFixture::setUp();

        mxDesktop.set(css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
    }

private:
    DECL_LINK(getLinkStream, GraphicObject*);

private:
    CPPUNIT_TEST_SUITE(GraphicObjectTest);
    CPPUNIT_TEST(testSwap);
    CPPUNIT_TEST(testSizeBasedAutoSwap);
    CPPUNIT_TEST(testTdf88836);
    CPPUNIT_TEST_SUITE_END();
};

static const char aGraphicFile[] = "/svtools/qa/unit/data/graphic.png";
static const sal_uLong nGraphicSizeBytes = 4800;

const Graphic lcl_loadGraphic(const rtl::OUString &rUrl)
{
    const Image aImage(rUrl);
    return Graphic(aImage.GetBitmapEx());
}

IMPL_LINK(GraphicObjectTest, getLinkStream, GraphicObject*, /*pGraphObj*/)
{
    return reinterpret_cast<sal_IntPtr>(GRFMGR_AUTOSWAPSTREAM_LINK);
}

void GraphicObjectTest::testSwap()
{
    // simple non-linked case
    {
        GraphicObject aGraphObj(lcl_loadGraphic(getURLFromSrc(aGraphicFile)));
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
        GraphicObject aGraphObj(lcl_loadGraphic(getURLFromSrc(aGraphicFile)));
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
        GraphicObject aGraphObj(lcl_loadGraphic(getURLFromSrc(aGraphicFile)));

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

void GraphicObjectTest::testSizeBasedAutoSwap()
{
    // Set cache size to a very small value to check what happens
    {
        boost::shared_ptr< comphelper::ConfigurationChanges > aBatch(comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Cache::GraphicManager::TotalCacheSize::set(sal_Int32(1), aBatch);
        aBatch->commit();
    }

    uno::Reference< lang::XComponent > xComponent =
        loadFromDesktop(getURLFromSrc("svtools/qa/unit/data/document_with_two_images.odt"), "com.sun.star.text.TextDocument");

    SwXTextDocument* pTxtDoc = dynamic_cast<SwXTextDocument *>(xComponent.get());
    CPPUNIT_ASSERT(pTxtDoc);
    SwDoc* pDoc = pTxtDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);
    SwNodes& aNodes = pDoc->GetNodes();

    // Find images
    const GraphicObject* pGrafObj1 = 0;
    const GraphicObject* pGrafObj2 = 0;
    for( sal_uLong nIndex = 0; nIndex < aNodes.Count(); ++nIndex)
    {
        if( aNodes[nIndex]->IsGrfNode() )
        {
            SwGrfNode* pGrfNode = aNodes[nIndex]->GetGrfNode();
            if( !pGrafObj1 )
            {
                pGrafObj1 = &pGrfNode->GetGrfObj();
            }
            else
            {
                pGrafObj2 = &pGrfNode->GetGrfObj();
            }
        }
    }
    CPPUNIT_ASSERT_MESSAGE("Missing image", pGrafObj1 != 0 && pGrafObj2 != 0);

    {
        // First image should be swapped out
        CPPUNIT_ASSERT(pGrafObj1->IsSwappedOut());
        CPPUNIT_ASSERT_EQUAL(sal_uLong(697230), pGrafObj1->GetSizeBytes());

        // Still swapped out: size is cached
        CPPUNIT_ASSERT(pGrafObj1->IsSwappedOut());
    }

    {
        // Second image should be in the memory
        // Size based swap out is triggered by swap in, so the last swapped in image should be
        // in the memory despite of size limit is reached.
        CPPUNIT_ASSERT(!pGrafObj2->IsSwappedOut());
        CPPUNIT_ASSERT_EQUAL(sal_uLong(1620000), pGrafObj2->GetSizeBytes());
    }

    // Swap in first image -> second image will be swapped out
    {
        pGrafObj1->GetGraphic(); // GetGraphic calls swap in on a const object
        CPPUNIT_ASSERT(!pGrafObj1->IsSwappedOut());
        CPPUNIT_ASSERT(pGrafObj2->IsSwappedOut());
    }

    // Swap in second image -> first image will be swapped out
    {
        pGrafObj2->GetGraphic(); // GetGraphic calls swap in on a const object
        CPPUNIT_ASSERT(!pGrafObj2->IsSwappedOut());
        CPPUNIT_ASSERT(pGrafObj1->IsSwappedOut());
    }

    // Use bigger cache
    {
        GraphicManager& rGrfMgr = pGrafObj1->GetGraphicManager();
        rGrfMgr.SetMaxCacheSize((pGrafObj1->GetSizeBytes()+pGrafObj2->GetSizeBytes())*10);
    }
    // Swap in both images -> both should be swapped in
    {
        pGrafObj1->GetGraphic();
        pGrafObj2->GetGraphic();
        CPPUNIT_ASSERT(!pGrafObj1->IsSwappedOut());
        CPPUNIT_ASSERT(!pGrafObj2->IsSwappedOut());
    }

    xComponent->dispose();
}

void GraphicObjectTest::testTdf88836()
{
    // Construction with empty bitmap -> type should be GRAPHIC_NONE
    Graphic aGraphic = Bitmap();
    CPPUNIT_ASSERT_EQUAL(GRAPHIC_NONE, aGraphic.GetType());
    aGraphic = Graphic(BitmapEx());
    CPPUNIT_ASSERT_EQUAL(GRAPHIC_NONE, aGraphic.GetType());
}

CPPUNIT_TEST_SUITE_REGISTRATION(GraphicObjectTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
