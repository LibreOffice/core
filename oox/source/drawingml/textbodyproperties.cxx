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

#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <drawingml/textbodyproperties.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <tools/gen.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdoashp.hxx>
#include <svx/sdtditm.hxx>

#include <array>

using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::text;
using namespace css;

namespace oox::drawingml {

TextBodyProperties::TextBodyProperties()
    : mbAnchorCtr(false)
    , meVA( TextVerticalAdjust_TOP )
{
}

/* For Legacy purposes: TODO: Check if it is required at all! */
void TextBodyProperties::pushVertSimulation()
{
    sal_Int32 tVert = moVert.get( XML_horz );
    if( !(tVert == XML_vert || tVert == XML_eaVert || tVert == XML_vert270 || tVert == XML_mongolianVert) )
        return;

    // #160799# fake different vertical text modes by top-bottom writing mode
    maPropertyMap.setProperty( PROP_TextWritingMode, WritingMode_TB_RL);

    // workaround for TB_LR as using WritingMode2 doesn't work
    if( meVA != TextVerticalAdjust_CENTER )
        maPropertyMap.setProperty( PROP_TextHorizontalAdjust,
                            (tVert == XML_vert270) ? TextHorizontalAdjust_RIGHT : TextHorizontalAdjust_LEFT);
    if( tVert == XML_vert270 )
        maPropertyMap.setProperty( PROP_TextVerticalAdjust, TextVerticalAdjust_BOTTOM);
    if( ( tVert == XML_vert && meVA == TextVerticalAdjust_TOP ) ||
        ( tVert == XML_vert270 && meVA == TextVerticalAdjust_BOTTOM ) )
        maPropertyMap.setProperty( PROP_TextHorizontalAdjust, TextHorizontalAdjust_RIGHT);
    else if( meVA == TextVerticalAdjust_CENTER )
        maPropertyMap.setProperty( PROP_TextHorizontalAdjust, TextHorizontalAdjust_CENTER);
}

/* Push text distances / insets, taking into consideration Shape Rotation */
void TextBodyProperties::pushTextDistances(Size const& rTextAreaSize)
{
    for (auto & rValue : maTextDistanceValues)
        rValue.reset();

    sal_Int32 nOff = 0;
    static constexpr const std::array<sal_Int32, 4> aProps {
        PROP_TextLeftDistance,
        PROP_TextUpperDistance,
        PROP_TextRightDistance,
        PROP_TextLowerDistance
    };

    switch (moRotation.get(0))
    {
        case 90*1*60000: nOff = 3; break;
        case 90*2*60000: nOff = 2; break;
        case 90*3*60000: nOff = 1; break;
        default: break;
    }

    for (size_t i = 0; i < aProps.size(); i++)
    {
        sal_Int32 nVal = 0;

        // Hack for n#760986
        // TODO: Preferred method would be to have a textbox on top
        // of the shape and the place it according to the (off,ext)
        if (nOff == 0 && moTextOffLeft)
            nVal = *moTextOffLeft;

        if (nOff == 1 && moTextOffUpper)
            nVal = *moTextOffUpper;


        if (nOff == 2 && moTextOffRight)
            nVal = *moTextOffRight;

        if (nOff == 3 && moTextOffLower)
            nVal = *moTextOffLower;


        if( nVal < 0 )
            nVal = 0;

        sal_Int32 nTextOffsetValue = nVal;

        if (moInsets[i])
        {
            nTextOffsetValue = *moInsets[i] + nVal;
        }

        // if inset is set, then always set the value
        // this prevents the default to be set (0 is a valid value)
        if (moInsets[i] || nTextOffsetValue)
        {
            maTextDistanceValues[nOff] = nTextOffsetValue;
        }

        nOff = (nOff + 1) % aProps.size();
    }

    // Check if bottom and top are set
    if (maTextDistanceValues[1] && maTextDistanceValues[3])
    {
        double nHeight = rTextAreaSize.getHeight();

        double nTop = *maTextDistanceValues[1];
        double nBottom = *maTextDistanceValues[3];

        // Check if top + bottom is more than text area height.
        // If yes, we need to adjust the values as defined in OOXML.
        if (nTop + nBottom >= nHeight)
        {
            double diffFactor = (nTop + nBottom - nHeight) / 2.0;

            maTextDistanceValues[1] = nTop - diffFactor;
            maTextDistanceValues[3] = nBottom - diffFactor;
        }
    }

    for (size_t i = 0; i < aProps.size(); i++)
    {
        if (maTextDistanceValues[i])
            maPropertyMap.setProperty(aProps[i], *maTextDistanceValues[i]);
    }
}

/* Readjust the text distances / insets if necessary to take
   the text area into account, not just the shape area*/
void TextBodyProperties::readjustTextDistances(uno::Reference<drawing::XShape> const& xShape)
{
    // Only for custom shapes (for now)
    auto* pCustomShape = dynamic_cast<SdrObjCustomShape*>(SdrObject::getSdrObjectFromXShape(xShape));
    if (pCustomShape)
    {
        sal_Int32 nLower = pCustomShape->GetTextLowerDistance();
        sal_Int32 nUpper = pCustomShape->GetTextUpperDistance();

        pCustomShape->SetMergedItem(makeSdrTextUpperDistItem(0));
        pCustomShape->SetMergedItem(makeSdrTextLowerDistItem(0));

        tools::Rectangle aAnchorRect;
        pCustomShape->TakeTextAnchorRect(aAnchorRect);
        Size aAnchorSize = aAnchorRect.GetSize();

        pushTextDistances(aAnchorSize);
        if (maTextDistanceValues[1] && maTextDistanceValues[3])
        {
            nLower = *maTextDistanceValues[3];
            nUpper = *maTextDistanceValues[1];
        }

        pCustomShape->SetMergedItem(makeSdrTextLowerDistItem(nLower));
        pCustomShape->SetMergedItem(makeSdrTextUpperDistItem(nUpper));
    }
}


} // namespace oox::drawingml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
