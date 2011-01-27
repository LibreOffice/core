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

PRJ = ..
PRJNAME = tl
TARGET = tldem
LIBTARGET = NO
TARGETTYPE = CUI
ENABLE_EXCEPTIONS=TRUE

.INCLUDE: settings.mk

OBJFILES = \
    $(OBJ)$/solar.obj \
    $(OBJ)$/urltest.obj \
    $(OBJ)$/inetmimetest.obj
#	$(OBJ)$/demostor.obj \
#	$(OBJ)$/fstest.obj \
#	$(OBJ)$/tldem.obj \

APP1TARGET = solar
APP1OBJS = $(OBJ)$/solar.obj
.IF "$(GUI)" == "UNX" || "$(GUI)" == "OS2"
APP1STDLIBS = $(TOOLSLIB)
.ELSE
APP1LIBS = $(LB)$/itools.lib
.ENDIF

APP2TARGET = urltest
APP2OBJS = $(OBJ)$/urltest.obj
.IF "$(GUI)" == "UNX" || "$(GUI)" == "OS2"
APP2STDLIBS = $(TOOLSLIB)  $(SALLIB) $(CPPULIB) $(CPPUHELPERLIB)
.ELSE
APP2STDLIBS = $(LB)$/itools.lib  $(SALLIB) $(CPPULIB) $(CPPUHELPERLIB)
.ENDIF

APP3TARGET = inetmimetest
APP3OBJS = $(OBJ)$/inetmimetest.obj
APP3STDLIBS = $(SALLIB) $(TOOLSLIB)

# APP3TARGET = tldem
# APP3OBJS = $(OBJ)$/tldem.obj
# .IF "$(GUI)" == "UNX"
# APP3STDLIBS = $(TOOLSLIB)
# .ELSE
# APP3STDLIBS = $(LB)$/itools.lib
# .ENDIF

# APP4TARGET = demostor
# APP4OBJS = $(OBJ)$/demostor.obj
# .IF "$(GUI)" == "UNX"
# APP4STDLIBS = $(TOOLSLIB)  $(SALLIB)
# .ELSE
# APP4STDLIBS = $(LB)$/itools.lib  $(SALLIB)
# .ENDIF

# APP5TARGET = fstest
# APP5OBJS = $(OBJ)$/fstest.obj
# .IF "$(GUI)" == "UNX"
# APP5STDLIBS = $(TOOLSLIB)  $(SALLIB)
# .ELSE
# APP5STDLIBS = $(LB)$/itools.lib  $(SALLIB)
# .ENDIF

.INCLUDE: target.mk
