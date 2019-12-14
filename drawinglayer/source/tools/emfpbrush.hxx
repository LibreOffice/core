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

#ifndef INCLUDED_DRAWINGLAYER_SOURCE_TOOLS_EMFPBRUSH_HXX
#define INCLUDED_DRAWINGLAYER_SOURCE_TOOLS_EMFPBRUSH_HXX

#include "emfphelperdata.hxx"
#include "emfpenums.hxx"
#include "emfpimage.hxx"

#include <tools/color.hxx>

namespace emfplushelper
{
    struct EMFPPath;

    struct EMFPBrush : public EMFPObject
    {
        ::Color solidColor;
        sal_uInt32 type;
        sal_uInt32 datasize;
        sal_uInt32 additionalFlags;

        /* linear gradient */
        sal_Int32 wrapMode;
        float firstPointX, firstPointY, secondPointX, secondPointY;
        ::Color secondColor; // first color is stored in solidColor;
        basegfx::B2DHomMatrix brush_transformation;
        bool hasTransformation;
        sal_Int32 blendPoints;
        std::unique_ptr<float[]> blendPositions;
        float* blendFactors;
        sal_Int32 colorblendPoints;
        std::unique_ptr<float[]> colorblendPositions;
        std::unique_ptr<::Color[]> colorblendColors;
        sal_Int32 surroundColorsNumber;
        std::unique_ptr<::Color[]> surroundColors;
        std::unique_ptr<EMFPPath> path;
        EmfPlusHatchStyle hatchStyle;
        std::unique_ptr<EMFPImage> image;

        EMFPBrush(sal_uInt32 datasize);
        virtual ~EMFPBrush() override;

        sal_uInt32 GetType() const { return type; }
        const ::Color& GetColor() const { return solidColor; }
        ::basegfx::B2DHomMatrix GetTextureTransformation(::basegfx::B2DHomMatrix const& rMapTransform);

        void Read(SvStream& s, EmfPlusHelperData const & rR);
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
