/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>
#include "document.hxx"
#include "docsh.hxx"

#include "rangelst.hxx"

class Test : public test::BootstrapFixture {

public:
    virtual void setUp();
    virtual void tearDown();

    void testDeleteArea_4Ranges();
    void testDeleteArea_2Ranges();
    void testDeleteArea_0Ranges();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testDeleteArea_4Ranges);
    CPPUNIT_TEST(testDeleteArea_2Ranges);
    CPPUNIT_TEST(testDeleteArea_0Ranges);
    CPPUNIT_TEST_SUITE_END();


private:
    ScDocument *m_pDoc;
    ScDocShellRef m_xDocShRef;
};


void Test::setUp()
{
    BootstrapFixture::setUp();

    ScDLL::Init();
    m_xDocShRef = new ScDocShell(
        SFXMODEL_STANDARD |
        SFXMODEL_DISABLE_EMBEDDED_SCRIPTS |
        SFXMODEL_DISABLE_DOCUMENT_RECOVERY);

    m_pDoc = m_xDocShRef->GetDocument();
}

void Test::tearDown()
{
    m_xDocShRef.Clear();
    BootstrapFixture::tearDown();
}

void Test::testDeleteArea_4Ranges()
{
    ScRangeList aList(ScRange(0,0,0,5,5,0));
    aList.DeleteArea(2,2,0,3,3,0);

    CPPUNIT_ASSERT_EQUAL(aList.size(), static_cast<size_t>(4));
    for(SCCOL nCol = 0; nCol <= 5; ++nCol)
    {
        for(SCROW nRow = 0; nRow <= 5; ++nRow)
        {
            if((nCol == 2 || nCol == 3) && ( nRow == 2 || nRow == 3))
                CPPUNIT_ASSERT(!aList.Intersects(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.Intersects(ScRange(nCol, nRow, 0)));
        }
    }
}

void Test::testDeleteArea_2Ranges()
{
    ScRangeList aList(ScRange(0,0,0,5,5,5));
    ScRangeList aList2(aList);

    aList.DeleteArea(4,4,0,6,7,0);
    aList2.DeleteArea(4,4,0,6,7,0);
    CPPUNIT_ASSERT_EQUAL(aList.size(), static_cast<size_t>(2));
    CPPUNIT_ASSERT_EQUAL(aList2.size(), static_cast<size_t>(2));

    for(SCCOL nCol = 0; nCol <= 5; ++nCol)
    {
        for(SCROW nRow = 0; nRow <= 5; ++nRow)
        {
            if(nCol>=4 && nRow >= 4)
                CPPUNIT_ASSERT(!aList.Intersects(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.Intersects(ScRange(nCol, nRow, 0)));
        }
    }
}

void Test::testDeleteArea_0Ranges()
{
    ScRangeList aList(ScRange(1,1,0,3,3,0));
    aList.DeleteArea(1,1,0,3,3,0);

    CPPUNIT_ASSERT(aList.empty());

    ScRangeList aList2(ScRange(1,1,0,3,3,0));
    aList2.DeleteArea(0,0,0,4,4,0);

    CPPUNIT_ASSERT(aList.empty());
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
