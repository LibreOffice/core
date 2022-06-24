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

#include <cmath>

#include <drawingml/transform2dcontext.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/drawingml/shape.hxx>
#include <drawingml/customshapeproperties.hxx>
#include <drawingml/textbody.hxx>
#include <oox/token/namespaces.hxx>

#include <com/sun/star/awt/Rectangle.hpp>

using namespace ::com::sun::star;
using ::oox::core::ContextHandlerRef;

namespace oox::drawingml {

/** context to import a CT_Transform2D */
Transform2DContext::Transform2DContext( ContextHandler2Helper const & rParent, const AttributeList& rAttribs, Shape& rShape, bool btxXfrm ) noexcept
: ContextHandler2( rParent )
, mrShape( rShape )
, mbtxXfrm ( btxXfrm )
{
    if( !btxXfrm )
    {
        mrShape.setRotation( rAttribs.getInteger( XML_rot, 0 ) ); // 60000ths of a degree Positive angles are clockwise; negative angles are counter-clockwise
        mrShape.setFlip( rAttribs.getBool( XML_flipH, false ), rAttribs.getBool( XML_flipV, false ) );
    }
    else
    {
        if (rAttribs.hasAttribute(XML_rot))
        {
            // We might have already a text rotation from 'vert="vert"' or 'vert="vert270"' in addition.
            // And we need the txXfrm rotation for adapting insets.
            sal_Int32 nVertRot = mrShape.getTextBody()->getTextProperties().moRotation.value_or(0);
            mno_txXfrmRot = rAttribs.getInteger(XML_rot, 0);
            mrShape.getTextBody()->getTextProperties().moRotation = mno_txXfrmRot.value() + nVertRot;
            // TextBodyProperties::pushTextDistances() rotates insets according moRotation.
            // Rotate them here opposite for compensation.
            std::array<sal_Int32, 4> aOldInsets;
            for (size_t i = 0; i < 4; i++)
                aOldInsets[i] = mrShape.getTextBody()->getTextProperties().moInsets[i].value_or(0);
            if (mno_txXfrmRot == 16200000 || mno_txXfrmRot == -5400000)
            {
                mrShape.getTextBody()->getTextProperties().moInsets[0] = aOldInsets[1];
                mrShape.getTextBody()->getTextProperties().moInsets[1] = aOldInsets[2];
                mrShape.getTextBody()->getTextProperties().moInsets[2] = aOldInsets[3];
                mrShape.getTextBody()->getTextProperties().moInsets[3] = aOldInsets[0];
            }
            else if (mno_txXfrmRot == 10800000 || mno_txXfrmRot == -10800000)
            {
                mrShape.getTextBody()->getTextProperties().moInsets[0] = aOldInsets[2];
                mrShape.getTextBody()->getTextProperties().moInsets[1] = aOldInsets[3];
                mrShape.getTextBody()->getTextProperties().moInsets[2] = aOldInsets[0];
                mrShape.getTextBody()->getTextProperties().moInsets[3] = aOldInsets[1];
            }
            else if (mno_txXfrmRot == 5400000 || mno_txXfrmRot == -16200000)
            {
                mrShape.getTextBody()->getTextProperties().moInsets[0] = aOldInsets[3];
                mrShape.getTextBody()->getTextProperties().moInsets[1] = aOldInsets[0];
                mrShape.getTextBody()->getTextProperties().moInsets[2] = aOldInsets[1];
                mrShape.getTextBody()->getTextProperties().moInsets[3] = aOldInsets[2];
            }
        }
    }
}

namespace
{
bool ConstructPresetTextRectangle(Shape& rShape, awt::Rectangle& rRect)
{
    const sal_Int32 nType = rShape.getCustomShapeProperties()->getShapePresetType();
    switch (nType)
    {
        case XML_ellipse:
            // The preset text rectangle touches the perimeter of the ellipse at 45deg.
            rRect.X = rShape.getPosition().X + rShape.getSize().Width * ((1.0 - M_SQRT1_2) / 2.0);
            rRect.Y = rShape.getPosition().Y + rShape.getSize().Height * ((1.0 - M_SQRT1_2) / 2.0);
            rRect.Width = rShape.getSize().Width * M_SQRT1_2;
            rRect.Height = rShape.getSize().Height * M_SQRT1_2;
            return true;
        case XML_roundRect:
        case XML_round2SameRect:
        {
            auto aAdjGdList = rShape.getCustomShapeProperties()->getAdjustmentGuideList();
            double fAdj = aAdjGdList.empty() ? 16667 : aAdjGdList[0].maFormula.toDouble();
            sal_Int32 nWidth = rShape.getSize().Width;
            sal_Int32 nHeight = rShape.getSize().Height;
            if (nWidth == 0 || nHeight == 0)
                return false;
            double fMaxAdj = 50000.0 * nWidth / std::min(nWidth, nHeight);
            std::clamp<double>(fAdj, 0, fMaxAdj);
            sal_Int32 nTextLeft = std::min(nWidth, nHeight) * fAdj / 100000.0 * 0.29289;
            sal_Int32 nTextTop = nTextLeft;
            rRect.X = rShape.getPosition().X + nTextLeft;
            rRect.Y = rShape.getPosition().Y + nTextTop;
            rRect.Width = nWidth - 2 * nTextLeft;
            rRect.Height = nHeight - 2 * nTextTop;
            return true;
        }
        case XML_trapezoid:
        {
            auto aAdjGdList = rShape.getCustomShapeProperties()->getAdjustmentGuideList();
            double fAdj = aAdjGdList.empty() ? 25000 : aAdjGdList[0].maFormula.toDouble();
            sal_Int32 nWidth = rShape.getSize().Width;
            sal_Int32 nHeight = rShape.getSize().Height;
            if (nWidth == 0 || nHeight == 0)
                return false;
            double fMaxAdj = 50000.0 * nWidth / std::min(nWidth, nHeight);
            std::clamp<double>(fAdj, 0, fMaxAdj);
            sal_Int32 nTextLeft = nWidth / 3.0 * fAdj / fMaxAdj;
            sal_Int32 nTextTop = nHeight / 3.0 * fAdj / fMaxAdj;
            rRect.X = rShape.getPosition().X + nTextLeft;
            rRect.Y = rShape.getPosition().Y + nTextTop;
            rRect.Width = nWidth - 2 * nTextLeft;
            rRect.Height = nHeight - 2 * nTextTop;
            return true;
        }
        case XML_flowChartManualOperation:
        {
            sal_Int32 nWidth = rShape.getSize().Width;
            sal_Int32 nTextLeft = nWidth / 5;
            rRect.X = rShape.getPosition().X + nTextLeft;
            rRect.Y = rShape.getPosition().Y;
            rRect.Width = nWidth - 2 * nTextLeft;
            rRect.Height = rShape.getSize().Height;
            return true;
        }
        case XML_pie:
        case XML_rect:
        default:
            // When tdf#149918 is fixed, pie will need its own case
            rRect.X = rShape.getPosition().X;
            rRect.Y = rShape.getPosition().Y;
            rRect.Width = rShape.getSize().Width;
            rRect.Height = rShape.getSize().Height;
            return true;
    }
}
}

ContextHandlerRef Transform2DContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    if( mbtxXfrm )
    {
        // The child elements <a:off> and <a:ext> of a <dsp:txXfrm> element describe the position and
        // size of the text area rectangle. Compared to the text area as defined in the preset it
        // might be rotated by 90deg, -90deg or 180deg, depending on shape rotation.
        // We cannot change the text area rectangle directly, because currently we depend on the
        // geometry definition of the preset. As workaround we change the indents to move and scale
        // the text block. The needed shifts are calculated here as moTextOff and used in
        // TextBodyProperties::pushTextDistances().

        awt::Rectangle aPresetTextRectangle;
        if (!ConstructPresetTextRectangle(mrShape, aPresetTextRectangle))
            return nullptr; // faulty shape

        switch (aElementToken)
        {
            case A_TOKEN(off):
            {
                // need <a:ext> too, so only save values here.
                const OUString sXValue = rAttribs.getStringDefaulted(XML_x);
                const OUString sYValue = rAttribs.getStringDefaulted(XML_y);
                if (!sXValue.isEmpty() && !sYValue.isEmpty())
                {
                    mno_txXfrmOffX = sXValue.toInt32();
                    mno_txXfrmOffY = sYValue.toInt32();
                }
            }
            break;
            case A_TOKEN(ext):
            {
                awt::Rectangle aUnrotatedTxXfrm = aPresetTextRectangle;
                const OUString sCXValue = rAttribs.getStringDefaulted(XML_cx);
                const OUString sCYValue = rAttribs.getStringDefaulted(XML_cy);
                // rotate if needed and possible
                if (!sCXValue.isEmpty() && !sCYValue.isEmpty() && mno_txXfrmOffX.has_value()
                    && mno_txXfrmOffY.has_value())
                {
                    sal_Int32 ntxXfrmWidth = sCXValue.toInt32();
                    sal_Int32 ntxXfrmHeight = sCYValue.toInt32();
                    // MS Office restricts the angles to -90deg, 90deg and 180deg
                    if (mno_txXfrmRot.has_value() && std::abs(mno_txXfrmRot.value()) == 5400000)
                    {
                        // rotate at center
                        sal_Int32 ntxXfrmCenterX = mno_txXfrmOffX.value() + ntxXfrmWidth / 2;
                        sal_Int32 ntxXfrmCenterY = mno_txXfrmOffY.value() + ntxXfrmHeight / 2;
                        aUnrotatedTxXfrm.X = ntxXfrmCenterX - ntxXfrmHeight / 2;
                        aUnrotatedTxXfrm.Y = ntxXfrmCenterY - ntxXfrmWidth / 2;
                        aUnrotatedTxXfrm.Width = ntxXfrmHeight;
                        aUnrotatedTxXfrm.Height = ntxXfrmWidth;
                    }
                    else // 180deg or 0deg
                    {
                        aUnrotatedTxXfrm.X = mno_txXfrmOffX.value();
                        aUnrotatedTxXfrm.Y = mno_txXfrmOffY.value();
                        aUnrotatedTxXfrm.Width = ntxXfrmWidth;
                        aUnrotatedTxXfrm.Height = ntxXfrmHeight;
                    }
                }
                else if (mno_txXfrmOffX.has_value() && mno_txXfrmOffY.has_value())
                {
                    aUnrotatedTxXfrm.X = mno_txXfrmOffX.value();
                    aUnrotatedTxXfrm.Y = mno_txXfrmOffY.value();
                }
                else if (!sCXValue.isEmpty() && !sCYValue.isEmpty())
                {
                    aUnrotatedTxXfrm.Width = sCXValue.toInt32();
                    aUnrotatedTxXfrm.Height = sCYValue.toInt32();
                }
                // Calulate indent offsets
                sal_Int32 nOffsetLeft = aUnrotatedTxXfrm.X - aPresetTextRectangle.X;
                if (nOffsetLeft)
                    mrShape.getTextBody()->getTextProperties().moTextOffLeft
                        = GetCoordinate(nOffsetLeft);

                sal_Int32 nOffsetTop = aUnrotatedTxXfrm.Y - aPresetTextRectangle.Y;
                if (nOffsetTop)
                    mrShape.getTextBody()->getTextProperties().moTextOffUpper
                        = GetCoordinate(nOffsetTop);

                sal_Int32 nOffsetRight
                    = aPresetTextRectangle.Width - aUnrotatedTxXfrm.Width - nOffsetLeft;
                if (nOffsetRight)
                    mrShape.getTextBody()->getTextProperties().moTextOffRight
                        = GetCoordinate(nOffsetRight);

                sal_Int32 nOffsetBottom
                    = aPresetTextRectangle.Height - aUnrotatedTxXfrm.Height - nOffsetTop;
                if (nOffsetBottom)
                    mrShape.getTextBody()->getTextProperties().moTextOffLower
                        = GetCoordinate(nOffsetBottom);
            }
            break;
        }
        return nullptr;
    }

