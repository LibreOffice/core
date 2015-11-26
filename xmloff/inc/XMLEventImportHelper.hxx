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

#ifndef INCLUDED_XMLOFF_INC_XMLEVENTIMPORTHELPER_HXX
#define INCLUDED_XMLOFF_INC_XMLEVENTIMPORTHELPER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <xmloff/xmlevent.hxx>

#include <map>
#include <list>


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
} } }
class XMLEventContextFactory;
class XMLEventsImportContext;
struct XMLEventNameTranslation;

typedef ::std::map< OUString, XMLEventContextFactory* > FactoryMap;
typedef ::std::map< XMLEventName, OUString > NameMap;


/**
 * Helps the XMLEventsImportContext.
 *
 * This class stores
 * a) the translation from XML event names to API event names, and
 * b) a mapping from script language names to XMLEventContextFactory objects
 *    (that handle particular languages).
 *
 * Event name translation tables may be added, i.e. they will be joined
 * together. If different translations are needed (i.e., if the same XML name
 * needs to be translated to different API names in different contexts), then
 * translation tables may be saved on a translation table stack.
 */
class XMLEventImportHelper
{
    /// map of XMLEventContextFactory objects
    FactoryMap aFactoryMap;

    /// map from XML to API names
    NameMap* pEventNameMap;

    /// stack of previous aEventNameMap
    std::list< NameMap* > aEventNameMapList;

public:
    XMLEventImportHelper();

    ~XMLEventImportHelper();

    /// register a handler for a particular language type
    void RegisterFactory( const OUString& rLanguage,
                          XMLEventContextFactory* aFactory );

    /// add event name translation to the internal table
    void AddTranslationTable( const XMLEventNameTranslation* pTransTable );

    /// save the old translation table on a stack and install an empty table
    void PushTranslationTable();

    /// recover the top-most previously saved translation table
    void PopTranslationTable();

    /// create an appropriate import context for a particular event
    SvXMLImportContext* CreateContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList> & xAttrList,
        XMLEventsImportContext* rEvents,
        const OUString& rXmlEventName,
        const OUString& rLanguage);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
