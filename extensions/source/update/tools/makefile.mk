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

PRJ = ..$/..$/..
PRJNAME = extensions
TARGET = ztool
LIBTARGET = NO
TARGETTYPE = CUI
ENABLE_EXCEPTIONS=TRUE

.INCLUDE: settings.mk

OBJFILES = \
    $(OBJ)$/ztool.obj

APP1TARGET = ztool
APP1OBJS = $(OBJ)$/ztool.obj
APP1STDLIBS = $(TOOLSLIB)

#APP1TARGET = ztool
#APP1OBJS = $(OBJ)$/ztool.obj
#.IF "$(GUI)" == "UNX"
#APP1STDLIBS = $(TOOLSLIB)
#.ELSE
#APP1LIBS = $(LB)$/itools.lib
#.ENDIF

#APP2TARGET = urltest
#APP2OBJS = $(OBJ)$/urltest.obj
#.IF "$(GUI)" == "UNX"
#APP2STDLIBS = $(TOOLSLIB)  $(SALLIB) $(CPPULIB) $(CPPUHELPERLIB)
#.ELSE
#APP2STDLIBS = $(LB)$/itools.lib isal.lib icppu.lib icppuhelper.lib
#.ENDIF

# APP3TARGET = tldem
# APP3OBJS = $(OBJ)$/tldem.obj
# .IF "$(GUI)" == "UNX"
# APP3STDLIBS = $(TOOLSLIB)
# .ELSE
# APP3STDLIBS = $(LB)$/itools.lib
# .ENDIF

.INCLUDE: target.mk
