/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/servicehelper.hxx>

#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/util/XThemeColor.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <utility>
#include <docmodel/dllapi.h>
#include <docmodel/theme/ThemeColor.hxx>

class DOCMODEL_DLLPUBLIC UnoThemeColor final
    : public cppu::WeakImplHelper<css::util::XThemeColor, css::lang::XUnoTunnel>
{
private:
    model::ThemeColor maThemeColor;

public:
    UnoThemeColor(model::ThemeColor const& rThemeColor)
        : maThemeColor(rThemeColor)
    {
    }

    model::ThemeColor const& getThemeColor() const { return maThemeColor; }

    // XThemeColor
    sal_Int16 SAL_CALL getType() override;

    UNO3_GETIMPLEMENTATION_DECL(UnoThemeColor)
};

namespace model::theme
{
DOCMODEL_DLLPUBLIC css::uno::Reference<css::util::XThemeColor>
createXThemeColor(model::ThemeColor const& rThemeColor);

DOCMODEL_DLLPUBLIC void
setFromXThemeColor(model::ThemeColor& rThemeColor,
                   css::uno::Reference<css::util::XThemeColor> const& rxThemeColor);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
