/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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

TYPEINIT1(XMLEventsImportContext,  SvXMLImportContext);


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


}


void XMLEventsImportContext::StartElement(
    const Reference<XAttributeList> &)
{
    
}

void XMLEventsImportContext::EndElement()
{
    
}

SvXMLImportContext* XMLEventsImportContext::CreateChildContext(
    sal_uInt16 p_nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    
    
    
    
    

    
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
            
        }
        
    }

    
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

sal_Bool XMLEventsImportContext::GetEventSequence(
    const OUString& rName,
    Sequence<PropertyValue> & rSequence )
{
    
    
    

    
    EventsVector::iterator aIter = aCollectEvents.begin();
    while( (aIter != aCollectEvents.end()) && (aIter->first != rName) )
    {
        ++aIter;
    }

    
    sal_Bool bRet = (aIter != aCollectEvents.end());
    if (bRet)
        rSequence = aIter->second;

    return bRet;
}

void XMLEventsImportContext::AddEventValues(
    const OUString& rEventName,
    const Sequence<PropertyValue> & rValues )
{
    
    if (xEvents.is())
    {
        
        if (xEvents->hasByName(rEventName))
        {
            Any aAny;
            aAny <<= rValues;

            try
            {
                xEvents->replaceByName(rEventName, aAny);
            } catch ( const IllegalArgumentException & rException )
            {
                Sequence<OUString> aMsgParams(1);

                aMsgParams[0] = rEventName;

                GetImport().SetError(XMLERROR_FLAG_ERROR |
                                     XMLERROR_ILLEGAL_EVENT,
                                     aMsgParams, rException.Message, 0);
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
