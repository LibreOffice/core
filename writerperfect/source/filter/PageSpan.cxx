/* SectionStyle: Stores (and writes) section-based information (e.g.: a column
 * break needs a new section) that is needed at the head of an OO document and
 * is referenced throughout the entire document
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
#include "PageSpan.hxx"
#include "DocumentElement.hxx"

const float fDefaultPageWidth = 8.5f; // inches (OOo required default: we will handle this later)
const float fDefaultPageHeight = 11.0f; // inches

PageSpan::PageSpan(const int iSpan, const float fFormLength, const float fFormWidth, const WPXFormOrientation fFormOrientation,
             const float fLeftMargin, const float fRightMargin, const float fTopMargin, const float fBottomMargin):
    miSpan(iSpan),
    mfFormLength(fFormLength),
    mfFormWidth(fFormWidth),
    mfFormOrientation(fFormOrientation),
    mfMarginLeft(fLeftMargin),
    mfMarginRight(fRightMargin),
    mfMarginTop(fTopMargin),
    mfMarginBottom(fBottomMargin),
    mpHeaderContent(NULL),
    mpFooterContent(NULL),
    mpHeaderLeftContent(NULL),
    mpFooterLeftContent(NULL)
{
}

PageSpan::~PageSpan()
{
    delete mpHeaderContent;
    delete mpHeaderLeftContent;
    delete mpFooterContent;
    delete mpFooterLeftContent;
}

void PageSpan::writePageMaster(const int iNum, Reference < XDocumentHandler > &xHandler) const
{
    TagOpenElement pageMasterOpen("style:page-master");
    UTF8String sPageMasterName;
    sPageMasterName.sprintf("PM%i", iNum);
    pageMasterOpen.addAttribute("style:name", sPageMasterName.getUTF8());
    pageMasterOpen.write(xHandler);

    TagOpenElement pageMasterPropertiesOpen("style:properties");
    UTF8String sMarginTop;
    sMarginTop.sprintf("%.4finch", mfMarginTop);
    UTF8String sMarginBottom;
    sMarginBottom.sprintf("%.4finch", mfMarginBottom);
    UTF8String sMarginLeft;
    sMarginLeft.sprintf("%.4finch", mfMarginLeft);
    UTF8String sMarginRight;
    sMarginRight.sprintf("%.4finch", mfMarginRight);
    UTF8String sPageWidth;
    sPageWidth.sprintf("%.4finch", mfFormWidth);
    UTF8String sPageHeight;
    sPageHeight.sprintf("%.4finch", mfFormLength);
    if (mfFormOrientation == LANDSCAPE)
    {
        pageMasterPropertiesOpen.addAttribute("style:print-orientation", "landscape");
    }
    else
    {
        pageMasterPropertiesOpen.addAttribute("style:print-orientation", "portrait");
    }
    pageMasterPropertiesOpen.addAttribute("fo:page-width", sPageWidth.getUTF8());
    pageMasterPropertiesOpen.addAttribute("fo:page-height", sPageHeight.getUTF8());
    pageMasterPropertiesOpen.addAttribute("fo:margin-top", sMarginTop.getUTF8());
    pageMasterPropertiesOpen.addAttribute("fo:margin-bottom", sMarginBottom.getUTF8());
    pageMasterPropertiesOpen.addAttribute("fo:margin-left", sMarginLeft.getUTF8());
    pageMasterPropertiesOpen.addAttribute("fo:margin-right", sMarginRight.getUTF8());
    pageMasterPropertiesOpen.write(xHandler);
    TagCloseElement pageMasterPropertiesClose("style:properties");
    pageMasterPropertiesClose.write(xHandler);

    TagCloseElement pageMasterClose("style:page-master");
    pageMasterClose.write(xHandler);
}

void PageSpan::writeMasterPages(const int iStartingNum, const int iPageMasterNum, const bool bLastPageSpan, Reference < XDocumentHandler > &xHandler) const
{
    int iSpan = 0;
    (bLastPageSpan) ? iSpan = 1 : iSpan = miSpan;

    for (int i=iStartingNum; i<(iStartingNum+iSpan); i++)
    {
        TagOpenElement masterPageOpen("style:master-page");
        UTF8String sMasterPageName;
        sMasterPageName.sprintf("Page Style %i", i);
        UTF8String sPageMasterName;
        sPageMasterName.sprintf("PM%i", iPageMasterNum);
        masterPageOpen.addAttribute("style:name", sMasterPageName.getUTF8());
        masterPageOpen.addAttribute("style:page-master-name", sPageMasterName.getUTF8());
        if (!bLastPageSpan)
        {
            UTF8String sNextMasterPageName;
            sNextMasterPageName.sprintf("Page Style %i", (i+1));
            masterPageOpen.addAttribute("style:next-style-name", sNextMasterPageName.getUTF8());
        }
        masterPageOpen.write(xHandler);

        if (mpHeaderContent)
            _writeHeaderFooter("style:header", *mpHeaderContent, xHandler);
        if (mpHeaderLeftContent)
            _writeHeaderFooter("style:header-left", *mpHeaderLeftContent, xHandler);
        if (mpFooterContent)
            _writeHeaderFooter("style:footer", *mpFooterContent, xHandler);
        if (mpFooterLeftContent)
            _writeHeaderFooter("style:footer-left", *mpFooterLeftContent, xHandler);

        TagCloseElement masterPageClose("style:master-page");
        masterPageClose.write(xHandler);
    }

}

void PageSpan::_writeHeaderFooter(const char *headerFooterTagName,
                  const vector<DocumentElement *> & headerFooterContent,
                  Reference < XDocumentHandler > &xHandler) const
{
    TagOpenElement headerFooterOpen(headerFooterTagName);
    headerFooterOpen.write(xHandler);
    for (vector<DocumentElement *>::const_iterator iter = headerFooterContent.begin();
         iter != headerFooterContent.end();
         iter++) {
        (*iter)->write(xHandler);
    }
    TagCloseElement headerFooterClose(headerFooterTagName);
    headerFooterClose.write(xHandler);
}

