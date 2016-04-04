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

#include "XMLStarBasicContextFactory.hxx"
#include <xmloff/XMLEventsImportContext.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>


using namespace ::xmloff::token;

using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;


XMLStarBasicContextFactory::XMLStarBasicContextFactory() :
    sEventType("EventType"),
    sLibrary("Library"),
    sMacroName("MacroName"),
    sStarBasic("StarBasic")
{
}

XMLStarBasicContextFactory::~XMLStarBasicContextFactory()
{
}

SvXMLImportContext* XMLStarBasicContextFactory::CreateContext(
    SvXMLImport& rImport,
    sal_uInt16 p_nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList,
    XMLEventsImportContext* rEvents,
    const OUString& rApiEventName,
    const OUString& /*rApiLanguage*/)
{
    OUString sLibraryVal;
    OUString sMacroNameVal;

    sal_Int16 nCount = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nCount; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr), &sLocalName );

        if (XML_NAMESPACE_SCRIPT == nPrefix)
        {
            if (IsXMLToken(sLocalName, XML_MACRO_NAME))
            {
                sMacroNameVal = xAttrList->getValueByIndex(nAttr);
            }
            // else: ignore
        }
        // else: ignore
    }

    const OUString& rApp = GetXMLToken( XML_APPLICATION );
    const OUString& rDoc = GetXMLToken( XML_DOCUMENT );
    if( sMacroNameVal.getLength() > rApp.getLength()+1 &&
        sMacroNameVal.copy(0,rApp.getLength()).equalsIgnoreAsciiCase( rApp ) &&
        ':' == sMacroNameVal[rApp.getLength()] )
    {
        sLibraryVal = "StarOffice";
        sMacroNameVal = sMacroNameVal.copy( rApp.getLength()+1 );
    }
    else if( sMacroNameVal.getLength() > rDoc.getLength()+1 &&
        sMacroNameVal.copy(0,rDoc.getLength()).equalsIgnoreAsciiCase( rDoc ) &&
        ':' == sMacroNameVal[rDoc.getLength()] )
    {
        sLibraryVal = rDoc;
        sMacroNameVal = sMacroNameVal.copy( rDoc.getLength()+1 );
    }

    Sequence<PropertyValue> aValues(3);

    // EventType
    aValues[0].Name = sEventType;
    aValues[0].Value <<= sStarBasic;

    // library name
    aValues[1].Name = sLibrary;
    aValues[1].Value <<= sLibraryVal;

    // macro name
    aValues[2].Name = sMacroName;
    aValues[2].Value <<= sMacroNameVal;

    // add values for event now
    rEvents->AddEventValues(rApiEventName, aValues);

    // return dummy context
    return new SvXMLImportContext(rImport, p_nPrefix, rLocalName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
