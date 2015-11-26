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

#include "XMLAutoTextEventImport.hxx"
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "XMLAutoTextContainerEventImport.hxx"
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <tools/debug.hxx>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Type;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::document::XEventsSupplier;
using ::com::sun::star::container::XNameReplace;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_AUTO_TEXT_EVENTS;

XMLAutoTextEventImport::XMLAutoTextEventImport(
    const css::uno::Reference< css::uno::XComponentContext >& xContext) throw()
:   SvXMLImport(xContext, "")
{
}

XMLAutoTextEventImport::~XMLAutoTextEventImport() throw()
{
}

void XMLAutoTextEventImport::initialize(
    const Sequence<Any> & rArguments )
        throw(Exception, RuntimeException, std::exception)
{
    // The events may come as either an XNameReplace or XEventsSupplier.

    const sal_Int32 nLength = rArguments.getLength();
    for( sal_Int32 i = 0; i < nLength; i++ )
    {
        const Type& rType = rArguments[i].getValueType();
        if ( rType == cppu::UnoType<XEventsSupplier>::get())
        {
            Reference<XEventsSupplier> xSupplier;
            rArguments[i] >>= xSupplier;
            DBG_ASSERT(xSupplier.is(), "need XEventsSupplier or XNameReplace");

            xEvents = xSupplier->getEvents();
        }
        else if (rType == cppu::UnoType<XNameReplace>::get())
        {
            rArguments[i] >>= xEvents;
            DBG_ASSERT(xEvents.is(), "need XEventsSupplier or XNameReplace");
        }
    }

    // call parent
    SvXMLImport::initialize(rArguments);
}



SvXMLImportContext* XMLAutoTextEventImport::CreateContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList > & xAttrList )
{
    if ( xEvents.is() && (XML_NAMESPACE_OOO == nPrefix) &&
         IsXMLToken( rLocalName, XML_AUTO_TEXT_EVENTS) )
    {
        return new XMLAutoTextContainerEventImport(
            *this, nPrefix, rLocalName, xEvents);
    }
    else
    {
        return SvXMLImport::CreateContext(nPrefix, rLocalName, xAttrList);
    }
}


Sequence< OUString > SAL_CALL
    XMLAutoTextEventImport_getSupportedServiceNames()
        throw()
{
    Sequence<OUString> aSeq { XMLAutoTextEventImport_getImplementationName() };
    return aSeq;
}

OUString SAL_CALL XMLAutoTextEventImport_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Writer.XMLOasisAutotextEventsImporter" );
}

Reference< XInterface > SAL_CALL XMLAutoTextEventImport_createInstance(
        const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return static_cast<cppu::OWeakObject*>(new XMLAutoTextEventImport( comphelper::getComponentContext(rSMgr) ));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
