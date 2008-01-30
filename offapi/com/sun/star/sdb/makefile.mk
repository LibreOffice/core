#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.22 $
#
#   last change: $Author: rt $ $Date: 2008-01-30 08:15:39 $
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
    DataColumn.idl \
    DataSettings.idl \
    DataSource.idl \
    DatasourceAdministrationDialog.idl \
    DataSourceBrowser.idl \
    DefinitionContainer.idl \
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
    XRowSetSupplier.idl \
    XSingleSelectQueryAnalyzer.idl \
    XSingleSelectQueryComposer.idl \
    XSQLErrorBroadcaster.idl \
    XSQLErrorListener.idl \
    XSQLQueryComposer.idl \
    XSQLQueryComposerFactory.idl

# ------------------------------------------------------------------
.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk


