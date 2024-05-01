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

#include <xmloff/XMLEventExport.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <com/sun/star/document/XEventsSupplier.hpp>

#include <com/sun/star/container/XNameReplace.hpp>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/namespacemap.hxx>


using namespace ::com::sun::star::uno;

using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::document::XEventsSupplier;
using ::com::sun::star::container::XNameReplace;
using ::com::sun::star::container::XNameAccess;
using ::xmloff::token::XML_EVENT_LISTENERS;

XMLEventExport::XMLEventExport(SvXMLExport& rExp) :
    m_rExport(rExp),
    m_bExtNamespace(false)
{
}

XMLEventExport::~XMLEventExport()
{
    // delete all handlers
    m_aHandlerMap.clear();
}

void XMLEventExport::AddHandler( const OUString& rName,
                                 std::unique_ptr<XMLEventExportHandler> pHandler )
{
    assert(pHandler);
    m_aHandlerMap[rName] = std::move(pHandler);
}

void XMLEventExport::AddTranslationTable(
    const XMLEventNameTranslation* pTransTable )
{
    if (nullptr != pTransTable)
    {
        // put translation table into map
        for(const XMLEventNameTranslation* pTrans = pTransTable;
            !pTrans->sAPIName.isEmpty();
            pTrans++)
        {
            m_aNameTranslationMap[pTrans->sAPIName] =
                XMLEventName(pTrans->nPrefix, pTrans->sXMLName);
        }
    }
    // else? ignore!
}

void XMLEventExport::Export( Reference<XEventsSupplier> const & rSupplier,
                             bool bWhitespace)
{
    if (rSupplier.is())
    {
        Export(rSupplier->getEvents(), bWhitespace);
    }
    // else: no supplier, no export -> ignore!
}

void XMLEventExport::Export( Reference<XNameReplace> const & rReplace,
                             bool bWhitespace)
{
    Reference<XNameAccess> xAccess(rReplace);
    Export(xAccess, bWhitespace);
}

void XMLEventExport::Export( Reference<XNameAccess> const & rAccess,
                             bool bWhitespace)
{
    // early out if we don't actually get any events
    if (!rAccess.is())
    {
        return;
    }

    // have we already processed an element?
    bool bStarted = false;

    // iterate over all event types
    const Sequence<OUString> aNames = rAccess->getElementNames();
    for(const auto& rName : aNames)
    {
        // translate name
        NameMap::iterator aIter = m_aNameTranslationMap.find(rName);
        if (aIter != m_aNameTranslationMap.end())
        {
            const XMLEventName& rXmlName = aIter->second;

            // get PropertyValues for this event
            Any aAny = rAccess->getByName( rName );
            Sequence<PropertyValue> aValues;
            aAny >>= aValues;

            // now export the current event
            ExportEvent( aValues, rXmlName, bWhitespace, bStarted );
        }
        else
        {
            // don't proceed further
            SAL_WARN("xmloff", "Unknown event name:" << rName );
        }
    }

    // close <script:events> element (if it was opened before)
    if (bStarted)
    {
        EndElement(bWhitespace);
    }
}

void XMLEventExport::ExportExt( Reference<XNameAccess> const & rAccess )
{
    // set bExtNamespace flag to use XML_NAMESPACE_OFFICE_EXT namespace
    // for events element (not for child elements)
    m_bExtNamespace = true;
    Export(rAccess);
    m_bExtNamespace = false;          // reset for future Export calls
}

/// export a singular event and write <office:events> container
void XMLEventExport::ExportSingleEvent(
    const Sequence<PropertyValue>& rEventValues,
    const OUString& rApiEventName,
    bool bUseWhitespace )
{
    // translate the name
    NameMap::iterator aIter = m_aNameTranslationMap.find(rApiEventName);
    if (aIter != m_aNameTranslationMap.end())
    {
        const XMLEventName& rXmlName = aIter->second;

        // export the event ...
        bool bStarted = false;
        ExportEvent( rEventValues, rXmlName, bUseWhitespace, bStarted );

        // ... and close the container element (if necessary)
        if (bStarted)
        {
            EndElement(bUseWhitespace);
        }
    }
    else
    {
        // don't proceed further
        SAL_WARN("xmloff", "Unknown event name:" << rApiEventName );
    }
}


/// export a single event
void XMLEventExport::ExportEvent(
    const Sequence<PropertyValue>& rEventValues,
    const XMLEventName& rXmlEventName,
    bool bUseWhitespace,
    bool& rExported )
{
    // search for EventType value and then delegate to EventHandler
    const PropertyValue* pValue = std::find_if(rEventValues.begin(), rEventValues.end(),
        [](const PropertyValue& rValue) { return u"EventType" == rValue.Name; });

    if (pValue == rEventValues.end())
        return;

    // found! Now find handler and delegate
    OUString sType;
    pValue->Value >>= sType;

    if (m_aHandlerMap.count(sType))
    {
        if (! rExported)
        {
            // OK, we have't yet exported the enclosing
            // element. So we do that now.
            rExported = true;
            StartElement(bUseWhitespace);
        }

        OUString aEventQName(
            m_rExport.GetNamespaceMap().GetQNameByKey(
                    rXmlEventName.m_nPrefix, rXmlEventName.m_aName ) );

        // delegate to proper ExportEventHandler
        m_aHandlerMap[sType]->Export(m_rExport, aEventQName,
                                   rEventValues, bUseWhitespace);
    }
    else
    {
        if ( sType != "None" )
        {
            OSL_FAIL("unknown event type returned by API");
            // unknown type -> error (ignore)
        }
        // else: we ignore None fields
    }
}


void XMLEventExport::StartElement(bool bWhitespace)
{
    if (bWhitespace)
    {
        m_rExport.IgnorableWhitespace();
    }
    sal_uInt16 nNamespace = m_bExtNamespace ? XML_NAMESPACE_OFFICE_EXT
                                          : XML_NAMESPACE_OFFICE;
    m_rExport.StartElement( nNamespace, XML_EVENT_LISTENERS,
                          bWhitespace);
}

void XMLEventExport::EndElement(bool bWhitespace)
{
    sal_uInt16 nNamespace = m_bExtNamespace ? XML_NAMESPACE_OFFICE_EXT
                                          : XML_NAMESPACE_OFFICE;
    m_rExport.EndElement(nNamespace, XML_EVENT_LISTENERS, bWhitespace);
    if (bWhitespace)
    {
        m_rExport.IgnorableWhitespace();
    }
}


// implement aStandardEventTable (defined in xmlevent.hxx)
const XMLEventNameTranslation aStandardEventTable[] =
{
    { u"OnSelect"_ustr,           XML_NAMESPACE_DOM, u"select"_ustr }, // "on-select"
    { u"OnInsertStart"_ustr,      XML_NAMESPACE_OFFICE, u"insert-start"_ustr }, // "on-insert-start"
    { u"OnInsertDone"_ustr,       XML_NAMESPACE_OFFICE, u"insert-done"_ustr }, // "on-insert-done"
    { u"OnMailMerge"_ustr,        XML_NAMESPACE_OFFICE, u"mail-merge"_ustr }, // "on-mail-merge"
    { u"OnAlphaCharInput"_ustr,   XML_NAMESPACE_OFFICE, u"alpha-char-input"_ustr }, // "on-alpha-char-input"
    { u"OnNonAlphaCharInput"_ustr,    XML_NAMESPACE_OFFICE, u"non-alpha-char-input"_ustr }, // "on-non-alpha-char-input"
    { u"OnResize"_ustr,           XML_NAMESPACE_DOM, u"resize"_ustr }, // "on-resize"
    { u"OnMove"_ustr,             XML_NAMESPACE_OFFICE, u"move"_ustr }, // "on-move"
    { u"OnPageCountChange"_ustr,  XML_NAMESPACE_OFFICE, u"page-count-change"_ustr }, // "on-page-count-change"
    { u"OnMouseOver"_ustr,        XML_NAMESPACE_DOM, u"mouseover"_ustr }, // "on-mouse-over"
    { u"OnClick"_ustr,            XML_NAMESPACE_DOM, u"click"_ustr }, // "on-click"
    { u"OnMouseOut"_ustr,         XML_NAMESPACE_DOM, u"mouseout"_ustr }, // "on-mouse-out"
    { u"OnLoadError"_ustr,        XML_NAMESPACE_OFFICE, u"load-error"_ustr }, // "on-load-error"
    { u"OnLoadCancel"_ustr,       XML_NAMESPACE_OFFICE, u"load-cancel"_ustr }, // "on-load-cancel"
    { u"OnLoadDone"_ustr,         XML_NAMESPACE_OFFICE, u"load-done"_ustr }, // "on-load-done"
    { u"OnLoad"_ustr,             XML_NAMESPACE_DOM, u"load"_ustr }, // "on-load"
    { u"OnUnload"_ustr,           XML_NAMESPACE_DOM, u"unload"_ustr }, // "on-unload"
    { u"OnStartApp"_ustr,         XML_NAMESPACE_OFFICE, u"start-app"_ustr }, // "on-start-app"
    { u"OnCloseApp"_ustr,         XML_NAMESPACE_OFFICE, u"close-app"_ustr }, // "on-close-app"
    { u"OnNew"_ustr,              XML_NAMESPACE_OFFICE, u"new"_ustr }, // "on-new"
    { u"OnSave"_ustr,             XML_NAMESPACE_OFFICE, u"save"_ustr }, // "on-save"
    { u"OnSaveAs"_ustr,           XML_NAMESPACE_OFFICE, u"save-as"_ustr }, // "on-save-as"
    { u"OnFocus"_ustr,            XML_NAMESPACE_DOM, u"DOMFocusIn"_ustr }, // "on-focus"
    { u"OnUnfocus"_ustr,          XML_NAMESPACE_DOM, u"DOMFocusOut"_ustr }, // "on-unfocus"
    { u"OnPrint"_ustr,            XML_NAMESPACE_OFFICE, u"print"_ustr }, // "on-print"
    { u"OnError"_ustr,            XML_NAMESPACE_DOM, u"error"_ustr }, // "on-error"
    { u"OnLoadFinished"_ustr,     XML_NAMESPACE_OFFICE, u"load-finished"_ustr }, // "on-load-finished"
    { u"OnSaveFinished"_ustr,     XML_NAMESPACE_OFFICE, u"save-finished"_ustr }, // "on-save-finished"
    { u"OnModifyChanged"_ustr,    XML_NAMESPACE_OFFICE, u"modify-changed"_ustr }, // "on-modify-changed"
    { u"OnPrepareUnload"_ustr,    XML_NAMESPACE_OFFICE, u"prepare-unload"_ustr }, // "on-prepare-unload"
    { u"OnNewMail"_ustr,          XML_NAMESPACE_OFFICE, u"new-mail"_ustr }, // "on-new-mail"
    { u"OnToggleFullscreen"_ustr, XML_NAMESPACE_OFFICE, u"toggle-fullscreen"_ustr }, // "on-toggle-fullscreen"
    { u"OnSaveDone"_ustr,         XML_NAMESPACE_OFFICE, u"save-done"_ustr }, // "on-save-done"
    { u"OnSaveAsDone"_ustr,       XML_NAMESPACE_OFFICE, u"save-as-done"_ustr }, // "on-save-as-done"
    { u"OnCopyTo"_ustr,           XML_NAMESPACE_OFFICE, u"copy-to"_ustr },
    { u"OnCopyToDone"_ustr,       XML_NAMESPACE_OFFICE, u"copy-to-done"_ustr },
    { u"OnViewCreated"_ustr,      XML_NAMESPACE_OFFICE, u"view-created"_ustr },
    { u"OnPrepareViewClosing"_ustr, XML_NAMESPACE_OFFICE, u"prepare-view-closing"_ustr },
    { u"OnViewClosed"_ustr,       XML_NAMESPACE_OFFICE, u"view-close"_ustr },
    { u"OnVisAreaChanged"_ustr,   XML_NAMESPACE_OFFICE, u"visarea-changed"_ustr }, // "on-visarea-changed"
    { u"OnCreate"_ustr,           XML_NAMESPACE_OFFICE, u"create"_ustr },
    { u"OnSaveAsFailed"_ustr,     XML_NAMESPACE_OFFICE, u"save-as-failed"_ustr },
    { u"OnSaveFailed"_ustr,       XML_NAMESPACE_OFFICE, u"save-failed"_ustr },
    { u"OnCopyToFailed"_ustr,     XML_NAMESPACE_OFFICE, u"copy-to-failed"_ustr },
    { u"OnTitleChanged"_ustr,     XML_NAMESPACE_OFFICE, u"title-changed"_ustr },
    { u"OnModeChanged"_ustr,      XML_NAMESPACE_OFFICE, u"mode-changed"_ustr },
    { u"OnSaveTo"_ustr,           XML_NAMESPACE_OFFICE, u"save-to"_ustr },
    { u"OnSaveToDone"_ustr,       XML_NAMESPACE_OFFICE, u"save-to-done"_ustr },
    { u"OnSaveToFailed"_ustr,     XML_NAMESPACE_OFFICE, u"save-to-failed"_ustr },
    { u"OnSubComponentOpened"_ustr,   XML_NAMESPACE_OFFICE, u"subcomponent-opened"_ustr },
    { u"OnSubComponentClosed"_ustr,   XML_NAMESPACE_OFFICE, u"subcomponent-closed"_ustr },
    { u"OnStorageChanged"_ustr,       XML_NAMESPACE_OFFICE, u"storage-changed"_ustr },
    { u"OnMailMergeFinished"_ustr,    XML_NAMESPACE_OFFICE, u"mail-merge-finished"_ustr },
    { u"OnFieldMerge"_ustr,           XML_NAMESPACE_OFFICE, u"field-merge"_ustr },
    { u"OnFieldMergeFinished"_ustr,   XML_NAMESPACE_OFFICE, u"field-merge-finished"_ustr },
    { u"OnLayoutFinished"_ustr,       XML_NAMESPACE_OFFICE, u"layout-finished"_ustr },
    { u"OnDoubleClick"_ustr,      XML_NAMESPACE_OFFICE, u"dblclick"_ustr },
    { u"OnRightClick"_ustr,       XML_NAMESPACE_OFFICE, u"contextmenu"_ustr },
    { u"OnChange"_ustr,           XML_NAMESPACE_OFFICE, u"content-changed"_ustr },
    { u"OnCalculate"_ustr,        XML_NAMESPACE_OFFICE, u"calculated"_ustr },

    { u""_ustr, 0, u""_ustr }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
