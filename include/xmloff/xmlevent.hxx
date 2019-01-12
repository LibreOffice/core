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

#ifndef INCLUDED_XMLOFF_XMLEVENT_HXX
#define INCLUDED_XMLOFF_XMLEVENT_HXX

#include <rtl/ustring.hxx>

namespace com { namespace sun { namespace star { namespace uno { template <class interface_type> class Reference; } } } }
namespace com { namespace sun { namespace star { namespace uno { template <typename > class Sequence; } } } }


/**
 * @#file
 *
 * Several definition used in im- and export of events
 */

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { struct PropertyValue; }
} } }

class SvXMLExport;
class SvXMLImportContext;
class SvXMLImport;
class XMLEventsImportContext;


struct XMLEventName
{
    sal_uInt16 m_nPrefix;
    OUString m_aName;

    XMLEventName() : m_nPrefix( 0 ) {}
    XMLEventName( sal_uInt16 n, const sal_Char *p ) :
        m_nPrefix( n ),
        m_aName( OUString::createFromAscii(p) )
       {}

    XMLEventName( sal_uInt16 n, const OUString& r ) :
        m_nPrefix( n ),
        m_aName( r )
       {}

    bool operator<( const XMLEventName& r ) const
    {
        return m_nPrefix < r.m_nPrefix ||
               (m_nPrefix == r.m_nPrefix && m_aName < r.m_aName );
    }

};

/**
 * XMLEventNameTranslation: define tables that translate between event names
 * as used in the XML file format and in the StarOffice API.
 * The last entry in the table must be { NULL, 0, NULL }.
 */
struct XMLEventNameTranslation
{
    const sal_Char* sAPIName;
    sal_uInt16 const nPrefix;    // namespace prefix
    const sal_Char* sXMLName;
};

/// a translation table for the events defined in the XEventsSupplier service
/// (implemented in XMLEventExport.cxx)
extern const XMLEventNameTranslation aStandardEventTable[];


/**
 * Handle export of an event for a certain event type (event type as
 * defined by the PropertyValue "EventType" in API).
 *
 * The Handler has to generate the full <script:event> element.
 */
class XMLEventExportHandler
{
public:
    virtual ~XMLEventExportHandler() {};

    virtual void Export(
        SvXMLExport& rExport,                   /// the current XML export
        const OUString& rEventQName,     /// the XML name of the event
        css::uno::Sequence<css::beans::PropertyValue> & rValues, /// the values for the event
        bool bUseWhitespace) = 0;  /// create whitespace around elements?
};


/**
 * Handle import of an event for a certain event type (as defined by
 * the PropertyValue "EventType" in the API).
 *
 * EventContextFactories must be registered with the EventImportHelper
 * that is attached to the SvXMLImport.
 *
 * The factory has to create an import context for a <script:event>
 * element.  The context has to call the
 * EventsImportContext::AddEventValues() method to save its event
 * registered with the enclosing element. For events consisting only
 * of attributes (and an empty element) an easy solution is to handle
 * all attributes in the CreateContext()-method and return a default
 * context.
 *
 * EventContextFactory objects have to be registered with the
 * EventsImportHelper.
 */
class XMLEventContextFactory
{
public:
    virtual ~XMLEventContextFactory() {};

    virtual SvXMLImportContext* CreateContext(
        SvXMLImport& rImport,               /// import context
        sal_uInt16 nPrefix,                 /// element: namespace prefix
        const OUString& rLocalName,  /// element: local name
        const css::uno::Reference<css::xml::sax::XAttributeList> & xAttrList, /// attribute list
        /// the context for the enclosing <script:events> element
        XMLEventsImportContext* rEvents,
        /// the event name (as understood by the API)
        const OUString& rApiEventName,
        /// the event type name (as registered)
        const OUString& rApiLanguage) = 0;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
