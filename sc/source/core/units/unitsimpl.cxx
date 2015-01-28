/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#include "unitsimpl.hxx"

#include "util.hxx"

#include "document.hxx"
#include "refdata.hxx"
#include "tokenarray.hxx"

#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <rtl/bootstrap.hxx>
#include <svl/zformat.hxx>

#include <boost/scoped_array.hpp>

using namespace formula;
using namespace sc;
using namespace sc::units;
using namespace std;

::osl::Mutex sc::units::UnitsImpl::ourSingletonMutex;
::boost::weak_ptr< UnitsImpl > sc::units::UnitsImpl::ourUnits;

::boost::shared_ptr< UnitsImpl > UnitsImpl::GetUnits() {
    osl::MutexGuard aGuard(ourSingletonMutex);
    boost::shared_ptr< UnitsImpl > pUnits = ourUnits.lock();

    if (!pUnits) {
        pUnits.reset( new UnitsImpl() );
        ourUnits = pUnits;
    }
    return pUnits;
}

UnitsImpl::UnitsImpl() {
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

UnitsImpl::~UnitsImpl() {
    // We only arrive here if all shared_ptr's to our Units get
    // disposed. In this case the weak_ptr is already cleared,
    // and any new calls to GetUnits don't need to care at what
    // stage of destruction we are?

    // We might need to lock on our singletonMutex if we can't
    // load the same unit system multiple times in memory
    // (i.e. if udunits can't handle being used across threads)?
}

UtUnit UnitsImpl::getOutputUnitsForOpCode(stack< UtUnit >& rUnitStack, const OpCode& rOpCode) {
    UtUnit pOut;

    auto nOpCode = static_cast<std::underlying_type<const OpCode>::type>(rOpCode);

    // TODO: sc/source/core/tool/parclass.cxx has a mapping of opcodes to possible operands, which we
    // should probably be using in practice.

    if (nOpCode >= SC_OPCODE_START_UN_OP &&
        nOpCode < SC_OPCODE_STOP_UN_OP) {

        if (!(rUnitStack.size() >= 1)) {
            SAL_WARN("sc.units", "no units on stack for unary operation");
            return 0;
        }

        UtUnit pUnit = rUnitStack.top();
        rUnitStack.pop();

        switch (rOpCode) {
        case ocNot:
            if (!ut_is_dimensionless(pUnit.get())) {
                return 0;
            }
            // We just keep the same unit (in this case no unit) so can
            // fall through.
        case ocNeg:
            // fall through -- same as OcNegSub
            // It seems the difference is that ocNeg: 'NEG(value)', and ocNegSub: '-value' when
            // in human readable form.
        case ocNegSub:
            // do nothing: since we're just negating the value which doesn't
            // affect units in any way, we just return the current unit.
            pOut = pUnit;
            break;
        default:
            // Only the above 3 opcodes are in the range we have tested for previously
            // (...START_UN_OP to ...STOP_UN_OP).
            assert(false);
        }
    } else if (nOpCode >= SC_OPCODE_START_BIN_OP &&
        nOpCode < SC_OPCODE_STOP_BIN_OP) {

        if (!(rUnitStack.size() >= 2)) {
            SAL_WARN("sc.units", "less than two units on stack when attempting binary operation");
            // TODO: what should we be telling the user in this case? Can this even happen (i.e.
            // should we just be asserting here?)
            return 0;
        }

        UtUnit pSecondUnit = rUnitStack.top();
        rUnitStack.pop();
        UtUnit pFirstUnit = rUnitStack.top();
        rUnitStack.pop();

        switch (rOpCode) {
        case ocAdd:
            // Adding and subtracting both require the same units on both sides
            // hence we can just fall through / use the same logic.
        case ocSub:
            if (pFirstUnit == pSecondUnit) {
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
            SAL_INFO("sc.units", "unit verification not supported for opcode: " << nOpCode);
            assert(false);
        }

    } else {
        SAL_INFO("sc.units", "unit verification not supported for opcode: " << nOpCode);
    }
    // TODO: else if unary, or no params, or ...
    // TODO: implement further sensible opcode handling

    return pOut;
}

OUString UnitsImpl::extractUnitStringFromFormat(const OUString& rFormatString) {
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


OUString UnitsImpl::extractUnitStringForCell(ScAddress& rAddress, ScDocument* pDoc) {
    sal_uInt32 nFormat = pDoc->GetNumberFormat(rAddress);
    const SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
    const SvNumberformat* pFormat = pFormatter->GetEntry( nFormat );
    const OUString& rFormatString = pFormat->GetFormatstring();

    return extractUnitStringFromFormat(rFormatString);
}

UtUnit UnitsImpl::getUnitForRef(FormulaToken* pToken, const ScAddress& rFormulaAddress,
                    ScDocument* pDoc) {
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
        return UtUnit(ut_get_dimensionless_unit_one(mpUnitSystem.get()));
    }

    SAL_INFO("sc.units", "got unit string [" << sUnitString << "]");
    OString sUnitStringUTF8 = OUStringToOString(sUnitString, RTL_TEXTENCODING_UTF8);

    // TODO: we should probably have a cache of unit strings here to save reparsing
    // on every run?

    UtUnit pUnit(ut_parse(mpUnitSystem.get(), sUnitStringUTF8.getStr(), UT_UTF8));

    if (!pUnit) {
        SAL_INFO("sc.units", "no unit obtained for token at cell " << aInputAddress.GetColRowString());
        SAL_INFO("sc.units", "error encountered: " << getUTStatus());
    }

    return pUnit;
}

// getUnitForRef: check format -> if not in format, use more complicated method? (Format overrides header definition)
bool UnitsImpl::verifyFormula(ScTokenArray* pArray, const ScAddress& rFormulaAddress, ScDocument* pDoc) {
#if DEBUG_FORMULA_COMPILER
    pArray->Dump();
#endif

    stack< UtUnit > aUnitStack;

    FormulaToken* pToken = pArray->FirstRPN();

    while (pToken != 0) {
        switch (pToken->GetType()) {
        case formula::svSingleRef:
        {
            UtUnit pUnit(getUnitForRef(pToken, rFormulaAddress, pDoc));

            if (!pUnit) {
                SAL_INFO("sc.units", "no unit returned for scSingleRef, ut_status: " << getUTStatus());

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
            UtUnit pOut = getOutputUnitsForOpCode(aUnitStack, pToken->GetOpCode());

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
