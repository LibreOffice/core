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
package org.libreoffice.report;

import com.sun.star.awt.XWindow;
import com.sun.star.beans.NamedValue;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XChild;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.frame.XController;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdb.XCompletedExecution;
import com.sun.star.sdb.XDocumentDataSource;
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

import java.math.BigDecimal;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;


/**
 * Very primitive implementation, just to show how this could be used ...
 *
 */
public class SDBCReportDataFactory implements DataSourceFactory
{

    private static final String ESCAPEPROCESSING = "EscapeProcessing";

    private static class RowSetProperties
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

        @Override
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
            if ((this.escapeProcessing == null) ? (other.escapeProcessing != null) : !this.escapeProcessing.equals(other.escapeProcessing))
            {
                return false;
            }
            if (this.commandType != other.commandType)
            {
                return false;
            }
            if ((this.maxRows == null) ? (other.maxRows != null) : !this.maxRows.equals(other.maxRows))
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

        @Override
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

    private static class ParameterDefinition
    {
        private int parameterCount = 0;
        private final ArrayList<Integer> parameterIndex = new ArrayList<Integer>();
    }
    private static final Logger LOGGER = Logger.getLogger(SDBCReportDataFactory.class.getName());
    public static final String COMMAND_TYPE = "command-type";
    public static final String ESCAPE_PROCESSING = "escape-processing";
    public static final String SORT_EXPRESSIONS = "sort-expressions";
    public static final String GROUP_EXPRESSIONS = "group-expressions";
    public static final String MASTER_VALUES = "master-values";
    public static final String MASTER_COLUMNS = "master-columns";
    public static final String DETAIL_COLUMNS = "detail-columns";
    public static final String UNO_FILTER = "Filter";
    private static final String APPLY_FILTER = "ApplyFilter";
    private static final String UNO_COMMAND = "Command";
    private static final String UNO_COMMAND_TYPE = "CommandType";
    private final XConnection connection;
    private final XComponentContext m_cmpCtx;
    private final Map<RowSetProperties,XRowSet> rowSetProperties = new HashMap<RowSetProperties,XRowSet>();
    private final Map<RowSetProperties,ParameterDefinition> parameterMap = new HashMap<RowSetProperties,ParameterDefinition>();
    private boolean rowSetCreated = false;

    public SDBCReportDataFactory(final XComponentContext cmpCtx, final XConnection connection)
    {
        this.connection = connection;
        m_cmpCtx = cmpCtx;
    }

    public XWindow getParentWindow()
    {
        final XChild child = UnoRuntime.queryInterface(XChild.class, connection);
        if (child == null)
            return null;
        final XDocumentDataSource docSource = UnoRuntime.queryInterface(XDocumentDataSource.class, child.getParent());
        if (docSource == null)
            return null;
        final XModel model = UnoRuntime.queryInterface(XModel.class, docSource.getDatabaseDocument());
        if (model == null)
            return null;
        final XController controller = model.getCurrentController();
        if (controller == null)
            return null;
        final XFrame frame = controller.getFrame();
        if (frame == null)
            return null;
        return frame.getContainerWindow();
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

                final XCompletedExecution execute = UnoRuntime.queryInterface(XCompletedExecution.class, rowSet);
                if (rowSetCreated && execute != null && paramDef.parameterCount > 0)
                {
                    final XWindow window = getParentWindow();
                    final XInteractionHandler handler = UnoRuntime.queryInterface(XInteractionHandler.class, m_cmpCtx.getServiceManager().createInstanceWithArgumentsAndContext("com.sun.star.sdb.InteractionHandler", new Object[] { new NamedValue("Parent", window) }, m_cmpCtx));
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

    private String getOrderStatement(final List sortExpressions)
    {
        final StringBuffer order = new StringBuffer();
        final int count = sortExpressions.size();
        String quote;
        try
        {
            quote = connection.getMetaData().getIdentifierQuoteString();
        }
        catch (SQLException ex)
        {
            LOGGER.severe("ReportProcessing failed / getOrderStatement could not get quote character: " + ex);
            // fall back to the SQL standard
            quote="";
        }
        for (int i = 0; i < count; i++)
        {
            final Object[] pair = (Object[]) sortExpressions.get(i);
            String expression = (String) pair[0];

            // LEM FIXME: ${EXPLETIVE}! Either the values we get are *always* already quoted
            // (and then this whole work is not useful)
            // or they are *never* quoted
            // (and then just quote them unconditionally)
            // The current mess gives an ambiguity when the column name starts with a quote character.
            // It *seems* they are never quoted, but this needs further testing.
            if (!expression.startsWith(quote))
            {
                expression = quote + expression + quote;
                // LEM TODO: we should escape quotes in expression?
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
        return order.toString();
    }

    private XSingleSelectQueryComposer getComposer(final XConnectionTools tools,
            final String command,
            final int commandType)
    {
        return tools.getComposer(commandType, command);
    }

    private void fillParameter(final Map parameters,
            final XRowSet rowSet, final ParameterDefinition paramDef)
            throws SQLException,
            IllegalArgumentException
    {
        final ArrayList<?> masterValues = (ArrayList<?>) parameters.get(MASTER_VALUES);
        if (masterValues != null && !masterValues.isEmpty())
        {
            final XParameters para = UnoRuntime.queryInterface(XParameters.class, rowSet);

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
        final XRowSet rowSet = UnoRuntime.queryInterface(XRowSet.class, m_cmpCtx.getServiceManager().createInstanceWithContext("com.sun.star.sdb.RowSet", m_cmpCtx));
        final XPropertySet rowSetProp = UnoRuntime.queryInterface(XPropertySet.class, rowSet);

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

        final XConnectionTools tools = UnoRuntime.queryInterface(XConnectionTools.class, connection);
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
            final XPropertySet rowSetProp = UnoRuntime.queryInterface(XPropertySet.class, rowSet);
            if ((Boolean) rowSetProp.getPropertyValue(APPLY_FILTER))
            {
                composer.setFilter((String) rowSetProp.getPropertyValue("Filter"));
            }
            // get old parameter count
            final ArrayList<?> detailColumns = (ArrayList<?>) parameters.get(DETAIL_COLUMNS);
            final ArrayList<String> handledColumns = new ArrayList<String>();
            final XParametersSupplier paraSup = UnoRuntime.queryInterface(XParametersSupplier.class, composer);
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
                                final XPropertySet parameter = UnoRuntime.queryInterface(XPropertySet.class, params.getByIndex(i));
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

    private void fillOrderStatement(final String command,
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
        final StringBuffer order = new StringBuffer(getOrderStatement((ArrayList<?>) parameters.get(SORT_EXPRESSIONS)));
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
                    final XQueriesSupplier xSupplyQueries = UnoRuntime.queryInterface(XQueriesSupplier.class, connection);
                    final XNameAccess queries = xSupplyQueries.getQueries();
                    if (queries.hasByName(command))
                    {
                        final XPropertySet prop = UnoRuntime.queryInterface(XPropertySet.class, queries.getByName(command));
                        final Boolean escape = (Boolean) prop.getPropertyValue(ESCAPEPROCESSING);
                        rowSetProp.setPropertyValue(ESCAPEPROCESSING, escape);
                        final String queryCommand = (String) prop.getPropertyValue(UNO_COMMAND);
                        statement = "SELECT * FROM (" + queryCommand + ") \"__LibreOffice_report_result\"";
                    }

                }
                else
                {
                    statement = "SELECT * FROM (" + command + ") \"__LibreOffice_report_result\"";
                }
            }
            rowSetProp.setPropertyValue(UNO_COMMAND, statement);
            rowSetProp.setPropertyValue(UNO_COMMAND_TYPE, Integer.valueOf(CommandType.COMMAND));
        }
        rowSetProp.setPropertyValue("Order", order.toString());
    }
}

