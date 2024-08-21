/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawinglayer/attribute/sdrglowtextattribute.hxx>

namespace drawinglayer::attribute
{
SdrGlowTextAttribute::SdrGlowTextAttribute(sal_Int32 nTextRadius, const Color& rTextColor)
    : m_nTextRadius(nTextRadius)
    , m_TextColor(rTextColor)
{
}

SdrGlowTextAttribute::SdrGlowTextAttribute() = default;

SdrGlowTextAttribute::SdrGlowTextAttribute(const SdrGlowTextAttribute&) = default;

SdrGlowTextAttribute::SdrGlowTextAttribute(SdrGlowTextAttribute&&) = default;

SdrGlowTextAttribute& SdrGlowTextAttribute::operator=(const SdrGlowTextAttribute&) = default;

SdrGlowTextAttribute& SdrGlowTextAttribute::operator=(SdrGlowTextAttribute&&) = default;

bool SdrGlowTextAttribute::operator==(const SdrGlowTextAttribute& rCandidate) const
{
    return m_nTextRadius == rCandidate.m_nTextRadius && m_TextColor == rCandidate.m_TextColor;
}

} // end of namespace drawinglayer::attribute

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
