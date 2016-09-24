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

#ifndef INCLUDED_VCL_RESAMPLEKERNEL_HXX
#define INCLUDED_VCL_RESAMPLEKERNEL_HXX

#include <boost/math/special_functions/sinc.hpp>

namespace vcl {

// Resample kernels

class Kernel
{
public:
             Kernel() {}
    virtual ~Kernel() {}

    virtual double  GetWidth() const = 0;
    virtual double  Calculate( double x ) const = 0;
};

class Lanczos3Kernel : public Kernel
{
public:
    Lanczos3Kernel() : Kernel () {}

    virtual double  GetWidth() const override { return 3.0; }
    virtual double  Calculate (double x) const override
    {
        return (-3.0 <= x && x < 3.0) ? SincFilter(x) * SincFilter( x / 3.0 ) : 0.0;
    }

    static inline double SincFilter(double x)
    {
        if (x == 0.0)
        {
            return 1.0;
        }
        x = x * M_PI;
        return boost::math::sinc_pi(x, SincPolicy());
    }

private:
    typedef boost::math::policies::policy<
        boost::math::policies::promote_double<false> > SincPolicy;
};

class BicubicKernel : public Kernel
{
public:
    BicubicKernel() : Kernel () {}

private:
    virtual double  GetWidth() const override { return 2.0; }
    virtual double  Calculate (double x) const override
    {
        if (x < 0.0)
        {
            x = -x;
        }

        if (x <= 1.0)
        {
            return (1.5 * x - 2.5) * x * x + 1.0;
        }
        else if (x < 2.0)
        {
            return ((-0.5 * x + 2.5) * x - 4) * x + 2;
        }
        return 0.0;
    }
};

class BilinearKernel : public Kernel
{
public:
    BilinearKernel() : Kernel () {}

private:
    virtual double  GetWidth() const override { return 1.0; }
    virtual double  Calculate (double x) const override
    {
        if (x < 0.0)
        {
            x = -x;
        }
        if (x < 1.0)
        {
            return 1.0-x;
        }
        return 0.0;
    }
};

class BoxKernel : public Kernel
{
public:
    BoxKernel() : Kernel () {}

private:
    virtual double  GetWidth() const override { return 0.5; }
    virtual double  Calculate (double x) const override
    {
        if (-0.5 <= x && x < 0.5)
            return 1.0;
        return 0.0;
    }
};

} // namespace vcl

#endif // INCLUDED_VCL_RESAMPLEKERNEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
