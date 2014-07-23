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

namespace
{

class GraphicObjectTest: public test::BootstrapFixture
{

public:
    void testSwap();

private:
    DECL_LINK(getLinkStream, GraphicObject*);

private:
    CPPUNIT_TEST_SUITE(GraphicObjectTest);
    CPPUNIT_TEST(testSwap);
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

CPPUNIT_TEST_SUITE_REGISTRATION(GraphicObjectTest);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
