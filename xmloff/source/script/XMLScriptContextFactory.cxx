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

#include <comphelper/propertyvalue.hxx>
#include <XMLScriptContextFactory.hxx>
#include <xmloff/XMLEventsImportContext.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>

using namespace ::xmloff::token;

using ::com::sun::star::xml::sax::XFastAttributeList;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

constexpr OUString gsEventType(u"EventType"_ustr);
constexpr OUString gsScript(u"Script"_ustr);
constexpr OUString gsURL(u"Script"_ustr);

XMLScriptContextFactory::XMLScriptContextFactory() {}

XMLScriptContextFactory::~XMLScriptContextFactory() {}

SvXMLImportContext* XMLScriptContextFactory::CreateContext(
    SvXMLImport& rImport, const Reference<XFastAttributeList>& xAttrList,
    XMLEventsImportContext* rEvents, const OUString& rApiEventName)
{
    OUString sURLVal;

    for (auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        if (aIter.getToken() == XML_ELEMENT(XLINK, XML_HREF))
            sURLVal = aIter.toString();
        // else: ignore
    }

    if (!sURLVal.isEmpty())
        rImport.NotifyMacroEventRead();

    Sequence<PropertyValue> aValues{ comphelper::makePropertyValue(gsEventType, gsScript),
                                     comphelper::makePropertyValue(gsURL, sURLVal) };

    // add values for event now
    rEvents->AddEventValues(rApiEventName, aValues);

    // return dummy context
    return new SvXMLImportContext(rImport);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
