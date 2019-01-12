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

#ifndef INCLUDED_XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX
#define INCLUDED_XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <com/sun/star/uno/Reference.hxx>
#include <xmloff/xmlictxt.hxx>

#include <vector>

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { struct PropertyValue; }
    namespace container { class XNameReplace; }
    namespace document { class XEventsSupplier; }
} } }

namespace com { namespace sun { namespace star { namespace uno { template <class E> class Sequence; } } } }

typedef ::std::pair<
            OUString,
            css::uno::Sequence<css::beans::PropertyValue> > EventNameValuesPair;

typedef ::std::vector< EventNameValuesPair > EventsVector;

/**
 * Import <script:events> element.
 *
 * The import context usually sets the events immediately at the event
 * XNameReplace. If none was given on construction, it operates in
 * delayed mode: All events are collected and may then be set
 * with the setEvents() method.
 */
class XMLOFF_DLLPUBLIC XMLEventsImportContext : public SvXMLImportContext
{
    // the event XNameReplace; may be empty
    css::uno::Reference<css::container::XNameReplace> xEvents;

protected:
    // if no XNameReplace is given, use this vector to collect events
    EventsVector aCollectEvents;

public:


    XMLEventsImportContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLocalName);

    XMLEventsImportContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference<css::document::XEventsSupplier> & xEventsSupplier);

    XMLEventsImportContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference<css::container::XNameReplace> & xNameRepl);

    virtual ~XMLEventsImportContext() override;

    void AddEventValues(
        const OUString& rEventName,
        const css::uno::Sequence<css::beans::PropertyValue> & rValues);

    /// if the import operates in delayed mode, you can use this method
    /// to set all events that have been read on the XEventsSupplier
    void SetEvents(
        const css::uno::Reference<css::document::XEventsSupplier> & xEventsSupplier);

    /// if the import operates in delayed mode, you can use this method
    /// to set all events that have been read on the XNameReplace
    void SetEvents(
        const css::uno::Reference<css::container::XNameReplace> & xNameRepl);

    /// if the import operates in delayed mode, you can use this method
    /// to obtain the value sequence for a specific event
    void GetEventSequence(
        const OUString& rName,
        css::uno::Sequence<css::beans::PropertyValue> & rSequence );

protected:

    virtual void StartElement(
        const css::uno::Reference<css::xml::sax::XAttributeList> & xAttrList) override;

    virtual void EndElement() override;

    virtual SvXMLImportContextRef CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference<css::xml::sax::XAttributeList> & xAttrList ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
