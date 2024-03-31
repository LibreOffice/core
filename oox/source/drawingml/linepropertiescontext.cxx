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

#include <drawingml/linepropertiescontext.hxx>
#include <drawingml/misccontexts.hxx>
#include <drawingml/lineproperties.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <docmodel/theme/FormatScheme.hxx>

using namespace ::oox::core;

// CT_LineProperties

namespace oox::drawingml {

LinePropertiesContext::LinePropertiesContext( ContextHandler2Helper const & rParent, const AttributeList& rAttribs,
        LineProperties& rLineProperties, model::LineStyle* pLineStyle) noexcept
    : ContextHandler2(rParent)
    , mpLineStyle(pLineStyle)
    , mrLineProperties(rLineProperties)
{
    mrLineProperties.moLineWidth = rAttribs.getInteger( XML_w );
    mrLineProperties.moLineCompound = rAttribs.getToken( XML_cmpd );
    mrLineProperties.moLineCap = rAttribs.getToken( XML_cap );

    if (mpLineStyle)
    {
        mpLineStyle->mnWidth = rAttribs.getInteger(XML_w, 0);

        switch (rAttribs.getToken(XML_cap, XML_TOKEN_INVALID))
        {
            case XML_rnd: mpLineStyle->meCapType = model::CapType::Round; break;
            case XML_sq: mpLineStyle->meCapType = model::CapType::Square; break;
            case XML_flat: mpLineStyle->meCapType = model::CapType::Flat; break;
            default:
                mpLineStyle->meCapType = model::CapType::Unset; break;
        }

        switch (rAttribs.getToken(XML_cmpd, XML_TOKEN_INVALID))
        {
            case XML_sng: mpLineStyle->meCompoundLineType = model::CompoundLineType::Single; break;
            case XML_dbl: mpLineStyle->meCompoundLineType = model::CompoundLineType::Double; break;
            case XML_thickThin: mpLineStyle->meCompoundLineType = model::CompoundLineType::ThickThin_Double; break;
            case XML_thinThick: mpLineStyle->meCompoundLineType = model::CompoundLineType::ThinThick_Double; break;
            case XML_tri: mpLineStyle->meCompoundLineType = model::CompoundLineType::Triple; break;
            default:
                mpLineStyle->meCompoundLineType = model::CompoundLineType::Unset; break;
        }

        switch (rAttribs.getToken(XML_algn, XML_TOKEN_INVALID))
        {
            case XML_ctr: mpLineStyle->mePenAlignment  = model::PenAlignmentType::Center; break;
            case XML_in: mpLineStyle->mePenAlignment  = model::PenAlignmentType::Inset; break;
            default:
                mpLineStyle->mePenAlignment = model::PenAlignmentType::Unset; break;
        }
    }
}

LinePropertiesContext::~LinePropertiesContext()
{
}

ContextHandlerRef LinePropertiesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        // LineFillPropertiesGroup
        case A_TOKEN( noFill ):
        case A_TOKEN( solidFill ):
        case A_TOKEN( gradFill ):
        case A_TOKEN( pattFill ):
        {
            model::FillStyle* pFillStyle = nullptr;
            if (mpLineStyle)
            {
                pFillStyle = &mpLineStyle->maLineFillStyle;
            }
            return FillPropertiesContext::createFillContext(*this, nElement, rAttribs, mrLineProperties.maLineFill, pFillStyle);
        }
        break;

        // LineDashPropertiesGroup
        case A_TOKEN( prstDash ):  // CT_PresetLineDashProperties
        {
            mrLineProperties.moPresetDash = rAttribs.getToken( XML_val );

            if (mpLineStyle)
            {
                auto& rLineDash = mpLineStyle->maLineDash;
                switch (rAttribs.getToken(XML_val, XML_TOKEN_INVALID))
                {
                    case XML_solid: rLineDash.mePresetType  = model::PresetDashType::Solid; break;
                    case XML_dot: rLineDash.mePresetType  = model::PresetDashType::Dot; break;
                    case XML_dash: rLineDash.mePresetType  = model::PresetDashType::Dash; break;
                    case XML_lgDash: rLineDash.mePresetType  = model::PresetDashType::LargeDash; break;
                    case XML_dashDot: rLineDash.mePresetType  = model::PresetDashType::DashDot; break;
                    case XML_lgDashDot: rLineDash.mePresetType  = model::PresetDashType::LargeDashDot; break;
                    case XML_lgDashDotDot: rLineDash.mePresetType  = model::PresetDashType::LargeDashDotDot; break;
                    case XML_sysDash: rLineDash.mePresetType  = model::PresetDashType::SystemDash; break;
                    case XML_sysDot: rLineDash.mePresetType  = model::PresetDashType::SystemDot; break;
                    case XML_sysDashDot: rLineDash.mePresetType  = model::PresetDashType::SystemDashDot; break;
                    case XML_sysDashDotDot: rLineDash.mePresetType  = model::PresetDashType::SystemDashDotDot; break;
                    default:
                        rLineDash.mePresetType = model::PresetDashType::Unset; break;
                }
            }
        }
        break;
        case A_TOKEN( custDash ):  // CT_DashStopList
            return this;
        case A_TOKEN( ds ):
        {
            // 'a:ds' has 2 attributes : 'd' and 'sp'
            // both are of type 'a:ST_PositivePercentage'
            // according to the specs Office will read percentages formatted with a trailing percent sign
            // or formatted as 1000th of a percent without a trailing percent sign, but only write percentages
            // as 1000th's of a percent without a trailing percent sign.
            // The code below takes care of both scenarios by converting to '1000th of a percent' always
            OUString aStr;
            sal_Int32 nDashLength = 0;
            aStr = rAttribs.getStringDefaulted( XML_d);
            if ( aStr.endsWith("%") )
            {
                // Ends with a '%'
                aStr = aStr.copy(0, aStr.getLength() - 1);
                aStr = aStr.trim();
                nDashLength = aStr.toInt32();

                // Convert to 1000th of a percent
                nDashLength *= 1000;
            }
            else
            {
                nDashLength = rAttribs.getInteger( XML_d, 0 );
            }

            sal_Int32 nSpaceLength = 0;
            aStr = rAttribs.getStringDefaulted( XML_sp);
            if ( aStr.endsWith("%") )
            {
                // Ends with a '%'
                aStr = aStr.copy(0, aStr.getLength() - 1);
                aStr = aStr.trim();
                nSpaceLength = aStr.toInt32();

                // Convert to 1000th of a percent
                nSpaceLength *= 1000;
            }
            else
            {
                nSpaceLength = rAttribs.getInteger( XML_sp, 0 );
            }

            mrLineProperties.maCustomDash.emplace_back( nDashLength, nSpaceLength );

            if (mpLineStyle)
            {
                mpLineStyle->maLineDash.maCustomList.push_back({ nDashLength, nSpaceLength });
            }
        }
        break;

