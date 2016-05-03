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

#include <xmloff/XMLEventsImportContext.hxx>

#include "XMLEventImportHelper.hxx"

#include <com/sun/star/document/XEventsSupplier.hpp>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlerror.hxx>

using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::container::XNameReplace;
using ::com::sun::star::document::XEventsSupplier;
using ::com::sun::star::lang::IllegalArgumentException;


XMLEventsImportContext::XMLEventsImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName) :
        SvXMLImportContext(rImport, nPrfx, rLocalName)
{
}


XMLEventsImportContext::XMLEventsImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const Reference<XEventsSupplier> & xEventsSupplier) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        xEvents(xEventsSupplier->getEvents())
{
}


XMLEventsImportContext::XMLEventsImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const Reference<XNameReplace> & xNameReplace) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        xEvents(xNameReplace)
{
}

XMLEventsImportContext::~XMLEventsImportContext()
{
//  // if, for whatever reason, the object gets destroyed prematurely,
//  // we need to delete the collected events
}


void XMLEventsImportContext::StartElement(
    const Reference<XAttributeList> &)
{
    // nothing to be done
}

void XMLEventsImportContext::EndElement()
{
    // nothing to be done
}

SvXMLImportContext* XMLEventsImportContext::CreateChildContext(
    sal_uInt16 p_nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    // a) search for script:language and script:event-name attribute
    // b) delegate to factory. The factory will:
    //    1) translate XML event name into API event name
    //    2) get proper event context factory from import
    //    3) instantiate context

    // a) search for script:language and script:event-name attribute
    OUString sLanguage;
    OUString sEventName;
    sal_Int16 nCount = xAttrList->getLength();
    for (sal_Int16 nAttr = 0; nAttr < nCount; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr), &sLocalName );

        if (XML_NAMESPACE_SCRIPT == nPrefix)
        {
            if (IsXMLToken(sLocalName, XML_EVENT_NAME))
            {
                sEventName = xAttrList->getValueByIndex(nAttr);
            }
            else if (IsXMLToken(sLocalName, XML_LANGUAGE))
            {
                sLanguage = xAttrList->getValueByIndex(nAttr);
            }
            // else: ignore -> let child context handle this
        }
        // else: ignore -> let child context handle this
    }

    // b) delegate to factory
    return GetImport().GetEventImport().CreateContext(
        GetImport(), p_nPrefix, rLocalName, xAttrList,
        this, sEventName, sLanguage);
}

void XMLEventsImportContext::SetEvents(
    const Reference<XEventsSupplier> & xEventsSupplier)
{
    if (xEventsSupplier.is())
    {
        SetEvents(xEventsSupplier->getEvents());
    }
}

void XMLEventsImportContext::SetEvents(
    const Reference<XNameReplace> & xNameRepl)
{
    if (xNameRepl.is())
    {
        xEvents = xNameRepl;

        // now iterate over vector and a) insert b) delete all elements
        EventsVector::iterator aEnd = aCollectEvents.end();
        for(EventsVector::iterator aIter = aCollectEvents.begin();
            aIter != aEnd;
            ++aIter)
        {
            AddEventValues(aIter->first, aIter->second);
        }
        aCollectEvents.clear();
    }
}

bool XMLEventsImportContext::GetEventSequence(
    const OUString& rName,
    Sequence<PropertyValue> & rSequence )
{
    // search through the vector
    // (This shouldn't take a lot of time, since this method should only get
    //  called if only one (or few) events are being expected)

    // iterate over vector until end or rName is found;
    EventsVector::iterator aIter = aCollectEvents.begin();
    while( (aIter != aCollectEvents.end()) && (aIter->first != rName) )
    {
        ++aIter;
    }

    // if we're not at the end, set the sequence
    if (aIter != aCollectEvents.end())
    {
        rSequence = aIter->second;
        return true;
    }

    return false;
}

void XMLEventsImportContext::AddEventValues(
    const OUString& rEventName,
    const Sequence<PropertyValue> & rValues )
{
    // if we already have the events, set them; else just collect
    if (xEvents.is())
    {
        // set event (if name is known)
        if (xEvents->hasByName(rEventName))
        {
            try
            {
                xEvents->replaceByName(rEventName, Any(rValues));
            } catch ( const IllegalArgumentException & rException )
            {
                Sequence<OUString> aMsgParams { rEventName };

                GetImport().SetError(XMLERROR_FLAG_ERROR |
                                     XMLERROR_ILLEGAL_EVENT,
                                     aMsgParams, rException.Message, nullptr);
            }
        }
    }
    else
    {
        EventNameValuesPair aPair(rEventName, rValues);
        aCollectEvents.push_back(aPair);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
