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

#include <oox/vml/vmltextbox.hxx>

#include <rtl/ustrbuf.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/text/XTextAppend.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <utility>

namespace oox::vml {

using namespace com::sun::star;

TextFontModel::TextFontModel()
{
}

TextPortionModel::TextPortionModel( TextParagraphModel aParagraph, TextFontModel aFont, OUString aText ) :
    maParagraph(std::move( aParagraph )),
    maFont(std::move( aFont )),
    maText(std::move( aText ))
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
    maPortions.emplace_back( rParagraph, rFont, rText );
}

const TextFontModel* TextBox::getFirstFont() const
{
    return maPortions.empty() ? nullptr : &maPortions.front().maFont;
}

OUString TextBox::getText() const
{
    OUStringBuffer aBuffer;
    for (auto const& portion : maPortions)
        aBuffer.append( portion.maText );
    return aBuffer.makeStringAndClear();
}

void TextBox::convert(const uno::Reference<drawing::XShape>& xShape) const
{
    uno::Reference<text::XTextAppend> xTextAppend(xShape, uno::UNO_QUERY);
    OUString sParaStyle;
    bool bAmbiguousStyle = true;

    for (auto const& portion : maPortions)
    {
        beans::PropertyValue aPropertyValue;
        std::vector<beans::PropertyValue> aPropVec;
        const TextParagraphModel& rParagraph = portion.maParagraph;
        const TextFontModel& rFont = portion.maFont;
        if (rFont.moName.has_value())
        {
            aPropertyValue.Name = "CharFontName";
            aPropertyValue.Value <<= rFont.moName.value();
            aPropVec.push_back(aPropertyValue);

            aPropertyValue.Name = "CharFontNameAsian";
            aPropertyValue.Value <<= rFont.moNameAsian.value_or("");
            aPropVec.push_back(aPropertyValue);

            aPropertyValue.Name = "CharFontNameComplex";
            aPropertyValue.Value <<= rFont.moNameComplex.value_or("");
            aPropVec.push_back(aPropertyValue);
        }
        if (rFont.mobBold.has_value())
        {
            aPropertyValue.Name = "CharWeight";
            aPropertyValue.Value <<= rFont.mobBold.value() ? awt::FontWeight::BOLD : awt::FontWeight::NORMAL;
            aPropVec.push_back(aPropertyValue);
        }
        if (rFont.monSize.has_value())
        {
            aPropertyValue.Name = "CharHeight";
            aPropertyValue.Value <<= double(rFont.monSize.value()) / 2.;
            aPropVec.push_back(aPropertyValue);
        }
        if (rFont.monSpacing.has_value())
        {
            aPropertyValue.Name = "CharKerning";
            // Value is not converted to mm100: SvxKerningItem::PutValue() gets
            // called with nMemberId = 0, so no mm100 -> twips conversion will
            // be done there.
            aPropertyValue.Value <<= sal_Int16(rFont.monSpacing.value());
            aPropVec.push_back(aPropertyValue);
        }
        if (rParagraph.moParaAdjust.has_value())
        {
            style::ParagraphAdjust eAdjust = style::ParagraphAdjust_LEFT;
            if (rParagraph.moParaAdjust.value() == "center")
                eAdjust = style::ParagraphAdjust_CENTER;
            else if (rParagraph.moParaAdjust.value() == "right")
                eAdjust = style::ParagraphAdjust_RIGHT;

            aPropertyValue.Name = "ParaAdjust";
            aPropertyValue.Value <<= eAdjust;
            aPropVec.push_back(aPropertyValue);
        }

        // All paragraphs should be either undefined (default) or the same style,
        // because it will only  be applied to the entire shape, and not per-paragraph.
        if (sParaStyle.isEmpty() )
        {
            if ( rParagraph.moParaStyleName.has_value() )
                sParaStyle = rParagraph.moParaStyleName.value();
            if ( bAmbiguousStyle )
                bAmbiguousStyle = false; // both empty parastyle and ambiguous can only be true at the first paragraph
            else
                bAmbiguousStyle = rParagraph.moParaStyleName.has_value(); // ambiguous if both default and specified style used.
        }
        else if ( !bAmbiguousStyle )
        {
            if ( !rParagraph.moParaStyleName.has_value() )
                bAmbiguousStyle = true; // ambiguous if both specified and default style used.
            else if ( rParagraph.moParaStyleName.value() != sParaStyle )
                bAmbiguousStyle = true; // ambiguous if two different styles specified.
        }
        if (rFont.moColor.has_value())
        {
            aPropertyValue.Name = "CharColor";
            aPropertyValue.Value <<= rFont.moColor.value().toUInt32(16);
            aPropVec.push_back(aPropertyValue);
        }
        xTextAppend->appendTextPortion(portion.maText, comphelper::containerToSequence(aPropVec));
    }

    try
    {
        // Track the style in a grabBag for use later when style details are known.
        comphelper::SequenceAsHashMap aGrabBag;
        uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY_THROW);
        aGrabBag.update( xPropertySet->getPropertyValue(u"CharInteropGrabBag"_ustr) );
        aGrabBag[u"mso-pStyle"_ustr] <<= sParaStyle;
        xPropertySet->setPropertyValue(u"CharInteropGrabBag"_ustr, uno::Any(aGrabBag.getAsConstPropertyValueList()));
    }
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION( "oox.vml","convert() grabbag exception" );
    }

    // Remove the last character of the shape text, if it would be a newline.
    uno::Reference< text::XTextCursor > xCursor = xTextAppend->createTextCursor();
    xCursor->gotoEnd(false);
    xCursor->goLeft(1, true);
    if (xCursor->getString() == "\n")
        xCursor->setString(u""_ustr);

    if ( maLayoutFlow != "vertical" )
        return;

    uno::Reference<beans::XPropertySet> xProperties(xShape, uno::UNO_QUERY);

    // VML has the text horizontally aligned to left (all the time),
    // v-text-anchor for vertical alignment, and vertical mode to swap the
    // two.  drawinglayer supports both horizontal and vertical alignment,
    // but no vertical mode: we use T->B, R->L instead.
    // As a result, we need to set horizontal adjustment here to 'right',
    // that will result in vertical 'top' after writing mode is applied,
    // which matches the VML behavior.
    xProperties->setPropertyValue(u"TextHorizontalAdjust"_ustr, uno::Any(drawing::TextHorizontalAdjust_RIGHT));

    xProperties->setPropertyValue( u"TextWritingMode"_ustr, uno::Any( text::WritingMode_TB_RL ) );
}

} // namespace oox::vml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
