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

PRJ=..

PRJNAME=soltools
TARGET=adjustvisibility
TARGETTYPE=CUI
TARGETPLATFORM=BUILD
ENABLE_EXCEPTIONS=TRUE
noadjust=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : $(PRJ)$/util$/makefile.pmk
.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

# This tool is for Solaris only
.IF "$(OS)"=="SOLARIS"

APP1TARGET	= 	adjustvisibility
APP1OBJS	=	$(OBJ)$/adjustvisibility.obj
DEPOBJFILES	=	$(APP1OBJ)
APP1STDLIBS =   -lelf
APP1RPATH   =   NONE

#APP1STDLIBS+=-lstlport
APP1STDLIBS+=-lCstd

.ENDIF "$(OS)"=="SOLARIS"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
