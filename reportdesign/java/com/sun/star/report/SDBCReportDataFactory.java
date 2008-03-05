/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SDBCReportDataFactory.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:27:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
package com.sun.star.report;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
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
import java.util.Iterator;
import java.util.Vector;

/**
 * Very primitive implementation, just to show how this could be used ...
 *
 */
public class SDBCReportDataFactory implements DataSourceFactory
{

    private XRowSet rowSet;
    private XConnection connection;
    private XComponentContext m_cmpCtx;

    public SDBCReportDataFactory(final XComponentContext cmpCtx, final XConnection connection)
    {
        this.connection = connection;
        m_cmpCtx = cmpCtx;
    }

    public DataSource queryData(final String command, final Map parameters) throws DataSourceException
    {
        try
        {
            rowSet = (XRowSet) UnoRuntime.queryInterface(XRowSet.class, m_cmpCtx.getServiceManager().createInstanceWithContext("com.sun.star.sdb.RowSet", m_cmpCtx));
            final XPropertySet rowSetProp = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, rowSet);
            int commandType = CommandType.TABLE;
            final String commandTypeValue = (String) parameters.get("command-type");
            if (commandTypeValue.equals("query"))
            {
                commandType = CommandType.QUERY;
            }
            else if (commandTypeValue.equals("command"))
            {
                commandType = CommandType.COMMAND;
            }


            rowSetProp.setPropertyValue("ActiveConnection", connection);
            rowSetProp.setPropertyValue("CommandType", new Integer(commandType));
            rowSetProp.setPropertyValue("Command", command);

            final String filter = (String) parameters.get("filter");
            if (filter != null)
            {
                rowSetProp.setPropertyValue("Filter", filter);
                rowSetProp.setPropertyValue("ApplyFilter", new Boolean(filter.length() != 0));
            }
            else
            {
                rowSetProp.setPropertyValue("ApplyFilter", new Boolean(false));
            }
            final XConnectionTools tools = (XConnectionTools) UnoRuntime.queryInterface(XConnectionTools.class, connection);
            String order = getOrderStatement(commandType, command, (Vector) parameters.get("group-expressions"));
            if (order.length() > 0)
            {
                if (commandType != CommandType.TABLE)
                {

                    XSingleSelectQueryComposer composer = tools.getComposer(commandType, command);
                    String sCurrentSQL = composer.getQuery();
                    // Magic here, read the nice documentation out of the IDL.
                    composer.setQuery(sCurrentSQL);
                    String sOldOrder = composer.getOrder();
                    if (sOldOrder.length() > 0)
                    {
                        order += ",";
                        order += sOldOrder;
                        composer.setOrder("");
                        final String sQuery = composer.getQuery();
                        rowSetProp.setPropertyValue("Command", sQuery);
                        rowSetProp.setPropertyValue("CommandType", new Integer(CommandType.COMMAND));
                    }
                }
            }
            rowSetProp.setPropertyValue("Order", order);

            if (command.length() != 0)
            {
                Vector masterValues = (Vector) parameters.get("master-values");
                if (masterValues != null && masterValues.size() > 0)
                {
                    // Vector masterColumns = (Vector) parameters.get("master-columns");
                    Vector detailColumns = (Vector) parameters.get("detail-columns");
                    XSingleSelectQueryComposer composer = tools.getComposer(commandType, command);
                    StringBuffer oldFilter = new StringBuffer();
                    oldFilter.append(composer.getFilter());
                    if (oldFilter.length() != 0)
                    {
                        oldFilter.append(" AND ");
                    }
                    //Iterator masterIt = masterColumns.iterator();
                    int newParamterCounter = 1;
                    for (Iterator it = detailColumns.iterator(); it.hasNext(); ++newParamterCounter)
                    {
                        String detail = (String) it.next();
                        //String master = (String) masterIt.next();
                        oldFilter.append(detail);
                        oldFilter.append(" = :_");
                        oldFilter.append(newParamterCounter);
                        if (it.hasNext())
                        {
                            oldFilter.append(" AND ");
                        }
                    }

                    composer.setFilter(oldFilter.toString());

                    final String sQuery = composer.getQuery();
                    rowSetProp.setPropertyValue("Command", sQuery);
                    rowSetProp.setPropertyValue("CommandType", new Integer(CommandType.COMMAND));

                    int oldParameterCount = 0;
                    final XParametersSupplier paraSup = (XParametersSupplier) UnoRuntime.queryInterface(XParametersSupplier.class, composer);
                    if (paraSup != null)
                    {
                        final XIndexAccess params = paraSup.getParameters();
                        if (params != null)
                        {
                            oldParameterCount = params.getCount();
                        }
                    }

                    final XParameters para = (XParameters) UnoRuntime.queryInterface(XParameters.class, rowSet);
                    for (int i = 0; i < masterValues.size(); i++)
                    {
                        final Object object = masterValues.elementAt(i);
                        para.setObject(oldParameterCount + i, object);
                    }
                }

                XCompletedExecution execute = (XCompletedExecution) UnoRuntime.queryInterface(XCompletedExecution.class, rowSet);
                if (execute != null)
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

    private String getOrderStatement(final int commandType, final String command, final Vector groupExpressions)
    {
        StringBuffer order = new StringBuffer();
        final int count = groupExpressions.size();
        if (count == 0)
        {
            return order.toString();
        }
        try
        {
            final String quote = connection.getMetaData().getIdentifierQuoteString();
            final XConnectionTools tools = (XConnectionTools) UnoRuntime.queryInterface(XConnectionTools.class, connection);
            XComponent[] hold = new XComponent[2];
            XNameAccess columns = tools.getFieldsByCommandDescriptor(commandType, command, hold);


            for (int i = 0; i < count; i++)
            {
                Object[] pair = (Object[]) groupExpressions.elementAt(i);
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
                        order.append(" ");
                    }
                    String sorting = (String) pair[1];
                    if (sorting == null || sorting.equals("false"))
                    {
                        order.append("DESC");
                    }
                    if ((i + 1) < count)
                    {
                        order.append(",");
                    }
                }
            }

        }
        catch (IndexOutOfBoundsException ex)
        {
            ex.printStackTrace();
        }
        catch (SQLException sQLException)
        {
            sQLException.printStackTrace();
        }

        return order.toString();
    }
}
