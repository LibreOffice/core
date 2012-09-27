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

import com.sun.star.awt.XReschedule;
import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.chart.XChartDataArray;
import com.sun.star.container.XIndexAccess;
import com.sun.star.document.XEmbeddedObjectSupplier;
import com.sun.star.frame.XModel;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.sheet.SolverConstraint;
import com.sun.star.sheet.SolverConstraintOperator;
import com.sun.star.sheet.XCellRangeData;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.CellAddress;
import com.sun.star.table.CellContentType;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCell;
import com.sun.star.table.XTableChartsSupplier;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.logging.Level;
import java.util.logging.Logger;

public abstract class BaseNLPSolver extends WeakBase
    implements com.sun.star.lang.XLocalizable,
               com.sun.star.sheet.XSolver,
               com.sun.star.sheet.XSolverDescription,
               com.sun.star.beans.XPropertySet,
               com.sun.star.beans.XPropertySetInfo
{

    protected final XComponentContext m_xContext;
    protected final String m_name;

    protected final ArrayList<PropertyInfo> m_properties = new ArrayList<PropertyInfo>();
    protected final HashMap<String, PropertyInfo> m_propertyMap = new HashMap<String, PropertyInfo>();

    protected com.sun.star.lang.Locale m_locale = new com.sun.star.lang.Locale();
    protected final ResourceManager resourceManager;

    public BaseNLPSolver(XComponentContext xContext, String name) {
        m_xContext = xContext;
        m_name = name;

        m_componentFactory = xContext.getServiceManager();
        try {
            Object toolkit = m_componentFactory.createInstanceWithContext("com.sun.star.awt.Toolkit", xContext);
            m_xReschedule = (XReschedule) UnoRuntime.queryInterface(XReschedule.class, toolkit);
        } catch (Exception ex) {
            Logger.getLogger(BaseNLPSolver.class.getName()).log(Level.SEVERE, null, ex);
        }

        resourceManager = new ResourceManager(xContext, "com.sun.star.comp.Calc.NLPSolver", "/locale", "NLPSolverCommon");

        registerProperty(m_assumeNonNegative);
    }

    protected void registerProperty(PropertyInfo property) {
        m_properties.add(property);
        m_propertyMap.put(property.getProperty().Name, property);
        property.localize(resourceManager);
    }

    // com.sun.star.lang.XLocalizable:
    public void setLocale(com.sun.star.lang.Locale eLocale)
    {
        m_locale = eLocale;
    }

    public com.sun.star.lang.Locale getLocale()
    {
        return m_locale;
    }

    // com.sun.star.sheet.XSolver:

    protected XSpreadsheetDocument m_document;
    protected XMultiComponentFactory m_componentFactory;
    protected XModel m_xModel;
    protected XReschedule m_xReschedule;
    protected CellAddress m_objective;
    protected CellAddress[] m_variables;
    protected SolverConstraint[] m_constraints;
    protected ExtSolverConstraint[] m_extConstraints;
    protected boolean m_maximize;

    protected int m_variableCount;
    protected int m_constraintCount;
    protected int m_cellRangeCount;
    protected XCell m_objectiveCell;
    protected XCell[] m_variableCells;
    protected CellRangeAddress[] m_cellRanges;
    protected XChartDataArray[] m_cellRangeData;
    protected CellMap[] m_variableMap;
    protected double[][][] m_variableData;

    protected double m_functionValue;
    protected double[] m_currentParameters;
    protected boolean m_success = false;

    public XSpreadsheetDocument getDocument() {
        return m_document;
    }

    public void setDocument(XSpreadsheetDocument document) {
        m_document = document;
        m_xModel = (XModel) UnoRuntime.queryInterface(XModel.class, m_document);
    }

    public CellAddress getObjective() {
        return m_objective;
    }

    public void setObjective(CellAddress objective) {
        m_objective = objective;
        m_objectiveCell = getCell(objective);
    }

    public CellAddress[] getVariables() {
        if (m_variables == null)
            return new CellAddress[0]; //Workaround for basic scripts; otherwise
                                       //setting the Variables property fails.
        return m_variables;
    }

    protected class RowInfo {
        protected short Sheet;
        protected int Row;
        protected int StartCol;
        protected int EndCol;

        public RowInfo(short sheet, int row) {
            Sheet = sheet;
            Row = row;
        }

        public CellRangeAddress getCellRangeAddress(int lastRow) {
            CellRangeAddress result = new CellRangeAddress();
            result.Sheet = Sheet;
            result.StartColumn = StartCol;
            result.StartRow = Row;
            result.EndColumn = EndCol;
            result.EndRow = lastRow;
            return result;
        }
    }

    protected class CellMap {
        protected int Range;
        protected int Col;
        protected int Row;
    }

    protected class ExtSolverConstraint {

        public XCell Left;
        public SolverConstraintOperator Operator;
        public XCell Right;
        public double Data;

        public ExtSolverConstraint(XCell left, SolverConstraintOperator operator, Object right) {
            this.Left = left;
            this.Operator = operator;
            this.Right = null;
            if (right instanceof Number) {
                this.Data = ((Number)right).doubleValue();
            } else if (right instanceof CellAddress) {
                XCell cell = getCell((CellAddress)right);
                if (cell.getType() == CellContentType.VALUE) {
                    this.Data = cell.getValue();
                } else {
                    this.Right = cell;
                    this.Data = 0.0;
                }
            }
        }

        public double getLeftValue() {
            if (this.Right == null) {
                return this.Left.getValue();
            } else {
                return this.Left.getValue() - this.Right.getValue();
            }
        }

    }

    public void setVariables(CellAddress[] variables) {
        m_variables = variables;
        m_variableCount = variables.length;

        //update cell references
        m_variableCells = new XCell[m_variableCount];
        m_currentParameters = new double[m_variableCount];
        for (int i = 0; i < m_variableCount; i++) {
            m_variableCells[i] = getCell(variables[i]);
            m_currentParameters[i] = m_variableCells[i].getValue();
        }

        //parse for cell ranges (under the assumption, that the cells are ordered
        //left to right, top to bottom for each cell range
        m_variableMap = new CellMap[m_variableCount];
        m_variableData = new double[m_variableCount][][];

        ArrayList<RowInfo> rows = new ArrayList<RowInfo>();
        RowInfo currentRow = null;
        int lastSheet = -1, lastRow = -1;
        for (int i = 0; i < m_variableCount; i++) {
            if (lastSheet == m_variables[i].Sheet && lastRow == m_variables[i].Row &&
                    currentRow.EndCol == m_variables[i].Column - 1)
                currentRow.EndCol++;
            else {
                currentRow = new RowInfo(m_variables[i].Sheet, m_variables[i].Row);
                currentRow.StartCol = m_variables[i].Column;
                currentRow.EndCol = m_variables[i].Column;
                rows.add(currentRow);
                lastSheet = currentRow.Sheet;
                lastRow = currentRow.Row;
            }
        }

        ArrayList<CellRangeAddress> cellRangeAddresses = new ArrayList<CellRangeAddress>();
        if (rows.size() > 0) {
            RowInfo firstRow = rows.get(0);
            int offset = 0;
            for (int i = 1; i < rows.size(); i++) {
                currentRow = rows.get(i);
                if (currentRow.Sheet != firstRow.Sheet ||
                        currentRow.Row != firstRow.Row + offset + 1 ||
                        currentRow.StartCol != firstRow.StartCol ||
                        currentRow.EndCol != firstRow.EndCol) {
                    cellRangeAddresses.add(firstRow.getCellRangeAddress(firstRow.Row + offset));
                    firstRow = currentRow;
                    offset = 0;
                } else {
                    offset++;
                }
            }
            cellRangeAddresses.add(firstRow.getCellRangeAddress(firstRow.Row + offset));
        }

        m_cellRangeCount = cellRangeAddresses.size();
        m_cellRanges = new CellRangeAddress[m_cellRangeCount];
        m_cellRanges = cellRangeAddresses.toArray(m_cellRanges);
        m_cellRangeData = new XChartDataArray[m_cellRangeCount];
        int varID = 0;
        //get cell range data and map the variables to their new location
        for (int i = 0; i < m_cellRangeCount; i++) {
            for (int y = 0; y <= m_cellRanges[i].EndRow - m_cellRanges[i].StartRow; y++)
                for (int x = 0; x <= m_cellRanges[i].EndColumn - m_cellRanges[i].StartColumn; x++) {
                    CellMap map = new CellMap();
                    m_variableMap[varID++] = map;
                    map.Range = i;
                    map.Col = x;
                    map.Row = y;
                }
            m_cellRangeData[i] = getChartDataArray(m_cellRanges[i]);
            m_variableData[i] = m_cellRangeData[i].getData();
        }
    }

    public SolverConstraint[] getConstraints() {
        if (m_constraints == null)
            return new SolverConstraint[0]; //Workaround for basic scripts; otherwise
                                            //setting the Constraints property fails.
        return m_constraints;
    }

    public void setConstraints(SolverConstraint[] constraints) {
        m_constraints = constraints;
        m_constraintCount = constraints.length;

        //update cell references
        m_extConstraints = new ExtSolverConstraint[m_constraintCount];
        for (int i = 0; i < m_constraintCount; i++) {
            m_extConstraints[i] = new ExtSolverConstraint(
                    getCell(constraints[i].Left),
                    constraints[i].Operator,
                    constraints[i].Right);
        }
    }

    public boolean getMaximize() {
        return m_maximize;
    }

    public void setMaximize(boolean maximize) {
        m_maximize = maximize;
    }

    public boolean getSuccess() {
        return m_success;
    }

    public double getResultValue() {
        return m_functionValue;
    }

    public double[] getSolution() {
        return m_currentParameters;
    }

    protected XCell getCell(CellAddress cellAddress) {
        return getCell(cellAddress.Column, cellAddress.Row, cellAddress.Sheet);
    }

    protected XCell getCell(int col, int row, int sheet) {
        try {
            XSpreadsheets xSpreadsheets = m_document.getSheets();
            XIndexAccess xSheetIndex = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets);
            XSpreadsheet xSpreadsheet = (XSpreadsheet) UnoRuntime.queryInterface(XSpreadsheet.class, xSheetIndex.getByIndex(sheet));
            return xSpreadsheet.getCellByPosition(col, row);
        } catch (IndexOutOfBoundsException ex) {
            Logger.getLogger(BaseNLPSolver.class.getName()).log(Level.SEVERE, null, ex);
        } catch (WrappedTargetException ex) {
            Logger.getLogger(BaseNLPSolver.class.getName()).log(Level.SEVERE, null, ex);
        }

        return null;
    }

    protected XCellRangeData getCellRangeData(CellRangeAddress cellRangeAddress) {
        return getCellRangeData(cellRangeAddress.Sheet, cellRangeAddress.StartColumn,
                cellRangeAddress.StartRow, cellRangeAddress.EndColumn, cellRangeAddress.EndRow);
    }

    protected XCellRangeData getCellRangeData(int sheet, int startCol, int startRow, int endCol, int endRow) {
        try {
            XSpreadsheets xSpreadsheets = m_document.getSheets();
            XIndexAccess xSheetIndex = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets);
            XSpreadsheet xSpreadsheet = (XSpreadsheet) UnoRuntime.queryInterface(XSpreadsheet.class, xSheetIndex.getByIndex(sheet));
            return (XCellRangeData) UnoRuntime.queryInterface(XCellRangeData.class, xSpreadsheet.getCellRangeByPosition(startCol, startRow, endCol, endRow));
        } catch (IndexOutOfBoundsException ex) {
            Logger.getLogger(BaseNLPSolver.class.getName()).log(Level.SEVERE, null, ex);
        } catch (WrappedTargetException ex) {
            Logger.getLogger(BaseNLPSolver.class.getName()).log(Level.SEVERE, null, ex);
        }

        return null;
    }

    protected XChartDataArray getChartDataArray(CellRangeAddress cellRangeAddress) {
        return getChartDataArray(cellRangeAddress.Sheet, cellRangeAddress.StartColumn,
                cellRangeAddress.StartRow, cellRangeAddress.EndColumn, cellRangeAddress.EndRow);
    }

    protected XChartDataArray getChartDataArray(int sheet, int startCol, int startRow, int endCol, int endRow) {
        try {
            XSpreadsheets xSpreadsheets = m_document.getSheets();
            XIndexAccess xSheetIndex = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets);
            XSpreadsheet xSpreadsheet = (XSpreadsheet) UnoRuntime.queryInterface(XSpreadsheet.class, xSheetIndex.getByIndex(sheet));
            return (XChartDataArray) UnoRuntime.queryInterface(XChartDataArray.class, xSpreadsheet.getCellRangeByPosition(startCol, startRow, endCol, endRow));
        } catch (IndexOutOfBoundsException ex) {
            Logger.getLogger(BaseNLPSolver.class.getName()).log(Level.SEVERE, null, ex);
        } catch (WrappedTargetException ex) {
            Logger.getLogger(BaseNLPSolver.class.getName()).log(Level.SEVERE, null, ex);
        }

        return null;
    }

    protected PropertyInfo<Boolean> m_assumeNonNegative = new PropertyInfo<Boolean>("AssumeNonNegative", false, "Assume Non-Negative Variables");

    protected void initializeSolve() {
        lockDocument();
    }

    protected void finalizeSolve() {
        unlockDocument();
    }

    public String getComponentDescription() {
        return m_name;
    }

    public String getStatusDescription() {
        return "";
    }

    public String getPropertyDescription(String property) {
        PropertyInfo propertyInfo = m_propertyMap.get(property);
        if (propertyInfo != null)
            return propertyInfo.getDescription();
        else
            return "";
    }

    // com.sun.star.beans.XPropertySet:

    public XPropertySetInfo getPropertySetInfo() {
        return this;
    }

    public void setPropertyValue(String property, Object value) throws UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException {
        PropertyInfo propertyInfo = m_propertyMap.get(property);
        if (propertyInfo != null)
            propertyInfo.setValue(value);
        else
            throw new UnknownPropertyException();
    }

    public Object getPropertyValue(String property) throws UnknownPropertyException, WrappedTargetException {
        PropertyInfo propertyInfo = m_propertyMap.get(property);
        if (propertyInfo != null)
            return propertyInfo.getValue();
        else
            throw new UnknownPropertyException();
    }

    public void addPropertyChangeListener(String property, XPropertyChangeListener listener) throws UnknownPropertyException, WrappedTargetException {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    public void removePropertyChangeListener(String property, XPropertyChangeListener listener) throws UnknownPropertyException, WrappedTargetException {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    public void addVetoableChangeListener(String property, XVetoableChangeListener listener) throws UnknownPropertyException, WrappedTargetException {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    public void removeVetoableChangeListener(String property, XVetoableChangeListener listener) throws UnknownPropertyException, WrappedTargetException {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    // com.sun.star.beans.XPropertySetInfo:

    public Property[] getProperties() {
        int propertyCount = m_properties.size();
        Property[] properties = new Property[propertyCount];
        for (int i = 0; i < propertyCount; i++)
            properties[i] = m_properties.get(i).getProperty();
        return properties;
    }

    public Property getPropertyByName(String property) throws UnknownPropertyException {
        PropertyInfo propertyInfo = m_propertyMap.get(property);
        if (propertyInfo != null)
            return propertyInfo.getProperty();
        else
            throw new UnknownPropertyException();
    }

    public boolean hasPropertyByName(String property) {
        return m_properties.contains(property);
    }

    // <editor-fold defaultstate="collapsed" desc="Helper functions">
    protected void lockDocument(boolean lock) {
        if (lock)
            m_xModel.lockControllers();
        else
            m_xModel.unlockControllers();

        try {
            XIndexAccess xSpreadsheets = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, m_document.getSheets());
            int sheets = xSpreadsheets.getCount();
            for (int i = 0; i < sheets; i++) {
                Object sheet = xSpreadsheets.getByIndex(i);
                XTableChartsSupplier xTableChartsSupplier = (XTableChartsSupplier) UnoRuntime.queryInterface(XTableChartsSupplier.class, sheet);
                XIndexAccess xCharts = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xTableChartsSupplier.getCharts());
                int charts = xCharts.getCount();
                for (int j = 0; j < charts; j++) {
                    Object chart = xCharts.getByIndex(j);
                    XEmbeddedObjectSupplier xChartObjects = (XEmbeddedObjectSupplier) UnoRuntime.queryInterface(XEmbeddedObjectSupplier.class, chart);
                    XModel xChartModel = (XModel) UnoRuntime.queryInterface(XModel.class, xChartObjects.getEmbeddedObject());
                    if (lock)
                        xChartModel.lockControllers();
                    else
                        xChartModel.unlockControllers();
                }
            }
        } catch (Exception ex) {
            Logger.getLogger(BaseNLPSolver.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    protected void lockDocument() {
        lockDocument(true);
    }

    protected void unlockDocument() {
        lockDocument(false);
    }

    public static String nanoTimeToString(ResourceManager resourceManager, long nanoseconds) {
        if (nanoseconds < 0) return null; //shouldn't happen .... but if it does, throw an error!

        if (nanoseconds == 0) return "0";

        if (nanoseconds < 1000)
            return nanoseconds + " " + resourceManager.getLocalizedString("Time.Nanoseconds", "Nanoseconds");

        double microseconds = (double) nanoseconds / 1000;
        if (microseconds < 1000)
            return String.format("%.2f %s", microseconds, resourceManager.getLocalizedString("Time.Microseconds", "Microseconds"));

        double milliseconds = microseconds / 1000;
        if (milliseconds < 1000)
            return String.format("%.2f %s", milliseconds, resourceManager.getLocalizedString("Time.Milliseconds", "Milliseconds"));

        double seconds = milliseconds / 1000;
        if (seconds < 90)
            return String.format("%.2f %s", seconds, resourceManager.getLocalizedString("Time.Seconds", "Seconds"));

        long minutes = (long) seconds / 60;
        seconds -= minutes * 60;
        long hours = minutes / 60;
        minutes -= hours * 60;
        long days = hours / 24;
        hours -= days * 24;

        if (days > 0)
            return String.format("%d %s, %d %s",
                  days, resourceManager.getLocalizedString(String.format("Time.Day%", days == 1 ? "" : "s"), "Days"),
                  hours, resourceManager.getLocalizedString(String.format("Time.Hour%s", hours == 1 ? "" : "s"), "Hours"));

        if (hours > 0)
            return String.format("%d %s, %d %s",
                  hours, resourceManager.getLocalizedString(String.format("Time.Hour%s", hours == 1 ? "" : "s"), "Hours"),
                  minutes, resourceManager.getLocalizedString(String.format("Time.Minute%s", minutes == 1 ? "" : "s"), "Minutes"));

        if (minutes > 0)
            return String.format("%d %s, %.0f %s",
                  minutes, resourceManager.getLocalizedString(String.format("Time.Minute%s", minutes == 1 ? "" : "s"), "Minutes"),
                  Math.floor(seconds), resourceManager.getLocalizedString(String.format("Time.Second%s", Math.floor(seconds) == 1 ? "" : "s"), "Seconds"));

        return String.format("%.2f %s", seconds, resourceManager.getLocalizedString("Time.Seconds", "Seconds"));
    }
    // </editor-fold>

}
