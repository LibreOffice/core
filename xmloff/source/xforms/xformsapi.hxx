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
#ifndef _XMLOFF_XFORMSAPI_HXX
#define _XMLOFF_XFORMSAPI_HXX

//
// this is a collection of several functions to make dealing with the XForms
// API a little easier
//

#include <com/sun/star/uno/Any.hxx>

namespace rtl { class OUString; }
namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
    namespace frame { class XModel; }
    namespace uno { template<class A> class Reference; }
    namespace xforms { class XDataTypeRepository; }
} } }
class SvXMLNamespaceMap;


#define OUSTRING(msg) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(msg))

com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> xforms_createXFormsModel();

void xforms_addXFormsModel(
    const com::sun::star::uno::Reference<com::sun::star::frame::XModel>& xDocument,
    const com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& xModel );

com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> xforms_findXFormsBinding( com::sun::star::uno::Reference<com::sun::star::frame::XModel>&, const rtl::OUString& );

com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> xforms_findXFormsSubmission( com::sun::star::uno::Reference<com::sun::star::frame::XModel>&, const rtl::OUString& );

void xforms_setValue(
    com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& xPropSet,
    const rtl::OUString& rName,
    const com::sun::star::uno::Any rAny );

template<typename T>
void xforms_setValue(
    com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& xPropSet,
    const rtl::OUString& rName,
    T& aValue )
{
    xforms_setValue( xPropSet, rName, com::sun::star::uno::makeAny( aValue ) );
}

sal_uInt16 xforms_getTypeClass(
    const com::sun::star::uno::Reference<com::sun::star::xforms::XDataTypeRepository>& xRepository,
    const SvXMLNamespaceMap& rNamespaceMap,
    const rtl::OUString& rXMLName );

rtl::OUString xforms_getTypeName(
    const com::sun::star::uno::Reference<com::sun::star::xforms::XDataTypeRepository>& xRepository,
    const SvXMLNamespaceMap& rNamespaceMap,
    const rtl::OUString& rXMLName );

rtl::OUString xforms_getBasicTypeName(
    const com::sun::star::uno::Reference<com::sun::star::xforms::XDataTypeRepository>& xRepository,
    const SvXMLNamespaceMap& rNamespaceMap,
    const rtl::OUString& rXMLName );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
