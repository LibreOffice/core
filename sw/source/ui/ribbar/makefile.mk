#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 10:45:41 $
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

PRJ=..$/..$/..

PRJNAME=sw
TARGET=ribbar

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =  \
        tbxanchr.src \
        inputwin.src \
        tblctrl.src \
        workctrl.src

SLOFILES =  \
        $(SLO)$/inputwin.obj \
        $(SLO)$/tbxanchr.obj \
        $(SLO)$/tbxmgr.obj \
        $(SLO)$/conrect.obj \
        $(SLO)$/conform.obj \
        $(SLO)$/conpoly.obj \
        $(SLO)$/conarc.obj \
        $(SLO)$/concustomshape.obj \
        $(SLO)$/dselect.obj \
        $(SLO)$/drawbase.obj \
        $(SLO)$/tblctrl.obj \
        $(SLO)$/workctrl.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

$(SRS)$/ribbar.srs: $(SOLARINCDIR)$/svx$/globlmn.hrc

