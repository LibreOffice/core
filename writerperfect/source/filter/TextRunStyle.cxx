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

void ParagraphStyle::write(DocumentHandler *pHandler) const
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
    WPXPropertyList::Iter k((*mpPropList));
    for (k.rewind(); k.next(); )
    {
                if (strcmp(k.key(), "style:list-style-name") == 0)
                        propList.insert("style:list-style-name", k()->getStr());
        if (strcmp(k.key(), "fo:margin-left") == 0)
            propList.insert("fo:margin-left", k()->getStr());
        if (strcmp(k.key(), "fo:margin-right") == 0)
            propList.insert("fo:margin-right", k()->getStr());
        if (strcmp(k.key(), "fo:text-indent") == 0)
            propList.insert("fo:text-indent", k()->getStr());
        if (strcmp(k.key(), "fo:margin-top") == 0)
            propList.insert("fo:margin-top", k()->getStr());
        if (strcmp(k.key(), "fo:margin-bottom") == 0)
        {
            if (k()->getFloat() > 0.0f)
                propList.insert("fo:margin-bottom", k()->getStr());
            else
                propList.insert("fo:margin-bottom", 0.0f);
        }
        if (strcmp(k.key(), "fo:line-height") == 0)
            propList.insert("fo:line-height", k()->getStr());
        if (strcmp(k.key(), "fo:break-before") == 0) 
            propList.insert("fo:break-before", k()->getStr());
        if (strcmp(k.key(), "fo:text-align") == 0) 
            propList.insert("fo:text-align", k()->getStr());
                if (strcmp(k.key(), "fo:text-align-last") == 0)
                        propList.insert("fo:text-align-last", k()->getStr());
    }
    
    propList.insert("style:justify-single-word", "false");
    pHandler->startElement("style:properties", propList);

        if (mxTabStops.count() > 0) 
        {
                TagOpenElement tabListOpen("style:tab-stops");
                tabListOpen.write(pHandler);
                WPXPropertyListVector::Iter i(mxTabStops);
                for (i.rewind(); i.next();)
                {
                        TagOpenElement tabStopOpen("style:tab-stop");
                        
                        WPXPropertyList::Iter j(i());
                        for (j.rewind(); j.next(); )
                        {
                                tabStopOpen.addAttribute(j.key(), j()->getStr().cstr());			
                        }
                        tabStopOpen.write(pHandler);
                        pHandler->endElement("style:tab-stop");
                }
                pHandler->endElement("style:tab-stops");
        }

    pHandler->endElement("style:properties");
    pHandler->endElement("style:style");
}

SpanStyle::SpanStyle(const char *psName, const WPXPropertyList &xPropList) :
    Style(psName),
        mPropList(xPropList)
{
}

void SpanStyle::write(DocumentHandler *pHandler) const 
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
        propList.insert("style:font-size-asian", mPropList["fo:font-size"]->getStr());
        propList.insert("style:font-size-complex", mPropList["fo:font-size"]->getStr());
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

        pHandler->startElement("style:properties", propList);

    pHandler->endElement("style:properties");
    pHandler->endElement("style:style");
}
