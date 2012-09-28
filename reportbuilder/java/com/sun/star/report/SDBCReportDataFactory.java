/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package com.sun.star.report;

import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XComponent;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdb.XCompletedExecution;
import com.sun.star.sdb.XParametersSupplier;
import com.sun.star.sdb.XQueriesSupplier;
import com.sun.star.sdb.XSingleSelectQueryComposer;
import com.sun.star.sdb.tools.XConnectionTools;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XParameters;
import com.sun.star.sdbc.XRowSet;
import com.sun.star.task.XInteractionHandler;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

import java.lang.reflect.Method;

import java.math.BigDecimal;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;


/**
 * Very primitive implementation, just to show how this could be used ...
 *
 */
public class SDBCReportDataFactory implements DataSourceFactory
{

    private static final String ESCAPEPROCESSING = "EscapeProcessing";

    private class RowSetProperties
    {

        final Boolean escapeProcessing;
        final int commandType;
        final Integer maxRows;
        final String command;
        final String filter;

        public RowSetProperties(final Boolean escapeProcessing, final int commandType, final String command, final String filter, final Integer maxRows)
        {
            this.escapeProcessing = escapeProcessing;
            this.commandType = commandType;
            this.command = command;
            this.filter = filter;
            this.maxRows = maxRows;
        }

        public boolean equals(Object obj)
        {
            if (obj == null)
            {
                return false;
            }
            if (getClass() != obj.getClass())
            {
                return false;
            }
            final RowSetProperties other = (RowSetProperties) obj;
            if (this.escapeProcessing != other.escapeProcessing && (this.escapeProcessing == null || !this.escapeProcessing.equals(other.escapeProcessing)))
            {
                return false;
            }
            if (this.commandType != other.commandType)
            {
                return false;
            }
            if (this.maxRows != other.maxRows && (this.maxRows == null || !this.maxRows.equals(other.maxRows)))
            {
                return false;
            }
            if ((this.command == null) ? (other.command != null) : !this.command.equals(other.command))
            {
                return false;
            }
            if ((this.filter == null) ? (other.filter != null) : !this.filter.equals(other.filter))
            {
                return false;
            }
            return true;
        }

        public int hashCode()
        {
            int hash = 3;
            hash = 59 * hash + (this.escapeProcessing != null ? this.escapeProcessing.hashCode() : 0);
            hash = 59 * hash + this.commandType;
            hash = 59 * hash + (this.maxRows != null ? this.maxRows.hashCode() : 0);
            hash = 59 * hash + (this.command != null ? this.command.hashCode() : 0);
            hash = 59 * hash + (this.filter != null ? this.filter.hashCode() : 0);
            return hash;
        }
    }

    class ParameterDefinition
    {
        int parameterCount = 0;
        private ArrayList<Integer> parameterIndex = new ArrayList<Integer>();
    }
    private static final Log LOGGER = LogFactory.getLog(SDBCReportDataFactory.class);
    public static final String COMMAND_TYPE = "command-type";
    public static final String ESCAPE_PROCESSING = "escape-processing";
    public static final String GROUP_EXPRESSIONS = "group-expressions";
    public static final String MASTER_VALUES = "master-values";
    public static final String MASTER_COLUMNS = "master-columns";
    public static final String DETAIL_COLUMNS = "detail-columns";
    public static final String UNO_FILTER = "Filter";
    private static final String APPLY_FILTER = "ApplyFilter";
    private static final String UNO_COMMAND = "Command";
    private static final String UNO_ORDER = "Order";
    private static final String UNO_APPLY_FILTER = "ApplyFilter";
    private static final String UNO_COMMAND_TYPE = "CommandType";
    private final XConnection connection;
    private final XComponentContext m_cmpCtx;
    private static final int FAILED = 0;
    private static final int DONE = 1;
    private static final int RETRIEVE_COLUMNS = 2;
    private static final int RETRIEVE_OBJECT = 3;
    private static final int HANDLE_QUERY = 4;
    private static final int HANDLE_TABLE = 5;
    private static final int HANDLE_SQL = 6;
    private final Map<RowSetProperties,XRowSet> rowSetProperties = new HashMap<RowSetProperties,XRowSet>();
    private final Map<RowSetProperties,ParameterDefinition> parameterMap = new HashMap<RowSetProperties,ParameterDefinition>();
    private boolean rowSetCreated = false;

    public SDBCReportDataFactory(final XComponentContext cmpCtx, final XConnection connection)
    {
        this.connection = connection;
        m_cmpCtx = cmpCtx;
    }

