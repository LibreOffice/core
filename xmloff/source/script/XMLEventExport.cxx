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
#include <tools/debug.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>


using namespace ::com::sun::star::uno;

using std::map;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::document::XEventsSupplier;
using ::com::sun::star::container::XNameReplace;
using ::com::sun::star::container::XNameAccess;
using ::xmloff::token::XML_EVENT_LISTENERS;


XMLEventExport::XMLEventExport(SvXMLExport& rExp,
                         const XMLEventNameTranslation* pTranslationTable) :
    sEventType("EventType"),
    rExport(rExp),
    bExtNamespace(false)
{
    AddTranslationTable(pTranslationTable);
}

XMLEventExport::~XMLEventExport()
{
    // delete all handlers
    HandlerMap::iterator aEnd = aHandlerMap.end();
    for( HandlerMap::iterator aIter =
             aHandlerMap.begin();
         aIter != aEnd;
         ++aIter )
    {
        delete aIter->second;
    }
    aHandlerMap.clear();
}

void XMLEventExport::AddHandler( const OUString& rName,
                                 XMLEventExportHandler* pHandler )
{
    DBG_ASSERT(pHandler != nullptr, "Need EventExportHandler");
    if (pHandler != nullptr)
    {
        aHandlerMap[rName] = pHandler;
    }
}

void XMLEventExport::AddTranslationTable(
    const XMLEventNameTranslation* pTransTable )
{
    if (nullptr != pTransTable)
    {
        // put translation table into map
        for(const XMLEventNameTranslation* pTrans = pTransTable;
            pTrans->sAPIName != nullptr;
            pTrans++)
        {
            aNameTranslationMap[OUString::createFromAscii(pTrans->sAPIName)] =
                XMLEventName(pTrans->nPrefix, pTrans->sXMLName);
        }
    }
    // else? ignore!
}

void XMLEventExport::Export( Reference<XEventsSupplier> & rSupplier,
                             bool bWhitespace)
{
    if (rSupplier.is())
    {
        Reference<XNameAccess> xAccess(rSupplier->getEvents(), UNO_QUERY);
        Export(xAccess, bWhitespace);
    }
    // else: no supplier, no export -> ignore!
}

void XMLEventExport::Export( Reference<XNameReplace> & rReplace,
                             bool bWhitespace)
{
    Reference<XNameAccess> xAccess(rReplace, UNO_QUERY);
    Export(xAccess, bWhitespace);
}

void XMLEventExport::Export( Reference<XNameAccess> & rAccess,
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
    Sequence<OUString> aNames = rAccess->getElementNames();
    sal_Int32 nCount = aNames.getLength();
    for(sal_Int32 i = 0; i < nCount; i++)
    {
        // translate name
        NameMap::iterator aIter = aNameTranslationMap.find(aNames[i]);
        if (aIter != aNameTranslationMap.end())
        {
            const XMLEventName& rXmlName = aIter->second;

            // get PropertyValues for this event
            Any aAny = rAccess->getByName( aNames[i] );
            Sequence<PropertyValue> aValues;
            aAny >>= aValues;

            // now export the current event
            ExportEvent( aValues, rXmlName, bWhitespace, bStarted );
        }
#ifdef DBG_UTIL
        else
        {
            // don't proceed further
            OString aStr("Unknown event name:" );
            aStr += OUStringToOString( aNames[i], RTL_TEXTENCODING_UTF8 );
            OSL_FAIL( aStr.getStr() );
        }
#endif
    }

    // close <script:events> element (if it was opened before)
    if (bStarted)
    {
        EndElement(bWhitespace);
    }
}

void XMLEventExport::ExportExt( Reference<XNameAccess> & rAccess )
{
    // set bExtNamespace flag to use XML_NAMESPACE_OFFICE_EXT namespace
    // for events element (not for child elements)
    bExtNamespace = true;
    Export(rAccess);
    bExtNamespace = false;          // reset for future Export calls
}

/// export a singular event and wirte <office:events> container
void XMLEventExport::ExportSingleEvent(
    Sequence<PropertyValue>& rEventValues,
    const OUString& rApiEventName,
    bool bUseWhitespace )
{
    // translate the name
    NameMap::iterator aIter = aNameTranslationMap.find(rApiEventName);
    if (aIter != aNameTranslationMap.end())
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
#ifdef DBG_UTIL
    else
    {
        // don't proceed further
        OString aStr("Unknown event name:" );
        aStr += OUStringToOString( rApiEventName, RTL_TEXTENCODING_UTF8 );
        OSL_FAIL( aStr.getStr() );
    }
#endif
}


/// export a single event
void XMLEventExport::ExportEvent(
    Sequence<PropertyValue>& rEventValues,
    const XMLEventName& rXmlEventName,
    bool bUseWhitespace,
    bool& rExported )
{
    // search for EventType value and then delegate to EventHandler
    sal_Int32 nValues = rEventValues.getLength();
    const PropertyValue* pValues = rEventValues.getConstArray();

    for(sal_Int32 nVal = 0; nVal < nValues; nVal++)
    {
        if (sEventType.equals(pValues[nVal].Name))
        {
            // found! Now find handler and delegate
            OUString sType;
            pValues[nVal].Value >>= sType;

            if (aHandlerMap.count(sType))
            {
                if (! rExported)
                {
                    // OK, we have't yet exported the enclosing
                    // element. So we do that now.
                    rExported = true;
                    StartElement(bUseWhitespace);
                }

                OUString aEventQName(
                    rExport.GetNamespaceMap().GetQNameByKey(
                            rXmlEventName.m_nPrefix, rXmlEventName.m_aName ) );

                // delegate to proper ExportEventHandler
                aHandlerMap[sType]->Export(rExport, aEventQName,
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

            // early out: we don't need to look for another type
            break;
        }
        // else: we only care for EventType -> ignore
    }
}


void XMLEventExport::StartElement(bool bWhitespace)
{
    if (bWhitespace)
    {
        rExport.IgnorableWhitespace();
    }
    sal_uInt16 nNamespace = bExtNamespace ? XML_NAMESPACE_OFFICE_EXT
                                          : XML_NAMESPACE_OFFICE;
    rExport.StartElement( nNamespace, XML_EVENT_LISTENERS,
                          bWhitespace);
}

void XMLEventExport::EndElement(bool bWhitespace)
{
    sal_uInt16 nNamespace = bExtNamespace ? XML_NAMESPACE_OFFICE_EXT
                                          : XML_NAMESPACE_OFFICE;
    rExport.EndElement(nNamespace, XML_EVENT_LISTENERS, bWhitespace);
    if (bWhitespace)
    {
        rExport.IgnorableWhitespace();
    }
}


// implement aStandardEventTable (defined in xmlevent.hxx)
const XMLEventNameTranslation aStandardEventTable[] =
{
    { "OnSelect",           XML_NAMESPACE_DOM, "select" }, // "on-select"
    { "OnInsertStart",      XML_NAMESPACE_OFFICE, "insert-start" }, // "on-insert-start"
    { "OnInsertDone",       XML_NAMESPACE_OFFICE, "insert-done" }, // "on-insert-done"
    { "OnMailMerge",        XML_NAMESPACE_OFFICE, "mail-merge" }, // "on-mail-merge"
    { "OnAlphaCharInput",   XML_NAMESPACE_OFFICE, "alpha-char-input" }, // "on-alpha-char-input"
    { "OnNonAlphaCharInput",    XML_NAMESPACE_OFFICE, "non-alpha-char-input" }, // "on-non-alpha-char-input"
    { "OnResize",           XML_NAMESPACE_DOM, "resize" }, // "on-resize"
    { "OnMove",             XML_NAMESPACE_OFFICE, "move" }, // "on-move"
    { "OnPageCountChange",  XML_NAMESPACE_OFFICE, "page-count-change" }, // "on-page-count-change"
    { "OnMouseOver",        XML_NAMESPACE_DOM, "mouseover" }, // "on-mouse-over"
    { "OnClick",            XML_NAMESPACE_DOM, "click" }, // "on-click"
    { "OnMouseOut",         XML_NAMESPACE_DOM, "mouseout" }, // "on-mouse-out"
    { "OnLoadError",        XML_NAMESPACE_OFFICE, "load-error" }, // "on-load-error"
    { "OnLoadCancel",       XML_NAMESPACE_OFFICE, "load-cancel" }, // "on-load-cancel"
    { "OnLoadDone",         XML_NAMESPACE_OFFICE, "load-done" }, // "on-load-done"
    { "OnLoad",             XML_NAMESPACE_DOM, "load" }, // "on-load"
    { "OnUnload",           XML_NAMESPACE_DOM, "unload" }, // "on-unload"
    { "OnStartApp",         XML_NAMESPACE_OFFICE, "start-app" }, // "on-start-app"
    { "OnCloseApp",         XML_NAMESPACE_OFFICE, "close-app" }, // "on-close-app"
    { "OnNew",              XML_NAMESPACE_OFFICE, "new" }, // "on-new"
    { "OnSave",             XML_NAMESPACE_OFFICE, "save" }, // "on-save"
    { "OnSaveAs",           XML_NAMESPACE_OFFICE, "save-as" }, // "on-save-as"
    { "OnFocus",            XML_NAMESPACE_DOM, "DOMFocusIn" }, // "on-focus"
    { "OnUnfocus",          XML_NAMESPACE_DOM, "DOMFocusOut" }, // "on-unfocus"
    { "OnPrint",            XML_NAMESPACE_OFFICE, "print" }, // "on-print"
    { "OnError",            XML_NAMESPACE_DOM, "error" }, // "on-error"
    { "OnLoadFinished",     XML_NAMESPACE_OFFICE, "load-finished" }, // "on-load-finished"
    { "OnSaveFinished",     XML_NAMESPACE_OFFICE, "save-finished" }, // "on-save-finished"
    { "OnModifyChanged",    XML_NAMESPACE_OFFICE, "modify-changed" }, // "on-modify-changed"
    { "OnPrepareUnload",    XML_NAMESPACE_OFFICE, "prepare-unload" }, // "on-prepare-unload"
    { "OnNewMail",          XML_NAMESPACE_OFFICE, "new-mail" }, // "on-new-mail"
    { "OnToggleFullscreen", XML_NAMESPACE_OFFICE, "toggle-fullscreen" }, // "on-toggle-fullscreen"
    { "OnSaveDone",         XML_NAMESPACE_OFFICE, "save-done" }, // "on-save-done"
    { "OnSaveAsDone",       XML_NAMESPACE_OFFICE, "save-as-done" }, // "on-save-as-done"
    { "OnCopyTo",           XML_NAMESPACE_OFFICE, "copy-to" },
    { "OnCopyToDone",       XML_NAMESPACE_OFFICE, "copy-to-done" },
    { "OnViewCreated",      XML_NAMESPACE_OFFICE, "view-created" },
    { "OnPrepareViewClosing", XML_NAMESPACE_OFFICE, "prepare-view-closing" },
    { "OnViewClosed",       XML_NAMESPACE_OFFICE, "view-close" },
    { "OnVisAreaChanged",   XML_NAMESPACE_OFFICE, "visarea-changed" }, // "on-visarea-changed"
    { "OnCreate",           XML_NAMESPACE_OFFICE, "create" },
    { "OnSaveAsFailed",     XML_NAMESPACE_OFFICE, "save-as-failed" },
    { "OnSaveFailed",       XML_NAMESPACE_OFFICE, "save-failed" },
    { "OnCopyToFailed",     XML_NAMESPACE_OFFICE, "copy-to-failed" },
    { "OnTitleChanged",     XML_NAMESPACE_OFFICE, "title-changed" },
    { "OnModeChanged",      XML_NAMESPACE_OFFICE, "mode-changed" },
    { "OnSaveTo",           XML_NAMESPACE_OFFICE, "save-to" },
    { "OnSaveToDone",       XML_NAMESPACE_OFFICE, "save-to-done" },
    { "OnSaveToFailed",     XML_NAMESPACE_OFFICE, "save-to-failed" },
    { "OnSubComponentOpened",   XML_NAMESPACE_OFFICE, "subcomponent-opened" },
    { "OnSubComponentClosed",   XML_NAMESPACE_OFFICE, "subcomponent-closed" },
    { "OnStorageChanged",       XML_NAMESPACE_OFFICE, "storage-changed" },
    { "OnMailMergeFinished",    XML_NAMESPACE_OFFICE, "mail-merge-finished" },
    { "OnFieldMerge",           XML_NAMESPACE_OFFICE, "field-merge" },
    { "OnFieldMergeFinished",   XML_NAMESPACE_OFFICE, "field-merge-finished" },
    { "OnLayoutFinished",       XML_NAMESPACE_OFFICE, "layout-finished" },
    { "OnDoubleClick",      XML_NAMESPACE_OFFICE, "dblclick" },
    { "OnRightClick",       XML_NAMESPACE_OFFICE, "contextmenu" },
    { "OnChange",           XML_NAMESPACE_OFFICE, "content-changed" },
    { "OnCalculate",        XML_NAMESPACE_OFFICE, "calculated" },

    { nullptr, 0, nullptr }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
