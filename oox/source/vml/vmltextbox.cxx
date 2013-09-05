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

#include "oox/vml/vmltextbox.hxx"

#include <rtl/ustrbuf.hxx>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/text/XTextAppend.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>

namespace oox {
namespace vml {

using namespace com::sun::star;

TextFontModel::TextFontModel()
{
}

TextPortionModel::TextPortionModel( const TextParagraphModel& rParagraph, const TextFontModel& rFont, const OUString& rText ) :
    maParagraph( rParagraph ),
    maFont( rFont ),
    maText( rText )
{
}

TextBox::TextBox(ShapeTypeModel& rTypeModel)
    : mrTypeModel(rTypeModel)
    , borderDistanceSet( false )
    , borderDistanceLeft(0)
    , borderDistanceTop(0)
    , borderDistanceRight(0)
    , borderDistanceBottom(0)
{
}

void TextBox::appendPortion( const TextParagraphModel& rParagraph, const TextFontModel& rFont, const OUString& rText )
{
    maPortions.push_back( TextPortionModel( rParagraph, rFont, rText ) );
}

const TextFontModel* TextBox::getFirstFont() const
{
    return maPortions.empty() ? 0 : &maPortions.front().maFont;
}

OUString TextBox::getText() const
{
    OUStringBuffer aBuffer;
    for( PortionVector::const_iterator aIt = maPortions.begin(), aEnd = maPortions.end(); aIt != aEnd; ++aIt )
        aBuffer.append( aIt->maText );
    return aBuffer.makeStringAndClear();
}

void TextBox::convert(uno::Reference<drawing::XShape> xShape) const
{
    uno::Reference<text::XTextAppend> xTextAppend(xShape, uno::UNO_QUERY);
    for (PortionVector::const_iterator aIt = maPortions.begin(), aEnd = maPortions.end(); aIt != aEnd; ++aIt)
    {
        beans::PropertyValue aPropertyValue;
        std::vector<beans::PropertyValue> aPropVec;
        const TextParagraphModel& rParagraph = aIt->maParagraph;
        const TextFontModel& rFont = aIt->maFont;
        if (rFont.mobBold.has())
        {
            aPropertyValue.Name = "CharWeight";
            aPropertyValue.Value = uno::makeAny(rFont.mobBold.get() ? awt::FontWeight::BOLD : awt::FontWeight::NORMAL);
            aPropVec.push_back(aPropertyValue);
        }
        if (rFont.monSize.has())
        {
            aPropertyValue.Name = "CharHeight";
            aPropertyValue.Value = uno::makeAny(double(rFont.monSize.get()) / 2.);
            aPropVec.push_back(aPropertyValue);
        }
        if (rParagraph.moParaAdjust.has())
        {
            style::ParagraphAdjust eAdjust = style::ParagraphAdjust_LEFT;
            if (rParagraph.moParaAdjust.get() == "center")
                eAdjust = style::ParagraphAdjust_CENTER;
            else if (rParagraph.moParaAdjust.get() == "right")
                eAdjust = style::ParagraphAdjust_RIGHT;

            aPropertyValue.Name = "ParaAdjust";
            aPropertyValue.Value = uno::makeAny(eAdjust);
            aPropVec.push_back(aPropertyValue);
        }
        if (rFont.moColor.has())
        {
            aPropertyValue.Name = "CharColor";
            aPropertyValue.Value = uno::makeAny(rFont.moColor.get().toUInt32(16));
            aPropVec.push_back(aPropertyValue);
        }
        uno::Sequence<beans::PropertyValue> aPropSeq(aPropVec.size());
        beans::PropertyValue* pValues = aPropSeq.getArray();
        for (std::vector<beans::PropertyValue>::iterator i = aPropVec.begin(); i != aPropVec.end(); ++i)
            *pValues++ = *i;
        xTextAppend->appendTextPortion(aIt->maText, aPropSeq);
    }

    // Remove the last character of the shape text, if it would be a newline.
    uno::Reference< text::XTextCursor > xCursor = xTextAppend->createTextCursor();
    xCursor->gotoEnd(false);
    xCursor->goLeft(1, true);
    if (xCursor->getString() == "\n")
        xCursor->setString("");

    if ( maLayoutFlow == "vertical" )
    {
        uno::Reference<beans::XPropertySet> xProperties(xShape, uno::UNO_QUERY);

        // VML has the text horizontally aligned to left (all the time),
        // v-text-anchor for vertical alignment, and vertical mode to swap the
        // two.  drawinglayer supports both horizontal and vertical alignment,
        // but no vertical mode: we use T->B, R->L instead.
        // As a result, we need to set horizontal adjustment here to 'right',
        // that will result in vertical 'top' after writing mode is applied,
        // which matches the VML behavior.
        xProperties->setPropertyValue("TextHorizontalAdjust", uno::makeAny(drawing::TextHorizontalAdjust_RIGHT));

        xProperties->setPropertyValue( "TextWritingMode", uno::makeAny( text::WritingMode_TB_RL ) );
    }
}

} // namespace vml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
