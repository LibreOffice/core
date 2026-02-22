/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <rtl/math.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/weld/MessageDialog.hxx>
#include <solveruno.hxx>
#include <docsh.hxx>
#include <docfunc.hxx>
#include <address.hxx>
#include <table.hxx>
#include <convuno.hxx>
#include <compiler.hxx>
#include <solverutil.hxx>
#include <rangeutl.hxx>
#include <scresid.hxx>
#include <globstr.hrc>
#include <optsolver.hxx>
#include <unonames.hxx>
#include <SolverSettings.hxx>
#include <o3tl/string_view.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/sheet/XSolver.hpp>
#include <com/sun/star/sheet/XSolverDescription.hpp>

using namespace css;

constexpr OUString SC_SOLVERSETTINGS_SERVICE = u"com.sun.star.sheet.SolverSettings"_ustr;

namespace
{
// Returns the sc::ConstraintOperator equivalent to the Uno operator
sc::ConstraintOperator getScOperatorFromUno(sheet::SolverConstraintOperator aOperator)
{
    sc::ConstraintOperator aRet(sc::ConstraintOperator::CO_LESS_EQUAL);

    switch (aOperator)
    {
        case sheet::SolverConstraintOperator_EQUAL:
            aRet = sc::ConstraintOperator::CO_EQUAL;
            break;
        case sheet::SolverConstraintOperator_GREATER_EQUAL:
            aRet = sc::ConstraintOperator::CO_GREATER_EQUAL;
            break;
        case sheet::SolverConstraintOperator_BINARY:
            aRet = sc::ConstraintOperator::CO_BINARY;
            break;
        case sheet::SolverConstraintOperator_INTEGER:
            aRet = sc::ConstraintOperator::CO_INTEGER;
            break;
        default:
        {
            // This should never be reached
        }
    }
    return aRet;
}

// Returns the sheet::SolverConstraintOperator equivalent to sc::ConstraintOperator
sheet::SolverConstraintOperator getUnoOperatorFromSc(sc::ConstraintOperator nOperator)
{
    sheet::SolverConstraintOperator aRet(sheet::SolverConstraintOperator_LESS_EQUAL);

    switch (nOperator)
    {
        case sc::ConstraintOperator::CO_EQUAL:
            aRet = sheet::SolverConstraintOperator_EQUAL;
            break;
        case sc::ConstraintOperator::CO_GREATER_EQUAL:
            aRet = sheet::SolverConstraintOperator_GREATER_EQUAL;
            break;
        case sc::ConstraintOperator::CO_BINARY:
            aRet = sheet::SolverConstraintOperator_BINARY;
            break;
        case sc::ConstraintOperator::CO_INTEGER:
            aRet = sheet::SolverConstraintOperator_INTEGER;
            break;
        default:
        {
            // This should never be reached
        }
    }
    return aRet;
}

// Returns the CellRangeAddress struct from a ScRange
table::CellRangeAddress getRangeAddress(ScRange aRange)
{
    table::CellRangeAddress aRet;
    aRet.Sheet = aRange.aStart.Tab();
    aRet.StartColumn = aRange.aStart.Col();
    aRet.StartRow = aRange.aStart.Row();
    aRet.EndColumn = aRange.aEnd.Col();
    aRet.EndRow = aRange.aEnd.Row();
    return aRet;
}

// Tests if a string is a valid number
bool isValidNumber(const OUString& sValue, double& fValue)
{
    if (sValue.isEmpty())
        return false;

    rtl_math_ConversionStatus eConvStatus;
    sal_Int32 nEnd;
    fValue = rtl::math::stringToDouble(sValue, ScGlobal::getLocaleData().getNumDecimalSep()[0],
                                       ScGlobal::getLocaleData().getNumThousandSep()[0],
                                       &eConvStatus, &nEnd);
    // A conversion is only valid if nEnd is equal to the string length (all chars processed)
    return nEnd == sValue.getLength();
}
}

ScSolverSettings::ScSolverSettings(ScDocShell* pDocSh, uno::Reference<container::XNamed> xSheet)
    : m_pDocShell(pDocSh)
    , m_rDoc(m_pDocShell->GetDocument())
    , m_xSheet(std::move(xSheet))
    , m_nStatus(sheet::SolverStatus::NONE)
    , m_bSuppressDialog(false)
    , m_pTable(nullptr)
{
    // Initialize member variables with information about the current sheet
    OUString aName = m_xSheet->getName();
    SCTAB nTab;
    if (m_rDoc.GetTable(aName, nTab))
    {
        m_pTable = m_rDoc.FetchTable(nTab);
        m_pSettings = m_pTable->GetSolverSettings();
    }
}

ScSolverSettings::~ScSolverSettings() {}

