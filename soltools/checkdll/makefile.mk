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
TARGET=checkdll
TARGETTYPE=CUI
NO_DEFAULT_STL=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : $(PRJ)$/util$/makefile.pmk
.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------


.IF "$(GUI)"=="UNX"
LIBSALCPPRT=$(0)
APP1TARGET	= 	checkdll
APP1OBJS	=	$(OBJ)$/checkdll.obj
DEPOBJFILES	=	$(APP1OBJS) 
.IF "$(OS)"!="FREEBSD" && "$(OS)"!="MACOSX" && "$(OS)"!="NETBSD" \
	&& "$(OS)"!="OPENBSD" && "$(OS)"!="DRAGONFLY"
STDLIB += -ldl
.ENDIF
.IF "$(OS)"=="NETBSD"
APP1STDLIBS	+= -Wl,--whole-archive -lgcc -Wl,--no-whole-archive
.ENDIF
.ENDIF # "$(GUI)"=="UNX"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
