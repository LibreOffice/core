/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLTEXTTABLECONTEXT_HXX
#define _XMLTEXTTABLECONTEXT_HXX

#ifndef _XMLOFF_XMLICTXT_HXX 
#include <bf_xmloff/xmlictxt.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_ 
#include <com/sun/star/uno/Reference.h>
#endif

namespace com { namespace sun { namespace star {
    namespace text { class XTextContent; }
} } }
namespace binfilter {

#define XML_TEXT_FRAME_TEXTBOX 1
#define XML_TEXT_FRAME_GRAPHIC 2

class XMLTextTableContext : public SvXMLImportContext
{

public:

    TYPEINFO();

    XMLTextTableContext( SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName );
    virtual ~XMLTextTableContext();

    virtual ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > GetXTextContent() const = 0;
};

}//end of namespace binfilter
#endif
