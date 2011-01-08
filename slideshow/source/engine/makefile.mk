#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..$/..

PRJNAME=slideshow
TARGET=engine
ENABLE_EXCEPTIONS=TRUE


# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

.IF "$(ENABLE_PRESENTER_EXTRA_UI)"=="YES"
ENABLE_PRESENTER_EXTRA_UI is not used anymore
.ENDIF

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
