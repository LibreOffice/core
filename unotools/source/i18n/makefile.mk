#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.20 $
#
#   last change: $Author: vg $ $Date: 2006-05-24 14:01:53 $
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
PRJINC=..$/..$/inc
PRJNAME=unotools
TARGET=i18n

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE :      $(PRJ)$/util$/makefile.pmk

# --- Files -------------------------------------

SLOFILES=	\
    $(SLO)$/charclass.obj	\
    $(SLO)$/calendarwrapper.obj	\
    $(SLO)$/collatorwrapper.obj	\
    $(SLO)$/intlwrapper.obj \
    $(SLO)$/localedatawrapper.obj	\
    $(SLO)$/nativenumberwrapper.obj	\
    $(SLO)$/numberformatcodewrapper.obj \
    $(SLO)$/readwritemutexguard.obj \
    $(SLO)$/transliterationwrapper.obj \
    $(SLO)$/textsearch.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk

