/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <docmodel/uno/UnoComplexColor.hxx>
#include <cppuhelper/queryinterface.hxx>

using namespace css;

sal_Int32 UnoComplexColor::getType() { return sal_Int32(maColor.getType()); }

sal_Int32 UnoComplexColor::getThemeColorType() { return sal_Int32(maColor.getThemeColorType()); }

util::Color UnoComplexColor::resolveColor(uno::Reference<util::XTheme> const& /*xTheme*/)
{
    return util::Color(maColor.getRGBColor());
}

namespace model::color
{
uno::Reference<util::XComplexColor> createXComplexColor(model::ComplexColor const& rColor)
{
    return new UnoComplexColor(rColor);
}

model::ComplexColor getFromXComplexColor(uno::Reference<util::XComplexColor> const& rxColor)
{
    model::ComplexColor aComplexColor;
    UnoComplexColor const* pUnoComplexColor = static_cast<UnoComplexColor const*>(rxColor.get());
    if (pUnoComplexColor)
        aComplexColor = pUnoComplexColor->getComplexColor();
    return aComplexColor;
}

} // end model::color

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
