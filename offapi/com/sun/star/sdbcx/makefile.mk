#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: ihi $ $Date: 2007-11-21 15:17:37 $
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


