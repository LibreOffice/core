/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/types.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <tools/stream.hxx>
#include <tools/Pair.hxx>

namespace tools
{
class PairTest : public CppUnit::TestFixture
{
public:
    void testPair()
    {
        long nExpectedA = 0;
        long nExpectedB = 0;
        long nActualA = 0;
        long nActualB = 0;

        {
            Pair aPair;
            nActualA = aPair.A();
            nActualB = aPair.B();

            CPPUNIT_ASSERT_EQUAL(nExpectedA, nActualA);
            CPPUNIT_ASSERT_EQUAL(nExpectedB, nActualB);
        }

        {
            Pair aPair(1, 2);
            nExpectedA = 1;
            nExpectedB = 2;
            nActualA = aPair.A();
            nActualB = aPair.B();

            CPPUNIT_ASSERT_EQUAL(nExpectedA, nActualA);
            CPPUNIT_ASSERT_EQUAL(nExpectedB, nActualB);
        }
    }

    void testToString()
    {
        OString sExpectedString("1, 2");
        Pair aPair(1, 2);
        CPPUNIT_ASSERT_EQUAL(sExpectedString, aPair.toString());
    }

    void testReadStream()
    {
        TestPair* pData = new TestPair(1, 2);
        SvMemoryStream aMemStm(pData, 8, StreamMode::READ);

        Pair aPair;
        ReadPair(aMemStm, aPair);

        sal_Int32 nExpectedA = 1;
        sal_Int32 nExpectedB = 2;
        sal_Int32 nActualA = aPair.A();
        sal_Int32 nActualB = aPair.B();

        CPPUNIT_ASSERT_EQUAL(nExpectedA, nActualA);
        CPPUNIT_ASSERT_EQUAL(nExpectedB, nActualB);
    }

    void testWriteStream()
    {
        SvMemoryStream aMemStm;

        WritePair(aMemStm, Pair(1, 2));

        Pair aPair;
        aMemStm.Seek(0); // reset to the beginning of the stream
        ReadPair(aMemStm, aPair);

        sal_Int32 nExpectedA = 1;
        sal_Int32 nExpectedB = 2;
        sal_Int32 nActualA = aPair.A();
        sal_Int32 nActualB = aPair.B();

        CPPUNIT_ASSERT_EQUAL(nExpectedA, nActualA);
        CPPUNIT_ASSERT_EQUAL(nExpectedB, nActualB);
    }

    CPPUNIT_TEST_SUITE(PairTest);
    CPPUNIT_TEST(testPair);
    CPPUNIT_TEST(testToString);
    CPPUNIT_TEST(testReadStream);
    CPPUNIT_TEST(testWriteStream);
    CPPUNIT_TEST_SUITE_END();

private:
    struct TestPair
    {
        sal_Int32 mnA;
        sal_Int32 mnB;

        TestPair(sal_Int32 nA, sal_Int32 nB)
            : mnA(nA)
            , mnB(nB){};
    };
};

CPPUNIT_TEST_SUITE_REGISTRATION(PairTest);
} // namespace tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
