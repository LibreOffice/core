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

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <o3tl/any.hxx>
#include <tools/color.hxx>

namespace
{
OUString lcl_convertFontWeight(double fontWeight)
{
    if (fontWeight == css::awt::FontWeight::THIN || fontWeight == css::awt::FontWeight::ULTRALIGHT)
        return "100";
    if (fontWeight == css::awt::FontWeight::LIGHT)
        return "200";
    if (fontWeight == css::awt::FontWeight::SEMILIGHT)
        return "300";
    if (fontWeight == css::awt::FontWeight::NORMAL)
        return "normal";
    if (fontWeight == css::awt::FontWeight::SEMIBOLD)
        return "500";
    if (fontWeight == css::awt::FontWeight::BOLD)
        return "bold";
    if (fontWeight == css::awt::FontWeight::ULTRABOLD)
        return "800";
    if (fontWeight == css::awt::FontWeight::BLACK)
        return "900";

    // awt::FontWeight::DONTKNOW || fontWeight == awt::FontWeight::NORMAL
    return "normal";
}

OUString lcl_ConvertFontSlant(css::awt::FontSlant eFontSlant)
{
    switch (eFontSlant)
    {
        case css::awt::FontSlant::FontSlant_NONE:
            return "normal";
        case css::awt::FontSlant::FontSlant_OBLIQUE:
        case css::awt::FontSlant::FontSlant_REVERSE_OBLIQUE:
            return "oblique";
        case css::awt::FontSlant::FontSlant_ITALIC:
        case css::awt::FontSlant::FontSlant_REVERSE_ITALIC:
            return "italic";
        case css::awt::FontSlant::FontSlant_DONTKNOW:
        case css::awt::FontSlant::FontSlant_MAKE_FIXED_SIZE:
        default:
            return "";
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
}

OUString AccessibleTextAttributeHelper::ConvertUnoToIAccessible2TextAttributes(
    const css::uno::Sequence<css::beans::PropertyValue>& rUnoAttributes)
{
    OUString aRet;
    for (css::beans::PropertyValue const& prop : rUnoAttributes)
    {
        OUString sAttribute;
        OUString sValue;
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
            const css::awt::FontSlant eFontSlant = *o3tl::doAccess<css::awt::FontSlant>(prop.Value);
            sValue = lcl_ConvertFontSlant(eFontSlant);
        }
        else if (prop.Name == "CharUnderline")
        {
            OUString sUnderlineStyle;
            OUString sUnderlineType;
            OUString sUnderlineWidth;
            const sal_Int16 nUnderline = *o3tl::doAccess<sal_Int16>(prop.Value);
            lcl_ConvertFontUnderline(nUnderline, sUnderlineStyle, sUnderlineType, sUnderlineWidth);

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

        if (!sAttribute.isEmpty() && !sValue.isEmpty())
            aRet += sAttribute + ":" + sValue + ";";
    }

    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
