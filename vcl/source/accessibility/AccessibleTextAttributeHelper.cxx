/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <vcl/accessibility/AccessibleTextAttributeHelper.hxx>

#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/accessibility/XAccessibleTextMarkup.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/TextMarkupType.hpp>
#include <o3tl/any.hxx>
#include <tools/color.hxx>

namespace
{
OUString lcl_ConvertCharEscapement(sal_Int16 nEscapement)
{
    if (nEscapement > 0)
        return u"super"_ustr;
    if (nEscapement < 0)
        return u"sub"_ustr;

    return u"baseline"_ustr;
}

OUString lcl_ConverCharStrikeout(sal_Int16 nStrikeout)
{
    OUString sTextLineThroughStyle;
    OUString sTextLineThroughText;
    OUString sTextLineThroughType;
    OUString sTextLineThroughWidth;

    switch (nStrikeout)
    {
        case css::awt::FontStrikeout::BOLD:
            sTextLineThroughType = "single";
            sTextLineThroughWidth = "bold";
            break;
        case css::awt::FontStrikeout::DONTKNOW:
            break;
        case css::awt::FontStrikeout::DOUBLE:
            sTextLineThroughType = "double";
            break;
        case css::awt::FontStrikeout::NONE:
            sTextLineThroughStyle = "none";
            break;
        case css::awt::FontStrikeout::SINGLE:
            sTextLineThroughType = "single";
            break;
        case css::awt::FontStrikeout::SLASH:
            sTextLineThroughText = u"/"_ustr;
            break;
        case css::awt::FontStrikeout::X:
            sTextLineThroughText = u"X"_ustr;
            break;
        default:
            assert(false && "Unhandled strikeout type");
    }

    OUString sResult;
    if (!sTextLineThroughStyle.isEmpty())
        sResult += u"text-line-through-style:"_ustr + sTextLineThroughStyle + ";";
    if (!sTextLineThroughText.isEmpty())
        sResult += u"text-line-through-text:"_ustr + sTextLineThroughText + ";";
    if (!sTextLineThroughType.isEmpty())
        sResult += u"text-line-through-type:"_ustr + sTextLineThroughType + ";";
    if (!sTextLineThroughWidth.isEmpty())
        sResult += u"text-line-through-width:"_ustr + sTextLineThroughWidth + ";";

    return sResult;
}

OUString lcl_convertFontWeight(double fontWeight)
{
    if (fontWeight == css::awt::FontWeight::THIN || fontWeight == css::awt::FontWeight::ULTRALIGHT)
        return u"100"_ustr;
    if (fontWeight == css::awt::FontWeight::LIGHT)
        return u"200"_ustr;
    if (fontWeight == css::awt::FontWeight::SEMILIGHT)
        return u"300"_ustr;
    if (fontWeight == css::awt::FontWeight::NORMAL)
        return u"normal"_ustr;
    if (fontWeight == css::awt::FontWeight::SEMIBOLD)
        return u"500"_ustr;
    if (fontWeight == css::awt::FontWeight::BOLD)
        return u"bold"_ustr;
    if (fontWeight == css::awt::FontWeight::ULTRABOLD)
        return u"800"_ustr;
    if (fontWeight == css::awt::FontWeight::BLACK)
        return u"900"_ustr;

    // awt::FontWeight::DONTKNOW || fontWeight == awt::FontWeight::NORMAL
    return u"normal"_ustr;
}

OUString lcl_ConvertFontSlant(css::awt::FontSlant eFontSlant)
{
    switch (eFontSlant)
    {
        case css::awt::FontSlant::FontSlant_NONE:
            return u"normal"_ustr;
        case css::awt::FontSlant::FontSlant_OBLIQUE:
        case css::awt::FontSlant::FontSlant_REVERSE_OBLIQUE:
            return u"oblique"_ustr;
        case css::awt::FontSlant::FontSlant_ITALIC:
        case css::awt::FontSlant::FontSlant_REVERSE_ITALIC:
            return u"italic"_ustr;
        case css::awt::FontSlant::FontSlant_DONTKNOW:
        case css::awt::FontSlant::FontSlant_MAKE_FIXED_SIZE:
        default:
            return u""_ustr;
    }
}

// s. https://wiki.linuxfoundation.org/accessibility/iaccessible2/textattributes
// for values
void lcl_ConvertFontUnderline(sal_Int16 nFontUnderline, OUString& rUnderlineStyle,
                              OUString& rUnderlineType, OUString& rUnderlineWidth)
{
    rUnderlineStyle = u""_ustr;
    rUnderlineType = u"single"_ustr;
    rUnderlineWidth = u"auto"_ustr;

    switch (nFontUnderline)
    {
        case css::awt::FontUnderline::BOLD:
            rUnderlineWidth = u"bold"_ustr;
            return;
        case css::awt::FontUnderline::BOLDDASH:
            rUnderlineWidth = u"bold"_ustr;
            rUnderlineStyle = u"dash"_ustr;
            return;
        case css::awt::FontUnderline::BOLDDASHDOT:
            rUnderlineWidth = u"bold"_ustr;
            rUnderlineStyle = u"dot-dash"_ustr;
            return;
        case css::awt::FontUnderline::BOLDDASHDOTDOT:
            rUnderlineWidth = u"bold"_ustr;
            rUnderlineStyle = u"dot-dot-dash"_ustr;
            return;
        case css::awt::FontUnderline::BOLDDOTTED:
            rUnderlineWidth = u"bold"_ustr;
            rUnderlineStyle = u"dotted"_ustr;
            return;
        case css::awt::FontUnderline::BOLDLONGDASH:
            rUnderlineWidth = u"bold"_ustr;
            rUnderlineStyle = u"long-dash"_ustr;
            return;
        case css::awt::FontUnderline::BOLDWAVE:
            rUnderlineWidth = u"bold"_ustr;
            rUnderlineStyle = u"wave"_ustr;
            return;
        case css::awt::FontUnderline::DASH:
            rUnderlineStyle = u"dash"_ustr;
            return;
        case css::awt::FontUnderline::DASHDOT:
            rUnderlineStyle = u"dot-dash"_ustr;
            return;
        case css::awt::FontUnderline::DASHDOTDOT:
            rUnderlineStyle = u"dot-dot-dash"_ustr;
            return;
        case css::awt::FontUnderline::DONTKNOW:
            rUnderlineWidth = u""_ustr;
            rUnderlineStyle = u""_ustr;
            rUnderlineType = u""_ustr;
            return;
        case css::awt::FontUnderline::DOTTED:
            rUnderlineStyle = u"dotted"_ustr;
            return;
        case css::awt::FontUnderline::DOUBLE:
            rUnderlineType = u"double"_ustr;
            return;
        case css::awt::FontUnderline::DOUBLEWAVE:
            rUnderlineStyle = u"wave"_ustr;
            rUnderlineType = u"double"_ustr;
            return;
        case css::awt::FontUnderline::LONGDASH:
            rUnderlineStyle = u"long-dash"_ustr;
            return;
        case css::awt::FontUnderline::NONE:
            rUnderlineWidth = u"none"_ustr;
            rUnderlineStyle = u"none"_ustr;
            rUnderlineType = u"none"_ustr;
            return;
        case css::awt::FontUnderline::SINGLE:
            rUnderlineType = u"single"_ustr;
            return;
        case css::awt::FontUnderline::SMALLWAVE:
        case css::awt::FontUnderline::WAVE:
            rUnderlineStyle = u"wave"_ustr;
            return;
        default:
            assert(false && "Unhandled font underline type");
    }
}

/** Converts Color to "rgb(r,g,b)" as specified in https://wiki.linuxfoundation.org/accessibility/iaccessible2/textattributes. */
OUString lcl_ConvertColor(Color aColor)
{
    return u"rgb(" + OUString::number(aColor.GetRed()) + u"\\,"
           + OUString::number(aColor.GetGreen()) + u"\\," + OUString::number(aColor.GetBlue())
           + u")";
}

OUString lcl_ConvertParagraphAdjust(css::style::ParagraphAdjust eParaAdjust)
{
    switch (eParaAdjust)
    {
        case css::style::ParagraphAdjust_LEFT:
            return u"left"_ustr;
        case css::style::ParagraphAdjust_RIGHT:
            return u"right"_ustr;
        case css::style::ParagraphAdjust_BLOCK:
        case css::style::ParagraphAdjust_STRETCH:
            return u"justify"_ustr;
        case css::style::ParagraphAdjust_CENTER:
            return u"center"_ustr;
        default:
            assert(false && "Unhandled ParagraphAdjust value");
            return u""_ustr;
    }
}
}