    public DataSource queryData(final String command, final Map<String,Object> parameters) throws DataSourceException
    {
        try
        {
            if (command == null)
            {
                return new SDBCReportData(null);
            }
            int commandType = CommandType.COMMAND;
            final String commandTypeValue = (String) parameters.get(COMMAND_TYPE);
            if (commandTypeValue != null)
            {
                if ("query".equals(commandTypeValue))
                {
                    commandType = CommandType.QUERY;
                }
                else if ("table".equals(commandTypeValue))
                {
                    commandType = CommandType.TABLE;
                }
                else
                {
                    commandType = CommandType.COMMAND;
                }
            }
            final Boolean escapeProcessing = (Boolean) parameters.get(ESCAPE_PROCESSING);
            final String filter = (String) parameters.get(UNO_FILTER);
            final Integer maxRows = (Integer) parameters.get("MaxRows");
            final RowSetProperties rowSetProps = new RowSetProperties(escapeProcessing, commandType, command, filter, maxRows);

            final Object[] p = createRowSet(rowSetProps, parameters);
            final XRowSet rowSet = (XRowSet) p[0];

            if (command.length() != 0)
            {
                final ParameterDefinition paramDef = (ParameterDefinition) p[1];
                fillParameter(parameters, rowSet, paramDef);
                rowSetCreated = rowSetCreated && (maxRows == null || maxRows == 0);

                final XCompletedExecution execute = (XCompletedExecution) UnoRuntime.queryInterface(XCompletedExecution.class, rowSet);
                if (rowSetCreated && execute != null && paramDef.parameterCount > 0)
                {
                    final XInteractionHandler handler = (XInteractionHandler) UnoRuntime.queryInterface(XInteractionHandler.class, m_cmpCtx.getServiceManager().createInstanceWithContext("com.sun.star.sdb.InteractionHandler", m_cmpCtx));
                    execute.executeWithCompletion(handler);
                }
                else
                {
                    rowSet.execute();
                }
            }

            rowSetCreated = false;
            return new SDBCReportData(rowSet);
        }
        catch (Exception ex)
        {
            rowSetCreated = false;
            throw new DataSourceException(ex.getMessage(), ex);
        }
    }

    private String getOrderStatement(final int commandType, final String command, final List groupExpressions)
    {
        final StringBuffer order = new StringBuffer();
        final int count = groupExpressions.size();
        if (count != 0)
        {
            try
            {
                final String quote = connection.getMetaData().getIdentifierQuoteString();
                final XComponent[] hold = new XComponent[1];
                final XNameAccess columns = getFieldsByCommandDescriptor(commandType, command, hold);
                if (columns != null)
                {
                    for (int i = 0; i < count; i++)
                    {
                        final Object[] pair = (Object[]) groupExpressions.get(i);
                        String expression = (String) pair[0];

                        if (!expression.startsWith(quote) && columns.hasByName(expression))
                        {
                            XPropertySet column;
                            try
                            {
                                column = UnoRuntime.queryInterface(XPropertySet.class, columns.getByName(expression));
                                expression = quote + column.getPropertyValue("TableName") + quote + "." + quote + expression + quote;
                            }
                            catch (Exception ex)
                            {
                                Logger.getLogger(SDBCReportDataFactory.class.getName()).log(Level.SEVERE, null, ex);
                                expression = quote + expression + quote;
                            }
                        }
                        expression = expression.trim(); // Trim away white spaces

                        if (expression.length() > 0)
                        {
                            order.append(expression);
                            if (order.length() > 0)
                            {
                                order.append(' ');
                            }
                            final String sorting = (String) pair[1];
                            if (sorting == null || sorting.equals(OfficeToken.FALSE))
                            {
                                order.append("DESC");
                            }
                            if ((i + 1) < count)
                            {
                                order.append(", ");
                            }
                        }
                    }
                }
            }
            catch (SQLException ex)
            {
                LOGGER.error("ReportProcessing failed", ex);
            }
        }
        return order.toString();
    }

    private XNameAccess getFieldsByCommandDescriptor(final int commandType, final String command, final XComponent[] out) throws SQLException
    {
        final Class[] parameter = new Class[3];
        parameter[0] = int.class;
        parameter[1] = String.class;
        parameter[2] = out.getClass();
        final XConnectionTools tools = (XConnectionTools) UnoRuntime.queryInterface(XConnectionTools.class, connection);
        try
        {
            tools.getClass().getMethod("getFieldsByCommandDescriptor", parameter);
            return tools.getFieldsByCommandDescriptor(commandType, command, out);
        }
        catch (NoSuchMethodException ex)
        {
        }

        throw new SQLException();
    }

