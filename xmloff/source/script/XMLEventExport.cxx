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
using ::xmloff::token::GetXMLToken;
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
    DBG_ASSERT(pHandler != NULL, "Need EventExportHandler");
    if (pHandler != NULL)
    {
        aHandlerMap[rName] = pHandler;
    }
}

void XMLEventExport::AddTranslationTable(
    const XMLEventNameTranslation* pTransTable )
{
    if (NULL != pTransTable)
    {
        
        for(const XMLEventNameTranslation* pTrans = pTransTable;
            pTrans->sAPIName != NULL;
            pTrans++)
        {
            aNameTranslationMap[OUString::createFromAscii(pTrans->sAPIName)] =
                XMLEventName(pTrans->nPrefix, pTrans->sXMLName);
        }
    }
    
}

void XMLEventExport::Export( Reference<XEventsSupplier> & rSupplier,
                             sal_Bool bWhitespace)
{
    if (rSupplier.is())
    {
        Reference<XNameAccess> xAccess(rSupplier->getEvents(), UNO_QUERY);
        Export(xAccess, bWhitespace);
    }
    
}

void XMLEventExport::Export( Reference<XNameReplace> & rReplace,
                             sal_Bool bWhitespace)
{
    Reference<XNameAccess> xAccess(rReplace, UNO_QUERY);
    Export(xAccess, bWhitespace);
}

void XMLEventExport::Export( Reference<XNameAccess> & rAccess,
                             sal_Bool bWhitespace)
{
    
    if (!rAccess.is())
    {
        return;
    }

    
    sal_Bool bStarted = sal_False;

    
    Sequence<OUString> aNames = rAccess->getElementNames();
    sal_Int32 nCount = aNames.getLength();
    for(sal_Int32 i = 0; i < nCount; i++)
    {
        
        NameMap::iterator aIter = aNameTranslationMap.find(aNames[i]);
        if (aIter != aNameTranslationMap.end())
        {
            const XMLEventName& rXmlName = aIter->second;

            
            Any aAny = rAccess->getByName( aNames[i] );
            Sequence<PropertyValue> aValues;
            aAny >>= aValues;

            
            ExportEvent( aValues, rXmlName, bWhitespace, bStarted );
        }
#ifdef DBG_UTIL
        else
        {
            
            OString aStr("Unknown event name:" );
            aStr += OUStringToOString( aNames[i], RTL_TEXTENCODING_UTF8 );
            OSL_FAIL( aStr.getStr() );
        }
#endif
    }

    
    if (bStarted)
    {
        EndElement(bWhitespace);
    }
}

void XMLEventExport::ExportExt( Reference<XNameAccess> & rAccess,
                                sal_Bool bWhitespace )
{
    
    
    bExtNamespace = true;
    Export(rAccess, bWhitespace);
    bExtNamespace = false;          
}


void XMLEventExport::ExportSingleEvent(
    Sequence<PropertyValue>& rEventValues,
    const OUString& rApiEventName,
    sal_Bool bUseWhitespace )
{
    
    NameMap::iterator aIter = aNameTranslationMap.find(rApiEventName);
    if (aIter != aNameTranslationMap.end())
    {
        const XMLEventName& rXmlName = aIter->second;

        
        sal_Bool bStarted = sal_False;
        ExportEvent( rEventValues, rXmlName, bUseWhitespace, bStarted );

        
        if (bStarted)
        {
            EndElement(bUseWhitespace);
        }
    }
#ifdef DBG_UTIL
    else
    {
        
        OString aStr("Unknown event name:" );
        aStr += OUStringToOString( rApiEventName, RTL_TEXTENCODING_UTF8 );
        OSL_FAIL( aStr.getStr() );
    }
#endif
}



void XMLEventExport::ExportEvent(
    Sequence<PropertyValue>& rEventValues,
    const XMLEventName& rXmlEventName,
    sal_Bool bUseWhitespace,
    sal_Bool& rExported )
{
    
    sal_Int32 nValues = rEventValues.getLength();
    const PropertyValue* pValues = rEventValues.getConstArray();

    for(sal_Int32 nVal = 0; nVal < nValues; nVal++)
    {
        if (sEventType.equals(pValues[nVal].Name))
        {
            
            OUString sType;
            pValues[nVal].Value >>= sType;

            if (aHandlerMap.count(sType))
            {
                if (! rExported)
                {
                    
                    
                    rExported = sal_True;
                    StartElement(bUseWhitespace);
                }

                OUString aEventQName(
                    rExport.GetNamespaceMap().GetQNameByKey(
                            rXmlEventName.m_nPrefix, rXmlEventName.m_aName ) );

                
                aHandlerMap[sType]->Export(rExport, aEventQName,
                                           rEventValues, bUseWhitespace);
            }
            else
            {
                if ( sType != "None" )
                {
                    OSL_FAIL("unknown event type returned by API");
                    
                }
                
            }

            
            break;
        }
        
    }
}


