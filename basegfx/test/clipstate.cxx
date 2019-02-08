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

#include <basegfx/utils/b2dclipstate.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

using namespace ::basegfx;

namespace basegfx2d
{

class clipstate : public CppUnit::TestFixture
{
private:
    utils::B2DClipState aUnion1;
    utils::B2DClipState aUnion2;
    utils::B2DClipState aIntersect;
    utils::B2DClipState aXor;
    utils::B2DClipState aSubtract;

public:
    void setUp() override
    {
        B2DRange aCenter(100, 100, -100, -100);
        B2DRange aNorth(-10, -110, 10, -90);
        B2DRange aWest(-110, -10, -90, 10);
        B2DRange aSouth(-10, 110, 10, 90);
        B2DRange aEast(110, -10, 90, 10);

        aUnion1.unionRange(aCenter);
        aUnion1.unionRange(aNorth);
        aUnion1.unionRange(aWest);
        aUnion1.unionRange(aSouth);
        aUnion1.unionRange(aEast);

        aUnion2.makeNull();
        aUnion2.unionRange(aCenter);
        aUnion2.unionRange(aNorth);
        aUnion2.unionRange(aWest);
        aUnion2.unionRange(aSouth);
        aUnion2.unionRange(aEast);

        aIntersect.intersectRange(aCenter);
        aIntersect.intersectRange(aNorth);
        aIntersect.intersectRange(aWest);
        aIntersect.intersectRange(aSouth);
        aIntersect.intersectRange(aEast);

        aXor.makeNull();
        aXor.xorRange(aCenter);
        aXor.xorRange(aNorth);
        aXor.xorRange(aWest);
        aXor.xorRange(aSouth);
        aXor.xorRange(aEast);

        aSubtract.intersectRange(aCenter);
        aSubtract.subtractRange(aNorth);
        aSubtract.subtractRange(aWest);
        aSubtract.subtractRange(aSouth);
        aSubtract.subtractRange(aEast);
    }

    void verifyPoly(const char* sName, const char* sSvg, const utils::B2DClipState& toTest) const
    {
#if OSL_DEBUG_LEVEL > 2
        fprintf(stderr, "%s - svg:d=\"%s\"\n",
                sName, OUStringToOString(
                    basegfx::utils::exportToSvgD(toTest.getClipPoly(), true, true, false),
                    RTL_TEXTENCODING_UTF8).getStr() );
#endif

        B2DPolyPolygon aTmp1;
        CPPUNIT_ASSERT_MESSAGE(sName,
                               utils::importFromSvgD(
                                   aTmp1, OUString::createFromAscii(sSvg), false, nullptr));

        const OUString aSvg=
            utils::exportToSvgD(toTest.getClipPoly(), true, true, false);
        B2DPolyPolygon aTmp2;
        CPPUNIT_ASSERT_MESSAGE(sName,
                               utils::importFromSvgD(
                                   aTmp2, aSvg, false, nullptr));

        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            sName,
            aTmp1, aTmp2);
    }

    void verifySimpleRange()
    {
        const char* const unionSvg="m100 10v90h-90v10h-20v-10h-90v-90h-10v-20h10v-90h90v-10h20v10h90v90h10v20z";
        const char* const intersectSvg="m-100 10v-20h10v20zm90 90v-10h20v10zm0-190v-10h20v10zm100 100v-20h10v20z";
        const char* const xorSvg="m-100 10h10v-20h-10zm90 90h20v-10h-20zm0-190h20v-10h-20zm100 100h10v-20h-10z"
                           "m10 0v90h-90v10h-20v-10h-90v-90h-10v-20h10v-90h90v-10h20v10h90v90h10v20z";
        const char* const subtractSvg="m-90 10v-20h-10v-90h90v10h20v-10h90v90h-10v20h10v90h-90v-10h-20v10h-90v-90z";

        CPPUNIT_ASSERT_MESSAGE("cleared clip stays empty under union operation",
                               aUnion1.isCleared());
        verifyPoly("union", unionSvg, aUnion2);
        verifyPoly("intersect", intersectSvg, aIntersect);
        verifyPoly("xor", xorSvg, aXor);
        verifyPoly("subtract", subtractSvg, aSubtract);
    }

    void verifyMixedClips()
    {
        utils::B2DClipState aMixedClip;

        const char unionSvg[]="m100 10v90h-90v10h-20v-10h-90v-90h-10v-20h10v-90h90v-10h20v10h90v90h10v20z";

        B2DPolyPolygon aTmp1;
        CPPUNIT_ASSERT(utils::importFromSvgD(aTmp1, unionSvg, false, nullptr));

        aMixedClip.intersectPolyPolygon(aTmp1);
        aMixedClip.subtractRange(B2DRange(-20,-150,20,0));
        aMixedClip.subtractRange(B2DRange(-150,-20,0,20));
        aMixedClip.xorRange(B2DRange(-150,-150,150,150));

        const char* const mixedClipSvg="m0 0v20h-100v80h90v10h20v-10h90v-90h10v-20h-10v-90h-80v100zm-20-20v-80h-80v80zm-130 170v-300h300v300z";
        verifyPoly("mixed clip", mixedClipSvg, aMixedClip);
    }

    CPPUNIT_TEST_SUITE(clipstate);
    CPPUNIT_TEST(verifySimpleRange);
    CPPUNIT_TEST(verifyMixedClips);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx2d::clipstate);
} // namespace basegfx2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
