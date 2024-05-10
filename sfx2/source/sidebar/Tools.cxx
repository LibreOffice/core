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

#include <sidebar/Tools.hxx>

#include <sfx2/sidebar/Theme.hxx>

#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/commandinfoprovider.hxx>

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>

using namespace css;
using namespace css::uno;

namespace sfx2::sidebar {

css::uno::Reference<css::graphic::XGraphic> Tools::GetImage(
    const OUString& rsImageURL,
    const OUString& rsHighContrastImageURL,
    const Reference<frame::XFrame>& rxFrame)
{
    if (Theme::IsHighContrastMode() && !rsHighContrastImageURL.isEmpty())
        return GetImage(rsHighContrastImageURL, rxFrame);
    else
        return GetImage(rsImageURL, rxFrame);
}

css::uno::Reference<css::graphic::XGraphic> Tools::GetImage(
    const OUString& rsURL,
    const Reference<frame::XFrame>& rxFrame)
{
    if (rsURL.getLength() > 0)
    {
        if (rsURL.startsWith(".uno:"))
            return vcl::CommandInfoProvider::GetXGraphicForCommand(rsURL, rxFrame);

        else
        {
             Reference<uno::XComponentContext> xContext(::comphelper::getProcessComponentContext());
             Reference<graphic::XGraphicProvider> xProvider(graphic::GraphicProvider::create(xContext));
             ::comphelper::NamedValueCollection aMediaProperties;
             aMediaProperties.put(u"URL"_ustr, rsURL);
             return xProvider->queryGraphic(aMediaProperties.getPropertyValues());
        }
    }
    return nullptr;
}

util::URL Tools::GetURL (const OUString& rsCommand)
{
    util::URL aURL;
    aURL.Complete = rsCommand;

    const Reference<XComponentContext> xComponentContext (::comphelper::getProcessComponentContext());
    const Reference<util::XURLTransformer> xParser = util::URLTransformer::create( xComponentContext );
    xParser->parseStrict(aURL);

    return aURL;
}

Reference<frame::XDispatch> Tools::GetDispatch (
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    const util::URL& rURL)
{
    Reference<frame::XDispatchProvider> xProvider (rxFrame, UNO_QUERY_THROW);
    Reference<frame::XDispatch> xDispatch (xProvider->queryDispatch(rURL, OUString(), 0));
    return xDispatch;
}

OUString Tools::GetModuleName (
    const css::uno::Reference<css::frame::XController>& rxController)
{
    if (!rxController.is())
        return OUString();

    try
    {
        const Reference<XComponentContext> xComponentContext (::comphelper::getProcessComponentContext());
        const Reference<frame::XModuleManager> xModuleManager = frame::ModuleManager::create( xComponentContext );
        return xModuleManager->identify(rxController);
    }
    catch (const Exception&)
    {
        // Ignored.
    }
    return OUString();
}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
