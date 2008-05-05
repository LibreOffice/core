/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SDBCReportDataFactory.java,v $
 * $Revision: 1.5 $
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
package com.sun.star.report;

import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.sdbc.XConnection;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdb.XCompletedExecution;
import com.sun.star.sdb.XParametersSupplier;
import com.sun.star.sdb.XSingleSelectQueryComposer;
import com.sun.star.sdb.tools.XConnectionTools;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XParameters;
import com.sun.star.uno.Exception;
import java.util.Map;

import com.sun.star.sdbc.XRowSet;
import com.sun.star.task.XInteractionHandler;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import org.jfree.util.Log;

/**
 * Very primitive implementation, just to show how this could be used ...
 *
 */
public class SDBCReportDataFactory implements DataSourceFactory
{

    public static final String COMMAND_TYPE = "command-type";
    public static final String GROUP_EXPRESSIONS = "group-expressions";
    public static final String MASTER_VALUES = "master-values";
    public static final String DETAIL_COLUMNS = "detail-columns";
    private static final String APPLY_FILTER = "ApplyFilter";
    private static final String UNO_COMMAND = "Command";
    private static final String UNO_COMMAND_TYPE = "CommandType";
    private final XConnection connection;
    private final XComponentContext m_cmpCtx;

    public SDBCReportDataFactory(final XComponentContext cmpCtx, final XConnection connection)
    {
        this.connection = connection;
        m_cmpCtx = cmpCtx;
    }

