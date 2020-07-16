/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRGLOWATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRGLOWATTRIBUTE_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <tools/color.hxx>

namespace drawinglayer::attribute
{
class DRAWINGLAYER_DLLPUBLIC SdrGlowAttribute
{
private:
    sal_Int32 m_nRadius = 0;
    Color m_color; // Includes alpha!

public:
    SdrGlowAttribute(sal_Int32 nRadius, const Color& rColor);
    SdrGlowAttribute();
    SdrGlowAttribute(const SdrGlowAttribute&);
    SdrGlowAttribute(SdrGlowAttribute&&);
    ~SdrGlowAttribute();
    bool operator==(const SdrGlowAttribute& rCandidate) const;

    SdrGlowAttribute& operator=(const SdrGlowAttribute&);
    SdrGlowAttribute& operator=(SdrGlowAttribute&&);

    // data access
    const Color& getColor() const { return m_color; }
    sal_Int32 getRadius() const { return m_nRadius; }
    bool isDefault() const { return m_nRadius == 0; }
};

} // end of namespace drawinglayer::attribute

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRGLOWATTRIBUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
