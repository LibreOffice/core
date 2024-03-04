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

#include "XMLAutoTextEventExport.hxx"
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <osl/diagnose.h>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/XMLEventExport.hxx>
#include <tools/debug.hxx>


using namespace ::com::sun::star;
using namespace ::xmloff::token;

using ::com::sun::star::container::XNameReplace;
using ::com::sun::star::document::XEventsSupplier;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::xml::sax::XDocumentHandler;


XMLAutoTextEventExport::XMLAutoTextEventExport(
    const css::uno::Reference< css::uno::XComponentContext >& xContext,
    OUString const & implementationName, SvXMLExportFlags nFlags
    )
:   SvXMLExport(xContext, implementationName, util::MeasureUnit::INCH, XML_AUTO_TEXT, nFlags)
{
}

XMLAutoTextEventExport::~XMLAutoTextEventExport()
{
}

void XMLAutoTextEventExport::initialize(
    const Sequence<Any> & rArguments )
{
    if (rArguments.getLength() > 1)
    {
        Reference<XEventsSupplier> xSupplier;
        rArguments[1] >>= xSupplier;
        if (xSupplier.is())
        {
            xEvents = xSupplier->getEvents();
        }
        else
        {
            Reference<XNameReplace> xReplace;
            rArguments[1] >>= xReplace;
            if (xReplace.is())
            {
                xEvents = xReplace;
            }
            else
            {
                rArguments[1] >>= xEvents;
            }
        }
    }

    // call super class (for XHandler)
    SvXMLExport::initialize(rArguments);
}


ErrCode XMLAutoTextEventExport::exportDoc( enum XMLTokenEnum )
{
    if( !(getExportFlags() & SvXMLExportFlags::OASIS) )
    {
        Reference< uno::XComponentContext> xContext = getComponentContext();
        try
        {

            Sequence<Any> aArgs{ Any(GetDocHandler()) };

            // get filter component
            Reference< xml::sax::XDocumentHandler > xTmpDocHandler(
                xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                    "com.sun.star.comp.Oasis2OOoTransformer",
                    aArgs,
                    xContext),
                UNO_QUERY);
            OSL_ENSURE( xTmpDocHandler.is(),
                "can't instantiate OASIS transformer component" );
            if( xTmpDocHandler.is() )
            {
                SetDocHandler( xTmpDocHandler );
            }
        }
        catch( css::uno::Exception& )
        {
        }
    }
    if (hasEvents())
    {
        GetDocHandler()->startDocument();

        addChaffWhenEncryptedStorage();

        addNamespaces();

        {
            // container element
            SvXMLElementExport aContainerElement(
                *this, XML_NAMESPACE_OOO, XML_AUTO_TEXT_EVENTS,
                true, true);

            exportEvents();
        }

        // and close document again
        GetDocHandler()->endDocument();
    }

    return ERRCODE_NONE;
}

bool XMLAutoTextEventExport::hasEvents() const
{
    // TODO: provide full implementation that check for presence of events
    return xEvents.is();
}

void XMLAutoTextEventExport::addNamespaces()
{
    // namespaces for office:, text: and script:
    GetAttrList().AddAttribute(
        GetNamespaceMap().GetAttrNameByIndex( XML_NAMESPACE_OFFICE ),
        GetNamespaceMap().GetNameByIndex( XML_NAMESPACE_OFFICE ) );
    GetAttrList().AddAttribute(
        GetNamespaceMap().GetAttrNameByIndex( XML_NAMESPACE_TEXT ),
        GetNamespaceMap().GetNameByIndex( XML_NAMESPACE_TEXT ) );
    GetAttrList().AddAttribute(
        GetNamespaceMap().GetAttrNameByIndex( XML_NAMESPACE_SCRIPT ),
        GetNamespaceMap().GetNameByIndex( XML_NAMESPACE_SCRIPT ) );
    GetAttrList().AddAttribute(
        GetNamespaceMap().GetAttrNameByIndex( XML_NAMESPACE_DOM ),
        GetNamespaceMap().GetNameByIndex( XML_NAMESPACE_DOM ) );
    GetAttrList().AddAttribute(
        GetNamespaceMap().GetAttrNameByIndex( XML_NAMESPACE_OOO ),
        GetNamespaceMap().GetNameByIndex( XML_NAMESPACE_OOO ) );
    GetAttrList().AddAttribute(
        GetNamespaceMap().GetAttrNameByIndex( XML_NAMESPACE_XLINK ),
        GetNamespaceMap().GetNameByIndex( XML_NAMESPACE_XLINK ) );
}

void XMLAutoTextEventExport::exportEvents()
{
    DBG_ASSERT(hasEvents(), "no events to export!");

    GetEventExport().Export(xEvents);
}


// methods without content:

void XMLAutoTextEventExport::ExportMeta_() {}
void XMLAutoTextEventExport::ExportScripts_() {}
void XMLAutoTextEventExport::ExportFontDecls_() {}
void XMLAutoTextEventExport::ExportStyles_( bool ) {}
void XMLAutoTextEventExport::ExportAutoStyles_() {}
void XMLAutoTextEventExport::ExportMasterStyles_() {}
void XMLAutoTextEventExport::ExportContent_() {}


// methods to support the component registration

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLOasisAutotextEventsExporter_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new XMLAutoTextEventExport(
        context, "com.sun.star.comp.Writer.XMLOasisAutotextEventsExporter",
        SvXMLExportFlags::ALL | SvXMLExportFlags::OASIS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLAutotextEventsExporter_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new XMLAutoTextEventExport(
        context, "com.sun.star.comp.Writer.XMLAutotextEventsExporter",
        SvXMLExportFlags::ALL));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
