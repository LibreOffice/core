#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2004-11-26 18:54:38 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..

PRJNAME=slideshow
TARGET=engine
ENABLE_EXCEPTIONS=TRUE


# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk

# --- Common ----------------------------------------------------------

.IF "$(verbose)"!="" || "$(VERBOSE)"!=""
CDEFS+= -DVERBOSE
.ENDIF

.IF "$(debug)"!="" || "$(DEBUG)"!=""
CDEFS+= -DBOOST_SP_ENABLE_DEBUG_HOOKS
.ENDIF

# Disable optimization for SunCC (funny loops
# when parsing e.g. "x+width/2")
.IF "$(OS)"=="SOLARIS" && "$(COM)"!="GCC"
NOOPTFILES= $(SLO)$/smilfunctionparser.obj
.ENDIF

SLOFILES =	$(SLO)$/activitiesfactory.obj \
            $(SLO)$/activitiesqueue.obj \
            $(SLO)$/animatedsprite.obj \
            $(SLO)$/animationfactory.obj \
            $(SLO)$/animationnodefactory.obj \
            $(SLO)$/attributemap.obj \
            $(SLO)$/backgroundshape.obj \
            $(SLO)$/color.obj \
            $(SLO)$/doctreenode.obj \
            $(SLO)$/drawshape.obj \
            $(SLO)$/eventmultiplexer.obj \
            $(SLO)$/eventqueue.obj \
            $(SLO)$/expressionnodefactory.obj \
            $(SLO)$/gdimtftools.obj \
            $(SLO)$/intrinsicanimationactivity.obj \
            $(SLO)$/layer.obj \
            $(SLO)$/layermanager.obj \
            $(SLO)$/mediashape.obj \
            $(SLO)$/presentation.obj \
            $(SLO)$/shapeattributelayer.obj \
            $(SLO)$/shapeeventbroadcaster.obj \
            $(SLO)$/shapeimporter.obj \
            $(SLO)$/shapesubset.obj \
            $(SLO)$/slide.obj \
            $(SLO)$/slideanimations.obj \
            $(SLO)$/slidebitmap.obj \
            $(SLO)$/smilfunctionparser.obj \
            $(SLO)$/soundplayer.obj \
            $(SLO)$/tools.obj \
            $(SLO)$/unoviewcontainer.obj \
            $(SLO)$/usereventqueue.obj \
            $(SLO)$/userpaintoverlay.obj \
            $(SLO)$/viewbackgroundshape.obj \
            $(SLO)$/viewmediashape.obj \
            $(SLO)$/viewshape.obj \
            $(SLO)$/wakeupevent.obj


# ==========================================================================

.INCLUDE :	target.mk
