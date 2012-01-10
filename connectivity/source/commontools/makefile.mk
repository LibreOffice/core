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


