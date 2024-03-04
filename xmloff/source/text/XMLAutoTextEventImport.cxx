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

#include "XMLAutoTextEventImport.hxx"
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "XMLAutoTextContainerEventImport.hxx"
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <tools/debug.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Type;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::document::XEventsSupplier;
using ::com::sun::star::container::XNameReplace;
using ::xmloff::token::XML_AUTO_TEXT_EVENTS;

XMLAutoTextEventImport::XMLAutoTextEventImport(
    const css::uno::Reference<css::uno::XComponentContext>& xContext)
    : SvXMLImport(xContext, "com.sun.star.comp.Writer.XMLOasisAutotextEventsImporter")
{
}

XMLAutoTextEventImport::~XMLAutoTextEventImport() noexcept {}

void XMLAutoTextEventImport::initialize(const Sequence<Any>& rArguments)
{
    // The events may come as either an XNameReplace or XEventsSupplier.

    for (const auto& rArgument : rArguments)
    {
        const Type& rType = rArgument.getValueType();
        if (rType == cppu::UnoType<XEventsSupplier>::get())
        {
            Reference<XEventsSupplier> xSupplier;
            rArgument >>= xSupplier;
            DBG_ASSERT(xSupplier.is(), "need XEventsSupplier or XNameReplace");

            xEvents = xSupplier->getEvents();
        }
        else if (rType == cppu::UnoType<XNameReplace>::get())
        {
            rArgument >>= xEvents;
            DBG_ASSERT(xEvents.is(), "need XEventsSupplier or XNameReplace");
        }
    }

    // call parent
    SvXMLImport::initialize(rArguments);
}

SvXMLImportContext* XMLAutoTextEventImport::CreateFastContext(
    sal_Int32 nElement, const Reference<css::xml::sax::XFastAttributeList>& /*xAttrList*/)
{
    if (xEvents.is() && nElement == XML_ELEMENT(OOO, XML_AUTO_TEXT_EVENTS))
    {
        return new XMLAutoTextContainerEventImport(*this, xEvents);
    }
    return nullptr;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_XMLOasisAutotextEventsImporter_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new XMLAutoTextEventImport(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
