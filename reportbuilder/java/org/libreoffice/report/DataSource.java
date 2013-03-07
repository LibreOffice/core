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

/**
 * This feeds data into the reporting engine, in case the data has been provided
 * by the caller. The methods are a mix of TableModel methods and methods borrowed
 * from the java.sql.ResultSet interface.
 *
 * The column and row index starts at 1 (as it is done in JDBC).
 *
 */
public interface DataSource extends DataRow
{

    int getRowCount() throws DataSourceException;

    boolean absolute(int row) throws DataSourceException;

    boolean next() throws DataSourceException;

    void close() throws DataSourceException;
}
