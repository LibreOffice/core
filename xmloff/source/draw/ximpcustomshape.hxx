/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_XMLOFF_SOURCE_DRAW_XIMPCUSTOMSHAPE_HXX
#define INCLUDED_XMLOFF_SOURCE_DRAW_XIMPCUSTOMSHAPE_HXX

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustring.hxx>
#include <vector>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameter.hpp>

namespace com { namespace sun { namespace star {
    namespace container { class XIndexContainer; }
    namespace beans { class XPropertySet; }
    namespace xml { namespace sax { class XAttributeList; } }
} } }

class XMLEnhancedCustomShapeContext : public SvXMLImportContext
{
    SvXMLUnitConverter& mrUnitConverter;
    com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& mrxShape;
    std::vector< com::sun::star::beans::PropertyValue >& mrCustomShapeGeometry;

    std::vector< com::sun::star::beans::PropertyValue > maExtrusion;
    std::vector< com::sun::star::beans::PropertyValue > maPath;
    std::vector< com::sun::star::beans::PropertyValue > maTextPath;
    std::vector< com::sun::star::beans::PropertyValues > maHandles;
    std::vector< OUString > maEquations;
    std::vector< OUString > maEquationNames;

public:

    TYPEINFO_OVERRIDE();

    XMLEnhancedCustomShapeContext( SvXMLImport& rImport, ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > &, sal_uInt16 nPrefix,
        const OUString& rLocalName, std::vector< com::sun::star::beans::PropertyValue >& rCustomShapeGeometry );

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList ) override;
    virtual void EndElement() override;

    SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList> & xAttrList ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
