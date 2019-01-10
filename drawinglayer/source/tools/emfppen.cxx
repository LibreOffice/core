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
#include <sal/log.hxx>
#include "emfppen.hxx"
#include "emfpcustomlinecap.hxx"

using namespace ::com::sun::star;
using namespace ::basegfx;

namespace emfplushelper
{
    enum EmfPlusPenData
    {
        PenDataTransform        = 0x00000001,
        PenDataStartCap         = 0x00000002,
        PenDataEndCap           = 0x00000004,
        PenDataJoin             = 0x00000008,
        PenDataMiterLimit       = 0x00000010,
        PenDataLineStyle        = 0x00000020,
        PenDataDashedLineCap    = 0x00000040,
        PenDataDashedLineOffset = 0x00000080,
        PenDataDashedLine       = 0x00000100,
        PenDataNonCenter        = 0x00000200,
        PenDataCompoundLine     = 0x00000400,
        PenDataCustomStartCap   = 0x00000800,
        PenDataCustomEndCap     = 0x00001000
    };

    EMFPPen::EMFPPen()
        : EMFPBrush()
        , penDataFlags(0)
        , penUnit(0)
        , penWidth(0.0)
        , startCap(0)
        , endCap(0)
        , lineJoin(0)
        , mitterLimit(0.0)
        , dashStyle(0)
        , dashCap(0)
        , dashOffset(0.0)
        , alignment(0)
        , customStartCapLen(0)
        , customEndCapLen(0)
    {
    }

    EMFPPen::~EMFPPen()
    {
    }

    /// Convert stroke caps between EMF+ and rendering API
    sal_Int8 EMFPPen::lcl_convertStrokeCap(sal_uInt32 nEmfStroke)
    {
        switch (nEmfStroke)
        {
            case EmfPlusLineCapTypeSquare: return rendering::PathCapType::SQUARE;
            case EmfPlusLineCapTypeRound:  return rendering::PathCapType::ROUND;
        }

        // we have no mapping for EmfPlusLineCapTypeTriangle = 0x00000003,
        // so return BUTT always
        return rendering::PathCapType::BUTT;
    }

    sal_Int8 EMFPPen::lcl_convertLineJoinType(sal_uInt32 nEmfLineJoin)
    {
        switch (nEmfLineJoin)
        {
            case EmfPlusLineJoinTypeMiter:        // fall-through
            case EmfPlusLineJoinTypeMiterClipped: return rendering::PathJoinType::MITER;
            case EmfPlusLineJoinTypeBevel:        return rendering::PathJoinType::BEVEL;
            case EmfPlusLineJoinTypeRound:        return rendering::PathJoinType::ROUND;
        }

        assert(false); // Line Join type isn't in specification.
        return 0;
    }

