#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: hr $ $Date: 2006-06-19 20:51:14 $
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
TARGET=svcontnr

PROJECTPCH4DLL=TRUE
PROJECTPCH=cont_pch
PROJECTPCHSOURCE=cont_pch

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/svt.pmk

# --- Files --------------------------------------------------------

EXCEPTIONSFILES=\
            $(SLO)$/contentenumeration.obj	\
            $(SLO)$/fileview.obj	\
            $(SLO)$/svlbox.obj		\
            $(SLO)$/svtabbx.obj		\
            $(SLO)$/svimpbox.obj	\
            $(SLO)$/templwin.obj

SLOFILES=   $(EXCEPTIONSFILES) \
            $(SLO)$/svicnvw.obj		\
            $(SLO)$/svimpicn.obj	\
            $(SLO)$/treelist.obj	\
            $(SLO)$/svlbitm.obj		\
            $(SLO)$/svtreebx.obj	\
            $(SLO)$/imivctl1.obj	\
            $(SLO)$/imivctl2.obj	\
            $(SLO)$/ivctrl.obj		\
            $(SLO)$/tooltiplbox.obj

SRS1NAME=$(TARGET)
SRC1FILES =\
            fileview.src	\
            templwin.src	\
            svcontnr.src

HXX1TARGET=	svcontnr
HXX1EXT=	hxx
HXX1FILES=	$(PRJ)$/inc$/svlbox.hxx \
        $(PRJ)$/inc$/svlbitm.hxx \
        $(PRJ)$/inc$/svtreebx.hxx \
        $(PRJ)$/inc$/svicnvw.hxx \
        $(PRJ)$/inc$/svtabbx.hxx \
        $(PRJ)$/inc$/treelist.hxx
HXX1EXCL=	-E:*include*

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

