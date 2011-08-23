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

#ifndef _XMLOFF_VISAREACONTEXT_HXX
#define _XMLOFF_VISAREACONTEXT_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <bf_xmloff/xmlictxt.hxx>
#endif

#ifndef _VCL_MAPUNIT_HXX
#include <vcl/mapunit.hxx>
#endif
class Rectangle;
namespace com { namespace sun { namespace star { namespace awt {
    struct Rectangle;
} } } }
namespace binfilter {


class XMLVisAreaContext : public SvXMLImportContext
{
public:
    // read all attributes and set the values in rRect
    XMLVisAreaContext( SvXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ::com::sun::star::awt::Rectangle& rRect, const sal_Int16 nMeasureUnit);

    virtual ~XMLVisAreaContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();

private:
    void process(	const ::com::sun::star::uno::Reference<	::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                    ::com::sun::star::awt::Rectangle& rRect,
                    const sal_Int16 nMeasureUnit );

};

}//end of namespace binfilter
#endif