    switch( aElementToken )
    {
    case A_TOKEN( off ):        // horz/vert translation
        mrShape.setPosition( awt::Point( rAttribs.getStringDefaulted( XML_x ).toInt32(), rAttribs.getStringDefaulted( XML_y ).toInt32() ) );
        break;
    case A_TOKEN( ext ):        // horz/vert size
        mrShape.setSize( awt::Size( rAttribs.getStringDefaulted( XML_cx ).toInt32(), rAttribs.getStringDefaulted( XML_cy ).toInt32() ) );
        break;
    case A_TOKEN( chOff ):  // horz/vert translation of children
        mrShape.setChildPosition( awt::Point( rAttribs.getStringDefaulted( XML_x ).toInt32(), rAttribs.getStringDefaulted( XML_y ).toInt32() ) );
        break;
    case A_TOKEN( chExt ):  // horz/vert size of children
        {
            sal_Int32 nChExtCx = rAttribs.getStringDefaulted(XML_cx).toInt32();

            if(nChExtCx == 0)
                nChExtCx = mrShape.getSize().Width;

            sal_Int32 nChExtCy = rAttribs.getStringDefaulted(XML_cy).toInt32();

            if(nChExtCy == 0)
                nChExtCy = mrShape.getSize().Height;

            mrShape.setChildSize(awt::Size(nChExtCx, nChExtCy));
        }
        break;
    }

    return nullptr;
}

} // namespace oox::drawingml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
