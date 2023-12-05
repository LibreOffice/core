/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <editeng/EPaM.hxx>

namespace
{
class EPaMTest : public test::BootstrapFixture
{
};

CPPUNIT_TEST_FIXTURE(EPaMTest, testConstruction)
{
    {
        EPaM aNew;
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aNew.nPara);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aNew.nIndex);
    }

    {
        EPaM aNew(1, 2);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aNew.nPara);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNew.nIndex);
    }

    {
        EPaM aNew = { 1, 2 };
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aNew.nPara);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNew.nIndex);
    }

    {
        EPaM aNew{ 1, 2 };
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aNew.nPara);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNew.nIndex);
    }
}

CPPUNIT_TEST_FIXTURE(EPaMTest, testAssign)
{
    EPaM aPaM1;
    EPaM aPaM2;

    // set PaM 2
    aPaM2 = EPaM{ 2, 1 };

    // selections are not equal
    CPPUNIT_ASSERT(aPaM2 != aPaM1);

    // assign PaM 1 with PaM 2 content
    aPaM1 = aPaM2;

    // expect selections to be equal
    CPPUNIT_ASSERT_EQUAL(aPaM2, aPaM1);
}

CPPUNIT_TEST_FIXTURE(EPaMTest, testEquals)
{
    EPaM aPaM1;
    EPaM aPaM2;

    // both empty = equal
    CPPUNIT_ASSERT_EQUAL(aPaM1, aPaM2);

    // set PaM 1
    aPaM1 = { 1, 2 };

    // expect them to be not equal
    CPPUNIT_ASSERT(aPaM1 != aPaM2);

    // set PaM 2 to the same value
    aPaM2 = { 1, 2 };

    // equal again
    CPPUNIT_ASSERT_EQUAL(aPaM1, aPaM2);
}

CPPUNIT_TEST_FIXTURE(EPaMTest, testLess)
{
    // Both equal
    CPPUNIT_ASSERT_EQUAL(false, EPaM(0, 0) < EPaM(0, 0));

    // Obviously not less
    CPPUNIT_ASSERT_EQUAL(false, EPaM(0, 2) < EPaM(0, 1));

    // Strictly "<"
    CPPUNIT_ASSERT_EQUAL(true, EPaM(0, 0) < EPaM(0, 1));

    // Check if paragraph taken into account
    CPPUNIT_ASSERT_EQUAL(false, EPaM(1, 0) < EPaM(0, 1));
    CPPUNIT_ASSERT_EQUAL(true, EPaM(1, 0) < EPaM(2, 0));
}

} // end anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
