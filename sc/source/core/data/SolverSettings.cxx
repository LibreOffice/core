/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <global.hxx>
#include <compiler.hxx>
#include <table.hxx>
#include <docsh.hxx>
#include <rtl/math.hxx>
#include <o3tl/string_view.hxx>
#include <solverutil.hxx>
#include <unotools/charclass.hxx>
#include <SolverSettings.hxx>

namespace sc
{
SolverSettings::SolverSettings(ScTable& rTable)
    : m_rTable(rTable)
    , m_rDoc(m_rTable.GetDoc())
    , m_pDocShell(m_rDoc.GetDocumentShell())
{
    // Get the named range manager for this tab
    std::map<OUString, ScRangeName*> rRangeMap;
    m_rDoc.GetRangeNameMap(rRangeMap);
    m_pRangeName = rRangeMap.find(m_rTable.GetName())->second;

    Initialize();
}

void SolverSettings::Initialize()
{
    // Assign default values for the solver parameters
    ResetToDefaults();

    // Read the parameter values in the sheet
    ReadParamValue(SP_OBJ_CELL, m_sObjCell);
    ReadParamValue(SP_OBJ_VAL, m_sObjVal);
    ReadParamValue(SP_VAR_CELLS, m_sVariableCells);

    // Read the objective type
    OUString sObjType;
    if (ReadParamValue(SP_OBJ_TYPE, sObjType))
    {
        switch (sObjType.toInt32())
        {
            case 1:
                m_eObjType = ObjectiveType::OT_MAXIMIZE;
                break;
            case 2:
                m_eObjType = ObjectiveType::OT_MINIMIZE;
                break;
            case 3:
                m_eObjType = ObjectiveType::OT_VALUE;
                break;
            default:
                m_eObjType = ObjectiveType::OT_MAXIMIZE;
        }
    }

    // Read all constraints in the tab
    ReadConstraints();

    // Read the solver engine being used
    ReadEngine();

    // Read engine options
    ReadParamValue(SP_INTEGER, m_sInteger);
    ReadParamValue(SP_NON_NEGATIVE, m_sNonNegative);
    ReadParamValue(SP_EPSILON_LEVEL, m_sEpsilonLevel);
    ReadParamValue(SP_LIMIT_BBDEPTH, m_sLimitBBDepth);
    ReadParamValue(SP_TIMEOUT, m_sTimeout);
    ReadParamValue(SP_ALGORITHM, m_sAlgorithm);
    // Engine options common for DEPS and SCO
    ReadParamValue(SP_SWARM_SIZE, m_sSwarmSize);
    ReadParamValue(SP_LEARNING_CYCLES, m_sLearningCycles);
    ReadParamValue(SP_GUESS_VARIABLE_RANGE, m_sGuessVariableRange);
    ReadDoubleParamValue(SP_VARIABLE_RANGE_THRESHOLD, m_sVariableRangeThreshold);
    ReadParamValue(SP_ACR_COMPARATOR, m_sUseACRComparator);
    ReadParamValue(SP_RND_STARTING_POINT, m_sUseRandomStartingPoint);
    ReadParamValue(SP_STRONGER_PRNG, m_sUseStrongerPRNG);
    ReadParamValue(SP_STAGNATION_LIMIT, m_sStagnationLimit);
    ReadDoubleParamValue(SP_STAGNATION_TOLERANCE, m_sTolerance);
    ReadParamValue(SP_ENHANCED_STATUS, m_sEnhancedSolverStatus);
    // DEPS Options
    ReadDoubleParamValue(SP_AGENT_SWITCH_RATE, m_sAgentSwitchRate);
    ReadDoubleParamValue(SP_SCALING_MIN, m_sScalingFactorMin);
    ReadDoubleParamValue(SP_SCALING_MAX, m_sScalingFactorMax);
    ReadDoubleParamValue(SP_CROSSOVER_PROB, m_sCrossoverProbability);
    ReadDoubleParamValue(SP_COGNITIVE_CONST, m_sCognitiveConstant);
    ReadDoubleParamValue(SP_SOCIAL_CONST, m_sSocialConstant);
    ReadDoubleParamValue(SP_CONSTRICTION_COEFF, m_sConstrictionCoeff);
    ReadDoubleParamValue(SP_MUTATION_PROB, m_sMutationProbability);
    // SCO Options
    ReadParamValue(SP_LIBRARY_SIZE, m_sLibrarySize);
}

// Returns the current value of the parameter in the object as a string
OUString SolverSettings::GetParameter(SolverParameter eParam)
{
    switch (eParam)
    {
        case SP_OBJ_CELL:
            return m_sObjCell;
            break;
        case SP_OBJ_TYPE:
            return OUString::number(m_eObjType);
            break;
        case SP_OBJ_VAL:
            return m_sObjVal;
            break;
        case SP_VAR_CELLS:
            return m_sVariableCells;
            break;
        case SP_CONSTR_COUNT:
            return OUString::number(m_aConstraints.size());
            break;
        case SP_LO_ENGINE:
            return m_sLOEngineName;
            break;
        case SP_MS_ENGINE:
            return m_sMSEngineId;
            break;
        case SP_INTEGER:
            return m_sInteger;
            break;
        case SP_NON_NEGATIVE:
            return m_sNonNegative;
            break;
        case SP_EPSILON_LEVEL:
            return m_sEpsilonLevel;
            break;
        case SP_LIMIT_BBDEPTH:
            return m_sLimitBBDepth;
            break;
        case SP_TIMEOUT:
            return m_sTimeout;
            break;
        case SP_ALGORITHM:
            return m_sAlgorithm;
            break;
        case SP_SWARM_SIZE:
            return m_sSwarmSize;
            break;
        case SP_LEARNING_CYCLES:
            return m_sLearningCycles;
            break;
        case SP_GUESS_VARIABLE_RANGE:
            return m_sGuessVariableRange;
            break;
        case SP_VARIABLE_RANGE_THRESHOLD:
            return m_sVariableRangeThreshold;
            break;
        case SP_ACR_COMPARATOR:
            return m_sUseACRComparator;
            break;
        case SP_RND_STARTING_POINT:
            return m_sUseRandomStartingPoint;
            break;
        case SP_STRONGER_PRNG:
            return m_sUseStrongerPRNG;
            break;
        case SP_STAGNATION_LIMIT:
            return m_sStagnationLimit;
            break;
        case SP_STAGNATION_TOLERANCE:
            return m_sTolerance;
            break;
        case SP_ENHANCED_STATUS:
            return m_sEnhancedSolverStatus;
            break;
        case SP_AGENT_SWITCH_RATE:
            return m_sAgentSwitchRate;
            break;
        case SP_SCALING_MIN:
            return m_sScalingFactorMin;
            break;
        case SP_SCALING_MAX:
            return m_sScalingFactorMax;
            break;
        case SP_CROSSOVER_PROB:
            return m_sCrossoverProbability;
            break;
        case SP_COGNITIVE_CONST:
            return m_sCognitiveConstant;
            break;
        case SP_SOCIAL_CONST:
            return m_sSocialConstant;
            break;
        case SP_CONSTRICTION_COEFF:
            return m_sConstrictionCoeff;
            break;
        case SP_MUTATION_PROB:
            return m_sMutationProbability;
            break;
        case SP_LIBRARY_SIZE:
            return m_sLibrarySize;
            break;
        default:
            return "";
    }
}

// Sets the value of a single solver parameter in the object
void SolverSettings::SetParameter(SolverParameter eParam, OUString sValue)
{
    switch (eParam)
    {
        case SP_OBJ_CELL:
            m_sObjCell = sValue;
            break;
        case SP_OBJ_TYPE:
        {
            sal_Int32 nObjType = sValue.toInt32();
            switch (nObjType)
            {
                case OT_MAXIMIZE:
                    m_eObjType = ObjectiveType::OT_MAXIMIZE;
                    break;
                case OT_MINIMIZE:
                    m_eObjType = ObjectiveType::OT_MINIMIZE;
                    break;
                case OT_VALUE:
                    m_eObjType = ObjectiveType::OT_VALUE;
                    break;
                default:
                    m_eObjType = ObjectiveType::OT_MAXIMIZE;
                    break;
            }
            break;
        }
        case SP_OBJ_VAL:
            m_sObjVal = sValue;
            break;
        case SP_VAR_CELLS:
            m_sVariableCells = sValue;
            break;
        case SP_LO_ENGINE:
            m_sLOEngineName = sValue;
            break;
        case SP_INTEGER:
        {
            if (sValue == "0" || sValue == "1")
                m_sInteger = sValue;
        }
        break;
        case SP_NON_NEGATIVE:
        {
            if (sValue == "1" || sValue == "2")
                m_sNonNegative = sValue;
        }
        break;
        case SP_EPSILON_LEVEL:
            m_sEpsilonLevel = sValue;
            break;
        case SP_LIMIT_BBDEPTH:
            m_sLimitBBDepth = sValue;
            break;
        case SP_TIMEOUT:
            m_sTimeout = sValue;
            break;
        case SP_ALGORITHM:
        {
            if (sValue == "1" || sValue == "2" || sValue == "3")
                m_sAlgorithm = sValue;
        }
        break;
        case SP_SWARM_SIZE:
            m_sSwarmSize = sValue;
            break;
        case SP_LEARNING_CYCLES:
            m_sLearningCycles = sValue;
            break;
        case SP_GUESS_VARIABLE_RANGE:
            m_sGuessVariableRange = sValue;
            break;
        case SP_VARIABLE_RANGE_THRESHOLD:
            m_sVariableRangeThreshold = sValue;
            break;
        case SP_ACR_COMPARATOR:
        {
            if (sValue == "0" || sValue == "1")
                m_sUseACRComparator = sValue;
        }
        break;
        case SP_RND_STARTING_POINT:
        {
            if (sValue == "0" || sValue == "1")
                m_sUseRandomStartingPoint = sValue;
        }
        break;
        case SP_STRONGER_PRNG:
        {
            if (sValue == "0" || sValue == "1")
                m_sUseStrongerPRNG = sValue;
        }
        break;
        case SP_STAGNATION_LIMIT:
            m_sStagnationLimit = sValue;
            break;
        case SP_STAGNATION_TOLERANCE:
            m_sTolerance = sValue;
            break;
        case SP_ENHANCED_STATUS:
        {
            if (sValue == "0" || sValue == "1")
                m_sEnhancedSolverStatus = sValue;
        }
        break;
        case SP_AGENT_SWITCH_RATE:
            m_sAgentSwitchRate = sValue;
            break;
        case SP_SCALING_MIN:
            m_sScalingFactorMin = sValue;
            break;
        case SP_SCALING_MAX:
            m_sScalingFactorMax = sValue;
            break;
        case SP_CROSSOVER_PROB:
            m_sCrossoverProbability = sValue;
            break;
        case SP_COGNITIVE_CONST:
            m_sCognitiveConstant = sValue;
            break;
        case SP_SOCIAL_CONST:
            m_sSocialConstant = sValue;
            break;
        case SP_CONSTRICTION_COEFF:
            m_sConstrictionCoeff = sValue;
            break;
        case SP_MUTATION_PROB:
            m_sMutationProbability = sValue;
            break;
        case SP_LIBRARY_SIZE:
            m_sLibrarySize = sValue;
            break;
        default:
            break;
    }
}

void SolverSettings::SetObjectiveType(ObjectiveType eType) { m_eObjType = eType; }

// Loads all constraints in the tab
void SolverSettings::ReadConstraints()
{
    // Condition indices start at 1 for MS compatibility
    // The number of "lhs", "rel" and "rhs" entries will always be the same
    tools::Long nConstraint = 1;
    m_aConstraints.clear();
    OUString sValue;

    while (ReadConstraintPart(CP_LEFT_HAND_SIDE, nConstraint, sValue))
    {
        // Left hand side
        ModelConstraint aNewCondition;
        aNewCondition.aLeftStr = sValue;

        // Right hand side
        if (ReadConstraintPart(CP_RIGHT_HAND_SIDE, nConstraint, sValue))
            aNewCondition.aRightStr = sValue;

        // Relation (operator)
        if (ReadConstraintPart(CP_OPERATOR, nConstraint, sValue))
            aNewCondition.nOperator = static_cast<sc::ConstraintOperator>(sValue.toInt32());

        m_aConstraints.push_back(aNewCondition);
        nConstraint++;
    }
}

// Writes all constraints to the file
void SolverSettings::WriteConstraints()
{
    // Condition indices start at 1 for MS compatibility
    tools::Long nConstraint = 1;

    for (auto& aConstraint : m_aConstraints)
    {
        // Left hand side
        WriteConstraintPart(CP_LEFT_HAND_SIDE, nConstraint, aConstraint.aLeftStr);
        // Relation (operator)
        WriteConstraintPart(CP_OPERATOR, nConstraint, OUString::number(aConstraint.nOperator));
        // Right hand side
        WriteConstraintPart(CP_RIGHT_HAND_SIDE, nConstraint, aConstraint.aRightStr);
        nConstraint++;
    }
}

// Write a single constraint part to the file
void SolverSettings::WriteConstraintPart(ConstraintPart ePart, tools::Long nIndex, OUString sValue)
{
    // Empty named ranges cannot be written to the file (this corrupts MS files)
    if (sValue.isEmpty())
        return;

    OUString sRange = m_aConstraintParts[ePart] + OUString::number(nIndex);
    ScRangeData* pNewEntry = new ScRangeData(m_rDoc, sRange, sValue);
    m_pRangeName->insert(pNewEntry);
}

// Reads a single constraint part from its associated named range; returns false if the named
// range does not exist in the file
bool SolverSettings::ReadConstraintPart(ConstraintPart ePart, tools::Long nIndex, OUString& rValue)
{
    OUString sRange = m_aConstraintParts[ePart] + OUString::number(nIndex);
    ScRangeData* pRangeData
        = m_pRangeName->findByUpperName(ScGlobal::getCharClass().uppercase(sRange));
    if (pRangeData)
    {
        rValue = pRangeData->GetSymbol();
        // tdf#156814 Remove sheet name if it is a range that refers to the same sheet
        ScRange aRange;
        ScRefFlags nFlags = aRange.ParseAny(rValue, m_rDoc);
        bool bIsValidRange = (nFlags & ScRefFlags::VALID) == ScRefFlags::VALID;
        if (bIsValidRange && m_rTable.GetTab() == aRange.aStart.Tab())
            rValue = aRange.Format(m_rDoc, ScRefFlags::RANGE_ABS);
        return true;
    }
    return false;
}

/* Reads the engine name parameter as informed in the file in the format used in LO.
 * If only a MS engine is informed, then it is converted to a LO-equivalent engine
 */
void SolverSettings::ReadEngine()
{
    if (!ReadParamValue(SP_LO_ENGINE, m_sLOEngineName, true))
    {
        // If no engine is defined, use CoinMP solver as default
        m_sLOEngineName = "com.sun.star.comp.Calc.CoinMPSolver";
    }

    if (SolverNamesToExcelEngines.count(m_sLOEngineName))
    {
        // Find equivalent MS engine code
        m_sMSEngineId = SolverNamesToExcelEngines.find(m_sLOEngineName)->second;
    }
}

// Write solver LO and MS-equivalent engine names
void SolverSettings::WriteEngine()
{
    WriteParamValue(SP_LO_ENGINE, m_sLOEngineName, true);
    // Find equivalent MS engine code
    if (SolverNamesToExcelEngines.count(m_sLOEngineName))
    {
        m_sMSEngineId = SolverNamesToExcelEngines.find(m_sLOEngineName)->second;
        WriteParamValue(SP_MS_ENGINE, m_sMSEngineId);
    }
}

// Assigns a new constraints vector
void SolverSettings::SetConstraints(std::vector<ModelConstraint> aConstraints)
{
    m_aConstraints = std::move(aConstraints);
}

// Saves all solver settings into the file
void SolverSettings::SaveSolverSettings()
{
    // Before saving, remove all existing named ranges related to the solver
    DeleteAllNamedRanges();

    WriteParamValue(SP_OBJ_CELL, m_sObjCell);
    WriteParamValue(SP_OBJ_TYPE, OUString::number(m_eObjType));
    WriteParamValue(SP_OBJ_VAL, m_sObjVal);
    WriteParamValue(SP_VAR_CELLS, m_sVariableCells);

    WriteConstraints();
    WriteEngine();

    sal_Int32 nConstrCount = m_aConstraints.size();
    WriteParamValue(SP_CONSTR_COUNT, OUString::number(nConstrCount));

    // Solver engine options
    WriteParamValue(SP_INTEGER, m_sInteger);
    WriteParamValue(SP_NON_NEGATIVE, m_sNonNegative);
    WriteParamValue(SP_EPSILON_LEVEL, m_sEpsilonLevel);
    WriteParamValue(SP_LIMIT_BBDEPTH, m_sLimitBBDepth);
    WriteParamValue(SP_TIMEOUT, m_sTimeout);
    WriteParamValue(SP_ALGORITHM, m_sAlgorithm);
    // Engine options common for DEPS and SCO
    WriteParamValue(SP_SWARM_SIZE, m_sSwarmSize);
    WriteParamValue(SP_LEARNING_CYCLES, m_sLearningCycles);
    WriteParamValue(SP_GUESS_VARIABLE_RANGE, m_sGuessVariableRange);
    WriteDoubleParamValue(SP_VARIABLE_RANGE_THRESHOLD, m_sVariableRangeThreshold);
    WriteParamValue(SP_ACR_COMPARATOR, m_sUseACRComparator);
    WriteParamValue(SP_RND_STARTING_POINT, m_sUseRandomStartingPoint);
    WriteParamValue(SP_STRONGER_PRNG, m_sUseStrongerPRNG);
    WriteParamValue(SP_STAGNATION_LIMIT, m_sStagnationLimit);
    WriteDoubleParamValue(SP_STAGNATION_TOLERANCE, m_sTolerance);
    WriteParamValue(SP_ENHANCED_STATUS, m_sEnhancedSolverStatus);
    // DEPS Options
    WriteDoubleParamValue(SP_AGENT_SWITCH_RATE, m_sAgentSwitchRate);
    WriteDoubleParamValue(SP_SCALING_MIN, m_sScalingFactorMin);
    WriteDoubleParamValue(SP_SCALING_MAX, m_sScalingFactorMax);
    WriteDoubleParamValue(SP_CROSSOVER_PROB, m_sCrossoverProbability);
    WriteDoubleParamValue(SP_COGNITIVE_CONST, m_sCognitiveConstant);
    WriteDoubleParamValue(SP_SOCIAL_CONST, m_sSocialConstant);
    WriteDoubleParamValue(SP_CONSTRICTION_COEFF, m_sConstrictionCoeff);
    WriteDoubleParamValue(SP_MUTATION_PROB, m_sMutationProbability);
    // SCO Options
    WriteParamValue(SP_LIBRARY_SIZE, m_sLibrarySize);

    if (m_pDocShell)
        m_pDocShell->SetDocumentModified();
}

/* Reads the current value of the parameter in the named range into rValue
 * If the value does not exist, the rValue is left unchanged
 * This is private because it is only used during initialization
 * Returns true if the value exits; returns false otherwise
 */
bool SolverSettings::ReadParamValue(SolverParameter eParam, OUString& rValue, bool bRemoveQuotes)
{
    const auto iter = m_mNamedRanges.find(eParam);
    assert(iter != m_mNamedRanges.end());
    OUString sRange = iter->second;
    ScRangeData* pRangeData
        = m_pRangeName->findByUpperName(ScGlobal::getCharClass().uppercase(sRange));
    if (pRangeData)
    {
        rValue = pRangeData->GetSymbol();
        if (bRemoveQuotes)
            ScGlobal::EraseQuotes(rValue, '"');

        // tdf#156814 Remove sheet name from the objective cell and value if they refer to the same sheet
        if (eParam == SP_OBJ_CELL || eParam == SP_OBJ_VAL)
        {
            ScRange aRange;
            ScRefFlags nFlags = aRange.ParseAny(rValue, m_rDoc);
            bool bIsValidRange = ((nFlags & ScRefFlags::VALID) == ScRefFlags::VALID);

            if (bIsValidRange && m_rTable.GetTab() == aRange.aStart.Tab())
                rValue = aRange.Format(m_rDoc, ScRefFlags::RANGE_ABS);
        }
        else if (eParam == SP_VAR_CELLS)
        {
            // Variable cells may contain multiple ranges separated by ';'
            sal_Int32 nIdx = 0;
            OUString sNewValue;
            bool bFirst = true;
            // Delimiter character to separate ranges
            sal_Unicode cDelimiter = ScCompiler::GetNativeSymbolChar(OpCode::ocSep);

            do
            {
                OUString aRangeStr(o3tl::getToken(rValue, 0, cDelimiter, nIdx));
                ScRange aRange;
                ScRefFlags nFlags = aRange.ParseAny(aRangeStr, m_rDoc);
                bool bIsValidRange = (nFlags & ScRefFlags::VALID) == ScRefFlags::VALID;

                if (bIsValidRange && m_rTable.GetTab() == aRange.aStart.Tab())
                    aRangeStr = aRange.Format(m_rDoc, ScRefFlags::RANGE_ABS);

                if (bFirst)
                {
                    sNewValue = aRangeStr;
                    bFirst = false;
                }
                else
                {
                    sNewValue += OUStringChar(cDelimiter) + aRangeStr;
                }
            } while (nIdx > 0);

            rValue = sNewValue;
        }
        return true;
    }
    return false;
}

// Reads a parameter value of type 'double' from the named range and into rValue
bool SolverSettings::ReadDoubleParamValue(SolverParameter eParam, OUString& rValue)
{
    const auto iter = m_mNamedRanges.find(eParam);
    assert(iter != m_mNamedRanges.end());
    OUString sRange = iter->second;
    ScRangeData* pRangeData
        = m_pRangeName->findByUpperName(ScGlobal::getCharClass().uppercase(sRange));
    if (pRangeData)
    {
        OUString sLocalizedValue = pRangeData->GetSymbol();
        double fValue = rtl::math::stringToDouble(sLocalizedValue,
                                                  ScGlobal::getLocaleData().getNumDecimalSep()[0],
                                                  ScGlobal::getLocaleData().getNumThousandSep()[0]);
        rValue = OUString::number(fValue);
        return true;
    }
    return false;
}

/* Writes a parameter value to the file as a named range.
 * Argument bQuoted indicates whether the value should be enclosed with quotes or not (used
 * for string expressions that must be enclosed with quotes)
 */
void SolverSettings::WriteParamValue(SolverParameter eParam, OUString sValue, bool bQuoted)
{
    // Empty parameters cannot be written to the file (this corrupts MS files)
    // There's no problem if the parameter is missing both for LO and MS
    if (sValue.isEmpty())
        return;

    if (bQuoted)
        ScGlobal::AddQuotes(sValue, '"');

    const auto iter = m_mNamedRanges.find(eParam);
    assert(iter != m_mNamedRanges.end());
    OUString sRange = iter->second;
    ScRangeData* pNewEntry = new ScRangeData(m_rDoc, sRange, sValue);
    m_pRangeName->insert(pNewEntry);
}

// Writes a parameter value of type 'double' to the file as a named range
// The argument 'sValue' uses dot as decimal separator and needs to be localized before
// being written to the file
void SolverSettings::WriteDoubleParamValue(SolverParameter eParam, std::u16string_view sValue)
{
    const auto iter = m_mNamedRanges.find(eParam);
    assert(iter != m_mNamedRanges.end());
    OUString sRange = iter->second;
    double fValue = rtl::math::stringToDouble(sValue, '.', ',');
    OUString sLocalizedValue = rtl::math::doubleToUString(
        fValue, rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
        ScGlobal::getLocaleData().getNumDecimalSep()[0], true);
    ScRangeData* pNewEntry = new ScRangeData(m_rDoc, sRange, sLocalizedValue);
    m_pRangeName->insert(pNewEntry);
}

void SolverSettings::GetEngineOptions(css::uno::Sequence<css::beans::PropertyValue>& aOptions)
{
    sal_Int32 nOptionsSize = aOptions.getLength();
    auto pParamValues = aOptions.getArray();

    for (auto i = 0; i < nOptionsSize; i++)
    {
        css::beans::PropertyValue aProp = aOptions[i];
        OUString sLOParamName = aProp.Name;
        // Only try to get the parameter value if it is an expected parameter name
        if (SolverParamNames.count(sLOParamName))
        {
            TParamInfo aParamInfo;
            aParamInfo = SolverParamNames.find(sLOParamName)->second;
            SolverParameter eParamId = std::get<SolverParameter>(aParamInfo[0]);
            OUString sParamType = std::get<OUString>(aParamInfo[2]);
            OUString sParamValue = GetParameter(eParamId);
            if (sParamType == "int")
            {
                css::uno::Any nValue(sParamValue.toInt32());
                pParamValues[i] = css::beans::PropertyValue(sLOParamName, -1, nValue,
                                                            css::beans::PropertyState_DIRECT_VALUE);
            }
            if (sParamType == "double")
            {
                css::uno::Any fValue(sParamValue.toDouble());
                pParamValues[i] = css::beans::PropertyValue(sLOParamName, -1, fValue,
                                                            css::beans::PropertyState_DIRECT_VALUE);
            }
            if (sParamType == "bool")
            {
                // The parameter NonNegative is a special case for MS compatibility
                // It uses "1" for "true" and "2" for "false"
                bool bTmpValue;
                if (sLOParamName == "NonNegative")
                    bTmpValue = sParamValue == "1" ? true : false;
                else
                    bTmpValue = sParamValue.toBoolean();

                css::uno::Any bValue(bTmpValue);
                pParamValues[i] = css::beans::PropertyValue(sLOParamName, -1, bValue,
                                                            css::beans::PropertyState_DIRECT_VALUE);
            }
        }
    }
}

// Updates the object members related to solver engine options using aOptions info
void SolverSettings::SetEngineOptions(css::uno::Sequence<css::beans::PropertyValue>& aOptions)
{
    sal_Int32 nOptionsSize = aOptions.getLength();

    for (auto i = 0; i < nOptionsSize; i++)
    {
        css::beans::PropertyValue aProp = aOptions[i];
        OUString sLOParamName = aProp.Name;
        // Only try to set the parameter value if it is an expected parameter name
        if (SolverParamNames.count(sLOParamName))
        {
            TParamInfo aParamInfo;
            aParamInfo = SolverParamNames.find(sLOParamName)->second;
            SolverParameter eParamId = std::get<SolverParameter>(aParamInfo[0]);
            OUString sParamType = std::get<OUString>(aParamInfo[2]);
            if (sParamType == "int")
            {
                sal_Int32 nValue = 0;
                aProp.Value >>= nValue;
                SetParameter(eParamId, OUString::number(nValue));
            }
            if (sParamType == "double")
            {
                double fValue = 0;
                aProp.Value >>= fValue;
                SetParameter(eParamId, OUString::number(fValue));
            }
            if (sParamType == "bool")
            {
                bool bValue = false;
                aProp.Value >>= bValue;
                if (sLOParamName == "NonNegative")
                {
                    // The parameter NonNegative is a special case for MS compatibility
                    // It uses "1" for "true" and "2" for "false"
                    if (bValue)
                        SetParameter(eParamId, OUString::number(1));
                    else
                        SetParameter(eParamId, OUString::number(2));
                }
                else
                {
                    SetParameter(eParamId, OUString::number(sal_Int32(bValue)));
                }
            }
        }
    }
}

// Deletes all named ranges in the current tab that are related to the solver (i.e. start with "solver_")
void SolverSettings::DeleteAllNamedRanges()
{
    std::vector<ScRangeData*> aItemsToErase;

    // Indices in m_pRangeName start at 1
    for (size_t i = 1; i <= m_pRangeName->size(); ++i)
    {
        ScRangeData* pData = m_pRangeName->findByIndex(i);
        if (pData && pData->GetName().startsWith("solver_"))
            aItemsToErase.push_back(pData);
    }

    for (auto pItem : aItemsToErase)
        m_pRangeName->erase(*pItem);
}

/* Sets all solver parameters to their default values and clear all constraints.
 * This method only resets the object properties, but does not save changes to the
 * document. To save changes, call SaveSolverSettings().
 */
void SolverSettings::ResetToDefaults()
{
    m_sObjCell = "";
    m_eObjType = ObjectiveType::OT_MAXIMIZE;
    m_sObjVal = "";
    m_sVariableCells = "";
    m_sMSEngineId = "1";

    // The default solver engine is the first implementation available
    css::uno::Sequence<OUString> aEngineNames;
    css::uno::Sequence<OUString> aDescriptions;
    ScSolverUtil::GetImplementations(aEngineNames, aDescriptions);
    m_sLOEngineName = aEngineNames[0];

    // Default engine options
    m_aEngineOptions = ScSolverUtil::GetDefaults(m_sLOEngineName);

    // Default solver engine options
    SetEngineOptions(m_aEngineOptions);

    // Clear all constraints
    m_aConstraints.clear();
}

/* Returns true if the current sheet already has a solver model.
   This is determined by checking if the current tab has the SP_OBJ_CELL named range
   which is associated with solver models.
   Note that the named ranges are only created after SaveSolverSettings is called,
   so before it is called, no solver-related named ranges exist.
*/
bool SolverSettings::TabHasSolverModel()
{
    // Check if the named range for the objective value exists in the sheet
    const auto iter = m_mNamedRanges.find(SP_OBJ_CELL);
    OUString sRange = iter->second;
    ScRangeData* pRangeData
        = m_pRangeName->findByUpperName(ScGlobal::getCharClass().uppercase(sRange));
    if (pRangeData)
        return true;
    return false;
}

} // namespace sc
