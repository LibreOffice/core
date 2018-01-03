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
#include <cppunit/plugin/TestPlugIn.h>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/curve/b2dbeziertools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dtrapezoid.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/range/b2ibox.hxx>
#include <basegfx/range/b1drange.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2dpolyrange.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/color/bcolortools.hxx>

#include <basegfx/utils/rectcliptools.hxx>

#include <iostream>
#include <fstream>

#include "boxclipper.hxx"

using namespace ::basegfx;
using basegfx2d::getRandomOrdinal;

namespace basegfx
{

class b2drange : public CppUnit::TestFixture
{
private:
public:
    void check()
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("simple range rounding from double to integer",
                               B2IRange(1, 2, 4, 5), fround(B2DRange(1.2, 2.3, 3.5, 4.8)));
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2drange);
    CPPUNIT_TEST(check);
    CPPUNIT_TEST_SUITE_END();
};

class b2dpolyrange : public CppUnit::TestFixture
{
private:
public:
    void check()
    {
        B2DPolyRange aRange;
        aRange.appendElement(B2DRange(0,0,1,1),B2VectorOrientation::Positive);
        aRange.appendElement(B2DRange(2,2,3,3),B2VectorOrientation::Positive);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("simple poly range - count",
                               sal_uInt32(2), aRange.count());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("simple poly range - first element",
                               B2DRange(0,0,1,1), std::get<0>(aRange.getElement(0)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("simple poly range - second element",
                               B2DRange(2,2,3,3), std::get<0>(aRange.getElement(1)));

        // B2DPolyRange relies on correctly orientated rects
        const B2DRange aRect(0,0,1,1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("createPolygonFromRect - correct orientation",
                               B2VectorOrientation::Positive, utils::getOrientation(utils::createPolygonFromRect(aRect)));
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2dpolyrange);
    CPPUNIT_TEST(check);
    CPPUNIT_TEST_SUITE_END();
};

class b2dhommatrix : public CppUnit::TestFixture
{
private:
    B2DHomMatrix    maIdentity;
    B2DHomMatrix    maScale;
    B2DHomMatrix    maTranslate;
    B2DHomMatrix    maShear;
    B2DHomMatrix    maAffine;
    B2DHomMatrix    maPerspective;

public:
    // initialise your test code values here.
    void setUp() override
    {
        // setup some test matrices
        maIdentity.identity(); // force compact layout
        maIdentity.set(0,0, 1.0);
        maIdentity.set(0,1, 0.0);
        maIdentity.set(0,2, 0.0);
        maIdentity.set(1,0, 0.0);
        maIdentity.set(1,1, 1.0);
        maIdentity.set(1,2, 0.0);

        maScale.identity(); // force compact layout
        maScale.set(0,0, 2.0);
        maScale.set(1,1, 20.0);

        maTranslate.identity(); // force compact layout
        maTranslate.set(0,2, 20.0);
        maTranslate.set(1,2, 2.0);

        maShear.identity(); // force compact layout
        maShear.set(0,1, 3.0);
        maShear.set(1,0, 7.0);
        maShear.set(1,1, 22.0);

        maAffine.identity(); // force compact layout
        maAffine.set(0,0, 1.0);
        maAffine.set(0,1, 2.0);
        maAffine.set(0,2, 3.0);
        maAffine.set(1,0, 4.0);
        maAffine.set(1,1, 5.0);
        maAffine.set(1,2, 6.0);

        maPerspective.set(0,0, 1.0);
        maPerspective.set(0,1, 2.0);
        maPerspective.set(0,2, 3.0);
        maPerspective.set(1,0, 4.0);
        maPerspective.set(1,1, 5.0);
        maPerspective.set(1,2, 6.0);
        maPerspective.set(2,0, 7.0);
        maPerspective.set(2,1, 8.0);
        maPerspective.set(2,2, 9.0);
    }

    void equal()
    {
        B2DHomMatrix    aIdentity;
        B2DHomMatrix    aScale;
        B2DHomMatrix    aTranslate;
        B2DHomMatrix    aShear;
        B2DHomMatrix    aAffine;
        B2DHomMatrix    aPerspective;

        // setup some test matrices
        aIdentity.identity(); // force compact layout
        aIdentity.set(0,0, 1.0);
        aIdentity.set(0,1, 0.0);
        aIdentity.set(0,2, 0.0);
        aIdentity.set(1,0, 0.0);
        aIdentity.set(1,1, 1.0);
        aIdentity.set(1,2, 0.0);

        aScale.identity(); // force compact layout
        aScale.set(0,0, 2.0);
        aScale.set(1,1, 20.0);

        aTranslate.identity(); // force compact layout
        aTranslate.set(0,2, 20.0);
        aTranslate.set(1,2, 2.0);

        aShear.identity(); // force compact layout
        aShear.set(0,1, 3.0);
        aShear.set(1,0, 7.0);
        aShear.set(1,1, 22.0);

        aAffine.identity(); // force compact layout
        aAffine.set(0,0, 1.0);
        aAffine.set(0,1, 2.0);
        aAffine.set(0,2, 3.0);
        aAffine.set(1,0, 4.0);
        aAffine.set(1,1, 5.0);
        aAffine.set(1,2, 6.0);

        aPerspective.set(0,0, 1.0);
        aPerspective.set(0,1, 2.0);
        aPerspective.set(0,2, 3.0);
        aPerspective.set(1,0, 4.0);
        aPerspective.set(1,1, 5.0);
        aPerspective.set(1,2, 6.0);
        aPerspective.set(2,0, 7.0);
        aPerspective.set(2,1, 8.0);
        aPerspective.set(2,2, 9.0);

        CPPUNIT_ASSERT_MESSAGE("operator==: identity matrix", aIdentity.operator ==(maIdentity));
        CPPUNIT_ASSERT_MESSAGE("operator==: scale matrix", aScale.operator ==(maScale));
        CPPUNIT_ASSERT_MESSAGE("operator==: translate matrix", aTranslate.operator ==(maTranslate));
        CPPUNIT_ASSERT_MESSAGE("operator==: shear matrix", aShear.operator ==(maShear));
        CPPUNIT_ASSERT_MESSAGE("operator==: affine matrix", aAffine.operator ==(maAffine));
        CPPUNIT_ASSERT_MESSAGE("operator==: perspective matrix", aPerspective.operator ==(maPerspective));
    }

    void identity()
    {
        B2DHomMatrix ident;

        CPPUNIT_ASSERT_EQUAL_MESSAGE("identity", maIdentity, ident);
    }

    void scale()
    {
        B2DHomMatrix mat;
        mat.scale(2.0,20.0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("scale", maScale, mat);
    }

    void rotate()
    {
        B2DHomMatrix mat;
        mat.rotate(90*F_PI180);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate pi/2 yields exact matrix", 0.0, mat.get(0,0), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate pi/2 yields exact matrix", -1.0, mat.get(0,1), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate pi/2 yields exact matrix", 0.0, mat.get(0,2), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate pi/2 yields exact matrix", 1.0, mat.get(1,0), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate pi/2 yields exact matrix", 0.0, mat.get(1,1), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate pi/2 yields exact matrix", 0.0, mat.get(1,2), 1E-12);
        mat.rotate(90*F_PI180);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate pi yields exact matrix", -1.0, mat.get(0,0), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate pi yields exact matrix", 0.0, mat.get(0,1), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate pi yields exact matrix", 0.0, mat.get(0,2), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate pi yields exact matrix", 0.0, mat.get(1,0), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate pi yields exact matrix", -1.0, mat.get(1,1), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate pi yields exact matrix", 0.0, mat.get(1,2), 1E-12);
        mat.rotate(90*F_PI180);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate 3/2 pi yields exact matrix", 0.0, mat.get(0,0), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate 3/2 pi yields exact matrix", 1.0, mat.get(0,1), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate 3/2 pi yields exact matrix", 0.0, mat.get(0,2), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate 3/2 pi yields exact matrix", -1.0, mat.get(1,0), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate 3/2 pi yields exact matrix", 0.0, mat.get(1,1), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate 3/2 pi yields exact matrix", 0.0, mat.get(1,2), 1E-12);
        mat.rotate(90*F_PI180);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate 2 pi yields exact matrix", 1.0, mat.get(0,0), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate 2 pi yields exact matrix", 0.0, mat.get(0,1), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate 2 pi yields exact matrix", 0.0, mat.get(0,2), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate 2 pi yields exact matrix", 0.0, mat.get(1,0), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate 2 pi yields exact matrix", 1.0, mat.get(1,1), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "rotate 2 pi yields exact matrix", 0.0, mat.get(1,2), 1E-12);
    }

    void translate()
    {
        B2DHomMatrix mat;
        mat.translate(20.0,2.0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("translate", maTranslate, mat);
    }

    void shear()
    {
        B2DHomMatrix mat;
        mat.shearX(3.0);
        mat.shearY(7.0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("translate", maShear, mat);
    }

    void multiply()
    {
        B2DHomMatrix affineAffineProd;

        affineAffineProd.set(0,0, 9);
        affineAffineProd.set(0,1, 12);
        affineAffineProd.set(0,2, 18);
        affineAffineProd.set(1,0, 24);
        affineAffineProd.set(1,1, 33);
        affineAffineProd.set(1,2, 48);

        B2DHomMatrix affinePerspectiveProd;

        affinePerspectiveProd.set(0,0, 30);
        affinePerspectiveProd.set(0,1, 36);
        affinePerspectiveProd.set(0,2, 42);
        affinePerspectiveProd.set(1,0, 66);
        affinePerspectiveProd.set(1,1, 81);
        affinePerspectiveProd.set(1,2, 96);
        affinePerspectiveProd.set(2,0, 7);
        affinePerspectiveProd.set(2,1, 8);
        affinePerspectiveProd.set(2,2, 9);

        B2DHomMatrix perspectiveAffineProd;

        perspectiveAffineProd.set(0,0, 9);
        perspectiveAffineProd.set(0,1, 12);
        perspectiveAffineProd.set(0,2, 18);
        perspectiveAffineProd.set(1,0, 24);
        perspectiveAffineProd.set(1,1, 33);
        perspectiveAffineProd.set(1,2, 48);
        perspectiveAffineProd.set(2,0, 39);
        perspectiveAffineProd.set(2,1, 54);
        perspectiveAffineProd.set(2,2, 78);

        B2DHomMatrix perspectivePerspectiveProd;

        perspectivePerspectiveProd.set(0,0, 30);
        perspectivePerspectiveProd.set(0,1, 36);
        perspectivePerspectiveProd.set(0,2, 42);
        perspectivePerspectiveProd.set(1,0, 66);
        perspectivePerspectiveProd.set(1,1, 81);
        perspectivePerspectiveProd.set(1,2, 96);
        perspectivePerspectiveProd.set(2,0, 102);
        perspectivePerspectiveProd.set(2,1, 126);
        perspectivePerspectiveProd.set(2,2, 150);

        B2DHomMatrix temp;

        temp = maAffine;
        temp*=maAffine;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("multiply: both compact", affineAffineProd, temp);

        temp = maPerspective;
        temp*=maAffine;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("multiply: first compact", affinePerspectiveProd, temp);

        temp = maAffine;
        temp*=maPerspective;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("multiply: second compact", perspectiveAffineProd, temp);

        temp = maPerspective;
        temp*=maPerspective;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("multiply: none compact", perspectivePerspectiveProd, temp);
    }

    void impFillMatrix(B2DHomMatrix& rSource, double fScaleX, double fScaleY, double fShearX, double fRotate) const
    {
        // fill rSource with a linear combination of scale, shear and rotate
        rSource.identity();
        rSource.scale(fScaleX, fScaleY);
        rSource.shearX(fShearX);
        rSource.rotate(fRotate);
    }

    bool impDecomposeComposeTest(double fScaleX, double fScaleY, double fShearX, double fRotate) const
    {
        // linear combine matrix with given values
        B2DHomMatrix aSource;
        impFillMatrix(aSource, fScaleX, fScaleY, fShearX, fRotate);

        // decompose that matrix
        B2DTuple aDScale;
        B2DTuple aDTrans;
        double fDRot;
        double fDShX;
        bool bWorked = aSource.decompose(aDScale, aDTrans, fDRot, fDShX);

        // linear combine another matrix with decomposition results
        B2DHomMatrix aRecombined;
        impFillMatrix(aRecombined, aDScale.getX(), aDScale.getY(), fDShX, fDRot);

        // if decomposition worked, matrices need to be the same
        return bWorked && aSource == aRecombined;
    }

    void decompose()
    {
        // test matrix decompositions. Each matrix decomposed and rebuilt
        // using the decompose result should be the same as before. Test
        // with all ranges of values. Translations are not tested since these
        // are just the two rightmost values and uncritical
        static double fSX(10.0);
        static double fSY(12.0);
        static double fR(45.0 * F_PI180);
        static double fS(15.0 * F_PI180);

        // check all possible scaling combinations
        CPPUNIT_ASSERT_MESSAGE("decompose: error test A1", impDecomposeComposeTest(fSX, fSY, 0.0, 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test A2", impDecomposeComposeTest(-fSX, fSY, 0.0, 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test A3", impDecomposeComposeTest(fSX, -fSY, 0.0, 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test A4", impDecomposeComposeTest(-fSX, -fSY, 0.0, 0.0));

        // check all possible scaling combinations with positive rotation
        CPPUNIT_ASSERT_MESSAGE("decompose: error test B1", impDecomposeComposeTest(fSX, fSY, 0.0, fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test B2", impDecomposeComposeTest(-fSX, fSY, 0.0, fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test B3", impDecomposeComposeTest(fSX, -fSY, 0.0, fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test B4", impDecomposeComposeTest(-fSX, -fSY, 0.0, fR));

        // check all possible scaling combinations with negative rotation
        CPPUNIT_ASSERT_MESSAGE("decompose: error test C1", impDecomposeComposeTest(fSX, fSY, 0.0, -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test C2", impDecomposeComposeTest(-fSX, fSY, 0.0, -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test C3", impDecomposeComposeTest(fSX, -fSY, 0.0, -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test C4", impDecomposeComposeTest(-fSX, -fSY, 0.0, -fR));

        // check all possible scaling combinations with positive shear
        CPPUNIT_ASSERT_MESSAGE("decompose: error test D1", impDecomposeComposeTest(fSX, fSY, tan(fS), 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test D2", impDecomposeComposeTest(-fSX, fSY, tan(fS), 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test D3", impDecomposeComposeTest(fSX, -fSY, tan(fS), 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test D4", impDecomposeComposeTest(-fSX, -fSY, tan(fS), 0.0));

        // check all possible scaling combinations with negative shear
        CPPUNIT_ASSERT_MESSAGE("decompose: error test E1", impDecomposeComposeTest(fSX, fSY, tan(-fS), 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test E2", impDecomposeComposeTest(-fSX, fSY, tan(-fS), 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test E3", impDecomposeComposeTest(fSX, -fSY, tan(-fS), 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test E4", impDecomposeComposeTest(-fSX, -fSY, tan(-fS), 0.0));

        // check all possible scaling combinations with positive rotate and positive shear
        CPPUNIT_ASSERT_MESSAGE("decompose: error test F1", impDecomposeComposeTest(fSX, fSY, tan(fS), fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test F2", impDecomposeComposeTest(-fSX, fSY, tan(fS), fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test F3", impDecomposeComposeTest(fSX, -fSY, tan(fS), fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test F4", impDecomposeComposeTest(-fSX, -fSY, tan(fS), fR));

        // check all possible scaling combinations with negative rotate and positive shear
        CPPUNIT_ASSERT_MESSAGE("decompose: error test G1", impDecomposeComposeTest(fSX, fSY, tan(fS), -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test G2", impDecomposeComposeTest(-fSX, fSY, tan(fS), -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test G3", impDecomposeComposeTest(fSX, -fSY, tan(fS), -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test G4", impDecomposeComposeTest(-fSX, -fSY, tan(fS), -fR));

        // check all possible scaling combinations with positive rotate and negative shear
        CPPUNIT_ASSERT_MESSAGE("decompose: error test H1", impDecomposeComposeTest(fSX, fSY, tan(-fS), fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test H2", impDecomposeComposeTest(-fSX, fSY, tan(-fS), fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test H3", impDecomposeComposeTest(fSX, -fSY, tan(-fS), fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test H4", impDecomposeComposeTest(-fSX, -fSY, tan(-fS), fR));

        // check all possible scaling combinations with negative rotate and negative shear
        CPPUNIT_ASSERT_MESSAGE("decompose: error test I1", impDecomposeComposeTest(fSX, fSY, tan(-fS), -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test I2", impDecomposeComposeTest(-fSX, fSY, tan(-fS), -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test I3", impDecomposeComposeTest(fSX, -fSY, tan(-fS), -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test I4", impDecomposeComposeTest(-fSX, -fSY, tan(-fS), -fR));

        // cover special case of 180 degree rotation
        B2DHomMatrix aTest=utils::createScaleShearXRotateTranslateB2DHomMatrix(
            6425,3938,
            0,
            180*F_PI180,
            10482,4921);
        // decompose that matrix
        B2DTuple aDScale;
        B2DTuple aDTrans;
        double fDRot;
        double fDShX;
        aTest.decompose(aDScale, aDTrans, fDRot, fDShX);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("decompose: error test J1", 6425.0, aDScale.getX(), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("decompose: error test J1", 3938.0, aDScale.getY(), 1E-12);
        CPPUNIT_ASSERT_MESSAGE("decompose: error test J1", aDTrans.getX() == 10482 && aDTrans.getY() == 4921);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("decompose: error test J1", 180*F_PI180, fDRot, 1E-12 );
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2dhommatrix);
    CPPUNIT_TEST(equal);
    CPPUNIT_TEST(identity);
    CPPUNIT_TEST(scale);
    CPPUNIT_TEST(translate);
    CPPUNIT_TEST(rotate);
    CPPUNIT_TEST(shear);
    CPPUNIT_TEST(multiply);
    CPPUNIT_TEST(decompose);
    CPPUNIT_TEST_SUITE_END();

}; // class b2dhommatrix

class b2dpoint : public CppUnit::TestFixture
{
public:
    // insert your test code here.
    // this is only demonstration code
    void EmptyMethod()
    {
          // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2dpoint);
    CPPUNIT_TEST(EmptyMethod);
    CPPUNIT_TEST_SUITE_END();
}; // class b2dpoint

class b2dpolygon : public CppUnit::TestFixture
{
public:
    // insert your test code here.
    void testBasics()
    {
        B2DPolygon aPoly;

        aPoly.appendBezierSegment(B2DPoint(1,1),B2DPoint(2,2),B2DPoint(3,3));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("#1 first polygon point wrong",
                               B2DPoint(3,3), aPoly.getB2DPoint(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("#1 first control point wrong",
                               B2DPoint(2,2), aPoly.getPrevControlPoint(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("#1 second control point wrong",
                               B2DPoint(3,3), aPoly.getNextControlPoint(0));
        CPPUNIT_ASSERT_MESSAGE("next control point not used",
                               !aPoly.isNextControlPointUsed(0));

        aPoly.setNextControlPoint(0,B2DPoint(4,4));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("#1.1 second control point wrong",
                               B2DPoint(4,4), aPoly.getNextControlPoint(0));
        CPPUNIT_ASSERT_MESSAGE("next control point used",
                               aPoly.isNextControlPointUsed(0));
        CPPUNIT_ASSERT_MESSAGE("areControlPointsUsed() wrong",
                               aPoly.areControlPointsUsed());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("getContinuityInPoint() wrong",
                               B2VectorContinuity::C2, aPoly.getContinuityInPoint(0));

        aPoly.resetControlPoints();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("resetControlPoints() did not clear",
                               B2DPoint(3,3), aPoly.getB2DPoint(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("resetControlPoints() did not clear",
                               B2DPoint(3,3), aPoly.getPrevControlPoint(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("resetControlPoints() did not clear",
                               B2DPoint(3,3), aPoly.getNextControlPoint(0));
        CPPUNIT_ASSERT_MESSAGE("areControlPointsUsed() wrong #2",
                               !aPoly.areControlPointsUsed());

        aPoly.clear();
        aPoly.append(B2DPoint(0,0));
        aPoly.appendBezierSegment(B2DPoint(1,1),B2DPoint(2,2),B2DPoint(3,3));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("#2 first polygon point wrong",
                               B2DPoint(0,0), aPoly.getB2DPoint(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("#2 first control point wrong",
                               B2DPoint(0,0), aPoly.getPrevControlPoint(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("#2 second control point wrong",
                               B2DPoint(1,1), aPoly.getNextControlPoint(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("#2 third control point wrong",
                               B2DPoint(2,2), aPoly.getPrevControlPoint(1));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("#2 fourth control point wrong",
                               B2DPoint(3,3), aPoly.getNextControlPoint(1));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("#2 second polygon point wrong",
                               B2DPoint(3,3), aPoly.getB2DPoint(1));
    }
    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2dpolygon);
    CPPUNIT_TEST(testBasics);
    CPPUNIT_TEST_SUITE_END();
}; // class b2dpolygon

class b2dpolygontools : public CppUnit::TestFixture
{
public:
    // insert your test code here.
    // this is only demonstration code
    void testIsRectangle()
    {
        B2DPolygon aRect1(
            utils::createPolygonFromRect(
                B2DRange(0,0,1,1) ) );

        B2DPolygon aRect2 {
            {0, 0},
            {1, 0},
            {1, 0.5},
            {1, 1},
            {0, 1}
        };
        aRect2.setClosed(true);

        B2DPolygon aNonRect1 {
            {0, 0},
            {1, 0},
            {0.5, 1},
            {0.5, 0}
        };
        aNonRect1.setClosed(true);

        B2DPolygon aNonRect2 {
            {0, 0},
            {1, 1},
            {1, 0},
            {0, 1}
        };
        aNonRect2.setClosed(true);

        B2DPolygon aNonRect3 {
            {0, 0},
            {1, 0},
            {1, 1}
        };
        aNonRect3.setClosed(true);

        B2DPolygon aNonRect4 {
            {0, 0},
            {1, 0},
            {1, 1},
            {0, 1}
        };

        B2DPolygon aNonRect5 {
            {0, 0},
            {1, 0},
            {1, 1},
            {0, 1}
        };
        aNonRect5.setControlPoints(1, B2DPoint(1,0), B2DPoint(-11,0));
        aNonRect5.setClosed(true);

        CPPUNIT_ASSERT_MESSAGE("checking rectangle-ness of rectangle 1",
                               utils::isRectangle( aRect1 ));
        CPPUNIT_ASSERT_MESSAGE("checking rectangle-ness of rectangle 2",
                               utils::isRectangle( aRect2 ));
        CPPUNIT_ASSERT_MESSAGE("checking non-rectangle-ness of polygon 1",
                               !utils::isRectangle( aNonRect1 ));
        CPPUNIT_ASSERT_MESSAGE("checking non-rectangle-ness of polygon 2",
                               !utils::isRectangle( aNonRect2 ));
        CPPUNIT_ASSERT_MESSAGE("checking non-rectangle-ness of polygon 3",
                               !utils::isRectangle( aNonRect3 ));
        CPPUNIT_ASSERT_MESSAGE("checking non-rectangle-ness of polygon 4",
                               !utils::isRectangle( aNonRect4 ));
        CPPUNIT_ASSERT_MESSAGE("checking non-rectangle-ness of polygon 5",
                               !utils::isRectangle( aNonRect5 ));
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2dpolygontools);
    CPPUNIT_TEST(testIsRectangle);
    CPPUNIT_TEST_SUITE_END();
}; // class b2dpolygontools

class b2dpolypolygon : public CppUnit::TestFixture
{
public:
    // insert your test code here.
    void testTrapezoidHelper()
    {
        B2DPolygon aPolygon;
        // provoke the PointBlockAllocator to exercise the freeIfLast path
        for(int i = 0; i < 16 * 10; i++)
        {
            B2DPoint aPoint(getRandomOrdinal(1000), getRandomOrdinal(1000));
            aPolygon.append(aPoint);
        }
        // scatter some duplicate points in to stress things more.
        for(int i = 0; i < 16 * 10; i++)
        {
            aPolygon.insert(getRandomOrdinal(aPolygon.count() - 1),
                            aPolygon.getB2DPoint(getRandomOrdinal(aPolygon.count() - 1)));
        }
        B2DPolygon aPolygonOffset;
        // duplicate the polygon and offset it slightly.
        for(size_t i = 0; i < aPolygon.count(); i++)
        {
            B2DPoint aPoint(aPolygon.getB2DPoint(i));
            aPoint += B2DPoint(0.5-getRandomOrdinal(1),0.5-getRandomOrdinal(1));
        }
        B2DPolyPolygon aPolyPolygon;
        aPolyPolygon.append(aPolygon);
        aPolyPolygon.append(aPolygonOffset);
        B2DTrapezoidVector aVector;
        basegfx::utils::trapezoidSubdivide(aVector, aPolyPolygon);
        CPPUNIT_ASSERT_MESSAGE("more than zero sub-divided trapezoids",
                               aVector.size() > 0);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2dpolypolygon);
    CPPUNIT_TEST(testTrapezoidHelper);
    CPPUNIT_TEST_SUITE_END();
}; // class b2dpolypolygon

class b1Xrange : public CppUnit::TestFixture
{
public:
    template<class Type> void implCheck()
    {
        // test interval axioms
        // (http://en.wikipedia.org/wiki/Interval_%28mathematics%29)
        Type aRange;
        CPPUNIT_ASSERT_MESSAGE("default ctor - empty range", aRange.isEmpty());
        CPPUNIT_ASSERT_MESSAGE("center - get cop-out value since range is empty", aRange.getCenter()==0);

        // degenerate interval
        aRange.expand(1);
        CPPUNIT_ASSERT_MESSAGE("degenerate range - still, not empty!", !aRange.isEmpty());
        CPPUNIT_ASSERT_MESSAGE("degenerate range - size of 0", aRange.getRange() == 0);
        CPPUNIT_ASSERT_MESSAGE("same value as degenerate range - is inside range", aRange.isInside(1));
        CPPUNIT_ASSERT_MESSAGE("center - must be the single range value", aRange.getCenter()==1);

        // proper interval
        aRange.expand(2);
        CPPUNIT_ASSERT_MESSAGE("proper range - size of 1", aRange.getRange() == 1);
        CPPUNIT_ASSERT_MESSAGE("smaller value of range - is inside *closed* range", aRange.isInside(1));
        CPPUNIT_ASSERT_MESSAGE("larger value of range - is inside *closed* range", aRange.isInside(2));

        // center for proper interval that works for ints, too
        aRange.expand(3);
        CPPUNIT_ASSERT_MESSAGE("center - must be half of the range", aRange.getCenter()==2);

        // check overlap
        Type aRange2(0,1);
        CPPUNIT_ASSERT_MESSAGE("range overlapping *includes* upper bound", aRange.overlaps(aRange2));
        CPPUNIT_ASSERT_MESSAGE("range overlapping *includes* upper bound, but only barely", !aRange.overlapsMore(aRange2));

        Type aRange3(0,2);
        CPPUNIT_ASSERT_MESSAGE("range overlapping is fully overlapping now", aRange.overlapsMore(aRange3));

        // check intersect
        Type aRange4(3,4);
        aRange.intersect(aRange4);
        CPPUNIT_ASSERT_MESSAGE("range intersection is yielding empty range!", !aRange.isEmpty());

        Type aRange5(5,6);
        aRange.intersect(aRange5);
        CPPUNIT_ASSERT_MESSAGE("range intersection is yielding nonempty range!", aRange.isEmpty());
    }

    void check()
    {
        implCheck<B1DRange>();
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b1Xrange);
    CPPUNIT_TEST(check);
    CPPUNIT_TEST_SUITE_END();
}; // class b1Xrange

class b2Xrange : public CppUnit::TestFixture
{
public:
    template<class Type> void implCheck()
    {
        // cohen sutherland clipping
        Type aRange(0,0,10,10);

        CPPUNIT_ASSERT_MESSAGE("(0,0) is outside range!",
                               utils::getCohenSutherlandClipFlags(B2IPoint(0,0),aRange) == 0);
        CPPUNIT_ASSERT_MESSAGE("(-1,-1) is inside range!",
                               utils::getCohenSutherlandClipFlags(B2IPoint(-1,-1),aRange) ==
                               (utils::RectClipFlags::LEFT|utils::RectClipFlags::TOP));
        CPPUNIT_ASSERT_MESSAGE("(10,10) is outside range!",
                               utils::getCohenSutherlandClipFlags(B2IPoint(10,10),aRange) == 0);
        CPPUNIT_ASSERT_MESSAGE("(11,11) is inside range!",
                               utils::getCohenSutherlandClipFlags(B2IPoint(11,11),aRange) ==
                               (utils::RectClipFlags::RIGHT|utils::RectClipFlags::BOTTOM));
    }

    void check()
    {
        implCheck<B2DRange>();
        implCheck<B2IRange>();
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2Xrange);
    CPPUNIT_TEST(check);
    CPPUNIT_TEST_SUITE_END();
}; // class b2Xrange

class b2ibox : public CppUnit::TestFixture
{
public:
    void TestBox()
    {
        // cohen sutherland clipping
        B2IBox aBox(0,0,10,10);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("(0,0) is outside range!",
                               sal_uInt32(0), utils::getCohenSutherlandClipFlags(B2IPoint(0,0),aBox));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("(-1,-1) is inside range!",
                               utils::RectClipFlags::LEFT|utils::RectClipFlags::TOP,
                               utils::getCohenSutherlandClipFlags(B2IPoint(-1,-1),aBox));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("(9,9) is outside range!",
                               sal_uInt32(0), utils::getCohenSutherlandClipFlags(B2IPoint(9,9),aBox));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("(10,10) is inside range!",
                               utils::RectClipFlags::RIGHT|utils::RectClipFlags::BOTTOM,
                               utils::getCohenSutherlandClipFlags(B2IPoint(10,10),aBox));
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.
    CPPUNIT_TEST_SUITE(b2ibox);
    CPPUNIT_TEST(TestBox);
    CPPUNIT_TEST_SUITE_END();
}; // class b2ibox

class b2dtuple : public CppUnit::TestFixture
{
public:
    // insert your test code here.
    // this is only demonstration code
    void EmptyMethod()
    {
          // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2dtuple);
    CPPUNIT_TEST(EmptyMethod);
    CPPUNIT_TEST_SUITE_END();
}; // class b2dtuple

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
    bcolor() :
        maWhite(1,1,1),
        maBlack(0,0,0),
        maRed(1,0,0),
        maGreen(0,1,0),
        maBlue(0,0,1),
        maYellow(1,1,0),
        maMagenta(1,0,1),
        maCyan(0,1,1)
    {}

    // insert your test code here.
    void hslTest()
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("white",
                               BColor(0,0,1), utils::rgb2hsl(maWhite));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("black",
                               BColor(0,0,0), utils::rgb2hsl(maBlack));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("red",
                               BColor(0,1,0.5), utils::rgb2hsl(maRed));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("green",
                               BColor(120,1,0.5), utils::rgb2hsl(maGreen));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("blue",
                               BColor(240,1,0.5), utils::rgb2hsl(maBlue));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("yellow",
                               BColor(60,1,0.5), utils::rgb2hsl(maYellow));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("magenta",
                               BColor(300,1,0.5), utils::rgb2hsl(maMagenta));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("cyan",
                               BColor(180,1,0.5), utils::rgb2hsl(maCyan));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("third hue case",
                               BColor(210,1,0.5), utils::rgb2hsl(BColor(0,0.5,1)));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip white",
                               maWhite, utils::hsl2rgb(utils::rgb2hsl(maWhite)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip black",
                               maBlack, utils::hsl2rgb(utils::rgb2hsl(maBlack)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip red",
                               maRed, utils::hsl2rgb(utils::rgb2hsl(maRed)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip green",
                               maGreen, utils::hsl2rgb(utils::rgb2hsl(maGreen)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip blue",
                               maBlue, utils::hsl2rgb(utils::rgb2hsl(maBlue)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip yellow",
                               maYellow, utils::hsl2rgb(utils::rgb2hsl(maYellow)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip magenta",
                               maMagenta, utils::hsl2rgb(utils::rgb2hsl(maMagenta)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip cyan",
                               maCyan, utils::hsl2rgb(utils::rgb2hsl(maCyan)));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("grey10",
                               BColor(0,0,.1), utils::rgb2hsl(maWhite*.1));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("grey90",
                               BColor(0,0,.9), utils::rgb2hsl(maWhite*.9));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("red/2",
                               BColor(0,1,0.25), utils::rgb2hsl(maRed*.5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("green/2",
                               BColor(120,1,0.25), utils::rgb2hsl(maGreen*.5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("blue/2",
                               BColor(240,1,0.25), utils::rgb2hsl(maBlue*.5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("yellow/2",
                               BColor(60,1,0.25), utils::rgb2hsl(maYellow*.5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("magenta/2",
                               BColor(300,1,0.25), utils::rgb2hsl(maMagenta*.5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("cyan/2",
                               BColor(180,1,0.25), utils::rgb2hsl(maCyan*.5));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("pastel",
                               BColor(0,.5,.5), utils::rgb2hsl(BColor(.75,.25,.25)));
    }

    // insert your test code here.
    void hsvTest()
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("white",
                               BColor(0,0,1), utils::rgb2hsv(maWhite));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("black",
                               BColor(0,0,0), utils::rgb2hsv(maBlack));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("red",
                               BColor(0,1,1), utils::rgb2hsv(maRed));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("green",
                               BColor(120,1,1), utils::rgb2hsv(maGreen));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("blue",
                               BColor(240,1,1), utils::rgb2hsv(maBlue));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("yellow",
                               BColor(60,1,1), utils::rgb2hsv(maYellow));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("magenta",
                               BColor(300,1,1), utils::rgb2hsv(maMagenta));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("cyan",
                               BColor(180,1,1), utils::rgb2hsv(maCyan));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip white",
                               maWhite, utils::hsv2rgb(utils::rgb2hsv(maWhite)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip black",
                               maBlack, utils::hsv2rgb(utils::rgb2hsv(maBlack)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip red",
                               maRed, utils::hsv2rgb(utils::rgb2hsv(maRed)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip green",
                               maGreen, utils::hsv2rgb(utils::rgb2hsv(maGreen)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip blue",
                               maBlue, utils::hsv2rgb(utils::rgb2hsv(maBlue)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip yellow",
                               maYellow, utils::hsv2rgb(utils::rgb2hsv(maYellow)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip magenta",
                               maMagenta, utils::hsv2rgb(utils::rgb2hsv(maMagenta)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("roundtrip cyan",
                               maCyan, utils::hsv2rgb(utils::rgb2hsv(maCyan)));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("grey10",
                               BColor(0,0,.1), utils::rgb2hsv(maWhite*.1));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("grey90",
                               BColor(0,0,.9), utils::rgb2hsv(maWhite*.9));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("red/2",
                               BColor(0,1,0.5), utils::rgb2hsv(maRed*.5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("green/2",
                               BColor(120,1,0.5), utils::rgb2hsv(maGreen*.5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("blue/2",
                               BColor(240,1,0.5), utils::rgb2hsv(maBlue*.5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("yellow/2",
                               BColor(60,1,0.5), utils::rgb2hsv(maYellow*.5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("magenta/2",
                               BColor(300,1,0.5), utils::rgb2hsv(maMagenta*.5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("cyan/2",
                               BColor(180,1,0.5), utils::rgb2hsv(maCyan*.5));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("pastel",
                               BColor(0,.5,.5), utils::rgb2hsv(BColor(.5,.25,.25)));
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(bcolor);
    CPPUNIT_TEST(hslTest);
    CPPUNIT_TEST(hsvTest);
    CPPUNIT_TEST_SUITE_END();
}; // class b2dvector

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::b2drange);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::b2dpolyrange);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::b2dhommatrix);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::b2dpoint);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::b2dpolygon);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::b2dpolygontools);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::b2dpolypolygon);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::b1Xrange);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::b2Xrange);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::b2ibox);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::b2dtuple);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::bcolor);
} // namespace basegfx2d

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
