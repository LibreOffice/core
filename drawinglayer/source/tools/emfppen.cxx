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
#include <o3tl/safeint.hxx>
#include <sal/log.hxx>
#include <rtl/ustrbuf.hxx>

#include "emfppen.hxx"
#include "emfpcustomlinecap.hxx"

using namespace ::com::sun::star;
using namespace ::basegfx;

namespace emfplushelper
{

    EMFPPen::EMFPPen()
        : penDataFlags(0)
        , penUnit(0)
        , penWidth(0.0)
        , startCap(0)
        , endCap(0)
        , maLineJoin(basegfx::B2DLineJoin::Miter)
        , fMiterMinimumAngle(basegfx::deg2rad(5.0))
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

    static OUString PenDataFlagsToString(sal_uInt32 flags)
    {
        rtl::OUStringBuffer sFlags;

        if (flags & EmfPlusPenDataTransform)
            sFlags.append("\nEMF+\t\t\tEmfPlusPenDataTransform");

        if (flags & EmfPlusPenDataStartCap)
            sFlags.append("\nEMF+\t\t\tEmfPlusPenDataStartCap");

        if (flags & EmfPlusPenDataEndCap)
             sFlags.append("\nEMF+\t\t\tEmfPlusPenDataEndCap");

        if (flags & EmfPlusPenDataJoin)
            sFlags.append("\nEMF+\t\t\tEmfPlusPenDataJoin");

        if (flags & EmfPlusPenDataMiterLimit)
            sFlags.append("\nEMF+\t\t\tEmfPlusPenDataMiterLimit");

        if (flags & EmfPlusPenDataLineStyle)
            sFlags.append("\nEMF+\t\t\tEmfPlusPenDataLineStyle");

        if (flags & EmfPlusPenDataDashedLineCap)
            sFlags.append("\nEMF+\t\t\tEmfPlusPenDataDashedLineCap");

        if (flags & EmfPlusPenDataDashedLineOffset)
            sFlags.append("\nEMF+\t\t\tEmfPlusPenDataDashedLineOffset");

        if (flags & EmfPlusPenDataDashedLine)
            sFlags.append("\nEMF+\t\t\tEmfPlusPenDataDashedLine");

        if (flags & EmfPlusPenDataAlignment)
            sFlags.append("\nEMF+\t\t\tEmfPlusPenDataAlignment");

        if (flags & EmfPlusPenDataCompoundLine)
            sFlags.append("\nEMF+\t\t\tEmfPlusPenDataCompoundLine");

        if (flags & EmfPlusPenDataCustomStartCap)
            sFlags.append("\nEMF+\t\t\tEmfPlusPenDataCustomStartCap");

        if (flags & EmfPlusPenDataCustomEndCap)
            sFlags.append("\nEMF+\t\t\tEmfPlusPenDataCustomEndCap");

        return sFlags.makeStringAndClear();
    }

    static OUString LineCapTypeToString(sal_uInt32 linecap)
    {
        switch (linecap)
        {
            case LineCapTypeFlat: return u"LineCapTypeFlat"_ustr;
            case LineCapTypeSquare: return u"LineCapTypeSquare"_ustr;
            case LineCapTypeRound: return u"LineCapTypeRound"_ustr;
            case LineCapTypeTriangle: return u"LineCapTypeTriangle"_ustr;
            case LineCapTypeNoAnchor: return u"LineCapTypeNoAnchor"_ustr;
            case LineCapTypeSquareAnchor: return u"LineCapTypeSquareAnchor"_ustr;
            case LineCapTypeRoundAnchor: return u"LineCapTypeRoundAchor"_ustr;
            case LineCapTypeDiamondAnchor: return u"LineCapTypeDiamondAnchor"_ustr;
            case LineCapTypeArrowAnchor: return u"LineCapTypeArrowAnchor"_ustr;
            case LineCapTypeAnchorMask: return u"LineCapTypeAnchorMask"_ustr;
            case LineCapTypeCustom: return u"LineCapTypeCustom"_ustr;
        }
        return u""_ustr;
    }

    static OUString DashedLineCapTypeToString(sal_uInt32 dashedlinecaptype)
    {
        switch (dashedlinecaptype)
        {
            case DashedLineCapTypeFlat: return u"DashedLineCapTypeFlat"_ustr;
            case DashedLineCapTypeRound: return u"DashedLineCapTypeRound"_ustr;
            case DashedLineCapTypeTriangle: return u"DashedLineCapTypeTriangle"_ustr;
        }
        return u""_ustr;
    }

