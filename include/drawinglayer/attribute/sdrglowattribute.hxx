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
#include <o3tl/cow_wrapper.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/range/b2drange.hxx>

#include <optional>

namespace drawinglayer
{
namespace attribute
{
class DRAWINGLAYER_DLLPUBLIC SdrGlowAttribute
{
private:
    sal_Int32 m_nRadius = 0;
    mutable std::optional<basegfx::B2DHomMatrix> m_oTransfCache;
    basegfx::BColor m_color;

public:
    SdrGlowAttribute(sal_Int32 nRadius, const basegfx::BColor& rColor);
    SdrGlowAttribute();
    SdrGlowAttribute(const SdrGlowAttribute&);
    SdrGlowAttribute(SdrGlowAttribute&&);
    ~SdrGlowAttribute();
    bool operator==(const SdrGlowAttribute& rCandidate) const;

    SdrGlowAttribute& operator=(const SdrGlowAttribute&);
    SdrGlowAttribute& operator=(SdrGlowAttribute&&);

    // data access
    const basegfx::B2DHomMatrix& GetTransfMatrix(basegfx::B2DRange nCenter) const;
    const basegfx::BColor& getColor() const { return m_color; };
    sal_Int32 getRadius() const { return m_nRadius; };
    bool isDefault() const { return m_nRadius == 0; };
};
} // end of namespace attribute
} // end of namespace drawinglayer

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRGLOWATTRIBUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
