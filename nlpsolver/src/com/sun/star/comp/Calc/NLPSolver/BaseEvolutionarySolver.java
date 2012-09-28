/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2009 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package com.sun.star.comp.Calc.NLPSolver;

import com.sun.star.comp.Calc.NLPSolver.dialogs.DummyEvolutionarySolverStatusDialog;
import com.sun.star.comp.Calc.NLPSolver.dialogs.EvolutionarySolverStatusUno;
import com.sun.star.comp.Calc.NLPSolver.dialogs.IEvolutionarySolverStatusDialog;
import com.sun.star.sheet.SolverConstraintOperator;
import com.sun.star.uno.XComponentContext;
import java.util.ArrayList;
import net.adaptivebox.global.BasicBound;
import net.adaptivebox.goodness.ACRComparator;
import net.adaptivebox.goodness.BCHComparator;
import net.adaptivebox.goodness.IGoodnessCompareEngine;
import net.adaptivebox.knowledge.Library;
import net.adaptivebox.knowledge.SearchPoint;
import net.adaptivebox.problem.ProblemEncoder;

public abstract class BaseEvolutionarySolver extends BaseNLPSolver {

    public BaseEvolutionarySolver(XComponentContext xContext, String name) {
        super(xContext, name);

        registerProperty(m_swarmSize);
        registerProperty(m_learningCycles);
        registerProperty(m_guessVariableRange);
        registerProperty(m_variableRangeThreshold);
        registerProperty(m_useACRComperator);

        registerProperty(m_useRandomStartingPoint);

        registerProperty(m_required);
        registerProperty(m_tolerance);

        registerProperty(m_enhancedSolverStatus);
    }

    protected class Variable {
        protected CellMap CellMap;
        protected int OriginalVariable;
        protected double MinValue;
        protected double MaxValue;
        protected double Granularity;

        protected Variable(CellMap cellMap, int originalVariable) {
            this.CellMap = cellMap;
            this.OriginalVariable = originalVariable;
            this.MinValue = BasicBound.MINDOUBLE;
            this.MaxValue = BasicBound.MAXDOUBLE;
            this.Granularity = 0.0;
        }
    }

    protected class CalcProblemEncoder extends ProblemEncoder {

        private ArrayList<Variable> m_variables;
        private ArrayList<ExtSolverConstraint> m_constraints;

        protected CalcProblemEncoder(ArrayList<Variable> variables,
                ArrayList<ExtSolverConstraint> constraints) throws Exception {
            //m_variableCount variables to solve, target function + constraints to match
            super(variables.size(), 1 + constraints.size());

            m_variables = variables;
            m_constraints = constraints;

            double objective = m_maximize ? BasicBound.MAXDOUBLE : BasicBound.MINDOUBLE;
            setDefaultYAt(0, objective, objective);

            for (int i = 0; i < constraints.size(); i++) {
                ExtSolverConstraint constraint = constraints.get(i);

                switch (constraint.Operator.getValue()) {
                    case SolverConstraintOperator.EQUAL_value:
                        setDefaultYAt(i + 1, constraint.Data, constraint.Data);
                        break;
                    case SolverConstraintOperator.GREATER_EQUAL_value:
                        setDefaultYAt(i + 1, constraint.Data, BasicBound.MAXDOUBLE);
                        break;
                    case SolverConstraintOperator.LESS_EQUAL_value:
                        setDefaultYAt(i + 1, BasicBound.MINDOUBLE, constraint.Data);
                        break;
                }
            }

            for (int i = 0; i < m_variables.size(); i++) {
                Variable variable = m_variables.get(i);
                setDefaultXAt(i, variable.MinValue, variable.MaxValue, variable.Granularity);
            }
        }

        @Override
        protected double calcTargetAt(int index, double[] VX) {
            if (index == 0) {
                //calcTargetAt is called in a loop over all functions, so it's
                //enough to set the variables in the first step only
                for (int i = 0; i < m_variables.size(); i++) {
                    CellMap variableMap = m_variables.get(i).CellMap;
                    m_variableData[variableMap.Range][variableMap.Row][variableMap.Col] = VX[i];
                }
                for (int i = 0; i < m_cellRangeCount; i++)
                    m_cellRangeData[i].setData(m_variableData[i]);

                //errors are punished
                if (m_objectiveCell.getError() != 0)
                    return m_maximize ? BasicBound.MINDOUBLE : BasicBound.MAXDOUBLE;

                double result = m_objectiveCell.getValue();

                if (result >= m_toleratedMin && result <= m_toleratedMax && checkConstraints())
                    m_toleratedCount++;

                return result;
            } else
                return m_constraints.get(index - 1).getLeftValue();
        }

    }

    protected CalcProblemEncoder m_problemEncoder;
    protected Library m_library;
    protected IGoodnessCompareEngine m_envCompareEngine;
    protected IGoodnessCompareEngine m_specCompareEngine;
    protected SearchPoint m_totalBestPoint;

    protected int m_toleratedCount;
    protected double m_toleratedMin;
    protected double m_toleratedMax;

    protected ArrayList<Variable> m_variables = new ArrayList<Variable>();

    //properties
    protected PropertyInfo<Integer> m_swarmSize = new PropertyInfo<Integer>("SwarmSize", 70, "Size of Swam");
    protected PropertyInfo<Integer> m_librarySize = new PropertyInfo<Integer>("LibrarySize", 210, "Size of Library");
    protected PropertyInfo<Integer> m_learningCycles = new PropertyInfo<Integer>("LearningCycles", 2000, "Learning Cycles");
    protected PropertyInfo<Boolean> m_guessVariableRange = new PropertyInfo<Boolean>("GuessVariableRange", true, "Variable Bounds Guessing");
    protected PropertyInfo<Double> m_variableRangeThreshold = new PropertyInfo<Double>("VariableRangeThreshold", 3.0, "Variable Bounds Threshold (when guessing)"); //to approximate the variable bounds
    protected PropertyInfo<Boolean> m_useACRComperator = new PropertyInfo<Boolean>("UseACRComparator", false, "Use ACR Comparator (instead of BCH)");
    protected PropertyInfo<Boolean> m_useRandomStartingPoint = new PropertyInfo<Boolean>("UseRandomStartingPoint", false, "Use Random starting point");
    protected PropertyInfo<Integer> m_required = new PropertyInfo<Integer>("StagnationLimit", 70, "Stagnation Limit");
    protected PropertyInfo<Double> m_tolerance = new PropertyInfo<Double>("Tolerance", 1e-6, "Stagnation Tolerance");
    protected PropertyInfo<Boolean> m_enhancedSolverStatus = new PropertyInfo<Boolean>("EnhancedSolverStatus", true, "Show enhanced solver status");

    protected IEvolutionarySolverStatusDialog m_solverStatusDialog;

    protected void prepareVariables(double[][] variableBounds) {
        m_variables.clear();
        for (int i = 0; i < m_variableCount; i++) {
            Variable var = new Variable(m_variableMap[i], i);
            var.MinValue = variableBounds[i][0];
            var.MaxValue = variableBounds[i][1];
            var.Granularity = variableBounds[i][2];
            m_variables.add(var);
        }
    }

    @Override
    protected void initializeSolve() {
        super.initializeSolve();

        if (m_enhancedSolverStatus.getValue())
            m_solverStatusDialog = new EvolutionarySolverStatusUno(m_xContext);
        else
            m_solverStatusDialog = new DummyEvolutionarySolverStatusDialog();

        //Init:
        double[][] variableBounds = new double[m_variableCount][3];
        //approximate variable bounds
        for (int i = 0; i < m_variableCount; i++) {
            if (m_guessVariableRange.getValue()) {
                double value = m_variableCells[i].getValue();

                //0 is a bad starting point, so just pick some other.
                //That is certainly not optimal but the user should specify
                //bounds or at least a good starting point anyway.
                if (value == 0.0)
                    value = 1000;

                double b1;
                double b2;

                if (m_assumeNonNegative.getValue()) {
                    b1 = 0;
                    b2 = value + value * 2 * m_variableRangeThreshold.getValue();
                } else {
                    b1 = value + value * m_variableRangeThreshold.getValue();
                    b2 = value - value * m_variableRangeThreshold.getValue();
                }

                variableBounds[i][0] = Math.min(b1, b2);
                variableBounds[i][1] = Math.max(b1, b2);
            } else {
                //that almost always leads to bad or no solutions at all
                if (m_assumeNonNegative.getValue())
                    variableBounds[i][0] = 0.0;
                else
                    variableBounds[i][0] = BasicBound.MINDOUBLE;
                variableBounds[i][1] = BasicBound.MAXDOUBLE;
            }
            variableBounds[i][2] = 0.0;
        }

        //prepare constraints and parse them for variable bounds
        ArrayList<ExtSolverConstraint> constraints = new ArrayList<ExtSolverConstraint>();
        for (int i = 0; i < m_constraintCount; i++) {
            Double doubleValue;
            
            if (m_extConstraints[i].Right != null)
                doubleValue = null;
            else
                doubleValue = m_extConstraints[i].Data;

            boolean isVariableBound = false;
            //If it refers to a cell, it has to be treated as constraint, not as
            //bound.
            if (m_extConstraints[i].Right == null) {
                for (int j = 0; j < m_variableCount && !isVariableBound; j++) {
                    if (m_constraints[i].Left.Sheet == super.m_variables[j].Sheet &&
                            m_constraints[i].Left.Column == super.m_variables[j].Column &&
                            m_constraints[i].Left.Row == super.m_variables[j].Row) {
                        isVariableBound = true;

                        //Therefore we try to use it as bounds for this variable.

                        switch (m_extConstraints[i].Operator.getValue()) {
                            case SolverConstraintOperator.EQUAL_value:
                                if (doubleValue == null)
                                    continue;
                                variableBounds[j][0] = doubleValue;
                                variableBounds[j][1] = doubleValue;
                                break;
                            case SolverConstraintOperator.GREATER_EQUAL_value:
                                if (doubleValue == null)
                                    continue;
                                variableBounds[j][0] = doubleValue;
                                break;
                            case SolverConstraintOperator.LESS_EQUAL_value:
                                if (doubleValue == null)
                                    continue;
                                variableBounds[j][1] = doubleValue;
                                break;
                            case SolverConstraintOperator.INTEGER_value:
                                variableBounds[j][2] = 1.0;
                                break;
                            case SolverConstraintOperator.BINARY_value:
                                variableBounds[j][0] = 0.0;
                                variableBounds[j][1] = 1.0;
                                variableBounds[j][2] = 1.0;
                                break;
                            default:
                                //If it is neither <=, nor =, nor >=, we treat
                                //it as normal constraint.
                                isVariableBound = false;
                        }
                    }
                }
            }

            if (!isVariableBound) {
                constraints.add(m_extConstraints[i]);
            }
        }

        prepareVariables(variableBounds);

        try {
            m_problemEncoder = new CalcProblemEncoder(m_variables, constraints);
        } catch (Exception e) {
            m_problemEncoder = null;
            return;
        }

        m_library = new Library(m_librarySize.getValue(), m_problemEncoder);

        if (m_useRandomStartingPoint.getValue()) {
            m_totalBestPoint = m_problemEncoder.getEncodedSearchPoint();
        } else {
            m_totalBestPoint = m_problemEncoder.getFreshSearchPoint();
            double[] currentValues = new double[m_variables.size()];
            for (int i = 0; i < m_variables.size(); i++)
                currentValues[i] = m_currentParameters[m_variables.get(i).OriginalVariable];
            m_totalBestPoint.importLocation(currentValues);
            m_problemEncoder.evaluate(m_totalBestPoint);
        }
        //input the chosen point into the library as reference for the individuals
        m_library.getSelectedPoint(0).importPoint(m_totalBestPoint);

        m_solverStatusDialog.setBestSolution(m_totalBestPoint.getObjectiveValue(), checkConstraints());

        m_envCompareEngine = new BCHComparator();
        m_specCompareEngine = m_useACRComperator.getValue() ? new ACRComparator(m_library, m_learningCycles.getValue()) : new BCHComparator();
    }

    protected void applySolution() {
        double[] location = m_totalBestPoint.getLocation();

        //make sure, the "Integer" variable type is met
        m_problemEncoder.getDesignSpace().getMappingPoint(location);

        //get the function value for our optimal point
        for (int i = 0; i < m_variableCount; i++) {
            m_variableCells[i].setValue(location[i]);
            m_currentParameters[i] = location[i];
        }
        m_functionValue = m_objectiveCell.getValue();
    }

    @Override
    protected void finalizeSolve() {
        applySolution();

        m_success = (m_objectiveCell.getError() == 0 && checkConstraints());

        m_solverStatusDialog.setVisible(false);
        m_solverStatusDialog.dispose();

        super.finalizeSolve();
    }

    protected boolean checkConstraints() {
        boolean result = true;
        for (int i = 0; i < m_constraintCount && result; i++) {
            if (m_extConstraints[i].Left.getError() == 0) {
                Double value, targetValue;

                value = m_extConstraints[i].getLeftValue();
                targetValue = m_extConstraints[i].Data;

                switch (m_extConstraints[i].Operator.getValue()) {
                    case SolverConstraintOperator.EQUAL_value:
                        result = (targetValue != null && value.equals(targetValue));
                        break;
                    case SolverConstraintOperator.GREATER_EQUAL_value:
                        result = (targetValue != null && value >= targetValue);
                        break;
                    case SolverConstraintOperator.LESS_EQUAL_value:
                        result = (targetValue != null && value <= targetValue);
                        break;
                    case SolverConstraintOperator.INTEGER_value:
                        result = (Math.rint(value) == value);
                        break;
                    case SolverConstraintOperator.BINARY_value:
                        result = (value == 0.0 || value == 1.0);
                        break;
                }
            } else {
                result = false;
            }
        }

        return result;
    }

}
