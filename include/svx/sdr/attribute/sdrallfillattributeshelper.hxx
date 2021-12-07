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
#ifndef INCLUDED_SVX_SDR_ATTRIBUTE_SDRALLFILLATTRIBUTESHELPER_HXX
#define INCLUDED_SVX_SDR_ATTRIBUTE_SDRALLFILLATTRIBUTESHELPER_HXX

#include <svx/svxdllapi.h>
#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <svl/itemset.hxx>
#include <optional>

//////////////////////////////////////////////////////////////////////////////

class Color;

namespace drawinglayer::attribute
    {
        class SVXCORE_DLLPUBLIC SdrAllFillAttributesHelper
        {
        private:
            basegfx::B2DRange                                                   maLastPaintRange;
            basegfx::B2DRange                                                   maLastDefineRange;
            std::optional<drawinglayer::attribute::SdrFillAttribute>            maFillAttribute;
            std::optional<drawinglayer::attribute::FillGradientAttribute>       maFillGradientAttribute;
            drawinglayer::primitive2d::Primitive2DContainer                     maPrimitives;

            void createPrimitive2DSequence(
                const basegfx::B2DRange& rPaintRange,
                const basegfx::B2DRange& rDefineRange);

        public:
            SdrAllFillAttributesHelper(const Color& rColor);
            SdrAllFillAttributesHelper(const SfxItemSet& rSet);
            ~SdrAllFillAttributesHelper();

            bool isUsed() const;
            bool hasSdrFillAttribute() const { return bool(maFillAttribute); }
            bool isTransparent() const;

            const drawinglayer::attribute::SdrFillAttribute& getFillAttribute() const;
            const drawinglayer::attribute::FillGradientAttribute& getFillGradientAttribute() const;
            const drawinglayer::primitive2d::Primitive2DContainer& getPrimitive2DSequence(
                const basegfx::B2DRange& rPaintRange,
                const basegfx::B2DRange& rDefineRange) const;

            // get average fill color; tries to calculate a 'medium' color
            // which e.g. may be used as comparison to decide if other
            // colors are visible
            basegfx::BColor getAverageColor(const basegfx::BColor& rFallback) const;

            // return if a repaint of this content needs a complete repaint. This
            // is e.g. not needed for no fill or color fill (a partial repaint
            // will do the trick), but necessary for everything that is not top-left
            // oriented
            bool needCompleteRepaint() const;
        };

} // end of namespace drawinglayer::attribute

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer::attribute
{
        typedef std::shared_ptr< SdrAllFillAttributesHelper > SdrAllFillAttributesHelperPtr;
} // end of namespace drawinglayer::attribute

//////////////////////////////////////////////////////////////////////////////

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
