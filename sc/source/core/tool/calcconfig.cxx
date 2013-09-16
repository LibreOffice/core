/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "calcconfig.hxx"

ScCalcConfig::ScCalcConfig() :
    meStringRefAddressSyntax(formula::FormulaGrammar::CONV_UNSPECIFIED),
    mbEmptyStringAsZero(false),
    mbOpenCLEnabled(false),
    mbOpenCLAutoSelect(true)
{
}

void ScCalcConfig::reset()
{
    *this = ScCalcConfig();
}

bool ScCalcConfig::operator== (const ScCalcConfig& r) const
{
    return meStringRefAddressSyntax == r.meStringRefAddressSyntax &&
           mbEmptyStringAsZero == r.mbEmptyStringAsZero &&
           mbOpenCLEnabled == r.mbOpenCLEnabled &&
           mbOpenCLAutoSelect == r.mbOpenCLAutoSelect &&
           maOpenCLDevice == r.maOpenCLDevice;
}

bool ScCalcConfig::operator!= (const ScCalcConfig& r) const
{
    return !operator==(r);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
