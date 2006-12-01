#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: rt $ $Date: 2006-12-01 17:13:50 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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