static OUString ConvertUnoToIAccessible2TextAttributes(
    const css::uno::Sequence<css::beans::PropertyValue>& rUnoAttributes,
    IA2AttributeType eAttributeType)
{
    OUString aRet;
    for (css::beans::PropertyValue const& prop : rUnoAttributes)
    {
        OUString sAttribute;
        OUString sValue;

        if (eAttributeType & IA2AttributeType::TextAttributes)
        {
            if (prop.Name == "CharBackColor")
            {
                sAttribute = "background-color";
                sValue = lcl_ConvertColor(
                    Color(ColorTransparency, *o3tl::doAccess<sal_Int32>(prop.Value)));
            }
            else if (prop.Name == "CharColor")
            {
                sAttribute = "color";
                sValue = lcl_ConvertColor(
                    Color(ColorTransparency, *o3tl::doAccess<sal_Int32>(prop.Value)));
            }
            else if (prop.Name == "CharEscapement")
            {
                sAttribute = "text-position";
                const sal_Int16 nEscapement = *o3tl::doAccess<sal_Int16>(prop.Value);
                sValue = lcl_ConvertCharEscapement(nEscapement);
            }
            else if (prop.Name == "CharFontName")
            {
                sAttribute = "font-family";
                sValue = *o3tl::doAccess<OUString>(prop.Value);
            }
            else if (prop.Name == "CharHeight")
            {
                sAttribute = "font-size";
                sValue = OUString::number(*o3tl::doAccess<double>(prop.Value)) + "pt";
            }
            else if (prop.Name == "CharPosture")
            {
                sAttribute = "font-style";
                const css::awt::FontSlant eFontSlant
                    = *o3tl::doAccess<css::awt::FontSlant>(prop.Value);
                sValue = lcl_ConvertFontSlant(eFontSlant);
            }
            else if (prop.Name == "CharStrikeout")
            {
                const sal_Int16 nStrikeout = *o3tl::doAccess<sal_Int16>(prop.Value);
                aRet += lcl_ConverCharStrikeout(nStrikeout);
            }
            else if (prop.Name == "CharUnderline")
            {
                OUString sUnderlineStyle;
                OUString sUnderlineType;
                OUString sUnderlineWidth;
                const sal_Int16 nUnderline = *o3tl::doAccess<sal_Int16>(prop.Value);
                lcl_ConvertFontUnderline(nUnderline, sUnderlineStyle, sUnderlineType,
                                         sUnderlineWidth);

                // leave 'sAttribute' and 'sName' empty, set all attributes here
                if (!sUnderlineStyle.isEmpty())
                    aRet += u"text-underline-style:" + sUnderlineStyle + ";";
                if (!sUnderlineType.isEmpty())
                    aRet += u"text-underline-type:" + sUnderlineType + ";";
                if (!sUnderlineWidth.isEmpty())
                    aRet += u"text-underline-width:" + sUnderlineWidth + ";";
            }
            else if (prop.Name == "CharWeight")
            {
                sAttribute = "font-weight";
                sValue = lcl_convertFontWeight(*o3tl::doAccess<double>(prop.Value));
            }
        }

        // so far, "ParaAdjust" is the only UNO text attribute that
        // maps to an object attribute for IAccessible2 ("text-align")
        if (sAttribute.isEmpty() && (eAttributeType & IA2AttributeType::ObjectAttributes)
            && prop.Name == "ParaAdjust")
        {
            sAttribute = "text-align";
            const css::style::ParagraphAdjust eParaAdjust
                = static_cast<css::style::ParagraphAdjust>(*o3tl::doAccess<sal_Int16>(prop.Value));
            sValue = lcl_ConvertParagraphAdjust(eParaAdjust);
        }

        if (!sAttribute.isEmpty() && !sValue.isEmpty())
            aRet += sAttribute + ":" + sValue + ";";
    }

    return aRet;
}

