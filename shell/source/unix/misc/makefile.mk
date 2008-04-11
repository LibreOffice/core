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
# $Revision: 1.8 $
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
PRJNAME=shell
TARGET=misc

LIBTARGET=NO
TARGETTYPE=CUI
NO_DEFAULT_STL=TRUE
LIBSALCPPRT=$(0)

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

SCRIPTFILES = \
    $(BIN)$/senddoc \
    $(BIN)$/open-url \
    $(BIN)$/cde-open-url \
    $(BIN)$/gnome-open-url \
    $(BIN)$/kde-open-url

APP1TARGET = gnome-open-url.bin
APP1OBJS = \
    $(OBJ)$/gnome-open-url.obj
APP1LIBS =
.IF "$(OS)"!="FREEBSD"
APP1STDLIBS=-ldl
.ENDIF

APP2TARGET = uri-encode
APP2OBJS = $(OBJ)$/uri-encode.obj
APP2LIBS =
APP2STDLIBS =

OBJFILES = $(APP1OBJS) $(APP2OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

ALLTAR : $(SCRIPTFILES) $(AWKFILES)

$(SCRIPTFILES) : $$(@:f:+".sh")
    @tr -d "\015" < $(@:f:+".sh") > $@
