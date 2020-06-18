/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <vcl/dllapi.h>
#include <tools/gen.hxx>

namespace vcl::pdf
{
// matrix helper class
// TODO: use basegfx matrix class instead or derive from it

/*  for sparse matrices of the form (2D linear transformations)
 *  f[0] f[1] 0
 *  f[2] f[3] 0
 *  f[4] f[5] 1
 */
class Matrix3
{
    double f[6];

    void set(const double* pn)
    {
        for (int i = 0; i < 6; i++)
            f[i] = pn[i];
    }

public:
    Matrix3();

    void skew(double alpha, double beta);
    void scale(double sx, double sy);
    void rotate(double angle);
    void translate(double tx, double ty);
    void invert();

    double get(size_t i) const { return f[i]; }

    Point transform(const Point& rPoint) const;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
