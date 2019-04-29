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

#include <oox/helper/diagramrenderfilter.hxx>
#include <oox/drawingml/themefragmenthandler.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>

namespace oox
{
using namespace ::com::sun::star;
using namespace ::oox::core;

DiagramRenderFilter::DiagramRenderFilter(const uno::Reference<uno::XComponentContext>& xContext)
    : ShapeFilterBase(xContext)
{
}

void DiagramRenderFilter::importTheme()
{
    drawingml::ThemePtr pTheme(new drawingml::Theme);
    uno::Reference<beans::XPropertySet> xPropSet(getModel(), uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aGrabBag;
    xPropSet->getPropertyValue("InteropGrabBag") >>= aGrabBag;

    for (int i = 0; i < aGrabBag.getLength(); i++)
    {
        if (aGrabBag[i].Name == "OOXTheme")
        {
            uno::Reference<xml::sax::XFastSAXSerializable> xDoc;
            if (aGrabBag[i].Value >>= xDoc)
            {
                rtl::Reference<core::FragmentHandler> xFragmentHandler(
                    new drawingml::ThemeFragmentHandler(*this, OUString(), *pTheme));
                importFragment(xFragmentHandler, xDoc);
                setCurrentTheme(pTheme);
            }
        }
    }
}

RelationsRef DiagramRenderFilter::importRelations(const OUString& rFragmentPath)
{
    return RelationsRef(new Relations(rFragmentPath));
}

OUString DiagramRenderFilter::getImplementationName()
{
    return OUString("com.sun.star.comp.oox.DiagramRenderFilter");
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
