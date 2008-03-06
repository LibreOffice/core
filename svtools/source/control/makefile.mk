#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.18 $
#
#   last change: $Author: kz $ $Date: 2008-03-06 19:24:39 $
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

PRJNAME=svtools
TARGET=ctrl

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/util$/svt.pmk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES=\
        ctrltool.src \
        ctrlbox.src \
        calendar.src \
        filectrl.src

EXCEPTIONSFILES=\
        $(SLO)$/filectrl2.obj \
        $(SLO)$/roadmap.obj \
        $(SLO)$/scriptedtext.obj\
        $(SLO)$/fmtfield.obj \
        $(SLO)$/inettbc.obj \
        $(SLO)$/valueacc.obj

SLOFILES=\
      $(EXCEPTIONSFILES) \
        $(SLO)$/asynclink.obj \
        $(SLO)$/urlcontrol.obj \
        $(SLO)$/fileurlbox.obj \
        $(SLO)$/ctrltool.obj  \
        $(SLO)$/ctrlbox.obj  \
        $(SLO)$/stdctrl.obj  \
        $(SLO)$/stdmenu.obj  \
        $(SLO)$/valueset.obj \
        $(SLO)$/tabbar.obj       \
        $(SLO)$/headbar.obj  \
        $(SLO)$/prgsbar.obj  \
        $(SLO)$/ruler.obj        \
        $(SLO)$/taskbar.obj  \
        $(SLO)$/taskbox.obj  \
        $(SLO)$/taskstat.obj \
        $(SLO)$/taskmisc.obj \
        $(SLO)$/calendar.obj \
        $(SLO)$/filectrl.obj \
        $(SLO)$/scrwin.obj       \
        $(SLO)$/reginfo.obj     \
        $(SLO)$/collatorres.obj \
        $(SLO)$/indexentryres.obj \
        $(SLO)$/hyperlabel.obj  \
        $(SLO)$/fixedhyper.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