    private XSingleSelectQueryComposer getComposer(final XConnectionTools tools,
            final String command,
            final int commandType)
    {
        final Class[] parameter = new Class[2];
        parameter[0] = int.class;
        parameter[1] = String.class;
        try
        {
            final Object[] param = new Object[2];
            param[0] = commandType;
            param[1] = command;
            final Method call = tools.getClass().getMethod("getComposer", parameter);
            return (XSingleSelectQueryComposer) call.invoke(tools, param);
        }
        catch (NoSuchMethodException ex)
        {
        }
        catch (IllegalAccessException ex)
        {
            // should not happen
            // assert False
        }
        catch (java.lang.reflect.InvocationTargetException ex)
        {
            // should not happen
            // assert False
        }

        return null;
    }

    private void fillParameter(final Map parameters,
            final XRowSet rowSet, final ParameterDefinition paramDef)
            throws SQLException,
            UnknownPropertyException,
            PropertyVetoException,
            IllegalArgumentException,
            WrappedTargetException
    {
        final ArrayList<?> masterValues = (ArrayList<?>) parameters.get(MASTER_VALUES);
        if (masterValues != null && !masterValues.isEmpty())
        {
            final XParameters para = (XParameters) UnoRuntime.queryInterface(XParameters.class, rowSet);

            for (int i = 0;
                    i < masterValues.size();
                    i++)
            {
                Object object = masterValues.get(i);
                if (object instanceof BigDecimal)
                {
                    object = ((BigDecimal) object).toString();
                }
                final Integer pos = paramDef.parameterIndex.get(i);
                para.setObject(pos + 1, object);
            }
        }
    }

    private final Object[] createRowSet(final RowSetProperties rowSetProps, final Map<String,Object> parameters)
            throws Exception
    {
        final ArrayList<?> detailColumns = (ArrayList<?>) parameters.get(DETAIL_COLUMNS);
        if (rowSetProperties.containsKey(rowSetProps) && detailColumns != null && !detailColumns.isEmpty())
        {
            return new Object[]
                    {
                        rowSetProperties.get(rowSetProps), parameterMap.get(rowSetProps)
                    };
        }

        rowSetCreated = true;
        final XRowSet rowSet = (XRowSet) UnoRuntime.queryInterface(XRowSet.class, m_cmpCtx.getServiceManager().createInstanceWithContext("com.sun.star.sdb.RowSet", m_cmpCtx));
        final XPropertySet rowSetProp = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, rowSet);

        rowSetProp.setPropertyValue("ActiveConnection", connection);
        rowSetProp.setPropertyValue(ESCAPEPROCESSING, rowSetProps.escapeProcessing);
        rowSetProp.setPropertyValue(UNO_COMMAND_TYPE, Integer.valueOf(rowSetProps.commandType));
        rowSetProp.setPropertyValue(UNO_COMMAND, rowSetProps.command);

        if (rowSetProps.filter != null)
        {
            rowSetProp.setPropertyValue("Filter", rowSetProps.filter);
            rowSetProp.setPropertyValue(APPLY_FILTER, Boolean.valueOf(rowSetProps.filter.length() != 0));
        }
        else
        {
            rowSetProp.setPropertyValue(APPLY_FILTER, Boolean.FALSE);
        }

        if (rowSetProps.maxRows != null)
        {
            rowSetProp.setPropertyValue("MaxRows", rowSetProps.maxRows);
        }

        final XConnectionTools tools = (XConnectionTools) UnoRuntime.queryInterface(XConnectionTools.class, connection);
        fillOrderStatement(rowSetProps.command, rowSetProps.commandType, parameters, tools, rowSetProp);
        final ParameterDefinition paramDef = createParameter(parameters, tools, rowSetProps, rowSet);

        rowSetProperties.put(rowSetProps, rowSet);
        parameterMap.put(rowSetProps, paramDef);

