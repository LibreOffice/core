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
# $Revision: 1.7 $
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

PRJNAME=idl
TARGET=idl

TARGETTYPE=CUI

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------------

LIB1TARGET= $(LB)$/$(TARGET).lib
LIB1FILES = $(LB)$/prj.lib		 \
            $(LB)$/objects.lib	 \
            $(LB)$/cmptools.lib

APP1TARGET= svidl

APP1STDLIBS=	\
    $(TOOLSLIB)	\
    $(SALLIB)

APP1LIBS= $(LB)$/$(TARGET).lib
.IF "$(GUI)" != "UNX"
.IF "$(COM)"!="GCC"
APP1OBJS=	$(OBJ)$/svidl.obj	\
            $(OBJ)$/command.obj
.ENDIF
.ENDIF

.IF "$(OS)"=="LINUX" || "$(OS)"=="FREEBSD"
APP1STDLIBS+=-lcrypt
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

