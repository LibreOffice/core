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

PROJECTPCH=sd
PROJECTPCHSOURCE=$(PRJ)$/util$/sd
PRJNAME=sd
TARGET=unoidl
ENABLE_EXCEPTIONS=TRUE
AUTOSEG=true
LIBTARGET = NO
PRJINC=..$/slidesorter

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

.IF "$(COM)"=="GCC"
NOOPTFILES= $(SLO)$/unowcntr.obj
.ENDIF

SLO1FILES =      \
        $(SLO)$/DrawController.obj \
        $(SLO)$/SdUnoSlideView.obj\
        $(SLO)$/SdUnoOutlineView.obj\
        $(SLO)$/SdUnoDrawView.obj \
        $(SLO)$/unopool.obj \
        $(SLO)$/UnoDocumentSettings.obj \
        $(SLO)$/facreg.obj \
        $(SLO)$/unomodel.obj    \
        $(SLO)$/unopage.obj     \
        $(SLO)$/unolayer.obj    \
        $(SLO)$/unocpres.obj    \
        $(SLO)$/unoobj.obj		\
        $(SLO)$/unosrch.obj		\
        $(SLO)$/unowcntr.obj	\
        $(SLO)$/unokywds.obj	\
        $(SLO)$/unopback.obj	\
        $(SLO)$/unodoc.obj      \
        $(SLO)$/unomodule.obj	\
        $(SLO)$/randomnode.obj

SLO2FILES = \
    $(SLO)$/sddetect.obj		\
    $(SLO)$/detreg.obj

SLOFILES = \
    $(SLO1FILES) \
    $(SLO2FILES)

LIB1TARGET = \
    $(SLB)$/$(TARGET).lib

LIB1OBJFILES = \
    $(SLO1FILES)


# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

