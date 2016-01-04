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

#include <rtl/instance.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <hommatrixtemplate.hxx>
#include <basegfx/vector/b3dvector.hxx>

namespace basegfx
{
    typedef ::basegfx::internal::ImplHomMatrixTemplate< 4 >Impl3DHomMatrix_Base;
    class Impl3DHomMatrix : public Impl3DHomMatrix_Base
    {
    };

    namespace { struct IdentityMatrix : public rtl::Static< B3DHomMatrix::ImplType,
                                                            IdentityMatrix > {}; }

    B3DHomMatrix::B3DHomMatrix() :
        mpImpl( IdentityMatrix::get() ) // use common identity matrix
    {
    }

    B3DHomMatrix::B3DHomMatrix(const B3DHomMatrix& rMat) :
        mpImpl(rMat.mpImpl)
    {
    }

    B3DHomMatrix::~B3DHomMatrix()
    {
    }

    B3DHomMatrix& B3DHomMatrix::operator=(const B3DHomMatrix& rMat)
    {
        mpImpl = rMat.mpImpl;
        return *this;
    }

    double B3DHomMatrix::get(sal_uInt16 nRow, sal_uInt16 nColumn) const
    {
        return mpImpl->get(nRow, nColumn);
    }

    void B3DHomMatrix::set(sal_uInt16 nRow, sal_uInt16 nColumn, double fValue)
    {
        mpImpl->set(nRow, nColumn, fValue);
    }

    bool B3DHomMatrix::isLastLineDefault() const
    {
        return mpImpl->isLastLineDefault();
    }

    bool B3DHomMatrix::isIdentity() const
    {
        if(mpImpl.same_object(IdentityMatrix::get()))
            return true;

        return mpImpl->isIdentity();
    }

    void B3DHomMatrix::identity()
    {
        mpImpl = IdentityMatrix::get();
    }

