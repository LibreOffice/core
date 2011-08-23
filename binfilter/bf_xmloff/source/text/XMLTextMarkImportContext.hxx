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


#ifndef _XMLOFF_XMLTEXTMARKIMPORTCONTEXT_HXX
#define _XMLOFF_XMLTEXTMARKIMPORTCONTEXT_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_ 
#include <com/sun/star/uno/Reference.h>
#endif


namespace com { namespace sun { namespace star {
    namespace text {
        class XTextRange;
    }
    namespace xml { namespace sax {
        class XAttributeList;
    } }
} } }
namespace rtl {
    class OUString;
}
namespace binfilter {
class XMLTextImportHelper;


/**
 * import bookmarks and reference marks
 * ( <bookmark>, <bookmark-start>, <bookmark-end>,
 *   <reference>, <reference-start>, <reference-end> )
 * 
 * All elements are handled by the same class due to their similarities.
 */
class XMLTextMarkImportContext : public SvXMLImportContext
{
    
    XMLTextImportHelper& rHelper;

public:
    
    TYPEINFO();

    XMLTextMarkImportContext(
        SvXMLImport& rImport, 
        XMLTextImportHelper& rHlp,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName );

protected:

    virtual void StartElement(
        const ::com::sun::star::uno::Reference< 
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

public:
    static void CreateAndInsertMark(
        SvXMLImport& rImport,
        const ::rtl::OUString& sServiceName,
        const ::rtl::OUString& sMarkName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::text::XTextRange> & rRange);

    static sal_Bool FindName(
        SvXMLImport& rImport,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList,
        ::rtl::OUString& sName);
};

}//end of namespace binfilter
#endif
