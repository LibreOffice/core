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
package org.libreoffice.report.pentaho;

import org.libreoffice.report.DataSource;

import org.jfree.report.DataSourceException;
import org.jfree.report.ReportData;

public class StarReportData implements ReportData
{

    private final DataSource dataSource;
    private int currentRow;
    private final int rowCount;

    public StarReportData(final DataSource dataSource)
            throws org.libreoffice.report.DataSourceException
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
        catch (org.libreoffice.report.DataSourceException e)
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
        catch (org.libreoffice.report.DataSourceException e)
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
        catch (org.libreoffice.report.DataSourceException e)
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
        catch (org.libreoffice.report.DataSourceException e)
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
        catch (org.libreoffice.report.DataSourceException e)
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
        catch (org.libreoffice.report.DataSourceException e)
        {
            throw new DataSourceException("Failed to query column name.", e);
        }
    }

    public boolean isReadable() throws DataSourceException
    {
        return currentRow > 0 && rowCount > 0;
    }
}
