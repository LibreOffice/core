#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.21 $
#
#   last change: $Author: ihi $ $Date: 2007-11-21 15:15:59 $
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
    ColumnSettings.idl \
    CommandType.idl \
    Connection.idl \
    ContentLoader.idl \
    DataAccessDescriptor.idl \
    DatabaseAccess.idl \
    DatabaseAccessConnection.idl \
    DatabaseAccessContext.idl \
    DatabaseAccessDataSource.idl\
    DatabaseContext.idl \
    DatabaseDocument.idl \
    OfficeDatabaseDocument.idl \
    DatabaseEnvironment.idl \
    DataColumn.idl \
    DataSettings.idl \
    DataSource.idl \
    DatasourceAdministrationDialog.idl \
    DataSourceBrowser.idl \
    DefinitionContainer.idl \
    Document.idl \
    DocumentDefinition.idl \
    DocumentSaveRequest.idl \
    ErrorCondition.idl \
    ErrorMessageDialog.idl \
    InteractionHandler.idl \
    ParametersRequest.idl \
    PreparedStatement.idl \
    Query.idl \
    QueryDefinition.idl \
    QueryDescriptor.idl \
    QueryDesign.idl \
    ResultColumn.idl \
    ResultSet.idl \
    RowChangeAction.idl \
    RowChangeEvent.idl \
    RowSet.idl \
    RowSetVetoException.idl \
    SQLContext.idl \
    SQLErrorEvent.idl \
    SQLQueryComposer.idl \
    Table.idl \
    TableDesign.idl \
    QueryDesign.idl \
    RelationDesign.idl \
    TableDescriptor.idl \
    XAlterQuery.idl \
    XBookmarksSupplier.idl \
    XColumn.idl \
    XColumnUpdate.idl \
    XCommandPreparation.idl \
    XCompletedConnection.idl \
    XCompletedExecution.idl \
    XDatabaseAccess.idl \
    XDatabaseAccessListener.idl \
    XDatabaseEnvironment.idl \
    XFormDocumentsSupplier.idl \
    XInteractionDocumentSave.idl \
    XInteractionSupplyParameters.idl \
    XParametersSupplier.idl \
    XQueriesSupplier.idl \
    XQueryDefinitionsSupplier.idl \
    XReportDocumentsSupplier.idl \
    XResultSetAccess.idl \
    XRowSetApproveBroadcaster.idl \
    XRowSetApproveListener.idl \
    XRowSetSupplier.idl \
    XSQLErrorBroadcaster.idl \
    XSQLErrorListener.idl \
    XSQLQueryComposer.idl \
    XSingleSelectQueryComposer.idl \
    OrderColumn.idl \
    SingleSelectQueryAnalyzer.idl \
    SingleSelectQueryComposer.idl \
    SQLFilterOperator.idl \
    XSingleSelectQueryAnalyzer.idl \
    ColumnDescriptorControl.idl \
    ColumnDescriptorControlModel.idl \
    Forms.idl \
    Reports.idl \
    DocumentContainer.idl \
    XSQLQueryComposerFactory.idl \
    XDocumentDataSource.idl \
    DocumentDataSource.idl \
    XOfficeDatabaseDocument.idl \
    OfficeDatabaseDocument.idl

# ------------------------------------------------------------------
.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk


