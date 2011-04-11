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

PRJ=..$/..

PRJNAME=connectivity
TARGET=commontools

# --- Settings -----------------------------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/dbtools.pmk

# Disable optimization for SunCC SPARC and MACOSX (funny loops
# when parsing e.g. "x+width/2"),
# also http://gcc.gnu.org/PR22392
.IF ("$(OS)$(CPU)"=="SOLARISS" && "$(COM)"!="GCC") || "$(OS)"=="MACOSX" || ("$(OS)"=="LINUX" && "$(CPU)"=="P") 
NOOPTFILES= $(SLO)$/RowFunctionParser.obj
.ENDIF

ENVCFLAGS += -DBOOST_SPIRIT_USE_OLD_NAMESPACE

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
        $(SLO)$/RowFunctionParser.obj                   \
        $(SLO)$/sqlerror.obj                            \
        $(SLO)$/filtermanager.obj                       \
        $(SLO)$/parameters.obj							\
        $(SLO)$/ParamterSubstitution.obj                \
        $(SLO)$/DriversConfig.obj                       \
        $(SLO)$/formattedcolumnvalue.obj                \
        $(SLO)$/BlobHelper.obj							\
        $(SLO)$/warningscontainer.obj

SLOFILES=\
        $(EXCEPTIONSFILES)								\
        $(SLO)$/AutoRetrievingBase.obj					\
        $(SLO)$/dbconversion.obj




# --- Targets ------------------------------------------------------

.INCLUDE :      target.mk


