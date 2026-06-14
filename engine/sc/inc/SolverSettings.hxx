/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <utility>
#include <variant>
#include <rtl/ustring.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

#include "document.hxx"
#include <docsh.hxx>

namespace sc
{
// These values are MS compatible
enum ObjectiveType
{
    OT_MAXIMIZE = 1,
    OT_MINIMIZE = 2,
    OT_VALUE = 3
};

enum SolverParameter
{
    SP_OBJ_CELL, // Objective cell
    SP_OBJ_TYPE, // Objective type (max, min, value)
    SP_OBJ_VAL, // Value (used when objective is of type "value")
    SP_VAR_CELLS, // Variable cells
    SP_CONSTR_COUNT, // Number of constraints (MSO only)
    SP_LO_ENGINE, // Engine name used in LO
    SP_MS_ENGINE, // Engine ID used in MSO
    SP_INTEGER, // Assume all variables are integer (0: no, 1: yes)
    // CoinMP and SwarmSolver
    SP_NON_NEGATIVE, // Assume non negativity (1: yes, 2: no)
    SP_EPSILON_LEVEL, // Epsilon level
    SP_LIMIT_BBDEPTH, // Branch and bound depth
    SP_TIMEOUT, // Time limit to return a solution
    SP_ALGORITHM, // Algorithm used by the SwarmSolver (1, 2 or 3)
    // Engine options common for DEPS and SCO
    SP_SWARM_SIZE, // Size of Swarm
    SP_LEARNING_CYCLES, // Learning Cycles
    SP_GUESS_VARIABLE_RANGE, // Variable Bounds Guessing
    SP_VARIABLE_RANGE_THRESHOLD, // Variable Bounds Threshold (when guessing)
    SP_ACR_COMPARATOR, // Use ACR Comparator (instead of BCH)
    SP_RND_STARTING_POINT, // Use Random starting point
    SP_STRONGER_PRNG, // Use a stronger random generator (slower)
    SP_STAGNATION_LIMIT, // Stagnation Limit
    SP_STAGNATION_TOLERANCE, // Stagnation Tolerance
    SP_ENHANCED_STATUS, // Show enhanced solver status
    // DEPS Options
    SP_AGENT_SWITCH_RATE, // Agent Switch Rate (DE Probability)
    SP_SCALING_MIN, // DE: Min Scaling Factor (0-1.2)
    SP_SCALING_MAX, // DE: Max Scaling Factor (0-1.2)
    SP_CROSSOVER_PROB, // DE: Crossover Probability (0-1)
    SP_COGNITIVE_CONST, // Cognitive Constant
    SP_SOCIAL_CONST, // Social Constant
    SP_CONSTRICTION_COEFF, // PS: Constriction Coefficient
    SP_MUTATION_PROB, // Mutation Probability (0-0.005)
    // SCO Options
    SP_LIBRARY_SIZE, // Size of library
};

// Starts at 1 to maintain MS compatibility
enum ConstraintOperator
{
    CO_LESS_EQUAL = 1,
    CO_EQUAL = 2,
    CO_GREATER_EQUAL = 3,
    CO_INTEGER = 4,
    CO_BINARY = 5
};

// Parts of a constraint
enum ConstraintPart
{
    CP_LEFT_HAND_SIDE,
    CP_OPERATOR,
    CP_RIGHT_HAND_SIDE
};

// Stores the information of a single constraint (condition)
struct ModelConstraint
{
    OUString aLeftStr;
    ConstraintOperator nOperator;
    OUString aRightStr;

    ModelConstraint()
        : nOperator(CO_LESS_EQUAL)
    {
    }
    bool IsDefault() const
    {
        return aLeftStr.isEmpty() && aRightStr.isEmpty() && nOperator == CO_LESS_EQUAL;
    }
};

/* Class SolverSettings
 *
 * This class is used to load/save and manipulate solver settings in a Calc tab.
 *
 * During initialization, (see Initialize() method) all settings stored in the tab are loaded onto
 * the object. Settings that are not defined use default values.
 *
 * Read/Write methods are private and are used internally to load/write solver settings from
 * named ranges associated with the sheet.
 *
 * Get/Set methods are public methods used to change object properties (they do not save data
 * to the file).
 *
 * The method SaveSolverSettings() is used to create the named ranges containing the current
 * property values into the file.
 *
 */

class SolverSettings
{
private:
    ScTable& m_rTable;
    ScDocument& m_rDoc;
    ScDocShell* m_pDocShell;

    // Used to read/write the named ranges in the tab
    ScRangeName* m_pRangeName;

    OUString m_sObjCell;
    ObjectiveType m_eObjType;
    OUString m_sObjVal;
    OUString m_sVariableCells;
    OUString m_sLOEngineName;
    OUString m_sMSEngineId;

    // Solver engine options
    OUString m_sInteger;
    OUString m_sNonNegative;
    OUString m_sEpsilonLevel;
    OUString m_sLimitBBDepth;
    OUString m_sTimeout;
    OUString m_sAlgorithm;
    // DEPS and SCO
    OUString m_sSwarmSize;
    OUString m_sLearningCycles;
    OUString m_sGuessVariableRange;
    OUString m_sVariableRangeThreshold;
    OUString m_sUseACRComparator;
    OUString m_sUseRandomStartingPoint;
    OUString m_sUseStrongerPRNG;
    OUString m_sStagnationLimit;
    OUString m_sTolerance;
    OUString m_sEnhancedSolverStatus;
    // DEPS only
    OUString m_sAgentSwitchRate;
    OUString m_sScalingFactorMin;
    OUString m_sScalingFactorMax;
    OUString m_sCrossoverProbability;
    OUString m_sCognitiveConstant;
    OUString m_sSocialConstant;
    OUString m_sConstrictionCoeff;
    OUString m_sMutationProbability;
    // SCO only
    OUString m_sLibrarySize;

    std::vector<ModelConstraint> m_aConstraints;

    void Initialize();

    // Used to create or read a single solver parameter based on its named range
    bool ReadParamValue(SolverParameter eParam, OUString& rValue, bool bRemoveQuotes = false);
    bool ReadDoubleParamValue(SolverParameter eParam, OUString& rValue);
    void WriteParamValue(SolverParameter eParam, OUString sValue, bool bQuoted = false);
    void WriteDoubleParamValue(SolverParameter eParam, std::u16string_view sValue);

    // Creates or reads all constraints stored in named ranges
    void ReadConstraints();
    void WriteConstraints();

    // Used to create or get a single constraint part
    bool ReadConstraintPart(ConstraintPart ePart, tools::Long nIndex, OUString& rValue);
    void WriteConstraintPart(ConstraintPart ePart, tools::Long nIndex, const OUString& sValue);

    // Creates or reads all named ranges associated with solver engine options
    void ReadEngine();
    void WriteEngine();

    void DeleteAllNamedRanges();

public:
    /* A SolverSettings object is linked to the ScTable where solver parameters
     *  are located and saved to */
    SolverSettings(ScTable& pTable);

    SC_DLLPUBLIC OUString GetParameter(SolverParameter eParam);
    SC_DLLPUBLIC void SetParameter(SolverParameter eParam, const OUString& sValue);
    SC_DLLPUBLIC ObjectiveType GetObjectiveType() { return m_eObjType; }
    SC_DLLPUBLIC void SetObjectiveType(ObjectiveType eType);
    SC_DLLPUBLIC void GetEngineOptions(css::uno::Sequence<css::beans::PropertyValue>& aOptions);
    SC_DLLPUBLIC void
    SetEngineOptions(const css::uno::Sequence<css::beans::PropertyValue>& aOptions);
    SC_DLLPUBLIC const std::vector<ModelConstraint>& GetConstraints() { return m_aConstraints; }
    SC_DLLPUBLIC void SetConstraints(std::vector<ModelConstraint> aConstraints);

    SC_DLLPUBLIC void SaveSolverSettings();
    SC_DLLPUBLIC void ResetToDefaults();
    SC_DLLPUBLIC bool TabHasSolverModel();
};

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
