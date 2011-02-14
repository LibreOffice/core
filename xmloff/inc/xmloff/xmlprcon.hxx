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

#ifndef _XMLOFF_XMLPROPERTYSETCONTEXT_HXX
#define _XMLOFF_XMLPROPERTYSETCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/uniref.hxx>

namespace rtl { class OUString; }

class SvXMLImportPropertyMapper;

class SvXMLPropertySetContext : public SvXMLImportContext
{
protected:
    sal_Int32 mnStartIdx;
    sal_Int32 mnEndIdx;
    sal_uInt32 mnFamily;
    ::std::vector< XMLPropertyState > &mrProperties;
    UniReference < SvXMLImportPropertyMapper >   mxMapper;

public:

    SvXMLPropertySetContext(
            SvXMLImport& rImport, sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
            sal_uInt32 nFamily,
            ::std::vector< XMLPropertyState > &rProps,
            const UniReference < SvXMLImportPropertyMapper > &rMap,
              sal_Int32 nStartIdx = -1, sal_Int32 nEndIdx = -1 );

    virtual ~SvXMLPropertySetContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    // This method is called from this instance implementation of
    // CreateChildContext if the element matches an entry in the
    // SvXMLImportItemMapper with the mid flag MID_FLAG_ELEMENT_ITEM_IMPORT
    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
                                   ::std::vector< XMLPropertyState > &rProperties,
                                   const XMLPropertyState& rProp );

};

#endif  //  _XMLOFF_XMLPROPERTYSETCONTEXT_HXX

