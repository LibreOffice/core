#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: kz $ $Date: 2008-04-03 15:43:07 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..$/..

PRJNAME=slideshow
TARGET=engine
ENABLE_EXCEPTIONS=TRUE


# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Common ----------------------------------------------------------

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
            $(SLO)$/wakeupevent.obj

.IF "$(debug)"!="" || "$(DEBUG)"!=""
SLOFILES +=  $(SLO)$/sp_debug.obj
.ENDIF

# ==========================================================================

.INCLUDE :	target.mk
