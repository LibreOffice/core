/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/GraphicLoader.hxx>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
//#include <com/sun/star/lang/XUnoTunnel.hpp>
//#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
//#include <cppuhelper/typeprovider.hxx>

using namespace css;

namespace vcl
{
namespace graphic
{
Graphic loadFromURL(OUString const& rURL)
{
    uno::Reference<css::graphic::XGraphic> xGraphic;

    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
    uno::Reference<css::graphic::XGraphicProvider> xProv(
        css::graphic::GraphicProvider::create(xContext));

    uno::Sequence<beans::PropertyValue> aLoadProps(1);
    aLoadProps[0].Name = "URL";
    aLoadProps[0].Value <<= rURL;

    xGraphic = xProv->queryGraphic(aLoadProps);

    std::unique_ptr<Graphic> pGraphic;
    if (xGraphic.is())
    {
        pGraphic.reset(new Graphic(xGraphic));
    }
    else
    {
        pGraphic.reset(new Graphic);
        pGraphic->SetDefaultType();
    }
    pGraphic->setOriginURL(rURL);

    return *pGraphic.get();
}
}
} // end vcl::graphic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
