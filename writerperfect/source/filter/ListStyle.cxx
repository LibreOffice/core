/* ListStyle: Stores (and writes) list-based information that is
 * needed at the head of an OO document.
 *
 * Copyright (C) 2002-2003 William Lachance (wrlach@gmail.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * For further information visit http://libwpd.sourceforge.net
 *
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

void OrderedListStyle::updateListLevel(const int iLevel, const WPXPropertyList &xPropList)
{
    if (iLevel < 0)
        return;
    if (!isListLevelDefined(iLevel))
        setListLevel(iLevel, new OrderedListLevelStyle(xPropList));
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
        // odf as to the version 1.1 does require the text:start-value to be a positive integer, means > 0
        if (mPropList["text:start-value"]->getInt() > 0)
            listLevelStyleOpen.addAttribute("text:start-value", mPropList["text:start-value"]->getStr());
        else
            listLevelStyleOpen.addAttribute("text:start-value", "1");
    }
    listLevelStyleOpen.write(pHandler);

    TagOpenElement stylePropertiesOpen("style:list-level-properties");
    if (mPropList["text:space-before"] && mPropList["text:space-before"]->getDouble() > 0.0)
        stylePropertiesOpen.addAttribute("text:space-before", mPropList["text:space-before"]->getStr());
    if (mPropList["text:min-label-width"] && mPropList["text:min-label-width"]->getDouble() > 0.0)
        stylePropertiesOpen.addAttribute("text:min-label-width", mPropList["text:min-label-width"]->getStr());
    if (mPropList["text:min-label-distance"] && mPropList["text:min-label-distance"]->getDouble() > 0.0)
        stylePropertiesOpen.addAttribute("text:min-label-distance", mPropList["text:min-label-distance"]->getStr());
    stylePropertiesOpen.write(pHandler);

    pHandler->endElement("style:list-level-properties");
    pHandler->endElement("text:list-level-style-number");
}

UnorderedListLevelStyle::UnorderedListLevelStyle(const WPXPropertyList &xPropList)
    : mPropList(xPropList)
{
}

void UnorderedListStyle::updateListLevel(const int iLevel, const WPXPropertyList &xPropList)
{
    if (iLevel < 0)
        return;
    if (!isListLevelDefined(iLevel))
        setListLevel(iLevel, new UnorderedListLevelStyle(xPropList));
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
        // The following is needed because the odf format does not accept bullet chars longer than one character
        WPXString::Iter i(mPropList["text:bullet-char"]->getStr()); i.rewind();
        WPXString sEscapedString(".");
        if (i.next())
            sEscapedString = WPXString(i(), true);
        listLevelStyleOpen.addAttribute("text:bullet-char", sEscapedString);

    }
    else
        listLevelStyleOpen.addAttribute("text:bullet-char", ".");
    listLevelStyleOpen.write(pHandler);

    TagOpenElement stylePropertiesOpen("style:list-level-properties");
    if (mPropList["text:space-before"] && mPropList["text:space-before"]->getDouble() > 0.0)
        stylePropertiesOpen.addAttribute("text:space-before", mPropList["text:space-before"]->getStr());
    if (mPropList["text:min-label-width"] && mPropList["text:min-label-width"]->getDouble() > 0.0)
        stylePropertiesOpen.addAttribute("text:min-label-width", mPropList["text:min-label-width"]->getStr());
    if (mPropList["text:min-label-distance"] && mPropList["text:min-label-distance"]->getDouble() > 0.0)
        stylePropertiesOpen.addAttribute("text:min-label-distance", mPropList["text:min-label-distance"]->getStr());
    stylePropertiesOpen.addAttribute("style:font-name", "OpenSymbol");
    stylePropertiesOpen.write(pHandler);

    pHandler->endElement("style:list-level-properties");
    pHandler->endElement("text:list-level-style-bullet");
}

ListStyle::ListStyle(const char *psName, const int iListID) :
    Style(psName),
    miNumListLevels(0),
    miListID(iListID)
{
    for (int i=0; i<WP6_NUM_LIST_LEVELS; i++)
        mppListLevels[i] = NULL;

}

ListStyle::~ListStyle()
{
    for (int i=0; i<WP6_NUM_LIST_LEVELS; i++) {
        if (mppListLevels[i])
            delete(mppListLevels[i]);
    }

}

bool ListStyle::isListLevelDefined(int iLevel) const
{
    if (mppListLevels[iLevel] == NULL)
        return false;

    return true;
}

void ListStyle::setListLevel(int iLevel, ListLevelStyle *iListLevelStyle)
{
    // can't uncomment this next line without adding some extra logic.
    // figure out which is best: use the initial message, or constantly
    // update?
    if (mppListLevels[iLevel] == NULL)
        mppListLevels[iLevel] = iListLevelStyle;
}

void ListStyle::write(OdfDocumentHandler *pHandler) const
{
    TagOpenElement listStyleOpenElement("text:list-style");
    listStyleOpenElement.addAttribute("style:name", getName());
    listStyleOpenElement.write(pHandler);

    for (int i=0; i<WP6_NUM_LIST_LEVELS; i++) {
        if (mppListLevels[i] != NULL)
            mppListLevels[i]->write(pHandler, i);
    }

    pHandler->endElement("text:list-style");
}
