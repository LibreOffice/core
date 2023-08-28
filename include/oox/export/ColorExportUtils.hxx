/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <oox/dllapi.h>

namespace model
{
class ComplexColor;
enum class ThemeColorType : sal_Int32;
}

namespace oox
{
OOX_DLLPUBLIC double convertColorTransformsToTintOrShade(model::ComplexColor const& rComplexColor);
OOX_DLLPUBLIC sal_Int32 convertThemeColorTypeToExcelThemeNumber(model::ThemeColorType eType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
