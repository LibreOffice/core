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

#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

namespace basegfx
{
    B2DRange::B2DRange( const B2IRange& rRange ) :
        maRangeX(),
        maRangeY()
    {
        if( !rRange.isEmpty() )
        {
            maRangeX = MyBasicRange(rRange.getMinX());
            maRangeY = MyBasicRange(rRange.getMinY());

            maRangeX.expand(rRange.getMaxX());
            maRangeY.expand(rRange.getMaxY());
        }
    }

    void B2DRange::transform(const B2DHomMatrix& rMatrix)
    {
        if(!isEmpty() && !rMatrix.isIdentity())
        {
            const B2DRange aSource(*this);
            reset();
            expand(rMatrix * B2DPoint(aSource.getMinX(), aSource.getMinY()));
            expand(rMatrix * B2DPoint(aSource.getMaxX(), aSource.getMinY()));
            expand(rMatrix * B2DPoint(aSource.getMinX(), aSource.getMaxY()));
            expand(rMatrix * B2DPoint(aSource.getMaxX(), aSource.getMaxY()));
        }
    }

    B2DRange& B2DRange::operator*=( const ::basegfx::B2DHomMatrix& rMat )
    {
        transform(rMat);
        return *this;
    }

    const B2DRange& B2DRange::getUnitB2DRange()
    {
        static const B2DRange aUnitB2DRange(0.0, 0.0, 1.0, 1.0);

        return aUnitB2DRange;
    }

    B2IRange fround(const B2DRange& rRange)
    {
        return rRange.isEmpty() ?
            B2IRange() :
            B2IRange(fround(rRange.getMinimum()),
                     fround(rRange.getMaximum()));
    }

    B2DRange operator*( const ::basegfx::B2DHomMatrix& rMat, const B2DRange& rB2DRange )
    {
        B2DRange aRes( rB2DRange );
        return aRes *= rMat;
    }

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
