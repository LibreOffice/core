/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <editeng/ESelection.hxx>

namespace
{
class ESelectionTest : public test::BootstrapFixture
{
};

CPPUNIT_TEST_FIXTURE(ESelectionTest, testConstruction)
{
    {
        ESelection aNewSelection;
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aNewSelection.start.nPara);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aNewSelection.start.nIndex);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aNewSelection.end.nPara);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aNewSelection.end.nIndex);
    }

    {
        ESelection aNewSelection(1, 2, 3, 4);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aNewSelection.start.nPara);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNewSelection.start.nIndex);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aNewSelection.end.nPara);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aNewSelection.end.nIndex);
    }

    {
        ESelection aNewSelection = { 1, 2, 3, 4 };
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aNewSelection.start.nPara);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNewSelection.start.nIndex);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aNewSelection.end.nPara);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aNewSelection.end.nIndex);
    }

    {
        ESelection aNewSelection{ 1, 2, 3, 4 };
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aNewSelection.start.nPara);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNewSelection.start.nIndex);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aNewSelection.end.nPara);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aNewSelection.end.nIndex);
    }

    {
        ESelection aNewSelection(1, 2);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aNewSelection.start.nPara);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNewSelection.start.nIndex);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aNewSelection.end.nPara);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNewSelection.end.nIndex);
    }
}

CPPUNIT_TEST_FIXTURE(ESelectionTest, testAssign)
{
    ESelection aSelection1;
    ESelection aSelection2;

    // set selection2
    aSelection2 = ESelection{ 1, 1, 2, 1 };

    // selections are not equal
    CPPUNIT_ASSERT(aSelection2 != aSelection1);

    // assign selection1 with selection2 content
    aSelection1 = aSelection2;

    // expect selections to be equal
    CPPUNIT_ASSERT_EQUAL(aSelection2, aSelection1);
}

CPPUNIT_TEST_FIXTURE(ESelectionTest, testEquals)
{
    ESelection aSelection1;
    ESelection aSelection2;

    // both empty = equal
    CPPUNIT_ASSERT_EQUAL(aSelection1, aSelection2);

    // set selection1
    aSelection1 = { 1, 2, 3, 4 };

    // expect them to be not equal
    CPPUNIT_ASSERT(aSelection1 != aSelection2);

    // set selection 2 to the same value
    aSelection2 = { 1, 2, 3, 4 };

    // equal again
    CPPUNIT_ASSERT_EQUAL(aSelection1, aSelection2);
}

CPPUNIT_TEST_FIXTURE(ESelectionTest, testLess)
{
    // Both equal
    CPPUNIT_ASSERT_EQUAL(false, ESelection(0, 0, 1, 1) < ESelection(0, 0, 1, 1));

    // Obviously not less
    CPPUNIT_ASSERT_EQUAL(false, ESelection(0, 2, 0, 2) < ESelection(0, 1, 0, 1));

    // Equal at a point therefore not strictly "<"
    CPPUNIT_ASSERT_EQUAL(false, ESelection(0, 0, 0, 0) < ESelection(0, 0, 0, 1));

    // Strictly "<"
    CPPUNIT_ASSERT_EQUAL(true, ESelection(0, 0, 0, 0) < ESelection(0, 1, 0, 1));

    // Check if paragraph taken into account
    CPPUNIT_ASSERT_EQUAL(false, ESelection(1, 0, 1, 0) < ESelection(0, 1, 0, 1));
    CPPUNIT_ASSERT_EQUAL(true, ESelection(1, 0, 1, 0) < ESelection(2, 0, 2, 0));
}

CPPUNIT_TEST_FIXTURE(ESelectionTest, testGreater)
{
    // Both equal
    CPPUNIT_ASSERT_EQUAL(false, ESelection(0, 0, 1, 1) > ESelection(0, 0, 1, 1));

    // Obviously not greater
    CPPUNIT_ASSERT_EQUAL(false, ESelection(0, 1, 0, 1) > ESelection(0, 2, 0, 2));

    // Equal at a point therefore not strictly ">"
    CPPUNIT_ASSERT_EQUAL(false, ESelection(0, 0, 0, 1) > ESelection(0, 0, 0, 0));

    // Strictly ">"
    CPPUNIT_ASSERT_EQUAL(true, ESelection(0, 1, 0, 1) > ESelection(0, 0, 0, 0));

    // Check if paragraph taken into account
    CPPUNIT_ASSERT_EQUAL(false, ESelection(0, 1, 0, 1) > ESelection(1, 0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(true, ESelection(2, 0, 2, 0) > ESelection(1, 0, 1, 0));
}

CPPUNIT_TEST_FIXTURE(ESelectionTest, testAdjust)
{
    // Scenarios where Adjust doesn't change the selection
    ESelection aSelection;
    aSelection.Adjust();
    CPPUNIT_ASSERT_EQUAL(ESelection(), aSelection);

    aSelection = { 1, 1, 1, 1 };
    aSelection.Adjust();
    CPPUNIT_ASSERT_EQUAL(ESelection(1, 1, 1, 1), aSelection);

    aSelection = { 1, 1, 1, 2 };
    aSelection.Adjust();
    CPPUNIT_ASSERT_EQUAL(ESelection(1, 1, 1, 2), aSelection);

    aSelection = { 1, 1, 2, 1 };
    aSelection.Adjust();
    CPPUNIT_ASSERT_EQUAL(ESelection(1, 1, 2, 1), aSelection);

    // Position is greater - flip
    aSelection = { 1, 2, 1, 1 };
    aSelection.Adjust();
    CPPUNIT_ASSERT_EQUAL(ESelection(1, 1, 1, 2), aSelection);

    // Paragraph is greater - flip
    aSelection = { 2, 1, 1, 1 };
    aSelection.Adjust();
    CPPUNIT_ASSERT_EQUAL(ESelection(1, 1, 2, 1), aSelection);

    // Both are greater - flip
    aSelection = { 2, 2, 1, 1 };
    aSelection.Adjust();
    CPPUNIT_ASSERT_EQUAL(ESelection(1, 1, 2, 2), aSelection);
}

} // end anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
