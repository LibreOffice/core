/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ListStyle: Stores (and writes) list-based information that is
 * needed at the head of an OO document.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */
#include "FilterInternal.hxx"
#include "ListStyle.hxx"
#include "DocumentElement.hxx"

OrderedListLevelStyle::OrderedListLevelStyle(const WPXPropertyList &xPropList) :
    mPropList(xPropList)
{
}

void OrderedListLevelStyle::write(OdfDocumentHandler *pHandler, int iLevel) const
{
    WPXString sLevel;
    sLevel.sprintf("%i", (iLevel+1));

    TagOpenElement listLevelStyleOpen("text:list-level-style-number");
    listLevelStyleOpen.addAttribute("text:level", sLevel);
    listLevelStyleOpen.addAttribute("text:style-name", "Numbering_Symbols");
    if (mPropList["style:num-prefix"])
    {
        WPXString sEscapedString(mPropList["style:num-prefix"]->getStr(), true);
        listLevelStyleOpen.addAttribute("style:num-prefix", sEscapedString);
    }
    if (mPropList["style:num-suffix"])
    {
        WPXString sEscapedString(mPropList["style:num-suffix"]->getStr(), true);
        listLevelStyleOpen.addAttribute("style:num-suffix", sEscapedString);
    }
    if (mPropList["style:num-format"])
        listLevelStyleOpen.addAttribute("style:num-format", mPropList["style:num-format"]->getStr());
    if (mPropList["text:start-value"])
    {
        // ODF as to the version 1.1 does require the text:start-value to be a positive integer, means > 0
        if (mPropList["text:start-value"]->getInt() > 0)
            listLevelStyleOpen.addAttribute("text:start-value", mPropList["text:start-value"]->getStr());
        else
            listLevelStyleOpen.addAttribute("text:start-value", "1");
    }
    if (mPropList["text:display-levels"])
        listLevelStyleOpen.addAttribute("text:display-levels", mPropList["text:display-levels"]->getStr());
    listLevelStyleOpen.write(pHandler);

    TagOpenElement stylePropertiesOpen("style:list-level-properties");
    if (mPropList["text:space-before"] && mPropList["text:space-before"]->getDouble() > 0.0)
        stylePropertiesOpen.addAttribute("text:space-before", mPropList["text:space-before"]->getStr());
    if (mPropList["text:min-label-width"] && mPropList["text:min-label-width"]->getDouble() > 0.0)
        stylePropertiesOpen.addAttribute("text:min-label-width", mPropList["text:min-label-width"]->getStr());
    if (mPropList["text:min-label-distance"] && mPropList["text:min-label-distance"]->getDouble() > 0.0)
        stylePropertiesOpen.addAttribute("text:min-label-distance", mPropList["text:min-label-distance"]->getStr());
    if (mPropList["fo:text-align"])
        stylePropertiesOpen.addAttribute("fo:text-align", mPropList["fo:text-align"]->getStr());
    stylePropertiesOpen.write(pHandler);

    pHandler->endElement("style:list-level-properties");
    pHandler->endElement("text:list-level-style-number");
}

UnorderedListLevelStyle::UnorderedListLevelStyle(const WPXPropertyList &xPropList)
    : mPropList(xPropList)
{
}

void UnorderedListLevelStyle::write(OdfDocumentHandler *pHandler, int iLevel) const
{
    WPXString sLevel;
    sLevel.sprintf("%i", (iLevel+1));
    TagOpenElement listLevelStyleOpen("text:list-level-style-bullet");
    listLevelStyleOpen.addAttribute("text:level", sLevel);
    listLevelStyleOpen.addAttribute("text:style-name", "Bullet_Symbols");
    if (mPropList["text:bullet-char"] && (mPropList["text:bullet-char"]->getStr().len()))
    {
        // The following is needed because the ODF format does not accept bullet chars longer than one character
        WPXString::Iter i(mPropList["text:bullet-char"]->getStr());
        i.rewind();
        WPXString sEscapedString(".");
        if (i.next())
            sEscapedString = WPXString(i(), true);
        listLevelStyleOpen.addAttribute("text:bullet-char", sEscapedString);

    }
    else
        listLevelStyleOpen.addAttribute("text:bullet-char", ".");
    if (mPropList["text:display-levels"])
        listLevelStyleOpen.addAttribute("text:display-levels", mPropList["text:display-levels"]->getStr());
    listLevelStyleOpen.write(pHandler);

    TagOpenElement stylePropertiesOpen("style:list-level-properties");
    if (mPropList["text:space-before"] && mPropList["text:space-before"]->getDouble() > 0.0)
        stylePropertiesOpen.addAttribute("text:space-before", mPropList["text:space-before"]->getStr());
    if (mPropList["text:min-label-width"] && mPropList["text:min-label-width"]->getDouble() > 0.0)
        stylePropertiesOpen.addAttribute("text:min-label-width", mPropList["text:min-label-width"]->getStr());
    if (mPropList["text:min-label-distance"] && mPropList["text:min-label-distance"]->getDouble() > 0.0)
        stylePropertiesOpen.addAttribute("text:min-label-distance", mPropList["text:min-label-distance"]->getStr());
    if (mPropList["fo:text-align"])
        stylePropertiesOpen.addAttribute("fo:text-align", mPropList["fo:text-align"]->getStr());
    stylePropertiesOpen.addAttribute("style:font-name", "OpenSymbol");
    stylePropertiesOpen.write(pHandler);

    pHandler->endElement("style:list-level-properties");
    pHandler->endElement("text:list-level-style-bullet");
}

ListStyle::ListStyle(const char *psName, const int iListID) :
    Style(psName),
    miListID(iListID)
{
    for (int i=0; i<WP6_NUM_LIST_LEVELS; ++i)
        mppListLevels[i] = 0;

}

ListStyle::~ListStyle()
{
    for (int i=0; i<WP6_NUM_LIST_LEVELS; ++i)
    {
        if (mppListLevels[i])
            delete(mppListLevels[i]);
    }

}

bool ListStyle::isListLevelDefined(int iLevel) const
{
    if (!mppListLevels[iLevel])
        return false;

    return true;
}

void ListStyle::setListLevel(int iLevel, ListLevelStyle *iListLevelStyle)
{
    // can't uncomment this next line without adding some extra logic.
    // figure out which is best: use the initial message, or constantly
    // update?
    if (!mppListLevels[iLevel])
        mppListLevels[iLevel] = iListLevelStyle;
}

void ListStyle::updateListLevel(const int iLevel, const WPXPropertyList &xPropList, bool ordered)
{
    if (iLevel < 0)
        return;
    if (!isListLevelDefined(iLevel))
    {
        if (ordered)
            setListLevel(iLevel, new OrderedListLevelStyle(xPropList));
        else
            setListLevel(iLevel, new UnorderedListLevelStyle(xPropList));
    }
}

void ListStyle::write(OdfDocumentHandler *pHandler) const
{
    TagOpenElement listStyleOpenElement("text:list-style");
    listStyleOpenElement.addAttribute("style:name", getName());
    listStyleOpenElement.write(pHandler);

    for (int i=0; i<WP6_NUM_LIST_LEVELS; ++i)
    {
        if (mppListLevels[i])
            mppListLevels[i]->write(pHandler, i);
    }

    pHandler->endElement("text:list-style");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
