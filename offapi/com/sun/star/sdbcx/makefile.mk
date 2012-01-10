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

TARGET=csssdbcx
PACKAGE=com$/sun$/star$/sdbcx

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------
IDLFILES=\
    CheckOption.idl \
    CompareBookmark.idl \
    Column.idl \
    ColumnDescriptor.idl \
    Container.idl \
    DatabaseDefinition.idl \
    Descriptor.idl \
    Driver.idl \
    Group.idl \
    GroupDescriptor.idl \
    Index.idl \
    IndexColumn.idl \
    IndexColumnDescriptor.idl \
    IndexDescriptor.idl \
    Key.idl \
    KeyColumn.idl \
    KeyDescriptor.idl \
    KeyColumnDescriptor.idl \
    KeyDescriptor.idl \
    KeyType.idl \
    PreparedStatement.idl \
    Privilege.idl \
    PrivilegeObject.idl \
    ReferenceColumn.idl \
    ResultSet.idl \
    Statement.idl \
    Table.idl \
    TableDescriptor.idl \
    User.idl \
    UserDescriptor.idl \
    View.idl \
    ViewDescriptor.idl \
    XAlterTable.idl \
    XAlterView.idl \
    XAppend.idl \
    XAuthorizable.idl \
    XColumnsSupplier.idl \
    XCreateCatalog.idl \
    XDeleteRows.idl \
    XDataDefinitionSupplier.idl \
    XDataDescriptorFactory.idl \
    XDrop.idl \
    XDropCatalog.idl \
    XGroupsSupplier.idl \
    XIndexesSupplier.idl \
    XKeysSupplier.idl \
    XRename.idl \
    XRowLocate.idl \
    XTablesSupplier.idl \
    XUser.idl \
    XUsersSupplier.idl \
    XViewsSupplier.idl \

# ------------------------------------------------------------------
.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk


