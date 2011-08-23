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

#ifndef _XMLOFF_XMLSHAPESTYLECONTEXT_HXX
#define _XMLOFF_XMLSHAPESTYLECONTEXT_HXX

#ifndef _XMLOFF_PRSTYLEI_HXX_
#include <bf_xmloff/prstylei.hxx>
#endif

#ifndef _XMLOFF_FAMILIES_HXX_
#include <bf_xmloff/families.hxx>
#endif
namespace binfilter {

class SvXMLImport;

//////////////////////////////////////////////////////////////////////////////
// style:style context

class XMLShapeStyleContext: public XMLPropStyleContext
{
protected:
    ::rtl::OUString		m_sControlDataStyleName;
    ::rtl::OUString		m_sListStyleName;
    sal_Bool			m_bIsNumRuleAlreadyConverted;

protected:
    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const ::rtl::OUString& rLocalName,
                               const ::rtl::OUString& rValue );
public:
    TYPEINFO();

    XMLShapeStyleContext(
        SvXMLImport& rImport, 
        sal_uInt16 nPrfx, 
        const ::rtl::OUString& rLName, 
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
        SvXMLStylesContext& rStyles,
        sal_uInt16 nFamily = XML_STYLE_FAMILY_SD_GRAPHICS_ID);
    virtual ~XMLShapeStyleContext();

    SvXMLImportContext * CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void Finish( sal_Bool bOverwrite );

    virtual void FillPropertySet(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > & rPropSet );
};

}//end of namespace binfilter
#endif	//  _XMLOFF_XMLSHAPESTYLECONTEXT_HXX
