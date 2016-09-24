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

#ifndef INCLUDED_BASEGFX_VECTOR_B2IVECTOR_HXX
#define INCLUDED_BASEGFX_VECTOR_B2IVECTOR_HXX

#include <ostream>

#include <basegfx/tuple/b2ituple.hxx>
#include <basegfx/vector/b2enums.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
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
        double scalar( const B2IVector& rVec ) const { return((mnX * rVec.mnX) + (mnY * rVec.mnY)); }

        /** Transform vector by given transformation matrix.

            Since this is a vector, translational components of the
            matrix are disregarded.
        */
        B2IVector& operator*=( const B2DHomMatrix& rMat );
    };

    // external operators

    template< typename charT, typename traits >
    inline std::basic_ostream<charT, traits> & operator <<(
        std::basic_ostream<charT, traits> & stream, const basegfx::B2IVector& vector )
    {
        return stream << "(" << vector.getX() << "," << vector.getY() << ")";
    }

} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_VECTOR_B2IVECTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
