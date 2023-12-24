/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <EditLine.hxx>

namespace
{
class EditLineTest : public test::BootstrapFixture
{
};

CPPUNIT_TEST_FIXTURE(EditLineTest, testConstruction)
{
    EditLine aNew;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aNew.GetStart());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aNew.GetEnd());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aNew.GetStartPortion());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aNew.GetEndPortion());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), aNew.GetMaxAscent());
    CPPUNIT_ASSERT_EQUAL(false, aNew.IsValid());
}

CPPUNIT_TEST_FIXTURE(EditLineTest, testCopyConstructor)
{
    EditLine aLine1;
    aLine1.SetStart(1);
    aLine1.SetEnd(2);
    aLine1.SetStartPortion(10);
    aLine1.SetEndPortion(20);
    aLine1.SetMaxAscent(200);
    aLine1.SetValid();

    // set Line2
    EditLine aLine2(aLine1);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aLine2.GetStart());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aLine2.GetEnd());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), aLine2.GetStartPortion());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(20), aLine2.GetEndPortion());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), aLine2.GetMaxAscent());
    CPPUNIT_ASSERT_EQUAL(false, aLine2.IsValid());
}

CPPUNIT_TEST_FIXTURE(EditLineTest, testAssign)
{
    EditLine aLine1;
    aLine1.SetStart(1);
    aLine1.SetEnd(2);
    aLine1.SetStartPortion(10);
    aLine1.SetEndPortion(20);
    aLine1.SetMaxAscent(200);
    aLine1.SetValid();

    // set Line2
    EditLine aLine2;
    aLine2 = aLine1;

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aLine2.GetStart());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aLine2.GetEnd());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), aLine2.GetStartPortion());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(20), aLine2.GetEndPortion());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), aLine2.GetMaxAscent());
    CPPUNIT_ASSERT_EQUAL(false, aLine2.IsValid());
}

CPPUNIT_TEST_FIXTURE(EditLineTest, testEquals)
{
    EditLine aLine1;
    EditLine aLine2;

    // both empty = equal
    CPPUNIT_ASSERT_EQUAL(true, aLine1 == aLine2);

    aLine1.SetStart(10);
    CPPUNIT_ASSERT_EQUAL(false, aLine1 == aLine2);

    aLine2.SetStart(10);
    CPPUNIT_ASSERT_EQUAL(true, aLine1 == aLine2);

    aLine1.SetEnd(20);
    CPPUNIT_ASSERT_EQUAL(false, aLine1 == aLine2);

    aLine2.SetEnd(20);
    CPPUNIT_ASSERT_EQUAL(true, aLine1 == aLine2);

    aLine1.SetStartPortion(100);
    CPPUNIT_ASSERT_EQUAL(false, aLine1 == aLine2);

    aLine2.SetStartPortion(100);
    CPPUNIT_ASSERT_EQUAL(true, aLine1 == aLine2);

    aLine1.SetEndPortion(200);
    CPPUNIT_ASSERT_EQUAL(false, aLine1 == aLine2);

    aLine2.SetEndPortion(200);
    CPPUNIT_ASSERT_EQUAL(true, aLine1 == aLine2);

    aLine2.SetMaxAscent(200); // doesn't influence equality
    CPPUNIT_ASSERT_EQUAL(true, aLine1 == aLine2);
}

} // end anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
