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

PRJNAME          :=cppu
TARGET           :=cppu
ENABLE_EXCEPTIONS:=TRUE
NO_BSYMBOLIC     :=TRUE
USE_DEFFILE      :=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE : ../source/helper/purpenv/export.mk

# --- Files --------------------------------------------------------

SHL3TARGET  := unsafe_uno_uno
SHL3IMPLIB  := i$(SHL3TARGET)
SHL3STDLIBS := $(purpenv_helper_LIB) $(SALLIB) 
SHL3OBJS    := $(SLO)$/UnsafeBridge.obj
SHL3DEF     := empty.def
SHL3OBJS    := $(SLO)$/UnsafeBridge.obj
SHL3RPATH   := URELIB

SHL4TARGET  := affine_uno_uno
SHL4IMPLIB  := i$(SHL4TARGET)
SHL4STDLIBS := $(purpenv_helper_LIB) $(SALLIB) 
SHL4OBJS    := $(SLO)$/AffineBridge.obj
SHL4DEF     := empty.def
SHL4OBJS    := $(SLO)$/AffineBridge.obj
SHL4RPATH   := URELIB


SHL5TARGET  := log_uno_uno
SHL5IMPLIB  := i$(SHL5TARGET)
SHL5STDLIBS := $(purpenv_helper_LIB) $(SALLIB) $(CPPULIB)
SHL5OBJS    := $(SLO)$/LogBridge.obj
                
SHL5DEF     := empty.def
SHL5RPATH   := URELIB

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