bool ScSolverSettings::ParseRef(ScRange& rRange, const OUString& rInput, bool bAllowRange)
{
    ScAddress::Details aDetails(m_rDoc.GetAddressConvention(), 0, 0);
    ScRefFlags nFlags = rRange.ParseAny(rInput, m_rDoc, aDetails);
    SCTAB nCurTab(m_pTable->GetTab());
    if (nFlags & ScRefFlags::VALID)
    {
        if ((nFlags & ScRefFlags::TAB_3D) == ScRefFlags::ZERO)
            rRange.aStart.SetTab(nCurTab);
        if ((nFlags & ScRefFlags::TAB2_3D) == ScRefFlags::ZERO)
            rRange.aEnd.SetTab(rRange.aStart.Tab());
        return (bAllowRange || rRange.aStart == rRange.aEnd);
    }
    else if (ScRangeUtil::MakeRangeFromName(rInput, m_rDoc, nCurTab, rRange, RUTL_NAMES, aDetails))
        return (bAllowRange || rRange.aStart == rRange.aEnd);

    return false;
}

bool ScSolverSettings::ParseWithNames(ScRangeList& rRanges, std::u16string_view rInput)
{
    if (rInput.empty())
        return true;

    ScAddress::Details aDetails(m_rDoc.GetAddressConvention(), 0, 0);
    SCTAB nCurTab(m_pTable->GetTab());
    sal_Unicode cDelimiter = ScCompiler::GetNativeSymbolChar(OpCode::ocSep);
    bool bError = false;
    sal_Int32 nIdx(0);
    do
    {
        ScRange aRange;
        OUString aRangeStr(o3tl::getToken(rInput, 0, cDelimiter, nIdx));
        ScRefFlags nFlags = aRange.ParseAny(aRangeStr, m_rDoc, aDetails);
        if (nFlags & ScRefFlags::VALID)
        {
            if ((nFlags & ScRefFlags::TAB_3D) == ScRefFlags::ZERO)
                aRange.aStart.SetTab(nCurTab);
            if ((nFlags & ScRefFlags::TAB2_3D) == ScRefFlags::ZERO)
                aRange.aEnd.SetTab(aRange.aStart.Tab());
            rRanges.push_back(aRange);
        }
        else if (ScRangeUtil::MakeRangeFromName(aRangeStr, m_rDoc, nCurTab, aRange, RUTL_NAMES,
                                                aDetails))
            rRanges.push_back(aRange);
        else
            bError = true;
    } while (nIdx > 0);

    return !bError;
}

void ScSolverSettings::ShowErrorMessage(const OUString& rMessage)
{
    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
        Application::GetDefDialogParent(), VclMessageType::Warning, VclButtonsType::Ok, rMessage));
    xBox->run();
}

// XSolverSettings
sal_Int8 SAL_CALL ScSolverSettings::getObjectiveType()
{
    sal_Int8 aRet(sheet::SolverObjectiveType::MAXIMIZE);
    switch (m_pSettings->GetObjectiveType())
    {
        case sc::ObjectiveType::OT_MINIMIZE:
            aRet = sheet::SolverObjectiveType::MINIMIZE;
            break;
        case sc::ObjectiveType::OT_VALUE:
            aRet = sheet::SolverObjectiveType::VALUE;
            break;
        default:
        {
            // This should never be reached
        }
    }
    return aRet;
}

void SAL_CALL ScSolverSettings::setObjectiveType(sal_Int8 aObjType)
{
    sc::ObjectiveType eType(sc::ObjectiveType::OT_MAXIMIZE);
    switch (aObjType)
    {
        case sheet::SolverObjectiveType::MINIMIZE:
            eType = sc::ObjectiveType::OT_MINIMIZE;
            break;
        case sheet::SolverObjectiveType::VALUE:
            eType = sc::ObjectiveType::OT_VALUE;
            break;
        default:
        {
            // This should never be reached
        }
    }
    m_pSettings->SetObjectiveType(eType);
}

