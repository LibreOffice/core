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


#include "XMLEventImportHelper.hxx"
#include <tools/debug.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlerror.hxx>

using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

XMLEventImportHelper::XMLEventImportHelper() :
    aFactoryMap(),
    pEventNameMap(new NameMap()),
    aEventNameMapList()
{
}


XMLEventImportHelper::~XMLEventImportHelper()
{
    // delete factories
    FactoryMap::iterator aEnd = aFactoryMap.end();
    for(FactoryMap::iterator aIter = aFactoryMap.begin();
        aIter != aEnd;
        ++aIter)
    {
        delete aIter->second;
    }
    aFactoryMap.clear();

    // delete name map
    delete pEventNameMap;
}

void XMLEventImportHelper::RegisterFactory(
    const OUString& rLanguage,
    XMLEventContextFactory* pFactory )
{
    DBG_ASSERT(pFactory != nullptr, "I need a factory.");
    if (nullptr != pFactory)
    {
        aFactoryMap[rLanguage] = pFactory;
    }
}

void XMLEventImportHelper::AddTranslationTable(
    const XMLEventNameTranslation* pTransTable )
{
    if (nullptr != pTransTable)
    {
        // put translation table into map
        for(const XMLEventNameTranslation* pTrans = pTransTable;
            pTrans->sAPIName != nullptr;
            pTrans++)
        {
            XMLEventName aName( pTrans->nPrefix, pTrans->sXMLName );

            // check for conflicting entries
            DBG_ASSERT(pEventNameMap->find(aName) == pEventNameMap->end(),
                       "conflicting event translations");

            // assign new translation
            (*pEventNameMap)[aName] =
                OUString::createFromAscii(pTrans->sAPIName);
        }
    }
    // else? ignore!
}

void XMLEventImportHelper::PushTranslationTable()
{
    // save old map and install new one
    aEventNameMapList.push_back(pEventNameMap);
    pEventNameMap = new NameMap();
}

void XMLEventImportHelper::PopTranslationTable()
{
    DBG_ASSERT(aEventNameMapList.size() > 0,
               "no translation tables left to pop");
    if ( !aEventNameMapList.empty() )
    {
        // delete current and install old map
        delete pEventNameMap;
        pEventNameMap = aEventNameMapList.back();
        aEventNameMapList.pop_back();
    }
}


SvXMLImportContext* XMLEventImportHelper::CreateContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList,
    XMLEventsImportContext* rEvents,
    const OUString& rXmlEventName,
    const OUString& rLanguage)
{
    SvXMLImportContext* pContext = nullptr;

    // translate event name form xml to api
    OUString sMacroName;
    sal_uInt16 nMacroPrefix =
        rImport.GetNamespaceMap().GetKeyByAttrName( rXmlEventName,
                                                        &sMacroName );
    XMLEventName aEventName( nMacroPrefix, sMacroName );
    NameMap::iterator aNameIter = pEventNameMap->find(aEventName);
    if (aNameIter != pEventNameMap->end())
    {
        OUString aScriptLanguage;
        sal_uInt16 nScriptPrefix = rImport.GetNamespaceMap().
                GetKeyByAttrName( rLanguage, &aScriptLanguage );
        if( XML_NAMESPACE_OOO != nScriptPrefix )
            aScriptLanguage = rLanguage ;

        // check for factory
        FactoryMap::iterator aFactoryIterator =
            aFactoryMap.find(aScriptLanguage);
        if (aFactoryIterator != aFactoryMap.end())
        {
            // delegate to factory
            pContext = aFactoryIterator->second->CreateContext(
                rImport, nPrefix, rLocalName, xAttrList,
                rEvents, aNameIter->second, aScriptLanguage);
        }
    }

    // default context (if no context was created above)
    if( nullptr == pContext )
    {
        pContext = new SvXMLImportContext(rImport, nPrefix, rLocalName);

        Sequence<OUString> aMsgParams(2);

        aMsgParams[0] = rXmlEventName;
        aMsgParams[1] = rLanguage;

        rImport.SetError(XMLERROR_FLAG_ERROR | XMLERROR_ILLEGAL_EVENT,
                         aMsgParams);

    }

    return pContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
