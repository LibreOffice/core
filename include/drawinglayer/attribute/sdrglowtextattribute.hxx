/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRGLOWTEXTATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRGLOWTEXTATTRIBUTE_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <tools/color.hxx>

namespace drawinglayer::attribute
{
class DRAWINGLAYER_DLLPUBLIC SdrGlowTextAttribute
{
private:
    sal_Int32 m_nTextRadius = 0;
    Color m_TextColor; // Includes alpha!

public:
    SdrGlowTextAttribute(sal_Int32 nTextRadius, const Color& rTextColor);
    SdrGlowTextAttribute();
    SdrGlowTextAttribute(const SdrGlowTextAttribute&);
    SdrGlowTextAttribute(SdrGlowTextAttribute&&);

    bool operator==(const SdrGlowTextAttribute& rCandidate) const;

    SdrGlowTextAttribute& operator=(const SdrGlowTextAttribute&);
    SdrGlowTextAttribute& operator=(SdrGlowTextAttribute&&);

    // data access
    const Color& getTextColor() const { return m_TextColor; }
    sal_Int32 getTextRadius() const { return m_nTextRadius; }
    bool isDefault() const { return m_nTextRadius == 0; }
};

} // end of namespace drawinglayer::attribute

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRGLOWTEXTATTRIBUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
