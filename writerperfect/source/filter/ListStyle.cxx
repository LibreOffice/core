/* ListStyle: Stores (and writes) list-based information that is
 * needed at the head of an OO document.
 *
 * Copyright (C) 2002-2003 William Lachance (william.lachance@sympatico.ca)
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

using namespace ::rtl;
using rtl::OUString;

OrderedListLevelStyle::OrderedListLevelStyle(const WPXNumberingType listType, const UCSString &sTextBeforeNumber,
                         const UCSString &sTextAfterNumber, const float fSpaceBefore, const int iStartingNumber) :
    mlistType(listType),
    msTextBeforeNumber(sTextBeforeNumber),
    msTextAfterNumber(sTextAfterNumber),
    mfSpaceBefore(fSpaceBefore),
    miStartingNumber(iStartingNumber)
{
}

void OrderedListStyle::updateListLevel(const int iLevel, const WPXNumberingType listType, const UCSString &sTextBeforeNumber, const UCSString &sTextAfterNumber, const int iStartingNumber)
{
    if (!isListLevelDefined(iLevel))
        setListLevel(iLevel, new OrderedListLevelStyle(listType, sTextBeforeNumber, sTextAfterNumber, iLevel*0.5f, iStartingNumber));
}

void OrderedListLevelStyle::write(Reference < XDocumentHandler > &xHandler, int iLevel) const
{
    // then convert from ucs4 to utf8 and write it
    //char *sBulletUTF8 = ucs2ArrayToUTF8String(msBullet);

    //if (mfSpaceBefore != 0.0f)
    UTF8String sListTypeSymbol("1");
    switch (mlistType)
    {
    case ARABIC:
        sListTypeSymbol.sprintf("1");
        break;
    case LOWERCASE:
        sListTypeSymbol.sprintf("a");
        break;
    case UPPERCASE:
        sListTypeSymbol.sprintf("A");
        break;
     case LOWERCASE_ROMAN:
        sListTypeSymbol.sprintf("i");
        break;
     case UPPERCASE_ROMAN:
        sListTypeSymbol.sprintf("I");
        break;
    }

    UTF8String sTextBeforeNumber(msTextBeforeNumber, true);
    UTF8String sTextAfterNumber(msTextAfterNumber, true);
    UTF8String sLevel;
    sLevel.sprintf("%i", (iLevel+1));
    UTF8String sStartValue;
    sStartValue.sprintf("%i", miStartingNumber);

    TagOpenElement listLevelStyleOpen("text:list-level-style-number");
    listLevelStyleOpen.addAttribute("text:level", sLevel.getUTF8());
    listLevelStyleOpen.addAttribute("text:style-name", "Numbering Symbols");
    listLevelStyleOpen.addAttribute("style:num-prefix", sTextBeforeNumber.getUTF8());
    listLevelStyleOpen.addAttribute("style:num-suffix", sTextAfterNumber.getUTF8());
    listLevelStyleOpen.addAttribute("style:num-format", sListTypeSymbol.getUTF8());
    listLevelStyleOpen.addAttribute("text:start-value", sStartValue.getUTF8());
    listLevelStyleOpen.write(xHandler);

    UTF8String sSpaceBefore;
    sSpaceBefore.sprintf("%fcm", mfSpaceBefore);
    TagOpenElement stylePropertiesOpen("style:properties");
    stylePropertiesOpen.addAttribute("text:space-before", sSpaceBefore.getUTF8());
    stylePropertiesOpen.addAttribute("text:min-label-width", "0.499cm");
    stylePropertiesOpen.write(xHandler);

    xHandler->endElement(OUString::createFromAscii("style:properties"));
    xHandler->endElement(OUString::createFromAscii("text:list-level-style-number"));
}

UnorderedListLevelStyle::UnorderedListLevelStyle(const UCSString &sBullet, const float fSpaceBefore)
    : msBullet(sBullet),
      mfSpaceBefore(fSpaceBefore)
{
}

void UnorderedListStyle::updateListLevel(const int iLevel, const UCSString &sBullet)
{
    if (!isListLevelDefined(iLevel))
        setListLevel(iLevel, new UnorderedListLevelStyle(sBullet, iLevel*0.5f));
}

void UnorderedListLevelStyle::write(Reference < XDocumentHandler > &xHandler, int iLevel) const
{
    UTF8String sBulletUTF8(msBullet, true);
    UTF8String sLevel;
    sLevel.sprintf("%i", (iLevel+1));
    TagOpenElement listLevelStyleOpen("text:list-level-style-bullet");
    listLevelStyleOpen.addAttribute("text:level", sLevel.getUTF8());
    listLevelStyleOpen.addAttribute("text:style-name", "Bullet Symbols");
    listLevelStyleOpen.addAttribute("style:num-suffice", ".");
    listLevelStyleOpen.addAttribute("text:bullet-char", sBulletUTF8.getUTF8());
    listLevelStyleOpen.write(xHandler);

    UTF8String sSpaceBefore;
    sSpaceBefore.sprintf("%fcm", mfSpaceBefore);
    TagOpenElement stylePropertiesOpen("style:properties");
    stylePropertiesOpen.addAttribute("text:space-before", sSpaceBefore.getUTF8());
    stylePropertiesOpen.addAttribute("text:min-label-width", "0.499cm");
    stylePropertiesOpen.addAttribute("style:font-name", "StarSymbol");
    stylePropertiesOpen.write(xHandler);

    xHandler->endElement(OUString::createFromAscii("style:properties"));
    xHandler->endElement(OUString::createFromAscii("text:list-level-style-bullet"));
}

ListStyle::ListStyle(const char *psName, const int iListID) :
    Style(psName),
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

const bool ListStyle::isListLevelDefined(int iLevel) const
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

void ListStyle::write(Reference < XDocumentHandler > &xHandler) const
{
    TagOpenElement listStyleOpenElement("text:list-style");
    listStyleOpenElement.addAttribute("style:name", getName());
    listStyleOpenElement.write(xHandler);

    for (int i=0; i<WP6_NUM_LIST_LEVELS; i++) {
        if (mppListLevels[i] != NULL)
            mppListLevels[i]->write(xHandler, i);
    }

    xHandler->endElement(OUString::createFromAscii("text:list-style"));
}
