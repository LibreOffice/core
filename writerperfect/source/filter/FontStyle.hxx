/* FontStyle: Stores (and writes) font-based information that is needed at
 * the head of an OO document.
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
#ifndef _FONTSTYLE_H
#define _FONTSTYLE_H
#include <libwpd/libwpd.h>

#include "Style.hxx"
#include "WriterProperties.hxx"

#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

using com::sun::star::uno::Reference;
using com::sun::star::xml::sax::XDocumentHandler;

class FontStyle : public Style
{
public:
    FontStyle(const char *psName, const char *psFontFamily);
    ~FontStyle();
    virtual void write(Reference < XDocumentHandler > &xHandler) const;
    const UTF8String &getFontFamily() const { return msFontFamily; }

private:
    UTF8String msFontFamily;
    UTF8String msFontPitch;
};
#endif
