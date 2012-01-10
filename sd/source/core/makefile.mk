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

PROJECTPCH=sd
PROJECTPCHSOURCE=$(PRJ)$/util$/sd
PRJNAME=sd
TARGET=core
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

IENV+=-I..\ui\inc

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES = glob.src

SLOFILES = $(SLO)$/stlsheet.obj  \
           $(SLO)$/stlfamily.obj \
           $(SLO)$/stlpool.obj	\
           $(SLO)$/drawdoc.obj \
           $(SLO)$/drawdoc2.obj \
           $(SLO)$/drawdoc3.obj \
           $(SLO)$/drawdoc4.obj \
           $(SLO)$/drawdoc_animations.obj\
           $(SLO)$/sdpage.obj \
           $(SLO)$/sdpage2.obj	\
           $(SLO)$/sdattr.obj \
           $(SLO)$/sdobjfac.obj \
           $(SLO)$/anminfo.obj	\
           $(SLO)$/sdiocmpt.obj	\
           $(SLO)$/typemap.obj	\
           $(SLO)$/pglink.obj   \
           $(SLO)$/cusshow.obj  \
           $(SLO)$/PageListWatcher.obj  \
           $(SLO)$/sdpage_animations.obj\
           $(SLO)$/CustomAnimationPreset.obj\
           $(SLO)$/CustomAnimationEffect.obj\
           $(SLO)$/TransitionPreset.obj\
           $(SLO)$/undoanim.obj\
           $(SLO)$/EffectMigration.obj\
           $(SLO)$/CustomAnimationCloner.obj\
           $(SLO)$/shapelist.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