    static OUString PenAlignmentToString(sal_uInt32 alignment)
    {
        switch (alignment)
        {
            case PenAlignmentCenter: return u"PenAlignmentCenter"_ustr;
            case PenAlignmentInset: return u"PenAlignmentInset"_ustr;
            case PenAlignmentLeft: return u"PenAlignmentLeft"_ustr;
            case PenAlignmentOutset: return u"PenAlignmentOutset"_ustr;
            case PenAlignmentRight: return u"PenAlignmentRight"_ustr;
        }
        return u""_ustr;
    }

    drawinglayer::attribute::StrokeAttribute
    EMFPPen::GetStrokeAttribute(const double aTransformation) const
    {
        if (penDataFlags & EmfPlusPenDataLineStyle // pen has a predefined line style
            && dashStyle != EmfPlusLineStyleCustom)
        {
            const double pw = aTransformation * penWidth;
            switch (dashStyle)
            {
                case EmfPlusLineStyleDash:
                    // [-loplugin:redundantfcast] false positive:
                    return drawinglayer::attribute::StrokeAttribute({ 3 * pw, pw });
                case EmfPlusLineStyleDot:
                    // [-loplugin:redundantfcast] false positive:
                    return drawinglayer::attribute::StrokeAttribute({ pw, pw });
                case EmfPlusLineStyleDashDot:
                    // [-loplugin:redundantfcast] false positive:
                    return drawinglayer::attribute::StrokeAttribute({ 3 * pw, pw, pw, pw });
                case EmfPlusLineStyleDashDotDot:
                    // [-loplugin:redundantfcast] false positive:
                    return drawinglayer::attribute::StrokeAttribute({ 3 * pw, pw, pw, pw, pw, pw });
            }
        }
        else if (penDataFlags & EmfPlusPenDataDashedLine) // pen has a custom dash line
        {
            const double pw = aTransformation * penWidth;
            // StrokeAttribute needs a double vector while the pen provides a float vector
            std::vector<double> aPattern(dashPattern.size());
            for (size_t i = 0; i < aPattern.size(); i++)
            {
                // convert from float to double and multiply with the adjusted pen width
                aPattern[i] = pw * dashPattern[i];
            }
            return drawinglayer::attribute::StrokeAttribute(std::move(aPattern));
        }
        //  EmfPlusLineStyleSolid: - do nothing special, use default stroke attribute
        return drawinglayer::attribute::StrokeAttribute();
    }

