/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _BGFX_VECTOR_B2IVECTOR_HXX
#define _BGFX_VECTOR_B2IVECTOR_HXX

#include <basegfx/tuple/b2ituple.hxx>
#include <basegfx/vector/b2enums.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    // predeclaration
    class B2DHomMatrix;

    /** Base Point class with two sal_Int32 values

        This class derives all operators and common handling for
        a 2D data class from B2ITuple. All necessary extensions
        which are special for 2D Vectors are added here.

        @see B2ITuple
    */
    class BASEGFX_DLLPUBLIC B2IVector : public ::basegfx::B2ITuple
    {
    public:
        /** Create a 2D Vector

            The vector is initialized to (0, 0)
        */
        B2IVector()
        :   B2ITuple()
        {}

        /** Create a 2D Vector

            @param nX
            This parameter is used to initialize the X-coordinate
            of the 2D Vector.

            @param nY
            This parameter is used to initialize the Y-coordinate
            of the 2D Vector.
        */
        B2IVector(sal_Int32 nX, sal_Int32 nY)
        :   B2ITuple(nX, nY)
        {}

        /** Create a copy of a 2D Vector

            @param rVec
            The 2D Vector which will be copied.
        */
        B2IVector(const B2IVector& rVec)
        :   B2ITuple(rVec)
        {}

        /** constructor with tuple to allow copy-constructing
            from B2ITuple-based classes
        */
        B2IVector(const ::basegfx::B2ITuple& rTuple)
        :   B2ITuple(rTuple)
        {}

        ~B2IVector()
        {}

        /** *=operator to allow usage from B2IVector, too
        */
        B2IVector& operator*=( const B2IVector& rPnt )
        {
            mnX *= rPnt.mnX;
            mnY *= rPnt.mnY;
            return *this;
        }

        /** *=operator to allow usage from B2IVector, too
        */
        B2IVector& operator*=(sal_Int32 t)
        {
            mnX *= t;
            mnY *= t;
            return *this;
        }

        /** assignment operator to allow assigning the results
            of B2ITuple calculations
        */
        B2IVector& operator=( const ::basegfx::B2ITuple& rVec );

        /** Set the length of this 2D Vector

            @param fLen
            The to be achieved length of the 2D Vector
        */
        B2IVector& setLength(double fLen);

        /** Calculate the Scalar with another 2D Vector

            @param rVec
            The second 2D Vector

            @return
            The Scalar value of the two involved 2D Vectors
        */
        double scalar( const B2IVector& rVec ) const;

        /** Transform vector by given transformation matrix.

            Since this is a vector, translational components of the
            matrix are disregarded.
        */
        B2IVector& operator*=( const B2DHomMatrix& rMat );
    };

    // external operators
    //////////////////////////////////////////////////////////////////////////

    /** Transform vector by given transformation matrix.

        Since this is a vector, translational components of the
        matrix are disregarded.
    */
    BASEGFX_DLLPUBLIC B2IVector operator*( const B2DHomMatrix& rMat, const B2IVector& rVec );

} // end of namespace basegfx

#endif /* _BGFX_VECTOR_B2IVECTOR_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
