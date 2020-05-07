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
}

CPPUNIT_TEST_SUITE_REGISTRATION(VectorGraphicSearchTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