    void EMFPPen::Read(SvStream& s, EmfPlusHelperData const & rR)
    {
        sal_Int32 lineJoin = EmfPlusLineJoinTypeMiter;
        sal_uInt32 graphicsVersion, penType;
        s.ReadUInt32(graphicsVersion).ReadUInt32(penType).ReadUInt32(penDataFlags).ReadUInt32(penUnit).ReadFloat(penWidth);
        SAL_INFO("drawinglayer.emf", "EMF+\t\tGraphics version: 0x" << std::hex << graphicsVersion);
        SAL_INFO("drawinglayer.emf", "EMF+\t\tType: " << penType);
        SAL_INFO("drawinglayer.emf", "EMF+\t\tPen data flags: 0x" << penDataFlags << PenDataFlagsToString(penDataFlags));
        SAL_INFO("drawinglayer.emf", "EMF+\t\tUnit: " << UnitTypeToString(penUnit));
        SAL_INFO("drawinglayer.emf", "EMF+\t\tWidth: " << std::dec << penWidth);

        // If a zero width is specified, a minimum value must be used, which is determined by the units
        if (penWidth == 0.0)
        { //TODO Check if these values is correct
            penWidth = penUnit == 0 ? 0.18f
                : 0.05f;  // 0.05f is taken from old EMF+ implementation (case of Unit == Pixel etc.)
        }

        if (penDataFlags & EmfPlusPenDataTransform)
        {
            EmfPlusHelperData::readXForm(s, pen_transformation);
            SAL_WARN("drawinglayer.emf", "EMF+\t\t TODO PenDataTransform: " << pen_transformation);
        }

        if (penDataFlags & EmfPlusPenDataStartCap)
        {
            s.ReadInt32(startCap);
            SAL_INFO("drawinglayer.emf", "EMF+\t\tstartCap: " << LineCapTypeToString(startCap) << " (0x" << std::hex << startCap << ")");
        }
        else
        {
            startCap = 0;
        }

        if (penDataFlags & EmfPlusPenDataEndCap)
        {
            s.ReadInt32(endCap);
            SAL_INFO("drawinglayer.emf", "EMF+\t\tendCap: " << LineCapTypeToString(endCap) << " (0x" << std::hex << startCap << ")");
        }
        else
        {
            endCap = 0;
        }

        if (penDataFlags & EmfPlusPenDataJoin)
        {
            s.ReadInt32(lineJoin);
            SAL_INFO("drawinglayer.emf", "EMF+\t\t LineJoin: " << lineJoin);
            switch (lineJoin)
            {
                case EmfPlusLineJoinTypeBevel:
                    maLineJoin = basegfx::B2DLineJoin::Bevel;
                    break;
                case EmfPlusLineJoinTypeRound:
                    maLineJoin = basegfx::B2DLineJoin::Round;
                    break;
                case EmfPlusLineJoinTypeMiter:
                case EmfPlusLineJoinTypeMiterClipped:
                default: // If nothing set, then apply Miter (based on MS Paint)
                    maLineJoin = basegfx::B2DLineJoin::Miter;
                    break;
            }
        }
        else
            maLineJoin = basegfx::B2DLineJoin::Miter;

        if (penDataFlags & EmfPlusPenDataMiterLimit)
        {
            float miterLimit;
            s.ReadFloat(miterLimit);

            // EMF+ JoinTypeMiterClipped is working as our B2DLineJoin::Miter
            // For EMF+ LineJoinTypeMiter we are simulating it by changing angle
            if (lineJoin == EmfPlusLineJoinTypeMiter)
                miterLimit = 3.0 * miterLimit;
            // asin angle must be in range [-1, 1]
            if (abs(miterLimit) > 1.0)
                fMiterMinimumAngle = 2.0 * asin(1.0 / miterLimit);
            else
                // enable miter limit for all angles
                fMiterMinimumAngle = basegfx::deg2rad(180.0);
            SAL_INFO("drawinglayer.emf",
                     "EMF+\t\t MiterLimit: " << std::dec << miterLimit
                                             << ", Miter minimum angle (rad): " << fMiterMinimumAngle);
        }
        else
            fMiterMinimumAngle = basegfx::deg2rad(5.0);


        if (penDataFlags & EmfPlusPenDataLineStyle)
        {
            s.ReadInt32(dashStyle);
            SAL_INFO("drawinglayer.emf", "EMF+\t\tdashStyle: " << DashedLineCapTypeToString(dashStyle) << " (0x" << std::hex << dashStyle << ")");
        }
        else
        {
            dashStyle = 0;
        }

        if (penDataFlags & EmfPlusPenDataDashedLineCap)
        {
            s.ReadInt32(dashCap);
            SAL_WARN("drawinglayer.emf", "EMF+\t\t TODO PenDataDashedLineCap: 0x" << std::hex << dashCap);
        }
        else
        {
            dashCap = 0;
        }

        if (penDataFlags & EmfPlusPenDataDashedLineOffset)
        {
            s.ReadFloat(dashOffset);
            SAL_WARN("drawinglayer.emf", "EMF+\t\t TODO PenDataDashedLineOffset: 0x" << std::hex << dashOffset);
        }
        else
        {
            dashOffset = 0;
        }

        if (penDataFlags & EmfPlusPenDataDashedLine)
        {
            dashStyle = EmfPlusLineStyleCustom;
            sal_uInt32 dashPatternLen;

            s.ReadUInt32(dashPatternLen);
            SAL_INFO("drawinglayer.emf", "EMF+\t\t\tdashPatternLen: " << dashPatternLen);

            dashPattern.resize( dashPatternLen );

            for (sal_uInt32 i = 0; i < dashPatternLen; i++)
            {
                s.ReadFloat(dashPattern[i]);
                SAL_INFO("drawinglayer.emf", "EMF+\t\t\t\tdashPattern[" << i << "]: " << dashPattern[i]);
            }
        }

        if (penDataFlags & EmfPlusPenDataAlignment)
        {
            s.ReadInt32(alignment);
            SAL_WARN("drawinglayer.emf", "EMF+\t\t\tTODO PenDataAlignment: " << PenAlignmentToString(alignment) << " (0x" << std::hex << alignment << ")");
        }
        else
        {
            alignment = 0;
        }

        if (penDataFlags & EmfPlusPenDataCompoundLine)
        {
            SAL_WARN("drawinglayer.emf", "EMF+\t\t\tTODO PenDataCompoundLine");
            sal_uInt32 compoundArrayLen;
            s.ReadUInt32(compoundArrayLen);

            compoundArray.resize(compoundArrayLen);

            for (sal_uInt32 i = 0; i < compoundArrayLen; i++)
            {
                s.ReadFloat(compoundArray[i]);
                SAL_INFO("drawinglayer.emf", "EMF+\t\t\t\tcompoundArray[" << i << "]: " << compoundArray[i]);
            }
        }

        if (penDataFlags & EmfPlusPenDataCustomStartCap)
        {
            s.ReadUInt32(customStartCapLen);
            SAL_INFO("drawinglayer.emf", "EMF+\t\t\tcustomStartCapLen: " << customStartCapLen);
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

        if (penDataFlags & EmfPlusPenDataCustomEndCap)
        {
            s.ReadUInt32(customEndCapLen);
            SAL_INFO("drawinglayer.emf", "EMF+\t\t\tcustomEndCapLen: " << customEndCapLen);
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
