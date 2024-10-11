/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>
#include <sfx2/dllapi.h>

#include <docmodel/color/ComplexColor.hxx>
#include <docmodel/theme/ThemeColorType.hxx>

struct SFX2_DLLPUBLIC NamedColor
{
    Color m_aColor;
    OUString m_aName;
    sal_Int16 m_nThemeIndex = -1;
    sal_Int16 m_nLumMod = 10000;
    sal_Int16 m_nLumOff = 0;

    NamedColor() = default;

    NamedColor(Color const& rColor, OUString const& rName)
        : m_aColor(rColor)
        , m_aName(rName)
    {
    }

    model::ComplexColor getComplexColor() const
    {
        model::ComplexColor aComplexColor;

        auto eThemeColorType = model::convertToThemeColorType(m_nThemeIndex);

        if (eThemeColorType != model::ThemeColorType::Unknown)
        {
            aComplexColor.setThemeColor(eThemeColorType);

            if (m_nLumMod != 10000)
                aComplexColor.addTransformation({ model::TransformationType::LumMod, m_nLumMod });

            if (m_nLumOff != 0)
                aComplexColor.addTransformation({ model::TransformationType::LumOff, m_nLumOff });

            aComplexColor.setFinalColor(m_aColor);
        }
        else
        {
            aComplexColor.setColor(m_aColor);
        }

        return aComplexColor;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
