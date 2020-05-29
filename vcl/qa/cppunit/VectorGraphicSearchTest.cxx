/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <unotest/bootstrapfixturebase.hxx>
#include <unotest/directories.hxx>

#include <vcl/VectorGraphicSearch.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <tools/stream.hxx>

class VectorGraphicSearchTest : public test::BootstrapFixtureBase
{
    OUString getFullUrl(const OUString& sFileName)
    {
        return m_directories.getURLFromSrc("/vcl/qa/cppunit/data/") + sFileName;
    }

    void test();
    void testNextPrevious();

    CPPUNIT_TEST_SUITE(VectorGraphicSearchTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST(testNextPrevious);
    CPPUNIT_TEST_SUITE_END();
};

void VectorGraphicSearchTest::test()
{
    OUString aURL = getFullUrl("Pangram.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    aGraphic.makeAvailable();

    VectorGraphicSearch aSearch(aGraphic);
    CPPUNIT_ASSERT_EQUAL(true, aSearch.search("lazy"));
    CPPUNIT_ASSERT_EQUAL(true, aSearch.next());
    CPPUNIT_ASSERT_EQUAL(34, aSearch.index());

    basegfx::B2DSize aSize = aSearch.pageSize();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(21590.00, aSize.getX(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(27940.00, aSize.getY(), 1E-2);

    auto aRectangles = aSearch.getTextRectangles();
    CPPUNIT_ASSERT_EQUAL(size_t(4), aRectangles.size());

    // Check first and last
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8078.61, aRectangles[0].getMinX(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8179.36, aRectangles[0].getMaxX(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2101.56, aRectangles[0].getMinY(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2395.36, aRectangles[0].getMaxY(), 1E-2);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(8565.86, aRectangles[3].getMinX(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8770.76, aRectangles[3].getMaxX(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2201.05, aRectangles[3].getMinY(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2486.37, aRectangles[3].getMaxY(), 1E-2);

    CPPUNIT_ASSERT_EQUAL(true, aSearch.next());
    CPPUNIT_ASSERT_EQUAL(817, aSearch.index());

    aRectangles = aSearch.getTextRectangles();
    CPPUNIT_ASSERT_EQUAL(size_t(4), aRectangles.size());

    // Check first and last
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6562.23, aRectangles[0].getMinX(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6662.98, aRectangles[0].getMaxX(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5996.23, aRectangles[0].getMinY(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6290.02, aRectangles[0].getMaxY(), 1E-2);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(7049.48, aRectangles[3].getMinX(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7254.38, aRectangles[3].getMaxX(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6095.71, aRectangles[3].getMinY(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6381.04, aRectangles[3].getMaxY(), 1E-2);
}

// Test next and previous work as expected to move
// between search matches.
void VectorGraphicSearchTest::testNextPrevious()
{
    OUString aURL = getFullUrl("Pangram.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    aGraphic.makeAvailable();

    VectorGraphicSearch aSearch(aGraphic);
    CPPUNIT_ASSERT_EQUAL(true, aSearch.search("lazy"));

    // next - first match found
    CPPUNIT_ASSERT_EQUAL(true, aSearch.next());
    CPPUNIT_ASSERT_EQUAL(34, aSearch.index());

    // next - second match found
    CPPUNIT_ASSERT_EQUAL(true, aSearch.next());
    CPPUNIT_ASSERT_EQUAL(817, aSearch.index());

    // next - not found, index unchanged
    CPPUNIT_ASSERT_EQUAL(false, aSearch.next());
    CPPUNIT_ASSERT_EQUAL(817, aSearch.index());

    // previous - first match
    CPPUNIT_ASSERT_EQUAL(true, aSearch.previous());
    CPPUNIT_ASSERT_EQUAL(34, aSearch.index());

    // previous - not found, index unchanged
    CPPUNIT_ASSERT_EQUAL(false, aSearch.previous());
    CPPUNIT_ASSERT_EQUAL(34, aSearch.index());

    // next - second match found
    CPPUNIT_ASSERT_EQUAL(true, aSearch.next());
    CPPUNIT_ASSERT_EQUAL(817, aSearch.index());
}

CPPUNIT_TEST_SUITE_REGISTRATION(VectorGraphicSearchTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
