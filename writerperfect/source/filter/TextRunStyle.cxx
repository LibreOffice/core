/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* TextRunStyle: Stores (and writes) paragraph/span-style-based information
 * (e.g.: a paragraph might be bold) that is needed at the head of an OO
 * document.
 *
 * Copyright (C) 2002-2004 William Lachance (william.lachance@sympatico.ca)
 * Copyright (C) 2004 Net Integration Technologies, Inc. (http://www.net-itech.com)
 * Copyright (C) 2004 Fridrich Strba (fridrich.strba@bluewin.ch)
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
#include "TextRunStyle.hxx"
#include "WriterProperties.hxx"
#include "DocumentElement.hxx"

#ifdef _MSC_VER
#include <minmax.h>
#endif

#include <string.h>

ParagraphStyle::ParagraphStyle(WPXPropertyList *pPropList, const WPXPropertyListVector &xTabStops, const WPXString &sName) :
    mpPropList(pPropList),
    mxTabStops(xTabStops),
    msName(sName)
{
}

ParagraphStyle::~ParagraphStyle()
{
    delete mpPropList;
}

void ParagraphStyle::write(DocumentHandlerInterface *pHandler) const
{
    WRITER_DEBUG_MSG(("Writing a paragraph style..\n"));

        WPXPropertyList propList;
    propList.insert("style:name", msName.cstr());
    propList.insert("style:family", "paragraph");
    propList.insert("style:parent-style-name", (*mpPropList)["style:parent-style-name"]->getStr());
    if ((*mpPropList)["style:master-page-name"])
        propList.insert("style:master-page-name", (*mpPropList)["style:master-page-name"]->getStr());
        pHandler->startElement("style:style", propList);

        propList.clear();
    WPXPropertyList::Iter i((*mpPropList));
    for (i.rewind(); i.next(); )
    {
#if 0
        if (strcmp(i.key(), "style:list-style-name") == 0)
            propList.insert("style:list-style-name", i()->getStr());
#endif
        if (strcmp(i.key(), "fo:margin-left") == 0)
            propList.insert("fo:margin-left", i()->getStr());
        if (strcmp(i.key(), "fo:margin-right") == 0)
            propList.insert("fo:margin-right", i()->getStr());
        if (strcmp(i.key(), "fo:text-indent") == 0)
            propList.insert("fo:text-indent", i()->getStr());
        if (strcmp(i.key(), "fo:margin-top") == 0)
            propList.insert("fo:margin-top", i()->getStr());
        if (strcmp(i.key(), "fo:margin-bottom") == 0)
        {
            if (i()->getDouble() > 0.0)
                propList.insert("fo:margin-bottom", i()->getStr());
            else
                propList.insert("fo:margin-bottom", 0.0);
        }
        if (strcmp(i.key(), "fo:line-height") == 0)
            propList.insert("fo:line-height", i()->getStr());
        if (strcmp(i.key(), "fo:break-before") == 0)
            propList.insert("fo:break-before", i()->getStr());
        if (strcmp(i.key(), "fo:text-align") == 0)
            propList.insert("fo:text-align", i()->getStr());
        if (strcmp(i.key(), "fo:text-align-last") == 0)
            propList.insert("fo:text-align-last", i()->getStr());
    }

    propList.insert("style:justify-single-word", "false");
    pHandler->startElement("style:paragraph-properties", propList);

        if (mxTabStops.count() > 0)
        {
                TagOpenElement tabListOpen("style:tab-stops");
                tabListOpen.write(pHandler);
        WPXPropertyListVector::Iter i2(mxTabStops);
        for (i2.rewind(); i2.next();)
                {
            if (i2()["style:position"] && i2()["style:position"]->getDouble() < 0)
                continue;
                        TagOpenElement tabStopOpen("style:tab-stop");

            WPXPropertyList::Iter j(i2());
                        for (j.rewind(); j.next(); )
                        {
                                tabStopOpen.addAttribute(j.key(), j()->getStr().cstr());			
                        }
                        tabStopOpen.write(pHandler);
                        pHandler->endElement("style:tab-stop");
                }
                pHandler->endElement("style:tab-stops");
        }

    pHandler->endElement("style:paragraph-properties");
    pHandler->endElement("style:style");
}

SpanStyle::SpanStyle(const char *psName, const WPXPropertyList &xPropList) :
    Style(psName),
        mPropList(xPropList)
{
}

void SpanStyle::write(DocumentHandlerInterface *pHandler) const
{
    WRITER_DEBUG_MSG(("Writing a span style..\n"));
        WPXPropertyList styleOpenList;
    styleOpenList.insert("style:name", getName());
    styleOpenList.insert("style:family", "text");
        pHandler->startElement("style:style", styleOpenList);

        WPXPropertyList propList(mPropList);

    if (mPropList["style:font-name"])
    {
        propList.insert("style:font-name-asian", mPropList["style:font-name"]->getStr());
        propList.insert("style:font-name-complex", mPropList["style:font-name"]->getStr());
    }

    if (mPropList["fo:font-size"])
    {
        if (mPropList["fo:font-size"]->getDouble() > 0.0)
        {
        propList.insert("style:font-size-asian", mPropList["fo:font-size"]->getStr());
        propList.insert("style:font-size-complex", mPropList["fo:font-size"]->getStr());
    }
        else
            propList.remove("fo:font-size");
    }

    if (mPropList["fo:font-weight"])
    {
        propList.insert("style:font-weight-asian", mPropList["fo:font-weight"]->getStr());
        propList.insert("style:font-weight-complex", mPropList["fo:font-weight"]->getStr());
    }

    if (mPropList["fo:font-style"])
    {
        propList.insert("style:font-style-asian", mPropList["fo:font-style"]->getStr());
        propList.insert("style:font-style-complex", mPropList["fo:font-style"]->getStr());
    }

    pHandler->startElement("style:text-properties", propList);

    pHandler->endElement("style:text-properties");
    pHandler->endElement("style:style");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
