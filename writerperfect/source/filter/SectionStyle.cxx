/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* SectionStyle: Stores (and writes) section-based information (e.g.: a column
 * break needs a new section) that is needed at the head of an OO document and
 * is referenced throughout the entire document
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information visit http://libwpd.sourceforge.net
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */
#include "FilterInternal.hxx"
#include "SectionStyle.hxx"
#include "DocumentElement.hxx"
#include <math.h>

#ifdef _MSC_VER
double rint(double x);
#endif /* _WIN32 */

SectionStyle::SectionStyle(const WPXPropertyList &xPropList,
                           const WPXPropertyListVector &xColumns,
                           const char *psName) :
    Style(psName),
    mPropList(xPropList),
    mColumns(xColumns)
{
}

void SectionStyle::write(OdfDocumentHandler *pHandler) const
{
    TagOpenElement styleOpen("style:style");
    styleOpen.addAttribute("style:name", getName());
    styleOpen.addAttribute("style:family", "section");
    styleOpen.write(pHandler);

    // if the number of columns is <= 1, we will never come here. This is only an additional check
    // style properties
    pHandler->startElement("style:section-properties", mPropList);

    // column properties
    WPXPropertyList columnProps;

    if (mColumns.count() > 1)
    {
        columnProps.insert("fo:column-count", (int)mColumns.count());
        pHandler->startElement("style:columns", columnProps);

        WPXPropertyListVector::Iter i(mColumns);
        for (i.rewind(); i.next();)
        {
            pHandler->startElement("style:column", i());
            pHandler->endElement("style:column");
        }
    }
    else
    {
        columnProps.insert("fo:column-count", 0);
        columnProps.insert("fo:column-gap", 0.0);
        pHandler->startElement("style:columns", columnProps);
    }

    pHandler->endElement("style:columns");


    pHandler->endElement("style:section-properties");

    pHandler->endElement("style:style");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
