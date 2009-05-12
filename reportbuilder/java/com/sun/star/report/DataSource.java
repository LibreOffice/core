/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DataSource.java,v $
 * $Revision: 1.4 $
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

/**
 * This feeds data into the reporting engine, in case the data has been provided
 * by the caller. The methods are a mix of TableModel methods and methods borrowed
 * from the java.sql.ResultSet interface.
 *
 * The column and row index starts at 1 (as it is done in JDBC).
 *
 * @author Thomas Morgner
 */
public interface DataSource extends DataRow
{

    public int getRowCount() throws DataSourceException;

    public boolean absolute(int row) throws DataSourceException;

    public boolean next() throws DataSourceException;

    public void close() throws DataSourceException;
}
