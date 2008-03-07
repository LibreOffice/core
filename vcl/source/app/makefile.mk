#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.22 $
#
#   last change: $Author: kz $ $Date: 2008-03-07 16:40:53 $
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

PRJNAME=vcl
TARGET=app
ENABLE_EXCEPTIONS=TRUE

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile2.pmk

CDEFS+=-DDLLPOSTFIX=$(DLLPOSTFIX)

# --- Files --------------------------------------------------------

SLOFILES=	$(SLO)$/dbggui.obj		\
            $(SLO)$/help.obj		\
            $(SLO)$/idlemgr.obj 	\
            $(SLO)$/settings.obj	\
            $(SLO)$/sound.obj		\
            $(SLO)$/stdtext.obj 	\
            $(SLO)$/svapp.obj		\
            $(SLO)$/svdata.obj		\
            $(SLO)$/svmain.obj		\
            $(SLO)$/svmainhook.obj	\
            $(SLO)$/timer.obj		\
            $(SLO)$/dndhelp.obj     \
            $(SLO)$/unohelp.obj     \
            $(SLO)$/unohelp2.obj    \
            $(SLO)$/vclevent.obj	\
            $(SLO)$/i18nhelp.obj	\
            $(SLO)$/salvtables.obj	\
            $(SLO)$/session.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.INCLUDE :	$(PRJ)$/util$/target.pmk

