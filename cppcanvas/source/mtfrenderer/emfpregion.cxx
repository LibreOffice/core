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

#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/tools/gradienttools.hxx>
#include <basegfx/tools/tools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <vcl/canvastools.hxx>
#include <implrenderer.hxx>
#include <emfpregion.hxx>
#include <emfppath.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>

using namespace ::com::sun::star;
using namespace ::basegfx;

enum EmfPlusCombineMode
{
    EmfPlusCombineModeReplace = 0x00000000,
    EmfPlusCombineModeIntersect = 0x00000001,
    EmfPlusCombineModeUnion = 0x00000002,
    EmfPlusCombineModeXOR = 0x00000003,
    EmfPlusCombineModeExclude = 0x00000004,
    EmfPlusCombineModeComplement = 0x00000005
};

namespace cppcanvas
{
    namespace internal
    {
        EMFPRegion::EMFPRegion()
        {
        }

        EMFPRegion::~EMFPRegion()
        {
        }

        ::basegfx::B2DPolyPolygon combineClip(::basegfx::B2DPolyPolygon const & leftPolygon, int combineMode, ::basegfx::B2DPolyPolygon const & rightPolygon)
        {
            basegfx::B2DPolyPolygon aClippedPolyPolygon;
            switch (combineMode)
            {
            case EmfPlusCombineModeReplace:
            {
                aClippedPolyPolygon = rightPolygon;
                break;
            }
            case EmfPlusCombineModeIntersect:
            {
                if (leftPolygon.count())
                {
                    aClippedPolyPolygon = basegfx::tools::clipPolyPolygonOnPolyPolygon(
                                leftPolygon,
                                rightPolygon,
                                true,
                                false);
                }
                break;
            }
            case EmfPlusCombineModeUnion:
            {
                aClippedPolyPolygon = ::basegfx::tools::solvePolygonOperationOr(leftPolygon, rightPolygon);
                break;
            }
            case EmfPlusCombineModeXOR:
            {
                aClippedPolyPolygon = ::basegfx::tools::solvePolygonOperationXor(leftPolygon, rightPolygon);
                break;
            }
            case EmfPlusCombineModeExclude:
            {
                // Replaces the existing region with the part of itself that is not in the new region.
                aClippedPolyPolygon = ::basegfx::tools::solvePolygonOperationDiff(leftPolygon, rightPolygon);
                break;
            }
            case EmfPlusCombineModeComplement:
            {
                // Replaces the existing region with the part of the new region that is not in the existing region.
                aClippedPolyPolygon = ::basegfx::tools::solvePolygonOperationDiff(rightPolygon, leftPolygon);
                break;
            }
            }
            return aClippedPolyPolygon;
        }

        ::basegfx::B2DPolyPolygon EMFPRegion::ReadRegionNode(SvStream& s, ImplRenderer& rR)
        {
            // Regions are specified as a binary tree of region nodes, and each node must either be a terminal node
            // (RegionNodeDataTypeRect, RegionNodeDataTypePath, RegionNodeDataTypeEmpty, RegionNodeDataTypeInfinite)
            // or specify one or two child nodes
            // (RegionNodeDataTypeAnd, RegionNodeDataTypeOr, RegionNodeDataTypeXor,
            // RegionNodeDataTypeExclude, RegionNodeDataTypeComplement).
            sal_uInt32 dataType;
            ::basegfx::B2DPolyPolygon polygon;
            s.ReadUInt32(dataType);
            SAL_INFO("cppcanvas.emf", "EMF+\t Region node data type 0x" << std::hex << dataType << std::dec);

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
                polygon = combineClip(leftPolygon, dataType, rightPolygon);
                break;
            }
            case RegionNodeDataTypeRect:
            {
                float dx, dy, dw, dh;
                s.ReadFloat(dx).ReadFloat(dy).ReadFloat(dw).ReadFloat(dh);
                SAL_INFO("cppcanvas.emf", "EMF+\t\t RegionNodeDataTypeRect x:" << dx << ", y:" << dy <<
                         ", width:" << dw << ", height:" << dh);

                const ::basegfx::B2DPoint mappedStartPoint(rR.Map(dx, dy));
                const ::basegfx::B2DPoint mappedEndPoint(rR.Map(dx + dw, dy + dh));
                const ::basegfx::B2DPolyPolygon polyPolygon(
                        ::basegfx::tools::createPolygonFromRect(
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
                SAL_INFO("cppcanvas.emf", "EMF+\t\t RegionNodeDataTypePath, Path Length: " << pathLength << " bytes");

                sal_uInt32 header, pathFlags;
                sal_Int32 points;

                s.ReadUInt32(header).ReadInt32(points).ReadUInt32(pathFlags);
                SAL_INFO("cppcanvas.emf", "EMF+\t\t header: 0x" << std::hex << header <<
                         " points: " << std::dec << points << " additional flags: 0x" << std::hex << pathFlags << std::dec);

                EMFPPath path(points);
                path.Read(s, pathFlags, rR);
                polygon = path.GetPolygon(rR);
                break;
            }
            case RegionNodeDataTypeEmpty:
            {
                SAL_INFO("cppcanvas.emf", "EMF+\t\t RegionNodeDataTypeEmpty");
                SAL_WARN("cppcanvas.emf", "EMF+\t\t TODO we need to set empty polygon here");
                polygon = ::basegfx::B2DPolyPolygon();

                break;
            }
            case RegionNodeDataTypeInfinite:
            {
                SAL_INFO("cppcanvas.emf", "EMF+\t\t RegionNodeDataTypeInfinite");
                polygon = ::basegfx::B2DPolyPolygon();
                break;
            }
            default:
            {
                SAL_WARN("cppcanvas.emf", "EMF+\t\t Unhandled region type: 0x" << std::hex << dataType << std::dec);
                polygon = ::basegfx::B2DPolyPolygon();
            }
            }
            return polygon;
        }

        void EMFPRegion::ReadRegion(SvStream& s, ImplRenderer& rR)
        {
            sal_uInt32 header, count;
            s.ReadUInt32(header).ReadUInt32(count);
            // An array should be RegionNodeCount+1 of EmfPlusRegionNode objects.
            SAL_INFO("cppcanvas.emf", "EMF+\t version: 0x" << std::hex << header << std::dec << ", region node count: " << count);

            regionPolyPolygon = ReadRegionNode(s, rR);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
