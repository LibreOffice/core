/*************************************************************************
 *
 *  $RCSfile: b3dhommatrix.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2003-10-31 10:13:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

#ifndef _HOMMATRIX_TEMPLATE_HXX
#include <hommatrixtemplate.hxx>
#endif

#ifndef _BGFX_VECTOR_B3DVECTOR_HXX
#include <basegfx/vector/b3dvector.hxx>
#endif

class Impl3DHomMatrix : public ImplHomMatrixTemplate< 4 >
{
};

static Impl3DHomMatrix& get3DIdentityMatrix()
{
    static Impl3DHomMatrix maStatic3DIdentityHomMatrix;
    return maStatic3DIdentityHomMatrix;
}

namespace basegfx
{
    namespace matrix
    {
        void B3DHomMatrix::implPrepareChange()
        {
            if(mpM->getRefCount())
            {
                mpM->decRefCount();
                mpM = new Impl3DHomMatrix(*mpM);
            }
        }

        B3DHomMatrix::B3DHomMatrix()
        :   mpM(&get3DIdentityMatrix())
        {
            mpM->incRefCount();
        }

        B3DHomMatrix::B3DHomMatrix(const B3DHomMatrix& rMat)
        :   mpM(rMat.mpM)
        {
            mpM->incRefCount();
        }

        B3DHomMatrix::~B3DHomMatrix()
        {
            if(mpM->getRefCount())
                mpM->decRefCount();
            else
                delete mpM;
        }

        B3DHomMatrix& B3DHomMatrix::operator=(const B3DHomMatrix& rMat)
        {
            if(mpM->getRefCount())
                mpM->decRefCount();
            else
                delete mpM;

            mpM = rMat.mpM;
            mpM->incRefCount();

            return *this;
        }

        double B3DHomMatrix::get(sal_uInt16 nRow, sal_uInt16 nColumn) const
        {
            return mpM->get(nRow, nColumn);
        }

        void B3DHomMatrix::set(sal_uInt16 nRow, sal_uInt16 nColumn, double fValue)
        {
            implPrepareChange();
            mpM->set(nRow, nColumn, fValue);
        }

        bool B3DHomMatrix::isIdentity() const
        {
            if(mpM == &get3DIdentityMatrix())
                return true;

            return mpM->isIdentity();
        }

        void B3DHomMatrix::identity()
        {
            if(mpM->getRefCount())
                mpM->decRefCount();
            else
                delete mpM;

            mpM = &get3DIdentityMatrix();
            mpM->incRefCount();
        }

        bool B3DHomMatrix::isInvertible() const
        {
            return mpM->isInvertible();
        }

        bool B3DHomMatrix::invert()
        {
            Impl3DHomMatrix aWork(*mpM);
            sal_uInt16* pIndex = new sal_uInt16[mpM->getEdgeLength()];
            sal_Int16 nParity;

            if(aWork.ludcmp(pIndex, nParity))
            {
                implPrepareChange();
                mpM->doInvert(aWork, pIndex);
                delete pIndex;

                return true;
            }

            delete pIndex;
            return false;
        }

        bool B3DHomMatrix::isNormalized() const
        {
            return mpM->isNormalized();
        }

        void B3DHomMatrix::normalize()
        {
            if(!mpM->isNormalized())
            {
                implPrepareChange();
                mpM->doNormalize();
            }
        }

        double B3DHomMatrix::determinant() const
        {
            return mpM->doDeterminant();
        }

        double B3DHomMatrix::trace() const
        {
            return mpM->doTrace();
        }

        void B3DHomMatrix::transpose()
        {
            implPrepareChange();
            mpM->doTranspose();
        }

        B3DHomMatrix& B3DHomMatrix::operator+=(const B3DHomMatrix& rMat)
        {
            implPrepareChange();
            mpM->doAddMatrix(*rMat.mpM);

            return *this;
        }

        B3DHomMatrix& B3DHomMatrix::operator-=(const B3DHomMatrix& rMat)
        {
            implPrepareChange();
            mpM->doSubMatrix(*rMat.mpM);

            return *this;
        }

        B3DHomMatrix& B3DHomMatrix::operator*=(double fValue)
        {
            const double fOne(1.0);

            if(!::basegfx::numeric::fTools::equal(fOne, fValue))
            {
                implPrepareChange();
                mpM->doMulMatrix(fValue);
            }

            return *this;
        }

        B3DHomMatrix& B3DHomMatrix::operator/=(double fValue)
        {
            const double fOne(1.0);

            if(!::basegfx::numeric::fTools::equal(fOne, fValue))
            {
                implPrepareChange();
                mpM->doMulMatrix(1.0 / fValue);
            }

            return *this;
        }

        B3DHomMatrix& B3DHomMatrix::operator*=(const B3DHomMatrix& rMat)
        {
            if(!rMat.isIdentity())
            {
                implPrepareChange();
                mpM->doMulMatrix(*rMat.mpM);
            }

            return *this;
        }

        bool B3DHomMatrix::operator==(const B3DHomMatrix& rMat) const
        {
            if(mpM == rMat.mpM)
                return true;

            return mpM->isEqual(*rMat.mpM);
        }

        bool B3DHomMatrix::operator!=(const B3DHomMatrix& rMat) const
        {
            if(mpM == rMat.mpM)
                return false;

            return !mpM->isEqual(*rMat.mpM);
        }

        void B3DHomMatrix::rotate(double fAngleX,double fAngleY,double fAngleZ)
        {
            if(!::basegfx::numeric::fTools::equalZero(fAngleX) || !::basegfx::numeric::fTools::equalZero(fAngleY) || !::basegfx::numeric::fTools::equalZero(fAngleZ))
            {
                implPrepareChange();

                if(!::basegfx::numeric::fTools::equalZero(fAngleX))
                {
                    Impl3DHomMatrix aRotMatX(get3DIdentityMatrix());
                    double fSin(sin(fAngleX));
                    double fCos(cos(fAngleX));

                    aRotMatX.set(1, 1, fCos);
                    aRotMatX.set(2, 2, fCos);
                    aRotMatX.set(2, 1, fSin);
                    aRotMatX.set(1, 2, -fSin);

                    mpM->doMulMatrix(aRotMatX);
                }

                if(!::basegfx::numeric::fTools::equalZero(fAngleY))
                {
                    Impl3DHomMatrix aRotMatY(get3DIdentityMatrix());
                    double fSin(sin(fAngleY));
                    double fCos(cos(fAngleY));

                    aRotMatY.set(0, 0, fCos);
                    aRotMatY.set(2, 2, fCos);
                    aRotMatY.set(0, 2, fSin);
                    aRotMatY.set(2, 0, -fSin);

                    mpM->doMulMatrix(aRotMatY);
                }

                if(!::basegfx::numeric::fTools::equalZero(fAngleZ))
                {
                    Impl3DHomMatrix aRotMatZ(get3DIdentityMatrix());
                    double fSin(sin(fAngleZ));
                    double fCos(cos(fAngleZ));

                    aRotMatZ.set(0, 0, fCos);
                    aRotMatZ.set(1, 1, fCos);
                    aRotMatZ.set(1, 0, fSin);
                    aRotMatZ.set(0, 1, -fSin);

                    mpM->doMulMatrix(aRotMatZ);
                }
            }
        }

        void B3DHomMatrix::translate(double fX, double fY, double fZ)
        {
            if(!::basegfx::numeric::fTools::equalZero(fX) || !::basegfx::numeric::fTools::equalZero(fY) || !::basegfx::numeric::fTools::equalZero(fZ))
            {
                Impl3DHomMatrix aTransMat(get3DIdentityMatrix());

                aTransMat.set(0, 2, fX);
                aTransMat.set(1, 2, fY);
                aTransMat.set(2, 2, fZ);

                implPrepareChange();
                mpM->doMulMatrix(aTransMat);
            }
        }

        void B3DHomMatrix::scale(double fX, double fY, double fZ)
        {
            const double fOne(1.0);

            if(!::basegfx::numeric::fTools::equal(fOne, fX) || !::basegfx::numeric::fTools::equal(fOne, fY) ||!::basegfx::numeric::fTools::equal(fOne, fZ))
            {
                Impl3DHomMatrix aScaleMat(get3DIdentityMatrix());

                aScaleMat.set(0, 0, fX);
                aScaleMat.set(1, 1, fY);
                aScaleMat.set(2, 2, fZ);

                implPrepareChange();
                mpM->doMulMatrix(aScaleMat);
            }
        }

        void B3DHomMatrix::shearXY(double fSx, double fSy)
        {
            const double fOne(1.0);

            if(!::basegfx::numeric::fTools::equal(fOne, fSx) || !::basegfx::numeric::fTools::equal(fOne, fSy))
            {
                Impl3DHomMatrix aShearXYMat(get3DIdentityMatrix());

                aShearXYMat.set(0, 2, fSx);
                aShearXYMat.set(1, 2, fSy);

                implPrepareChange();
                mpM->doMulMatrix(aShearXYMat);
            }
        }

        void B3DHomMatrix::shearYZ(double fSy, double fSz)
        {
            const double fOne(1.0);

            if(!::basegfx::numeric::fTools::equal(fOne, fSy) || !::basegfx::numeric::fTools::equal(fOne, fSz))
            {
                Impl3DHomMatrix aShearYZMat(get3DIdentityMatrix());

                aShearYZMat.set(1, 0, fSy);
                aShearYZMat.set(2, 0, fSz);

                implPrepareChange();
                mpM->doMulMatrix(aShearYZMat);
            }
        }

        void B3DHomMatrix::shearXZ(double fSx, double fSz)
        {
            const double fOne(1.0);

            if(!::basegfx::numeric::fTools::equal(fOne, fSx) || !::basegfx::numeric::fTools::equal(fOne, fSz))
            {
                Impl3DHomMatrix aShearXZMat(get3DIdentityMatrix());

                aShearXZMat.set(0, 1, fSx);
                aShearXZMat.set(2, 1, fSz);

                implPrepareChange();
                mpM->doMulMatrix(aShearXZMat);
            }
        }

        void B3DHomMatrix::frustum(double fLeft, double fRight, double fBottom, double fTop, double fNear, double fFar)
        {
            const double fZero(0.0);
            const double fOne(1.0);

            if(!::basegfx::numeric::fTools::more(fNear, fZero))
            {
                fNear = 0.001;
            }

            if(!::basegfx::numeric::fTools::more(fFar, fZero))
            {
                fFar = fOne;
            }

            if(::basegfx::numeric::fTools::equal(fNear, fFar))
            {
                fFar = fNear + fOne;
            }

            if(::basegfx::numeric::fTools::equal(fLeft, fRight))
            {
                fLeft -= fOne;
                fRight += fOne;
            }

            if(::basegfx::numeric::fTools::equal(fTop, fBottom))
            {
                fBottom -= fOne;
                fTop += fOne;
            }

            Impl3DHomMatrix aFrustumMat(get3DIdentityMatrix());

            aFrustumMat.set(0, 0, 2.0 * fNear / (fRight - fLeft));
            aFrustumMat.set(1, 1, 2.0 * fNear / (fTop - fBottom));
            aFrustumMat.set(0, 2, (fRight + fLeft) / (fRight - fLeft));
            aFrustumMat.set(1, 2, (fTop + fBottom) / (fTop - fBottom));
            aFrustumMat.set(2, 2, -fOne * ((fFar + fNear) / (fFar - fNear)));
            aFrustumMat.set(3, 2, -fOne);
            aFrustumMat.set(2, 3, -fOne * ((2.0 * fFar * fNear) / (fFar - fNear)));
            aFrustumMat.set(3, 3, fZero);

            if(mpM->getRefCount())
                mpM->decRefCount();
            else
                delete mpM;

            mpM = new Impl3DHomMatrix(aFrustumMat);
        }

        void B3DHomMatrix::ortho(double fLeft, double fRight, double fBottom, double fTop, double fNear, double fFar)
        {
            if(::basegfx::numeric::fTools::equal(fNear, fFar))
            {
                fFar = fNear + 1.0;
            }

            if(::basegfx::numeric::fTools::equal(fLeft, fRight))
            {
                fLeft -= 1.0;
                fRight += 1.0;
            }

            if(::basegfx::numeric::fTools::equal(fTop, fBottom))
            {
                fBottom -= 1.0;
                fTop += 1.0;
            }

            Impl3DHomMatrix aOrthoMat(get3DIdentityMatrix());

            aOrthoMat.set(0, 0, 2.0 / (fRight - fLeft));
            aOrthoMat.set(1, 1, 2.0 / (fTop - fBottom));
            aOrthoMat.set(2, 2, -1.0 * (2.0 / (fFar - fNear)));
            aOrthoMat.set(0, 3, -1.0 * ((fRight + fLeft) / (fRight - fLeft)));
            aOrthoMat.set(1, 3, -1.0 * ((fTop + fBottom) / (fTop - fBottom)));
            aOrthoMat.set(2, 3, -1.0 * ((fFar + fNear) / (fFar - fNear)));

            if(mpM->getRefCount())
                mpM->decRefCount();
            else
                delete mpM;

            mpM = new Impl3DHomMatrix(aOrthoMat);
        }

        bool B3DHomMatrix::decompose(tuple::B3DTuple& rScale, tuple::B3DTuple& rTranslate, tuple::B3DTuple& rRotate, tuple::B3DTuple& rShear) const
        {
            // when perspective is used, decompose is not made here
            if(!mpM->isLastLineDefault())
                return false;

            // If determinant is zero, decomposition is not possible
            if(0.0 == mpM->doDeterminant())
                return false;

            // isolate translation
            rTranslate.setX(mpM->get(0, 3));
            rTranslate.setY(mpM->get(1, 3));
            rTranslate.setZ(mpM->get(2, 3));

            // correct translate values
            rTranslate.correctValues();

            // get scale and shear
            vector::B3DVector aCol0(mpM->get(0, 0), mpM->get(1, 0), mpM->get(2, 0));
            vector::B3DVector aCol1(mpM->get(0, 1), mpM->get(1, 1), mpM->get(2, 1));
            vector::B3DVector aCol2(mpM->get(0, 2), mpM->get(1, 2), mpM->get(2, 2));
            vector::B3DVector aTemp;

            // get ScaleX
            rScale.setX(aCol0.getLength());
            aCol0.normalize();

            // get ShearXY
            rShear.setX(aCol0.scalar(aCol1));

            if(::basegfx::numeric::fTools::equalZero(rShear.getX()))
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

            if(!::basegfx::numeric::fTools::equalZero(fShearX))
            {
                rShear.setX(rShear.getX() / rScale.getY());
            }

            // get ShearXZ
            rShear.setY(aCol0.scalar(aCol2));

            if(::basegfx::numeric::fTools::equalZero(rShear.getY()))
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

            if(::basegfx::numeric::fTools::equalZero(rShear.getZ()))
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

            if(!::basegfx::numeric::fTools::equalZero(fShearY))
            {
                rShear.setY(rShear.getY() / rScale.getZ());
            }

            const double fShearZ(rShear.getZ());

            if(!::basegfx::numeric::fTools::equalZero(fShearZ))
            {
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
            rRotate.setY(asin(-aCol0.getZ()));

            if(::basegfx::numeric::fTools::equalZero(cos(rRotate.getY())))
            {
                rRotate.setX(atan2(aCol1.getX(), aCol1.getY()));
                rRotate.setZ(0.0);
            }
            else
            {
                rRotate.setX(atan2(aCol1.getZ(), aCol2.getZ()));
                rRotate.setZ(atan2(aCol0.getY(), aCol0.getX()));
            }

            // corrcet rotate values
            rRotate.correctValues();

            return true;
        }
    } // end of namespace matrix
} // end of namespace basegfx

// eof
