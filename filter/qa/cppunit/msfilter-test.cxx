/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <sal/types.h>
#include <tools/color.hxx>
#include <filter/msfilter/util.hxx>

#include <unotest/bootstrapfixturebase.hxx>

namespace {

class MSFilterTest
    : public test::BootstrapFixtureBase
{
public:
    void testTransColToIco();

    CPPUNIT_TEST_SUITE(MSFilterTest);
    CPPUNIT_TEST(testTransColToIco);
    CPPUNIT_TEST_SUITE_END();
};

void MSFilterTest::testTransColToIco()
{
    const sal_uInt32 aStdCol[] = {
        0xeeeeee, 0xffff99, 0xff6600, 0xff3333, 0xff00cc, 0xff33ff, 0x9900ff, 0x6666ff, 0x00ccff, 0x66ffff, 0x33ff99, 0x99ff66, 0xccff00,
        0xdddddd, 0xffff66, 0xffcc00, 0xff9999, 0xff66cc, 0xff99ff, 0xcc66ff, 0x9999ff, 0x9999ff, 0x99ffff, 0x66ff99, 0x99ff99, 0xccff66,
        0xcccccc, 0xffff00, 0xff9900, 0xff6666, 0xff3399, 0xff66ff, 0x9933ff, 0x3333ff, 0x3399ff, 0x00ffff, 0x00ff66, 0x66ff66, 0x99ff33,
        0xb2b2b2, 0xcc9900, 0xff3300, 0xff0000, 0xff0066, 0xff00ff, 0x6600ff, 0x0000ff, 0x0066ff, 0x00cccc, 0x00cc33, 0x00cc00, 0x66ff00,
        0x999999, 0x996600, 0xcc3300, 0xcc0000, 0xcc0066, 0xcc00cc, 0x6600cc, 0x0000cc, 0x0066cc, 0x009999, 0x009933, 0x009900, 0x66cc00,
        0x808080, 0x663300, 0x801900, 0x990000, 0x990066, 0x990099, 0x330099, 0x000099, 0x006699, 0x006666, 0x007826, 0x006600, 0x669900,
        0x666666, 0x333300, 0x461900, 0x330000, 0x330033, 0x660066, 0x000033, 0x000066, 0x000080, 0x003333, 0x00331a, 0x003300, 0x193300,
        0x333333, 0x666633, 0x661900, 0x663333, 0x660033, 0x663366, 0x330066, 0x333366, 0x003366, 0x336666, 0x006633, 0x336633, 0x336600 };

    const sal_uInt16 aExpected[] = {
         8,  7,  6,  6,  5,  5,  5, 2,  3,  3, 10,  4,  7,
        16,  7,  7,  6,  5,  5,  5, 2,  2,  3,  4,  4,  7,
        16,  7,  7,  6, 12,  5, 12, 2, 10,  3,  4,  4, 14,
        16, 14,  6,  6,  6,  5,  2, 2,  2,  3,  4,  4,  4,
        15, 14,  6,  6, 12,  5, 12, 2, 10, 10, 11, 11, 14,
        15,  1, 13, 13, 12, 12,  9, 9, 10, 10, 11, 11, 14,
        15, 14, 13, 13, 12, 12,  9, 9,  9, 10, 10, 11, 11,
         1, 14, 13, 13,  1, 12,  1, 9,  1, 10,  1, 11,  1 };

    for( size_t i = 0; i < SAL_N_ELEMENTS(aStdCol); ++i)
    {
        const OString sMessage = "Index of unmatched color: " + OString::number(i);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), aExpected[i],
                                     static_cast<sal_uInt16>(msfilter::util::TransColToIco( Color(ColorTransparency, aStdCol[i]) )));
    }

    // tdf#92471
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), static_cast<sal_uInt16>(msfilter::util::TransColToIco( Color( 0x6666ff ))));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), static_cast<sal_uInt16>(msfilter::util::TransColToIco( Color( 0x6566ff ))));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), static_cast<sal_uInt16>(msfilter::util::TransColToIco( Color( 0x6665ff ))));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), static_cast<sal_uInt16>(msfilter::util::TransColToIco( Color( 0x6666fe ))));

}

CPPUNIT_TEST_SUITE_REGISTRATION(MSFilterTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
