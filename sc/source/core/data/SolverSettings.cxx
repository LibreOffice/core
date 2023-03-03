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
#include <table.hxx>
#include <docsh.hxx>
#include <solverutil.hxx>
#include <unotools/charclass.hxx>
#include <SolverSettings.hxx>

namespace sc
{
SolverSettings::SolverSettings(ScTable& rTable)
    : m_rTable(rTable)
    , m_rDoc(m_rTable.GetDoc())
    , m_pDocShell(dynamic_cast<ScDocShell*>(m_rDoc.GetDocumentShell()))
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

    WriteParamValue(SP_INTEGER, m_sInteger);
    WriteParamValue(SP_NON_NEGATIVE, m_sNonNegative);
    WriteParamValue(SP_EPSILON_LEVEL, m_sEpsilonLevel);
    WriteParamValue(SP_LIMIT_BBDEPTH, m_sLimitBBDepth);
    WriteParamValue(SP_TIMEOUT, m_sTimeout);
    WriteParamValue(SP_ALGORITHM, m_sAlgorithm);

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
    OUString sRange = m_mNamedRanges.find(eParam)->second;
    ScRangeData* pRangeData
        = m_pRangeName->findByUpperName(ScGlobal::getCharClass().uppercase(sRange));
    if (pRangeData)
    {
        rValue = pRangeData->GetSymbol();
        if (bRemoveQuotes)
            ScGlobal::EraseQuotes(rValue, '"');
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

    OUString sRange = m_mNamedRanges.find(eParam)->second;
    ScRangeData* pNewEntry = new ScRangeData(m_rDoc, sRange, sValue);
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
                sal_Int32 nValue;
                aProp.Value >>= nValue;
                SetParameter(eParamId, OUString::number(nValue));
            }
            if (sParamType == "bool")
            {
                bool bValue;
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

} // namespace sc
