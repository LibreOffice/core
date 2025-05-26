/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "EmbeddedFontListContext.hxx"

#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <sax/fshelper.hxx>

using namespace css;

namespace oox::ppt
{
EmbeddedFontListContext::EmbeddedFontListContext(
    FragmentHandler2 const& rParent, bool bEmbedTrueType,
    css::uno::Reference<css::beans::XPropertySet> const& rxDocSettings)
    : FragmentHandler2(rParent)
    , mbEmbedTrueType(bEmbedTrueType)
    , mxDocSettings(rxDocSettings)
{
}

EmbeddedFontListContext::~EmbeddedFontListContext() = default;

oox::core::ContextHandlerRef
EmbeddedFontListContext::onCreateContext(sal_Int32 aElementToken, const AttributeList& rAttributes)
{
    switch (aElementToken)
    {
        case PPT_TOKEN(embeddedFont):
        {
            moCurrentFont = EmbeddedFont();
            return this;
        }
        case PPT_TOKEN(font):
        {
            moCurrentFont->aTypeface = rAttributes.getStringDefaulted(XML_typeface);
            moCurrentFont->aPanose = rAttributes.getStringDefaulted(XML_panose);
            moCurrentFont->nPitchFamily = rAttributes.getUnsigned(XML_pitchFamily, 0);
            moCurrentFont->nCharset = rAttributes.getUnsigned(XML_charset, 0);
            return this;
        }
        case PPT_TOKEN(regular):
        {
            moCurrentFont->aRegularID = rAttributes.getStringDefaulted(R_TOKEN(id));
            return this;
        }
        case PPT_TOKEN(bold):
        {
            moCurrentFont->aBoldID = rAttributes.getStringDefaulted(R_TOKEN(id));
            return this;
        }
        case PPT_TOKEN(italic):
        {
            moCurrentFont->aItalicID = rAttributes.getStringDefaulted(R_TOKEN(id));
            return this;
        }
        case PPT_TOKEN(boldItalic):
        {
            moCurrentFont->aBoldItalicID = rAttributes.getStringDefaulted(R_TOKEN(id));
            return this;
        }
        default:
            break;
    }

    return this;
}

void EmbeddedFontListContext::onEndElement()
{
    if (!isCurrentElement(PPT_TOKEN(embeddedFont)))
        return;

    if (!mbEmbedTrueType || !moCurrentFont)
        return;

    if (mxDocSettings.is())
    {
        mxDocSettings->setPropertyValue(u"EmbedFonts"_ustr, uno::Any(true));
        mxDocSettings->setPropertyValue(u"EmbedOnlyUsedFonts"_ustr, uno::Any(true));
    }

    if (!moCurrentFont->aRegularID.isEmpty())
    {
        OUString aFragmentPath = getFragmentPathFromRelId(moCurrentFont->aRegularID);
        uno::Reference<io::XInputStream> xInputStream = getFilter().openInputStream(aFragmentPath);
        maEmbeddedFontHelper.addEmbeddedFont(xInputStream, moCurrentFont->aTypeface, u"",
                                             std::vector<unsigned char>(), true, false);
    }

    if (!moCurrentFont->aBoldID.isEmpty())
    {
        OUString aFragmentPath = getFragmentPathFromRelId(moCurrentFont->aBoldID);
        uno::Reference<io::XInputStream> xInputStream = getFilter().openInputStream(aFragmentPath);
        maEmbeddedFontHelper.addEmbeddedFont(xInputStream, moCurrentFont->aTypeface, u"b",
                                             std::vector<unsigned char>(), true, false);
    }

    if (!moCurrentFont->aItalicID.isEmpty())
    {
        OUString aFragmentPath = getFragmentPathFromRelId(moCurrentFont->aItalicID);
        uno::Reference<io::XInputStream> xInputStream = getFilter().openInputStream(aFragmentPath);
        maEmbeddedFontHelper.addEmbeddedFont(xInputStream, moCurrentFont->aTypeface, u"i",
                                             std::vector<unsigned char>(), true, false);
    }

    if (!moCurrentFont->aBoldItalicID.isEmpty())
    {
        OUString aFragmentPath = getFragmentPathFromRelId(moCurrentFont->aBoldItalicID);
        uno::Reference<io::XInputStream> xInputStream = getFilter().openInputStream(aFragmentPath);
        maEmbeddedFontHelper.addEmbeddedFont(xInputStream, moCurrentFont->aTypeface, u"bi",
                                             std::vector<unsigned char>(), true, false);
    }

    moCurrentFont = std::nullopt;
}

} // end oox::ppt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
