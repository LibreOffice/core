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

#include "XMLEventsImportContext.hxx"

#include "XMLEventImportHelper.hxx"

#include <com/sun/star/document/XEventsSupplier.hpp>


#include "xmlimp.hxx"

#include "nmspmap.hxx"

#include "xmlnmspe.hxx"


#include "xmlerror.hxx"
namespace binfilter {

using namespace ::com::sun::star::uno;
using namespace ::binfilter::xmloff::token;

using ::rtl::OUString;
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


XMLEventsImportContext::~XMLEventsImportContext()
{
// 	// if, for whatever reason, the object gets destroyed prematurely,
// 	// we need to delete the collected events
// 	EventsVector::iterator aEnd = aCollectEvents.end();
// 	for(EventsVector::iterator aIter = aCollectEvents.begin();
// 		aIter != aEnd;
// 		aIter++)
// 	{
// 		EventNameValuesPair* pPair = &(*aIter);
// 		delete pPair;
// 	}
// 	aCollectEvents.clear();
}


void XMLEventsImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    // nothing to be done
}

void XMLEventsImportContext::EndElement()
{
    // nothing to be done
}

SvXMLImportContext* XMLEventsImportContext::CreateChildContext( 
    sal_uInt16 nPrefix,
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
        GetImport(), nPrefix, rLocalName, xAttrList, 
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
            aIter++)
        {
            AddEventValues(aIter->first, aIter->second);
// 			EventNameValuesPair* pPair = &(*aIter);
// 			delete pPair;
        }
        aCollectEvents.clear();
    }
}

sal_Bool XMLEventsImportContext::GetEventSequence(
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
        aIter++;
    }
    
    // if we're not at the end, set the sequence
    sal_Bool bRet = (aIter != aCollectEvents.end());
    if (bRet)
        rSequence = aIter->second;

    return bRet;
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
// 		EventNameValuesPair* aPair = new EventNameValuesPair(rEventName, 
// 															rValues);
// 		aCollectEvents.push_back(*aPair);
        EventNameValuesPair aPair(rEventName, rValues);
        aCollectEvents.push_back(aPair);
    }
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
