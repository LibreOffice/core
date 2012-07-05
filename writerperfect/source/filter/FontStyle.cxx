/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* FontStyle: Stores (and writes) font-based information that is needed at
 * the head of an OO document.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information visit http://libwpd.sourceforge.net
 */

#include "FilterInternal.hxx"

#include "FontStyle.hxx"

#include "DocumentElement.hxx"

FontStyle::FontStyle(const char *psName, const char *psFontFamily) : Style(psName),
    msFontFamily(psFontFamily, true),
    msFontPitch(IMP_DEFAULT_FONT_PITCH)
{
}

FontStyle::~FontStyle()
{
}

void FontStyle::write(OdfDocumentHandler *pHandler) const
{
    TagOpenElement styleOpen("style:font-face");
    styleOpen.addAttribute("style:name", getName());
    styleOpen.addAttribute("svg:font-family", msFontFamily);
    styleOpen.write(pHandler);
    TagCloseElement styleClose("style:font-face");
    styleClose.write(pHandler);
}

void FontStyleManager::clean()
{
    mStyleHash.clear();
}

void FontStyleManager::writeFontsDeclaration(OdfDocumentHandler *pHandler) const
{
    TagOpenElement("office:font-face-decls").write(pHandler);
    std::map<WPXString, shared_ptr<FontStyle>, ltstr>::const_iterator iter;
    for (iter = mStyleHash.begin(); iter != mStyleHash.end(); ++iter)
    {
        (iter->second)->write(pHandler);
    }

    TagOpenElement symbolFontOpen("style:font-face");
    symbolFontOpen.addAttribute("style:name", "StarSymbol");
    symbolFontOpen.addAttribute("svg:font-family", "StarSymbol");
    symbolFontOpen.addAttribute("style:font-charset", "x-symbol");
    symbolFontOpen.write(pHandler);
    pHandler->endElement("style:font-face");

    pHandler->endElement("office:font-face-decls");
}

WPXString FontStyleManager::findOrAdd(const char *psFontFamily)
{
    std::map<WPXString, shared_ptr<FontStyle>, ltstr>::const_iterator iter =
        mStyleHash.find(psFontFamily);
    if (iter!=mStyleHash.end()) return psFontFamily;

    // ok create a new font
    shared_ptr<FontStyle> font(new FontStyle(psFontFamily, psFontFamily));
    mStyleHash[psFontFamily] = font;
    return psFontFamily;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
