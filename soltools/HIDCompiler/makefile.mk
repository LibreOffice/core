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
TARGET=hidc
TARGETTYPE=CUI
LIBTARGET=no
NO_DEFAULT_STL=TRUE
INCPRE=$(MISC)

# --- Settings -----------------------------------------------------

.INCLUDE : $(PRJ)$/util$/makefile.pmk
.INCLUDE :  settings.mk
.INCLUDE :  libs.mk
CDEFS+= -DYY_NEVER_INTERACTIVE=1

UWINAPILIB=$(0)

# --- Files --------------------------------------------------------

# HID compiler
APP1TARGET=     $(TARGET)
APP1OBJS=   $(OBJ)$/wrap_hidclex.obj
APP1DEPN=   $(OBJ)$/wrap_hidclex.obj
APP1LIBSALCPPRT=

DEPOBJFILES=$(APP1OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/%_yy.cxx : %lex.l
    flex -l -8 -o$@ $<

$(OBJ)$/wrap_hidclex.obj: $(MISC)$/hidc_yy.cxx
