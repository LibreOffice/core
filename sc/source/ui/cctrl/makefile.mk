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

PRJNAME=sc
TARGET=cctrl
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

EXCEPTIONSFILES= \
    $(SLO)$/tbinsert.obj	\
    $(SLO)$/tbzoomsliderctrl.obj \
    $(SLO)$/dpcontrol.obj

SLOFILES =	\
        $(SLO)$/popmenu.obj		\
        $(SLO)$/tbinsert.obj	\
        $(SLO)$/cbuttonw.obj	\
        $(SLO)$/dpcontrol.obj	\
        $(SLO)$/editfield.obj	\
        $(EXCEPTIONSFILES)

SRS1NAME=$(TARGET)
SRC1FILES = \
    dpcontrol.src

LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1OBJFILES= \
        $(SLO)$/popmenu.obj		\
        $(SLO)$/tbinsert.obj	\
        $(SLO)$/cbuttonw.obj	\
        $(SLO)$/dpcontrol.obj	\
        $(SLO)$/tbzoomsliderctrl.obj


# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk
