/*************************************************************************
 *
 *  $RCSfile: b2dhommatrix.cxx,v $
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

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _HOMMATRIX_TEMPLATE_HXX
#include <hommatrixtemplate.hxx>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

#ifndef _BGFX_TUPLE_B3DTUPLE_HXX
#include <basegfx/tuple/b3dtuple.hxx>
#endif

#ifndef _BGFX_TUPLE_B2DTUPLE_HXX
#include <basegfx/tuple/b2dtuple.hxx>
#endif

class Impl2DHomMatrix : public ImplHomMatrixTemplate< 3 >
{
};

static Impl2DHomMatrix& get2DIdentityMatrix()
{
    static Impl2DHomMatrix maStatic2DIdentityHomMatrix;
    return maStatic2DIdentityHomMatrix;
}

namespace basegfx
{
    namespace matrix
    {
        void B2DHomMatrix::implPrepareChange()
        {
            if(mpM->getRefCount())
            {
                mpM->decRefCount();
                mpM = new Impl2DHomMatrix(*mpM);
            }
        }

        B2DHomMatrix::B2DHomMatrix()
        :   mpM(&get2DIdentityMatrix())
        {
            mpM->incRefCount();
        }

        B2DHomMatrix::B2DHomMatrix(const B2DHomMatrix& rMat)
        :   mpM(rMat.mpM)
        {
            mpM->incRefCount();
        }

        B2DHomMatrix::~B2DHomMatrix()
        {
            if(mpM->getRefCount())
                mpM->decRefCount();
            else
                delete mpM;
        }

        B2DHomMatrix& B2DHomMatrix::operator=(const B2DHomMatrix& rMat)
        {
            if(mpM->getRefCount())
                mpM->decRefCount();
            else
                delete mpM;

            mpM = rMat.mpM;
            mpM->incRefCount();

            return *this;
        }

        double B2DHomMatrix::get(sal_uInt16 nRow, sal_uInt16 nColumn) const
        {
            return mpM->get(nRow, nColumn);
        }

        void B2DHomMatrix::set(sal_uInt16 nRow, sal_uInt16 nColumn, double fValue)
        {
            implPrepareChange();
            mpM->set(nRow, nColumn, fValue);
        }

        bool B2DHomMatrix::isIdentity() const
        {
            if(mpM == &get2DIdentityMatrix())
                return true;

            return mpM->isIdentity();
        }

        void B2DHomMatrix::identity()
        {
            if(mpM->getRefCount())
                mpM->decRefCount();
            else
                delete mpM;

            mpM = &get2DIdentityMatrix();
            mpM->incRefCount();
        }

        bool B2DHomMatrix::isInvertible() const
        {
            return mpM->isInvertible();
        }

        bool B2DHomMatrix::invert()
        {
            Impl2DHomMatrix aWork(*mpM);
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

        bool B2DHomMatrix::isNormalized() const
        {
            return mpM->isNormalized();
        }

        void B2DHomMatrix::normalize()
        {
            if(!mpM->isNormalized())
            {
                implPrepareChange();
                mpM->doNormalize();
            }
        }

        double B2DHomMatrix::determinant() const
        {
            return mpM->doDeterminant();
        }

        double B2DHomMatrix::trace() const
        {
            return mpM->doTrace();
        }

        void B2DHomMatrix::transpose()
        {
            implPrepareChange();
            mpM->doTranspose();
        }

        B2DHomMatrix& B2DHomMatrix::operator+=(const B2DHomMatrix& rMat)
        {
            implPrepareChange();
            mpM->doAddMatrix(*rMat.mpM);

            return *this;
        }

        B2DHomMatrix& B2DHomMatrix::operator-=(const B2DHomMatrix& rMat)
        {
            implPrepareChange();
            mpM->doSubMatrix(*rMat.mpM);

            return *this;
        }

        B2DHomMatrix& B2DHomMatrix::operator*=(double fValue)
        {
            const double fOne(1.0);

            if(!::basegfx::numeric::fTools::equal(fOne, fValue))
            {
                implPrepareChange();
                mpM->doMulMatrix(fValue);
            }

            return *this;
        }

        B2DHomMatrix& B2DHomMatrix::operator/=(double fValue)
        {
            const double fOne(1.0);

            if(!::basegfx::numeric::fTools::equal(fOne, fValue))
            {
                implPrepareChange();
                mpM->doMulMatrix(1.0 / fValue);
            }

            return *this;
        }

        B2DHomMatrix& B2DHomMatrix::operator*=(const B2DHomMatrix& rMat)
        {
            if(!rMat.isIdentity())
            {
                implPrepareChange();
                mpM->doMulMatrix(*rMat.mpM);
            }

            return *this;
        }

        bool B2DHomMatrix::operator==(const B2DHomMatrix& rMat) const
        {
            if(mpM == rMat.mpM)
                return true;

            return mpM->isEqual(*rMat.mpM);
        }

        bool B2DHomMatrix::operator!=(const B2DHomMatrix& rMat) const
        {
            if(mpM == rMat.mpM)
                return false;

            return !mpM->isEqual(*rMat.mpM);
        }

        void B2DHomMatrix::rotate(double fRadiant)
        {
            if(!::basegfx::numeric::fTools::equalZero(fRadiant))
            {
                Impl2DHomMatrix aRotMat(get2DIdentityMatrix());
                double fSin(sin(fRadiant));
                double fCos(cos(fRadiant));

                aRotMat.set(0, 0, fCos);
                aRotMat.set(1, 1, fCos);
                aRotMat.set(1, 0, fSin);
                aRotMat.set(0, 1, -fSin);

                implPrepareChange();
                mpM->doMulMatrix(aRotMat);
            }
        }

        void B2DHomMatrix::translate(double fX, double fY)
        {
            if(!::basegfx::numeric::fTools::equalZero(fX) || !::basegfx::numeric::fTools::equalZero(fY))
            {
                Impl2DHomMatrix aTransMat(get2DIdentityMatrix());

                aTransMat.set(0, 2, fX);
                aTransMat.set(1, 2, fY);

                implPrepareChange();
                mpM->doMulMatrix(aTransMat);
            }
        }

        void B2DHomMatrix::scale(double fX, double fY)
        {
            const double fOne(1.0);

            if(!::basegfx::numeric::fTools::equal(fOne, fX) || !::basegfx::numeric::fTools::equal(fOne, fY))
            {
                Impl2DHomMatrix aScaleMat(get2DIdentityMatrix());

                aScaleMat.set(0, 0, fX);
                aScaleMat.set(1, 1, fY);

                implPrepareChange();
                mpM->doMulMatrix(aScaleMat);
            }
        }

        void B2DHomMatrix::shearX(double fSx)
        {
            const double fOne(1.0);

            if(!::basegfx::numeric::fTools::equal(fOne, fSx))
            {
                Impl2DHomMatrix aShearXMat(get2DIdentityMatrix());

                aShearXMat.set(0, 1, fSx);

                implPrepareChange();
                mpM->doMulMatrix(aShearXMat);
            }
        }

        void B2DHomMatrix::shearY(double fSy)
        {
            const double fOne(1.0);

            if(!::basegfx::numeric::fTools::equal(fOne, fSy))
            {
                Impl2DHomMatrix aShearYMat(get2DIdentityMatrix());

                aShearYMat.set(1, 0, fSy);

                implPrepareChange();
                mpM->doMulMatrix(aShearYMat);
            }
        }

        // Decomposition
        bool B2DHomMatrix::decompose(tuple::B2DTuple& rScale, tuple::B2DTuple& rTranslate, double& rRotate, double& rShearX) const
        {
            // when perspective is used, decompose is not made here
            if(!mpM->isLastLineDefault())
                return false;

            // If determinant is zero, decomposition is not possible
            if(0.0 == mpM->doDeterminant())
                return false;

            // copy 2x2 matrix and translate vector to 3x3 matrix
            ::basegfx::matrix::B3DHomMatrix a3DHomMat;

            a3DHomMat.set(0, 0, get(0, 0));
            a3DHomMat.set(0, 1, get(0, 1));
            a3DHomMat.set(1, 0, get(1, 0));
            a3DHomMat.set(1, 1, get(1, 1));
            a3DHomMat.set(0, 2, get(0, 3));
            a3DHomMat.set(1, 2, get(1, 3));

            ::basegfx::tuple::B3DTuple r3DScale, r3DTranslate, r3DRotate, r3DShear;

            if(a3DHomMat.decompose(r3DScale, r3DTranslate, r3DRotate, r3DShear))
            {
                // copy scale values
                rScale.setX(r3DScale.getX());
                rScale.setY(r3DScale.getY());

                // copy shear
                rShearX = r3DShear.getX();

                // copy rotate
                rRotate = r3DRotate.getZ();

                // copy translate
                rTranslate.setX(r3DTranslate.getX());
                rTranslate.setY(r3DTranslate.getY());

                return true;
            }

            return false;
        }
    } // end of namespace matrix
} // end of namespace basegfx

// eof
