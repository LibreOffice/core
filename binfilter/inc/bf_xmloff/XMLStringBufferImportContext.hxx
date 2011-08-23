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
#ifndef _XMLOFF_XMLSTRINGBUFFERIMPORTCONTEXT_HXX
#define _XMLOFF_XMLSTRINGBUFFERIMPORTCONTEXT_HXX


#ifndef _XMLOFF_XMLICTXT_HXX 
#include "xmlictxt.hxx"
#endif


#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
namespace binfilter {


/**
 * Import all text into a string buffer.  Paragraph elements (<text:p>) 
 * are recognized and cause a return character (0x0a) to be added.
 */
class XMLStringBufferImportContext : public SvXMLImportContext
{
    ::rtl::OUStringBuffer& rTextBuffer;

public:

    TYPEINFO();

    XMLStringBufferImportContext(
        SvXMLImport& rImport, 
        sal_uInt16 nPrefix,
        const ::rtl::OUString& sLocalName,
        ::rtl::OUStringBuffer& rBuffer);

    virtual ~XMLStringBufferImportContext();

    virtual SvXMLImportContext *CreateChildContext(
        USHORT nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< 
            ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void Characters( 
        const ::rtl::OUString& rChars );

    virtual void EndElement();
};

}//end of namespace binfilter
#endif
