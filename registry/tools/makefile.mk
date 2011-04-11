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

PRJNAME=registry
TARGET=regmerge
TARGETTYPE=CUI
LIBTARGET=NO

ENABLE_EXCEPTIONS := TRUE

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

.INCLUDE :  ..$/version.mk

# --- Files --------------------------------------------------------
CDEFS += -DDLL_VERSION=$(EMQ)"$(DLLPOSTFIX)$(EMQ)"

APP1TARGET= $(TARGET)
APP1OBJS=   $(OBJ)$/regmerge.obj $(OBJ)/fileurl.obj $(OBJ)/options.obj
APP1RPATH=  UREBIN

APP1STDLIBS=\
            $(SALLIB) \
            $(REGLIB)

APP2TARGET= regview
APP2OBJS=   $(OBJ)$/regview.obj $(OBJ)/fileurl.obj
APP2RPATH=  UREBIN

APP2STDLIBS=\
            $(SALLIB) \
            $(REGLIB)

APP3TARGET= regcompare
APP3OBJS=   $(OBJ)$/regcompare.obj $(OBJ)/fileurl.obj $(OBJ)/options.obj
APP3RPATH=  SDK

APP3STDLIBS=\
            $(SALLIB) \
            $(SALHELPERLIB) \
            $(REGLIB)

APP4TARGET= checksingleton
APP4OBJS=   $(OBJ)$/checksingleton.obj $(OBJ)/fileurl.obj $(OBJ)/options.obj

APP4STDLIBS=\
            $(SALLIB) \
            $(SALHELPERLIB) \
            $(REGLIB)

APP5TARGET= rdbedit
APP5OBJS=   $(OBJ)$/rdbedit.obj

APP5STDLIBS=\
            $(SALLIB) \
            $(SALHELPERLIB) \
            $(REGLIB)

OBJFILES = $(APP1OBJS) $(APP2OBJS) $(APP3OBJS) $(APP4OBJS) $(APP5OBJS)

.INCLUDE :  target.mk
