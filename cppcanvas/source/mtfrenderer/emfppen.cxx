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
#include <emfppen.hxx>
#include <emfpcustomlinecap.hxx>

using namespace ::com::sun::star;
using namespace ::basegfx;

namespace cppcanvas
{
    namespace internal
    {
        const sal_Int32 EmfPlusLineStyleSolid = 0x00000000;
        const sal_Int32 EmfPlusLineStyleDash = 0x00000001;
        const sal_Int32 EmfPlusLineStyleDot = 0x00000002;
        const sal_Int32 EmfPlusLineStyleDashDot = 0x00000003;
        const sal_Int32 EmfPlusLineStyleDashDotDot = 0x00000004;
        const sal_Int32 EmfPlusLineStyleCustom = 0x00000005;

        EMFPPen::EMFPPen()
            : EMFPBrush()
            , width(0.0)
            , startCap(0)
            , endCap(0)
            , lineJoin(0)
            , mitterLimit(0.0)
            , dashStyle(0)
            , dashCap(0)
            , dashOffset(0.0)
            , dashPatternLen(0)
            , dashPattern(nullptr)
            , alignment(0)
            , compoundArrayLen(0)
            , compoundArray(nullptr)
            , customStartCapLen(0)
            , customStartCap(nullptr)
            , customEndCapLen(0)
            , customEndCap(nullptr)
        {
        }

        EMFPPen::~EMFPPen()
        {
            delete[] dashPattern;
            delete[] compoundArray;
            delete customStartCap;
            delete customEndCap;
        }

