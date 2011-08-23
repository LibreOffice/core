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

#ifndef _XMLTEXTCOLUMNSCONTEXT_HXX
#define _XMLTEXTCOLUMNSCONTEXT_HXX

#ifndef _XMLOFF_XMLELEMENTPROPERTYCONTEXT_HXX
#include "XMLElementPropertyContext.hxx"
#endif


namespace rtl { class OUString; }
namespace binfilter {
class XMLTextColumnsArray_Impl;
class XMLTextColumnSepContext_Impl;
class SvXMLTokenMap;

class XMLTextColumnsContext :public XMLElementPropertyContext
{
    const ::rtl::OUString sSeparatorLineIsOn;
    const ::rtl::OUString sSeparatorLineWidth;
    const ::rtl::OUString sSeparatorLineColor;
    const ::rtl::OUString sSeparatorLineRelativeHeight;
    const ::rtl::OUString sSeparatorLineVerticalAlignment;
    const ::rtl::OUString sIsAutomatic;
    const ::rtl::OUString sAutomaticDistance;


    XMLTextColumnsArray_Impl *pColumns;
    XMLTextColumnSepContext_Impl	 *pColumnSep;
    SvXMLTokenMap	 		 *pColumnAttrTokenMap;
    SvXMLTokenMap	 		 *pColumnSepAttrTokenMap;
    sal_Int16				 nCount;
    sal_Bool                 bAutomatic;
    sal_Int32                nAutomaticDistance;

public:
    TYPEINFO();

    XMLTextColumnsContext(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
        const XMLPropertyState& rProp,
        ::std::vector< XMLPropertyState > &rProps );

    virtual ~XMLTextColumnsContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void EndElement();
};


}//end of namespace binfilter
#endif