    public DataSource queryData(final String command, final Map parameters) throws DataSourceException
    {
        try
        {
            int commandType = CommandType.COMMAND;
            final String commandTypeValue = (String) parameters.get(COMMAND_TYPE);
            if (commandTypeValue != null)
            {
                if (commandTypeValue.equals("query"))
                {
                    commandType = CommandType.QUERY;
                }
                else if (commandTypeValue.equals("table"))
                {
                    commandType = CommandType.TABLE;
                }
                else
                {
                    commandType = CommandType.COMMAND;
                }
            }
            final XRowSet rowSet = createRowSet(command, commandType, parameters);
            final XPropertySet rowSetProp = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, rowSet);

            final XConnectionTools tools = (XConnectionTools) UnoRuntime.queryInterface(XConnectionTools.class, connection);
            fillOrderStatement(command, commandType, parameters, tools, rowSetProp);

            if (command.length() != 0)
            {
                final int oldParameterCount = fillParameter(parameters, tools, command, commandType, rowSet);

                final XCompletedExecution execute = (XCompletedExecution) UnoRuntime.queryInterface(XCompletedExecution.class, rowSet);
                if (execute != null && oldParameterCount > 0)
                {
                    final XInteractionHandler handler = (XInteractionHandler) UnoRuntime.queryInterface(XInteractionHandler.class, m_cmpCtx.getServiceManager().createInstanceWithContext("com.sun.star.sdb.InteractionHandler", m_cmpCtx));
                    execute.executeWithCompletion(handler);
                }
                else
                {
                    rowSet.execute();
                }
            }
            return new SDBCReportData(rowSet);
        }
        catch (Exception ex)
        {
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
                final XConnectionTools tools = (XConnectionTools) UnoRuntime.queryInterface(XConnectionTools.class, connection);
                final XComponent[] hold = new XComponent[2];
                final XNameAccess columns = tools.getFieldsByCommandDescriptor(commandType, command, hold);

                for (int i = 0; i < count; i++)
                {
                    final Object[] pair = (Object[]) groupExpressions.get(i);
                    String expression = (String) pair[0];

                    if (columns.hasByName(expression))
                    {
                        expression = quote + expression + quote;
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
                            order.append(' ');
                        }
                    }
                }
            }
            catch (IndexOutOfBoundsException ex)
            {
                Log.error("ReportProcessing failed", ex);
            }
            catch (SQLException ex)
            {
                Log.error("ReportProcessing failed", ex);
            }
        }
        return order.toString();
    }

    int fillParameter(final Map parameters, final XConnectionTools tools, final String command, final int commandType, final XRowSet rowSet) throws SQLException, UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException
    {
        int oldParameterCount = 0;
        final ArrayList masterValues = (ArrayList) parameters.get(MASTER_VALUES);
        if (masterValues != null && !masterValues.isEmpty())
        {
            // Vector masterColumns = (Vector) parameters.get("master-columns");
            final ArrayList detailColumns = (ArrayList) parameters.get(DETAIL_COLUMNS);
            final XSingleSelectQueryComposer composer = tools.getComposer(commandType, command);

            // get old parameter count
            final XParametersSupplier paraSup = (XParametersSupplier) UnoRuntime.queryInterface(XParametersSupplier.class, composer);
            if (paraSup != null)
            {
                final XIndexAccess params = paraSup.getParameters();
                if (params != null)
                {
                    oldParameterCount = params.getCount();
                }
            }
            // create the new filter
            final String quote = connection.getMetaData().getIdentifierQuoteString();
            final StringBuffer oldFilter = new StringBuffer();
            oldFilter.append(composer.getFilter());
            if (oldFilter.length() != 0)
            {
                oldFilter.append(" AND ");
            }

            int newParamterCounter = 1;

            for (final Iterator it = detailColumns.iterator(); it.hasNext(); ++newParamterCounter)
            {
                final String detail = (String) it.next();
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
            }

            composer.setFilter(oldFilter.toString());

            final String sQuery = composer.getQuery();
            final XPropertySet rowSetProp = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, rowSet);
            rowSetProp.setPropertyValue(UNO_COMMAND, sQuery);
            rowSetProp.setPropertyValue(UNO_COMMAND_TYPE, new Integer(CommandType.COMMAND));

            final XParameters para = (XParameters) UnoRuntime.queryInterface(XParameters.class, rowSet);
            for (int i = 0; i < masterValues.size(); i++)
            {
                final Object object = masterValues.get(i);
                para.setObject(oldParameterCount + i + 1, object);
            }
        }
        return oldParameterCount;
    }

    final XRowSet createRowSet(final String command, final int commandType, final Map parameters) throws Exception
    {
        final XRowSet rowSet = (XRowSet) UnoRuntime.queryInterface(XRowSet.class, m_cmpCtx.getServiceManager().createInstanceWithContext("com.sun.star.sdb.RowSet", m_cmpCtx));
        final XPropertySet rowSetProp = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, rowSet);

        rowSetProp.setPropertyValue("ActiveConnection", connection);
        rowSetProp.setPropertyValue(UNO_COMMAND_TYPE, new Integer(commandType));
        rowSetProp.setPropertyValue(UNO_COMMAND, command);

        final String filter = (String) parameters.get("filter");
        if (filter != null)
        {
            rowSetProp.setPropertyValue("Filter", filter);
            rowSetProp.setPropertyValue(APPLY_FILTER, Boolean.valueOf(filter.length() != 0));
        }
        else
        {
            rowSetProp.setPropertyValue(APPLY_FILTER, Boolean.FALSE);
        }
        return rowSet;
    }

    void fillOrderStatement(final String command, final int commandType, final Map parameters, final XConnectionTools tools, final XPropertySet rowSetProp) throws SQLException, UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException
    {
        final StringBuffer order = new StringBuffer(getOrderStatement(commandType, command, (ArrayList) parameters.get(GROUP_EXPRESSIONS)));
        if (order.length() > 0 && commandType != CommandType.TABLE)
        {
            final XSingleSelectQueryComposer composer = tools.getComposer(commandType, command);
            final String sCurrentSQL = composer.getQuery();
            // Magic here, read the nice documentation out of the IDL.
            composer.setQuery(sCurrentSQL);
            final String sOldOrder = composer.getOrder();
            if (sOldOrder.length() > 0)
            {
                order.append(',');
                order.append(sOldOrder);
                composer.setOrder("");
                final String sQuery = composer.getQuery();
                rowSetProp.setPropertyValue(UNO_COMMAND, sQuery);
                rowSetProp.setPropertyValue(UNO_COMMAND_TYPE, new Integer(CommandType.COMMAND));
            }
        }
        rowSetProp.setPropertyValue("Order", order.toString());
    }
}
