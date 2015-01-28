/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#include "units.hxx"

#include "document.hxx"
#include "refdata.hxx"
#include "tokenarray.hxx"

#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <rtl/bootstrap.hxx>
#include <svl/zformat.hxx>

#include <boost/scoped_array.hpp>
#include <stack>

#include <udunits2.h>

using namespace formula;
using namespace sc;
using namespace std;

::osl::Mutex sc::Units::ourSingletonMutex;
::boost::weak_ptr< Units > sc::Units::ourUnits;

OUString dumpUTStatus() {
    switch(ut_get_status()) {
    case UT_SUCCESS:
        return "UT_SUCCESS: successful!";
    case UT_BAD_ARG:
        return "UT_BAD_ARG: invalid argument";
    case UT_EXISTS:
        return "UT_EXISTS: unit/prefix/identifier already exists";
    case UT_NO_UNIT:
        return "UT_NO_UNIT: no such unit exists";
    case UT_OS:
        return "UT_OS: operating system error (check errno?)";
    case UT_NOT_SAME_SYSTEM:
        return "UT_NOT_SAME_SYSTEM: units not in same unit system";
    case UT_MEANINGLESS:
        return "UT_MEANINGLESS: operation is meaningless";
    case UT_NO_SECOND:
        return "UT_NO_SECOND: no unit named second";
    case UT_VISIT_ERROR:
        return "UT_VISIT_ERROR";
    case UT_CANT_FORMAT:
        return "UT_CANT_FORMAT";
    case UT_SYNTAX:
        return "UT_SYNTAX: syntax error in unit string";
    case UT_UNKNOWN:
        return "UT_UNKNOWN: unknown unit encountered";
    case UT_OPEN_ARG:
        return "UT_OPEN_ARG: can't open specified unit database (arg)";
    case UT_OPEN_ENV:
        return "UT_OPEN_ENV: can't open specified unit databse (env)";
    case UT_OPEN_DEFAULT:
        return "UT_OPEN_DEFAULT: can't open default unit database";
    default:
        return "other (unpspecified) error encountered";
    }
}

class UnitP:
    public ::boost::shared_ptr< ut_unit > {
public:
    UnitP(ut_unit* pUnit):
        boost::shared_ptr< ut_unit >(pUnit, &freeUt)
        {}

    UnitP():
        boost::shared_ptr< ut_unit >(0, &freeUt)
        {}

    void reset(ut_unit* pUnit) {
        boost::shared_ptr< ut_unit >::reset(pUnit, &freeUt);
    }

    OUString getString() const {
        char aBuf[200];
        int nChars = ut_format(this->get(), aBuf, 200, UT_UTF8);
        if (nChars == -1) {
            SAL_INFO("sc.units", "couldn't format unit: " << dumpUTStatus());
            // Placeholder for unformattable strings.
            return "?";
        }

        // If the output doesn't fit in the buffer, ut_format doesn't write
        // a terminating null. However for any output we have the correct length
        // as returned by ut_format, which is the easiest way to ensure the OString
        // constructor will always work correctly. (Alternatively we could retry with
        // a larger buffer, however this method is purely for debugging purposes for now.)

        return OUString(aBuf, nChars, RTL_TEXTENCODING_UTF8);
    }

private:
    static void freeUt(ut_unit* pUnit) {
        ut_free(pUnit);
    }
};

UnitP getOutputUnitsForOpCode(const UnitP& pFirstUnit, const UnitP& pSecondUnit, const OpCode& rOpCode) {
    UnitP pOut;

    switch (rOpCode) {
    case ocAdd:
        // Adding and subtracting both require the same units on both sides
        // hence we can just fall through / use the same logic.
    case ocSub:
        if (ut_compare(pFirstUnit.get(), pSecondUnit.get()) == 0) {
            // The two units are identical, hence we can return either.
            pOut = pFirstUnit;
            SAL_INFO("sc.units", "verified equality for unit " << pFirstUnit.getString());
        } else {
            // TODO: notify/link UI.
        }
        break;
    case ocMul:
        pOut.reset(ut_multiply(pFirstUnit.get(), pSecondUnit.get()));
        break;
    case ocDiv:
        pOut.reset(ut_divide(pFirstUnit.get(), pSecondUnit.get()));
        break;
    default:
        SAL_INFO("sc.units", "unit verification not supported for opcode: " << static_cast<std::underlying_type<const OpCode>::type>(rOpCode));
        assert(false);
    }

    return pOut;

// TODO: implement further sensible opcode handling
}

