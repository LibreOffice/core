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
    
    FactoryMap::iterator aEnd = aFactoryMap.end();
    for(FactoryMap::iterator aIter = aFactoryMap.begin();
        aIter != aEnd;
        ++aIter)
    {
        delete aIter->second;
    }
    aFactoryMap.clear();

    
    delete pEventNameMap;
}

void XMLEventImportHelper::RegisterFactory(
    const OUString& rLanguage,
    XMLEventContextFactory* pFactory )
{
    DBG_ASSERT(pFactory != NULL, "I need a factory.");
    if (NULL != pFactory)
    {
        aFactoryMap[rLanguage] = pFactory;
    }
}

void XMLEventImportHelper::AddTranslationTable(
    const XMLEventNameTranslation* pTransTable )
{
    if (NULL != pTransTable)
    {
        
        for(const XMLEventNameTranslation* pTrans = pTransTable;
            pTrans->sAPIName != NULL;
            pTrans++)
        {
            XMLEventName aName( pTrans->nPrefix, pTrans->sXMLName );

            
            DBG_ASSERT(pEventNameMap->find(aName) == pEventNameMap->end(),
                       "conflicting event translations");

            
            (*pEventNameMap)[aName] =
                OUString::createFromAscii(pTrans->sAPIName);
        }
    }
    
}

void XMLEventImportHelper::PushTranslationTable()
{
    
    aEventNameMapList.push_back(pEventNameMap);
    pEventNameMap = new NameMap();
}

void XMLEventImportHelper::PopTranslationTable()
{
    DBG_ASSERT(aEventNameMapList.size() > 0,
               "no translation tables left to pop");
    if ( !aEventNameMapList.empty() )
    {
        
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
    SvXMLImportContext* pContext = NULL;

    
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

        
        FactoryMap::iterator aFactoryIterator =
            aFactoryMap.find(aScriptLanguage);
        if (aFactoryIterator != aFactoryMap.end())
        {
            
            pContext = aFactoryIterator->second->CreateContext(
                rImport, nPrefix, rLocalName, xAttrList,
                rEvents, aNameIter->second, aScriptLanguage);
        }
    }

    
    if( NULL == pContext )
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
