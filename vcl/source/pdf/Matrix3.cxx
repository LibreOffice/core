/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <pdf/Matrix3.hxx>
#include <cmath>

namespace vcl::pdf
{
Matrix3::Matrix3()
{
    // initialize to unity
    f[0] = 1.0;
    f[1] = 0.0;
    f[2] = 0.0;
    f[3] = 1.0;
    f[4] = 0.0;
    f[5] = 0.0;
}

Point Matrix3::transform(const Point& rOrig) const
{
    double x = static_cast<double>(rOrig.X()), y = static_cast<double>(rOrig.Y());
    return Point(x * f[0] + y * f[2] + f[4], x * f[1] + y * f[3] + f[5]);
}

void Matrix3::skew(double alpha, double beta)
{
    double fn[6];
    double tb = tan(beta);
    fn[0] = f[0] + f[2] * tb;
    fn[1] = f[1];
    fn[2] = f[2] + f[3] * tb;
    fn[3] = f[3];
    fn[4] = f[4] + f[5] * tb;
    fn[5] = f[5];
    if (alpha != 0.0)
    {
        double ta = tan(alpha);
        fn[1] += f[0] * ta;
        fn[3] += f[2] * ta;
        fn[5] += f[4] * ta;
    }
    set(fn);
}

void Matrix3::scale(double sx, double sy)
{
    double fn[6];
    fn[0] = sx * f[0];
    fn[1] = sy * f[1];
    fn[2] = sx * f[2];
    fn[3] = sy * f[3];
    fn[4] = sx * f[4];
    fn[5] = sy * f[5];
    set(fn);
}

void Matrix3::rotate(double angle)
{
    double fn[6];
    double fSin = sin(angle);
    double fCos = cos(angle);
    fn[0] = f[0] * fCos - f[1] * fSin;
    fn[1] = f[0] * fSin + f[1] * fCos;
    fn[2] = f[2] * fCos - f[3] * fSin;
    fn[3] = f[2] * fSin + f[3] * fCos;
    fn[4] = f[4] * fCos - f[5] * fSin;
    fn[5] = f[4] * fSin + f[5] * fCos;
    set(fn);
}

void Matrix3::translate(double tx, double ty)
{
    f[4] += tx;
    f[5] += ty;
}

void Matrix3::invert()
{
    // short circuit trivial cases
    if (f[1] == f[2] && f[1] == 0.0 && f[0] == f[3] && f[0] == 1.0)
    {
        f[4] = -f[4];
        f[5] = -f[5];
        return;
    }

    // check determinant
    const double fDet = f[0] * f[3] - f[1] * f[2];
    if (fDet == 0.0)
        return;

    // invert the matrix
    double fn[6];
    fn[0] = +f[3] / fDet;
    fn[1] = -f[1] / fDet;
    fn[2] = -f[2] / fDet;
    fn[3] = +f[0] / fDet;

    // apply inversion to translation
    fn[4] = -(f[4] * fn[0] + f[5] * fn[2]);
    fn[5] = -(f[4] * fn[1] + f[5] * fn[3]);

    set(fn);
}

} // end vcl::pdf

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
