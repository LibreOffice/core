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


#include <XMLEventImportHelper.hxx>
#include <tools/debug.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlerror.hxx>

using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

XMLEventImportHelper::XMLEventImportHelper() :
    aFactoryMap(),
    pEventNameMap(new NameMap),
    aEventNameMapVector()
{
}

XMLEventImportHelper::~XMLEventImportHelper()
{
    // delete factories
    aFactoryMap.clear();

    // delete name map
    pEventNameMap.reset();
}

void XMLEventImportHelper::RegisterFactory(
    const OUString& rLanguage,
    std::unique_ptr<XMLEventContextFactory> pFactory )
{
    assert(pFactory);
    aFactoryMap[rLanguage] = std::move(pFactory);
}

void XMLEventImportHelper::AddTranslationTable(
    const XMLEventNameTranslation* pTransTable )
{
    if (nullptr == pTransTable)
        return;

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
    // else? ignore!
}

void XMLEventImportHelper::PushTranslationTable()
{
    // save old map and install new one
    aEventNameMapVector.push_back(std::move(pEventNameMap));
    pEventNameMap.reset( new NameMap );
}

void XMLEventImportHelper::PopTranslationTable()
{
    DBG_ASSERT(!aEventNameMapVector.empty(),
               "no translation tables left to pop");
    if ( !aEventNameMapVector.empty() )
    {
        // delete current and install old map
        pEventNameMap = std::move(aEventNameMapVector.back());
        aEventNameMapVector.pop_back();
    }
}


SvXMLImportContext* XMLEventImportHelper::CreateContext(
    SvXMLImport& rImport,
    const Reference<XAttributeList> & xAttrList,
    XMLEventsImportContext* rEvents,
    const OUString& rXmlEventName,
    const OUString& rLanguage)
{
    rImport.NotifyMacroEventRead();

    SvXMLImportContext* pContext = nullptr;

    // translate event name from xml to api
    OUString sMacroName;
    sal_uInt16 nMacroPrefix =
        rImport.GetNamespaceMap().GetKeyByAttrValueQName(rXmlEventName,
                                                        &sMacroName );
    XMLEventName aEventName( nMacroPrefix, sMacroName );
    NameMap::iterator aNameIter = pEventNameMap->find(aEventName);
    if (aNameIter != pEventNameMap->end())
    {
        OUString aScriptLanguage;
        sal_uInt16 nScriptPrefix = rImport.GetNamespaceMap().
                GetKeyByAttrValueQName(rLanguage, &aScriptLanguage);
        if( XML_NAMESPACE_OOO != nScriptPrefix )
            aScriptLanguage = rLanguage ;

        // check for factory
        FactoryMap::iterator aFactoryIterator =
            aFactoryMap.find(aScriptLanguage);
        if (aFactoryIterator != aFactoryMap.end())
        {
            // delegate to factory
            pContext = aFactoryIterator->second->CreateContext(
                rImport, xAttrList,
                rEvents, aNameIter->second);
        }
    }

    // default context (if no context was created above)
    if( nullptr == pContext )
    {
        pContext = new SvXMLImportContext(rImport);

        Sequence<OUString> aMsgParams(2);

        aMsgParams[0] = rXmlEventName;
        aMsgParams[1] = rLanguage;

        rImport.SetError(XMLERROR_FLAG_ERROR | XMLERROR_ILLEGAL_EVENT,
                         aMsgParams);

    }

    return pContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
