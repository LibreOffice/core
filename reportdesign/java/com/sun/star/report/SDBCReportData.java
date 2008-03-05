/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SDBCReportData.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:27:10 $
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
import com.sun.star.sdb.XParametersSupplier;
import java.sql.Timestamp;

import com.sun.star.sdbc.DataType;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XResultSetMetaData;
import com.sun.star.sdbc.XResultSetMetaDataSupplier;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbc.XRowSet;
import com.sun.star.uno.Any;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.DateTime;
import com.sun.star.util.Time;

public class SDBCReportData implements DataSource
{

    private XRowSet rowSet;
    private XRow row;
    private int rowCount;
    private XIndexAccess parameters;
    private int firstParameterIndex = -1;
    private int columnCount;
    private final String[] columnNames;
    private final int[] columnTypes;

    public SDBCReportData(final XRowSet rowSet) throws SQLException
    {
        row = (XRow) UnoRuntime.queryInterface(XRow.class, rowSet);
        this.rowSet = rowSet;

        XParametersSupplier xSuppParams = (XParametersSupplier) UnoRuntime.queryInterface(
                XParametersSupplier.class, rowSet);
        if (xSuppParams != null)
        {
            parameters = xSuppParams.getParameters();
        }

        final XResultSetMetaDataSupplier sup = (XResultSetMetaDataSupplier) UnoRuntime.queryInterface(XResultSetMetaDataSupplier.class, rowSet);
        final XResultSetMetaData resultSetMetaData = sup.getMetaData();

        columnCount = resultSetMetaData.getColumnCount();
        if (parameters != null)
        {
            firstParameterIndex = columnCount + 1;
            columnCount += parameters.getCount();
        }

        columnTypes = new int[columnCount];
        columnNames = new String[columnCount];

        for (int i = 1; i <= columnCount; ++i)
        {
            if (i < firstParameterIndex)
            {
                columnNames[i - 1] = resultSetMetaData.getColumnName(i);
                columnTypes[i - 1] = resultSetMetaData.getColumnType(i);
            }
            else
            {
                try
                {
                    XPropertySet paramColumn = (XPropertySet) UnoRuntime.queryInterface(
                            XPropertySet.class, parameters.getByIndex(i - firstParameterIndex));
                    columnNames[i - 1] = (String) paramColumn.getPropertyValue("Name");
                    columnTypes[i - 1] = ((Integer) paramColumn.getPropertyValue("Type")).intValue();
                }
                catch (Exception e)
                {
                    columnNames[i - 1] = "Error";
                    columnTypes[i - 1] = DataType.CHAR;
                }
            }
        }

        if (rowSet.last())
        {
            rowCount = rowSet.getRow();
            rowSet.beforeFirst();
        }
        else
        {
            rowCount = 0;
        }
    }

    public int getColumnCount() throws DataSourceException
    {
        return columnCount;
    }

    public int getRowCount()
    {
        return rowCount;
    }

    public String getColumnName(final int column) throws DataSourceException
    {
        return columnNames[column - 1];
    }

    public boolean absolute(final int row) throws DataSourceException
    {
        try
        {
            if (row == 0)
            {
                rowSet.beforeFirst();
                return true;
            }
            return rowSet.absolute(row);
        }
        catch (SQLException e)
        {
            throw new DataSourceException(e.getMessage(), e);
        }
    }

    public boolean next() throws DataSourceException
    {
        try
        {
            return rowSet.next();
        }
        catch (SQLException e)
        {
            throw new DataSourceException(e.getMessage(), e);
        }
    }

    public void close() throws DataSourceException
    {
    }

//  public int getCurrentRow() throws DataSourceException
//  {
//    try
//    {
//      return rowSet.getRow();
//    }
//    catch (SQLException e)
//    {
//      throw new DataSourceException(e.getMessage(), e);
//    }
//  }
    static private java.sql.Date getDate(final Object obj)
    {
        java.sql.Date date = null;
        if (obj != null && obj instanceof com.sun.star.util.Date)
        {
            final com.sun.star.util.Date unodate = (com.sun.star.util.Date) obj;
            date = java.sql.Date.valueOf(getDateString(unodate.Year, unodate.Month, unodate.Day).toString());
        }
        return date;
    }

    private static StringBuffer getTimeString(final int hours, final int minutes, final int seconds)
    {
        final StringBuffer timeString = new StringBuffer();
        if (hours < 10)
        {
            timeString.append("0");
        }
        timeString.append(hours);
        timeString.append(":");
        if (minutes < 10)
        {
            timeString.append("0");
        }
        timeString.append(minutes);
        timeString.append(":");
        if (seconds < 10)
        {
            timeString.append("0");
        }
        timeString.append(seconds);
        return timeString;
    }

    static private StringBuffer getDateString(final int years, final int months, final int days)
    {
        final StringBuffer str = new StringBuffer();
        str.append(years);
        StringBuffer str2 = new StringBuffer("0000");
        str2 = str2.delete(0, str.length());
        str.insert(0, str2);
        str.append("-");
        if (months < 10)
        {
            str.append("0");
        }
        str.append(months);
        str.append("-");
        if (days < 10)
        {
            str.append("0");
        }
        str.append(days);
        return str;
    }

    static private java.sql.Time getTime(final Object obj)
    {
        java.sql.Time time = null;
        if (obj != null && obj instanceof Time)
        {
            final Time unoTime = (Time) obj;
            time = java.sql.Time.valueOf(getTimeString(unoTime.Hours, unoTime.Minutes, unoTime.Seconds).toString());
        }
        return time;
    }

    static private Timestamp getTimestamp(final Object obj)
    {
        Timestamp ts = null;
        if (obj != null && obj instanceof DateTime)
        {
            final DateTime unoTs = (DateTime) obj;
            final StringBuffer str = getDateString(unoTs.Year, unoTs.Month, unoTs.Day);
            str.append(" ");
            str.append(getTimeString(unoTs.Hours, unoTs.Minutes, unoTs.Seconds));
            str.append(".");
            str.append(unoTs.HundredthSeconds);
            ts = java.sql.Timestamp.valueOf(str.toString());
        }
        return ts;
    }

    public Object getObject(final int column) throws DataSourceException
    {
        try
        {
            boolean isParameterValue = (parameters != null) && (column >= firstParameterIndex);
            Object obj = null;
            boolean wasNull = true;
            if (isParameterValue)
            {
                try
                {
                    XPropertySet paramCol = (XPropertySet) UnoRuntime.queryInterface(
                            XPropertySet.class, parameters.getByIndex(column - firstParameterIndex));
                    obj = paramCol.getPropertyValue("Value");
                    wasNull = false;
                }
                catch (Exception e)
                {
                    wasNull = true;
                }
            }
            else
            {
                obj = row.getObject(column, null);
                wasNull = row.wasNull();
            }

            if (wasNull)
            {
                return null;
            }

            switch (columnTypes[column - 1])
            {
                case DataType.DATE:
                    obj = getDate(obj);
                    break;
                case DataType.TIME:
                    obj = getTime(obj);
                    break;
                case DataType.TIMESTAMP:
                    obj = getTimestamp(obj);
                    break;
                case DataType.DECIMAL:
                case DataType.NUMERIC:
                    if (obj != null && !(obj instanceof Any))
                    {
                        obj = new java.math.BigDecimal((String) obj);
                    }
                    break;
                default:
                    break;
            }
            return obj;
        }
        catch (SQLException e)
        {
            throw new DataSourceException(e.getMessage(), e);
        }
    }
}
