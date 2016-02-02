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

#include <osl/diagnose.h>
#include <rtl/instance.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <hommatrixtemplate.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

namespace basegfx
{
    typedef ::basegfx::internal::ImplHomMatrixTemplate< 3 > Impl2DHomMatrix_Base;
    class Impl2DHomMatrix : public Impl2DHomMatrix_Base
    {
    };

    namespace { struct IdentityMatrix : public rtl::Static< B2DHomMatrix::ImplType,
                                                            IdentityMatrix > {}; }

    B2DHomMatrix::B2DHomMatrix() :
        mpImpl( IdentityMatrix::get() ) // use common identity matrix
    {
    }

    B2DHomMatrix::B2DHomMatrix(const B2DHomMatrix& rMat) :
        mpImpl(rMat.mpImpl)
    {
    }

    B2DHomMatrix::~B2DHomMatrix()
    {
    }

    B2DHomMatrix::B2DHomMatrix(double f_0x0, double f_0x1, double f_0x2, double f_1x0, double f_1x1, double f_1x2)
    :   mpImpl( IdentityMatrix::get() ) // use common identity matrix, will be made unique with 1st set-call
    {
        mpImpl->set(0, 0, f_0x0);
        mpImpl->set(0, 1, f_0x1);
        mpImpl->set(0, 2, f_0x2);
        mpImpl->set(1, 0, f_1x0);
        mpImpl->set(1, 1, f_1x1);
        mpImpl->set(1, 2, f_1x2);
    }

    B2DHomMatrix& B2DHomMatrix::operator=(const B2DHomMatrix& rMat)
    {
        mpImpl = rMat.mpImpl;
        return *this;
    }

    double B2DHomMatrix::get(sal_uInt16 nRow, sal_uInt16 nColumn) const
    {
        return mpImpl->get(nRow, nColumn);
    }

    void B2DHomMatrix::set(sal_uInt16 nRow, sal_uInt16 nColumn, double fValue)
    {
        mpImpl->set(nRow, nColumn, fValue);
    }

    void B2DHomMatrix::set3x2(double f_0x0, double f_0x1, double f_0x2, double f_1x0, double f_1x1, double f_1x2)
    {
        mpImpl->set(0, 0, f_0x0);
        mpImpl->set(0, 1, f_0x1);
        mpImpl->set(0, 2, f_0x2);
        mpImpl->set(1, 0, f_1x0);
        mpImpl->set(1, 1, f_1x1);
        mpImpl->set(1, 2, f_1x2);
    }

    bool B2DHomMatrix::isLastLineDefault() const
    {
        return mpImpl->isLastLineDefault();
    }

    bool B2DHomMatrix::isIdentity() const
    {
        if(mpImpl.same_object(IdentityMatrix::get()))
            return true;

        return mpImpl->isIdentity();
    }

    void B2DHomMatrix::identity()
    {
        mpImpl = IdentityMatrix::get();
    }

    bool B2DHomMatrix::isInvertible() const
    {
        return mpImpl->isInvertible();
    }

    bool B2DHomMatrix::invert()
    {
        Impl2DHomMatrix aWork(*mpImpl);
        sal_uInt16* pIndex = new sal_uInt16[Impl2DHomMatrix_Base::getEdgeLength()];
        sal_Int16 nParity;

        if(aWork.ludcmp(pIndex, nParity))
        {
            mpImpl->doInvert(aWork, pIndex);
            delete[] pIndex;

            return true;
        }

        delete[] pIndex;
        return false;
    }

    B2DHomMatrix& B2DHomMatrix::operator+=(const B2DHomMatrix& rMat)
    {
        mpImpl->doAddMatrix(*rMat.mpImpl);
        return *this;
    }

    B2DHomMatrix& B2DHomMatrix::operator-=(const B2DHomMatrix& rMat)
    {
        mpImpl->doSubMatrix(*rMat.mpImpl);
        return *this;
    }

    B2DHomMatrix& B2DHomMatrix::operator*=(double fValue)
    {
        const double fOne(1.0);

        if(!fTools::equal(fOne, fValue))
            mpImpl->doMulMatrix(fValue);

        return *this;
    }

    B2DHomMatrix& B2DHomMatrix::operator/=(double fValue)
    {
        const double fOne(1.0);

        if(!fTools::equal(fOne, fValue))
            mpImpl->doMulMatrix(1.0 / fValue);

        return *this;
    }