static void freeUtSystem(ut_system* pSystem) {
    ut_free_system(pSystem);
}

Units::Units() {
    SAL_INFO("sc.units", "initialising udunits2");

    // System udunits will (/should) be able to find it's unit database
    // itself -- however for bundled udunits we always need to find the
    // correct relative path within our LO installation.
#ifdef USING_SYSTEM_UDUNITS
    const sal_Char* pPath = 0;
#else
    OUString sDBURL("$BRAND_BASE_DIR/$BRAND_SHARE_SUBDIR/udunits2/udunits2.xml");
    ::rtl::Bootstrap::expandMacros(sDBURL);
    OUString sDBPath;
    ::osl::FileBase::getSystemPathFromFileURL(sDBURL, sDBPath);

    OString sDBPathOut = OUStringToOString(sDBPath, RTL_TEXTENCODING_ASCII_US);
    const sal_Char* pPath = sDBPathOut.getStr();
#endif

    mpUnitSystem = boost::shared_ptr< ut_system >( ut_read_xml( pPath ),
                                                   &freeUtSystem );

    SAL_INFO("sc.units", "udunits2 initialised");
}

Units::~Units() {
    // We only arrive here if all shared_ptr's to our Units get
    // disposed. In this case the weak_ptr is already cleared,
    // and any new calls to GetUnits don't need to care at what
    // stage of destruction we are?

    // We might need to lock on our singletonMutex if we can't
    // load the same unit system multiple times in memory
    // (i.e. if udunits can't handle being used across threads)?
}

boost::shared_ptr< Units > Units::GetUnits() {
    osl::MutexGuard aGuard(ourSingletonMutex);
    boost::shared_ptr< Units > pUnits = ourUnits.lock();

    if (!pUnits) {
        pUnits.reset( new Units() );
        ourUnits = pUnits;
    }
    return pUnits;
}

OUString extractUnitStringFromFormat(const OUString& rFormatString) {
    // TODO: decide what we do for different subformats? Simplest solution
    // would be to not allow unit storage for multiple subformats.
    // TODO: we should check the number of subformats here in future?

    // TODO: use proper string processing routines?

    sal_Int32 nPos = rFormatString.getLength() - 1;

    // Only iterate if we have a string item at the end of our format string
    if (rFormatString[nPos] == '\"') {
       // TODO: deal with escaped strings? (Does that exist in these?)
        while (rFormatString[--nPos] != '\"') {
            if (nPos == 0) {
                // TODO: plug into our error reporting here to return bad escaping?
                return "";
            }
        }
    } else { // otherwise we have no units for this cell
        return "";
    }

    // Ensure that the parentheses are NOT included in our unit string.
    return rFormatString.copy(nPos + 1, rFormatString.getLength() - nPos - 2);
}


OUString extractUnitStringForCell(ScAddress& rAddress, ScDocument* pDoc) {
    sal_uInt32 nFormat = pDoc->GetNumberFormat(rAddress);
    const SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
    const SvNumberformat* pFormat = pFormatter->GetEntry( nFormat );
    const OUString& rFormatString = pFormat->GetFormatstring();

    return extractUnitStringFromFormat(rFormatString);
}

// get units for single ref -- use format, but then fall back to header?
// or have some sort of marker per cell? (Per cell range -- linked to mdds?)