        void EMFPPen::SetStrokeWidth(rendering::StrokeAttributes& rStrokeAttributes, ImplRenderer& rR, const OutDevState& rState)
        {
#if OSL_DEBUG_LEVEL > 1
            if (width == 0.0) {
                SAL_INFO("cppcanvas.emf", "TODO: pen with zero width - using minimal which might not be correct");
            }
#endif
            rStrokeAttributes.StrokeWidth = fabs((rState.mapModeTransform * rR.MapSize(width == 0.0 ? 0.05 : width, 0)).getLength());
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


        void EMFPPen::SetStrokeAttributes(rendering::StrokeAttributes& rStrokeAttributes)
        {
            rStrokeAttributes.JoinType = lcl_convertLineJoinType(lineJoin);

            if (dashStyle != EmfPlusLineStyleSolid)
            {
                const float dash[] = { 3, 3 };
                const float dot[] = { 1, 3 };
                const float dashdot[] = { 3, 3, 1, 3 };
                const float dashdotdot[] = { 3, 3, 1, 3, 1, 3 };

                sal_Int32 nLen = 0;
                const float *pPattern = nullptr;
                switch (dashStyle)
                {
                case EmfPlusLineStyleDash:       nLen = SAL_N_ELEMENTS(dash); pPattern = dash; break;
                case EmfPlusLineStyleDot:        nLen = SAL_N_ELEMENTS(dot); pPattern = dot; break;
                case EmfPlusLineStyleDashDot:    nLen = SAL_N_ELEMENTS(dashdot); pPattern = dashdot; break;
                case EmfPlusLineStyleDashDotDot: nLen = SAL_N_ELEMENTS(dashdotdot); pPattern = dashdotdot; break;
                case EmfPlusLineStyleCustom:     nLen = dashPatternLen; pPattern = dashPattern; break;
                }
                if (nLen > 0)
                {
                    uno::Sequence<double> aDashArray(nLen);
                    for (int i = 0; i < nLen; ++i)
                        aDashArray[i] = pPattern[i];

                    rStrokeAttributes.DashArray = aDashArray;
                }
            }
        }

        void EMFPPen::Read(SvStream& s, ImplRenderer& rR)
        {
            sal_uInt32 header, unknown, penFlags, unknown2;
            int i;

            s.ReadUInt32(header).ReadUInt32(unknown).ReadUInt32(penFlags).ReadUInt32(unknown2).ReadFloat(width);

            SAL_INFO("cppcanvas.emf", "EMF+\tpen");
            SAL_INFO("cppcanvas.emf", "EMF+\theader: 0x" << std::hex << header << " unknown: 0x" << unknown <<
                " additional flags: 0x" << penFlags << " unknown: 0x" << unknown2 << " width: " << std::dec << width);

            if (penFlags & 1)
                ReadXForm(s, pen_transformation);

            if (penFlags & 2)
            {
                s.ReadInt32(startCap);
                SAL_INFO("cppcanvas.emf", "EMF+\t\tstartCap: 0x" << std::hex << startCap);
            }
            else
                startCap = 0;

            if (penFlags & 4)
            {
                s.ReadInt32(endCap);
                SAL_INFO("cppcanvas.emf", "EMF+\t\tendCap: 0x" << std::hex << endCap);
            }
            else
                endCap = 0;

            if (penFlags & 8)
                s.ReadInt32(lineJoin);
            else
                lineJoin = 0;

            if (penFlags & 16)
                s.ReadFloat(mitterLimit);
            else
                mitterLimit = 0;

            if (penFlags & 32)
            {
                s.ReadInt32(dashStyle);
                SAL_INFO("cppcanvas.emf", "EMF+\t\tdashStyle: 0x" << std::hex << dashStyle);
            }
            else
                dashStyle = 0;

            if (penFlags & 64)
                s.ReadInt32(dashCap);
            else
                dashCap = 0;

            if (penFlags & 128)
                s.ReadFloat(dashOffset);
            else
                dashOffset = 0;

            if (penFlags & 256)
            {
                dashStyle = EmfPlusLineStyleCustom;

                s.ReadInt32(dashPatternLen);
                SAL_INFO("cppcanvas.emf", "EMF+\t\tdashPatternLen: " << dashPatternLen);

                if (dashPatternLen<0 || sal_uInt32(dashPatternLen)>SAL_MAX_INT32 / sizeof(float))
                    dashPatternLen = SAL_MAX_INT32 / sizeof(float);
                dashPattern = new float[dashPatternLen];
                for (i = 0; i < dashPatternLen; i++)
                {
                    s.ReadFloat(dashPattern[i]);
                    SAL_INFO("cppcanvas.emf", "EMF+\t\t\tdashPattern[" << i << "]: " << dashPattern[i]);
                }
            }
            else
                dashPatternLen = 0;

            if (penFlags & 512)
                s.ReadInt32(alignment);
            else
                alignment = 0;

            if (penFlags & 1024) {
                s.ReadInt32(compoundArrayLen);
                if (compoundArrayLen<0 || sal_uInt32(compoundArrayLen)>SAL_MAX_INT32 / sizeof(float))
                    compoundArrayLen = SAL_MAX_INT32 / sizeof(float);
                compoundArray = new float[compoundArrayLen];
                for (i = 0; i < compoundArrayLen; i++)
                    s.ReadFloat(compoundArray[i]);
            }
            else
                compoundArrayLen = 0;

            if (penFlags & 2048)
            {
                s.ReadInt32(customStartCapLen);
                SAL_INFO("cppcanvas.emf", "EMF+\t\tcustomStartCapLen: " << customStartCapLen);
                sal_uInt64 const pos = s.Tell();

                customStartCap = new EMFPCustomLineCap();
                customStartCap->Read(s, rR);

                // maybe we don't read everything yet, play it safe ;-)
                s.Seek(pos + customStartCapLen);
            }
            else
                customStartCapLen = 0;

            if (penFlags & 4096)
            {
                s.ReadInt32(customEndCapLen);
                SAL_INFO("cppcanvas.emf", "EMF+\t\tcustomEndCapLen: " << customEndCapLen);
                sal_uInt64 const pos = s.Tell();

                customEndCap = new EMFPCustomLineCap();
                customEndCap->Read(s, rR);

                // maybe we don't read everything yet, play it safe ;-)
                s.Seek(pos + customEndCapLen);
            }
            else
                customEndCapLen = 0;

            EMFPBrush::Read(s, rR);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
