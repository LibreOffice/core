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

#pragma once

#include <basegfx/tuple/Size2D.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/vector/b2isize.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
class B2DSize : public Size2D<double>
{
public:
    B2DSize()
        : Size2D(0.0, 0.0)
    {
    }

    B2DSize(double fX, double fY)
        : Size2D(fX, fY)
    {
    }

    B2DSize(Size2D<double> const& rSize)
        : Size2D(rSize)
    {
    }

    explicit B2DSize(B2ISize const& rSize)
        : Size2D(rSize.getX(), rSize.getY())
    {
    }

    /** Transform size by given transformation matrix. */
    B2DSize& operator*=(const B2DHomMatrix& rMatrix)
    {
        const double fTempX(rMatrix.get(0, 0) * getWidth() + rMatrix.get(0, 1) * getHeight());
        const double fTempY(rMatrix.get(1, 0) * getWidth() + rMatrix.get(1, 1) * getHeight());
        setWidth(fTempX);
        setHeight(fTempY);
        return *this;
    }

    using Size2D<double>::operator+=;
    using Size2D<double>::operator-=;
    using Size2D<double>::operator*=;
    using Size2D<double>::operator/=;
    using Size2D<double>::operator-;

    double getLength() const
    {
        if (fTools::equalZero(getWidth()))
        {
            return fabs(getHeight());
        }
        else if (fTools::equalZero(getHeight()))
        {
            return fabs(getWidth());
        }

        return hypot(getWidth(), getHeight());
    }
};

inline B2DSize operator*(B2DHomMatrix const& rMatrix, B2DSize const& rSize)
{
    B2DSize aRes(rSize);
    aRes *= rMatrix;
    return aRes;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
