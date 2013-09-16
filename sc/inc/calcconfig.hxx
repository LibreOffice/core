/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SC_CALCCONFIG_HXX__
#define __SC_CALCCONFIG_HXX__

#include "scdllapi.h"
#include "formula/grammar.hxx"

#include "rtl/ustring.hxx"

// have to match the registry values
enum ScRecalcOptions
{
    RECALC_ALWAYS = 0,
    RECALC_NEVER,
    RECALC_ASK,
};

/**
 * Configuration options for formula interpreter.
 */
struct SC_DLLPUBLIC ScCalcConfig
{
    formula::FormulaGrammar::AddressConvention meStringRefAddressSyntax;
    bool mbEmptyStringAsZero:1;
    bool mbOpenCLEnabled:1;
    bool mbOpenCLAutoSelect:1;
    OUString maOpenCLDevice;

    ScCalcConfig();

    void reset();

    bool operator== (const ScCalcConfig& r) const;
    bool operator!= (const ScCalcConfig& r) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
