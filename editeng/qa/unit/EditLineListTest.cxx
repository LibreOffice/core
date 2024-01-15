/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <EditLineList.hxx>

namespace
{
class EditLineListTest : public test::BootstrapFixture
{
};

CPPUNIT_TEST_FIXTURE(EditLineListTest, testConstruction)
{
    EditLineList aNewList;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aNewList.Count());
}

CPPUNIT_TEST_FIXTURE(EditLineListTest, testAppendAndInsert)
{
    // Test case:
    // Append EditLine1
    // Append EditLine3
    // Insert EditLine2 in-between EditLine1 and pEditLine3

    EditLineList aNewList;

    EditLine* pEditLine1 = new EditLine;
    EditLine* pEditLine2 = new EditLine;
    EditLine* pEditLine3 = new EditLine;

    aNewList.Append(std::unique_ptr<EditLine>(pEditLine1));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aNewList.Count());

    aNewList.Append(std::unique_ptr<EditLine>(pEditLine3));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNewList.Count());

    CPPUNIT_ASSERT_EQUAL(pEditLine1, &aNewList[0]);
    CPPUNIT_ASSERT_EQUAL(pEditLine3, &aNewList[1]);

    aNewList.Insert(1, std::unique_ptr<EditLine>(pEditLine2));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aNewList.Count());

    CPPUNIT_ASSERT_EQUAL(pEditLine1, &aNewList[0]);
    CPPUNIT_ASSERT_EQUAL(pEditLine2, &aNewList[1]);
    CPPUNIT_ASSERT_EQUAL(pEditLine3, &aNewList[2]);
}

CPPUNIT_TEST_FIXTURE(EditLineListTest, testReset)
{
    EditLineList aNewList;

    aNewList.Append(std::make_unique<EditLine>());
    aNewList.Append(std::make_unique<EditLine>());
    aNewList.Append(std::make_unique<EditLine>());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aNewList.Count());

    aNewList.Reset();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aNewList.Count());
}

CPPUNIT_TEST_FIXTURE(EditLineListTest, testDeleteFrom)
{
    EditLineList aNewList;

    aNewList.Append(std::make_unique<EditLine>());
    aNewList.Append(std::make_unique<EditLine>());
    aNewList.Append(std::make_unique<EditLine>());
    aNewList.Append(std::make_unique<EditLine>());
    aNewList.Append(std::make_unique<EditLine>());

    aNewList[0].SetStart(10);
    aNewList[1].SetStart(11);
    aNewList[2].SetStart(12);
    aNewList[3].SetStart(13);
    aNewList[4].SetStart(14);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aNewList.Count());

    aNewList.DeleteFromLine(2);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNewList.Count());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), aNewList[0].GetStart());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), aNewList[1].GetStart());

    aNewList.Append(std::make_unique<EditLine>());
    aNewList.Append(std::make_unique<EditLine>());
    aNewList.Append(std::make_unique<EditLine>());
    aNewList.Append(std::make_unique<EditLine>());

    aNewList[2].SetStart(15);
    aNewList[3].SetStart(16);
    aNewList[4].SetStart(17);
    aNewList[5].SetStart(18);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aNewList.Count());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), aNewList[0].GetStart());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), aNewList[1].GetStart());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(15), aNewList[2].GetStart());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16), aNewList[3].GetStart());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(17), aNewList[4].GetStart());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(18), aNewList[5].GetStart());

    aNewList.DeleteFromLine(4);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aNewList.Count());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), aNewList[0].GetStart());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), aNewList[1].GetStart());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(15), aNewList[2].GetStart());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16), aNewList[3].GetStart());
}

CPPUNIT_TEST_FIXTURE(EditLineListTest, testFindLine)
{
    EditLineList aNewList;
    EditLine* pEditLine1 = new EditLine;
    pEditLine1->SetStart(0);
    pEditLine1->SetEnd(10);
    aNewList.Append(std::unique_ptr<EditLine>(pEditLine1));

    EditLine* pEditLine2 = new EditLine;
    pEditLine2->SetStart(10);
    pEditLine2->SetEnd(20);
    aNewList.Append(std::unique_ptr<EditLine>(pEditLine2));

    EditLine* pEditLine3 = new EditLine;
    pEditLine3->SetStart(20);
    pEditLine3->SetEnd(30);
    aNewList.Append(std::unique_ptr<EditLine>(pEditLine3));

    // Exclude end
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aNewList.FindLine(-1, false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aNewList.FindLine(0, false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aNewList.FindLine(5, false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aNewList.FindLine(9, false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aNewList.FindLine(10, false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aNewList.FindLine(15, false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aNewList.FindLine(19, false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNewList.FindLine(20, false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNewList.FindLine(25, false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNewList.FindLine(30, false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNewList.FindLine(31, false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNewList.FindLine(10000, false));

    // Include end
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aNewList.FindLine(-1, true));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aNewList.FindLine(0, true));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aNewList.FindLine(5, true));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aNewList.FindLine(9, true));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aNewList.FindLine(10, true));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aNewList.FindLine(15, true));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aNewList.FindLine(19, true));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aNewList.FindLine(20, true));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNewList.FindLine(25, true));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNewList.FindLine(30, true));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNewList.FindLine(31, true));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNewList.FindLine(10000, true));
}

} // end anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