uno::Any SAL_CALL ScSolverSettings::getObjectiveCell()
{
    // The objective cell must be a valid cell address
    OUString sValue(m_pSettings->GetParameter(sc::SolverParameter::SP_OBJ_CELL));

    // Test if it is a valid cell reference; if so, return its CellAddress
    ScRange aRange;
    const formula::FormulaGrammar::AddressConvention eConv = m_rDoc.GetAddressConvention();
    bool bOk = (aRange.ParseAny(sValue, m_rDoc, eConv) & ScRefFlags::VALID) == ScRefFlags::VALID;
    if (bOk)
    {
        SCTAB nTab1, nTab2;
        SCROW nRow1, nRow2;
        SCCOL nCol1, nCol2;
        aRange.GetVars(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
        table::CellAddress aAddress(nTab1, nCol1, nRow1);
        return uno::Any(aAddress);
    }

    // If converting to a CellAddress fails, returns the raw string
    return uno::Any(sValue);
}

// The value being set must be either a string referencing a single cell or
// a CellAddress instance
void SAL_CALL ScSolverSettings::setObjectiveCell(const uno::Any& aValue)
{
    // Check if a string value is being used
    OUString sValue;
    bool bIsString(aValue >>= sValue);
    if (bIsString)
    {
        // The string must correspond to a valid range; if not, an empty string is set
        ScRange aRange;
        OUString sRet;
        ScDocument& rDoc = m_pDocShell->GetDocument();
        const formula::FormulaGrammar::AddressConvention eConv = rDoc.GetAddressConvention();
        bool bOk = (aRange.ParseAny(sValue, rDoc, eConv) & ScRefFlags::VALID) == ScRefFlags::VALID;
        if (bOk)
        {
            SCTAB nTab1, nTab2;
            SCROW nRow1, nRow2;
            SCCOL nCol1, nCol2;
            aRange.GetVars(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
            // The range must consist of a single cell
            if (nTab1 == nTab2 && nCol1 == nCol2 && nRow1 == nRow2)
                sRet = sValue;
        }
        m_pSettings->SetParameter(sc::SolverParameter::SP_OBJ_CELL, sRet);
        return;
    }

    // Check if a CellAddress is being used
    table::CellAddress aUnoAddress;
    bool bIsAddress(aValue >>= aUnoAddress);
    if (bIsAddress)
    {
        OUString sRet;
        ScAddress aAdress(aUnoAddress.Column, aUnoAddress.Row, aUnoAddress.Sheet);
        sRet = aAdress.Format(ScRefFlags::RANGE_ABS, &m_rDoc);
        m_pSettings->SetParameter(sc::SolverParameter::SP_OBJ_CELL, sRet);
        return;
    }

    // If all fails, set an empty string
    m_pSettings->SetParameter(sc::SolverParameter::SP_OBJ_CELL, "");
}

uno::Any SAL_CALL ScSolverSettings::getGoalValue()
{
    OUString sValue(m_pSettings->GetParameter(sc::SolverParameter::SP_OBJ_VAL));

    // Test if it is a valid cell reference; if so, return its CellAddress
    ScRange aRange;
    const formula::FormulaGrammar::AddressConvention eConv = m_rDoc.GetAddressConvention();
    bool bOk = (aRange.ParseAny(sValue, m_rDoc, eConv) & ScRefFlags::VALID) == ScRefFlags::VALID;
    if (bOk)
    {
        SCTAB nTab1, nTab2;
        SCROW nRow1, nRow2;
        SCCOL nCol1, nCol2;
        aRange.GetVars(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
        table::CellAddress aAddress(nTab1, nCol1, nRow1);
        return uno::Any(aAddress);
    }

    double fValue;
    bool bValid = isValidNumber(sValue, fValue);
    if (bValid)
        return uno::Any(fValue);

    // If the conversion was not successful, return "empty"
    return uno::Any();
}

void SAL_CALL ScSolverSettings::setGoalValue(const uno::Any& aValue)
{
    // Check if a numeric value is being used
    double fValue;
    bool bIsDouble(aValue >>= fValue);
    if (bIsDouble)
    {
        // The value must be set as a localized number
        OUString sLocalizedValue = rtl::math::doubleToUString(
            fValue, rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
            ScGlobal::getLocaleData().getNumDecimalSep()[0], true);
        m_pSettings->SetParameter(sc::SolverParameter::SP_OBJ_VAL, sLocalizedValue);
        return;
    }

    // Check if a string value is being used
    OUString sValue;
    bool bIsString(aValue >>= sValue);
    if (bIsString)
    {
        // The string must correspond to a valid range; if not, an empty string is set
        ScRange aRange;
        OUString sRet;
        ScDocument& rDoc = m_pDocShell->GetDocument();
        const formula::FormulaGrammar::AddressConvention eConv = rDoc.GetAddressConvention();
        bool bOk = (aRange.ParseAny(sValue, rDoc, eConv) & ScRefFlags::VALID) == ScRefFlags::VALID;
        if (bOk)
        {
            SCTAB nTab1, nTab2;
            SCROW nRow1, nRow2;
            SCCOL nCol1, nCol2;
            aRange.GetVars(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
            // The range must consist of a single cell
            if (nTab1 == nTab2 && nCol1 == nCol2 && nRow1 == nRow2)
                sRet = sValue;
        }
        m_pSettings->SetParameter(sc::SolverParameter::SP_OBJ_VAL, sRet);
        return;
    }

    // Check if a CellAddress is being used
    table::CellAddress aUnoAddress;
    bool bIsAddress(aValue >>= aUnoAddress);
    if (bIsAddress)
    {
        OUString sRet;
        ScAddress aAdress(aUnoAddress.Column, aUnoAddress.Row, aUnoAddress.Sheet);
        sRet = aAdress.Format(ScRefFlags::RANGE_ABS, &m_rDoc);
        m_pSettings->SetParameter(sc::SolverParameter::SP_OBJ_VAL, sRet);
        return;
    }

    // If all fails, set an empty string
    m_pSettings->SetParameter(sc::SolverParameter::SP_OBJ_VAL, "");
}

OUString SAL_CALL ScSolverSettings::getEngine()
{
    return m_pSettings->GetParameter(sc::SP_LO_ENGINE);
}

void SAL_CALL ScSolverSettings::setEngine(const OUString& sEngine)
{
    // Only change the engine if the new engine exists; otherwise leave it unchanged
    uno::Sequence<OUString> arrEngineNames;
    uno::Sequence<OUString> arrDescriptions;
    ScSolverUtil::GetImplementations(arrEngineNames, arrDescriptions);
    if (comphelper::findValue(arrEngineNames, sEngine) == -1)
        return;

    m_pSettings->SetParameter(sc::SP_LO_ENGINE, sEngine);
}

uno::Sequence<OUString> SAL_CALL ScSolverSettings::getAvailableEngines()
{
    uno::Sequence<OUString> arrEngineNames;
    uno::Sequence<OUString> arrDescriptions;
    ScSolverUtil::GetImplementations(arrEngineNames, arrDescriptions);
    return arrEngineNames;
}

uno::Sequence<uno::Any> SAL_CALL ScSolverSettings::getVariableCells()
{
    // Variable cells parameter is stored as a single string composed of valid ranges
    // separated using the formula separator character
    OUString sVarCells(m_pSettings->GetParameter(sc::SP_VAR_CELLS));
    // Delimiter character to separate ranges
    sal_Unicode cDelimiter = ScCompiler::GetNativeSymbolChar(OpCode::ocSep);
    const formula::FormulaGrammar::AddressConvention eConv = m_rDoc.GetAddressConvention();
    uno::Sequence<uno::Any> aRangeSeq;
    sal_Int32 nIdx(0);
    sal_Int32 nArrPos(0);

    do
    {
        OUString aRangeStr(o3tl::getToken(sVarCells, 0, cDelimiter, nIdx));
        // Check if range is valid
        ScRange aRange;
        bool bOk
            = (aRange.ParseAny(aRangeStr, m_rDoc, eConv) & ScRefFlags::VALID) == ScRefFlags::VALID;
        if (bOk)
        {
            table::CellRangeAddress aRangeAddress(getRangeAddress(aRange));
            aRangeSeq.realloc(nArrPos + 1);
            auto pArrRanges = aRangeSeq.getArray();
            pArrRanges[nArrPos] <<= aRangeAddress;
            nArrPos++;
        }
    } while (nIdx > 0);

    return aRangeSeq;
}

void SAL_CALL ScSolverSettings::setVariableCells(const uno::Sequence<uno::Any>& aRanges)
{
    OUString sVarCells;
    bool bFirst(true);
    const formula::FormulaGrammar::AddressConvention eConv = m_rDoc.GetAddressConvention();
    OUStringChar cDelimiter(ScCompiler::GetNativeSymbolChar(OpCode::ocSep));

    for (const auto& rRange : aRanges)
    {
        OUString sRange;
        bool bIsString(rRange >>= sRange);
        bool bOk(false);
        if (bIsString)
        {
            ScRange aRange;
            bOk = (aRange.ParseAny(sRange, m_rDoc, eConv) & ScRefFlags::VALID) == ScRefFlags::VALID;
        }

        table::CellRangeAddress aRangeAddress;
        bool bIsRangeAddress(rRange >>= aRangeAddress);
        if (bIsRangeAddress)
        {
            bOk = true;
            ScRange aRange(aRangeAddress.StartColumn, aRangeAddress.StartRow, aRangeAddress.Sheet,
                           aRangeAddress.EndColumn, aRangeAddress.EndRow, aRangeAddress.Sheet);
            sRange = aRange.Format(m_rDoc, ScRefFlags::RANGE_ABS);
        }

        if (bOk)
        {
            if (bFirst)
            {
                sVarCells = sRange;
                bFirst = false;
            }
            else
            {
                sVarCells += cDelimiter + sRange;
            }
        }
    }

    m_pSettings->SetParameter(sc::SP_VAR_CELLS, sVarCells);
}

uno::Sequence<sheet::ModelConstraint> SAL_CALL ScSolverSettings::getConstraints()
{
    uno::Sequence<sheet::ModelConstraint> aRet;
    std::vector<sc::ModelConstraint> vConstraints = m_pSettings->GetConstraints();
    const formula::FormulaGrammar::AddressConvention eConv = m_rDoc.GetAddressConvention();
    sal_Int32 nCount(0);

    for (const auto& rConst : vConstraints)
    {
        sheet::ModelConstraint aConstraint;

        // Left side: must be valid string representing a cell range
        ScRange aLeftRange;
        bool bIsLeftRange
            = (aLeftRange.ParseAny(rConst.aLeftStr, m_rDoc, eConv) & ScRefFlags::VALID)
              == ScRefFlags::VALID;
        if (bIsLeftRange)
            aConstraint.Left <<= getRangeAddress(aLeftRange);

        // Operator
        aConstraint.Operator = getUnoOperatorFromSc(rConst.nOperator);

        // Right side: must be either
        // - valid string representing a cell range or
        // - a numeric value
        ScRange aRightRange;
        bool bIsRightRange
            = (aRightRange.ParseAny(rConst.aRightStr, m_rDoc, eConv) & ScRefFlags::VALID)
              == ScRefFlags::VALID;
        if (bIsRightRange)
        {
            aConstraint.Right <<= getRangeAddress(aRightRange);
        }
        else
        {
            double fValue;
            bool bValid = isValidNumber(rConst.aRightStr, fValue);
            if (bValid)
                aConstraint.Right <<= fValue;
            else
                aConstraint.Right = uno::Any();
        }

        // Adds the constraint to the sequence
        aRet.realloc(nCount + 1);
        auto pArrConstraints = aRet.getArray();
        pArrConstraints[nCount] = std::move(aConstraint);
        nCount++;
    }

    return aRet;
}

void SAL_CALL
ScSolverSettings::setConstraints(const uno::Sequence<sheet::ModelConstraint>& aConstraints)
{
    const formula::FormulaGrammar::AddressConvention eConv = m_rDoc.GetAddressConvention();
    std::vector<sc::ModelConstraint> vRetConstraints;

    for (const auto& rConst : aConstraints)
    {
        sc::ModelConstraint aNewConst;

        // Left side
        OUString sLeft;
        bool bOkLeft(false);
        bool bIsString(rConst.Left >>= sLeft);
        if (bIsString)
        {
            ScRange aRange;
            bOkLeft
                = (aRange.ParseAny(sLeft, m_rDoc, eConv) & ScRefFlags::VALID) == ScRefFlags::VALID;
        }

        table::CellRangeAddress aLeftRangeAddress;
        bool bIsRangeAddress(rConst.Left >>= aLeftRangeAddress);
        if (bIsRangeAddress)
        {
            bOkLeft = true;
            ScRange aRange(aLeftRangeAddress.StartColumn, aLeftRangeAddress.StartRow,
                           aLeftRangeAddress.Sheet, aLeftRangeAddress.EndColumn,
                           aLeftRangeAddress.EndRow, aLeftRangeAddress.Sheet);
            sLeft = aRange.Format(m_rDoc, ScRefFlags::RANGE_ABS);
        }

        if (bOkLeft)
            aNewConst.aLeftStr = sLeft;

        // Constraint operator
        aNewConst.nOperator = getScOperatorFromUno(rConst.Operator);

        // Right side (may have numeric values)
        OUString sRight;
        bool bOkRight(false);

        double fValue;
        bool bIsDouble(rConst.Right >>= fValue);
        if (bIsDouble)
        {
            bOkRight = true;
            // The value must be set as a localized number
            sRight = rtl::math::doubleToUString(
                fValue, rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                ScGlobal::getLocaleData().getNumDecimalSep()[0], true);
        }

        bIsString = (rConst.Right >>= sRight);
        if (bIsString)
        {
            ScRange aRange;
            bOkRight
                = (aRange.ParseAny(sRight, m_rDoc, eConv) & ScRefFlags::VALID) == ScRefFlags::VALID;
        }

        table::CellRangeAddress aRightRangeAddress;
        bIsRangeAddress = (rConst.Right >>= aRightRangeAddress);
        if (bIsRangeAddress)
        {
            bOkRight = true;
            ScRange aRange(aRightRangeAddress.StartColumn, aRightRangeAddress.StartRow,
                           aRightRangeAddress.Sheet, aRightRangeAddress.EndColumn,
                           aRightRangeAddress.EndRow, aRightRangeAddress.Sheet);
            sRight = aRange.Format(m_rDoc, ScRefFlags::RANGE_ABS);
        }

        if (bOkRight)
            aNewConst.aRightStr = sRight;

        vRetConstraints.push_back(aNewConst);
    }

    m_pSettings->SetConstraints(std::move(vRetConstraints));
}

sal_Int32 SAL_CALL ScSolverSettings::getConstraintCount()
{
    if (!m_pTable)
        return -1;

    return static_cast<sal_Int32>(m_pSettings->GetConstraints().size());
}

uno::Sequence<beans::PropertyValue> SAL_CALL ScSolverSettings::getEngineOptions()
{
    uno::Sequence<beans::PropertyValue> aRet = ScSolverUtil::GetDefaults(getEngine());
    m_pSettings->GetEngineOptions(aRet);
    return aRet;
}

void SAL_CALL ScSolverSettings::setEngineOptions(const uno::Sequence<beans::PropertyValue>& rProps)
{
    m_pSettings->SetEngineOptions(rProps);
}

sal_Int8 SAL_CALL ScSolverSettings::getStatus() { return m_nStatus; }

OUString SAL_CALL ScSolverSettings::getErrorMessage() { return m_sErrorMessage; }

sal_Bool SAL_CALL ScSolverSettings::getSuppressDialog() { return m_bSuppressDialog; }

void SAL_CALL ScSolverSettings::setSuppressDialog(sal_Bool bSuppress)
{
    m_bSuppressDialog = bSuppress;
}

void SAL_CALL ScSolverSettings::reset() { m_pSettings->ResetToDefaults(); }

void SAL_CALL ScSolverSettings::solve()
{
    // Show the progress dialog
    auto xProgress = std::make_shared<ScSolverProgressDialog>(Application::GetDefDialogParent());
    if (!m_bSuppressDialog)
    {
        // Get the value of the timeout property of the solver engine
        uno::Sequence<beans::PropertyValue> aProps(getEngineOptions());
        sal_Int32 nTimeout(0);
        sal_Int32 nPropCount(aProps.getLength());
        bool bHasTimeout(false);
        for (sal_Int32 nProp = 0; nProp < nPropCount && !bHasTimeout; ++nProp)
        {
            const beans::PropertyValue& rValue = aProps[nProp];
            if (rValue.Name == SC_UNONAME_TIMEOUT)
                bHasTimeout = (rValue.Value >>= nTimeout);
        }

        if (bHasTimeout)
            xProgress->SetTimeLimit(nTimeout);
        else
            xProgress->HideTimeLimit();

        weld::DialogController::runAsync(xProgress, [](sal_Int32 /*nResult*/) {});
        // try to make sure the progress dialog is painted before continuing
        Application::Reschedule(true);
    }

    // Check the validity of the objective cell
    ScRange aObjRange;
    if (!ParseRef(aObjRange, m_pSettings->GetParameter(sc::SP_OBJ_CELL), false))
    {
        m_nStatus = sheet::SolverStatus::PARSE_ERROR;
        m_sErrorMessage = ScResId(STR_SOLVER_OBJCELL_FAIL);
        if (!m_bSuppressDialog)
            ScSolverSettings::ShowErrorMessage(m_sErrorMessage);
        return;
    }
    table::CellAddress aObjCell(aObjRange.aStart.Tab(), aObjRange.aStart.Col(),
                                aObjRange.aStart.Row());

    // Check the validity of the variable cells
    ScRangeList aVarRanges;
    if (!ParseWithNames(aVarRanges, m_pSettings->GetParameter(sc::SP_VAR_CELLS)))
    {
        m_nStatus = sheet::SolverStatus::PARSE_ERROR;
        m_sErrorMessage = ScResId(STR_SOLVER_VARCELL_FAIL);
        if (!m_bSuppressDialog)
            ScSolverSettings::ShowErrorMessage(m_sErrorMessage);
        return;
    }

    // Resolve ranges into single cells
    uno::Sequence<table::CellAddress> aVariableCells;
    sal_Int32 nVarPos(0);
    for (size_t nRangePos = 0, nRange = aVarRanges.size(); nRangePos < nRange; ++nRangePos)
    {
        ScRange aRange(aVarRanges[nRangePos]);
        aRange.PutInOrder();
        SCTAB nTab = aRange.aStart.Tab();

        sal_Int32 nAdd = (aRange.aEnd.Col() - aRange.aStart.Col() + 1)
                         * (aRange.aEnd.Row() - aRange.aStart.Row() + 1);
        aVariableCells.realloc(nVarPos + nAdd);
        auto pVariables = aVariableCells.getArray();

        for (SCROW nRow = aRange.aStart.Row(); nRow <= aRange.aEnd.Row(); ++nRow)
            for (SCCOL nCol = aRange.aStart.Col(); nCol <= aRange.aEnd.Col(); ++nCol)
                pVariables[nVarPos++] = table::CellAddress(nTab, nCol, nRow);
    }

    // Prepare model constraints
    uno::Sequence<sheet::SolverConstraint> aConstraints;
    sal_Int32 nConstrPos = 0;
    for (const auto& rConstr : m_pSettings->GetConstraints())
    {
        if (!rConstr.aLeftStr.isEmpty())
        {
            sheet::SolverConstraint aConstraint;
            aConstraint.Operator = getUnoOperatorFromSc(rConstr.nOperator);

            // The left side of the constraint must be a valid range or a single cell
            ScRange aLeftRange;
            if (!ParseRef(aLeftRange, rConstr.aLeftStr, true))
            {
                m_nStatus = sheet::SolverStatus::PARSE_ERROR;
                m_sErrorMessage = ScResId(STR_INVALIDCONDITION);
                if (!m_bSuppressDialog)
                    ScSolverSettings::ShowErrorMessage(m_sErrorMessage);
                return;
            }

            // The right side can be either a cell range, a single cell or a numeric value
            bool bIsRange(false);
            ScRange aRightRange;
            if (ParseRef(aRightRange, rConstr.aRightStr, true))
            {
                if (aRightRange.aStart == aRightRange.aEnd)
                    aConstraint.Right
                        <<= table::CellAddress(aRightRange.aStart.Tab(), aRightRange.aStart.Col(),
                                               aRightRange.aStart.Row());

                else if (aRightRange.aEnd.Col() - aRightRange.aStart.Col()
                             == aLeftRange.aEnd.Col() - aLeftRange.aStart.Col()
                         && aRightRange.aEnd.Row() - aRightRange.aStart.Row()
                                == aLeftRange.aEnd.Row() - aLeftRange.aStart.Row())
                    // If the right side of the constraint is a range, it must have the
                    // same shape as the left side
                    bIsRange = true;
                else
                {
                    m_nStatus = sheet::SolverStatus::PARSE_ERROR;
                    m_sErrorMessage = ScResId(STR_INVALIDCONDITION);
                    if (!m_bSuppressDialog)
                        ScSolverSettings::ShowErrorMessage(m_sErrorMessage);

                    return;
                }
            }
            else
            {
                // Test if the right side is a numeric value
                sal_uInt32 nFormat = 0;
                double fValue(0);
                if (m_rDoc.GetFormatTable()->IsNumberFormat(rConstr.aRightStr, nFormat, fValue))
                    aConstraint.Right <<= fValue;
                else if (aConstraint.Operator != sheet::SolverConstraintOperator_INTEGER
                         && aConstraint.Operator != sheet::SolverConstraintOperator_BINARY)
                {
                    m_nStatus = sheet::SolverStatus::PARSE_ERROR;
                    m_sErrorMessage = ScResId(STR_INVALIDCONDITION);
                    if (!m_bSuppressDialog)
                        ScSolverSettings::ShowErrorMessage(ScResId(STR_INVALIDCONDITION));
                    return;
                }
            }

            // Resolve constraint into single cells
            sal_Int32 nAdd = (aLeftRange.aEnd.Col() - aLeftRange.aStart.Col() + 1)
                             * (aLeftRange.aEnd.Row() - aLeftRange.aStart.Row() + 1);
            aConstraints.realloc(nConstrPos + nAdd);
            auto pConstraints = aConstraints.getArray();

            for (SCROW nRow = aLeftRange.aStart.Row(); nRow <= aLeftRange.aEnd.Row(); ++nRow)
                for (SCCOL nCol = aLeftRange.aStart.Col(); nCol <= aLeftRange.aEnd.Col(); ++nCol)
                {
                    aConstraint.Left = table::CellAddress(aLeftRange.aStart.Tab(), nCol, nRow);
                    if (bIsRange)
                        aConstraint.Right <<= table::CellAddress(
                            aRightRange.aStart.Tab(),
                            aRightRange.aStart.Col() + (nCol - aLeftRange.aStart.Col()),
                            aRightRange.aStart.Row() + (nRow - aLeftRange.aStart.Row()));
                    pConstraints[nConstrPos++] = aConstraint;
                }
        }
    }

    // Type of the objective function
    // If the objective is of type VALUE then a minimization model is used
    sc::ObjectiveType aObjType(m_pSettings->GetObjectiveType());
    bool bMaximize = aObjType == sc::ObjectiveType::OT_MAXIMIZE;

    if (aObjType == sc::ObjectiveType::OT_VALUE)
    {
        // An additional constraint is added to the model forcing
        // the objective cell to be equal to a given value
        sheet::SolverConstraint aConstraint;
        aConstraint.Left = aObjCell;
        aConstraint.Operator = sheet::SolverConstraintOperator_EQUAL;

        OUString aValStr = m_pSettings->GetParameter(sc::SP_OBJ_VAL);
        ScRange aRightRange;

        if (ParseRef(aRightRange, aValStr, false))
            aConstraint.Right <<= table::CellAddress(
                aRightRange.aStart.Tab(), aRightRange.aStart.Col(), aRightRange.aStart.Row());
        else
        {
            // Test if the right side is a numeric value
            sal_uInt32 nFormat = 0;
            double fValue(0);
            if (m_rDoc.GetFormatTable()->IsNumberFormat(aValStr, nFormat, fValue))
                aConstraint.Right <<= fValue;
            else
            {
                m_nStatus = sheet::SolverStatus::PARSE_ERROR;
                m_sErrorMessage = ScResId(STR_SOLVER_TARGETVALUE_FAIL);
                if (!m_bSuppressDialog)
                    ScSolverSettings::ShowErrorMessage(m_sErrorMessage);
                return;
            }
        }

        aConstraints.realloc(nConstrPos + 1);
        aConstraints.getArray()[nConstrPos++] = std::move(aConstraint);
    }

    // Create a copy of document values in case the user chooses to restore them
    sal_Int32 nVarCount = aVariableCells.getLength();
    uno::Sequence<double> aOldValues(nVarCount);
    std::transform(std::cbegin(aVariableCells), std::cend(aVariableCells), aOldValues.getArray(),
                   [this](const table::CellAddress& rVariable) -> double {
                       ScAddress aCellPos;
                       ScUnoConversion::FillScAddress(aCellPos, rVariable);
                       return m_rDoc.GetValue(aCellPos);
                   });

    // Create and initialize solver
    uno::Reference<sheet::XSolver> xSolver = ScSolverUtil::GetSolver(getEngine());
    OSL_ENSURE(xSolver.is(), "Unable to get solver component");
    if (!xSolver.is())
    {
        if (!m_bSuppressDialog)
            ScSolverSettings::ShowErrorMessage(ScResId(STR_INVALIDINPUT));
        m_nStatus = sheet::SolverStatus::ENGINE_ERROR;
        m_sErrorMessage = ScResId(STR_SOLVER_LOAD_FAIL);
        return;
    }

    rtl::Reference<ScModelObj> xDocument(m_pDocShell->GetModel());
    xSolver->setDocument(xDocument);
    xSolver->setObjective(aObjCell);
    xSolver->setVariables(aVariableCells);
    xSolver->setConstraints(aConstraints);
    xSolver->setMaximize(bMaximize);

    // Set engine options
    uno::Reference<beans::XPropertySet> xOptProp(xSolver, uno::UNO_QUERY);
    if (xOptProp.is())
    {
        for (const beans::PropertyValue& rValue : getEngineOptions())
        {
            try
            {
                xOptProp->setPropertyValue(rValue.Name, rValue.Value);
            }
            catch (uno::Exception&)
            {
                OSL_FAIL("Unable to set solver option property");
            }
        }
    }

    xSolver->solve();
    bool bSuccess = xSolver->getSuccess();

    // Close progress dialog
    if (!m_bSuppressDialog && xProgress)
        xProgress->response(RET_CLOSE);

    if (bSuccess)
    {
        m_nStatus = sheet::SolverStatus::SOLUTION_FOUND;
        // Write solution to the document
        uno::Sequence<double> aSolution = xSolver->getSolution();
        if (aSolution.getLength() == nVarCount)
        {
            m_pDocShell->LockPaint();
            ScDocFunc& rFunc = m_pDocShell->GetDocFunc();
            for (nVarPos = 0; nVarPos < nVarCount; ++nVarPos)
            {
                ScAddress aCellPos;
                ScUnoConversion::FillScAddress(aCellPos, aVariableCells[nVarPos]);
                rFunc.SetValueCell(aCellPos, aSolution[nVarPos], false);
            }
            m_pDocShell->UnlockPaint();
        }
        else
        {
            OSL_FAIL("Wrong number of variables in the solver solution");
        }

        // Show success dialog
        if (!m_bSuppressDialog)
        {
            // Get formatted result from document to show in the Success dialog
            OUString aResultStr = m_rDoc.GetString(static_cast<SCCOL>(aObjCell.Column),
                                                   static_cast<SCROW>(aObjCell.Row),
                                                   static_cast<SCTAB>(aObjCell.Sheet));

            ScSolverSuccessDialog xSuccessDialog(Application::GetDefDialogParent(), aResultStr);
            bool bRestore(true);
            if (xSuccessDialog.run() == RET_OK)
                // Keep results in the document
                bRestore = false;

            if (bRestore)
            {
                // Restore values to the document
                m_pDocShell->LockPaint();
                ScDocFunc& rFunc = m_pDocShell->GetDocFunc();
                for (nVarPos = 0; nVarPos < nVarCount; ++nVarPos)
                {
                    ScAddress aCellPos;
                    ScUnoConversion::FillScAddress(aCellPos, aVariableCells[nVarPos]);
                    rFunc.SetValueCell(aCellPos, aOldValues[nVarPos], false);
                }
                m_pDocShell->UnlockPaint();
            }
        }
    }
    else
    {
        // The solver failed to find a solution
        m_nStatus = sheet::SolverStatus::SOLUTION_NOT_FOUND;
        uno::Reference<sheet::XSolverDescription> xDesc(xSolver, uno::UNO_QUERY);
        // Get error message reported by the solver
        if (xDesc.is())
            m_sErrorMessage = xDesc->getStatusDescription();
        if (!m_bSuppressDialog)
        {
            ScSolverNoSolutionDialog aDialog(Application::GetDefDialogParent(), m_sErrorMessage);
            aDialog.run();
        }
    }
}

void SAL_CALL ScSolverSettings::saveToFile() { m_pSettings->SaveSolverSettings(); }

// XServiceInfo
OUString SAL_CALL ScSolverSettings::getImplementationName() { return u"ScSolverSettings"_ustr; }

sal_Bool SAL_CALL ScSolverSettings::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL ScSolverSettings::getSupportedServiceNames()
{
    return { SC_SOLVERSETTINGS_SERVICE };
}
