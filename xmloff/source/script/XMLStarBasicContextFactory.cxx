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
#include <XMLStarBasicContextFactory.hxx>
#include <xmloff/XMLEventsImportContext.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <o3tl/string_view.hxx>


using namespace ::xmloff::token;

using ::com::sun::star::xml::sax::XFastAttributeList;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;


constexpr OUString gsEventType(u"EventType"_ustr);
constexpr OUString gsLibrary(u"Library"_ustr);
constexpr OUString gsMacroName(u"MacroName"_ustr);
constexpr OUString gsStarBasic(u"StarBasic"_ustr);

XMLStarBasicContextFactory::XMLStarBasicContextFactory()
{
}

XMLStarBasicContextFactory::~XMLStarBasicContextFactory()
{
}

SvXMLImportContext* XMLStarBasicContextFactory::CreateContext(
    SvXMLImport& rImport,
    const Reference<XFastAttributeList> & xAttrList,
    XMLEventsImportContext* rEvents,
    const OUString& rApiEventName)
{
    OUString sLibraryVal;
    OUString sMacroNameVal;

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        if (aIter.getToken() == XML_ELEMENT(SCRIPT, XML_MACRO_NAME))
            sMacroNameVal = aIter.toString();
        // else: ignore
    }

    const OUString& rApp = GetXMLToken( XML_APPLICATION );
    const OUString& rDoc = GetXMLToken( XML_DOCUMENT );
    if( sMacroNameVal.getLength() > rApp.getLength()+1 &&
        o3tl::equalsIgnoreAsciiCase(sMacroNameVal.subView(0,rApp.getLength()), rApp ) &&
        ':' == sMacroNameVal[rApp.getLength()] )
    {
        sLibraryVal = "StarOffice";
        sMacroNameVal = sMacroNameVal.copy( rApp.getLength()+1 );
    }
    else if( sMacroNameVal.getLength() > rDoc.getLength()+1 &&
        o3tl::equalsIgnoreAsciiCase(sMacroNameVal.subView(0,rDoc.getLength()), rDoc ) &&
        ':' == sMacroNameVal[rDoc.getLength()] )
    {
        sLibraryVal = rDoc;
        sMacroNameVal = sMacroNameVal.copy( rDoc.getLength()+1 );
    }

    if (!sMacroNameVal.isEmpty())
        rImport.NotifyMacroEventRead();

    Sequence<PropertyValue> aValues
    {
        comphelper::makePropertyValue(gsEventType, gsStarBasic),
        comphelper::makePropertyValue(gsLibrary, sLibraryVal),
        comphelper::makePropertyValue(gsMacroName, sMacroNameVal)
    };

    // add values for event now
    rEvents->AddEventValues(rApiEventName, aValues);

    // return dummy context
    return new SvXMLImportContext(rImport);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