UnitP getUnitForRef(FormulaToken* pToken, const ScAddress& rFormulaAddress,
                    ScDocument* pDoc, ::boost::shared_ptr< ut_system > pUnitSystem) {
    assert(pToken->GetType() == formula::svSingleRef);

    ScSingleRefData* pRef = pToken->GetSingleRef();
    assert(pRef);

    // Addresses can/will be relative to the formula, for extracting
    // units however we will need to get the absolute address (i.e.
    // by adding the current address to the relative formula address).
    ScAddress aInputAddress = pRef->toAbs( rFormulaAddress );

    // udunits requires strings to be trimmed before parsing -- it's easiest to do this
    // using the OUString utils (as opposed to using ut_trim once we have a c string.
    OUString sUnitString = extractUnitStringForCell(aInputAddress, pDoc).trim();

    // empty string == dimensionless unit. ut_parse returns an error for an empty string
    // hence we need to manually detect that case and return the dimensionless unit.
    if (sUnitString.getLength() == 0) {
        SAL_INFO("sc.units", "empty unit string: returning dimensionless unit");
        return UnitP(ut_get_dimensionless_unit_one(pUnitSystem.get()));
    }

    SAL_INFO("sc.units", "got unit string [" << sUnitString << "]");
    OString sUnitStringUTF8 = OUStringToOString(sUnitString, RTL_TEXTENCODING_UTF8);

    // TODO: we should probably have a cache of unit strings here to save reparsing
    // on every run?

    UnitP pUnit(ut_parse(pUnitSystem.get(), sUnitStringUTF8.getStr(), UT_UTF8));

    if (!pUnit) {
        SAL_INFO("sc.units", "no unit obtained for token at cell " << aInputAddress.GetColRowString());
        SAL_INFO("sc.units", "error encountered: " << dumpUTStatus());
    }

    return pUnit;
}

// getUnitForRef: check format -> if not in format, use more complicated method? (Format overrides header definition)

bool Units::verifyFormula(ScTokenArray* pArray, const ScAddress& rFormulaAddress, ScDocument* pDoc) {
#if DEBUG_FORMULA_COMPILER
    pArray->Dump();
#endif

    stack< UnitP > aUnitStack;

    FormulaToken* pToken = pArray->FirstRPN();

    while (pToken != 0) {
        switch (pToken->GetType()) {
        case formula::svSingleRef:
        {
            UnitP pUnit(getUnitForRef(pToken, rFormulaAddress, pDoc, mpUnitSystem));

            if (!pUnit) {
                SAL_INFO("sc.units", "no unit returned for scSingleRef, ut_status: " << dumpUTStatus());

                // This only happens in case of parsing (or system) errors.
                // However maybe we should be returning "unverified" for
                // unparseable formulas?
                // (or even have a "can't be verified" state too?)
                // see below for more.
                return false;
            }

            aUnitStack.push(pUnit);
            break;
        }
        case formula::svByte:
        {
            if (!(aUnitStack.size() >= 2)) {
                SAL_WARN("sc.units", "less than two units on stack when attempting binary operation");
                return false;
            }

            UnitP pSecondUnit = aUnitStack.top();
            aUnitStack.pop();
            UnitP pFirstUnit = aUnitStack.top();
            aUnitStack.pop();

            UnitP pOut = getOutputUnitsForOpCode(pFirstUnit, pSecondUnit, pToken->GetOpCode());

            // A null unit indicates either invalid units and/or other erronous input
            // i.e. is an indication that getOutputUnitsForOpCode failed.
            if (pOut) {
                aUnitStack.push(pOut);
            } else {
                return false;
            }

            break;
        }
        default:
            // We can't parse any other types of tokens yet, so assume that the formula
            // was correct.
            // TODO: maybe we should have a "unverified" return state instead?
            SAL_WARN("sc.units", "Unrecognised token type " << pToken->GetType());
            return true;
        }

        pToken = pArray->NextRPN();
    }

    // TODO: only fail if actual parsing fails?

    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
