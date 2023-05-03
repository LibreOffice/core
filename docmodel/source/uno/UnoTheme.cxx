/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <docmodel/uno/UnoTheme.hxx>
#include <docmodel/theme/ThemeColorType.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <o3tl/enumrange.hxx>
#include <comphelper/sequence.hxx>
#include <docmodel/theme/Theme.hxx>

using namespace css;

OUString UnoTheme::getName() { return mpTheme->GetName(); }

css::uno::Sequence<sal_Int32> UnoTheme::getColorSet()
{
    std::vector<sal_Int32> aColorScheme(12);
    auto pColorSet = mpTheme->getColorSet();
    if (pColorSet)
    {
        size_t i = 0;

        for (auto eThemeColorType : o3tl::enumrange<model::ThemeColorType>())
        {
            if (eThemeColorType == model::ThemeColorType::Unknown)
                continue;
            Color aColor = pColorSet->getColor(eThemeColorType);
            aColorScheme[i] = sal_Int32(aColor);
            i++;
        }
    }
    return comphelper::containerToSequence(aColorScheme);
}

namespace model::theme
{
uno::Reference<util::XTheme> createXTheme(std::shared_ptr<model::Theme> const& pTheme)
{
    return new UnoTheme(pTheme);
}

} // end model::theme

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
