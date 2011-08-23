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

#ifndef _XMLOFF_XMLLINENUMBERINGSEPARATORIMPORTCONTEXT_HXX_
#define _XMLOFF_XMLLINENUMBERINGSEPARATORIMPORTCONTEXT_HXX_

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_ 
#include <com/sun/star/uno/Reference.h>
#endif


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
} } }
namespace binfilter {
class XMLLineNumberingImportContext;


/** import <text:linenumbering-separator> elements */
class XMLLineNumberingSeparatorImportContext : public SvXMLImportContext
{
    ::rtl::OUStringBuffer sSeparatorBuf;
    XMLLineNumberingImportContext& rLineNumberingContext;

public:	

    TYPEINFO();

    XMLLineNumberingSeparatorImportContext(
        SvXMLImport& rImport, 
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName,
        XMLLineNumberingImportContext& rLineNumbering);

    ~XMLLineNumberingSeparatorImportContext();

protected:

    virtual void StartElement(
        const ::com::sun::star::uno::Reference< 
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    virtual void Characters( const ::rtl::OUString& rChars );

    virtual void EndElement();
};

}//end of namespace binfilter
#endif
