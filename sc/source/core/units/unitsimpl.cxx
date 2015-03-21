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
#include "stringutil.hxx"
#include "tokenarray.hxx"

#include <comphelper/string.hxx>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <rtl/bootstrap.hxx>
#include <svl/zformat.hxx>

#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/util/SearchAlgorithms.hpp>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/XTextSearch.hpp>

#include <boost/scoped_array.hpp>

using namespace com::sun::star;
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
            return UtUnit();
        }

        UtUnit pUnit = rUnitStack.top();
        rUnitStack.pop();

        switch (rOpCode) {
        case ocNot:
            if (!pUnit.isDimensionless()) {
                return UtUnit();
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
            return UtUnit();
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
                SAL_INFO("sc.units", "verified equality for unit " << pFirstUnit);
            } else {
                // TODO: notify/link UI.
            }
            break;
        case ocMul:
            pOut = pFirstUnit * pSecondUnit;
            break;
        case ocDiv:
            pOut = pFirstUnit / pSecondUnit;
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


OUString UnitsImpl::extractUnitStringForCell(const ScAddress& rAddress, ScDocument* pDoc) {
    sal_uInt32 nFormat = pDoc->GetNumberFormat(rAddress);
    const SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
    const SvNumberformat* pFormat = pFormatter->GetEntry( nFormat );
    const OUString& rFormatString = pFormat->GetFormatstring();

    return extractUnitStringFromFormat(rFormatString);
}

bool UnitsImpl::findUnitInStandardHeader(const OUString& rsHeader, UtUnit& aUnit, OUString& sUnitString) {
    // TODO: we should do a sanity check that there's only one such unit though (and fail if there are multiple).
    //       Since otherwise there's no way for us to know which unit is the intended one, hence we need to get
    //       the user to deconfuse us by correcting their header to only contain the intended unit.
    com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext> xContext = comphelper::getProcessComponentContext();

    uno::Reference<lang::XMultiServiceFactory> xFactory(xContext->getServiceManager(), uno::UNO_QUERY_THROW);

    uno::Reference<util::XTextSearch> xSearch =
        uno::Reference< util::XTextSearch >(
            xFactory->createInstance(
                "com.sun.star.util.TextSearch"), uno::UNO_QUERY_THROW);

    util::SearchOptions aOptions;
    aOptions.algorithmType = util::SearchAlgorithms_REGEXP ;
    aOptions.searchFlag = util::SearchFlags::ALL_IGNORE_CASE;

    aOptions.searchString = "\\[([^\\]]+)\\]"; // Grab the contents between [ and ].
    xSearch->setOptions( aOptions );

    util::SearchResult aResult;
    sal_Int32 nStartPosition = rsHeader.getLength();
    while (nStartPosition) {
        // Search from the back since units are more likely to be at the end of the header.
        aResult = xSearch->searchBackward(rsHeader, nStartPosition, 0);

        // We have either 0 items (no match), or 2 (matched string + the group within)
        if (aResult.subRegExpressions != 2) {
            break;
        } else {
            // Confusingly (to me) when doing a backwards search we end up with: endOffset < startOffset.
            // i.e. startOffset is the last character of the intended substring, endOffset the first character.
            // We specifically grab the offsets for the first actual regex group, which are stored in [1], the indexes
            // at [0] represent the whole matched string (i.e. including square brackets).
            sUnitString = rsHeader.copy(aResult.endOffset[1], aResult.startOffset[1] - aResult.endOffset[1]);

            if (UtUnit::createUnit(sUnitString, aUnit, mpUnitSystem)) {
                return true;
            }

            nStartPosition = aResult.endOffset[0];
        }
    }
    sUnitString.clear();
    return false;
}


bool UnitsImpl::findFreestandingUnitInHeader(const OUString& rsHeader, UtUnit& aUnit, OUString& sUnitString) {
    // We just split the string and test whether each token is either a valid unit in it's own right,
    // or is an operator that could glue together multiple units (i.e. multiplication/division).
    // This is sufficient for when there are spaces between elements composing the unit, and none
    // of the individual elements starts or begins with an operator.
    // There's an inherent limit to how well we can cope with various spacing issues here without
    // a ton of computational complexity.
    // E.g. by parsing in this way we might end up with unmatched parentheses which udunits won't like
    // (thus rejecting the unit) etc.

    const sal_Int32 nTokenCount = comphelper::string::getTokenCount(rsHeader, ' ');
    const OUString sOperators = "/*"; // valid
    sUnitString.clear();
    for (sal_Int32 nToken = 0; nToken < nTokenCount; nToken++) {
        OUString sToken = rsHeader.getToken(nToken, ' ');
        UtUnit aTestUnit;
        if (UtUnit::createUnit(sToken, aTestUnit, mpUnitSystem) ||
            // Only test for a separator character if we have already got something in our string, as
            // some of the operators could be used as separators from description to unit
            // (e.g. "a description / kg").
            ((sUnitString.getLength() > 0) && (sToken.getLength() == 1) && (sOperators.indexOf(sToken[0]) != -1))) {
                // we're repeatedly testing the string hence using an OUStringBuffer isn't of much use since there's
                // no simple/efficient way of repeatedly getting a testable OUString from the buffer.
                sUnitString += sToken;
        } else if (sUnitString.getLength() > 0) {
            // If we have units, followed by text, followed by units, we should still flag an error since
            // that's ambiguous (unless the desired units are enclose in [] in which case we've
            // already extracted these desired units in step 1 above.
            break;
        }
    }

    // We test the length to make sure we don't return the dimensionless unit 1 if we haven't found any units
    // in the header.
    if (sUnitString.getLength() && UtUnit::createUnit(sUnitString, aUnit, mpUnitSystem)) {
        return true;
    }
    sUnitString.clear();
    return false;
}

bool UnitsImpl::extractUnitFromHeaderString(const OUString& rsHeader, UtUnit& aUnit, OUString& sUnitString) {
    // 1. Ideally we have units in a 'standard' format, i.e. enclose in square brackets:
    if (findUnitInStandardHeader(rsHeader, aUnit, sUnitString)) {
        return true;
    }

    // 2. But if not we check for free-standing units
    if (findFreestandingUnitInHeader(rsHeader, aUnit, sUnitString)) {
        return true;
    }

    // 3. Give up
    aUnit = UtUnit(); // assign invalid
    sUnitString.clear();
    return false;
}

UtUnit UnitsImpl::getUnitForRef(FormulaToken* pToken, const ScAddress& rFormulaAddress,
                    ScDocument* pDoc) {
    assert(pToken->GetType() == formula::svSingleRef);

    ScSingleRefData* pRef = pToken->GetSingleRef();
    assert(pRef);

    // Addresses can/will be relative to the formula, for extracting
    // units however we will need to get the absolute address (i.e.
    // by adding the current address to the relative formula address).
    const ScAddress aInputAddress = pRef->toAbs( rFormulaAddress );

    OUString sUnitString = extractUnitStringForCell(aInputAddress, pDoc);

    UtUnit aUnit;
    if (sUnitString.getLength() > 0 &&
        UtUnit::createUnit(sUnitString, aUnit, mpUnitSystem)) {
        return aUnit;
    }

    OUString aHeaderUnitString; // Unused -- passed by reference below
    ScAddress aHeaderAddress; // Unused too
    UtUnit aHeaderUnit = findHeaderUnitForCell(aInputAddress, pDoc, aHeaderUnitString, aHeaderAddress);
    if (aHeaderUnit.isValid())
        return aHeaderUnit;

    SAL_INFO("sc.units", "no unit obtained for token at cell " << aInputAddress.GetColRowString());

    // We return the dimensionless unit 1 if we don't find any other data suggesting a unit.
    UtUnit::createUnit("", aUnit, mpUnitSystem);
    return aUnit;
}

UtUnit UnitsImpl::findHeaderUnitForCell(const ScAddress& rCellAddress,
                                        ScDocument* pDoc,
                                        OUString& rsHeaderUnitString,
                                        ScAddress& rHeaderAddress) {
    // Scan UPwards from the current cell to find a header. This is since we could potentially
    // have two different sets of data sharing a column, hence finding the closest header is necessary.
    rHeaderAddress = rCellAddress;
    while (rHeaderAddress.Row() > 0) {
        rHeaderAddress.IncRow(-1);

        // We specifically test for string cells as intervening data cells could have
        // differently defined units of their own. (However as these intervening cells
        // will have the unit stored in the number format it would be ignored when
        // checking the cell's string anyway.)
        UtUnit aUnit;
        if (pDoc->GetCellType(rHeaderAddress) == CELLTYPE_STRING &&
            extractUnitFromHeaderString(pDoc->GetString(rHeaderAddress), aUnit, rsHeaderUnitString)) {
            // TODO: one potential problem is that we could have a text only "united" data cell
            // (where the unit wasn't automatically extracted due to being entered via
            // a different spreadsheet program).
            // We could solve that maybe by trying the unit extraction for such cells first?
            // (I.e if(extractUnitStringForCell(...)) -> do the splitUnitsFrom... dance.
            //
            // TODO: and what if there are multiple units in the header (for whatever reason?)?
            // We can probably just warn the user that we'll be giving them garbage in that case?
            return aUnit;
        }
    }
    rHeaderAddress.SetInvalid();
    rsHeaderUnitString.clear();
    return UtUnit();
}

// getUnitForRef: check format -> if not in format, use more complicated method? (Format overrides header definition)
bool UnitsImpl::verifyFormula(ScTokenArray* pArray, const ScAddress& rFormulaAddress, ScDocument* pDoc) {
#if DEBUG_FORMULA_COMPILER
    pArray->Dump();
#endif

    stack< UtUnit > aUnitStack;

    for (FormulaToken* pToken = pArray->FirstRPN(); pToken != 0; pToken = pArray->NextRPN()) {
        switch (pToken->GetType()) {
        case formula::svSingleRef:
        {
            UtUnit pUnit(getUnitForRef(pToken, rFormulaAddress, pDoc));

            if (!pUnit.isValid()) {
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
            if (pOut.isValid()) {
                aUnitStack.push(pOut);
            } else {
                return false;
            }

            break;
        }
        // As far as I can tell this is only used for an actual numerical value
        // in which case we just want to use it as scaling factor
        // (for example [m] + [cm]/100 would be a valid operation)
        case formula::svDouble:
        {
            UtUnit aScale;
            UtUnit::createUnit("", aScale, mpUnitSystem); // Get the dimensionless unit 1
            aScale = aScale*(pToken->GetDouble());

            aUnitStack.push(aScale);
            break;
        }
        default:
            // We can't parse any other types of tokens yet, so assume that the formula
            // was correct.
            // TODO: maybe we should have a "unverified" return state instead?
            SAL_WARN("sc.units", "Unrecognised token type " << pToken->GetType());
            return true;
        }
    }

    // TODO: only fail if actual parsing fails?

    return true;
}

bool IsDigit(sal_Unicode c) {
    return (c>= '0' && c <= '9');
}

bool UnitsImpl::splitUnitsFromInputString(const OUString& rInput, OUString& rValueOut, OUString& rUnitOut) {
    int nPos = rInput.getLength();

    while (nPos) {
        if (IsDigit(rInput[nPos-1])) {
            break;
        }
        nPos--;
    }

    rUnitOut = rInput.copy(nPos);

    UtUnit aUnit;
    // If the entire input is a string (nPos == 0) then treating it as a unit
    // makes little sense as there is no numerical value associated with it.
    // Hence it makes sense to skip testing in this case.
    // We also need to specifically ignore the no unit case (nPos == rInput.getLength())
    // as otherwise we are obtaining the unit for "" which is a valid unit
    // (the dimensionless) unit, even though in reality we should obtain no unit
    // and return false.
    if ((nPos < rInput.getLength())
        && (nPos > 0)
        && UtUnit::createUnit(rUnitOut, aUnit, mpUnitSystem)) {
        rValueOut = rInput.copy(0, nPos);
        return true;
    } else {
        rValueOut = rInput;
        rUnitOut.clear();
        return false;
    }
}

bool UnitsImpl::isCellConversionRecommended(const ScAddress& rCellAddress,
                                 ScDocument* pDoc,
                                 OUString& rsHeaderUnit,
                                 ScAddress& rHeaderCellAddress,
                                 OUString& rsCellUnit) {
    assert(rCellAddress.IsValid());

    UtUnit aCellUnit;
    rsCellUnit = extractUnitStringForCell(rCellAddress, pDoc);

    if (!rsCellUnit.isEmpty() && UtUnit::createUnit(rsCellUnit, aCellUnit, mpUnitSystem)) {
        UtUnit aHeaderUnit = findHeaderUnitForCell(rCellAddress, pDoc, rsHeaderUnit, rHeaderCellAddress);
        if (rHeaderCellAddress.IsValid()) {
            if (aHeaderUnit.areConvertibleTo(aCellUnit)) {
                return true;
            }
        }
    }

    rsHeaderUnit.clear();
    rHeaderCellAddress.SetInvalid();
    rsCellUnit.clear();
    return false;
}

bool UnitsImpl::convertCellToHeaderUnit(const ScAddress& rCellAddress,
                             ScDocument* pDoc,
                             const OUString& rsNewUnit,
                             const OUString& rsOldUnit) {
    assert(rCellAddress.IsValid());

    OUString sCellUnit = extractUnitStringForCell(rCellAddress, pDoc);
    UtUnit aOldUnit;
    UtUnit::createUnit(sCellUnit, aOldUnit, mpUnitSystem);

    OUString sHeaderUnitFound;
    ScAddress aHeaderAddress; // Unused, but passed by reference
    UtUnit aNewUnit = findHeaderUnitForCell(rCellAddress, pDoc, sHeaderUnitFound, aHeaderAddress);

    // We test that we still have all data in the same format as expected.
    // This is maybe a tad defensive, but this call is most likely to be delayed
    // relative to isCellConversionRecommended (e.g. if the user is asked for
    // confirmation that conversion is desired), hence it's entirely feasible
    // for data to be changed in the document but this action to be still
    // called afterwards (especially for non-modal interactions, e.g.
    // with an infobar which can remain open whilst the document is edited).
    if ((sCellUnit == rsOldUnit) &&
        (sHeaderUnitFound == rsNewUnit) &&
        (pDoc->GetCellType(rCellAddress) == CELLTYPE_VALUE)) {
        assert(aOldUnit.areConvertibleTo(aNewUnit));
        double nOldValue = pDoc->GetValue(rCellAddress);
        double nNewValue = aOldUnit.convertValueTo(nOldValue, aNewUnit);

        pDoc->SetValue(rCellAddress, nNewValue);
        pDoc->SetNumberFormat(rCellAddress, 0); // 0 == no number format?

        return true;
    }

    // In an ideal scenario the UI is written such that we never reach this point,
    // however that is likely to be hard to achieve, hence we still allow
    // for this case (see above for more information).
    SAL_INFO("sc.units", "Unit conversion cancelled: units changed in meantime.");
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
