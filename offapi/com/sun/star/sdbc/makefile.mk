#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.13 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

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


