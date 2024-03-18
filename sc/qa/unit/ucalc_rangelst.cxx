/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include "helper/qahelper.hxx"

#include <rangelst.hxx>

class Test : public ScUcalcTestBase
{
};

CPPUNIT_TEST_FIXTURE(Test, testDeleteArea_4Ranges)
{
    ScRangeList aList(ScRange(0,0,0,5,5,0));
    aList.DeleteArea(2,2,0,3,3,0);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), aList.size());
    for(SCCOL nCol = 0; nCol <= 5; ++nCol)
    {
        for(SCROW nRow = 0; nRow <= 5; ++nRow)
        {
            if((nCol == 2 || nCol == 3) && ( nRow == 2 || nRow == 3))
                CPPUNIT_ASSERT(!aList.Contains(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.Contains(ScRange(nCol, nRow, 0)));
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteArea_3Ranges)
{
    ScRangeList aList(ScRange(1,1,0,6,6,0));
    aList.DeleteArea(3,3,0,8,4,0);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aList.size());
    for(SCCOL nCol = 1; nCol <= 6; ++nCol)
    {
        for(SCROW nRow = 1; nRow <= 6; ++nRow)
        {
            if((nRow == 3 || nRow == 4) && (nCol >= 3))
                CPPUNIT_ASSERT(!aList.Contains(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.Contains(ScRange(nCol, nRow, 0)));
        }
    }

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt64>(28), aList.GetCellCount());
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteArea_3Ranges_Case2)
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
                CPPUNIT_ASSERT(!aList.Contains(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.Contains(ScRange(nCol, nRow, 0)));
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteArea_3Ranges_Case3)
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
                CPPUNIT_ASSERT(!aList.Contains(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.Contains(ScRange(nCol, nRow, 0)));
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteArea_3Ranges_Case4)
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
                CPPUNIT_ASSERT(!aList.Contains(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.Contains(ScRange(nCol, nRow, 0)));
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteArea_3Ranges_Case5)
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
                CPPUNIT_ASSERT(!aList.Contains(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.Contains(ScRange(nCol, nRow, 0)));
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteArea_2Ranges)
{
    ScRangeList aList(ScRange(0,0,0,5,5,5));
    ScRangeList aList2(aList);

    aList.DeleteArea(4,4,0,6,7,0);
    aList2.DeleteArea(4,4,0,6,7,0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aList.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aList2.size());

    for(SCCOL nCol = 0; nCol <= 5; ++nCol)
    {
        for(SCROW nRow = 0; nRow <= 5; ++nRow)
        {
            if(nCol>=4 && nRow >= 4)
                CPPUNIT_ASSERT(!aList.Contains(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.Contains(ScRange(nCol, nRow, 0)));
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteArea_2Ranges_Case2)
{
    ScRangeList aList(ScRange(1,1,0,1,5,0));
    aList.DeleteArea(0,3,0,ScSheetLimits::CreateDefault().MaxCol(),3,0);

    for(SCROW nRow = 1; nRow <= 5; ++nRow)
    {
        if(nRow == 3)
            CPPUNIT_ASSERT(!aList.Contains(ScRange(1,3,0)));
        else
            CPPUNIT_ASSERT(aList.Contains(ScRange(1,nRow,0)));
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt64>(4), aList.GetCellCount());
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteArea_2Ranges_Case3)
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
                CPPUNIT_ASSERT(!aList.Contains(ScRange(nCol,nRow,0)));
            else
                CPPUNIT_ASSERT(aList.Contains(ScRange(nCol,nRow,0)));
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteArea_2Ranges_Case4)
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
                CPPUNIT_ASSERT(!aList.Contains(ScRange(nCol,nRow,0)));
            else
                CPPUNIT_ASSERT(aList.Contains(ScRange(nCol,nRow,0)));
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteArea_2Ranges_Case5)
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
                CPPUNIT_ASSERT(!aList.Contains(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.Contains(ScRange(nCol, nRow, 0)));
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteArea_2Ranges_Case6)
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
                CPPUNIT_ASSERT(!aList.Contains(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.Contains(ScRange(nCol, nRow, 0)));
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteArea_2Ranges_Case7)
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
                CPPUNIT_ASSERT(!aList.Contains(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.Contains(ScRange(nCol, nRow, 0)));
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteArea_2Ranges_Case8)
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
                CPPUNIT_ASSERT(!aList.Contains(ScRange(nCol, nRow, 0)));
            else
                CPPUNIT_ASSERT(aList.Contains(ScRange(nCol, nRow, 0)));
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteArea_1Range)
{
    ScRangeList aList(ScRange(1,1,0,3,3,0));
    aList.DeleteArea(1,1,0,2,3,0);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aList.size());

    for(SCROW nRow = 1; nRow <= 3; ++nRow)
    {
        CPPUNIT_ASSERT(aList.Contains(ScRange(3,nRow,0)));
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt64>(3), aList.GetCellCount());
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteArea_0Ranges)
{
    ScRangeList aList(ScRange(1,1,0,3,3,0));
    aList.DeleteArea(1,1,0,3,3,0);

    CPPUNIT_ASSERT(aList.empty());

    ScRangeList aList2(ScRange(1,1,0,3,3,0));
    aList2.DeleteArea(0,0,0,4,4,0);

    CPPUNIT_ASSERT(aList.empty());
}

