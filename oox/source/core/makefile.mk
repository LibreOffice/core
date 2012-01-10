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

PRJNAME=oox
TARGET=core
AUTOSEG=true

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

.IF "$(SYSTEM_OPENSSL)" == "YES"
CFLAGS+= $(OPENSSL_CFLAGS)
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/binarycodec.obj				\
        $(SLO)$/binaryfilterbase.obj		\
        $(SLO)$/contexthandler.obj			\
        $(SLO)$/contexthandler2.obj			\
        $(SLO)$/fastparser.obj				\
        $(SLO)$/fasttokenhandler.obj		\
        $(SLO)$/filterbase.obj				\
        $(SLO)$/filterdetect.obj			\
        $(SLO)$/fragmenthandler.obj			\
        $(SLO)$/fragmenthandler2.obj		\
        $(SLO)$/recordparser.obj			\
        $(SLO)$/relations.obj				\
        $(SLO)$/relationshandler.obj		\
        $(SLO)$/services.obj				\
        $(SLO)$/xmlfilterbase.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
