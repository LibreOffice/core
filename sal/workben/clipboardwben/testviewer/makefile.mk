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

PRJNAME=sal
TARGET=cbvtest
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# --- Files --------------------------------------------------------

#
# test clipboard paste
#
    # --- Resources ----------------------------------------------------
    RCFILES=  cbvtest.rc

    OBJFILES= $(OBJ)$/cbvtest.obj

    APP1TARGET=	$(TARGET)
    APP1OBJS=	$(OBJFILES)
    APP1NOSAL=  TRUE
    APP1NOSVRES= $(RES)$/$(TARGET).res

    APP1STDLIBS+=$(OLE32LIB) $(USER32LIB) $(KERNEL32LIB)
    
    APP1LIBS=$(LB)$/iole9x.lib \
             $(LB)$/tools32.lib 

    APP1DEPN=   makefile.mk $(APP1NOSVRES)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


