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

PRJNAME=slideshow
TARGET=animationnodes
ENABLE_EXCEPTIONS=TRUE
PRJINC=..$/..

# --- Settings -----------------------------------------------------------

.INCLUDE :      settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Common ----------------------------------------------------------

SLOFILES = $(SLO)$/animationaudionode.obj \
           $(SLO)$/animationcommandnode.obj \
           $(SLO)$/animationbasenode.obj \
           $(SLO)$/animationcolornode.obj \
           $(SLO)$/animationnodefactory.obj \
           $(SLO)$/animationpathmotionnode.obj \
           $(SLO)$/animationsetnode.obj \
           $(SLO)$/animationtransformnode.obj \
           $(SLO)$/animationtransitionfilternode.obj \
           $(SLO)$/basecontainernode.obj \
           $(SLO)$/basenode.obj \
           $(SLO)$/nodetools.obj \
           $(SLO)$/paralleltimecontainer.obj \
           $(SLO)$/propertyanimationnode.obj \
           $(SLO)$/sequentialtimecontainer.obj \
           $(SLO)$/generateevent.obj

# ==========================================================================

.INCLUDE :      target.mk
