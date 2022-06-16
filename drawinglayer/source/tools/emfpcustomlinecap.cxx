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

#include <sal/log.hxx>
#include "emfpcustomlinecap.hxx"
#include "emfppath.hxx"
#include "emfppen.hxx"
#include <basegfx/matrix/b2dhommatrixtools.hxx>

using namespace ::com::sun::star;
using namespace ::basegfx;

namespace emfplushelper
{
    const sal_uInt32 EmfPlusCustomLineCapDataTypeDefault = 0x00000000;
    const sal_uInt32 EmfPlusCustomLineCapDataTypeAdjustableArrow = 0x00000001;
    const sal_uInt32 EmfPlusCustomLineCapDataFillPath = 0x00000001;
    const sal_uInt32 EmfPlusCustomLineCapDataLinePath = 0x00000002;

    EMFPCustomLineCap::EMFPCustomLineCap()
        : EMFPObject()
        , type(0)
        , strokeStartCap(0)
        , strokeEndCap(0)
        , strokeJoin(0)
        , miterLimit(0.0)
        , widthScale(0.0)
        , mbIsFilled(false)
    {
    }

    void EMFPCustomLineCap::ReadPath(SvStream& s, EmfPlusHelperData const & rR, bool bFill)
    {
        sal_Int32 pathLength;
        s.ReadInt32(pathLength);
        SAL_INFO("drawinglayer.emf", "EMF+\t\tpath length: " << pathLength);
        sal_uInt32 pathHeader;
        sal_Int32 pathPoints, pathFlags;
        s.ReadUInt32(pathHeader).ReadInt32(pathPoints).ReadInt32(pathFlags);
        SAL_INFO("drawinglayer.emf", "EMF+\t\tpath (custom cap line path)");
        SAL_INFO("drawinglayer.emf", "EMF+\t\theader: 0x" << std::hex << pathHeader << " points: " << std::dec << pathPoints << " additional flags: 0x" << std::hex << pathFlags << std::dec);

        EMFPPath path(pathPoints);
        path.Read(s, pathFlags);
        polygon = path.GetPolygon(rR, false);
        // rotate polygon by 180 degrees
        polygon.transform(basegfx::utils::createRotateB2DHomMatrix(M_PI));
        mbIsFilled = bFill;
    }

    void EMFPCustomLineCap::Read(SvStream& s, EmfPlusHelperData const & rR)
    {
        sal_uInt32 header;
        s.ReadUInt32(header).ReadUInt32(type);
        SAL_INFO("drawinglayer.emf", "EMF+\t\tcustom cap");
        SAL_INFO("drawinglayer.emf", "EMF+\t\theader: 0x" << std::hex << header << " type: " << type << std::dec);

        if (type == EmfPlusCustomLineCapDataTypeDefault)
        {
            sal_uInt32 customLineCapDataFlags, baseCap;
            float baseInset;
            float fillHotSpotX, fillHotSpotY, strokeHotSpotX, strokeHotSpotY;

            s.ReadUInt32(customLineCapDataFlags).ReadUInt32(baseCap).ReadFloat(baseInset)
                .ReadUInt32(strokeStartCap).ReadUInt32(strokeEndCap).ReadUInt32(strokeJoin)
                .ReadFloat(miterLimit).ReadFloat(widthScale)
                .ReadFloat(fillHotSpotX).ReadFloat(fillHotSpotY).ReadFloat(strokeHotSpotX).ReadFloat(strokeHotSpotY);

            SAL_INFO("drawinglayer.emf", "EMF+\t\tcustomLineCapDataFlags: 0x" << std::hex << customLineCapDataFlags);
            SAL_INFO("drawinglayer.emf", "EMF+\t\tbaseCap: 0x" << std::hex << baseCap);
            SAL_INFO("drawinglayer.emf", "EMF+\t\tbaseInset: " << baseInset);

            if (customLineCapDataFlags & EmfPlusCustomLineCapDataFillPath)
            {
                ReadPath(s, rR, true);
            }

            if (customLineCapDataFlags & EmfPlusCustomLineCapDataLinePath)
            {
                ReadPath(s, rR, false);
            }
        }
        else if (type == EmfPlusCustomLineCapDataTypeAdjustableArrow)
        {
            // TODO only reads the data, does not use them [I've had
            // no test document to be able to implement it]

            sal_Int32 fillState;
            float width, height, middleInset, unusedHotSpot;

            s.ReadFloat(width).ReadFloat(height).ReadFloat(middleInset).ReadInt32(fillState).ReadUInt32(strokeStartCap)
                .ReadUInt32(strokeEndCap).ReadUInt32(strokeJoin).ReadFloat(miterLimit).ReadFloat(widthScale)
                .ReadFloat(unusedHotSpot).ReadFloat(unusedHotSpot).ReadFloat(unusedHotSpot).ReadFloat(unusedHotSpot);

            SAL_INFO("drawinglayer.emf", "EMF+\t\tTODO - actually read EmfPlusCustomLineCapArrowData object (section 2.2.2.12)");
        }
        SAL_INFO("drawinglayer.emf", "EMF+\t\tstrokeStartCap: 0x" << std::hex << strokeStartCap);
        SAL_INFO("drawinglayer.emf", "EMF+\t\tstrokeEndCap: 0x" << std::hex << strokeEndCap);
        SAL_INFO("drawinglayer.emf", "EMF+\t\tstrokeJoin: 0x" << std::hex << strokeJoin);
        SAL_INFO("drawinglayer.emf", "EMF+\t\tmiterLimit: " << miterLimit);
        SAL_INFO("drawinglayer.emf", "EMF+\t\twidthScale: " << widthScale);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
