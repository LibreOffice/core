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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basegfx.hxx"
#include <basegfx/range/b3drange.hxx>
#include <basegfx/range/b3irange.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>

namespace basegfx
{
    B3DRange::B3DRange(const B3IRange& rRange) :
        maRangeX(),
        maRangeY(),
        maRangeZ()
    {
        if( !rRange.isEmpty() )
        {
            maRangeX = rRange.getMinX();
            maRangeY = rRange.getMinY();
            maRangeZ = rRange.getMinZ();

            maRangeX.expand( rRange.getMaxX() );
            maRangeY.expand( rRange.getMaxY() );
            maRangeZ.expand( rRange.getMaxZ() );
        }
    }

    void B3DRange::transform(const B3DHomMatrix& rMatrix)
    {
        if(!isEmpty() && !rMatrix.isIdentity())
        {
            const B3DRange aSource(*this);
            reset();
            expand(rMatrix * B3DPoint(aSource.getMinX(), aSource.getMinY(), aSource.getMinZ()));
            expand(rMatrix * B3DPoint(aSource.getMaxX(), aSource.getMinY(), aSource.getMinZ()));
            expand(rMatrix * B3DPoint(aSource.getMinX(), aSource.getMaxY(), aSource.getMinZ()));
            expand(rMatrix * B3DPoint(aSource.getMaxX(), aSource.getMaxY(), aSource.getMinZ()));
            expand(rMatrix * B3DPoint(aSource.getMinX(), aSource.getMinY(), aSource.getMaxZ()));
            expand(rMatrix * B3DPoint(aSource.getMaxX(), aSource.getMinY(), aSource.getMaxZ()));
            expand(rMatrix * B3DPoint(aSource.getMinX(), aSource.getMaxY(), aSource.getMaxZ()));
            expand(rMatrix * B3DPoint(aSource.getMaxX(), aSource.getMaxY(), aSource.getMaxZ()));
        }
    }

    B3IRange fround(const B3DRange& rRange )
    {
        return rRange.isEmpty() ?
            B3IRange() :
            B3IRange(fround(rRange.getMinX()),
                     fround(rRange.getMinY()),
                     fround(rRange.getMinZ()),
                     fround(rRange.getMaxX()),
                     fround(rRange.getMaxY()),
                     fround(rRange.getMaxZ()));
    }

} // end of namespace basegfx

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
