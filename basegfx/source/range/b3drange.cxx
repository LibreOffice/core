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

#include <basegfx/range/b3drange.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>

namespace basegfx
{
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

    B3DRange& B3DRange::operator*=( const ::basegfx::B3DHomMatrix& rMat )
    {
        transform(rMat);
        return *this;
    }

    const B3DRange& B3DRange::getUnitB3DRange()
    {
        static const B3DRange aUnitB3DRange(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);

        return aUnitB3DRange;
    }

    B3DRange operator*( const ::basegfx::B3DHomMatrix& rMat, const B3DRange& rB3DRange )
    {
        B3DRange aRes( rB3DRange );
        return aRes *= rMat;
    }

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
