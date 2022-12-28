/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <docmodel/uno/UnoThemeColor.hxx>
#include <cppuhelper/queryinterface.hxx>

using namespace css;

// css::lang::XUnoTunnel
UNO3_GETIMPLEMENTATION_IMPL(UnoThemeColor);

sal_Int16 UnoThemeColor::getType() { return sal_uInt16(maThemeColor.getType()); }

namespace model::theme
{
uno::Reference<util::XThemeColor> createXThemeColor(model::ThemeColor const& rThemeColor)
{
    return new UnoThemeColor(rThemeColor);
}

void setFromXThemeColor(model::ThemeColor& rThemeColor,
                        uno::Reference<util::XThemeColor> const& rxColorTheme)
{
    UnoThemeColor* pUnoThemeColor = comphelper::getFromUnoTunnel<UnoThemeColor>(rxColorTheme);
    if (pUnoThemeColor)
    {
        rThemeColor = pUnoThemeColor->getThemeColor();
    }
}

} // end model::theme

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
