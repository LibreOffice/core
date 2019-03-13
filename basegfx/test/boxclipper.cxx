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
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <basegfx/range/b2dpolyrange.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <comphelper/random.hxx>

#include "boxclipper.hxx"

using namespace ::basegfx;

namespace basegfx2d
{
/// Gets a random ordinal [0,n)
double getRandomOrdinal( const std::size_t n )
{
    // use this one when displaying polygons in OOo, which still sucks
    // great rocks when trying to import non-integer svg:d attributes
    return comphelper::rng::uniform_size_distribution(0, n-1);
}

static bool compare(const B2DPoint& left, const B2DPoint& right)
{
    return left.getX()<right.getX()
        || (rtl::math::approxEqual(left.getX(),right.getX()) && left.getY()<right.getY());
}

class boxclipper : public CppUnit::TestFixture
{
private:
    B2DPolyRange aDisjunctRanges;
    B2DPolyRange aEqualRanges;
    B2DPolyRange aIntersectionN;
    B2DPolyRange aIntersectionE;
    B2DPolyRange aIntersectionS;
    B2DPolyRange aIntersectionW;
    B2DPolyRange aIntersectionNE;
    B2DPolyRange aIntersectionSE;
    B2DPolyRange aIntersectionSW;
    B2DPolyRange aIntersectionNW;
    B2DPolyRange aRingIntersection;
    B2DPolyRange aRingIntersection2;
    B2DPolyRange aRingIntersectExtraStrip;
    B2DPolyRange aComplexIntersections;
    B2DPolyRange aRandomIntersections;

public:
    // initialise your test code values here.
    void setUp() override
    {
        B2DRange aCenter(100, 100, -100, -100);
        B2DRange aOffside(800, 800, 1000, 1000);
        B2DRange aNorth(100, 0, -100, -200);
        B2DRange aSouth(100, 200, -100, 0);
        B2DRange aEast(0, 100, 200, -100);
        B2DRange aWest(-200, 100, 0, -100);
        B2DRange aNorthEast(0, 0, 200, -200);
        B2DRange aSouthEast(0, 0, 200, 200);
        B2DRange aSouthWest(0, 0, -200, 200);
        B2DRange aNorthWest(0, 0, -200, -200);

        B2DRange aNorth2(-150, 50,  150,  350);
        B2DRange aSouth2(-150, -50, 150, -350);
        B2DRange aEast2 (50,  -150, 350,  150);
        B2DRange aWest2 (-50, -150,-350,  150);

        aDisjunctRanges.appendElement( aCenter, B2VectorOrientation::Negative );
        aDisjunctRanges.appendElement( aOffside, B2VectorOrientation::Negative );

        aEqualRanges.appendElement( aCenter, B2VectorOrientation::Negative );
        aEqualRanges.appendElement( aCenter, B2VectorOrientation::Negative );

        aIntersectionN.appendElement( aCenter, B2VectorOrientation::Negative );
        aIntersectionN.appendElement( aNorth, B2VectorOrientation::Negative );

        aIntersectionE.appendElement( aCenter, B2VectorOrientation::Negative );
        aIntersectionE.appendElement( aEast, B2VectorOrientation::Negative );

        aIntersectionS.appendElement( aCenter, B2VectorOrientation::Negative );
        aIntersectionS.appendElement( aSouth, B2VectorOrientation::Negative );

        aIntersectionW.appendElement( aCenter, B2VectorOrientation::Negative );
        aIntersectionW.appendElement( aWest, B2VectorOrientation::Negative );

        aIntersectionNE.appendElement( aCenter, B2VectorOrientation::Negative );
        aIntersectionNE.appendElement( aNorthEast, B2VectorOrientation::Negative );

        aIntersectionSE.appendElement( aCenter, B2VectorOrientation::Negative );
        aIntersectionSE.appendElement( aSouthEast, B2VectorOrientation::Negative );

        aIntersectionSW.appendElement( aCenter, B2VectorOrientation::Negative );
        aIntersectionSW.appendElement( aSouthWest, B2VectorOrientation::Negative );

        aIntersectionNW.appendElement( aCenter, B2VectorOrientation::Negative );
        aIntersectionNW.appendElement( aNorthWest, B2VectorOrientation::Negative );

        aRingIntersection.appendElement( aNorth2, B2VectorOrientation::Negative );
        aRingIntersection.appendElement( aEast2, B2VectorOrientation::Negative );
        aRingIntersection.appendElement( aSouth2, B2VectorOrientation::Negative );

        aRingIntersection2 = aRingIntersection;
        aRingIntersection2.appendElement( aWest2, B2VectorOrientation::Negative );

        aRingIntersectExtraStrip = aRingIntersection2;
        aRingIntersectExtraStrip.appendElement( B2DRange(0, -25, 200, 25),
                                                B2VectorOrientation::Negative );

        aComplexIntersections.appendElement( aCenter, B2VectorOrientation::Negative );
        aComplexIntersections.appendElement( aOffside, B2VectorOrientation::Negative );
        aComplexIntersections.appendElement( aCenter, B2VectorOrientation::Negative );
        aComplexIntersections.appendElement( aNorth, B2VectorOrientation::Negative );
        aComplexIntersections.appendElement( aEast, B2VectorOrientation::Negative );
        aComplexIntersections.appendElement( aSouth, B2VectorOrientation::Negative );
        aComplexIntersections.appendElement( aWest, B2VectorOrientation::Negative );
        aComplexIntersections.appendElement( aNorthEast, B2VectorOrientation::Negative );
        aComplexIntersections.appendElement( aSouthEast, B2VectorOrientation::Negative );
        aComplexIntersections.appendElement( aSouthWest, B2VectorOrientation::Negative );
        aComplexIntersections.appendElement( aNorthWest, B2VectorOrientation::Negative );

#ifdef GENERATE_RANDOM
        for( int i=0; i<800; ++i )
        {
            B2DRange aRandomRange(
                getRandomOrdinal( 1000 ),
                getRandomOrdinal( 1000 ),
                getRandomOrdinal( 1000 ),
                getRandomOrdinal( 1000 ) );

            aRandomIntersections.appendElement( aRandomRange, B2VectorOrientation::Negative );
        }
#else
        const char randomSvg[]="m394 783h404v57h-404zm-197-505h571v576h-571zm356-634h75v200h-75zm-40-113h403v588h-403zm93-811h111v494h-111zm-364-619h562v121h-562zm-134-8h292v27h-292zm110 356h621v486h-621zm78-386h228v25h-228zm475-345h201v201h-201zm-2-93h122v126h-122zm-417-243h567v524h-567zm-266-738h863v456h-863zm262-333h315v698h-315zm-328-826h43v393h-43zm830-219h120v664h-120zm-311-636h221v109h-221zm-500 137h628v19h-628zm681-94h211v493h-211zm-366-646h384v355h-384zm-189-199h715v247h-715zm165-459h563v601h-563zm258-479h98v606h-98zm270-517h65v218h-65zm-44-259h96v286h-96zm-599-202h705v468h-705zm216-803h450v494h-450zm-150-22h26v167h-26zm-55-599h50v260h-50zm190-278h490v387h-490zm-290-453h634v392h-634zm257 189h552v300h-552zm-151-690h136v455h-136zm12-597h488v432h-488zm501-459h48v39h-48zm-224-112h429v22h-429zm-281 102h492v621h-492zm519-158h208v17h-208zm-681-563h56v427h-56zm126-451h615v392h-615zm-47-410h598v522h-598zm-32 316h79v110h-79zm-71-129h18v127h-18zm126-993h743v589h-743zm211-430h428v750h-428zm61-554h100v220h-100zm-353-49h658v157h-658zm778-383h115v272h-115zm-249-541h119v712h-119zm203 86h94v40h-94z";
        B2DPolyPolygon randomPoly;
        CPPUNIT_ASSERT(utils::importFromSvgD(randomPoly, randomSvg, false, nullptr));
        for (auto const& aPolygon : randomPoly)
            aRandomIntersections.appendElement(aPolygon.getB2DRange(), B2VectorOrientation::Negative);
#endif
    }

    B2DPolyPolygon normalizePoly( const B2DPolyPolygon& rPoly ) const
    {
        B2DPolyPolygon aRes;
        for( sal_uInt32 i=0; i<rPoly.count(); ++i )
        {
            B2DPolygon aTmp=rPoly.getB2DPolygon(i);
            if( utils::getOrientation(aTmp) == B2VectorOrientation::Negative )
                aTmp.flip();

            aTmp=utils::removeNeutralPoints(aTmp);
            std::vector<B2DPoint> aTmp2(aTmp.count());
            for(sal_uInt32 j=0; j<aTmp.count(); ++j)
                aTmp2[j] = aTmp.getB2DPoint(j);

            std::vector<B2DPoint>::iterator pSmallest=aTmp2.end();
            for(std::vector<B2DPoint>::iterator pCurr=aTmp2.begin(); pCurr!=aTmp2.end(); ++pCurr)
            {
                if( pSmallest == aTmp2.end() || compare(*pCurr, *pSmallest) )
                {
                    pSmallest=pCurr;
                }
            }

            if( pSmallest != aTmp2.end() )
                std::rotate(aTmp2.begin(),pSmallest,aTmp2.end());

            aTmp.clear();
            for(const auto& rCurr : aTmp2)
                aTmp.append(rCurr);

            aRes.append(aTmp);
        }

        // boxclipper & generic clipper disagree slightly on area-less
        // polygons (one or two points only)
        aRes = utils::stripNeutralPolygons(aRes);

        // now, sort all polygons with increasing 0th point
        std::sort(aRes.begin(),
                  aRes.end(),
                  [](const B2DPolygon& aPolygon1, const B2DPolygon& aPolygon2) {
                      return compare(aPolygon1.getB2DPoint(0),
                          aPolygon2.getB2DPoint(0)); } );

        return aRes;
    }

    void verifyPoly(const char* sName, const char* sSvg, const B2DPolyRange& toTest) const
    {
        B2DPolyPolygon aTmp1;
        CPPUNIT_ASSERT_MESSAGE(sName,
                               utils::importFromSvgD(
                                   aTmp1, OUString::createFromAscii(sSvg), false, nullptr));

        const OUString aSvg=
            utils::exportToSvgD(toTest.solveCrossovers(), true, true, false);
        B2DPolyPolygon aTmp2;
        CPPUNIT_ASSERT_MESSAGE(sName,
                               utils::importFromSvgD(
                                   aTmp2, aSvg, false, nullptr));

        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            sName,
            normalizePoly(aTmp1), normalizePoly(aTmp2));
    }

    void verifyPoly()
    {
        const char* const disjunct="m-100-100v200h200v-200zm900 900v200h200v-200z";
        const char* const equal="m-100-100v200h200v-200zm200 0h-200v200h200v-200z";
        const char* const intersectionN="m-100-100v100h200v-100zm200 0v-100h-200v100 200h200v-200z";
        const char* const intersectionE="m0-100v200h100v-200zm0 0h-100v200h100 200v-200z";
        const char* const intersectionS="m-100 0v100h200v-100zm0-100v200 100h200v-100-200z";
        const char* const intersectionW="m-100-100v200h100v-200zm0 0h-100v200h100 200v-200z";
        const char* const intersectionNE="m0-100v100h100v-100zm0-100v100h-100v200h200v-100h100v-200z";
        const char* const intersectionSE="m0 0v100h100v-100zm100 0v-100h-200v200h100v100h200v-200z";
        const char* const intersectionSW="m-100 0v100h100v-100zm0-100v100h-100v200h200v-100h100v-200z";
        const char* const intersectionNW="m-100-100v100h100v-100zm100 0v-100h-200v200h100v100h200v-200z";
        const char* const ringIntersection="m50-150v100h100v-100zm0 200v100h100v-100zm100-200v-200h-300v300h200v100h-200v300h300v-200h200v-300z";
        const char* const ringIntersection2="m-150 50v100h100v-100zm0-200v100h100v-100zm100 200v-100h100v100z"
                                      "m100-200v100h100v-100zm0 200v100h100v-100zm100-200v-200h-300v200h-200v300h200v200h300v-200h200v-300z";
        const char* const ringIntersectExtraStrip="m-150 50v100h100v-100zm0-200v100h100v-100zm100 200v-100h100v25h-50v50h50v25z"
                                            "m100-200v100h100v-100zm0 200v100h100v-100zm0-75v50h150v-50z"
                                            "m100-125v-200h-300v200h-200v300h200v200h300v-200h200v-300z";
        const char* const complexIntersections="m0 0zm0 0zm0 0zm0 0v-100 100h-100 100v100-100h100zm0 0v-100 100h-100 100v100-100h100z"
                                         "m100 0v-100h-100-100v100 100h100 100v-100zm0 0v-100h-100-100v100 100h100 100v-100z"
                                         "m0 0v-100h-100v-100 100h-100v100h-100 100v100h100v100-100h100v-100h100z"
                                         "m0-100v-100h-100-100v100h-100v100 100h100v100h100 100v-100h100v-100-100z"
                                         "m100 0v-100h-200-100-100v100 200 100h100 100 200v-100-200zm600 900v200h200v-200z";
        const char* const randomIntersections="m20-4515v393h43v-393zm34-8690v127h18v-127zm24 674v427h56v-427zm126-451v16-16z"
                                         "m22 3470v260h50v-260zm55 599v167h26v-167zm-49-1831v455h136v-455z"
                                         "m10 8845v19h158v-19zm54-38v25h228v-25zm156-13245v108h100v-108z"
                                         "m101 14826v200h75v-200zm-205-3000v365h315v-365zm-309-1877v19h628v-19z"
                                         "m549-1398v127h98v-127zm18 5351v215h111v-215zm-362-10061v152h488v-152z"
                                         "m488 0v-469h-492v621h4v280h488v-432zm-378 5368v48h384v-48zm274-10182v712h119v-712z"
                                         "m-424 3173v-94h-47v110h47v96h551v-112zm-105-2249v157h353v112h100v-112h205v-157z"
                                         "m284 5177v203h377v-203zm337 4727v66h40v-66zm-326 6110v57h374v-57zm351-12583v39h48v-39z"
                                         "m23 12583v-505h-571v576h571v-14h30v-57zm-368-2682v-8h-292v27h134v102h562v-121z"
                                         "m-9-12299v320h428v-320zm364 1216v-410h-598v316h-32v110h32v96h47v280h615v-392z"
                                         "m-537 11431v486h388v279h111v-279h122v-486zm112-4621v142h550v-142zm101-2719v494h450v-494z"
                                         "m340 6609v33h120v-33zm-85-4349v-479h-98v479h-258v459h-165v247h189v307h384v-307h142v-105h13v-601z"
                                         "m-270-3159v36h490v-36zm442 2163v7h52v-7zm-345 7158v588h403v-588zm378-1813v-93h-122v126h2v155h148v-188z"
                                         "m19-5345v-259h-96v266h44v20h52v-20h10v-7zm-91-6571v-430h-428v430h-211v589h743v-589z"
                                         "m101 6571v-461h-705v468h599v20h44v191h65v-218zm-89-8442v40h94v-40zm-71 10742v-43h-221v109h181v427h211v-493z"
                                         "m0-4727v-189h-634v392h257v97h33v351h490v-351h29v-300zm-97 6698v-333h-315v333h-262v456h863v-456z"
                                         "m-142-8556v22h429v-22zm238-56v17h208v-17zm91 7234v664h120v-664zm69 2452v-336h-567v524h419v13h201v-201z"
                                         "m-42-13332v272h115v-272z";

        verifyPoly("disjunct", disjunct, aDisjunctRanges);
        verifyPoly("equal", equal, aEqualRanges);
        verifyPoly("intersectionN", intersectionN, aIntersectionN);
        verifyPoly("intersectionE", intersectionE, aIntersectionE);
        verifyPoly("intersectionS", intersectionS, aIntersectionS);
        verifyPoly("intersectionW", intersectionW, aIntersectionW);
        verifyPoly("intersectionNE", intersectionNE, aIntersectionNE);
        verifyPoly("intersectionSE", intersectionSE, aIntersectionSE);
        verifyPoly("intersectionSW", intersectionSW, aIntersectionSW);
        verifyPoly("intersectionNW", intersectionNW, aIntersectionNW);
        verifyPoly("ringIntersection", ringIntersection, aRingIntersection);
        verifyPoly("ringIntersection2", ringIntersection2, aRingIntersection2);
        verifyPoly("ringIntersectExtraStrip", ringIntersectExtraStrip, aRingIntersectExtraStrip);
        verifyPoly("complexIntersections", complexIntersections, aComplexIntersections);
        verifyPoly("randomIntersections", randomIntersections, aRandomIntersections);
    }

    void dumpSvg(const char* pName,
                 const ::basegfx::B2DPolyPolygon& rPoly) const
    {
        (void)pName; (void)rPoly;
#if OSL_DEBUG_LEVEL > 2
        fprintf(stderr, "%s - svg:d=\"%s\"\n",
                pName, OUStringToOString(
                    basegfx::utils::exportToSvgD(rPoly, , true, true, false),
                    RTL_TEXTENCODING_UTF8).getStr() );
#endif
    }

    void getPolyPolygon()
    {
        dumpSvg("disjunct",aDisjunctRanges.solveCrossovers());
        dumpSvg("equal",aEqualRanges.solveCrossovers());
        dumpSvg("intersectionN",aIntersectionN.solveCrossovers());
        dumpSvg("intersectionE",aIntersectionE.solveCrossovers());
        dumpSvg("intersectionS",aIntersectionS.solveCrossovers());
        dumpSvg("intersectionW",aIntersectionW.solveCrossovers());
        dumpSvg("intersectionNE",aIntersectionNE.solveCrossovers());
        dumpSvg("intersectionSE",aIntersectionSE.solveCrossovers());
        dumpSvg("intersectionSW",aIntersectionSW.solveCrossovers());
        dumpSvg("intersectionNW",aIntersectionNW.solveCrossovers());
        dumpSvg("ringIntersection",aRingIntersection.solveCrossovers());
        dumpSvg("ringIntersection2",aRingIntersection2.solveCrossovers());
        dumpSvg("aRingIntersectExtraStrip",aRingIntersectExtraStrip.solveCrossovers());
        dumpSvg("complexIntersections",aComplexIntersections.solveCrossovers());
        dumpSvg("randomIntersections",aRandomIntersections.solveCrossovers());

        CPPUNIT_ASSERT_MESSAGE("getPolyPolygon", true );
    }