OUString AccessibleTextAttributeHelper::GetIAccessible2TextAttributes(
    const css::uno::Reference<css::accessibility::XAccessibleText>& xText,
    IA2AttributeType eAttributeType, sal_Int32 nOffset, sal_Int32& rStartOffset,
    sal_Int32& rEndOffset)
{
    assert(xText.is());

    const css::uno::Sequence<css::beans::PropertyValue> attribs
        = xText->getCharacterAttributes(nOffset, css::uno::Sequence<OUString>());
    OUString sAttributes = ConvertUnoToIAccessible2TextAttributes(attribs, eAttributeType);

    css::accessibility::TextSegment aAttributeRun
        = xText->getTextAtIndex(nOffset, css::accessibility::AccessibleTextType::ATTRIBUTE_RUN);
    rStartOffset = aAttributeRun.SegmentStart;
    rEndOffset = aAttributeRun.SegmentEnd;

    // report spelling error as "invalid:spelling;" IA2 text attribute and
    // adapt start/end index as necessary
    css::uno::Reference<css::accessibility::XAccessibleTextMarkup> xTextMarkup(xText,
                                                                               css::uno::UNO_QUERY);
    if ((eAttributeType & IA2AttributeType::TextAttributes) && xTextMarkup.is())
    {
        bool bInvalidSpelling = false;
        const sal_Int32 nMarkupCount(
            xTextMarkup->getTextMarkupCount(css::text::TextMarkupType::SPELLCHECK));
        for (sal_Int32 nMarkupIndex = 0; nMarkupIndex < nMarkupCount; ++nMarkupIndex)
        {
            const css::accessibility::TextSegment aTextSegment
                = xTextMarkup->getTextMarkup(nMarkupIndex, css::text::TextMarkupType::SPELLCHECK);
            const sal_Int32 nStartOffsetTextMarkup = aTextSegment.SegmentStart;
            const sal_Int32 nEndOffsetTextMarkup = aTextSegment.SegmentEnd;
            if (nStartOffsetTextMarkup <= nOffset)
            {
                if (nOffset < nEndOffsetTextMarkup)
                {
                    // offset is inside invalid spelling
                    rStartOffset = ::std::max(rStartOffset, nStartOffsetTextMarkup);
                    rEndOffset = ::std::min(rEndOffset, nEndOffsetTextMarkup);
                    bInvalidSpelling = true;
                    break;
                }
                else
                {
                    rStartOffset = ::std::max(rStartOffset, nEndOffsetTextMarkup);
                }
            }
            else
            {
                rEndOffset = ::std::min(rEndOffset, nStartOffsetTextMarkup);
            }
        }

        if (bInvalidSpelling)
            sAttributes += u"invalid:spelling;"_ustr;
    }

    return sAttributes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
