#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: obo $ $Date: 2008-02-26 14:25:52 $
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
TARGET=uiview

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =  \
        view.src \
        pview.src

EXCEPTIONSFILES= \
        $(SLO)$/formatclipboard.obj \
        $(SLO)$/srcview.obj \
        $(SLO)$/swcli.obj  \
        $(SLO)$/uivwimp.obj \
        $(SLO)$/view.obj \
        $(SLO)$/view2.obj \
        $(SLO)$/viewling.obj

SLOFILES =  \
        $(SLO)$/view0.obj \
        $(SLO)$/view.obj \
        $(SLO)$/view1.obj \
        $(SLO)$/view2.obj \
        $(SLO)$/viewcoll.obj \
        $(SLO)$/viewdlg2.obj \
        $(SLO)$/viewdlg.obj \
        $(SLO)$/viewdraw.obj \
        $(SLO)$/viewling.obj \
        $(SLO)$/viewmdi.obj \
        $(SLO)$/pview.obj \
        $(SLO)$/viewport.obj \
        $(SLO)$/viewstat.obj \
        $(SLO)$/viewtab.obj \
        $(SLO)$/viewprt.obj \
        $(SLO)$/viewsrch.obj \
        $(SLO)$/scroll.obj \
        $(SLO)$/swcli.obj  \
        $(SLO)$/srcview.obj \
        $(SLO)$/uivwimp.obj \
        $(SLO)$/formatclipboard.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

$(SRS)$/uiview.srs: $(SOLARINCDIR)$/svx$/globlmn.hrc