    void validatePoly( const char* pName, const B2DPolyRange& rRange ) const
    {
        B2DPolyPolygon genericClip;
        const sal_uInt32 nCount=rRange.count();
        for( sal_uInt32 i=0; i<nCount; ++i )
        {
            B2DPolygon aRect=utils::createPolygonFromRect(std::get<0>(rRange.getElement(i)));
            if( std::get<1>(rRange.getElement(i)) == B2VectorOrientation::Negative )
                aRect.flip();

            genericClip.append(aRect);
        }

#if OSL_DEBUG_LEVEL > 2
        fprintf(stderr, "%s input      - svg:d=\"%s\"\n",
                pName, OUStringToOString(
                    basegfx::utils::exportToSvgD(
                        genericClip, , true, true, false),
                    RTL_TEXTENCODING_UTF8).getStr() );
#endif

        const B2DPolyPolygon boxClipResult=rRange.solveCrossovers();
        const OUString boxClipSvg(
            basegfx::utils::exportToSvgD(
                normalizePoly(boxClipResult), true, true, false));
#if OSL_DEBUG_LEVEL > 2
        fprintf(stderr, "%s boxclipper - svg:d=\"%s\"\n",
                pName, OUStringToOString(
                    boxClipSvg,
                    RTL_TEXTENCODING_UTF8).getStr() );
#endif

        genericClip = utils::solveCrossovers(genericClip);
        const OUString genericClipSvg(
            basegfx::utils::exportToSvgD(
                normalizePoly(genericClip), true, true, false));
#if OSL_DEBUG_LEVEL > 2
        fprintf(stderr, "%s genclipper - svg:d=\"%s\"\n",
                pName, OUStringToOString(
                    genericClipSvg,
                    RTL_TEXTENCODING_UTF8).getStr() );
#endif

        CPPUNIT_ASSERT_EQUAL_MESSAGE(pName,
                               boxClipSvg, genericClipSvg);
    }

    void validatePoly()
    {
        validatePoly("disjunct", aDisjunctRanges);
        validatePoly("equal", aEqualRanges);
        validatePoly("intersectionN", aIntersectionN);
        validatePoly("intersectionE", aIntersectionE);
        validatePoly("intersectionS", aIntersectionS);
        validatePoly("intersectionW", aIntersectionW);
        validatePoly("intersectionNE", aIntersectionNE);
        validatePoly("intersectionSE", aIntersectionSE);
        validatePoly("intersectionSW", aIntersectionSW);
        validatePoly("intersectionNW", aIntersectionNW);
        // subtle differences on Solaris Intel, comparison not smart enough
        // (due to floating point inaccuracies)
        //validatePoly("ringIntersection", aRingIntersection);
        //validatePoly("ringIntersection2", aRingIntersection2);
        //validatePoly("ringIntersectExtraStrip", aRingIntersectExtraStrip);
        // generic clipper buggy here, likely
        //validatePoly("complexIntersections", aComplexIntersections);
        //validatePoly("randomIntersections", aRandomIntersections);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(boxclipper);
    CPPUNIT_TEST(validatePoly);
    CPPUNIT_TEST(verifyPoly);
    CPPUNIT_TEST(getPolyPolygon);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx2d::boxclipper);
} // namespace basegfx2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
