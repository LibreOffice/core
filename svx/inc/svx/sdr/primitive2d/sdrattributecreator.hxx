/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
