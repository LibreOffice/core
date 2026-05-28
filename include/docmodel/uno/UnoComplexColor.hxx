/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <config_options.h>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/util/XComplexColor.hpp>

#include <docmodel/dllapi.h>
#include <docmodel/color/ComplexColor.hxx>

class UNLESS_MERGELIBS(DOCMODEL_DLLPUBLIC) UnoComplexColor final
    : public cppu::WeakImplHelper<css::util::XComplexColor>
{
private:
    model::ComplexColor maColor;

public:
    UnoComplexColor(model::ComplexColor const& rColor)
        : maColor(rColor)
    {
    }

    model::ComplexColor const& getComplexColor() const { return maColor; }

    // XComplexColor
    sal_Int32 SAL_CALL getType() override;
    sal_Int32 SAL_CALL getThemeColorType() override;
    css::util::Color SAL_CALL
    resolveColor(css::uno::Reference<css::util::XTheme> const& xTheme) override;
};

namespace model::color
{
DOCMODEL_DLLPUBLIC css::uno::Reference<css::util::XComplexColor>
createXComplexColor(model::ComplexColor const& rColor);
DOCMODEL_DLLPUBLIC model::ComplexColor
getFromXComplexColor(css::uno::Reference<css::util::XComplexColor> const& rxColor);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
