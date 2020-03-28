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

    double get(sal_uInt16 nRow, sal_uInt16 nColumn) const
    {
        if (nRow == 0)
        {
            if (nColumn == 0)
                return v00;
            else if (nColumn == 1)
                return v01;
            else if (nColumn == 2)
                return v02;
        }
        else if (nRow == 1)
        {
            if (nColumn == 0)
                return v10;
            else if (nColumn == 1)
                return v11;
            else if (nColumn == 2)
                return v12;
        }
        return 0.0;
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
        double newA = ma * other.ma + mb * other.mc;
        double newB = ma * other.mb + mb * other.md;
        double newC = mc * other.ma + md * other.mc;
        double newD = mc * other.mb + md * other.md;
        double newE = me * other.ma + mf * other.mc + other.me;
        double newF = me * other.mb + mf * other.md + other.mf;

        ma = newA;
        mb = newB;
        mc = newC;
        md = newD;
        me = newE;
        mf = newF;
    }

    /// Transform the point (x, y) by this Matrix.
    template <typename T> void Transform(T& x, T& y)
    {
        T newX = v00 * x + v01 * y + v02;
        T newY = v10 * x + v11 * y + v12;
        x = newX;
        y = newY;
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
    union {
        struct
        {
            double ma;
            double mb;
            double mc;
            double md;
            double me;
            double mf;
        };

        struct
        {
            double v00;
            double v10;
            double v01;
            double v11;
            double v02;
            double v12;
        };
    };
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