        return new Object[]
                {
                    rowSet, paramDef
                };
    }

    private ParameterDefinition createParameter(final Map parameters,
            final XConnectionTools tools,
            RowSetProperties rowSetProps, final XRowSet rowSet)
            throws SQLException,
            UnknownPropertyException,
            PropertyVetoException,
            IllegalArgumentException,
            WrappedTargetException
    {
        final ParameterDefinition paramDef = new ParameterDefinition();
        final XSingleSelectQueryComposer composer = getComposer(tools, rowSetProps.command, rowSetProps.commandType);
        if (composer != null)
        {
            final XPropertySet rowSetProp = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, rowSet);
            if ((Boolean) rowSetProp.getPropertyValue(APPLY_FILTER))
            {
                composer.setFilter((String) rowSetProp.getPropertyValue("Filter"));
            }
            // get old parameter count
            final ArrayList<?> detailColumns = (ArrayList<?>) parameters.get(DETAIL_COLUMNS);
            final ArrayList<String> handledColumns = new ArrayList<String>();
            final XParametersSupplier paraSup = (XParametersSupplier) UnoRuntime.queryInterface(XParametersSupplier.class, composer);
            if (paraSup != null)
            {
                final XIndexAccess params = paraSup.getParameters();
                if (params != null)
                {
                    final int oldParameterCount = params.getCount();
                    paramDef.parameterCount = oldParameterCount;
                    if (detailColumns != null)
                    {
                        for (int i = 0; i < oldParameterCount; i++)
                        {
                            try
                            {
                                final XPropertySet parameter = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, params.getByIndex(i));
                                if (parameter != null)
                                {
                                    final String name = (String) parameter.getPropertyValue("Name");
                                    for (int j = 0; j < detailColumns.size(); j++)
                                    {
                                        if (name.equals(detailColumns.get(j)))
                                        {
                                            handledColumns.add(name);
                                            paramDef.parameterIndex.add(i);
                                            --paramDef.parameterCount;
                                            break;
                                        }
                                    }
                                }
                            }
                            catch (IndexOutOfBoundsException ex)
                            {
                                Logger.getLogger(SDBCReportDataFactory.class.getName()).log(Level.SEVERE, null, ex);
                            }
                        }
                    }
                }
            }
            final ArrayList<?> masterValues = (ArrayList<?>) parameters.get(MASTER_VALUES);
            if (masterValues != null && !masterValues.isEmpty() && paramDef.parameterIndex.size() != detailColumns.size())
            {
                // Vector masterColumns = (Vector) parameters.get("master-columns");

                // create the new filter
                final String quote = connection.getMetaData().getIdentifierQuoteString();
                final StringBuffer oldFilter = new StringBuffer();
                oldFilter.append(composer.getFilter());
                if (oldFilter.length() != 0)
                {
                    oldFilter.append(" AND ");
                }
                int newParamterCounter = 1;
                for (final Iterator it = detailColumns.iterator(); it.hasNext();
                        ++newParamterCounter)
                {
                    final String detail = (String) it.next();
                    if (!handledColumns.contains(detail))
                    {
                        //String master = (String) masterIt.next();
                        oldFilter.append(quote);
                        oldFilter.append(detail);
                        oldFilter.append(quote);
                        oldFilter.append(" = :link_");
                        oldFilter.append(newParamterCounter);
                        if (it.hasNext())
                        {
                            oldFilter.append(" AND ");
                        }
                        paramDef.parameterIndex.add(newParamterCounter + paramDef.parameterCount - 1);
                    }
                }

                composer.setFilter(oldFilter.toString());

                final String sQuery = composer.getQuery();
                rowSetProp.setPropertyValue(UNO_COMMAND, sQuery);
                rowSetProp.setPropertyValue(UNO_COMMAND_TYPE, Integer.valueOf(CommandType.COMMAND));
            }
        }
        return paramDef;
    }

    void fillOrderStatement(final String command,
            final int commandType, final Map parameters,
            final XConnectionTools tools,
            final XPropertySet rowSetProp)
            throws SQLException,
            UnknownPropertyException,
            PropertyVetoException,
            IllegalArgumentException,
            WrappedTargetException,
            NoSuchElementException
    {
        final StringBuffer order = new StringBuffer(getOrderStatement(commandType, command, (ArrayList<?>) parameters.get(GROUP_EXPRESSIONS)));
        if (order.length() > 0 && commandType != CommandType.TABLE)
        {
            String statement = command;
            final XSingleSelectQueryComposer composer = getComposer(tools, command, commandType);
            if (composer != null)
            {
                statement = composer.getQuery();
                composer.setQuery(statement);
                final String sOldOrder = composer.getOrder();
                if (sOldOrder.length() > 0)
                {
                    order.append(',');
                    order.append(sOldOrder);
                    composer.setOrder("");
                    statement = composer.getQuery();
                }
            }
            else
            {
                if (commandType == CommandType.QUERY)
                {
                    final XQueriesSupplier xSupplyQueries = (XQueriesSupplier) UnoRuntime.queryInterface(XQueriesSupplier.class, connection);
                    final XNameAccess queries = xSupplyQueries.getQueries();
                    if (queries.hasByName(command))
                    {
                        final XPropertySet prop = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, queries.getByName(command));
                        final Boolean escape = (Boolean) prop.getPropertyValue(ESCAPEPROCESSING);
                        rowSetProp.setPropertyValue(ESCAPEPROCESSING, escape);
                        final String queryCommand = (String) prop.getPropertyValue(UNO_COMMAND);
                        statement = "SELECT * FROM (" + queryCommand + ") AS \"__LibreOffice_report_result\"";
                    }

                }
                else
                {
                    statement = "SELECT * FROM (" + command + ") AS \"__LibreOffice_report_result\"";
                }
            }
            rowSetProp.setPropertyValue(UNO_COMMAND, statement);
            rowSetProp.setPropertyValue(UNO_COMMAND_TYPE, Integer.valueOf(CommandType.COMMAND));
        }
        rowSetProp.setPropertyValue("Order", order.toString());
    }
}

