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
            sal_Int32 nVertRot = mrShape.getTextBody()->getTextProperties().moRotation.value_or(0);
            // And need to know the txXfrm rotation for adapting insets
            sal_Int32 nRot = rAttribs.getInteger(XML_rot, 0);
            mrShape.getTextBody()->getTextProperties().moRotation = nRot + nVertRot;

            // Adapt insets. The needed offset depends on nVertRot and nRot.
            // normalize angles
            while (nRot < 0)
                nRot += 21600000;
            while (nRot >= 21600000)
                nRot -= 21600000;
            while (nVertRot < 0)
                nVertRot += 21600000;
            while (nRot >= 21600000)
                nVertRot -= 21600000;
            // determine round shift offset
            sal_Int32 nOff(0);
            if (nRot == 16200000 && (nVertRot == 16200000 || nVertRot == 5400000))
                nOff = 3;
            else if (nRot == 5400000)
                nOff = 1;
            else if (nRot == 10800000 && nVertRot == 16200000)
                nOff = 1; // Error in PowerPoint ?
            else if (nRot == 10800000)
                nOff = 2;
            // move the insets values
            std::array<sal_Int32, 4> aOldInsets;
            for (size_t i = 0; i < 4; i++)
                aOldInsets[i] = mrShape.getTextBody()->getTextProperties().moInsets[i].value_or(0);
            for (size_t i = 0; i < 4; i++)
            {
                mrShape.getTextBody()->getTextProperties().moInsets[nOff] = aOldInsets[i];
                nOff = (nOff + 1) % 4;
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
        case XML_rect:
        case XML_pie:
            // When tdf#149918 is fixed, pie will need its own case
            rRect.X = rShape.getPosition().X;
            rRect.Y = rShape.getPosition().Y;
            rRect.Width = rShape.getSize().Width;
            rRect.Height = rShape.getSize().Height;
            return true;
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
        default:
            return false;
    }
}
}

ContextHandlerRef Transform2DContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    if( mbtxXfrm )
    {
        // The child elements <a:off> and <a:ext> of a <dsp:txXfrm> element describe the position and
        // size of the text area rectangle. Often this is the same as the text area rectangle
        // specified for the type. Some known exception are handled here.
        // We cannot change the text area rectangle directly as that would disallow to use any preset
        // definition. As workaround we change the indents to move the text block. The needed shifts
        // are calculated here and used in TextBodyProperties::pushTextDistances().

        awt::Rectangle aPresetTextRectangle;
        if (!ConstructPresetTextRectangle(mrShape, aPresetTextRectangle))
            return nullptr;

        switch (aElementToken)
        {
            case A_TOKEN(off):
            {
                const OUString sXValue = rAttribs.getStringDefaulted(XML_x);
                const OUString sYValue = rAttribs.getStringDefaulted(XML_y);
                // only left and top, for right and bottom <a:ext> is needed
                if (!sXValue.isEmpty())
                    mrShape.getTextBody()->getTextProperties().moTextOffLeft
                        = GetCoordinate(sXValue.toInt32() - aPresetTextRectangle.X);

                if (!sYValue.isEmpty())
                    mrShape.getTextBody()->getTextProperties().moTextOffUpper
                        = GetCoordinate(sYValue.toInt32() - aPresetTextRectangle.Y);
            }
            break;
            case A_TOKEN(ext):
            {
                const OUString sCXValue = rAttribs.getStringDefaulted(XML_cx);
                const OUString sCYValue = rAttribs.getStringDefaulted(XML_cy);
                if (!sCXValue.isEmpty() && mrShape.getTextBody()->getTextProperties().moTextOffLeft)
                {
                    mrShape.getTextBody()->getTextProperties().moTextOffRight
                        = GetCoordinate(aPresetTextRectangle.Width - sCXValue.toInt32());
                    *mrShape.getTextBody()->getTextProperties().moTextOffRight
                        -= *mrShape.getTextBody()->getTextProperties().moTextOffLeft;
                }
                if (!sCYValue.isEmpty()
                    && mrShape.getTextBody()->getTextProperties().moTextOffUpper)
                {
                    mrShape.getTextBody()->getTextProperties().moTextOffLower
                        = GetCoordinate(aPresetTextRectangle.Height - sCYValue.toInt32());
                    *mrShape.getTextBody()->getTextProperties().moTextOffLower
                        -= *mrShape.getTextBody()->getTextProperties().moTextOffUpper;
                }
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
