#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.32 $
#
#   last change: $Author: ihi $ $Date: 2007-11-21 16:56:12 $
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

PRJ=..$/..

PRJNAME=connectivity
TARGET=commontools

# --- Settings -----------------------------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : settings.mk
# --- Files --------------------------------------------------------
EXCEPTIONSFILES=\
        $(SLO)$/predicateinput.obj						\
        $(SLO)$/ConnectionWrapper.obj					\
        $(SLO)$/TConnection.obj							\
        $(SLO)$/conncleanup.obj							\
        $(SLO)$/dbtools.obj								\
        $(SLO)$/dbtools2.obj							\
        $(SLO)$/dbexception.obj							\
        $(SLO)$/CommonTools.obj							\
        $(SLO)$/TColumnsHelper.obj						\
        $(SLO)$/TTableHelper.obj						\
        $(SLO)$/TKeys.obj								\
        $(SLO)$/TKey.obj								\
        $(SLO)$/TKeyColumns.obj							\
        $(SLO)$/TIndexes.obj							\
        $(SLO)$/TIndex.obj								\
        $(SLO)$/TIndexColumns.obj						\
        $(SLO)$/DateConversion.obj						\
        $(SLO)$/FDatabaseMetaDataResultSetMetaData.obj	\
        $(SLO)$/FDatabaseMetaDataResultSet.obj			\
        $(SLO)$/TDatabaseMetaDataBase.obj				\
        $(SLO)$/TPrivilegesResultSet.obj				\
        $(SLO)$/TSkipDeletedSet.obj                     \
        $(SLO)$/dbmetadata.obj                          \
        $(SLO)$/TSortIndex.obj                          \
        $(SLO)$/dbcharset.obj                           \
        $(SLO)$/propertyids.obj                         \
        $(SLO)$/FValue.obj                              \
        $(SLO)$/paramwrapper.obj                        \
        $(SLO)$/statementcomposer.obj                   \
        $(SLO)$/sqlerror.obj                            \
        $(SLO)$/formattedcolumnvalue.obj

SLOFILES=\
        $(EXCEPTIONSFILES)								\
        $(SLO)$/AutoRetrievingBase.obj					\
        $(SLO)$/dbconversion.obj




# --- Targets ------------------------------------------------------

.INCLUDE :      target.mk


