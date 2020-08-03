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

#pragma once

#include "emfpbrush.hxx"
#include <vector>

namespace emfplushelper
{
    const sal_uInt32 EmfPlusLineCapTypeSquare = 0x00000001;
    const sal_uInt32 EmfPlusLineCapTypeRound = 0x00000002;

    const sal_uInt32 EmfPlusLineJoinTypeMiter = 0x00000000;
    const sal_uInt32 EmfPlusLineJoinTypeBevel = 0x00000001;
    const sal_uInt32 EmfPlusLineJoinTypeRound = 0x00000002;
    const sal_uInt32 EmfPlusLineJoinTypeMiterClipped = 0x00000003;

    const sal_Int32 EmfPlusLineStyleSolid = 0x00000000;
    const sal_Int32 EmfPlusLineStyleDash = 0x00000001;
    const sal_Int32 EmfPlusLineStyleDot = 0x00000002;
    const sal_Int32 EmfPlusLineStyleDashDot = 0x00000003;
    const sal_Int32 EmfPlusLineStyleDashDotDot = 0x00000004;
    const sal_Int32 EmfPlusLineStyleCustom = 0x00000005;

    const sal_uInt32 EmfPlusPenDataTransform = 0x00000001;
    const sal_uInt32 EmfPlusPenDataStartCap = 0x00000002;
    const sal_uInt32 EmfPlusPenDataEndCap = 0x00000004;
    const sal_uInt32 EmfPlusPenDataJoin = 0x00000008;
    const sal_uInt32 EmfPlusPenDataMiterLimit = 0x00000010;
    const sal_uInt32 EmfPlusPenDataLineStyle = 0x00000020;
    const sal_uInt32 EmfPlusPenDataDashedLineCap = 0x00000040;
    const sal_uInt32 EmfPlusPenDataDashedLineOffset = 0x00000080;
    const sal_uInt32 EmfPlusPenDataDashedLine = 0x00000100;
    const sal_uInt32 EmfPlusPenDataAlignment = 0x00000200;
    const sal_uInt32 EmfPlusPenDataCompoundLine = 0x00000400;
    const sal_uInt32 EmfPlusPenDataCustomStartCap = 0x00000800;
    const sal_uInt32 EmfPlusPenDataCustomEndCap = 0x000001000;

    enum LineCapType
    {
        LineCapTypeFlat = 0x00000000,
        LineCapTypeSquare = 0x00000001,
        LineCapTypeRound = 0x00000002,
        LineCapTypeTriangle = 0x00000003,
        LineCapTypeNoAnchor = 0x00000010,
        LineCapTypeSquareAnchor = 0x00000011,
        LineCapTypeRoundAnchor = 0x00000012,
        LineCapTypeDiamondAnchor = 0x00000013,
        LineCapTypeArrowAnchor = 0x00000014,
        LineCapTypeAnchorMask = 0x000000F0,
        LineCapTypeCustom = 0x000000FF
    };

    enum LineJoinType
    {
        LineJoinTypeMiter = 0x00000000,
        LineJoinTypeBevel = 0x00000001,
        LineJoinTypeRound = 0x00000002,
        LineJoinTypeMiterClipped = 0x00000003
    };

    enum DashedLineCapType
    {
        DashedLineCapTypeFlat = 0x00000000,
        DashedLineCapTypeRound = 0x00000002,
        DashedLineCapTypeTriangle = 0x00000003
    };

    enum PenAlignment
    {
        PenAlignmentCenter = 0x00000000,
        PenAlignmentInset = 0x00000001,
        PenAlignmentLeft = 0x00000002,
        PenAlignmentOutset = 0x00000003,
        PenAlignmentRight = 0x00000004
    };

    struct EMFPCustomLineCap;

    struct EMFPPen : public EMFPBrush
    {
        basegfx::B2DHomMatrix pen_transformation;   //TODO: This isn't used
        sal_uInt32 penDataFlags;
        sal_uInt32 penUnit;
        float penWidth;
        sal_Int32 startCap;
        sal_Int32 endCap;
        sal_Int32 lineJoin;
        float miterLimit;
        sal_Int32 dashStyle;
        sal_Int32 dashCap;
        float dashOffset;
        std::vector<float> dashPattern;
        sal_Int32 alignment;
        std::vector<float> compoundArray;
        sal_Int32 customStartCapLen;
        std::unique_ptr<EMFPCustomLineCap> customStartCap;
        sal_Int32 customEndCapLen;
        std::unique_ptr<EMFPCustomLineCap> customEndCap;

        EMFPPen();

        virtual ~EMFPPen() override;

        void Read(SvStream& s, EmfPlusHelperData const & rR);

        static sal_Int8 lcl_convertStrokeCap(sal_uInt32 nEmfStroke);
        static sal_Int8 lcl_convertLineJoinType(sal_uInt32 nEmfLineJoin);
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
