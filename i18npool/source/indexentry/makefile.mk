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


PRJNAME=i18npool
TARGET=indexentry

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk


# --- Files --------------------------------------------------------

SLOFILES=   \
            $(SLO)$/indexentrysupplier.obj \
            $(SLO)$/indexentrysupplier_asian.obj \
            $(SLO)$/indexentrysupplier_ja_phonetic.obj \
            $(SLO)$/indexentrysupplier_default.obj \
            $(SLO)$/indexentrysupplier_common.obj

OBJFILES   = $(OBJ)$/genindex_data.obj

APP1TARGET = genindex_data
APP1RPATH = NONE

DEPOBJFILES   = $(OBJ)$/genindex_data.obj
APP1OBJS   = $(DEPOBJFILES)

APP1STDLIBS = $(SALLIB) \
        $(ICUINLIB) \
        $(ICUUCLIB) \
        $(ICUDATALIB)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