    B2DHomMatrix& B2DHomMatrix::operator*=(const B2DHomMatrix& rMat)
    {
        if(!rMat.isIdentity())
            mpImpl->doMulMatrix(*rMat.mpImpl);

        return *this;
    }

    bool B2DHomMatrix::operator==(const B2DHomMatrix& rMat) const
    {
        if(mpImpl.same_object(rMat.mpImpl))
            return true;

        return mpImpl->isEqual(*rMat.mpImpl);
    }

    bool B2DHomMatrix::operator!=(const B2DHomMatrix& rMat) const
    {
        return !(*this == rMat);
    }

    void B2DHomMatrix::rotate(double fRadiant)
    {
        if(!fTools::equalZero(fRadiant))
        {
            double fSin(0.0);
            double fCos(1.0);

            tools::createSinCosOrthogonal(fSin, fCos, fRadiant);
            Impl2DHomMatrix aRotMat;

            aRotMat.set(0, 0, fCos);
            aRotMat.set(1, 1, fCos);
            aRotMat.set(1, 0, fSin);
            aRotMat.set(0, 1, -fSin);

            mpImpl->doMulMatrix(aRotMat);
        }
    }

    void B2DHomMatrix::translate(double fX, double fY)
    {
        if(!fTools::equalZero(fX) || !fTools::equalZero(fY))
        {
            Impl2DHomMatrix aTransMat;

            aTransMat.set(0, 2, fX);
            aTransMat.set(1, 2, fY);

            mpImpl->doMulMatrix(aTransMat);
        }
    }

    void B2DHomMatrix::scale(double fX, double fY)
    {
        const double fOne(1.0);

        if(!fTools::equal(fOne, fX) || !fTools::equal(fOne, fY))
        {
            Impl2DHomMatrix aScaleMat;

            aScaleMat.set(0, 0, fX);
            aScaleMat.set(1, 1, fY);

            mpImpl->doMulMatrix(aScaleMat);
        }
    }

    void B2DHomMatrix::shearX(double fSx)
    {
        // #i76239# do not test against 1.0, but against 0.0. We are talking about a value not on the diagonal (!)
        if(!fTools::equalZero(fSx))
        {
            Impl2DHomMatrix aShearXMat;

            aShearXMat.set(0, 1, fSx);

            mpImpl->doMulMatrix(aShearXMat);
        }
    }

    void B2DHomMatrix::shearY(double fSy)
    {
        // #i76239# do not test against 1.0, but against 0.0. We are talking about a value not on the diagonal (!)
        if(!fTools::equalZero(fSy))
        {
            Impl2DHomMatrix aShearYMat;

            aShearYMat.set(1, 0, fSy);

            mpImpl->doMulMatrix(aShearYMat);
        }
    }

