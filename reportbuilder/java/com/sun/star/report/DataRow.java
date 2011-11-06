/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
public interface DataRow
{

    int getColumnCount() throws DataSourceException;

    String getColumnName(int column) throws DataSourceException;

    Object getObject(int column) throws DataSourceException;
}
