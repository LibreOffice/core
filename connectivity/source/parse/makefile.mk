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
TARGET=sql
INCPRE=$(MISC)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/dbtools.pmk

# --- Files --------------------------------------------------------

YACCTARGET= $(MISC)$/sqlbison.cxx
YACCFILES= sqlbison.y
YACCFLAGS=-v -d -l -pSQLyy -bsql

EXCEPTIONSFILES =  \
        $(SLO)$/PColumn.obj			\
        $(SLO)$/sqliterator.obj		\
        $(SLO)$/sqlnode.obj         \
        $(SLO)$/wrap_sqlbison.obj		\
        $(SLO)$/internalnode.obj

SLOFILES =  \
        $(EXCEPTIONSFILES)			\
        $(SLO)$/wrap_sqlflex.obj


# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/%.cxx:	%.l
    flex -i -8 -PSQLyy -L -o$(MISC)$/sqlflex.cxx sqlflex.l

$(INCCOM)$/sqlbison.hxx : $(YACCTARGET)
$(EXCEPTIONSFILES) : $(INCCOM)$/sqlbison.hxx
$(SLO)$/wrap_sqlbison.obj : $(YACCTARGET)
$(SLO)$/wrap_sqlflex.obj : $(MISC)$/sqlflex.cxx $(INCCOM)$/sqlbison.hxx
