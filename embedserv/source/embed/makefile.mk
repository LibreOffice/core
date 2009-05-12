#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.9 $
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

#MKDEPENDSOLVER=YES

PRJ=..$/..

PRJNAME=embedserv
TARGET=emser


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
#.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(DISABLE_ATL)"==""

LIBTARGET=NO
USE_DEFFILE=NO

INCPRE+= $(ATL_INCLUDE) 

# --- Files --------------------------------------------------------

SLOFILES =  \
        $(SLO)$/register.obj       \
        $(SLO)$/servprov.obj       \
        $(SLO)$/docholder.obj      \
        $(SLO)$/ed_ipersiststr.obj \
        $(SLO)$/ed_idataobj.obj    \
        $(SLO)$/ed_ioleobject.obj  \
        $(SLO)$/ed_iinplace.obj    \
        $(SLO)$/iipaobj.obj        \
        $(SLO)$/guid.obj           \
        $(SLO)$/esdll.obj          \
        $(SLO)$/intercept.obj      \
        $(SLO)$/syswinwrapper.obj  \
        $(SLO)$/tracker.obj

EXCEPTIONSFILES= \
        $(SLO)$/register.obj       \
        $(SLO)$/docholder.obj      \
        $(SLO)$/ed_ipersiststr.obj \
        $(SLO)$/ed_idataobj.obj    \
        $(SLO)$/ed_iinplace.obj    \
        $(SLO)$/ed_ioleobject.obj  \
        $(SLO)$/iipaobj.obj        \
        $(SLO)$/intercept.obj      \
        $(SLO)$/syswinwrapper.obj  \
        $(SLO)$/tracker.obj

.ENDIF
# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

