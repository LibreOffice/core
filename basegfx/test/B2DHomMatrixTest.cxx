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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/tuple/b2dtuple.hxx>

namespace basegfx
{
class b2dhommatrix : public CppUnit::TestFixture
{
private:
    B2DHomMatrix maIdentity;
    B2DHomMatrix maScale;
    B2DHomMatrix maTranslate;
    B2DHomMatrix maShear;
    B2DHomMatrix maAffine;
    B2DHomMatrix maPerspective;

public:
    // initialise your test code values here.
    void setUp() override
    {
        // setup some test matrices
        maIdentity.identity(); // force compact layout
        maIdentity.set(0, 0, 1.0);
        maIdentity.set(0, 1, 0.0);
        maIdentity.set(0, 2, 0.0);
        maIdentity.set(1, 0, 0.0);
        maIdentity.set(1, 1, 1.0);
        maIdentity.set(1, 2, 0.0);

        maScale.identity(); // force compact layout
        maScale.set(0, 0, 2.0);
        maScale.set(1, 1, 20.0);

        maTranslate.identity(); // force compact layout
        maTranslate.set(0, 2, 20.0);
        maTranslate.set(1, 2, 2.0);

        maShear.identity(); // force compact layout
        maShear.set(0, 1, 3.0);
        maShear.set(1, 0, 7.0);
        maShear.set(1, 1, 22.0);

        maAffine.identity(); // force compact layout
        maAffine.set(0, 0, 1.0);
        maAffine.set(0, 1, 2.0);
        maAffine.set(0, 2, 3.0);
        maAffine.set(1, 0, 4.0);
        maAffine.set(1, 1, 5.0);
        maAffine.set(1, 2, 6.0);

        maPerspective.set(0, 0, 1.0);
        maPerspective.set(0, 1, 2.0);
        maPerspective.set(0, 2, 3.0);
        maPerspective.set(1, 0, 4.0);
        maPerspective.set(1, 1, 5.0);
        maPerspective.set(1, 2, 6.0);
        maPerspective.set(2, 0, 7.0);
        maPerspective.set(2, 1, 8.0);
        maPerspective.set(2, 2, 9.0);
    }

    void equal()
    {
        B2DHomMatrix aIdentity;
        B2DHomMatrix aScale;
        B2DHomMatrix aTranslate;
        B2DHomMatrix aShear;
        B2DHomMatrix aAffine;
        B2DHomMatrix aPerspective;

        // setup some test matrices
        aIdentity.identity(); // force compact layout
        aIdentity.set(0, 0, 1.0);
        aIdentity.set(0, 1, 0.0);
        aIdentity.set(0, 2, 0.0);
        aIdentity.set(1, 0, 0.0);
        aIdentity.set(1, 1, 1.0);
        aIdentity.set(1, 2, 0.0);

        aScale.identity(); // force compact layout
        aScale.set(0, 0, 2.0);
        aScale.set(1, 1, 20.0);

        aTranslate.identity(); // force compact layout
        aTranslate.set(0, 2, 20.0);
        aTranslate.set(1, 2, 2.0);

        aShear.identity(); // force compact layout
        aShear.set(0, 1, 3.0);
        aShear.set(1, 0, 7.0);
        aShear.set(1, 1, 22.0);

        aAffine.identity(); // force compact layout
        aAffine.set(0, 0, 1.0);
        aAffine.set(0, 1, 2.0);
        aAffine.set(0, 2, 3.0);
        aAffine.set(1, 0, 4.0);
        aAffine.set(1, 1, 5.0);
        aAffine.set(1, 2, 6.0);

        aPerspective.set(0, 0, 1.0);
        aPerspective.set(0, 1, 2.0);
        aPerspective.set(0, 2, 3.0);
        aPerspective.set(1, 0, 4.0);
        aPerspective.set(1, 1, 5.0);
        aPerspective.set(1, 2, 6.0);
        aPerspective.set(2, 0, 7.0);
        aPerspective.set(2, 1, 8.0);
        aPerspective.set(2, 2, 9.0);

        CPPUNIT_ASSERT_MESSAGE("operator==: identity matrix", aIdentity.operator==(maIdentity));
        CPPUNIT_ASSERT_MESSAGE("operator==: scale matrix", aScale.operator==(maScale));
        CPPUNIT_ASSERT_MESSAGE("operator==: translate matrix", aTranslate.operator==(maTranslate));
        CPPUNIT_ASSERT_MESSAGE("operator==: shear matrix", aShear.operator==(maShear));
        CPPUNIT_ASSERT_MESSAGE("operator==: affine matrix", aAffine.operator==(maAffine));
        CPPUNIT_ASSERT_MESSAGE("operator==: perspective matrix",
                               aPerspective.operator==(maPerspective));
    }

    void identity()
    {
        B2DHomMatrix ident;

        CPPUNIT_ASSERT_EQUAL_MESSAGE("identity", maIdentity, ident);
    }

    void scale()
    {
        B2DHomMatrix mat;
        mat.scale(2.0, 20.0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("scale", maScale, mat);
    }

    void rotate()
    {
        B2DHomMatrix mat;
        mat.rotate(F_PI2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate pi/2 yields exact matrix", 0.0, mat.get(0, 0),
                                             1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate pi/2 yields exact matrix", -1.0, mat.get(0, 1),
                                             1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate pi/2 yields exact matrix", 0.0, mat.get(0, 2),
                                             1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate pi/2 yields exact matrix", 1.0, mat.get(1, 0),
                                             1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate pi/2 yields exact matrix", 0.0, mat.get(1, 1),
                                             1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate pi/2 yields exact matrix", 0.0, mat.get(1, 2),
                                             1E-12);
        mat.rotate(F_PI2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate pi yields exact matrix", -1.0, mat.get(0, 0),
                                             1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate pi yields exact matrix", 0.0, mat.get(0, 1),
                                             1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate pi yields exact matrix", 0.0, mat.get(0, 2),
                                             1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate pi yields exact matrix", 0.0, mat.get(1, 0),
                                             1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate pi yields exact matrix", -1.0, mat.get(1, 1),
                                             1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate pi yields exact matrix", 0.0, mat.get(1, 2),
                                             1E-12);
        mat.rotate(F_PI2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate 3/2 pi yields exact matrix", 0.0,
                                             mat.get(0, 0), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate 3/2 pi yields exact matrix", 1.0,
                                             mat.get(0, 1), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate 3/2 pi yields exact matrix", 0.0,
                                             mat.get(0, 2), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate 3/2 pi yields exact matrix", -1.0,
                                             mat.get(1, 0), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate 3/2 pi yields exact matrix", 0.0,
                                             mat.get(1, 1), 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate 3/2 pi yields exact matrix", 0.0,
                                             mat.get(1, 2), 1E-12);
        mat.rotate(F_PI2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate 2 pi yields exact matrix", 1.0, mat.get(0, 0),
                                             1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate 2 pi yields exact matrix", 0.0, mat.get(0, 1),
                                             1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate 2 pi yields exact matrix", 0.0, mat.get(0, 2),
                                             1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate 2 pi yields exact matrix", 0.0, mat.get(1, 0),
                                             1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate 2 pi yields exact matrix", 1.0, mat.get(1, 1),
                                             1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("rotate 2 pi yields exact matrix", 0.0, mat.get(1, 2),
                                             1E-12);
    }

    void translate()
    {
        B2DHomMatrix mat;
        mat.translate(20.0, 2.0);
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

        affineAffineProd.set(0, 0, 9);
        affineAffineProd.set(0, 1, 12);
        affineAffineProd.set(0, 2, 18);
        affineAffineProd.set(1, 0, 24);
        affineAffineProd.set(1, 1, 33);
        affineAffineProd.set(1, 2, 48);

        B2DHomMatrix affinePerspectiveProd;

        affinePerspectiveProd.set(0, 0, 30);
        affinePerspectiveProd.set(0, 1, 36);
        affinePerspectiveProd.set(0, 2, 42);
        affinePerspectiveProd.set(1, 0, 66);
        affinePerspectiveProd.set(1, 1, 81);
        affinePerspectiveProd.set(1, 2, 96);
        affinePerspectiveProd.set(2, 0, 7);
        affinePerspectiveProd.set(2, 1, 8);
        affinePerspectiveProd.set(2, 2, 9);

        B2DHomMatrix perspectiveAffineProd;

        perspectiveAffineProd.set(0, 0, 9);
        perspectiveAffineProd.set(0, 1, 12);
        perspectiveAffineProd.set(0, 2, 18);
        perspectiveAffineProd.set(1, 0, 24);
        perspectiveAffineProd.set(1, 1, 33);
        perspectiveAffineProd.set(1, 2, 48);
        perspectiveAffineProd.set(2, 0, 39);
        perspectiveAffineProd.set(2, 1, 54);
        perspectiveAffineProd.set(2, 2, 78);

        B2DHomMatrix perspectivePerspectiveProd;

        perspectivePerspectiveProd.set(0, 0, 30);
        perspectivePerspectiveProd.set(0, 1, 36);
        perspectivePerspectiveProd.set(0, 2, 42);
        perspectivePerspectiveProd.set(1, 0, 66);
        perspectivePerspectiveProd.set(1, 1, 81);
        perspectivePerspectiveProd.set(1, 2, 96);
        perspectivePerspectiveProd.set(2, 0, 102);
        perspectivePerspectiveProd.set(2, 1, 126);
        perspectivePerspectiveProd.set(2, 2, 150);

        B2DHomMatrix temp;

        temp = maAffine;
        temp *= maAffine;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("multiply: both compact", affineAffineProd, temp);

        temp = maPerspective;
        temp *= maAffine;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("multiply: first compact", affinePerspectiveProd, temp);

        temp = maAffine;
        temp *= maPerspective;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("multiply: second compact", perspectiveAffineProd, temp);

        temp = maPerspective;
        temp *= maPerspective;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("multiply: none compact", perspectivePerspectiveProd, temp);
    }

    void impFillMatrix(B2DHomMatrix& rSource, double fScaleX, double fScaleY, double fShearX,
                       double fRotate) const
    {
        // fill rSource with a linear combination of scale, shear and rotate
        rSource.identity();
        rSource.scale(fScaleX, fScaleY);
        rSource.shearX(fShearX);
        rSource.rotate(fRotate);
    }

    bool impDecomposeComposeTest(double fScaleX, double fScaleY, double fShearX,
                                 double fRotate) const
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
        static const double fSX(10.0);
        static const double fSY(12.0);
        static const double fR(F_PI4);
        static const double fS(deg2rad(15.0));

        // check all possible scaling combinations
        CPPUNIT_ASSERT_MESSAGE("decompose: error test A1",
                               impDecomposeComposeTest(fSX, fSY, 0.0, 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test A2",
                               impDecomposeComposeTest(-fSX, fSY, 0.0, 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test A3",
                               impDecomposeComposeTest(fSX, -fSY, 0.0, 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test A4",
                               impDecomposeComposeTest(-fSX, -fSY, 0.0, 0.0));

        // check all possible scaling combinations with positive rotation
        CPPUNIT_ASSERT_MESSAGE("decompose: error test B1",
                               impDecomposeComposeTest(fSX, fSY, 0.0, fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test B2",
                               impDecomposeComposeTest(-fSX, fSY, 0.0, fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test B3",
                               impDecomposeComposeTest(fSX, -fSY, 0.0, fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test B4",
                               impDecomposeComposeTest(-fSX, -fSY, 0.0, fR));

        // check all possible scaling combinations with negative rotation
        CPPUNIT_ASSERT_MESSAGE("decompose: error test C1",
                               impDecomposeComposeTest(fSX, fSY, 0.0, -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test C2",
                               impDecomposeComposeTest(-fSX, fSY, 0.0, -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test C3",
                               impDecomposeComposeTest(fSX, -fSY, 0.0, -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test C4",
                               impDecomposeComposeTest(-fSX, -fSY, 0.0, -fR));

        // check all possible scaling combinations with positive shear
        CPPUNIT_ASSERT_MESSAGE("decompose: error test D1",
                               impDecomposeComposeTest(fSX, fSY, tan(fS), 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test D2",
                               impDecomposeComposeTest(-fSX, fSY, tan(fS), 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test D3",
                               impDecomposeComposeTest(fSX, -fSY, tan(fS), 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test D4",
                               impDecomposeComposeTest(-fSX, -fSY, tan(fS), 0.0));

        // check all possible scaling combinations with negative shear
        CPPUNIT_ASSERT_MESSAGE("decompose: error test E1",
                               impDecomposeComposeTest(fSX, fSY, tan(-fS), 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test E2",
                               impDecomposeComposeTest(-fSX, fSY, tan(-fS), 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test E3",
                               impDecomposeComposeTest(fSX, -fSY, tan(-fS), 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test E4",
                               impDecomposeComposeTest(-fSX, -fSY, tan(-fS), 0.0));

        // check all possible scaling combinations with positive rotate and positive shear
        CPPUNIT_ASSERT_MESSAGE("decompose: error test F1",
                               impDecomposeComposeTest(fSX, fSY, tan(fS), fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test F2",
                               impDecomposeComposeTest(-fSX, fSY, tan(fS), fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test F3",
                               impDecomposeComposeTest(fSX, -fSY, tan(fS), fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test F4",
                               impDecomposeComposeTest(-fSX, -fSY, tan(fS), fR));

        // check all possible scaling combinations with negative rotate and positive shear
        CPPUNIT_ASSERT_MESSAGE("decompose: error test G1",
                               impDecomposeComposeTest(fSX, fSY, tan(fS), -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test G2",
                               impDecomposeComposeTest(-fSX, fSY, tan(fS), -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test G3",
                               impDecomposeComposeTest(fSX, -fSY, tan(fS), -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test G4",
                               impDecomposeComposeTest(-fSX, -fSY, tan(fS), -fR));

        // check all possible scaling combinations with positive rotate and negative shear
        CPPUNIT_ASSERT_MESSAGE("decompose: error test H1",
                               impDecomposeComposeTest(fSX, fSY, tan(-fS), fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test H2",
                               impDecomposeComposeTest(-fSX, fSY, tan(-fS), fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test H3",
                               impDecomposeComposeTest(fSX, -fSY, tan(-fS), fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test H4",
                               impDecomposeComposeTest(-fSX, -fSY, tan(-fS), fR));

        // check all possible scaling combinations with negative rotate and negative shear
        CPPUNIT_ASSERT_MESSAGE("decompose: error test I1",
                               impDecomposeComposeTest(fSX, fSY, tan(-fS), -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test I2",
                               impDecomposeComposeTest(-fSX, fSY, tan(-fS), -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test I3",
                               impDecomposeComposeTest(fSX, -fSY, tan(-fS), -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test I4",
                               impDecomposeComposeTest(-fSX, -fSY, tan(-fS), -fR));

        // cover special case of 180 degree rotation
        B2DHomMatrix aTest
            = utils::createScaleShearXRotateTranslateB2DHomMatrix(6425, 3938, 0, F_PI, 10482, 4921);
        // decompose that matrix
        B2DTuple aDScale;
        B2DTuple aDTrans;
        double fDRot;
        double fDShX;
        aTest.decompose(aDScale, aDTrans, fDRot, fDShX);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("decompose: error test J1", 6425.0, aDScale.getX(),
                                             1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("decompose: error test J1", 3938.0, aDScale.getY(),
                                             1E-12);
        CPPUNIT_ASSERT_MESSAGE("decompose: error test J1",
                               aDTrans.getX() == 10482 && aDTrans.getY() == 4921);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("decompose: error test J1", F_PI, fDRot, 1E-12);
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
}

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::b2dhommatrix);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
