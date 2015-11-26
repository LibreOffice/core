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
#ifndef INCLUDED_XMLOFF_SOURCE_XFORMS_XFORMSAPI_HXX
#define INCLUDED_XMLOFF_SOURCE_XFORMS_XFORMSAPI_HXX


// this is a collection of several functions to make dealing with the XForms
// API a little easier


#include <com/sun/star/uno/Any.hxx>

namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
    namespace frame { class XModel; }
    namespace uno { template<class A> class Reference; }
    namespace xforms { class XDataTypeRepository; class XModel2; }
} } }
class SvXMLNamespaceMap;

css::uno::Reference<css::xforms::XModel2> xforms_createXFormsModel();

void xforms_addXFormsModel(
    const css::uno::Reference<css::frame::XModel>& xDocument,
    const css::uno::Reference<css::xforms::XModel2>& xModel );

css::uno::Reference<css::beans::XPropertySet> xforms_findXFormsBinding( css::uno::Reference<css::frame::XModel>&, const OUString& );

css::uno::Reference<css::beans::XPropertySet> xforms_findXFormsSubmission( css::uno::Reference<css::frame::XModel>&, const OUString& );

void xforms_setValue(
    css::uno::Reference<css::beans::XPropertySet>& xPropSet,
    const OUString& rName,
    const css::uno::Any& rAny );

template<typename T>
void xforms_setValue(
    css::uno::Reference<css::beans::XPropertySet>& xPropSet,
    const OUString& rName,
    T& aValue )
{
    xforms_setValue( xPropSet, rName, css::uno::makeAny( aValue ) );
}

sal_uInt16 xforms_getTypeClass(
    const css::uno::Reference<css::xforms::XDataTypeRepository>& xRepository,
    const SvXMLNamespaceMap& rNamespaceMap,
    const OUString& rXMLName );

OUString xforms_getTypeName(
    const css::uno::Reference<css::xforms::XDataTypeRepository>& xRepository,
    const SvXMLNamespaceMap& rNamespaceMap,
    const OUString& rXMLName );

OUString xforms_getBasicTypeName(
    const css::uno::Reference<css::xforms::XDataTypeRepository>& xRepository,
    const SvXMLNamespaceMap& rNamespaceMap,
    const OUString& rXMLName );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
