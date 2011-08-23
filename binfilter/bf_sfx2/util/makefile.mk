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

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

BFPRJ=..
PRJ=..$/..

ENABLE_EXCEPTIONS=TRUE

PRJNAME=binfilter
TARGET=bf_sfx

NO_HIDS=TRUE

.INCLUDE :  $(BFPRJ)$/util$/makefile.pmk

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

INC+= -I$(PRJ)$/inc$/bf_sfx2
# --- Allgemein ----------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1ARCHIV=$(LB)$/lib$(TARGET).a
LIB1FILES=  $(SLB)$/sfx2_appl.lib		\
            $(SLB)$/sfx2_explorer.lib	\
            $(SLB)$/sfx2_doc.lib			\
            $(SLB)$/sfx2_view.lib		\
            $(SLB)$/sfx2_control.lib		\
            $(SLB)$/sfx2_notify.lib		\
            $(SLB)$/sfx2_bastyp.lib		\
            $(SLB)$/sfx2_config.lib

# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk


## --- SFX-Filter-Datei ---
#
#$(MISC)$/$(SHL1TARGET).flt: makefile.mk
#    @echo ------------------------------
#    @echo Making: $@
#    @$(TYPE) sfxwin.flt > $@