    /** Decomposition

       New, optimized version with local shearX detection. Old version (keeping
       below, is working well, too) used the 3D matrix decomposition when
       shear was used. Keeping old version as comment below since it may get
       necessary to add the determinant() test from there here, too.
    */
    bool B2DHomMatrix::decompose(B2DTuple& rScale, B2DTuple& rTranslate, double& rRotate, double& rShearX) const
    {
        // when perspective is used, decompose is not made here
        if(!mpImpl->isLastLineDefault())
        {
            return false;
        }

        // reset rotate and shear and copy translation values in every case
        rRotate = rShearX = 0.0;
        rTranslate.setX(get(0, 2));
        rTranslate.setY(get(1, 2));

        // test for rotation and shear
        if(fTools::equalZero(get(0, 1)) && fTools::equalZero(get(1, 0)))
        {
            // no rotation and shear, copy scale values
            rScale.setX(get(0, 0));
            rScale.setY(get(1, 1));

            // or is there?
            if( rScale.getX() < 0 && rScale.getY() < 0 )
            {
                // there is - 180 degree rotated
                rScale *= -1;
                rRotate = 180*F_PI180;
            }
        }
        else
        {
            // get the unit vectors of the transformation -> the perpendicular vectors
            B2DVector aUnitVecX(get(0, 0), get(1, 0));
            B2DVector aUnitVecY(get(0, 1), get(1, 1));
            const double fScalarXY(aUnitVecX.scalar(aUnitVecY));

            // Test if shear is zero. That's the case if the unit vectors in the matrix
            // are perpendicular -> scalar is zero. This is also the case when one of
            // the unit vectors is zero.
            if(fTools::equalZero(fScalarXY))
            {
                // calculate unsigned scale values
                rScale.setX(aUnitVecX.getLength());
                rScale.setY(aUnitVecY.getLength());

                // check unit vectors for zero lengths
                const bool bXIsZero(fTools::equalZero(rScale.getX()));
                const bool bYIsZero(fTools::equalZero(rScale.getY()));

                if(bXIsZero || bYIsZero)
                {
                    // still extract as much as possible. Scalings are already set
                    if(!bXIsZero)
                    {
                        // get rotation of X-Axis
                        rRotate = atan2(aUnitVecX.getY(), aUnitVecX.getX());
                    }
                    else if(!bYIsZero)
                    {
                        // get rotation of X-Axis. When assuming X and Y perpendicular
                        // and correct rotation, it's the Y-Axis rotation minus 90 degrees
                        rRotate = atan2(aUnitVecY.getY(), aUnitVecY.getX()) - M_PI_2;
                    }

                    // one or both unit vectors do not exist, determinant is zero, no decomposition possible.
                    // Eventually used rotations or shears are lost
                    return false;
                }
                else
                {
                    // no shear
                    // calculate rotation of X unit vector relative to (1, 0)
                    rRotate = atan2(aUnitVecX.getY(), aUnitVecX.getX());

                    // use orientation to evtl. correct sign of Y-Scale
                    const double fCrossXY(aUnitVecX.cross(aUnitVecY));

                    if(fCrossXY < 0.0)
                    {
                        rScale.setY(-rScale.getY());
                    }
                }
            }
            else
            {
                // fScalarXY is not zero, thus both unit vectors exist. No need to handle that here
                // shear, extract it
                double fCrossXY(aUnitVecX.cross(aUnitVecY));

                // get rotation by calculating angle of X unit vector relative to (1, 0).
                // This is before the parallel test following the motto to extract
                // as much as possible
                rRotate = atan2(aUnitVecX.getY(), aUnitVecX.getX());

                // get unsigned scale value for X. It will not change and is useful
                // for further corrections
                rScale.setX(aUnitVecX.getLength());

                if(fTools::equalZero(fCrossXY))
                {
                    // extract as much as possible
                    rScale.setY(aUnitVecY.getLength());

                    // unit vectors are parallel, thus not linear independent. No
                    // useful decomposition possible. This should not happen since
                    // the only way to get the unit vectors nearly parallel is
                    // a very big shearing. Anyways, be prepared for hand-filled
                    // matrices
                    // Eventually used rotations or shears are lost
                    return false;
                }
                else
                {
                    // calculate the contained shear
                    rShearX = fScalarXY / fCrossXY;

                    if(!fTools::equalZero(rRotate))
                    {
                        // To be able to correct the shear for aUnitVecY, rotation needs to be
                        // removed first. Correction of aUnitVecX is easy, it will be rotated back to (1, 0).
                        aUnitVecX.setX(rScale.getX());
                        aUnitVecX.setY(0.0);

                        // for Y correction we rotate the UnitVecY back about -rRotate
                        const double fNegRotate(-rRotate);
                        const double fSin(sin(fNegRotate));
                        const double fCos(cos(fNegRotate));

                        const double fNewX(aUnitVecY.getX() * fCos - aUnitVecY.getY() * fSin);
                        const double fNewY(aUnitVecY.getX() * fSin + aUnitVecY.getY() * fCos);

                        aUnitVecY.setX(fNewX);
                        aUnitVecY.setY(fNewY);
                    }

                    // Correct aUnitVecY and fCrossXY to fShear=0. Rotation is already removed.
                    // Shear correction can only work with removed rotation
                    aUnitVecY.setX(aUnitVecY.getX() - (aUnitVecY.getY() * rShearX));
                    fCrossXY = aUnitVecX.cross(aUnitVecY);

                    // calculate unsigned scale value for Y, after the corrections since
                    // the shear correction WILL change the length of aUnitVecY
                    rScale.setY(aUnitVecY.getLength());

                    // use orientation to set sign of Y-Scale
                    if(fCrossXY < 0.0)
                    {
                        rScale.setY(-rScale.getY());
                    }
                }
            }
        }

        return true;
    }
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
