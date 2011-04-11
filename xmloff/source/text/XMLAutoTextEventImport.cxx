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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include "XMLAutoTextEventImport.hxx"
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "XMLAutoTextContainerEventImport.hxx"
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <tools/debug.hxx>

using namespace ::com::sun::star;

using ::rtl::OUString;
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

const sal_Char sAPI_AutoText[] = "com.sun.star.text.AutoTextContainer";


// #110680#
XMLAutoTextEventImport::XMLAutoTextEventImport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory) throw()
:   SvXMLImport(xServiceFactory)
{
}

XMLAutoTextEventImport::~XMLAutoTextEventImport() throw()
{
}

void XMLAutoTextEventImport::initialize(
    const Sequence<Any> & rArguments )
        throw(Exception, RuntimeException)
{
    // The events may come as either an XNameReplace or XEventsSupplier.

    const sal_Int32 nLength = rArguments.getLength();
    for( sal_Int32 i = 0; i < nLength; i++ )
    {
        const Type& rType = rArguments[i].getValueType();
        if ( rType == ::getCppuType( (Reference<XEventsSupplier>*)NULL ) )
        {
            Reference<XEventsSupplier> xSupplier;
            rArguments[i] >>= xSupplier;
            DBG_ASSERT(xSupplier.is(), "need XEventsSupplier or XNameReplace");

            xEvents = xSupplier->getEvents();
        }
        else if (rType == ::getCppuType( (Reference<XNameReplace>*)NULL ) )
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
    Sequence< OUString > aSeq( 1 );
    aSeq[0] = XMLAutoTextEventImport_getImplementationName();
    return aSeq;
}

OUString SAL_CALL XMLAutoTextEventImport_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.Writer.XMLOasisAutotextEventsImporter" ) );
}

Reference< XInterface > SAL_CALL XMLAutoTextEventImport_createInstance(
        const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new XMLAutoTextEventImport;
    return (cppu::OWeakObject*)new XMLAutoTextEventImport(rSMgr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
