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
#include "helper/qahelper.hxx"
#include "document.hxx"
#include "docsh.hxx"

#include "rangelst.hxx"

class Test : public test::BootstrapFixture
{
public:
    Test()
        : m_pDoc(nullptr)
    {
    }

    virtual void setUp() override;
    virtual void tearDown() override;

    void testDeleteArea_4Ranges();
    void testDeleteArea_3Ranges();
    void testDeleteArea_3Ranges_Case2();
    void testDeleteArea_3Ranges_Case3();
    void testDeleteArea_3Ranges_Case4();
    void testDeleteArea_3Ranges_Case5();
    void testDeleteArea_2Ranges();
    void testDeleteArea_2Ranges_Case2();
    void testDeleteArea_2Ranges_Case3();
    void testDeleteArea_2Ranges_Case4();
    void testDeleteArea_2Ranges_Case5();
    void testDeleteArea_2Ranges_Case6();
    void testDeleteArea_2Ranges_Case7();
    void testDeleteArea_2Ranges_Case8();
    void testDeleteArea_1Range();
    void testDeleteArea_0Ranges();
    void testJoin_Case1();
    void testJoin_Case2();
    void testGetIntersectedRange();

    void testUpdateReference_DeleteRow();
    void testUpdateReference_DeleteLastRow();
    void testUpdateReference_DeleteCol();

    void testInsertRow();
    void testInsertCol();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testDeleteArea_4Ranges);
    CPPUNIT_TEST(testDeleteArea_3Ranges);
    CPPUNIT_TEST(testDeleteArea_3Ranges_Case2);
    CPPUNIT_TEST(testDeleteArea_3Ranges_Case3);
    CPPUNIT_TEST(testDeleteArea_3Ranges_Case4);
    CPPUNIT_TEST(testDeleteArea_3Ranges_Case5);
    CPPUNIT_TEST(testDeleteArea_2Ranges);
    CPPUNIT_TEST(testDeleteArea_2Ranges_Case2);
    CPPUNIT_TEST(testDeleteArea_2Ranges_Case3);
    CPPUNIT_TEST(testDeleteArea_2Ranges_Case4);
    CPPUNIT_TEST(testDeleteArea_2Ranges_Case5);
    CPPUNIT_TEST(testDeleteArea_2Ranges_Case6);
    CPPUNIT_TEST(testDeleteArea_2Ranges_Case7);
    CPPUNIT_TEST(testDeleteArea_2Ranges_Case8);
    CPPUNIT_TEST(testDeleteArea_1Range);
    CPPUNIT_TEST(testDeleteArea_0Ranges);
    CPPUNIT_TEST(testJoin_Case1);
    CPPUNIT_TEST(testJoin_Case2);
    CPPUNIT_TEST(testUpdateReference_DeleteRow);
    CPPUNIT_TEST(testUpdateReference_DeleteLastRow);
    CPPUNIT_TEST(testUpdateReference_DeleteCol);
    CPPUNIT_TEST(testGetIntersectedRange);
    CPPUNIT_TEST(testInsertRow);
    CPPUNIT_TEST(testInsertCol);
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
        SfxModelFlags::EMBEDDED_OBJECT |
        SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS |
        SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);

    m_pDoc = &m_xDocShRef->GetDocument();
}

void Test::tearDown()
{
    m_xDocShRef.clear();
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
                CPPUNIT_ASSERT(!aList.In(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.In(ScRange(nCol, nRow, 0)));
        }
    }
}

void Test::testDeleteArea_3Ranges()
{
    ScRangeList aList(ScRange(1,1,0,6,6,0));
    aList.DeleteArea(3,3,0,8,4,0);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aList.size());
    for(SCCOL nCol = 1; nCol <= 6; ++nCol)
    {
        for(SCROW nRow = 1; nRow <= 6; ++nRow)
        {
            if((nRow == 3 || nRow == 4) && (nCol >= 3))
                CPPUNIT_ASSERT(!aList.In(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.In(ScRange(nCol, nRow, 0)));
        }
    }

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(28), aList.GetCellCount());
}

void Test::testDeleteArea_3Ranges_Case2()
{
    ScRangeList aList(ScRange(1,1,0,6,6,0));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aList.size());
    aList.DeleteArea(0,2,0,2,4,0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aList.size());

    // Column 1-2 && Row 2-4 should not be in the range list. The rest should
    // be in the list.
    for (SCCOL nCol = 1; nCol <= 6; ++nCol)
    {
        for (SCROW nRow = 1; nRow <= 6; ++nRow)
        {
            if ((1 <= nCol && nCol <= 2) && (2 <= nRow && nRow <= 4))
                CPPUNIT_ASSERT(!aList.In(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.In(ScRange(nCol, nRow, 0)));
        }
    }
}

void Test::testDeleteArea_3Ranges_Case3()
{
    ScRangeList aList(ScRange(1,5,0,6,11,0));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aList.size());
    aList.DeleteArea(3,2,0,4,8,0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aList.size());

    // Column 3-4 && Row 5-8 should not be in the range list.
    for (SCCOL nCol = 1; nCol <= 6; ++nCol)
    {
        for (SCROW nRow = 5; nRow <= 11; ++nRow)
        {
            if ((3 <= nCol && nCol <= 4) && (5 <= nRow && nRow <= 8))
                CPPUNIT_ASSERT(!aList.In(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.In(ScRange(nCol, nRow, 0)));
        }
    }
}

void Test::testDeleteArea_3Ranges_Case4()
{
    ScRangeList aList(ScRange(1,5,0,6,11,0));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aList.size());
    aList.DeleteArea(3,5,0,4,5,0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aList.size());

    // Column 3-4 && Row 5 should not be in the range list.
    for (SCCOL nCol = 1; nCol <= 6; ++nCol)
    {
        for (SCROW nRow = 5; nRow <= 11; ++nRow)
        {
            if ((3 <= nCol && nCol <= 4) && 5 == nRow )
                CPPUNIT_ASSERT(!aList.In(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.In(ScRange(nCol, nRow, 0)));
        }
    }
}

void Test::testDeleteArea_3Ranges_Case5()
{
    ScRangeList aList(ScRange(1,5,0,6,11,0));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aList.size());
    aList.DeleteArea(6,7,0,6,9,0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aList.size());

    // Column 6 && Row 7-9 should not be in the range list.
    for (SCCOL nCol = 1; nCol <= 6; ++nCol)
    {
        for (SCROW nRow = 5; nRow <= 11; ++nRow)
        {
            if ( nCol == 6 && (7 <= nRow && nRow <= 9))
                CPPUNIT_ASSERT(!aList.In(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.In(ScRange(nCol, nRow, 0)));
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
                CPPUNIT_ASSERT(!aList.In(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.In(ScRange(nCol, nRow, 0)));
        }
    }
}

void Test::testDeleteArea_2Ranges_Case2()
{
    ScRangeList aList(ScRange(1,1,0,1,5,0));
    aList.DeleteArea(0,3,0,MAXCOL,3,0);

    for(SCROW nRow = 1; nRow <= 5; ++nRow)
    {
        if(nRow == 3)
            CPPUNIT_ASSERT(!aList.In(ScRange(1,3,0)));
        else
            CPPUNIT_ASSERT(aList.In(ScRange(1,nRow,0)));
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), aList.GetCellCount());
}

void Test::testDeleteArea_2Ranges_Case3()
{
    ScRangeList aList(ScRange(0,5,0,2,10,0));
    aList.DeleteArea(2,3,0,3,7,0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aList.size());

    // Column 2 Row 5-7 shouldn't be in the list.
    for (SCCOL nCol = 0; nCol <= 2; ++nCol)
    {
        for (SCROW nRow = 5; nRow <= 10; ++nRow)
        {
            if (nCol == 2 && (5 <= nRow && nRow <= 7))
                CPPUNIT_ASSERT(!aList.In(ScRange(nCol,nRow,0)));
            else
                CPPUNIT_ASSERT(aList.In(ScRange(nCol,nRow,0)));
        }
    }
}

void Test::testDeleteArea_2Ranges_Case4()
{
    ScRangeList aList(ScRange(2,3,0,4,7,0));
    aList.DeleteArea(0,1,0,2,5,0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aList.size());

    // Column 2 Row 3-5 shouldn't be in the list.
    for (SCCOL nCol = 2; nCol <= 4; ++nCol)
    {
        for (SCROW nRow = 3; nRow <= 7; ++nRow)
        {
            if (nCol == 2 && (3 <= nRow && nRow <= 5))
                CPPUNIT_ASSERT(!aList.In(ScRange(nCol,nRow,0)));
            else
                CPPUNIT_ASSERT(aList.In(ScRange(nCol,nRow,0)));
        }
    }
}

void Test::testDeleteArea_2Ranges_Case5()
{
    ScRangeList aList(ScRange(2,2,0,5,5,0));
    aList.DeleteArea(4,5,0,5,5,0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aList.size());

    // Column 4 and 5 Row 5 shouldn't be in the list.
    for(SCCOL nCol = 2; nCol <= 5; ++nCol)
    {
        for(SCROW nRow = 2; nRow <= 5; ++nRow)
        {
            if(nRow == 5 && 4 <= nCol)
                CPPUNIT_ASSERT(!aList.In(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.In(ScRange(nCol, nRow, 0)));
        }
    }
}

void Test::testDeleteArea_2Ranges_Case6()
{
    ScRangeList aList(ScRange(2,2,0,5,5,0));
    aList.DeleteArea(4,2,0,5,2,0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aList.size());

    // Column 4 and 5 Row 2 shouldn't be in the list.
    for(SCCOL nCol = 2; nCol <= 5; ++nCol)
    {
        for(SCROW nRow = 2; nRow <= 5; ++nRow)
        {
            if(nRow == 2 && 4 <= nCol)
                CPPUNIT_ASSERT(!aList.In(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.In(ScRange(nCol, nRow, 0)));
        }
    }
}

void Test::testDeleteArea_2Ranges_Case7()
{
    ScRangeList aList(ScRange(2,2,0,5,5,0));
    aList.DeleteArea(2,5,0,2,5,0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aList.size());

    // Column 2 Row 5 shouldn't be in the list.
    for(SCCOL nCol = 2; nCol <= 5; ++nCol)
    {
        for(SCROW nRow = 2; nRow <= 5; ++nRow)
        {
            if(nRow == 5 && nCol == 2)
                CPPUNIT_ASSERT(!aList.In(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.In(ScRange(nCol, nRow, 0)));
        }
    }
}

void Test::testDeleteArea_2Ranges_Case8()
{
    ScRangeList aList(ScRange(2,2,0,5,5,0));
    aList.DeleteArea(2,2,0,3,2,0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aList.size());

    // Column 2 & 3 Row 2 shouldn't be in the list.
    for(SCCOL nCol = 2; nCol <= 5; ++nCol)
    {
        for(SCROW nRow = 2; nRow <= 5; ++nRow)
        {
            if(nRow == 2 && nCol <= 3)
                CPPUNIT_ASSERT(!aList.In(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.In(ScRange(nCol, nRow, 0)));
        }
    }
}

void Test::testDeleteArea_1Range()
{
    ScRangeList aList(ScRange(1,1,0,3,3,0));
    aList.DeleteArea(1,1,0,2,3,0);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aList.size());

    for(SCROW nRow = 1; nRow <= 3; ++nRow)
    {
        CPPUNIT_ASSERT(aList.In(ScRange(3,nRow,0)));
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aList.GetCellCount());
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

void Test::testJoin_Case1()
{
    ScRangeList aList;
    aList.push_back(new ScRange(1,1,0,3,3,0));
    aList.Join(ScRange(4,1,0,6,3,0));

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aList.size());
    CPPUNIT_ASSERT_EQUAL( ScRange(1,1,0,6,3,0), *aList[0]);
}

void Test::testJoin_Case2()
{
    ScRangeList aList;
    aList.push_back(new ScRange(1,1,0,3,3,0));
    aList.push_back(new ScRange(4,1,0,6,3,0));
    aList.push_back(new ScRange(7,1,0,9,3,0));

    aList.Join(*aList[2], true);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aList.size());
    CPPUNIT_ASSERT_EQUAL(ScRange(1,1,0,9,3,0), *aList[0]);
}

void Test::testUpdateReference_DeleteRow()
{
    ScRangeList aList(ScRange(1,1,0,4,4,0));
    bool bUpdated = aList.UpdateReference(URM_INSDEL, m_pDoc, ScRange(0,3,0,MAXCOL,MAXROW,0), 0, -1, 0);
    CPPUNIT_ASSERT(bUpdated);

    for(SCCOL nCol = 1; nCol <= 4; ++nCol)
    {
        for(SCROW nRow = 1; nRow <= 3; ++nRow)
        {
            CPPUNIT_ASSERT(aList.In(ScRange(nCol, nRow, 0)));
        }
        CPPUNIT_ASSERT(!aList.In(ScRange(nCol, 4, 0)));
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), aList.GetCellCount());

    ScRangeList aList2(ScRange(2,2,0,2,2,0));
    aList2.UpdateReference(URM_INSDEL, m_pDoc, ScRange(0,3,0,MAXCOL,MAXROW,0), 0, -1, 0);
    CPPUNIT_ASSERT(aList2.empty());
}

void Test::testUpdateReference_DeleteLastRow()
{
    ScRangeList aList(ScRange(1,1,0,4,4,0));
    bool bUpdated = aList.UpdateReference(URM_INSDEL, m_pDoc, ScRange(0,4,0,MAXCOL,4,0), 0, -1, 0);
    CPPUNIT_ASSERT(bUpdated);
}

void Test::testUpdateReference_DeleteCol()
{
    ScRangeList aList(ScRange(1,1,0,4,4,0));
    bool bUpdated = aList.UpdateReference(URM_INSDEL, m_pDoc, ScRange(3,0,0,MAXCOL,MAXROW,0), -1, 0, 0);
    CPPUNIT_ASSERT(bUpdated);

    for(SCROW nRow = 1; nRow <= 4; ++nRow)
    {
        for(SCCOL nCol = 1; nCol <= 3; ++nCol)
        {
            CPPUNIT_ASSERT(aList.In(ScRange(nCol, nRow, 0)));
        }
        CPPUNIT_ASSERT(!aList.In(ScRange(4, nRow, 0)));
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), aList.GetCellCount());
}

void Test::testGetIntersectedRange()
{
    ScRangeList aList(ScRange(2, 2, 0, 5, 5, 0));
    ScRangeList aIntersecting = aList.GetIntersectedRange(ScRange(0, 0, 0, 3, 3, 0));
    CPPUNIT_ASSERT_EQUAL(ScRangeList(ScRange(2,2,0,3,3,0)), aIntersecting);
}

void Test::testInsertRow()
{
    ScRangeList aList(ScRange(1,1,0,4,4,0));
    aList.InsertRow(0, 0, MAXCOL, 5, 2);
    CPPUNIT_ASSERT_EQUAL(ScRangeList(ScRange(1,1,0,4,6,0)), aList);
}

void Test::testInsertCol()
{
    ScRangeList aList(ScRange(1,1,0,4,4,0));
    aList.InsertCol(0, 0, MAXROW, 5, 2);
    CPPUNIT_ASSERT_EQUAL(ScRangeList(ScRange(1,1,0,6,4,0)), aList);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
