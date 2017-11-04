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
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/utils/gradienttools.hxx>
#include <basegfx/utils/tools.hxx>
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
#include "emfpregion.hxx"
#include "emfppath.hxx"

using namespace ::com::sun::star;
using namespace ::basegfx;

namespace emfplushelper
{
    EMFPRegion::EMFPRegion()
        : count(0)
        , regionNodeDataType(nullptr)
        , initialState(0)
        , ix(0.0)
        , iy(0.0)
        , iw(0.0)
        , ih(0.0)
    {
    }

    EMFPRegion::~EMFPRegion()
    {
        if (regionNodeDataType)
        {
            delete[] regionNodeDataType;
            regionNodeDataType = nullptr;
        }
    }

    void EMFPRegion::ReadRegionNode(SvStream& s)
    {
        sal_uInt32 dataType;
        s.ReadUInt32(dataType);
        SAL_INFO("drawinglayer", "EMF+\t Region node data type 0x" << std::hex << dataType << std::dec);

        switch (dataType)
        {
        case RegionNodeDataTypeAnd:
        {
            ReadRegionNode(s);
            ReadRegionNode(s);
            break;
        }
        case RegionNodeDataTypeOr:
        {
            ReadRegionNode(s);
            ReadRegionNode(s);
            break;
        }
        case RegionNodeDataTypeXor:
        {
            ReadRegionNode(s);
            ReadRegionNode(s);
            break;
        }
        case RegionNodeDataTypeExclude:
        {
            ReadRegionNode(s);
            ReadRegionNode(s);
            break;
        }
        case RegionNodeDataTypeComplement:
        {
            ReadRegionNode(s);
            ReadRegionNode(s);
            break;
        }
        case RegionNodeDataTypeRect:
        {
            s.ReadFloat(ix).ReadFloat(iy).ReadFloat(iw).ReadFloat(ih);
            SAL_INFO("drawinglayer", "EMF+\t\t RegionNodeDataTypeRect x:" << ix << ", y:" << iy << ", width:" << iw << ", height:" << ih);
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
            SAL_INFO("drawinglayer", "EMF+\t\t header: 0x" << std::hex << header << " points: " << std::dec << points << " additional flags: 0x" << std::hex << pathFlags << std::dec);

            EMFPPath path(points);
            path.Read(s, pathFlags);
            break;
        }
        case RegionNodeDataTypeEmpty:
        {
            SAL_INFO("drawinglayer", "EMF+\t\t RegionNodeDataTypeEmpty");

            break;
        }
        case RegionNodeDataTypeInfinite:
        {
            SAL_INFO("drawinglayer", "EMF+\t\t RegionNodeDataTypeInfinite");
            break;
        }
        default:
        {
            SAL_WARN("drawinglayer", "EMF+\t\t Unhandled region type: 0x" << std::hex << dataType << std::dec);
        }
        }
    }

    void EMFPRegion::ReadRegion(SvStream& s)
    {
        sal_uInt32 header;
        s.ReadUInt32(header).ReadUInt32(count);
        // An array should be RegionNodeCount+1 of EmfPlusRegionNode objects.
        SAL_WARN("drawinglayer", "EMF+\t EmfPlusRegion TODO");
        SAL_INFO("drawinglayer", "EMF+\t version: 0x" << std::hex << header << std::dec << ", region node count: " << count);

        regionNodeDataType = new sal_uInt32[count];

        ReadRegionNode(s);
        s.ReadUInt32(initialState);
        SAL_INFO("drawinglayer", "EMF+\tinitial state: 0x" << std::hex << initialState << std::dec);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
