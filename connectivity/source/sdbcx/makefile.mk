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
PRJINC=..
PRJNAME=connectivity
TARGET=sdbcx

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/dbtools.pmk

# --- Files -------------------------------------

SLOFILES=\
        $(SLO)$/VDescriptor.obj					\
        $(SLO)$/VCollection.obj					\
        $(SLO)$/VColumn.obj						\
        $(SLO)$/VIndexColumn.obj				\
        $(SLO)$/VKeyColumn.obj					\
        $(SLO)$/VUser.obj						\
        $(SLO)$/VGroup.obj						\
        $(SLO)$/VTable.obj						\
        $(SLO)$/VKey.obj						\
        $(SLO)$/VIndex.obj						\
        $(SLO)$/VCatalog.obj					\
        $(SLO)$/VView.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk


