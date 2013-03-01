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
    namespace xforms { class XDataTypeRepository; class XModel2; }
} } }
class SvXMLNamespaceMap;

com::sun::star::uno::Reference<com::sun::star::xforms::XModel2> xforms_createXFormsModel();

void xforms_addXFormsModel(
    const com::sun::star::uno::Reference<com::sun::star::frame::XModel>& xDocument,
    const com::sun::star::uno::Reference<com::sun::star::xforms::XModel2>& xModel );

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
