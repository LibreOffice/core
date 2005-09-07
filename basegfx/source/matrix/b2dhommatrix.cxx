/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dhommatrix.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:42:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

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

namespace basegfx
{
    class Impl2DHomMatrix : public ::basegfx::internal::ImplHomMatrixTemplate< 3 >
    {
    };

    static Impl2DHomMatrix& get2DIdentityMatrix()
    {
        static Impl2DHomMatrix maStatic2DIdentityHomMatrix;
        return maStatic2DIdentityHomMatrix;
    }

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
            delete[] pIndex;

            return true;
        }

        delete[] pIndex;
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

        if(!::basegfx::fTools::equal(fOne, fValue))
        {
            implPrepareChange();
            mpM->doMulMatrix(fValue);
        }

        return *this;
    }

    B2DHomMatrix& B2DHomMatrix::operator/=(double fValue)
    {
        const double fOne(1.0);

        if(!::basegfx::fTools::equal(fOne, fValue))
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
        if(!::basegfx::fTools::equalZero(fRadiant))
        {
            double fSin;
            double fCos;

            // is the rotation angle an approximate multiple of pi/2?
            // If yes, force fSin/fCos to -1/0/1, to maintain
            // orthogonality (which might also be advantageous for the
            // other cases, but: for multiples of pi/2, the exact
            // values _can_ be attained. It would be largely
            // unintuitive, if a 180 degrees rotation would introduce
            // slight roundoff errors, instead of exactly mirroring
            // the coordinate system).
            if( fTools::equalZero( fmod( fRadiant, F_PI2 ) ) )
            {
                // determine quadrant
                const sal_Int32 nQuad(
                    (4 + fround( 4/F_2PI*fmod( fRadiant, F_2PI ) )) % 4 );
                switch( nQuad )
                {
                    case 0: // -2pi,0,2pi
                        fSin = 0.0;
                        fCos = 1.0;
                        break;

                    case 1: // -3/2pi,1/2pi
                        fSin = 1.0;
                        fCos = 0.0;
                        break;

                    case 2: // -pi,pi
                        fSin = 0.0;
                        fCos = -1.0;
                        break;

                    case 3: // -1/2pi,3/2pi
                        fSin = -1.0;
                        fCos = 0.0;
                        break;

                    default:
                        OSL_ENSURE( false,
                                    "B2DHomMatrix::rotate(): Impossible case reached" );
                }
            }
            else
            {
                fSin = sin(fRadiant);
                fCos = cos(fRadiant);
            }

            Impl2DHomMatrix aRotMat(get2DIdentityMatrix());

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
        if(!::basegfx::fTools::equalZero(fX) || !::basegfx::fTools::equalZero(fY))
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

        if(!::basegfx::fTools::equal(fOne, fX) || !::basegfx::fTools::equal(fOne, fY))
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

        if(!::basegfx::fTools::equal(fOne, fSx))
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

        if(!::basegfx::fTools::equal(fOne, fSy))
        {
            Impl2DHomMatrix aShearYMat(get2DIdentityMatrix());

            aShearYMat.set(1, 0, fSy);

            implPrepareChange();
            mpM->doMulMatrix(aShearYMat);
        }
    }

    // Decomposition
    bool B2DHomMatrix::decompose(B2DTuple& rScale, B2DTuple& rTranslate, double& rRotate, double& rShearX) const
    {
        // when perspective is used, decompose is not made here
        if(!mpM->isLastLineDefault())
            return false;

        // If determinant is zero, decomposition is not possible
        if(0.0 == mpM->doDeterminant())
            return false;

        // copy 2x2 matrix and translate vector to 3x3 matrix
        ::basegfx::B3DHomMatrix a3DHomMat;

        a3DHomMat.set(0, 0, get(0, 0));
        a3DHomMat.set(0, 1, get(0, 1));
        a3DHomMat.set(1, 0, get(1, 0));
        a3DHomMat.set(1, 1, get(1, 1));
        a3DHomMat.set(0, 3, get(0, 2));
        a3DHomMat.set(1, 3, get(1, 2));

        ::basegfx::B3DTuple r3DScale, r3DTranslate, r3DRotate, r3DShear;

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
} // end of namespace basegfx

// eof
