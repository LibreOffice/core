/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawinglayer/attribute/sdrglowattribute.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/color/bcolor.hxx>
#include <rtl/instance.hxx>

#include <utility>

#include <sal/log.hxx>

namespace drawinglayer
{
namespace attribute
{
SdrGlowAttribute::SdrGlowAttribute(sal_Int32 nRadius, const basegfx::BColor& rColor)
    : m_nRadius(nRadius)
    , m_color(rColor)
{
}

SdrGlowAttribute::SdrGlowAttribute() = default;

SdrGlowAttribute::SdrGlowAttribute(const SdrGlowAttribute&) = default;

SdrGlowAttribute::SdrGlowAttribute(SdrGlowAttribute&&) = default;

SdrGlowAttribute::~SdrGlowAttribute() = default;

SdrGlowAttribute& SdrGlowAttribute::operator=(const SdrGlowAttribute&) = default;

SdrGlowAttribute& SdrGlowAttribute::operator=(SdrGlowAttribute&&) = default;

bool SdrGlowAttribute::operator==(const SdrGlowAttribute& rCandidate) const
{
    return m_nRadius == rCandidate.m_nRadius && m_color == rCandidate.m_color;
}

const basegfx::B2DHomMatrix& SdrGlowAttribute::GetTransfMatrix(basegfx::B2DRange nRange) const
{
    if (!m_oTransfCache)
    {
        double dRadius100mm = static_cast<double>(m_nRadius) / 360.0;
        // Apply a scaling with the center point of the shape as origin.
        // 1) translate shape to the origin
        basegfx::B2DHomMatrix matrix = basegfx::utils::createCoordinateSystemTransform(
            nRange.getCenter(), basegfx::B2DVector(-1, 0), basegfx::B2DVector(0, -1));

        basegfx::B2DHomMatrix inverse(matrix);
        inverse.invert();

        // 2) Scale up
        double scale_x = (nRange.getWidth() + dRadius100mm) / nRange.getWidth();
        double scale_y = (nRange.getHeight() + dRadius100mm) / nRange.getHeight();
        matrix *= basegfx::utils::createScaleB2DHomMatrix(scale_x, scale_y);

        // 3) Translate shape back to its place
        matrix *= inverse;
        m_oTransfCache = std::move(matrix);
    }
    return *m_oTransfCache;
}

} // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
