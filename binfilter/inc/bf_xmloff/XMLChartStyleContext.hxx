/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _XMLOFF_XMLCHARTSTYLECONTEXT_HXX_
#define _XMLOFF_XMLCHARTSTYLECONTEXT_HXX_

#include "prstylei.hxx"
namespace binfilter {

class XMLChartStyleContext : public XMLPropStyleContext
{
private:
    ::rtl::OUString msDataStyleName;
    SvXMLStylesContext& mrStyles;

protected:
    /// is called when an attribute at the (auto)style element is found
    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const ::rtl::OUString& rLocalName,
                               const ::rtl::OUString& rValue );

public:
    TYPEINFO();

    XMLChartStyleContext(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
        SvXMLStylesContext& rStyles, sal_uInt16 nFamily );
    virtual ~XMLChartStyleContext();

    /// is called after all styles have been read to apply styles
    void FillPropertySet(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > & rPropSet );

    /// necessary for property context (element-property symbol-image)
    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
};

}//end of namespace binfilter
#endif	// _XMLOFF_XMLCHARTSTYLECONTEXT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
