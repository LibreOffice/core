/* SectionStyle: Stores (and writes) section-based information (e.g.: a column
 * change needs a new section) that is needed at the head of an OO document.
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
#ifndef _SECTIONSTYLE_H
#define _SECTIONSTYLE_H
#include <libwpd/libwpd.h>

#include "Style.hxx"
#include "WriterProperties.hxx"

using com::sun::star::uno::Reference;
using com::sun::star::xml::sax::XDocumentHandler;

class SectionStyle : public Style
{
public:
    SectionStyle(const int iNumColumns, const char *psName);
    virtual void write(Reference < XDocumentHandler > &xHandler) const;

private:
    int miNumColumns;
};
#endif
