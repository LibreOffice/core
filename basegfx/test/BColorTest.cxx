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

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <basegfx/color/bcolor.hxx>
#include <basegfx/color/bcolortools.hxx>

namespace basegfx
{
class bcolor : public CppUnit::TestFixture
{
    BColor maWhite;
    BColor maBlack;
    BColor maRed;
    BColor maGreen;
    BColor maBlue;
    BColor maYellow;
    BColor maMagenta;
    BColor maCyan;

public:
    bcolor()
        : maWhite(1, 1, 1)
        , maBlack(0, 0, 0)
        , maRed(1, 0, 0)
        , maGreen(0, 1, 0)
        , maBlue(0, 0, 1)
        , maYellow(1, 1, 0)
        , maMagenta(1, 0, 1)
        , maCyan(0, 1, 1)
    {
    }

    // insert your test code here.
    void hslTest()
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("white", BColor(0, 0, 1), utils::rgb2hsl(maWhite));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("black", BColor(0, 0, 0), utils::rgb2hsl(maBlack));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("red", BColor(0, 1, 0.5), utils::rgb2hsl(maRed));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("green", BColor(120, 1, 0.5), utils::rgb2hsl(maGreen));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("blue", BColor(240, 1, 0.5), utils::rgb2hsl(maBlue));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("yellow", BColor(60, 1, 0.5), utils::rgb2hsl(maYellow));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("magenta", BColor(300, 1, 0.5), utils::rgb2hsl(maMagenta));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("cyan", BColor(180, 1, 0.5), utils::rgb2hsl(maCyan));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("third hue case", BColor(210, 1, 0.5),
                                     utils::rgb2hsl(BColor(0, 0.5, 1)));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip white", maWhite,
                                     utils::hsl2rgb(utils::rgb2hsl(maWhite)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip black", maBlack,
                                     utils::hsl2rgb(utils::rgb2hsl(maBlack)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip red", maRed, utils::hsl2rgb(utils::rgb2hsl(maRed)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip green", maGreen,
                                     utils::hsl2rgb(utils::rgb2hsl(maGreen)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip blue", maBlue,
                                     utils::hsl2rgb(utils::rgb2hsl(maBlue)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip yellow", maYellow,
                                     utils::hsl2rgb(utils::rgb2hsl(maYellow)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip magenta", maMagenta,
                                     utils::hsl2rgb(utils::rgb2hsl(maMagenta)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip cyan", maCyan,
                                     utils::hsl2rgb(utils::rgb2hsl(maCyan)));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("grey10", BColor(0, 0, .1), utils::rgb2hsl(maWhite * .1));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("grey90", BColor(0, 0, .9), utils::rgb2hsl(maWhite * .9));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("red/2", BColor(0, 1, 0.25), utils::rgb2hsl(maRed * .5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("green/2", BColor(120, 1, 0.25), utils::rgb2hsl(maGreen * .5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("blue/2", BColor(240, 1, 0.25), utils::rgb2hsl(maBlue * .5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("yellow/2", BColor(60, 1, 0.25),
                                     utils::rgb2hsl(maYellow * .5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("magenta/2", BColor(300, 1, 0.25),
                                     utils::rgb2hsl(maMagenta * .5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("cyan/2", BColor(180, 1, 0.25), utils::rgb2hsl(maCyan * .5));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("pastel", BColor(0, .5, .5),
                                     utils::rgb2hsl(BColor(.75, .25, .25)));
    }

    // insert your test code here.
    void hsvTest()
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("white", BColor(0, 0, 1), utils::rgb2hsv(maWhite));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("black", BColor(0, 0, 0), utils::rgb2hsv(maBlack));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("red", BColor(0, 1, 1), utils::rgb2hsv(maRed));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("green", BColor(120, 1, 1), utils::rgb2hsv(maGreen));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("blue", BColor(240, 1, 1), utils::rgb2hsv(maBlue));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("yellow", BColor(60, 1, 1), utils::rgb2hsv(maYellow));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("magenta", BColor(300, 1, 1), utils::rgb2hsv(maMagenta));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("cyan", BColor(180, 1, 1), utils::rgb2hsv(maCyan));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip white", maWhite,
                                     utils::hsv2rgb(utils::rgb2hsv(maWhite)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip black", maBlack,
                                     utils::hsv2rgb(utils::rgb2hsv(maBlack)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip red", maRed, utils::hsv2rgb(utils::rgb2hsv(maRed)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip green", maGreen,
                                     utils::hsv2rgb(utils::rgb2hsv(maGreen)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip blue", maBlue,
                                     utils::hsv2rgb(utils::rgb2hsv(maBlue)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip yellow", maYellow,
                                     utils::hsv2rgb(utils::rgb2hsv(maYellow)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip magenta", maMagenta,
                                     utils::hsv2rgb(utils::rgb2hsv(maMagenta)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip cyan", maCyan,
                                     utils::hsv2rgb(utils::rgb2hsv(maCyan)));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("grey10", BColor(0, 0, .1), utils::rgb2hsv(maWhite * .1));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("grey90", BColor(0, 0, .9), utils::rgb2hsv(maWhite * .9));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("red/2", BColor(0, 1, 0.5), utils::rgb2hsv(maRed * .5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("green/2", BColor(120, 1, 0.5), utils::rgb2hsv(maGreen * .5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("blue/2", BColor(240, 1, 0.5), utils::rgb2hsv(maBlue * .5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("yellow/2", BColor(60, 1, 0.5), utils::rgb2hsv(maYellow * .5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("magenta/2", BColor(300, 1, 0.5),
                                     utils::rgb2hsv(maMagenta * .5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("cyan/2", BColor(180, 1, 0.5), utils::rgb2hsv(maCyan * .5));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("pastel", BColor(0, .5, .5),
                                     utils::rgb2hsv(BColor(.5, .25, .25)));
    }

    CPPUNIT_TEST_SUITE(bcolor);
    CPPUNIT_TEST(hslTest);
    CPPUNIT_TEST(hsvTest);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace basegfx

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::bcolor);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
