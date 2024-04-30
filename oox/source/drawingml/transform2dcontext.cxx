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

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <drawingml/customshapeproperties.hxx>
#include <drawingml/textbody.hxx>
#include <oox/drawingml/shape.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>

#include <com/sun/star/awt/Rectangle.hpp>

using namespace ::com::sun::star;
using ::oox::core::ContextHandlerRef;

namespace oox::drawingml {

/** context to import a CT_Transform2D */
Transform2DContext::Transform2DContext( ContextHandler2Helper const & rParent, const AttributeList& rAttribs, Shape& rShape, bool btxXfrm )
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
        if (rAttribs.hasAttribute(XML_rot) && mrShape.getTextBody())
        {
            mno_txXfrmRot = rAttribs.getInteger(XML_rot, 0);
            sal_Int32 nTextAreaRot = mrShape.getTextBody()->getTextProperties().moTextAreaRotation.value_or(0);
            mrShape.getTextBody()->getTextProperties().moTextAreaRotation = mno_txXfrmRot.value() + nTextAreaRot;
        }
    }
}

namespace
{
bool ConstructPresetTextRectangle(Shape& rShape, awt::Rectangle& rRect)
{
    // When we are here, we have neither xShape nor a SdrObject. So need to manually calc the text
    // area rectangle defined in the preset in OOXML standard, but only for those types of shapes
    // where we know, that MS Office SmartArt presets do not use the default text area rectangle.
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
            // Second handle of round2SameRect used in preset diagrams has value 0.
            const auto& aAdjGdList = rShape.getCustomShapeProperties()->getAdjustmentGuideList();
            double fAdj = aAdjGdList.empty() ? 16667 : aAdjGdList[0].maFormula.toDouble();
            sal_Int32 nWidth = rShape.getSize().Width;
            sal_Int32 nHeight = rShape.getSize().Height;
            if (nWidth == 0 || nHeight == 0)
                return false;
            double fMaxAdj = 50000.0 * nWidth / std::min(nWidth, nHeight);
            fAdj = std::clamp<double>(fAdj, 0, fMaxAdj);
            sal_Int32 nTextLeft = std::min(nWidth, nHeight) * fAdj / 100000.0 * 0.29289;
            sal_Int32 nTextTop = nTextLeft;
            rRect.X = rShape.getPosition().X + nTextLeft;
            rRect.Y = rShape.getPosition().Y + nTextTop;
            rRect.Width = nWidth - 2 * nTextLeft;
            rRect.Height = nHeight - (nType == XML_roundRect ? 2 : 1) * nTextTop;
            return true;
        }
        case XML_trapezoid:
        {
            const auto& aAdjGdList = rShape.getCustomShapeProperties()->getAdjustmentGuideList();
            double fAdj = aAdjGdList.empty() ? 25000 : aAdjGdList[0].maFormula.toDouble();
            sal_Int32 nWidth = rShape.getSize().Width;
            sal_Int32 nHeight = rShape.getSize().Height;
            if (nWidth == 0 || nHeight == 0)
                return false;
            double fMaxAdj = 50000.0 * nWidth / std::min(nWidth, nHeight);
            fAdj = std::clamp<double>(fAdj, 0, fMaxAdj);
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
        case XML_wedgeRectCallout:
        {
            // When tdf#149918 is fixed, pie will need its own case
            rRect.X = rShape.getPosition().X;
            rRect.Y = rShape.getPosition().Y;
            rRect.Width = rShape.getSize().Width;
            rRect.Height = rShape.getSize().Height;
            return true;
        }
        case XML_gear6:
        {
            // The identifiers here reflect the guides name value in presetShapeDefinitions.xml
            double w = rShape.getSize().Width;
            double h = rShape.getSize().Height;
            if (w <= 0 || h <= 0)
                return false;
            double a1(15000.0);
            double a2(3526.0);
            const auto& aAdjGdList = rShape.getCustomShapeProperties()->getAdjustmentGuideList();
            if (aAdjGdList.size() == 2)
            {
                a1 = aAdjGdList[0].maFormula.toDouble();
                a2 = aAdjGdList[1].maFormula.toDouble();
                a1 = std::clamp<double>(a1, 0, 20000);
                a2 = std::clamp<double>(a2, 0, 5358);
            }
            double th = std::min(w, h) * a1 / 100000.0;
            double l2 = std::min(w, h) * a2 / 100000.0 / 2.0;
            double l3 = th / 2.0 + l2;

            double rh = h / 2.0 - th;
            double rw = w / 2.0 - th;

            double maxr = std::min(rw, rh);
            double ha = atan2(l3, maxr);

            double aA1 = basegfx::deg2rad(330) - ha;
            double ta11 = rw * cos(aA1);
            double ta12 = rh * sin(aA1);
            double bA1 = atan2(ta12, ta11);
            double cta1 = rh * cos(bA1);
            double sta1 = rw * sin(bA1);
            double ma1 = std::hypot(cta1, sta1);
            double na1 = rw * rh / ma1;
            double dxa1 = na1 * cos(bA1);
            double dya1 = na1 * sin(bA1);

            double xA1 = w / 2.0 + dxa1; // r
            double yA1 = h / 2.0 + dya1; // t
            double yD2 = h - yA1; // b
            double xD5 = w - xA1; // l

            rRect.X = rShape.getPosition().X + xD5;
            rRect.Y = rShape.getPosition().Y + yA1;
            rRect.Width = xA1 - xD5;
            rRect.Height = yD2 - yA1;
            return true;
        }
        case XML_hexagon:
        {
            const auto& aAdjGdList = rShape.getCustomShapeProperties()->getAdjustmentGuideList();
            double fAdj = aAdjGdList.empty() ? 25000 : aAdjGdList[0].maFormula.toDouble();
            sal_Int32 nWidth = rShape.getSize().Width;
            sal_Int32 nHeight = rShape.getSize().Height;
            if (nWidth == 0 || nHeight == 0)
                return false;
            double fMaxAdj = 50000.0 * nWidth / std::min(nWidth, nHeight);
            fAdj = std::clamp<double>(fAdj, 0, fMaxAdj);
            double fFactor = fAdj / fMaxAdj / 6.0 + 1.0 / 12.0;
            sal_Int32 nTextLeft = nWidth * fFactor;
            sal_Int32 nTextTop = nHeight * fFactor;
            rRect.X = rShape.getPosition().X + nTextLeft;
            rRect.Y = rShape.getPosition().Y + nTextTop;
            rRect.Width = nWidth - 2 * nTextLeft;
            rRect.Height = nHeight - 2 * nTextTop;
            return true;
        }
        case XML_round1Rect:
        {
            sal_Int32 nWidth = rShape.getSize().Width;
            sal_Int32 nHeight = rShape.getSize().Height;
            if (nWidth == 0 || nHeight == 0)
                return false;
            const auto& aAdjGdList = rShape.getCustomShapeProperties()->getAdjustmentGuideList();
            double fAdj = aAdjGdList.empty() ? 16667.0 : aAdjGdList[0].maFormula.toDouble();
            fAdj = std::clamp<double>(fAdj, 0.0, 50000.0);
            double fDx = std::min(nWidth, nHeight) * fAdj / 100000.0 * 0.29289;
            rRect.X = rShape.getPosition().X;
            rRect.Y = rShape.getPosition().Y;
            rRect.Width = nWidth - fDx;
            rRect.Height = nHeight;
            return true;
        }
        case XML_rightArrow:
        {
            // The identifiers here reflect the guides name value in presetShapeDefinitions.xml
            sal_Int32 nWidth = rShape.getSize().Width;
            sal_Int32 nHeight = rShape.getSize().Height;
            if (nWidth == 0 || nHeight == 0)
                return false;
            double a1(50000.0);
            double a2(50000.0);
            const auto& aAdjGdList = rShape.getCustomShapeProperties()->getAdjustmentGuideList();
            if (aAdjGdList.size() == 2)
            {
                a1 = aAdjGdList[0].maFormula.toDouble();
                a2 = aAdjGdList[1].maFormula.toDouble();
                a1 = std::clamp<double>(a1, 0, 100000);
            }
            double maxAdj2 = 100000.0 * nWidth / std::min(nWidth, nHeight);
            a2 = std::clamp<double>(a2, 0, maxAdj2);
            double dx1 = std::min(nWidth, nHeight) * a2 / 100000.0;
            double x1 = nWidth - dx1;
            double dy1 = nHeight * a1 / 200000.0;
            double y1 = nHeight / 2.0 - dy1; // top
            double y2 = nHeight / 2.0 + dy1; // bottom
            double dx2 = y1 * dx1 / (nHeight / 2.0);
            double x2 = x1 + dx2; // right
            rRect.X = rShape.getPosition().X; // left = 0
            rRect.Y = rShape.getPosition().Y + y1;
            rRect.Width = x2;
            rRect.Height = y2 - y1;
            return true;
        }
        default:
            return false;
    }
}

