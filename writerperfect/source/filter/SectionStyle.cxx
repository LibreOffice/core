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
#include "SectionStyle.hxx"
#include "DocumentElement.hxx"

using namespace ::rtl;
using rtl::OUString;

const float fDefaultSideMargin = 1.0f; // inches
const float fDefaultPageWidth = 8.5f; // inches (OOo required default: we will handle this later)
const float fDefaultPageHeight = 11.0f; // inches

SectionStyle::SectionStyle(const int iNumColumns, const char *psName) : Style(psName),
    miNumColumns(iNumColumns)
{

    WRITER_DEBUG_MSG(("WriterWordPerfect: Created a new set of section props with this no. of columns: %i and this name: %s\n",
           (int)miNumColumns, (const char *)getName()));
}

void SectionStyle::write(Reference < XDocumentHandler > &xHandler) const
{
    TagOpenElement styleOpen("style:style");
    styleOpen.addAttribute("style:name", getName());
    styleOpen.addAttribute("style:family", "section");
    styleOpen.write(xHandler);

    // style properties
    TagOpenElement stylePropertiesOpen("style:properties");
    stylePropertiesOpen.addAttribute("text:dont-balance-text-columns", "false");
    stylePropertiesOpen.write(xHandler);

    // column properties
    TagOpenElement columnsOpen("style:columns");
    UTF8String sColumnCount;
    sColumnCount.sprintf("%i", miNumColumns);
    columnsOpen.addAttribute("fo:column-count", sColumnCount.getUTF8());
    columnsOpen.addAttribute("fo:column-gap", "0inch");
    columnsOpen.write(xHandler);

    if (miNumColumns > 1) {
        for (int i=0; i<miNumColumns; i++) {
            // theoretically, we would put column widths in here, but that's currently unsupported..
            // so we just allocate a size of "1" for each
            TagOpenElement columnOpen("style:column");
            columnOpen.addAttribute("style:rel-width", "1");
            columnOpen.addAttribute("fo:margin-left", "0inch");
            columnOpen.addAttribute("fo:margin-right", "0inch");
            columnOpen.write(xHandler);

            TagCloseElement columnClose("style:column");
            columnClose.write(xHandler);
        }
    }

    xHandler->endElement(OUString::createFromAscii("style:columns"));
    xHandler->endElement(OUString::createFromAscii("style:properties"));
    xHandler->endElement(OUString::createFromAscii("style:style"));
}
