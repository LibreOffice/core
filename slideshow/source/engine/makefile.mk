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

PRJNAME=slideshow
TARGET=engine
ENABLE_EXCEPTIONS=TRUE


# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Common ----------------------------------------------------------

ENVCFLAGS += -DBOOST_SPIRIT_USE_OLD_NAMESPACE

.IF "$(OS)"=="SOLARIS"
.IF "$(CCNUMVER)"=="00050009"
# SunStudio12: anachronism warning in boost code (smilfunctionparser.cxx)
# reevaluate with newer boost or compiler version
CFLAGSWARNCXX!:=$(CFLAGSWARNCXX),badargtype2w
.ENDIF
.ENDIF

# Disable optimization for SunCC Sparc (funny loops
# when parsing e.g. "x+width/2")
# Do not disable optimization for SunCC++ 5.5 Solaris x86,
# this compiler has an ICE on smilfunctionparser.cxx *without*
# optimization
.IF "$(OS)$(CPU)"=="SOLARISS" && "$(COM)"!="GCC"
NOOPTFILES= $(SLO)$/smilfunctionparser.obj
.ENDIF
# same issue for MACOSX
.IF "$(OS)"=="MACOSX"
NOOPTFILES= $(SLO)$/smilfunctionparser.obj
.ENDIF

SLOFILES =	$(SLO)$/activitiesqueue.obj \
            $(SLO)$/animatedsprite.obj \
            $(SLO)$/animationfactory.obj \
            $(SLO)$/attributemap.obj \
            $(SLO)$/color.obj \
            $(SLO)$/delayevent.obj \
            $(SLO)$/effectrewinder.obj \
            $(SLO)$/eventmultiplexer.obj \
            $(SLO)$/eventqueue.obj \
            $(SLO)$/expressionnodefactory.obj \
            $(SLO)$/rehearsetimingsactivity.obj \
            $(SLO)$/screenupdater.obj \
            $(SLO)$/shapeattributelayer.obj \
            $(SLO)$/shapesubset.obj \
            $(SLO)$/slidebitmap.obj \
            $(SLO)$/slideshowcontext.obj \
            $(SLO)$/slideshowimpl.obj \
            $(SLO)$/slideview.obj \
            $(SLO)$/smilfunctionparser.obj \
            $(SLO)$/soundplayer.obj \
            $(SLO)$/tools.obj \
            $(SLO)$/unoviewcontainer.obj \
            $(SLO)$/usereventqueue.obj \
            $(SLO)$/waitsymbol.obj \
            $(SLO)$/wakeupevent.obj \
            $(SLO)$/debug.obj

.IF "$(debug)"!="" || "$(DEBUG)"!=""
SLOFILES +=  $(SLO)$/sp_debug.obj
.ENDIF

# ==========================================================================

.INCLUDE :	target.mk
