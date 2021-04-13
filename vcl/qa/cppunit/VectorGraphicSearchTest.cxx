/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

#include <string_view>

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
    OUString getFullUrl(std::u16string_view sFileName)
    {
        return m_directories.getURLFromSrc(u"/vcl/qa/cppunit/data/") + sFileName;
    }

    void test();
    void testNextPrevious();
    void testSearchStringChange();
    void testSearchMatchWholeWord();
    void testSearchMatchCase();

    CPPUNIT_TEST_SUITE(VectorGraphicSearchTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST(testNextPrevious);
    CPPUNIT_TEST(testSearchStringChange);
    CPPUNIT_TEST(testSearchMatchWholeWord);
    CPPUNIT_TEST(testSearchMatchCase);
    CPPUNIT_TEST_SUITE_END();
};

void VectorGraphicSearchTest::test()
{
    OUString aURL = getFullUrl(u"Pangram.pdf");
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
    OUString aURL = getFullUrl(u"Pangram.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    aGraphic.makeAvailable();

    { // Start from the beginning of the page
        VectorGraphicSearch aSearch(aGraphic);
        CPPUNIT_ASSERT_EQUAL(true, aSearch.search("lazy"));

        // no previous - we are at the begin
        CPPUNIT_ASSERT_EQUAL(false, aSearch.previous());
        CPPUNIT_ASSERT_EQUAL(0, aSearch.index()); // nothing was yet found, so it is 0

        // next - first position found
        CPPUNIT_ASSERT_EQUAL(true, aSearch.next());
        CPPUNIT_ASSERT_EQUAL(34, aSearch.index());

        // next - second position found
        CPPUNIT_ASSERT_EQUAL(true, aSearch.next());
        CPPUNIT_ASSERT_EQUAL(817, aSearch.index());

        // next - not found, index unchanged
        CPPUNIT_ASSERT_EQUAL(false, aSearch.next());
        CPPUNIT_ASSERT_EQUAL(817, aSearch.index());

        // previous - first position
        CPPUNIT_ASSERT_EQUAL(true, aSearch.previous());
        CPPUNIT_ASSERT_EQUAL(34, aSearch.index());

        // previous - not found, index unchanged
        CPPUNIT_ASSERT_EQUAL(false, aSearch.previous());
        CPPUNIT_ASSERT_EQUAL(34, aSearch.index());

        // next - second position found
        CPPUNIT_ASSERT_EQUAL(true, aSearch.next());
        CPPUNIT_ASSERT_EQUAL(817, aSearch.index());
    }

    { // Start from the end of the page
        VectorGraphicSearch aSearch(aGraphic);
        CPPUNIT_ASSERT_EQUAL(true,
                             aSearch.search("lazy", { SearchStartPosition::End, false, false }));

        // no next - we are at the end
        CPPUNIT_ASSERT_EQUAL(false, aSearch.next());
        CPPUNIT_ASSERT_EQUAL(0, aSearch.index()); // nothing was yet found, so it is 0

        // previous - second position found
        CPPUNIT_ASSERT_EQUAL(true, aSearch.previous());
        CPPUNIT_ASSERT_EQUAL(817, aSearch.index());

        // previous - first position found
        CPPUNIT_ASSERT_EQUAL(true, aSearch.previous());
        CPPUNIT_ASSERT_EQUAL(34, aSearch.index());

        // previous - not found, index unchanged
        CPPUNIT_ASSERT_EQUAL(false, aSearch.previous());
        CPPUNIT_ASSERT_EQUAL(34, aSearch.index());

        // next - second position
        CPPUNIT_ASSERT_EQUAL(true, aSearch.next());
        CPPUNIT_ASSERT_EQUAL(817, aSearch.index());

        // next - not found, index unchanged
        CPPUNIT_ASSERT_EQUAL(false, aSearch.next());
        CPPUNIT_ASSERT_EQUAL(817, aSearch.index());

        // previous - first match found
        CPPUNIT_ASSERT_EQUAL(true, aSearch.previous());
        CPPUNIT_ASSERT_EQUAL(34, aSearch.index());
    }
}

void VectorGraphicSearchTest::testSearchStringChange()
{
    OUString aURL = getFullUrl(u"Pangram.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    aGraphic.makeAvailable();

    VectorGraphicSearch aSearch(aGraphic);

    // Set search to "lazy"
    CPPUNIT_ASSERT_EQUAL(true, aSearch.search("lazy"));

    CPPUNIT_ASSERT_EQUAL(true, aSearch.next());
    CPPUNIT_ASSERT_EQUAL(34, aSearch.index());

    CPPUNIT_ASSERT_EQUAL(true, aSearch.next());
    CPPUNIT_ASSERT_EQUAL(817, aSearch.index());

    // Change search to "fox"
    CPPUNIT_ASSERT_EQUAL(true, aSearch.search("fox"));

    CPPUNIT_ASSERT_EQUAL(true, aSearch.next());
    CPPUNIT_ASSERT_EQUAL(822, aSearch.index());

    // Change search to "Quick"
    CPPUNIT_ASSERT_EQUAL(true, aSearch.search("Quick"));
    CPPUNIT_ASSERT_EQUAL(true, aSearch.previous());
    CPPUNIT_ASSERT_EQUAL(784, aSearch.index());
}

void VectorGraphicSearchTest::testSearchMatchWholeWord()
{
    OUString aURL = getFullUrl(u"Pangram.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    aGraphic.makeAvailable();

    {
        VectorGraphicSearch aSearch(aGraphic);
        // Search, whole word disabled - "Flummoxed" - found
        CPPUNIT_ASSERT_EQUAL(true, aSearch.search("Flummoxed"));
        CPPUNIT_ASSERT_EQUAL(true, aSearch.next());
        CPPUNIT_ASSERT_EQUAL(618, aSearch.index());
    }
    {
        VectorGraphicSearch aSearch(aGraphic);
        // Search, whole word disabled - "Flummo" - found
        CPPUNIT_ASSERT_EQUAL(true, aSearch.search("Flummo"));
        CPPUNIT_ASSERT_EQUAL(true, aSearch.next());
        CPPUNIT_ASSERT_EQUAL(618, aSearch.index());
    }
    {
        VectorGraphicSearch aSearch(aGraphic);
        // Search, whole word enabled - "Flummoxed" - found
        CPPUNIT_ASSERT_EQUAL(
            true, aSearch.search("Flummoxed", { SearchStartPosition::Begin, false, true }));
        CPPUNIT_ASSERT_EQUAL(true, aSearch.next());
        CPPUNIT_ASSERT_EQUAL(618, aSearch.index());
    }
    {
        VectorGraphicSearch aSearch(aGraphic);
        // Search, whole word enabled - "Flummo" - not found
        CPPUNIT_ASSERT_EQUAL(true,
                             aSearch.search("Flummo", { SearchStartPosition::Begin, false, true }));
        CPPUNIT_ASSERT_EQUAL(false, aSearch.next());
    }
}

void VectorGraphicSearchTest::testSearchMatchCase()
{
    OUString aURL = getFullUrl(u"Pangram.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    aGraphic.makeAvailable();

    {
        VectorGraphicSearch aSearch(aGraphic);
        // Search "Flummoxed" - case insensitive - found
        CPPUNIT_ASSERT_EQUAL(true, aSearch.search("Flummoxed"));
        CPPUNIT_ASSERT_EQUAL(true, aSearch.next());
        CPPUNIT_ASSERT_EQUAL(618, aSearch.index());
    }

    {
        VectorGraphicSearch aSearch(aGraphic);
        // Search "FLUMMOXED" - case insensitive - found
        CPPUNIT_ASSERT_EQUAL(true, aSearch.search("FLUMMOXED"));
        CPPUNIT_ASSERT_EQUAL(true, aSearch.next());
        CPPUNIT_ASSERT_EQUAL(618, aSearch.index());
    }

    {
        VectorGraphicSearch aSearch(aGraphic);
        // Search "Flummoxed" - case sensitive - found
        CPPUNIT_ASSERT_EQUAL(
            true, aSearch.search("Flummoxed", { SearchStartPosition::Begin, true, false }));
        CPPUNIT_ASSERT_EQUAL(true, aSearch.next());
        CPPUNIT_ASSERT_EQUAL(618, aSearch.index());
    }

    {
        VectorGraphicSearch aSearch(aGraphic);
        // Search to "FLUMMOXED" - case sensitive - not found
        CPPUNIT_ASSERT_EQUAL(
            true, aSearch.search("FLUMMOXED", { SearchStartPosition::Begin, true, false }));
        CPPUNIT_ASSERT_EQUAL(false, aSearch.next());
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(VectorGraphicSearchTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
