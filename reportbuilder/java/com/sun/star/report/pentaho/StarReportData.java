/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: StarReportData.java,v $
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
package com.sun.star.report.pentaho;

import com.sun.star.report.DataSource;
import org.jfree.report.DataSourceException;
import org.jfree.report.ReportData;

public class StarReportData implements ReportData
{

    private final DataSource dataSource;
    private int currentRow;
    private final int rowCount;

    public StarReportData(final DataSource dataSource)
            throws com.sun.star.report.DataSourceException
    {
        if (dataSource == null)
        {
            throw new NullPointerException();
        }
        this.dataSource = dataSource;
        this.currentRow = 0;
        this.rowCount = dataSource.getRowCount();
    }

    public boolean setCursorPosition(final int row) throws DataSourceException
    {
        try
        {
            final boolean ret = dataSource.absolute(row);
            if (ret)
            {
                currentRow = row;
            }
            return ret;
        }
        catch (com.sun.star.report.DataSourceException e)
        {
            throw new DataSourceException("Failed to move cursor", e);
        }
    }

    public void close()
            throws DataSourceException
    {
        try
        {
            dataSource.close();
        }
        catch (com.sun.star.report.DataSourceException e)
        {
            throw new DataSourceException("Failed to close datasource", e);
        }
    }

    public int getCursorPosition()
            throws DataSourceException
    {
        return currentRow;
    }

    /**
     * This operation checks, whether a call to next will be likely to succeed. If
     * there is a next data row, this should return true.
     *
     * @return
     * @throws org.jfree.report.DataSourceException
     *
     */
    public boolean isAdvanceable() throws DataSourceException
    {
        return currentRow < rowCount;
    }

    public boolean next()
            throws DataSourceException
    {
        try
        {
            if (dataSource.next())
            {
                currentRow += 1;
                return true;
            }
            return false;
        }
        catch (com.sun.star.report.DataSourceException e)
        {
            throw new DataSourceException("Failed to move cursor", e);
        }
    }

    public Object get(final int column)
            throws DataSourceException
    {
        if (!isReadable())
        {
            throw new DataSourceException("Failed to query column.");
        }

        try
        {
            return dataSource.getObject(column + 1);
        }
        catch (com.sun.star.report.DataSourceException e)
        {
            throw new DataSourceException("Failed to query column.", e);
        }
    }

    public int getColumnCount()
            throws DataSourceException
    {
        try
        {
            return dataSource.getColumnCount();
        }
        catch (com.sun.star.report.DataSourceException e)
        {
            throw new DataSourceException("Failed to query column count.", e);
        }
    }

    public String getColumnName(final int column)
            throws DataSourceException
    {
        try
        {
            return dataSource.getColumnName(column + 1);
        }
        catch (com.sun.star.report.DataSourceException e)
        {
            throw new DataSourceException("Failed to query column name.", e);
        }
    }

    public boolean isReadable() throws DataSourceException
    {
        return currentRow > 0 && rowCount > 0;
    }
}
