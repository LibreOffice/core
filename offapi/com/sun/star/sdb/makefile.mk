#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

TARGET=csssdb
PACKAGE=com$/sun$/star$/sdb

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk


UNOIDLDEFS += -w

# ------------------------------------------------------------------------
IDLFILES=\
    BooleanComparisonMode.idl \
    CallableStatement.idl \
    Column.idl \
    ColumnDescriptorControl.idl \
    ColumnDescriptorControlModel.idl \
    ColumnSettings.idl \
    CommandType.idl \
    Connection.idl \
    ContentLoader.idl \
    DataAccessDescriptor.idl \
    DataAccessDescriptorFactory.idl \
    DatabaseAccess.idl \
    DatabaseAccessConnection.idl \
    DatabaseAccessContext.idl \
    DatabaseAccessDataSource.idl\
    DatabaseContext.idl \
    DatabaseDocument.idl \
    DatabaseEnvironment.idl \
    DatabaseInteractionHandler.idl \
    DatabaseRegistrationEvent.idl \
    DataColumn.idl \
    DataSettings.idl \
    DataSource.idl \
    DatasourceAdministrationDialog.idl \
    DataSourceBrowser.idl \
    DefinitionContainer.idl \
    DefinitionContent.idl \
    Document.idl \
    DocumentContainer.idl \
    DocumentDataSource.idl \
    DocumentDefinition.idl \
    DocumentSaveRequest.idl \
    ErrorCondition.idl \
    ErrorMessageDialog.idl \
    Forms.idl \
    InteractionHandler.idl \
    OfficeDatabaseDocument.idl \
    OrderColumn.idl \
    ParametersRequest.idl \
    PreparedStatement.idl \
    Query.idl \
    QueryDefinition.idl \
    QueryDescriptor.idl \
    QueryDesign.idl \
    RelationDesign.idl \
    Reports.idl \
    ResultColumn.idl \
    ResultSet.idl \
    RowChangeAction.idl \
    RowChangeEvent.idl \
    RowsChangeEvent.idl \
    RowSet.idl \
    RowSetVetoException.idl \
    SingleSelectQueryAnalyzer.idl \
    SingleSelectQueryComposer.idl \
    SQLContext.idl \
    SQLErrorEvent.idl \
    SQLFilterOperator.idl \
    SQLQueryComposer.idl \
    Table.idl \
    TableDescriptor.idl \
    TableDesign.idl \
    XAlterQuery.idl \
    XBookmarksSupplier.idl \
    XColumn.idl \
    XColumnUpdate.idl \
    XCommandPreparation.idl \
    XCompletedConnection.idl \
    XCompletedExecution.idl \
    XDataAccessDescriptorFactory.idl \
    XDatabaseAccess.idl \
    XDatabaseAccessListener.idl \
    XDatabaseRegistrations.idl \
    XDatabaseRegistrationsListener.idl \
    XDatabaseEnvironment.idl \
    XDocumentDataSource.idl \
    XFormDocumentsSupplier.idl \
    XInteractionDocumentSave.idl \
    XInteractionSupplyParameters.idl \
    XOfficeDatabaseDocument.idl \
    XParametersSupplier.idl \
    XQueriesSupplier.idl \
    XQueryDefinitionsSupplier.idl \
    XReportDocumentsSupplier.idl \
    XResultSetAccess.idl \
    XRowSetApproveBroadcaster.idl \
    XRowSetApproveListener.idl \
    XRowSetChangeBroadcaster.idl \
    XRowSetChangeListener.idl \
    XRowsChangeBroadcaster.idl \
    XRowsChangeListener.idl \
    XRowSetSupplier.idl \
    XSingleSelectQueryAnalyzer.idl \
    XSingleSelectQueryComposer.idl \
    XSubDocument.idl \
    XSQLErrorBroadcaster.idl \
    XSQLErrorListener.idl \
    XSQLQueryComposer.idl \
    XSQLQueryComposerFactory.idl

# ------------------------------------------------------------------
.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk


