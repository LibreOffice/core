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

class Matrix
{
public:
    Matrix()
        : Matrix(1, 0, 0, 1, 0, 0)
    {
    }

    Matrix(const Matrix& other)
        : Matrix(other.ma, other.mb, other.mc, other.md, other.me, other.mf)
    {
    }

    Matrix(double da, double db, double dc, double dd, double de, double df)
        : ma(da)
        , mb(db)
        , mc(dc)
        , md(dd)
        , me(de)
        , mf(df)
    {
    }

    const Matrix& operator=(const Matrix& other)
    {
        ma = other.ma;
        mb = other.mb;
        mc = other.mc;
        md = other.md;
        me = other.me;
        mf = other.mf;
        return *this;
    }

    double a() const { return ma; }
    double b() const { return mb; }
    double c() const { return mc; }
    double d() const { return md; }
    double e() const { return me; }
    double f() const { return mf; }

    /// Multiply this * other.
    void Concatinate(const Matrix& other)
    {
        ma = ma * other.ma + mb * other.mc;
        mb = ma * other.mb + mb * other.md;
        mc = mc * other.ma + md * other.mc;
        md = mc * other.mb + md * other.md;
        me = me * other.ma + mf * other.mc + other.me;
        mf = me * other.mb + mf * other.md + other.mf;
    }

    /// Transform the point (x, y) by this Matrix.
    template <typename T> void Transform(T& x, T& y)
    {
        x = ma * x + mc * y + me;
        y = mb * x + md * y + mf;
    }

    /// Transform the rectangle (left, right, top, bottom) by this Matrix.
    template <typename T> void Transform(T& left, T& right, T& top, T& bottom)
    {
        T leftTopX = left;
        T leftTopY = top;
        Transform(leftTopX, leftTopY);

        T leftBottomX = left;
        T leftBottomY = bottom;
        Transform(leftBottomX, leftBottomY);

        T rightTopX = right;
        T rightTopY = top;
        Transform(rightTopX, rightTopY);

        T rightBottomX = right;
        T rightBottomY = bottom;
        Transform(rightBottomX, rightBottomY);

        left = std::min(leftTopX, leftBottomX);
        right = std::max(rightTopX, rightBottomX);

        if (top > bottom)
            top = std::max(leftTopY, rightTopY);
        else
            top = std::min(leftTopY, rightTopY);

        if (top > bottom)
            bottom = std::max(leftBottomY, rightBottomY);
        else
            bottom = std::min(leftBottomY, rightBottomY);
    }

    std::string toString() const
    {
        std::ostringstream oss;
        oss << '(' << ma << ", " << mb << ", " << mc << ", " << md << ", " << me << ", " << mf
            << ')';
        return oss.str();
    }

private:
    double ma, mb, mc, md, me, mf;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
