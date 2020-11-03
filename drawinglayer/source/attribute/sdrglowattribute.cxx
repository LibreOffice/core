/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawinglayer/attribute/sdrglowattribute.hxx>

namespace drawinglayer::attribute
{
SdrGlowAttribute::SdrGlowAttribute(sal_Int32 nRadius, const Color& rColor)
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

} // end of namespace drawinglayer::attribute

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
