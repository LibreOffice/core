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

PRJ=..$/..$/..

PRJNAME=sc
TARGET=drawfunc

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =  \
        objdraw.src\
        drformsh.src

SLOFILES =  \
        $(SLO)$/fupoor.obj \
        $(SLO)$/fumark.obj \
        $(SLO)$/fudraw.obj \
        $(SLO)$/fusel.obj \
        $(SLO)$/fusel2.obj \
        $(SLO)$/fuconstr.obj \
        $(SLO)$/fuconrec.obj \
        $(SLO)$/fuconarc.obj \
        $(SLO)$/fuconuno.obj \
        $(SLO)$/fuconpol.obj \
        $(SLO)$/fuconcustomshape.obj \
        $(SLO)$/fuins1.obj \
        $(SLO)$/fuins2.obj \
        $(SLO)$/futext.obj \
        $(SLO)$/futext2.obj \
        $(SLO)$/futext3.obj \
        $(SLO)$/drawsh.obj \
        $(SLO)$/drawsh2.obj \
        $(SLO)$/drawsh4.obj \
        $(SLO)$/drawsh5.obj \
        $(SLO)$/drtxtob.obj \
        $(SLO)$/drtxtob1.obj \
        $(SLO)$/drtxtob2.obj \
        $(SLO)$/drformsh.obj \
        $(SLO)$/oleobjsh.obj \
        $(SLO)$/chartsh.obj  \
        $(SLO)$/graphsh.obj	\
        $(SLO)$/mediash.obj

EXCEPTIONSFILES= \
        $(SLO)$/fusel.obj \
        $(SLO)$/fuins2.obj \
        $(SLO)$/graphsh.obj	\
        $(SLO)$/mediash.obj

NOOPTFILES=\
    $(SLO)$/fusel.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

