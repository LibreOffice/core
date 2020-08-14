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

#include <XMLEventImportHelper.hxx>

#include <com/sun/star/document/XEventsSupplier.hpp>
#include <comphelper/attributelist.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/namespacemap.hxx>
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

XMLEventsImportContext::XMLEventsImportContext(SvXMLImport& rImport) :
    SvXMLImportContext(rImport)
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
    const Reference<XEventsSupplier> & xEventsSupplier) :
        SvXMLImportContext(rImport),
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

SvXMLImportContextRef XMLEventsImportContext::CreateChildContext(
    sal_uInt16 /*p_nPrefix*/,
    const OUString& /*rLocalName*/,
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
        GetImport(), xAttrList, this, sEventName, sLanguage);
}

void XMLEventsImportContext::startFastElement(sal_Int32 /*nElement*/,
                const css::uno::Reference< css::xml::sax::XFastAttributeList >&)
{
    // nothing to be done
}

void XMLEventsImportContext::endFastElement(sal_Int32 /*nElement*/)
{
    // nothing to be done
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLEventsImportContext::createFastChildContext(
    sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    // a) search for script:language and script:event-name attribute
    // b) delegate to factory. The factory will:
    //    1) translate XML event name into API event name
    //    2) get proper event context factory from import
    //    3) instantiate context

    // a) search for script:language and script:event-name attribute
    OUString sLanguage;
    OUString sEventName;
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        OUString sValue = aIter.toString();

        if (aIter.getToken() == XML_ELEMENT(SCRIPT, XML_EVENT_NAME))
        {
            sEventName = sValue;
        }
        else if (aIter.getToken() == XML_ELEMENT(SCRIPT, XML_EVENT_NAME))
        {
            sLanguage = sValue;
            // else: ignore -> let child context handle this
        }
        // else: ignore -> let child context handle this
    }

    rtl::Reference < comphelper::AttributeList > rAttrList = new comphelper::AttributeList;
    const Sequence< css::xml::FastAttribute > fastAttribs = xAttrList->getFastAttributes();
    for (const auto& rAttr : fastAttribs)
    {
        const OUString& rAttrValue = rAttr.Value;
        sal_Int32 nToken = rAttr.Token;
        const OUString& rAttrNamespacePrefix = SvXMLImport::getNamespacePrefixFromToken( nToken, nullptr );
        OUString sAttrName = SvXMLImport::getNameFromToken( nToken );
        if ( !rAttrNamespacePrefix.isEmpty() )
            sAttrName = rAttrNamespacePrefix + ":" + sAttrName;
        rAttrList->AddAttribute( sAttrName, "CDATA", rAttrValue );
    }

    // b) delegate to factory
    return GetImport().GetEventImport().CreateContext(
        GetImport(), rAttrList.get(), this, sEventName, sLanguage);
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
        for(const auto& rEvent : aCollectEvents)
        {
            AddEventValues(rEvent.first, rEvent.second);
        }
        aCollectEvents.clear();
    }
}

void XMLEventsImportContext::GetEventSequence(
    const OUString& rName,
    Sequence<PropertyValue> & rSequence )
{
    // search through the vector
    // (This shouldn't take a lot of time, since this method should only get
    //  called if only one (or few) events are being expected)

    auto aIter = std::find_if(aCollectEvents.begin(), aCollectEvents.end(),
        [&rName](EventNameValuesPair& rEvent) { return rEvent.first == rName; });

    // if we're not at the end, set the sequence
    if (aIter != aCollectEvents.end())
    {
        rSequence = aIter->second;
    }
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
