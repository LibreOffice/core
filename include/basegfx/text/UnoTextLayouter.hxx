/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/servicehelper.hxx>

#include <com/sun/star/graphic/XTextLayouter.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <basegfx/text/ITextLayouter.hxx>
#include <basegfx/basegfxdllapi.h>

namespace gfx
{
BASEGFX_DLLPUBLIC std::shared_ptr<gfx::ITextLayouter>
getTextLayouterFromUno(css::uno::Reference<css::graphic::XTextLayouter> const& xTextLayouter);

class UnoTextLayouter final
    : public cppu::WeakImplHelper<css::graphic::XTextLayouter, css::lang::XUnoTunnel>
{
private:
    std::shared_ptr<gfx::ITextLayouter> mpTextLayouter;

public:
    UnoTextLayouter() {}

    UnoTextLayouter(std::shared_ptr<gfx::ITextLayouter> const& rTextLayouter)
        : mpTextLayouter(rTextLayouter)
    {
    }

    std::shared_ptr<gfx::ITextLayouter> const& getTextLayouter() const { return mpTextLayouter; }

    void setTextLayouter(std::shared_ptr<gfx::ITextLayouter> const& rTextLayouter)
    {
        mpTextLayouter = rTextLayouter;
    }

    UNO3_GETIMPLEMENTATION_DECL(UnoTextLayouter)
};
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
