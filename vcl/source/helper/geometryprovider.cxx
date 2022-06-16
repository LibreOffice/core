/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/GeometryProvider.hxx>

using namespace vcl;

SalGeometryProvider::~SalGeometryProvider() {}

sal_Int32 SalGeometryProviderImpl::GetSgpMetric(SGPmetric eMetric) const
{
    switch (eMetric)
    {
        case SGPmetric::Width:
            return m_nWidth;
        case SGPmetric::Height:
            return m_nHeight;
        case SGPmetric::DPIX:
            return mnDPIX;
        case SGPmetric::DPIY:
            return mnDPIY;
        case SGPmetric::ScalePercentage:
            return mnDPIScalePercentage;
        case SGPmetric::OffScreen:
            return m_bOffScreen;
        case SGPmetric::BitCount:
            return mnBitCount;
    }
    return -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
