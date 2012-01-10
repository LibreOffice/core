#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



PRJ=..$/..$/..$/..

PRJNAME=offapi

TARGET=csssdbc
PACKAGE=com$/sun$/star$/sdbc

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------
IDLFILES=\
    BatchUpdateException.idl \
    BestRowScope.idl \
    BestRowType.idl \
    CallableStatement.idl \
    ChangeAction.idl \
    ChangeEvent.idl \
    ColumnSearch.idl \
    ColumnType.idl \
    ColumnValue.idl \
    Connection.idl \
    ConnectionPool.idl \
    ConnectionProperties.idl \
    DataTruncation.idl \
    DataType.idl \
    DBASEConnectionProperties.idl \
    Deferrability.idl \
    Driver.idl \
    DriverManager.idl \
    DriverPropertyInfo.idl \
    FetchDirection.idl \
    FILEConnectionProperties.idl \
    FLATConnectionProperties.idl \
    IndexType.idl \
    JDBCConnectionProperties.idl \
    KeyRule.idl \
    ODBCConnectionProperties.idl \
    PreparedStatement.idl \
    ProcedureColumn.idl \
    ProcedureResult.idl \
    ResultSet.idl \
    ResultSetConcurrency.idl \
    ResultSetType.idl \
    RowSet.idl \
    SQLException.idl \
    SQLWarning.idl \
    Statement.idl \
    TransactionIsolation.idl \
    XArray.idl \
    XBatchExecution.idl \
    XBlob.idl \
    XClob.idl \
    XCloseable.idl \
    XColumnLocate.idl \
    XConnection.idl \
    XDatabaseMetaData.idl \
    XDatabaseMetaData2.idl \
    XDataSource.idl \
    XDriver.idl \
    XDriverAccess.idl \
    XDriverManager.idl \
    XGeneratedResultSet.idl \
    XIsolatedConnection.idl \
    XMultipleResults.idl \
    XOutParameters.idl \
    XParameters.idl \
    XPooledConnection.idl \
    XPreparedBatchExecution.idl \
    XPreparedStatement.idl \
    XRef.idl \
    XResultSet.idl \
    XResultSetMetaData.idl \
    XResultSetMetaDataSupplier.idl \
    XResultSetUpdate.idl \
    XRow.idl \
    XRowSet.idl \
    XRowSetListener.idl \
    XRowUpdate.idl \
    XSQLData.idl \
    XSQLInput.idl \
    XSQLOutput.idl \
    XStatement.idl \
    XStruct.idl \
    XWarningsSupplier.idl

# ------------------------------------------------------------------
.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk


