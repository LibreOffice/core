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

PRJ=..$/..$/..
PRJINC=$(PRJ)$/source
PRJNAME=dbaccess
TARGET=api

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/dba.pmk

# --- Files -------------------------------------
SLOFILES=	\
        $(SLO)$/querydescriptor.obj				\
        $(SLO)$/FilteredContainer.obj			\
        $(SLO)$/TableDeco.obj					\
        $(SLO)$/RowSetCacheIterator.obj			\
        $(SLO)$/RowSet.obj						\
        $(SLO)$/column.obj						\
        $(SLO)$/RowSetBase.obj					\
        $(SLO)$/CRowSetDataColumn.obj			\
        $(SLO)$/CRowSetColumn.obj				\
        $(SLO)$/querycomposer.obj				\
        $(SLO)$/CIndexes.obj					\
        $(SLO)$/BookmarkSet.obj					\
        $(SLO)$/KeySet.obj						\
        $(SLO)$/StaticSet.obj					\
        $(SLO)$/CacheSet.obj					\
        $(SLO)$/RowSetCache.obj					\
        $(SLO)$/definitioncolumn.obj			\
        $(SLO)$/resultcolumn.obj				\
        $(SLO)$/datacolumn.obj					\
        $(SLO)$/resultset.obj					\
        $(SLO)$/statement.obj					\
        $(SLO)$/preparedstatement.obj			\
        $(SLO)$/callablestatement.obj			\
        $(SLO)$/query.obj						\
        $(SLO)$/querycontainer.obj				\
        $(SLO)$/table.obj						\
        $(SLO)$/viewcontainer.obj				\
        $(SLO)$/tablecontainer.obj				\
        $(SLO)$/SingleSelectQueryComposer.obj	\
        $(SLO)$/HelperCollections.obj			\
        $(SLO)$/datasettings.obj                \
        $(SLO)$/View.obj						\
        $(SLO)$/WrappedResultSet.obj			\
        $(SLO)$/OptimisticSet.obj				\
        $(SLO)$/PrivateRow.obj					\
        $(SLO)$/columnsettings.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk

