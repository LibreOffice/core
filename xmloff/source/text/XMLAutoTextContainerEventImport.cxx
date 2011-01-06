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
#include "XMLAutoTextContainerEventImport.hxx"
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <xmloff/xmlimp.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/XMLEventsImportContext.hxx>


using namespace ::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::container::XNameReplace;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_EVENT_LISTENERS;


TYPEINIT1(XMLAutoTextContainerEventImport, SvXMLImportContext);


XMLAutoTextContainerEventImport::XMLAutoTextContainerEventImport(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const Reference<XNameReplace> & rEvnts ) :
        SvXMLImportContext(rImport, nPrfx, rLName),
        rEvents(rEvnts)
{
}

XMLAutoTextContainerEventImport::~XMLAutoTextContainerEventImport()
{
}

SvXMLImportContext* XMLAutoTextContainerEventImport::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & )
{
    if ( (XML_NAMESPACE_OFFICE == nPrefix) &&
         IsXMLToken( rLocalName, XML_EVENT_LISTENERS)   )
    {
        return new XMLEventsImportContext(GetImport(), nPrefix, rLocalName,
                                          rEvents);
    }
    else
        return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}
