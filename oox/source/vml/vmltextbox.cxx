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
#include <com/sun/star/text/XTextAppend.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>

namespace oox {
namespace vml {

using namespace com::sun::star;

TextFontModel::TextFontModel()
{
}

TextPortionModel::TextPortionModel( const TextFontModel& rFont, const OUString& rText, const OUString& rJc ) :
    maFont( rFont ),
    maText( rText ),
    maJc( rJc )
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

void TextBox::appendPortion( const TextFontModel& rFont, const OUString& rText )
{
    maPortions.push_back( TextPortionModel( rFont, rText, maJc ) );
}

void TextBox::appendLineBreak()
{
    if (getPortionCount() <= 0)
        return;

    maPortions.push_back( TextPortionModel( maPortions.back().maFont, OUString("\x0A"), maJc ) );
}

void TextBox::setJustify(const OUString& rText)
{
    maJc = rText;
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

        if (rFont.moName.has())
        {
            aPropertyValue.Name = "CharFontName";
            aPropertyValue.Value = uno::makeAny(rFont.moName.get());
            aPropVec.push_back(aPropertyValue);
        }
        if (!aIt->maJc.isEmpty())
        {
            sal_Int16 adj = 0; // left - SVX_ADJUST_LEFT
            if (aIt->maJc == "right") // - SVX_ADJUST_RIGHT
                adj = 1;
            else if (aIt->maJc == "block") // - SVX_ADJUST_BLOCK
                adj = 2;
            else if (aIt->maJc == "center") // - SVX_ADJUST_CENTER
                adj = 3;
            else if (aIt->maJc == "blockline") // - SVX_ADJUST_BLOCKLINE
                adj = 4;
            aPropertyValue.Name = "ParaAdjust";
            aPropertyValue.Value = uno::makeAny(adj);
            aPropVec.push_back(aPropertyValue);
        }

        uno::Sequence<beans::PropertyValue> aPropSeq(aPropVec.size());
        beans::PropertyValue* pValues = aPropSeq.getArray();
        for (std::vector<beans::PropertyValue>::iterator i = aPropVec.begin(); i != aPropVec.end(); ++i)
            *pValues++ = *i;
        xTextAppend->appendTextPortion(aIt->maText, aPropSeq);
    }

    if ( maLayoutFlow == "vertical" )
    {
        uno::Reference<beans::XPropertySet> xProperties(xShape, uno::UNO_QUERY);
        xProperties->setPropertyValue( "TextWritingMode", uno::makeAny( text::WritingMode_TB_RL ) );
    }
}

} // namespace vml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
