/* SectionStyle: Stores (and writes) section-based information (e.g.: a column
 * break needs a new section) that is needed at the head of an OO document and
 * is referenced throughout the entire document
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

#include <stdlib.h>

#include "FilterInternal.hxx"
#include "PageSpan.hxx"
#include "DocumentElement.hxx"

PageSpan::PageSpan(const WPXPropertyList &xPropList) :
        mxPropList(xPropList),
    mpHeaderContent(NULL),
    mpFooterContent(NULL),
    mpHeaderLeftContent(NULL),
    mpFooterLeftContent(NULL)
{
}

namespace {
    typedef std::vector<DocumentElement *>::iterator DEVIter;
}

PageSpan::~PageSpan()
{
    if (mpHeaderContent)
    {
        for (DEVIter iterHeaderContent = mpHeaderContent->begin();
            iterHeaderContent != mpHeaderContent->end();
            iterHeaderContent++)
                delete(*iterHeaderContent);
        delete mpHeaderContent;
    }

    if (mpHeaderLeftContent)
    {
        for (DEVIter iterHeaderLeftContent = mpHeaderLeftContent->begin();
            iterHeaderLeftContent != mpHeaderLeftContent->end();
            iterHeaderLeftContent++)
                delete(*iterHeaderLeftContent);
        delete mpHeaderLeftContent;
    }

    if (mpFooterContent)
    {
        for (DEVIter iterFooterContent = mpFooterContent->begin();
            iterFooterContent != mpFooterContent->end();
            iterFooterContent++)
                delete(*iterFooterContent);
        delete mpFooterContent;
    }

    if (mpFooterLeftContent)
    {
        for (DEVIter iterFooterLeftContent = mpFooterLeftContent->begin();
            iterFooterLeftContent != mpFooterLeftContent->end();
            iterFooterLeftContent++)
                delete(*iterFooterLeftContent);
        delete mpFooterLeftContent;
    }
}

int PageSpan::getSpan() const
{
        if (mxPropList["libwpd:num-pages"])
                return mxPropList["libwpd:num-pages"]->getInt();

        return 0; // should never happen
}

double PageSpan::getMarginLeft() const
{
        if (mxPropList["fo:margin-left"])
                return mxPropList["fo:margin-left"]->getDouble();

         return 0.0;
}

double PageSpan::getMarginRight() const
{
        if (mxPropList["fo:margin-right"])
                return mxPropList["fo:margin-right"]->getDouble();

         return 0.0;
}

void PageSpan::setHeaderContent(std::vector<DocumentElement *> * pHeaderContent)
{
    if (mpHeaderContent)
    {
        for (DEVIter iterHeaderContent = mpHeaderContent->begin();
            iterHeaderContent != mpHeaderContent->end();
            iterHeaderContent++)
                delete(*iterHeaderContent);
        delete mpHeaderContent;
    }

    mpHeaderContent = pHeaderContent;
}

void PageSpan::setFooterContent(std::vector<DocumentElement *> * pFooterContent)
{
    if (mpFooterContent)
    {
        for (DEVIter iterFooterContent = mpFooterContent->begin();
            iterFooterContent != mpFooterContent->end();
            iterFooterContent++)
                delete(*iterFooterContent);
        delete mpFooterContent;
    }

    mpFooterContent = pFooterContent;
}

void PageSpan::setHeaderLeftContent(std::vector<DocumentElement *> * pHeaderContent)
{
    if (mpHeaderLeftContent)
    {
        for (DEVIter iterHeaderLeftContent = mpHeaderLeftContent->begin();
            iterHeaderLeftContent != mpHeaderLeftContent->end();
            iterHeaderLeftContent++)
                delete(*iterHeaderLeftContent);
        delete mpHeaderLeftContent;
    }

    mpHeaderLeftContent = pHeaderContent;
}

void PageSpan::setFooterLeftContent(std::vector<DocumentElement *> * pFooterContent)
{
    if (mpFooterLeftContent)
    {
        for (DEVIter iterFooterLeftContent = mpFooterLeftContent->begin();
            iterFooterLeftContent != mpFooterLeftContent->end();
            iterFooterLeftContent++)
                delete(*iterFooterLeftContent);
        delete mpFooterLeftContent;
    }

    mpFooterLeftContent = pFooterContent;
}

void PageSpan::writePageLayout(const int iNum, OdfDocumentHandler *pHandler) const
{
        WPXPropertyList propList;

    WPXString sPageLayoutName;
        sPageLayoutName.sprintf("PM%i", iNum+2);
        propList.insert("style:name", sPageLayoutName);
    pHandler->startElement("style:page-layout", propList);

    WPXPropertyList tempPropList = mxPropList;
    if (!tempPropList["style:writing-mode"])
        tempPropList.insert("style:writing-mode", WPXString("lr-tb"));
    if (!tempPropList["style:footnote-max-height"])
        tempPropList.insert("style:footnote-max-height", WPXString("0in"));
        pHandler->startElement("style:page-layout-properties", tempPropList);

    WPXPropertyList footnoteSepPropList;
    footnoteSepPropList.insert("style:width", WPXString("0.0071in"));
    footnoteSepPropList.insert("style:distance-before-sep", WPXString("0.0398in"));
    footnoteSepPropList.insert("style:distance-after-sep", WPXString("0.0398in"));
    footnoteSepPropList.insert("style:adjustment", WPXString("left"));
    footnoteSepPropList.insert("style:rel-width", WPXString("25%"));
    footnoteSepPropList.insert("style:color", WPXString("#000000"));
    pHandler->startElement("style:footnote-sep", footnoteSepPropList);

    pHandler->endElement("style:footnote-sep");
        pHandler->endElement("style:page-layout-properties");
        pHandler->endElement("style:page-layout");
}

void PageSpan::writeMasterPages(const int iStartingNum, const int iPageLayoutNum, const bool bLastPageSpan,
                                OdfDocumentHandler *pHandler) const
{
    int iSpan = 0;
    (bLastPageSpan) ? iSpan = 1 : iSpan = getSpan();

    for (int i=iStartingNum; i<(iStartingNum+iSpan); i++)
    {
        TagOpenElement masterPageOpen("style:master-page");
        WPXString sMasterPageName, sMasterPageDisplayName;
        sMasterPageName.sprintf("Page_Style_%i", i);
        sMasterPageDisplayName.sprintf("Page Style %i", i);
        WPXString sPageLayoutName;
        WPXPropertyList propList;
        sPageLayoutName.sprintf("PM%i", iPageLayoutNum+2);
                propList.insert("style:name", sMasterPageName);
        propList.insert("style:display-name", sMasterPageDisplayName);
        propList.insert("style:page-layout-name", sPageLayoutName);
        if (!bLastPageSpan)
        {
            WPXString sNextMasterPageName;
            sNextMasterPageName.sprintf("Page_Style_%i", (i+1));
                        propList.insert("style:next-style-name", sNextMasterPageName);
        }
                pHandler->startElement("style:master-page", propList);

        if (mpHeaderContent)
        {
            _writeHeaderFooter("style:header", *mpHeaderContent, pHandler);
            pHandler->endElement("style:header");
            if (mpHeaderLeftContent)
            {
                _writeHeaderFooter("style:header-left", *mpHeaderLeftContent, pHandler);
                pHandler->endElement("style:header-left");
            }
        }
        else if (mpHeaderLeftContent)
        {
            TagOpenElement("style:header").write(pHandler);
            pHandler->endElement("style:header");
            _writeHeaderFooter("style:header-left", *mpHeaderLeftContent, pHandler);
            pHandler->endElement("style:header-left");
        }

        if (mpFooterContent)
        {
            _writeHeaderFooter("style:footer", *mpFooterContent, pHandler);
            pHandler->endElement("style:footer");
            if (mpFooterLeftContent)
            {
                _writeHeaderFooter("style:footer-left", *mpFooterLeftContent, pHandler);
                pHandler->endElement("style:footer-left");
            }
        }
        else if (mpFooterLeftContent)
        {
            TagOpenElement("style:footer").write(pHandler);
            pHandler->endElement("style:footer");
            _writeHeaderFooter("style:footer-left", *mpFooterLeftContent, pHandler);
            pHandler->endElement("style:footer-left");
        }

                pHandler->endElement("style:master-page");
    }
}

void PageSpan::_writeHeaderFooter(const char *headerFooterTagName,
                  const std::vector<DocumentElement *> & headerFooterContent,
                  OdfDocumentHandler *pHandler) const
{
    TagOpenElement headerFooterOpen(headerFooterTagName);
    headerFooterOpen.write(pHandler);
    for (std::vector<DocumentElement *>::const_iterator iter = headerFooterContent.begin();
         iter != headerFooterContent.end();
         ++iter) {
        (*iter)->write(pHandler);
    }
}