    void EMFPPen::Read(SvStream& s, EmfPlusHelperData const & rR)
    {
        sal_uInt32 graphicsVersion, penType;
        int i;
        s.ReadUInt32(graphicsVersion).ReadUInt32(penType).ReadUInt32(penDataFlags).ReadUInt32(penUnit).ReadFloat(penWidth);
        SAL_INFO("drawinglayer", "EMF+\tpen");
        SAL_INFO("drawinglayer", "EMF+\t graphics version: 0x" << std::hex << graphicsVersion << " type (must be set to zero): " << penType <<
            " pen data flags: 0x" << penDataFlags << " unit: " << penUnit << " width: " << std::dec << penWidth);

        penWidth = penWidth * EmfPlusHelperData::getUnitToPixelMultiplier(static_cast<UnitType>(penUnit));

        // If a zero width is specified, a minimum value must be used, which is determined by the units
        if (penWidth == 0.0)
        { //TODO Check if these values is correct
            penWidth = penUnit == 0 ? 0.18f
                : 0.05f;  // 0.05f is taken from old EMF+ implementation (case of Unit == Pixel etc.)
        }

        if (penDataFlags & PenDataTransform)
        {
            EmfPlusHelperData::readXForm(s, pen_transformation);
            SAL_WARN("drawinglayer", "EMF+\t\t TODO PenDataTransform: " << pen_transformation);
        }

        if (penDataFlags & PenDataStartCap)
        {
            s.ReadInt32(startCap);
            SAL_INFO("drawinglayer", "EMF+\t\tstartCap: 0x" << std::hex << startCap);
        }
        else
        {
            startCap = 0;
        }

        if (penDataFlags & PenDataEndCap)
        {
            s.ReadInt32(endCap);
            SAL_INFO("drawinglayer", "EMF+\t\tendCap: 0x" << std::hex << endCap);
        }
        else
        {
            endCap = 0;
        }

        if (penDataFlags & PenDataJoin)
        {
            s.ReadInt32(lineJoin);
            SAL_WARN("drawinglayer", "EMF+\t\t TODO PenDataJoin: 0x" << std::hex << lineJoin);
        }
        else
        {
            lineJoin = 0;
        }

        if (penDataFlags & PenDataMiterLimit)
        {
            s.ReadFloat(mitterLimit);
            SAL_WARN("drawinglayer", "EMF+\t\t TODO PenDataMiterLimit: 0x" << std::hex << mitterLimit);
        }
        else
        {
            mitterLimit = 0;
        }

        if (penDataFlags & PenDataLineStyle)
        {
            s.ReadInt32(dashStyle);
            SAL_INFO("drawinglayer", "EMF+\t\tdashStyle: 0x" << std::hex << dashStyle);
        }
        else
        {
            dashStyle = 0;
        }

        if (penDataFlags & PenDataDashedLineCap)
        {
            s.ReadInt32(dashCap);
            SAL_WARN("drawinglayer", "EMF+\t\t TODO PenDataDashedLineCap: 0x" << std::hex << dashCap);
        }
        else
        {
            dashCap = 0;
        }

        if (penDataFlags & PenDataDashedLineOffset)
        {
            s.ReadFloat(dashOffset);
            SAL_WARN("drawinglayer", "EMF+\t\t TODO PenDataDashedLineOffset: 0x" << std::hex << dashOffset);
        }
        else
        {
            dashOffset = 0;
        }

        if (penDataFlags & PenDataDashedLine)
        {
            dashStyle = EmfPlusLineStyleCustom;
            sal_Int32 dashPatternLen;

            s.ReadInt32(dashPatternLen);
            SAL_INFO("drawinglayer", "EMF+\t\tdashPatternLen: " << dashPatternLen);

            if (dashPatternLen<0 || sal_uInt32(dashPatternLen)>SAL_MAX_INT32 / sizeof(float))
            {
                dashPatternLen = SAL_MAX_INT32 / sizeof(float);
            }

            dashPattern.resize( dashPatternLen );

            for (i = 0; i < dashPatternLen; i++)
            {
                s.ReadFloat(dashPattern[i]);
                SAL_INFO("drawinglayer", "EMF+\t\t\tdashPattern[" << i << "]: " << dashPattern[i]);
            }
        }

        if (penDataFlags & PenDataNonCenter)
        {
            s.ReadInt32(alignment);
            SAL_WARN("drawinglayer", "EMF+\t\t TODO PenDataNonCenter: 0x" << std::hex << alignment);
        }
        else
        {
            alignment = 0;
        }

        if (penDataFlags & PenDataCompoundLine)
        {
            sal_Int32 compoundArrayLen;
            s.ReadInt32(compoundArrayLen);

            if (compoundArrayLen<0 || sal_uInt32(compoundArrayLen)>SAL_MAX_INT32 / sizeof(float))
            {
                compoundArrayLen = SAL_MAX_INT32 / sizeof(float);
            }

            compoundArray.resize(compoundArrayLen);

            for (i = 0; i < compoundArrayLen; i++)
            {
                s.ReadFloat(compoundArray[i]);
            }
            SAL_WARN("drawinglayer", "EMF+\t\t TODO PenDataCompoundLine: 0x");
        }

        if (penDataFlags & PenDataCustomStartCap)
        {
            s.ReadInt32(customStartCapLen);
            SAL_INFO("drawinglayer", "EMF+\t\tcustomStartCapLen: " << customStartCapLen);
            sal_uInt64 const pos = s.Tell();

            customStartCap.reset( new EMFPCustomLineCap() );
            customStartCap->Read(s, rR);

            // maybe we don't read everything yet, play it safe ;-)
            s.Seek(pos + customStartCapLen);
        }
        else
        {
            customStartCapLen = 0;
        }

        if (penDataFlags & PenDataCustomEndCap)
        {
            s.ReadInt32(customEndCapLen);
            SAL_INFO("drawinglayer", "EMF+\t\tcustomEndCapLen: " << customEndCapLen);
            sal_uInt64 const pos = s.Tell();

            customEndCap.reset( new EMFPCustomLineCap() );
            customEndCap->Read(s, rR);

            // maybe we don't read everything yet, play it safe ;-)
            s.Seek(pos + customEndCapLen);
        }
        else
        {
            customEndCapLen = 0;
        }

        EMFPBrush::Read(s, rR);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