    bool B3DHomMatrix::invert()
    {
        Impl3DHomMatrix aWork(*mpImpl);
        sal_uInt16* pIndex = new sal_uInt16[Impl3DHomMatrix_Base::getEdgeLength()];
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

    double B3DHomMatrix::determinant() const
    {
        return mpImpl->doDeterminant();
    }

    B3DHomMatrix& B3DHomMatrix::operator+=(const B3DHomMatrix& rMat)
    {
        mpImpl->doAddMatrix(*rMat.mpImpl);
        return *this;
    }

    B3DHomMatrix& B3DHomMatrix::operator-=(const B3DHomMatrix& rMat)
    {
        mpImpl->doSubMatrix(*rMat.mpImpl);
        return *this;
    }

    B3DHomMatrix& B3DHomMatrix::operator*=(double fValue)
    {
        const double fOne(1.0);

        if(!fTools::equal(fOne, fValue))
            mpImpl->doMulMatrix(fValue);

        return *this;
    }

    B3DHomMatrix& B3DHomMatrix::operator/=(double fValue)
    {
        const double fOne(1.0);

        if(!fTools::equal(fOne, fValue))
            mpImpl->doMulMatrix(1.0 / fValue);

        return *this;
    }

    B3DHomMatrix& B3DHomMatrix::operator*=(const B3DHomMatrix& rMat)
    {
        if(!rMat.isIdentity())
            mpImpl->doMulMatrix(*rMat.mpImpl);

        return *this;
    }

    bool B3DHomMatrix::operator==(const B3DHomMatrix& rMat) const
    {
        if(mpImpl.same_object(rMat.mpImpl))
            return true;

        return mpImpl->isEqual(*rMat.mpImpl);
    }

    bool B3DHomMatrix::operator!=(const B3DHomMatrix& rMat) const
    {
        return !(*this == rMat);
    }

    void B3DHomMatrix::rotate(double fAngleX,double fAngleY,double fAngleZ)
    {
        if(!fTools::equalZero(fAngleX) || !fTools::equalZero(fAngleY) || !fTools::equalZero(fAngleZ))
        {
            if(!fTools::equalZero(fAngleX))
            {
                Impl3DHomMatrix aRotMatX;
                double fSin(sin(fAngleX));
                double fCos(cos(fAngleX));

                aRotMatX.set(1, 1, fCos);
                aRotMatX.set(2, 2, fCos);
                aRotMatX.set(2, 1, fSin);
                aRotMatX.set(1, 2, -fSin);

                mpImpl->doMulMatrix(aRotMatX);
            }

            if(!fTools::equalZero(fAngleY))
            {
                Impl3DHomMatrix aRotMatY;
                double fSin(sin(fAngleY));
                double fCos(cos(fAngleY));

                aRotMatY.set(0, 0, fCos);
                aRotMatY.set(2, 2, fCos);
                aRotMatY.set(0, 2, fSin);
                aRotMatY.set(2, 0, -fSin);

                mpImpl->doMulMatrix(aRotMatY);
            }

            if(!fTools::equalZero(fAngleZ))
            {
                Impl3DHomMatrix aRotMatZ;
                double fSin(sin(fAngleZ));
                double fCos(cos(fAngleZ));

                aRotMatZ.set(0, 0, fCos);
                aRotMatZ.set(1, 1, fCos);
                aRotMatZ.set(1, 0, fSin);
                aRotMatZ.set(0, 1, -fSin);

                mpImpl->doMulMatrix(aRotMatZ);
            }
        }
    }

    void B3DHomMatrix::translate(double fX, double fY, double fZ)
    {
        if(!fTools::equalZero(fX) || !fTools::equalZero(fY) || !fTools::equalZero(fZ))
        {
            Impl3DHomMatrix aTransMat;

            aTransMat.set(0, 3, fX);
            aTransMat.set(1, 3, fY);
            aTransMat.set(2, 3, fZ);

            mpImpl->doMulMatrix(aTransMat);
        }
    }

    void B3DHomMatrix::scale(double fX, double fY, double fZ)
    {
        const double fOne(1.0);

        if(!fTools::equal(fOne, fX) || !fTools::equal(fOne, fY) ||!fTools::equal(fOne, fZ))
        {
            Impl3DHomMatrix aScaleMat;

            aScaleMat.set(0, 0, fX);
            aScaleMat.set(1, 1, fY);
            aScaleMat.set(2, 2, fZ);

            mpImpl->doMulMatrix(aScaleMat);
        }
    }

    void B3DHomMatrix::shearXY(double fSx, double fSy)
    {
        // #i76239# do not test against 1.0, but against 0.0. We are talking about a value not on the diagonal (!)
        if(!fTools::equalZero(fSx) || !fTools::equalZero(fSy))
        {
            Impl3DHomMatrix aShearXYMat;

            aShearXYMat.set(0, 2, fSx);
            aShearXYMat.set(1, 2, fSy);

            mpImpl->doMulMatrix(aShearXYMat);
        }
    }

    void B3DHomMatrix::shearXZ(double fSx, double fSz)
    {
        // #i76239# do not test against 1.0, but against 0.0. We are talking about a value not on the diagonal (!)
        if(!fTools::equalZero(fSx) || !fTools::equalZero(fSz))
        {
            Impl3DHomMatrix aShearXZMat;

            aShearXZMat.set(0, 1, fSx);
            aShearXZMat.set(2, 1, fSz);

            mpImpl->doMulMatrix(aShearXZMat);
        }
    }
    void B3DHomMatrix::frustum(double fLeft, double fRight, double fBottom, double fTop, double fNear, double fFar)
    {
        const double fZero(0.0);
        const double fOne(1.0);

        if(!fTools::more(fNear, fZero))
        {
            fNear = 0.001;
        }

        if(!fTools::more(fFar, fZero))
        {
            fFar = fOne;
        }

        if(fTools::equal(fNear, fFar))
        {
            fFar = fNear + fOne;
        }

        if(fTools::equal(fLeft, fRight))
        {
            fLeft -= fOne;
            fRight += fOne;
        }

        if(fTools::equal(fTop, fBottom))
        {
            fBottom -= fOne;
            fTop += fOne;
        }

        Impl3DHomMatrix aFrustumMat;

        aFrustumMat.set(0, 0, 2.0 * fNear / (fRight - fLeft));
        aFrustumMat.set(1, 1, 2.0 * fNear / (fTop - fBottom));
        aFrustumMat.set(0, 2, (fRight + fLeft) / (fRight - fLeft));
        aFrustumMat.set(1, 2, (fTop + fBottom) / (fTop - fBottom));
        aFrustumMat.set(2, 2, -fOne * ((fFar + fNear) / (fFar - fNear)));
        aFrustumMat.set(3, 2, -fOne);
        aFrustumMat.set(2, 3, -fOne * ((2.0 * fFar * fNear) / (fFar - fNear)));
        aFrustumMat.set(3, 3, fZero);

        mpImpl->doMulMatrix(aFrustumMat);
    }

    void B3DHomMatrix::ortho(double fLeft, double fRight, double fBottom, double fTop, double fNear, double fFar)
    {
        if(fTools::equal(fNear, fFar))
        {
            fFar = fNear + 1.0;
        }

        if(fTools::equal(fLeft, fRight))
        {
            fLeft -= 1.0;
            fRight += 1.0;
        }

        if(fTools::equal(fTop, fBottom))
        {
            fBottom -= 1.0;
            fTop += 1.0;
        }

        Impl3DHomMatrix aOrthoMat;

        aOrthoMat.set(0, 0, 2.0 / (fRight - fLeft));
        aOrthoMat.set(1, 1, 2.0 / (fTop - fBottom));
        aOrthoMat.set(2, 2, -1.0 * (2.0 / (fFar - fNear)));
        aOrthoMat.set(0, 3, -1.0 * ((fRight + fLeft) / (fRight - fLeft)));
        aOrthoMat.set(1, 3, -1.0 * ((fTop + fBottom) / (fTop - fBottom)));
        aOrthoMat.set(2, 3, -1.0 * ((fFar + fNear) / (fFar - fNear)));

        mpImpl->doMulMatrix(aOrthoMat);
    }

    void B3DHomMatrix::orientation(const B3DPoint& rVRP, B3DVector aVPN, B3DVector aVUV)
    {
        Impl3DHomMatrix aOrientationMat;

        // translate -VRP
        aOrientationMat.set(0, 3, -rVRP.getX());
        aOrientationMat.set(1, 3, -rVRP.getY());
        aOrientationMat.set(2, 3, -rVRP.getZ());

        // build rotation
        aVUV.normalize();
        aVPN.normalize();

        // build x-axis as perpendicular fron aVUV and aVPN
        B3DVector aRx(aVUV.getPerpendicular(aVPN));
        aRx.normalize();

        // y-axis perpendicular to that
        B3DVector aRy(aVPN.getPerpendicular(aRx));
        aRy.normalize();

        // the calculated normals are the line vectors of the rotation matrix,
        // set them to create rotation
        aOrientationMat.set(0, 0, aRx.getX());
        aOrientationMat.set(0, 1, aRx.getY());
        aOrientationMat.set(0, 2, aRx.getZ());
        aOrientationMat.set(1, 0, aRy.getX());
        aOrientationMat.set(1, 1, aRy.getY());
        aOrientationMat.set(1, 2, aRy.getZ());
        aOrientationMat.set(2, 0, aVPN.getX());
        aOrientationMat.set(2, 1, aVPN.getY());
        aOrientationMat.set(2, 2, aVPN.getZ());

        mpImpl->doMulMatrix(aOrientationMat);
    }

    bool B3DHomMatrix::decompose(B3DTuple& rScale, B3DTuple& rTranslate, B3DTuple& rRotate, B3DTuple& rShear) const
    {
        // when perspective is used, decompose is not made here
        if(!mpImpl->isLastLineDefault())
            return false;

        // If determinant is zero, decomposition is not possible
        if(0.0 == determinant())
            return false;

        // isolate translation
        rTranslate.setX(mpImpl->get(0, 3));
        rTranslate.setY(mpImpl->get(1, 3));
        rTranslate.setZ(mpImpl->get(2, 3));

        // correct translate values
        rTranslate.correctValues();

        // get scale and shear
        B3DVector aCol0(mpImpl->get(0, 0), mpImpl->get(1, 0), mpImpl->get(2, 0));
        B3DVector aCol1(mpImpl->get(0, 1), mpImpl->get(1, 1), mpImpl->get(2, 1));
        B3DVector aCol2(mpImpl->get(0, 2), mpImpl->get(1, 2), mpImpl->get(2, 2));
        B3DVector aTemp;

        // get ScaleX
        rScale.setX(aCol0.getLength());
        aCol0.normalize();

        // get ShearXY
        rShear.setX(aCol0.scalar(aCol1));

        if(fTools::equalZero(rShear.getX()))
        {
            rShear.setX(0.0);
        }
        else
        {
            aTemp.setX(aCol1.getX() - rShear.getX() * aCol0.getX());
            aTemp.setY(aCol1.getY() - rShear.getX() * aCol0.getY());
            aTemp.setZ(aCol1.getZ() - rShear.getX() * aCol0.getZ());
            aCol1 = aTemp;
        }

        // get ScaleY
        rScale.setY(aCol1.getLength());
        aCol1.normalize();

        const double fShearX(rShear.getX());

        if(!fTools::equalZero(fShearX))
        {
            rShear.setX(rShear.getX() / rScale.getY());
        }

        // get ShearXZ
        rShear.setY(aCol0.scalar(aCol2));

        if(fTools::equalZero(rShear.getY()))
        {
            rShear.setY(0.0);
        }
        else
        {
            aTemp.setX(aCol2.getX() - rShear.getY() * aCol0.getX());
            aTemp.setY(aCol2.getY() - rShear.getY() * aCol0.getY());
            aTemp.setZ(aCol2.getZ() - rShear.getY() * aCol0.getZ());
            aCol2 = aTemp;
        }

        // get ShearYZ
        rShear.setZ(aCol1.scalar(aCol2));

        if(fTools::equalZero(rShear.getZ()))
        {
            rShear.setZ(0.0);
        }
        else
        {
            aTemp.setX(aCol2.getX() - rShear.getZ() * aCol1.getX());
            aTemp.setY(aCol2.getY() - rShear.getZ() * aCol1.getY());
            aTemp.setZ(aCol2.getZ() - rShear.getZ() * aCol1.getZ());
            aCol2 = aTemp;
        }

        // get ScaleZ
        rScale.setZ(aCol2.getLength());
        aCol2.normalize();

        const double fShearY(rShear.getY());

        if(!fTools::equalZero(fShearY))
        {
            // coverity[copy_paste_error] - this is correct getZ, not getY
            rShear.setY(rShear.getY() / rScale.getZ());
        }

        const double fShearZ(rShear.getZ());

        if(!fTools::equalZero(fShearZ))
        {
            // coverity[original] - this is not an original copy-and-paste source for ^^^
            rShear.setZ(rShear.getZ() / rScale.getZ());
        }

        // correct shear values
        rShear.correctValues();

        // Coordinate system flip?
        if(0.0 > aCol0.scalar(aCol1.getPerpendicular(aCol2)))
        {
            rScale = -rScale;
            aCol0 = -aCol0;
            aCol1 = -aCol1;
            aCol2 = -aCol2;
        }

        // correct scale values
        rScale.correctValues(1.0);

        // Get rotations
        {
            double fy=0;
            double cy=0;

            if( ::basegfx::fTools::equal( aCol0.getZ(), 1.0 )
                || aCol0.getZ() > 1.0 )
            {
                fy = -F_PI/2.0;
                cy = 0.0;
            }
            else if( ::basegfx::fTools::equal( aCol0.getZ(), -1.0 )
                || aCol0.getZ() < -1.0 )
            {
                fy = F_PI/2.0;
                cy = 0.0;
            }
            else
            {
                fy = asin( -aCol0.getZ() );
                cy = cos(fy);
            }

            rRotate.setY(fy);
            if( ::basegfx::fTools::equalZero( cy ) )
            {
                if( aCol0.getZ() > 0.0 )
                    rRotate.setX(atan2(-1.0*aCol1.getX(), aCol1.getY()));
                else
                    rRotate.setX(atan2(aCol1.getX(), aCol1.getY()));
                rRotate.setZ(0.0);
            }
            else
            {
                rRotate.setX(atan2(aCol1.getZ(), aCol2.getZ()));
                rRotate.setZ(atan2(aCol0.getY(), aCol0.getX()));
            }

            // correct rotate values
            rRotate.correctValues();
        }

        return true;
    }
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
