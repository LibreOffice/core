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
# $Revision: 1.9 $
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