void XMLEventExport::StartElement(sal_Bool bWhitespace)
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

void XMLEventExport::EndElement(sal_Bool bWhitespace)
{
    sal_uInt16 nNamespace = bExtNamespace ? XML_NAMESPACE_OFFICE_EXT
                                          : XML_NAMESPACE_OFFICE;
    rExport.EndElement(nNamespace, XML_EVENT_LISTENERS, bWhitespace);
    if (bWhitespace)
    {
        rExport.IgnorableWhitespace();
    }
}



const XMLEventNameTranslation aStandardEventTable[] =
{
    { "OnSelect",           XML_NAMESPACE_DOM, "select" }, 
    { "OnInsertStart",      XML_NAMESPACE_OFFICE, "insert-start" }, 
    { "OnInsertDone",       XML_NAMESPACE_OFFICE, "insert-done" }, 
    { "OnMailMerge",        XML_NAMESPACE_OFFICE, "mail-merge" }, 
    { "OnAlphaCharInput",   XML_NAMESPACE_OFFICE, "alpha-char-input" }, 
    { "OnNonAlphaCharInput",    XML_NAMESPACE_OFFICE, "non-alpha-char-input" }, 
    { "OnResize",           XML_NAMESPACE_DOM, "resize" }, 
    { "OnMove",             XML_NAMESPACE_OFFICE, "move" }, 
    { "OnPageCountChange",  XML_NAMESPACE_OFFICE, "page-count-change" }, 
    { "OnMouseOver",        XML_NAMESPACE_DOM, "mouseover" }, 
    { "OnClick",            XML_NAMESPACE_DOM, "click" }, 
    { "OnMouseOut",         XML_NAMESPACE_DOM, "mouseout" }, 
    { "OnLoadError",        XML_NAMESPACE_OFFICE, "load-error" }, 
    { "OnLoadCancel",       XML_NAMESPACE_OFFICE, "load-cancel" }, 
    { "OnLoadDone",         XML_NAMESPACE_OFFICE, "load-done" }, 
    { "OnLoad",             XML_NAMESPACE_DOM, "load" }, 
    { "OnUnload",           XML_NAMESPACE_DOM, "unload" }, 
    { "OnStartApp",         XML_NAMESPACE_OFFICE, "start-app" }, 
    { "OnCloseApp",         XML_NAMESPACE_OFFICE, "close-app" }, 
    { "OnNew",              XML_NAMESPACE_OFFICE, "new" }, 
    { "OnSave",             XML_NAMESPACE_OFFICE, "save" }, 
    { "OnSaveAs",           XML_NAMESPACE_OFFICE, "save-as" }, 
    { "OnFocus",            XML_NAMESPACE_DOM, "DOMFocusIn" }, 
    { "OnUnfocus",          XML_NAMESPACE_DOM, "DOMFocusOut" }, 
    { "OnPrint",            XML_NAMESPACE_OFFICE, "print" }, 
    { "OnError",            XML_NAMESPACE_DOM, "error" }, 
    { "OnLoadFinished",     XML_NAMESPACE_OFFICE, "load-finished" }, 
    { "OnSaveFinished",     XML_NAMESPACE_OFFICE, "save-finished" }, 
    { "OnModifyChanged",    XML_NAMESPACE_OFFICE, "modify-changed" }, 
    { "OnPrepareUnload",    XML_NAMESPACE_OFFICE, "prepare-unload" }, 
    { "OnNewMail",          XML_NAMESPACE_OFFICE, "new-mail" }, 
    { "OnToggleFullscreen", XML_NAMESPACE_OFFICE, "toggle-fullscreen" }, 
    { "OnSaveDone",         XML_NAMESPACE_OFFICE, "save-done" }, 
    { "OnSaveAsDone",       XML_NAMESPACE_OFFICE, "save-as-done" }, 
    { "OnCopyTo",           XML_NAMESPACE_OFFICE, "copy-to" },
    { "OnCopyToDone",       XML_NAMESPACE_OFFICE, "copy-to-done" },
    { "OnViewCreated",      XML_NAMESPACE_OFFICE, "view-created" },
    { "OnPrepareViewClosing", XML_NAMESPACE_OFFICE, "prepare-view-closing" },
    { "OnViewClosed",       XML_NAMESPACE_OFFICE, "view-close" },
    { "OnVisAreaChanged",   XML_NAMESPACE_OFFICE, "visarea-changed" }, 
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

    { NULL, 0, 0 }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
