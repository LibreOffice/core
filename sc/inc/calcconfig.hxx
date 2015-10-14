/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_CALCCONFIG_HXX
#define INCLUDED_SC_INC_CALCCONFIG_HXX

#include "scdllapi.h"

#include <memory>
#include <ostream>
#include <set>

#include <formula/grammar.hxx>
#include <formula/opcode.hxx>
#include <rtl/ustring.hxx>

#include <comphelper/configurationlistener.hxx>

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
    // from most stringent to most relaxed
    enum class StringConversion
    {
        ILLEGAL,          ///<  =1+"1" or =1+"x" give #VALUE!
        ZERO,             ///<  =1+"1" or =1+"x" give 1
        UNAMBIGUOUS,      ///<  =1+"1" gives 2, but =1+"1.000" or =1+"x" give #VALUE!
        LOCALE            ///<  =1+"1.000" may be 2 or 1001 ... =1+"x" gives #VALUE!
    };
    formula::FormulaGrammar::AddressConvention meStringRefAddressSyntax;
    StringConversion meStringConversion;
    bool mbEmptyStringAsZero:1;
    bool mbHasStringRefSyntax:1;

    comphelper::ConfigurationListenerProperty<bool> mbOpenCLEnabled;

    bool mbOpenCLSubsetOnly:1;
    bool mbOpenCLAutoSelect:1;
    OUString maOpenCLDevice;
    sal_Int32 mnOpenCLMinimumFormulaGroupSize;

    typedef std::shared_ptr<std::set<OpCode>> OpCodeSet;

    OpCodeSet mpOpenCLSubsetOpCodes;

    ScCalcConfig();

    void setOpenCLConfigToDefault();

    void reset();
    void MergeDocumentSpecific( const ScCalcConfig& r );
    void SetStringRefSyntax( formula::FormulaGrammar::AddressConvention eConv );

    bool operator== (const ScCalcConfig& r) const;
    bool operator!= (const ScCalcConfig& r) const;
};

SC_DLLPUBLIC OUString ScOpCodeSetToSymbolicString(const ScCalcConfig::OpCodeSet& rOpCodes);
SC_DLLPUBLIC ScCalcConfig::OpCodeSet ScStringToOpCodeSet(const OUString& rOpCodes);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
