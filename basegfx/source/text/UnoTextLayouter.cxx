/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <basegfx/text/UnoTextLayouter.hxx>
#include <cppuhelper/queryinterface.hxx>

using namespace css;

namespace gfx
{
// css::lang::XUnoTunnel
UNO3_GETIMPLEMENTATION_IMPL(UnoTextLayouter);

std::shared_ptr<gfx::ITextLayouter>
getTextLayouterFromUno(uno::Reference<graphic::XTextLayouter> const& xTextLayouter)
{
    gfx::UnoTextLayouter* pUnoTextLayouter
        = comphelper::getUnoTunnelImplementation<gfx::UnoTextLayouter>(xTextLayouter);
    if (pUnoTextLayouter)
        return pUnoTextLayouter->getTextLayouter();

    return std::shared_ptr<gfx::ITextLayouter>();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
