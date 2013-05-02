/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* SectionStyle: Stores (and writes) section-based information (e.g.: a column
 * change needs a new section) that is needed at the head of an OO document.
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
#ifndef _SECTIONSTYLE_H
#define _SECTIONSTYLE_H
#include <libwpd/libwpd.h>
#include <libwpd/WPXPropertyListVector.h>

#include "Style.hxx"


class SectionStyle : public Style
{
public:
    SectionStyle(const WPXPropertyList &xPropList, const WPXPropertyListVector &xColumns, const char *psName);
    virtual void write(OdfDocumentHandler *pHandler) const;

private:
    WPXPropertyList mPropList;
    WPXPropertyListVector mColumns;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
