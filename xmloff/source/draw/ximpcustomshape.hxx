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

#ifndef _XMLOFF_XIMPCUSTOMSHAPE_HXX_
#define _XMLOFF_XIMPCUSTOMSHAPE_HXX_

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustring.hxx>
#include <vector>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#ifndef __com_sun_star_beans_PropertyValues_hpp__
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameter.hpp>

namespace com { namespace sun { namespace star {
    namespace container { class XIndexContainer; }
    namespace beans { class XPropertySet; }
    namespace xml { namespace sax { class XAttributeList; } }
} } }

class SdXMLCustomShapeContext;
class XMLEnhancedCustomShapeContext : public SvXMLImportContext
{
    SvXMLUnitConverter& mrUnitConverter;
    com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& mrxShape;
    std::vector< com::sun::star::beans::PropertyValue >& mrCustomShapeGeometry;

    std::vector< com::sun::star::beans::PropertyValue > maExtrusion;
    std::vector< com::sun::star::beans::PropertyValue > maPath;
    std::vector< com::sun::star::beans::PropertyValue > maTextPath;
    std::vector< com::sun::star::beans::PropertyValues > maHandles;
    std::vector< rtl::OUString > maEquations;
    std::vector< rtl::OUString > maEquationNames;

public:

    TYPEINFO();

    XMLEnhancedCustomShapeContext( SvXMLImport& rImport, ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > &, sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName, std::vector< com::sun::star::beans::PropertyValue >& rCustomShapeGeometry );

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList> & xAttrList );
};

#endif
