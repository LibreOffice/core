/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StarReportData.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:30:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
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
package com.sun.star.report.pentaho;

import com.sun.star.report.DataSource;
import org.jfree.report.DataSourceException;
import org.jfree.report.ReportData;

public class StarReportData implements ReportData
{

    private DataSource dataSource;
    private int currentRow;
    private int rowCount;

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
            boolean ret = dataSource.absolute(row);
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
        if (isReadable() == false)
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
