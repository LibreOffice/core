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

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <sal/log.hxx>
#include "emfpregion.hxx"
#include "emfppath.hxx"

namespace emfplushelper
{
    EMFPRegion::EMFPRegion()
    {
    }

    EMFPRegion::~EMFPRegion()
    {
    }

    ::basegfx::B2DPolyPolygon EMFPRegion::ReadRegionNode(SvStream& s, EmfPlusHelperData& rR)
    {
        // Regions are specified as a binary tree of region nodes, and each node must either be a terminal node
        // (RegionNodeDataTypeRect, RegionNodeDataTypePath, RegionNodeDataTypeEmpty, RegionNodeDataTypeInfinite)
        // or specify one or two child nodes
        // (RegionNodeDataTypeAnd, RegionNodeDataTypeOr, RegionNodeDataTypeXor,
        // RegionNodeDataTypeExclude, RegionNodeDataTypeComplement).
        sal_uInt32 dataType;
        ::basegfx::B2DPolyPolygon polygon;
        s.ReadUInt32(dataType);
        SAL_INFO("drawinglayer", "EMF+\t Region node data type 0x" << std::hex << dataType << std::dec);

        switch (dataType)
        {
        case RegionNodeDataTypeAnd: // CombineModeIntersect
        case RegionNodeDataTypeOr: // CombineModeUnion
        case RegionNodeDataTypeXor: // CombineModeXOR
        case RegionNodeDataTypeExclude: // CombineModeExclude
        case RegionNodeDataTypeComplement: // CombineModeComplement
        {
            ::basegfx::B2DPolyPolygon leftPolygon = ReadRegionNode(s, rR);
            ::basegfx::B2DPolyPolygon rightPolygon = ReadRegionNode(s, rR);
            polygon = EmfPlusHelperData::combineClip(leftPolygon, dataType, rightPolygon);
            break;
        }
        case RegionNodeDataTypeRect:
        {
            float dx, dy, dw, dh;
            s.ReadFloat(dx).ReadFloat(dy).ReadFloat(dw).ReadFloat(dh);
            SAL_INFO("drawinglayer", "EMF+\t\t RegionNodeDataTypeRect x:" << dx << ", y:" << dy <<
                     ", width:" << dw << ", height:" << dh);

            const ::basegfx::B2DPoint mappedStartPoint(rR.Map(dx, dy));
            const ::basegfx::B2DPoint mappedEndPoint(rR.Map(dx + dw, dy + dh));
            const ::basegfx::B2DPolyPolygon polyPolygon(
                    ::basegfx::utils::createPolygonFromRect(
                        ::basegfx::B2DRectangle(
                            mappedStartPoint.getX(),
                            mappedStartPoint.getY(),
                            mappedEndPoint.getX(),
                            mappedEndPoint.getY())));
            polygon = polyPolygon;
            break;
        }
        case RegionNodeDataTypePath:
        {
            sal_Int32 pathLength;
            s.ReadInt32(pathLength);
            SAL_INFO("drawinglayer", "EMF+\t\t RegionNodeDataTypePath, Path Length: " << pathLength << " bytes");

            sal_uInt32 header, pathFlags;
            sal_Int32 points;

            s.ReadUInt32(header).ReadInt32(points).ReadUInt32(pathFlags);
            SAL_INFO("drawinglayer", "EMF+\t\t header: 0x" << std::hex << header <<
                     " points: " << std::dec << points << " additional flags: 0x" << std::hex << pathFlags << std::dec);

            EMFPPath path(points);
            path.Read(s, pathFlags);
            polygon = path.GetPolygon(rR);
            break;
        }
        case RegionNodeDataTypeEmpty:
        {
            SAL_INFO("drawinglayer", "EMF+\t\t RegionNodeDataTypeEmpty");
            SAL_WARN("drawinglayer", "EMF+\t\t TODO we need to set empty polygon here");
            polygon = ::basegfx::B2DPolyPolygon();

            break;
        }
        case RegionNodeDataTypeInfinite:
        {
            SAL_INFO("drawinglayer", "EMF+\t\t RegionNodeDataTypeInfinite");
            polygon = ::basegfx::B2DPolyPolygon();
            break;
        }
        default:
        {
            SAL_WARN("drawinglayer", "EMF+\t\t Unhandled region type: 0x" << std::hex << dataType << std::dec);
            polygon = ::basegfx::B2DPolyPolygon();
        }
        }
        return polygon;
    }

    void EMFPRegion::ReadRegion(SvStream& s, EmfPlusHelperData& rR)
    {
        sal_uInt32 header, count;
        s.ReadUInt32(header).ReadUInt32(count);
        // An array should be RegionNodeCount+1 of EmfPlusRegionNode objects.
        SAL_INFO("drawinglayer", "EMF+\t version: 0x" << std::hex << header << std::dec << ", region node count: " << count);

        regionPolyPolygon = ReadRegionNode(s, rR);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
