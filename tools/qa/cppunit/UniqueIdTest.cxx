/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/UniqueID.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

namespace
{
class UniqueIdTest : public CppUnit::TestFixture
{
};

struct ObjectExample1
{
    UniqueID maID;
};

struct ObjectExample2
{
    UniqueID maID;
};

CPPUNIT_TEST_FIXTURE(UniqueIdTest, testUniqueness)
{
    UniqueID aID;
    // Check ID
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(1), aID.getID());
    // Call again - same result
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(1), aID.getID());

    // Check creating another instance
    {
        UniqueID aID2;
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(1), aID.getID());
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(2), aID2.getID());
    }

    // Check creating third instance
    {
        UniqueID aID3;
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(1), aID.getID());
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(3), aID3.getID());
    }

    // Check object copying - preserve the id
    ObjectExample1 objectA;
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(4), objectA.maID.getID());

    ObjectExample1 objectB = objectA;
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(4), objectA.maID.getID());
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(4), objectB.maID.getID());

    // Multiple objects
    ObjectExample2 objectC;
    ObjectExample1 objectD;
    ObjectExample2 objectE;
    ObjectExample1 objectF;

    CPPUNIT_ASSERT_EQUAL(sal_uInt64(5), objectC.maID.getID());
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(6), objectD.maID.getID());
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(7), objectE.maID.getID());
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(8), objectF.maID.getID());
}

} // end anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
