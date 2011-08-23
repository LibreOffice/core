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

#ifndef _XMLBRSHI_HXX
#define _XMLBRSHI_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif
namespace rtl { class OUString; }
namespace com { namespace sun { namespace star {
    namespace io { class XOutputStream; }
} } }
namespace binfilter {//STRIP009
class SvXMLImport; 
class SvXMLUnitConverter; 
class SvxBrushItem;
class SwXMLBrushItemImportContext : public SvXMLImportContext
{
private:
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > xBase64Stream;
    SvxBrushItem				*pItem;
    
    void ProcessAttrs(
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
               const SvXMLUnitConverter& rUnitConv );

public:
    TYPEINFO();

    SwXMLBrushItemImportContext(
            SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            const SvXMLUnitConverter& rUnitConv,
               const SvxBrushItem& rItem	);

    SwXMLBrushItemImportContext(
            SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            const SvXMLUnitConverter& rUnitConv,
            sal_uInt16 nWhich	);

    virtual ~SwXMLBrushItemImportContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const ::rtl::OUString& rLocalName,
                 const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void EndElement();

    const SvxBrushItem& GetItem() const { return *pItem; }
};


} //namespace binfilter
#endif	//  _XMLBRSHI_HXX

