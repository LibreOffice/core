/*************************************************************************
 *
 *  $RCSfile: XMLEventExport.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: dvo $ $Date: 2001-02-21 20:30:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLEVENTEXPORT_HXX
#include "XMLEventExport.hxx"
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP
#include <com/sun/star/container/XNameReplace.hpp>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif


using namespace ::com::sun::star::uno;

using std::map;
using ::rtl::OUString;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::document::XEventsSupplier;
using ::com::sun::star::container::XNameReplace;
using ::com::sun::star::container::XNameAccess;


XMLEventExport::XMLEventExport(SvXMLExport& rExp,
                         const XMLEventNameTranslation* pTranslationTable) :
    rExport(rExp),
    sEvents(RTL_CONSTASCII_USTRINGPARAM(sXML_events)),
    sScriptEvents(rExp.GetNamespaceMap().GetQNameByKey(
        XML_NAMESPACE_OFFICE, sEvents)),
    sEventType(RTL_CONSTASCII_USTRINGPARAM("EventType"))
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
         aIter++ )
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
        // put translation table into map
        for(const XMLEventNameTranslation* pTrans = pTransTable;
            pTrans->sAPIName != NULL;
            pTrans++)
        {
            aNameTranslationMap[OUString::createFromAscii(pTrans->sAPIName)] =
                OUString::createFromAscii(pTrans->sXMLName);
        }
    }
    // else? ignore!
}

void XMLEventExport::Export( Reference<XEventsSupplier> & rSupplier,
                             sal_Bool bWhitespace)
{
    if (rSupplier.is())
    {
        Reference<XNameAccess> xAccess(rSupplier->getEvents(), UNO_QUERY);
        Export(xAccess, bWhitespace);
    }
    // else: no supplier, no export -> ignore!
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
    // early out if we don't actually get any events
    if (!rAccess.is())
    {
        return;
    }

    // have we already processed an element?
    sal_Bool bStarted = sal_False;

    // iterate over all event types
    Sequence<OUString> aNames = rAccess->getElementNames();
    sal_Int32 nCount = aNames.getLength();
    for(sal_Int32 i = 0; i < nCount; i++)
    {
        // translate name
        if (aNameTranslationMap.count(aNames[i]))
        {
            OUString sXmlName = aNameTranslationMap[aNames[i]];

            // get PropertyValues for this event
            Any aAny = rAccess->getByName( aNames[i] );
            Sequence<PropertyValue> aValues;
            aAny >>= aValues;

            // search for EventType value and then delegate to EventHandler
            sal_Int32 nValues = aValues.getLength();
            for(sal_Int32 nVal = 0; nVal < nValues; nVal++)
            {
                if (sEventType.equals(aValues[nVal].Name))
                {
                    // found! Now find handler and delegate
                    OUString sType;
                    aValues[nVal].Value >>= sType;

                    if (aHandlerMap.count(sType))
                    {
                        if (! bStarted)
                        {
                            // OK, we have't yet exported the enclosing
                            // element. So we do that now.
                            bStarted = sal_True;
                            StartElement(bWhitespace);
                        }

                        // delegate to proper ExportEventHandler
                        aHandlerMap[sType]->Export(rExport, sXmlName,
                                                   aValues, bWhitespace);
                    }
                    else
                    {
                        if (! sType.equalsAsciiL("None", sizeof("None")-1))
                        {
                            DBG_ERROR("unknown event type returned by API");
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
        else
        {
            // don't proceed further
            DBG_ERROR("Unknown event name.");
        }
    }

    // close <script:events> element (if it was opened before)
    if (bStarted)
    {
        EndElement(bWhitespace);
    }
}

void XMLEventExport::StartElement(sal_Bool bWhitespace)
{
    if (bWhitespace)
    {
        rExport.GetDocHandler()->ignorableWhitespace(rExport.sWS);
        rExport.GetDocHandler()->startElement(sScriptEvents, NULL);
        rExport.GetDocHandler()->ignorableWhitespace(rExport.sWS);
    }
    else
    {
        rExport.GetDocHandler()->startElement(sScriptEvents, NULL);
    }
}

void XMLEventExport::EndElement(sal_Bool bWhitespace)
{
    if (bWhitespace)
    {
        rExport.GetDocHandler()->ignorableWhitespace(rExport.sWS);
        rExport.GetDocHandler()->endElement(sScriptEvents);
        rExport.GetDocHandler()->ignorableWhitespace(rExport.sWS);
    }
    else
    {
        rExport.GetDocHandler()->endElement(sScriptEvents);
    }
}


// implement aStandardEventTable (defined in xmlevent.hxx)
const XMLEventNameTranslation aStandardEventTable[] =
{
    { "OnSelect",           "on-select" },
    { "OnInsertStart",      "on-insert-start" },
    { "OnInsertDone",       "on-insert-done" },
    { "OnMailMerge",        "on-mail-merge" },
    { "OnAlphaCharInput",   "on-alpha-char-input" },
    { "OnNonAlphaCharInput",    "on-non-alpha-char-input" },
    { "OnResize",           "on-resize" },
    { "OnMove",             "on-move" },
    { "OnPageCountChange",  "page-count-change" },
    { "OnMouseOver",        "on-mouse-over" },
    { "OnClick",            "on-click" },
    { "OnMouseOut",         "on-mouse-out" },
    { "OnLoadError",        "on-load-error" },
    { "OnLoadCancel",       "on-load-cancel" },
    { "OnLoadDone",         "on-load-done" },
    { "OnLoad",             "on-load" },
    { "OnUnload",           "on-unload" },
    { "OnStartApp",         "on-start-app" },
    { "OnCloseApp",         "on-close-app" },
    { "OnNew",              "on-new" },
    { "OnSave",             "on-save" },
    { "OnSaveAs",           "on-save-as" },
    { "OnFocus",            "on-focus" },
    { "OnUnfocus",          "on-unfocus" },
    { "OnPrint",            "on-print" },
    { "OnError",            "on-error" },
    { "OnLoadFinished",     "on-load-finished" },
    { "OnSaveFinished",     "on-save-finished" },
    { "OnModifyChanged",    "on-modify-changed" },
    { "OnPrepareUnload",    "on-prepare-unload" },
    { "OnNewMail",          "on-new-mail" },
    { "OnToggleFullscreen", "on-toggle-fullscreen" },
    { "OnSaveDone",         "on-save-done" },
    { "OnSaveAsDone",       "on-save-as-done" },
    { NULL, NULL }
};
