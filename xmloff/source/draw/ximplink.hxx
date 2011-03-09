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

#ifndef _XIMPLINK_HXX
#define _XIMPLINK_HXX

#include <xmloff/xmlictxt.hxx>
#include "sdxmlimp_impl.hxx"
#include <xmloff/nmspmap.hxx>
#include <com/sun/star/drawing/XShapes.hpp>
#include <tools/rtti.hxx>
#include "ximpshap.hxx"

//////////////////////////////////////////////////////////////////////////////
// draw:a context

// this should have been a SvXMLImportContext but CreateGroupChildContext() returns
// an unneeded derivation. Should be changed sometime during refactoring.

class SdXMLShapeLinkContext : public SvXMLShapeContext
{
    // the parent shape group this link is placed in
    com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > mxParent;
    rtl::OUString msHyperlink;

public:
    TYPEINFO();

    SdXMLShapeLinkContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLShapeLinkContext();

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix, const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();
};


#endif  //  _XIMPLINK_HXX
