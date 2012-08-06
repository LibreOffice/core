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

#ifndef INCLUDED_SDR_PRIMITIVE2D_SDRATTRIBUTECREATOR_HXX
#define INCLUDED_SDR_PRIMITIVE2D_SDRATTRIBUTECREATOR_HXX

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////
// predefines
class SfxItemSet;
class SdrText;

namespace drawinglayer { namespace attribute {
    class SdrLineAttribute;
    class SdrLineStartEndAttribute;
    class SdrShadowAttribute;
    class SdrFillAttribute;
    class SdrTextAttribute;
    class FillGradientAttribute;
    class SdrFillBitmapAttribute;
    class SdrShadowTextAttribute;
    class SdrLineShadowTextAttribute;
    class SdrLineFillShadowTextAttribute;
    class SdrLineFillShadowAttribute3D;
    class SdrSceneAttribute;
    class SdrLightingAttribute;
    class SdrFillTextAttribute;
}}

namespace basegfx {
    class B2DRange;
}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        // SdrAttribute creators
        attribute::SdrLineAttribute createNewSdrLineAttribute(
            const SfxItemSet& rSet);

        attribute::SdrLineStartEndAttribute createNewSdrLineStartEndAttribute(
            const SfxItemSet& rSet,
            double fWidth);

        attribute::SdrShadowAttribute createNewSdrShadowAttribute(
            const SfxItemSet& rSet);

        attribute::SdrFillAttribute createNewSdrFillAttribute(
            const SfxItemSet& rSet);

        // #i101508# Support handing over given text-to-border distances
        attribute::SdrTextAttribute createNewSdrTextAttribute(
            const SfxItemSet& rSet,
            const SdrText& rText,
            const sal_Int32* pLeft = 0,
            const sal_Int32* pUpper = 0,
            const sal_Int32* pRight = 0,
            const sal_Int32* pLower = 0);

        attribute::FillGradientAttribute createNewTransparenceGradientAttribute(
            const SfxItemSet& rSet);

        attribute::SdrFillBitmapAttribute createNewSdrFillBitmapAttribute(
            const SfxItemSet& rSet);

        attribute::SdrShadowTextAttribute createNewSdrShadowTextAttribute(
            const SfxItemSet& rSet,
            const SdrText* pText,
            bool bSuppressText); // #i98072# added option to suppress text on demand

        attribute::SdrLineShadowTextAttribute createNewSdrLineShadowTextAttribute(
            const SfxItemSet& rSet,
            const SdrText* pText);

        attribute::SdrLineFillShadowTextAttribute createNewSdrLineFillShadowTextAttribute(
            const SfxItemSet& rSet,
            const SdrText* pText);

        attribute::SdrLineFillShadowAttribute3D createNewSdrLineFillShadowAttribute(
            const SfxItemSet& rSet,
            bool bSuppressFill);

        attribute::SdrSceneAttribute createNewSdrSceneAttribute(
            const SfxItemSet& rSet);

        attribute::SdrLightingAttribute createNewSdrLightingAttribute(
            const SfxItemSet& rSet);

        // #i101508# Support handing over given text-to-border distances
        attribute::SdrFillTextAttribute createNewSdrFillTextAttribute(
            const SfxItemSet& rSet,
            const SdrText* pSdrText,
            const sal_Int32* pLeft = 0,
            const sal_Int32* pUpper = 0,
            const sal_Int32* pRight = 0,
            const sal_Int32* pLower = 0);

        // helpers
        void calculateRelativeCornerRadius(
            sal_Int32 nRadius,
            const ::basegfx::B2DRange& rObjectRange,
            double& rfCornerRadiusX,
            double& rfCornerRadiusY);

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SDR_PRIMITIVE2D_SDRATTRIBUTECREATOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
