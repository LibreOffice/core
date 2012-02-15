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
TARGET=ppt
AUTOSEG=true

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/animationspersist.obj \
        $(SLO)$/animationtypes.obj \
        $(SLO)$/animvariantcontext.obj \
        $(SLO)$/backgroundproperties.obj\
        $(SLO)$/buildlistcontext.obj \
        $(SLO)$/commonbehaviorcontext.obj \
        $(SLO)$/commontimenodecontext.obj \
        $(SLO)$/conditioncontext.obj \
        $(SLO)$/customshowlistcontext.obj \
        $(SLO)$/headerfootercontext.obj \
        $(SLO)$/layoutfragmenthandler.obj\
        $(SLO)$/pptfilterhelpers.obj\
        $(SLO)$/pptimport.obj\
        $(SLO)$/pptshape.obj \
        $(SLO)$/pptshapecontext.obj \
        $(SLO)$/pptshapegroupcontext.obj \
        $(SLO)$/pptshapepropertiescontext.obj \
        $(SLO)$/presentationfragmenthandler.obj\
        $(SLO)$/slidefragmenthandler.obj\
        $(SLO)$/slidemastertextstylescontext.obj \
        $(SLO)$/slidepersist.obj\
        $(SLO)$/slidetimingcontext.obj\
        $(SLO)$/slidetransition.obj\
        $(SLO)$/slidetransitioncontext.obj\
        $(SLO)$/soundactioncontext.obj \
        $(SLO)$/timeanimvaluecontext.obj \
        $(SLO)$/timenode.obj\
        $(SLO)$/timenodelistcontext.obj \
        $(SLO)$/timetargetelementcontext.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
