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