CPPUNIT_TEST_FIXTURE(Test, testJoin_Case1)
{
    ScRangeList aList;
    aList.push_back(ScRange(1,1,0,3,3,0));
    aList.Join(ScRange(4,1,0,6,3,0));

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aList.size());
    CPPUNIT_ASSERT_EQUAL( ScRange(1,1,0,6,3,0), aList[0]);
}

CPPUNIT_TEST_FIXTURE(Test, testJoin_Case2)
{
    ScRangeList aList;
    aList.push_back(ScRange(1,1,0,3,3,0));
    aList.push_back(ScRange(4,1,0,6,3,0));
    aList.push_back(ScRange(7,1,0,9,3,0));

    aList.Join(aList[2], true);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aList.size());
    CPPUNIT_ASSERT_EQUAL(ScRange(1,1,0,9,3,0), aList[0]);
}

CPPUNIT_TEST_FIXTURE(Test, testJoin_Case3)
{
    ScRangeList aList;
    aList.Join(ScRange(1,1,0,6,6,0));
    aList.Join(ScRange(3,3,0,4,4,0));

    // The second one should have been swallowed by the first one
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aList.size());
    CPPUNIT_ASSERT_EQUAL(ScRange(1,1,0,6,6,0), aList[0]);

    // Add a disjoint one
    aList.Join(ScRange(8,8,0,9,9,0));

    // Should be two ones now
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aList.size());
    // The first one should still be as is
    CPPUNIT_ASSERT_EQUAL(ScRange(1,1,0,6,6,0), aList[0]);
    // Ditto for the second one
    CPPUNIT_ASSERT_EQUAL(ScRange(8,8,0,9,9,0), aList[1]);
}

CPPUNIT_TEST_FIXTURE(Test, testJoin_Case4)
{
    ScRangeList aList;
    aList.Join(ScRange(1,1,0,2,6,0));
    // Join a range that overlaps it and extends it vertically
    aList.Join(ScRange(1,4,0,2,8,0));

    // The one range in the list should have been extended
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aList.size());
    CPPUNIT_ASSERT_EQUAL(ScRange(1,1,0,2,8,0), aList[0]);

    // Join a range that overlaps it and extends it horizontally
    aList.Join(ScRange(2,1,0,4,8,0));

    // Again, should have just been extended
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aList.size());
    CPPUNIT_ASSERT_EQUAL(ScRange(1,1,0,4,8,0), aList[0]);

    // And then the same but on top / to the left of existing range
    ScRangeList aList2;
    aList2.Join(ScRange(4,4,0,8,8,0));
    aList2.Join(ScRange(4,1,0,8,6,0));

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aList2.size());
    CPPUNIT_ASSERT_EQUAL(ScRange(4,1,0,8,8,0), aList2[0]);

    aList2.Join(ScRange(1,1,0,6,8,0));

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aList2.size());
    CPPUNIT_ASSERT_EQUAL(ScRange(1,1,0,8,8,0), aList2[0]);
}

CPPUNIT_TEST_FIXTURE(Test, testJoin_Case5)
{
    ScRangeList aList;
    aList.Join(ScRange(0,0,0,4,4,0));
    aList.Join(ScRange(8,0,0,10,4,0));

    // Nothing special so far, two disjoint ranges
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aList.size());
    CPPUNIT_ASSERT_EQUAL(ScRange(0,0,0,4,4,0), aList[0]);
    CPPUNIT_ASSERT_EQUAL(ScRange(8,0,0,10,4,0), aList[1]);

    // This should join the two ranges into one
    aList.Join(ScRange(5,0,0,9,4,0));

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aList.size());
    CPPUNIT_ASSERT_EQUAL(ScRange(0,0,0,10,4,0), aList[0]);
}

CPPUNIT_TEST_FIXTURE(Test, testUpdateReference_DeleteRow)
{
    ScRangeList aList(ScRange(1,1,0,4,4,0));
    bool bUpdated = aList.UpdateReference(URM_INSDEL, m_pDoc, ScRange(0,3,0,m_pDoc->MaxCol(),m_pDoc->MaxRow(),0), 0, -1, 0);
    CPPUNIT_ASSERT(bUpdated);

    for(SCCOL nCol = 1; nCol <= 4; ++nCol)
    {
        for(SCROW nRow = 1; nRow <= 3; ++nRow)
        {
            CPPUNIT_ASSERT(aList.Contains(ScRange(nCol, nRow, 0)));
        }
        CPPUNIT_ASSERT(!aList.Contains(ScRange(nCol, 4, 0)));
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt64>(12), aList.GetCellCount());

    ScRangeList aList2(ScRange(2,2,0,2,2,0));
    aList2.UpdateReference(URM_INSDEL, m_pDoc, ScRange(0,3,0,m_pDoc->MaxCol(),m_pDoc->MaxRow(),0), 0, -1, 0);
    CPPUNIT_ASSERT(aList2.empty());

    ScRangeList aList3;
    aList3.push_back(ScRange(2,2,0,2,8,0));
    aList3.push_back(ScRange(4,2,0,4,8,0));
    aList3.UpdateReference(URM_INSDEL, m_pDoc, ScRange(2,5,0,m_pDoc->MaxCol(),m_pDoc->MaxRow(),0), 0, -1, 0);
    // Verify all ranges in the list have been updated properly.
    CPPUNIT_ASSERT_EQUAL(size_t(2), aList3.size());
    CPPUNIT_ASSERT_EQUAL(ScRange(2,2,0,2,7,0), aList3[0]);
    CPPUNIT_ASSERT_EQUAL(ScRange(4,2,0,4,7,0), aList3[1]);

    ScRangeList aList4(ScRange(0,0,0,m_pDoc->MaxCol(),m_pDoc->MaxRow(),0));
    ScRangeList aList4Copy = aList4;
    aList4.UpdateReference(URM_INSDEL, m_pDoc, ScRange(14,3,0,m_pDoc->MaxCol(),7,0), 0, -2, 0);
    CPPUNIT_ASSERT_EQUAL(aList4Copy, aList4);
}

CPPUNIT_TEST_FIXTURE(Test, testUpdateReference_DeleteLastRow)
{
    ScRangeList aList(ScRange(1,1,0,4,4,0));
    bool bUpdated = aList.UpdateReference(URM_INSDEL, m_pDoc, ScRange(0,4,0,m_pDoc->MaxCol(),4,0), 0, -1, 0);
    CPPUNIT_ASSERT(bUpdated);
}

CPPUNIT_TEST_FIXTURE(Test, testUpdateReference_DeleteCol)
{
    ScRangeList aList(ScRange(1,1,0,4,4,0));
    bool bUpdated = aList.UpdateReference(URM_INSDEL, m_pDoc, ScRange(3,0,0,m_pDoc->MaxCol(),m_pDoc->MaxRow(),0), -1, 0, 0);
    CPPUNIT_ASSERT(bUpdated);

    for(SCROW nRow = 1; nRow <= 4; ++nRow)
    {
        for(SCCOL nCol = 1; nCol <= 3; ++nCol)
        {
            CPPUNIT_ASSERT(aList.Contains(ScRange(nCol, nRow, 0)));
        }
        CPPUNIT_ASSERT(!aList.Contains(ScRange(4, nRow, 0)));
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt64>(12), aList.GetCellCount());
}

CPPUNIT_TEST_FIXTURE(Test, testGetIntersectedRange)
{
    ScRangeList aList(ScRange(2, 2, 0, 5, 5, 0));
    ScRangeList aIntersecting = aList.GetIntersectedRange(ScRange(0, 0, 0, 3, 3, 0));
    CPPUNIT_ASSERT_EQUAL(ScRangeList(ScRange(2,2,0,3,3,0)), aIntersecting);
}

CPPUNIT_TEST_FIXTURE(Test, testInsertRow)
{
    ScRangeList aList(ScRange(1,1,0,4,4,0));
    aList.InsertRow(0, 0, m_pDoc->MaxCol(), 5, 2);
    CPPUNIT_ASSERT_EQUAL(ScRangeList(ScRange(1,1,0,4,6,0)), aList);
}

CPPUNIT_TEST_FIXTURE(Test, testInsertCol)
{
    ScRangeList aList(ScRange(1,1,0,4,4,0));
    aList.InsertCol(0, 0, m_pDoc->MaxRow(), 5, 2);
    CPPUNIT_ASSERT_EQUAL(ScRangeList(ScRange(1,1,0,6,4,0)), aList);
}

CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