        // LineJoinPropertiesGroup
        case A_TOKEN( round ):
        case A_TOKEN( bevel ):
        case A_TOKEN( miter ):
        {
            sal_Int32 nToken = getBaseToken(nElement);
            mrLineProperties.moLineJoint = nToken;

            if (mpLineStyle)
            {
                switch (nToken)
                {
                    case XML_round: mpLineStyle->maLineJoin.meType = model::LineJoinType::Round; break;
                    case XML_bevel: mpLineStyle->maLineJoin.meType = model::LineJoinType::Bevel; break;
                    case XML_miter: mpLineStyle->maLineJoin.meType = model::LineJoinType::Miter; break;
                    default:
                        mpLineStyle->maLineJoin.meType = model::LineJoinType::Miter;
                }
                if (nToken == XML_miter)
                {
                    sal_Int32 nMiterLimit = rAttribs.getInteger(XML_lim, 0);
                    mpLineStyle->maLineJoin.mnMiterLimit = nMiterLimit;
                }
            }
        }
        break;

        case A_TOKEN( headEnd ):  // CT_LineEndProperties
        case A_TOKEN( tailEnd ):  // CT_LineEndProperties
        {                         // ST_LineEndType
            bool bTailEnd = nElement == A_TOKEN( tailEnd );
            LineArrowProperties& rArrowProps = bTailEnd ? mrLineProperties.maEndArrow : mrLineProperties.maStartArrow;
            rArrowProps.moArrowType = rAttribs.getToken( XML_type );
            rArrowProps.moArrowWidth = rAttribs.getToken( XML_w );
            rArrowProps.moArrowLength = rAttribs.getToken( XML_len );

            if (mpLineStyle)
            {
                model::LineEndType eLineEndType = model::LineEndType::None;
                switch (rAttribs.getToken(XML_type, XML_none))
                {
                    case XML_triangle: eLineEndType = model::LineEndType::Triangle; break;
                    case XML_stealth: eLineEndType = model::LineEndType::Stealth; break;
                    case XML_diamond: eLineEndType = model::LineEndType::Diamond; break;
                    case XML_oval: eLineEndType = model::LineEndType::Oval; break;
                    case XML_arrow: eLineEndType = model::LineEndType::Arrow; break;
                    default:
                    case XML_none: eLineEndType = model::LineEndType::None; break;
                }

                model::LineEndLength eLineEndLength = model::LineEndLength::Unset;
                switch (rAttribs.getToken(XML_len, XML_TOKEN_INVALID))
                {
                    case XML_sm: eLineEndLength = model::LineEndLength::Small; break;
                    case XML_med: eLineEndLength = model::LineEndLength::Medium; break;
                    case XML_lg: eLineEndLength = model::LineEndLength::Large; break;
                    default:
                        break;
                }

                model::LineEndWidth eLineEndWidth = model::LineEndWidth::Unset;
                switch (rAttribs.getToken(XML_w, XML_TOKEN_INVALID))
                {
                    case XML_sm: eLineEndWidth = model::LineEndWidth::Small; break;
                    case XML_med: eLineEndWidth = model::LineEndWidth::Medium; break;
                    case XML_lg: eLineEndWidth = model::LineEndWidth::Large; break;
                    default:
                        break;
                }

                if (nElement == A_TOKEN(tailEnd))
                {
                    mpLineStyle->maTailEnd.meType = eLineEndType;
                    mpLineStyle->maTailEnd.meLength = eLineEndLength;
                    mpLineStyle->maTailEnd.meWidth = eLineEndWidth;
                }
                else if (nElement == A_TOKEN(headEnd))
                {
                    mpLineStyle->maHeadEnd.meType = eLineEndType;
                    mpLineStyle->maHeadEnd.meLength = eLineEndLength;
                    mpLineStyle->maHeadEnd.meWidth = eLineEndWidth;
                }
            }
        }
        break;
    }
    return nullptr;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
