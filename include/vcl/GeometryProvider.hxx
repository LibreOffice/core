/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dllapi.h>
#include <tools/gen.hxx>

namespace vcl
{
enum class SGPmetric
{
    Width = 1,
    Height,
    DPIX,
    DPIY,
    ScalePercentage,
    OffScreen,
    BitCount,
};

class VCL_DLLPUBLIC SalGeometryProvider
{
public:
    virtual ~SalGeometryProvider();

    sal_Int32 GetWidth() const { return GetSgpMetric(SGPmetric::Width); }
    sal_Int32 GetHeight() const { return GetSgpMetric(SGPmetric::Height); }
    sal_Int32 GetDPIX() const { return GetSgpMetric(SGPmetric::DPIX); }
    sal_Int32 GetDPIY() const { return GetSgpMetric(SGPmetric::DPIY); }
    sal_Int32 GetDPIScalePercentage() const { return GetSgpMetric(SGPmetric::ScalePercentage); }
    float GetDPIScaleFactor() const { return GetSgpMetric(SGPmetric::ScalePercentage) / 100.0f; }
    bool IsOffScreen() const { return !!GetSgpMetric(SGPmetric::OffScreen); }
    sal_Int32 GetBitCount() const { return GetSgpMetric(SGPmetric::BitCount); }
    Size GetSizePixel() const
    {
        return Size(GetSgpMetric(SGPmetric::Width), GetSgpMetric(SGPmetric::Height));
    }
    // alias for SalBitmap
    Size GetSize() const { return GetSizePixel(); }

    virtual sal_Int32 GetSgpMetric(SGPmetric eMetric) const = 0;
};

struct VCL_DLLPUBLIC SalSgpMetrics
{
    sal_Int32 m_nWidth = 1;
    sal_Int32 m_nHeight = 1;
    sal_Int32 mnDPIX = 96;
    sal_Int32 mnDPIY = 96;
    sal_Int32 mnDPIScalePercentage = 100;
    bool m_bOffScreen = false;
    sal_Int32 mnBitCount = 32;
};

class VCL_DLLPUBLIC SalGeometryProviderImpl : public SalGeometryProvider, protected SalSgpMetrics
{
public:
    SalGeometryProviderImpl() = default;

    virtual sal_Int32 GetSgpMetric(SGPmetric eMetric) const override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
