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

#ifndef INCLUDED_XMLOFF_XMLEVENTEXPORT_HXX
#define INCLUDED_XMLOFF_XMLEVENTEXPORT_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <xmloff/xmlevent.hxx>

#include <map>
#include <memory>

class SvXMLExport;
namespace com { namespace sun { namespace star {
    namespace document { class XEventsSupplier; }
    namespace container { class XNameReplace; }
    namespace container { class XNameAccess; }
    namespace beans { struct PropertyValue; }
} } }

namespace com { namespace sun { namespace star { namespace uno { template <class interface_type> class Reference; } } } }
namespace com { namespace sun { namespace star { namespace uno { template <typename > class Sequence; } } } }

typedef ::std::map< OUString, std::unique_ptr<XMLEventExportHandler> > HandlerMap;
typedef ::std::map< OUString, XMLEventName > NameMap;

/**
 * Export instances of EventsSupplier services. To use this class you
 * must fulfill two conditions:
 *
 * 1) provide a translation from the API event names to XML event
 * names
 * 2) Register XMLEventExportHandler instances for all script types
 * that you would like to export.
 *
 * The Export()-Methods all have a bUseWhitespace parameter that
 * causes the exported elements to be surrounded by whitespace, which
 * in turn causes the elements to be indented properly. By default,
 * whitespace is used, but it may not be called for in all cases (e.g
 * events attached to hyperlink within a paragraph.)
 */
class XMLOFF_DLLPUBLIC XMLEventExport
{
    SvXMLExport& rExport;

    HandlerMap aHandlerMap;
    NameMap aNameTranslationMap;

    bool bExtNamespace;

public:
    XMLEventExport(SvXMLExport& rExport);
    ~XMLEventExport();

    XMLEventExport& operator=( XMLEventExport const & ) = delete; // MSVC2017 workaround
    XMLEventExport( XMLEventExport const & ) = delete; // MSVC2017 workaround

    /// register an EventExportHandler for a particular script type
    ///
    /// The handlers will be deleted when the object is destroyed, hence
    /// no pointers to a handler registered with AddHandler() should be
    /// held by anyone.
    void AddHandler( const OUString& rName,
                     std::unique_ptr<XMLEventExportHandler> pHandler );

    /// register additional event names
    void AddTranslationTable( const XMLEventNameTranslation* pTransTable );

    /// export the events (calls EventExport::Export(Reference<XNameAccess>) )
    void Export( css::uno::Reference<css::document::XEventsSupplier> const & xAccess,
                bool bUseWhitespace = true);

    /// export the events (calls EventExport::Export(Reference<XNameAccess>) )
    void Export( css::uno::Reference<css::container::XNameReplace> const & xAccess,
                bool bUseWhitespace = true);

    /// export the events (writes <office:events> element)
    void Export( css::uno::Reference<css::container::XNameAccess> const & xAccess,
                bool bUseWhitespace = true);

    /// export the events, but write <officeooo:events> element
    /// (for new file format additions)
    void ExportExt( css::uno::Reference<css::container::XNameAccess> const & xAccess);

    /// export a single event (writes <office:events> element)
    void ExportSingleEvent(
        css::uno::Sequence<css::beans::PropertyValue>& rEventValues,
        const OUString& rApiEventName,
        bool bUseWhitespace = true );

private:

    /// export one event (start container-element if necessary)
    SAL_DLLPRIVATE void ExportEvent(
        css::uno::Sequence<css::beans::PropertyValue>& rEventValues,
        const XMLEventName& rXmlEventName,
        bool bUseWhitespace,
        bool& rExported);

    /// export the start element
    SAL_DLLPRIVATE void StartElement(bool bUseWhitespace);

    /// export the end element
    SAL_DLLPRIVATE void EndElement(bool bUseWhitespace);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
