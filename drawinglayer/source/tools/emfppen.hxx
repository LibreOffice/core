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

#ifndef INCLUDED_DRAWINGLAYER_SOURCE_TOOLS_EMFPPEN_HXX
#define INCLUDED_DRAWINGLAYER_SOURCE_TOOLS_EMFPPEN_HXX

#include <emfpbrush.hxx>
#include <com/sun/star/rendering/StrokeAttributes.hpp>

namespace emfplushelper
{
    const sal_uInt32 EmfPlusLineCapTypeSquare = 0x00000001;
    const sal_uInt32 EmfPlusLineCapTypeRound = 0x00000002;

    const sal_uInt32 EmfPlusLineJoinTypeMiter = 0x00000000;
    const sal_uInt32 EmfPlusLineJoinTypeBevel = 0x00000001;
    const sal_uInt32 EmfPlusLineJoinTypeRound = 0x00000002;
    const sal_uInt32 EmfPlusLineJoinTypeMiterClipped = 0x00000003;

    struct EMFPCustomLineCap;

    struct EMFPPen : public EMFPBrush
    {
        basegfx::B2DHomMatrix pen_transformation;   //TODO: This isn't used
        float penWidth;
        sal_Int32 startCap;
        sal_Int32 endCap;
        sal_Int32 lineJoin;
        float mitterLimit;
        sal_Int32 dashStyle;
        sal_Int32 dashCap;
        float dashOffset;
        sal_Int32 dashPatternLen;
        float *dashPattern;
        sal_Int32 alignment;
        sal_Int32 compoundArrayLen;
        float *compoundArray;
        sal_Int32 customStartCapLen;
        EMFPCustomLineCap *customStartCap;
        sal_Int32 customEndCapLen;
        EMFPCustomLineCap *customEndCap;

        EMFPPen();

        virtual ~EMFPPen() override;

        void SetStrokeWidth(com::sun::star::rendering::StrokeAttributes& rStrokeAttributes, EmfPlusHelperData& rR, const ::basegfx::B2DHomMatrix& mapModeTransform);

        void SetStrokeAttributes(com::sun::star::rendering::StrokeAttributes& rStrokeAttributes);

        void Read(SvStream& s, EmfPlusHelperData& rR);

        static sal_Int8 lcl_convertStrokeCap(sal_uInt32 nEmfStroke);
        static sal_Int8 lcl_convertLineJoinType(sal_uInt32 nEmfLineJoin);

    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
