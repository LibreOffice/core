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

#include <ostream>
#include <set>

#include <formula/grammar.hxx>
#include <formula/opcode.hxx>
#include <rtl/ustring.hxx>

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
    enum StringConversion
    {
        STRING_CONVERSION_AS_ERROR = 0,     ///<  =1+"1" or =1+"x" give #VALUE!
        STRING_CONVERSION_AS_ZERO,          ///<  =1+"1" or =1+"x" give 1
        STRING_CONVERSION_UNAMBIGUOUS,      ///<  =1+"1" gives 2, but =1+"1.000" or =1+"x" give #VALUE!
        STRING_CONVERSION_LOCALE_DEPENDENT  ///<  =1+"1.000" may be 2 or 1001 ... =1+"x" gives #VALUE!
    };

    struct OpenCLImpl
    {
        OUString maOS;
        OUString maOSVersion;
        OUString maPlatformVendor;
        OUString maDevice;
        OUString maDriverVersion;

        OpenCLImpl()
        {
        }

        OpenCLImpl(const OUString& rOS,
                   const OUString& rOSVersion,
                   const OUString& rPlatformVendor,
                   const OUString& rDevice,
                   const OUString& rDriverVersion)
            : maOS(rOS),
              maOSVersion(rOSVersion),
              maPlatformVendor(rPlatformVendor),
              maDevice(rDevice),
              maDriverVersion(rDriverVersion)
        {
        }

        bool operator==(const OpenCLImpl& r) const
        {
            return maOS == r.maOS &&
                   maOSVersion == r.maOSVersion &&
                   maPlatformVendor == r.maPlatformVendor &&
                   maDevice == r.maDevice &&
                   maDriverVersion == r.maDriverVersion;
        }
        bool operator!=(const OpenCLImpl& r) const
        {
            return !operator==(r);
        }
        bool operator<(const OpenCLImpl& r) const
        {
            return (maOS < r.maOS ||
                    (maOS == r.maOS &&
                     (maOSVersion < r.maOSVersion ||
                      (maOSVersion == r.maOSVersion &&
                       (maPlatformVendor < r.maPlatformVendor ||
                        (maPlatformVendor == r.maPlatformVendor &&
                         (maDevice < r.maDevice ||
                          (maDevice == r.maDevice &&
                           (maDriverVersion < r.maDriverVersion)))))))));
        }
    };

    formula::FormulaGrammar::AddressConvention meStringRefAddressSyntax;
    StringConversion meStringConversion;
    bool mbEmptyStringAsZero:1;

    bool mbOpenCLEnabled:1;
    bool mbOpenCLSubsetOnly:1;
    bool mbOpenCLAutoSelect:1;
    OUString maOpenCLDevice;
    sal_Int32 mnOpenCLMinimumFormulaGroupSize;

    typedef std::set<OpCodeEnum> OpCodeSet;

    OpCodeSet maOpenCLSubsetOpCodes;

    typedef std::set<OpenCLImpl> OpenCLImplSet;

    OpenCLImplSet maOpenCLWhiteList;
    OpenCLImplSet maOpenCLBlackList;

    ScCalcConfig();

    void setOpenCLConfigToDefault();

    void reset();
    void MergeDocumentSpecific( const ScCalcConfig& r );

    bool operator== (const ScCalcConfig& r) const;
    bool operator!= (const ScCalcConfig& r) const;
};

SC_DLLPUBLIC std::ostream& operator<<(std::ostream& rStream, const ScCalcConfig::OpenCLImpl& rImpl);
SC_DLLPUBLIC std::ostream& operator<<(std::ostream& rStream, const ScCalcConfig::OpenCLImplSet& rSet);
SC_DLLPUBLIC std::ostream& operator<<(std::ostream& rStream, const ScCalcConfig& rConfig);

SC_DLLPUBLIC OUString ScOpCodeSetToNumberString(const ScCalcConfig::OpCodeSet& rOpCodes);
SC_DLLPUBLIC OUString ScOpCodeSetToSymbolicString(const ScCalcConfig::OpCodeSet& rOpCodes);
SC_DLLPUBLIC ScCalcConfig::OpCodeSet ScStringToOpCodeSet(const OUString& rOpCodes);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
