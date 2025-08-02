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

#include <vcl/bitmap/BitmapFilter.hxx>

enum class BasicMorphologyOp
{
    erode,
    dilate
};

/* Black is foreground, white is background */
class VCL_DLLPUBLIC BitmapBasicMorphologyFilter : public BitmapFilter
{
public:
    BitmapBasicMorphologyFilter(BasicMorphologyOp op, sal_Int32 nRadius);
    BitmapBasicMorphologyFilter(BasicMorphologyOp op, sal_Int32 nRadius, sal_uInt8 nValueOutside);
    virtual ~BitmapBasicMorphologyFilter();

    virtual Bitmap execute(Bitmap const& rBitmap) const override;

private:
    BasicMorphologyOp m_eOp;
    sal_Int32 m_nRadius;
    sal_uInt8 m_nValueOutside = 0;
    bool m_bUseValueOutside = false;
};

class BitmapErodeFilter final : public BitmapBasicMorphologyFilter
{
public:
    BitmapErodeFilter(sal_Int32 nRadius)
        : BitmapBasicMorphologyFilter(BasicMorphologyOp::erode, nRadius)
    {
    }
    BitmapErodeFilter(sal_Int32 nRadius, sal_uInt8 nValueOutside)
        : BitmapBasicMorphologyFilter(BasicMorphologyOp::erode, nRadius, nValueOutside)
    {
    }
};

class BitmapDilateFilter final : public BitmapBasicMorphologyFilter
{
public:
    BitmapDilateFilter(sal_Int32 nRadius)
        : BitmapBasicMorphologyFilter(BasicMorphologyOp::dilate, nRadius)
    {
    }
    BitmapDilateFilter(sal_Int32 nRadius, sal_uInt8 nValueOutside)
        : BitmapBasicMorphologyFilter(BasicMorphologyOp::dilate, nRadius, nValueOutside)
    {
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