basegfx::B2DPoint getCenter(const awt::Rectangle& rRect)
{
    return basegfx::B2DPoint(rRect.X + rRect.Width / 2.0, rRect.Y + rRect.Height / 2.0);
}
} // end namespace

ContextHandlerRef Transform2DContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    if (mbtxXfrm)
    {
        // The child elements <a:off> and <a:ext> of a <dsp:txXfrm> element describe the position and
        // size of the text area rectangle. We cannot change the text area rectangle directly, because
        // currently we depend on the geometry definition of the preset. As workaround we change the
        // indents to move and scale the text block. The needed shifts are calculated here as moTextOff
        // and used in TextBodyProperties::pushTextDistances().
        awt::Rectangle aPresetTextRectangle;
        if (!ConstructPresetTextRectangle(mrShape, aPresetTextRectangle))
            return nullptr; // faulty shape or text area calculation not implemented

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
                // Build text frame from txXfrm element
                awt::Rectangle aUnrotatedTxXfrm = aPresetTextRectangle; // dummy initialize
                const OUString sCXValue = rAttribs.getStringDefaulted(XML_cx);
                const OUString sCYValue = rAttribs.getStringDefaulted(XML_cy);
                if (!sCXValue.isEmpty() && !sCYValue.isEmpty())
                {
                    aUnrotatedTxXfrm.Width = sCXValue.toInt32();
                    aUnrotatedTxXfrm.Height = sCYValue.toInt32();
                }
                if (mno_txXfrmOffX.has_value() && mno_txXfrmOffY.has_value())
                {
                    aUnrotatedTxXfrm.X = mno_txXfrmOffX.value();
                    aUnrotatedTxXfrm.Y = mno_txXfrmOffY.value();
                }

                // Has the txXfrm an own rotation beyond compensation of the shape rotation?
                // Happens e.g. in diagram type 'Detailed Process'.
                sal_Int32 nAngleDiff
                    = (mrShape.getRotation() + mno_txXfrmRot.value_or(0)) % 21600000;
                if (nAngleDiff != 0)
                {
                    // Rectangle aUnrotatedTxXfrm rotates around its center not around text area
                    // center from preset. We shift aUnrotatedTxXfrm so that it is at the original
                    // position after rotation of text area rectangle from preset.
                    basegfx::B2DPoint aXfrmCenter(getCenter(aUnrotatedTxXfrm));
                    basegfx::B2DPoint aPresetCenter(getCenter(aPresetTextRectangle));

                    if (!aXfrmCenter.equal(aPresetCenter))
                    {
                        double fAngleRad = basegfx::deg2rad(nAngleDiff / 60000.0);
                        basegfx::B2DHomMatrix aRotMatrix(
                            basegfx::utils::createRotateAroundPoint(aPresetCenter, -fAngleRad));
                        basegfx::B2DPoint aNewCenter(aRotMatrix * aXfrmCenter);
                        aUnrotatedTxXfrm.X += aNewCenter.getX() - aXfrmCenter.getX();
                        aUnrotatedTxXfrm.Y += aNewCenter.getY() - aXfrmCenter.getY();
                    }
                }

                if(mrShape.getTextBody())
                {
                    // Calculate indent offsets
                    sal_Int32 nOffsetLeft = aUnrotatedTxXfrm.X - aPresetTextRectangle.X;
                    sal_Int32 nOffsetTop = aUnrotatedTxXfrm.Y - aPresetTextRectangle.Y;
                    sal_Int32 nOffsetRight
                        = aPresetTextRectangle.Width - aUnrotatedTxXfrm.Width - nOffsetLeft;
                    sal_Int32 nOffsetBottom
                        = aPresetTextRectangle.Height - aUnrotatedTxXfrm.Height - nOffsetTop;

                    if (nOffsetLeft)
                        mrShape.getTextBody()->getTextProperties().moTextOffLeft
                            = GetCoordinate(nOffsetLeft);
                    if (nOffsetTop)
                        mrShape.getTextBody()->getTextProperties().moTextOffUpper
                            = GetCoordinate(nOffsetTop);
                    if (nOffsetRight)
                        mrShape.getTextBody()->getTextProperties().moTextOffRight
                            = GetCoordinate(nOffsetRight);
                    if (nOffsetBottom)
                        mrShape.getTextBody()->getTextProperties().moTextOffLower
                            = GetCoordinate(nOffsetBottom);
                }
            }
            break;
        }
        return nullptr;
    } // end of case mbtxXfrm

    switch( aElementToken )
    {
    case A_TOKEN( off ):        // horz/vert translation
        mrShape.setPosition( awt::Point( rAttribs.getInteger( XML_x, 0 ), rAttribs.getInteger( XML_y, 0 ) ) );
        break;
    case A_TOKEN( ext ):        // horz/vert size
        mrShape.setSize( awt::Size( rAttribs.getInteger( XML_cx, 0 ), rAttribs.getInteger( XML_cy, 0 ) ) );
        break;
    case A_TOKEN( chOff ):  // horz/vert translation of children
        mrShape.setChildPosition( awt::Point( rAttribs.getInteger( XML_x, 0 ), rAttribs.getInteger( XML_y, 0 ) ) );
        break;
    case A_TOKEN( chExt ):  // horz/vert size of children
        {
            sal_Int32 nChExtCx = rAttribs.getInteger(XML_cx, 0);

            if(nChExtCx == 0)
                nChExtCx = mrShape.getSize().Width;

            sal_Int32 nChExtCy = rAttribs.getInteger(XML_cy, 0);

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
