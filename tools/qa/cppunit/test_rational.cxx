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

#include <rtl/math.hxx>
#include <tools/rational.hxx>

namespace tools
{

class RationalTest : public CppUnit::TestFixture
{
public:

    void testReduceInaccurate()
    {
        const boost::rational<sal_Int64> aFract(1082130431,1073741824);
        CPPUNIT_ASSERT_MESSAGE( "Fraction #1 not approximately equal to 1.007812499068677",
                                rtl::math::approxEqual(boost::rational_cast<double>(aFract),1.007812499068677) );

        boost::rational<sal_Int64> aFract2( aFract );
        rational_ReduceInaccurate(aFract2, 8);
        CPPUNIT_ASSERT_MESSAGE( "Fraction #2 not 1",
                                aFract2.numerator() == 1 &&
                                aFract2.denominator() == 1 );

        boost::rational<sal_Int64> aFract3( 0x7AAAAAAA, 0x35555555 );
        CPPUNIT_ASSERT_MESSAGE( "Fraction #3 cancellation wrong",
                                aFract3.numerator() == 0x7AAAAAAA &&
                                aFract3.denominator() == 0x35555555 );
        rational_ReduceInaccurate(aFract3, 30);
        CPPUNIT_ASSERT_MESSAGE( "Fraction #3 ReduceInaccurate errorneously cut precision",
                                aFract3.numerator() == 0x7AAAAAAA &&
                                aFract3.denominator() == 0x35555555 );

        rational_ReduceInaccurate(aFract3, 29);
        CPPUNIT_ASSERT_MESSAGE( "Fraction #3 reduce to 29 bits failed",
                                aFract3.numerator() == 0x3D555555 &&
                                aFract3.denominator() == 0x1AAAAAAA );

        rational_ReduceInaccurate(aFract3, 9);
        CPPUNIT_ASSERT_MESSAGE( "Fraction #3 reduce to 9 bits failed",
                                aFract3.numerator() == 0x0147 &&
                                aFract3.denominator() == 0x008E );

        rational_ReduceInaccurate(aFract3, 1);
        CPPUNIT_ASSERT_MESSAGE( "Fraction #3 reduce to 1 bit failed",
                                aFract3.numerator() == 2 &&
                                aFract3.denominator() == 1 );

        rational_ReduceInaccurate(aFract3, 0);
        CPPUNIT_ASSERT_MESSAGE( "Fraction #3 reduce to 0 bits failed",
                                aFract3.numerator() == 2 &&
                                aFract3.denominator() == 1 );

#if SAL_TYPES_SIZEOFLONG == 8
        boost::rational<sal_Int64> aFract4(0x7AAAAAAAAAAAAAAA, 0x3555555555555555);
        CPPUNIT_ASSERT_MESSAGE( "Fraction #4 cancellation wrong",
                                aFract4.numerator() == 0x7AAAAAAAAAAAAAAA &&
                                aFract4.denominator() == 0x3555555555555555 );
        rational_ReduceInaccurate(aFract4, 62);
        CPPUNIT_ASSERT_MESSAGE( "Fraction #4 ReduceInaccurate errorneously cut precision",
                                aFract4.numerator() == 0x7AAAAAAAAAAAAAAA &&
                                aFract4.denominator() == 0x3555555555555555 );

        rational_ReduceInaccurate(aFract4, 61);
        CPPUNIT_ASSERT_MESSAGE( "Fraction #4 ReduceInaccurate reduce to 61 bit failed",
                                aFract4.numerator() == 0x3D55555555555555 &&
                                aFract4.denominator() == 0x1AAAAAAAAAAAAAAA );
#endif
    }

    CPPUNIT_TEST_SUITE(RationalTest);
    CPPUNIT_TEST(testReduceInaccurate);
    CPPUNIT_TEST_SUITE_END();
};


CPPUNIT_TEST_SUITE_REGISTRATION(RationalTest);
} // namespace tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
