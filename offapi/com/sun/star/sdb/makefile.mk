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


