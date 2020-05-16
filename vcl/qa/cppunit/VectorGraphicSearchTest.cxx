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

    CPPUNIT_TEST_SUITE(VectorGraphicSearchTest);
    CPPUNIT_TEST(test);
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
    auto aRectangles = aSearch.getTextRectangles();
    CPPUNIT_ASSERT_EQUAL(size_t(4), aRectangles.size());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(229.00, aRectangles[0].getMinX(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(231.85, aRectangles[0].getMaxX(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(724.10, aRectangles[0].getMinY(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(732.42, aRectangles[0].getMaxY(), 1E-2);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(232.47, aRectangles[1].getMinX(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(237.22, aRectangles[1].getMaxX(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(723.99, aRectangles[1].getMinY(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(729.72, aRectangles[1].getMaxY(), 1E-2);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(237.68, aRectangles[2].getMinX(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(242.35, aRectangles[2].getMaxX(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(724.09, aRectangles[2].getMinY(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(729.60, aRectangles[2].getMaxY(), 1E-2);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(242.81, aRectangles[3].getMinX(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(248.61, aRectangles[3].getMaxX(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(721.51, aRectangles[3].getMinY(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(729.60, aRectangles[3].getMaxY(), 1E-2);
}

CPPUNIT_TEST_SUITE_REGISTRATION(VectorGraphicSearchTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
