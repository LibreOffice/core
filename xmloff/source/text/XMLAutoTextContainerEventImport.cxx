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

#include "XMLAutoTextContainerEventImport.hxx"
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/container/XNameReplace.hpp>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/XMLEventsImportContext.hxx>
#include <sal/log.hxx>


using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::container::XNameReplace;
using ::xmloff::token::XML_EVENT_LISTENERS;


XMLAutoTextContainerEventImport::XMLAutoTextContainerEventImport(
    SvXMLImport& rImport,
    const Reference<XNameReplace> & rEvnts ) :
        SvXMLImportContext(rImport),
        rEvents(rEvnts)
{
}

XMLAutoTextContainerEventImport::~XMLAutoTextContainerEventImport()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLAutoTextContainerEventImport::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& )
{
    if ( nElement == XML_ELEMENT(OFFICE, XML_EVENT_LISTENERS) )
    {
        return new XMLEventsImportContext(GetImport(), rEvents);
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
