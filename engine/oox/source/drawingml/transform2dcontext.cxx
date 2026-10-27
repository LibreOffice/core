/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <basegfx/vector/b2dvector.hxx>
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
    // we have neither xShape nor SdrObject here, the geometry of the preset gives the text area
    if (!rShape.getCustomShapeProperties())
        return false;
    const std::optional<awt::Rectangle> oArea(
        rShape.getCustomShapeProperties()->getPresetTextArea(rShape.getSize()));
    if (!oArea)
        return false;
    rRect = awt::Rectangle(rShape.getPosition().X + oArea->X, rShape.getPosition().Y + oArea->Y,
                           oArea->Width, oArea->Height);
    return true;
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

                // txXfrm is turned with the shape around the shape's centre, whatever its own turn
                // is, so we turn its centre back the same way
                if (mrShape.getRotation() % 21600000 != 0)
                {
                    const basegfx::B2DPoint aShapeCenter(
                        mrShape.getPosition().X + mrShape.getSize().Width / 2.0,
                        mrShape.getPosition().Y + mrShape.getSize().Height / 2.0);
                    const basegfx::B2DPoint aXfrmCenter(getCenter(aUnrotatedTxXfrm));
                    const double fAngleRad(basegfx::deg2rad(mrShape.getRotation() / 60000.0));
                    const basegfx::B2DPoint aNewCenter(
                        basegfx::utils::createRotateAroundPoint(aShapeCenter, -fAngleRad)
                        * aXfrmCenter);
                    aUnrotatedTxXfrm.X += aNewCenter.getX() - aXfrmCenter.getX();
                    aUnrotatedTxXfrm.Y += aNewCenter.getY() - aXfrmCenter.getY();
                }

                if(mrShape.getTextBody())
                {
                    // Calculate indent offsets. The indents are inside the text area turned by the
                    // own turn of txXfrm, so we turn the shift of the centre back into it first
                    const basegfx::B2DPoint aTextCenter(getCenter(aUnrotatedTxXfrm));
                    const basegfx::B2DPoint aPresetCenter(getCenter(aPresetTextRectangle));
                    const basegfx::B2DVector aShift(aTextCenter.getX() - aPresetCenter.getX(),
                                                    aTextCenter.getY() - aPresetCenter.getY());
                    const double fTextTurn(
                        basegfx::deg2rad(mno_txXfrmRot.value_or(0) / 60000.0));
                    const double fShiftX(aShift.getX() * std::cos(fTextTurn)
                                         + aShift.getY() * std::sin(fTextTurn));
                    const double fShiftY(-aShift.getX() * std::sin(fTextTurn)
                                         + aShift.getY() * std::cos(fTextTurn));
                    const double fFreeX((aPresetTextRectangle.Width - aUnrotatedTxXfrm.Width)
                                        / 2.0);
                    const double fFreeY((aPresetTextRectangle.Height - aUnrotatedTxXfrm.Height)
                                        / 2.0);
                    sal_Int32 nOffsetLeft = std::lround(fFreeX + fShiftX);
                    sal_Int32 nOffsetTop = std::lround(fFreeY + fShiftY);
                    sal_Int32 nOffsetRight = std::lround(fFreeX - fShiftX);
                    sal_Int32 nOffsetBottom = std::lround(fFreeY - fShiftY);

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
