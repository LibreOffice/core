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
# $Revision: 1.21 $
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

PRJ=..$/..$/..$/..$/..$/..$/..

PRJNAME=icc
TARGET=create_sRGB_profile
LIBTARGET=NO
TARGETTYPE=CUI

ENABLE_EXCEPTIONS=TRUE

EXTERNAL_WARNINGS_NOT_ERRORS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

CFLAGS+=-I..$/..$/..$/IccProfLib -I..$/..$/ICC_utils

# This tool uses unaligned memory accesses which will lead
# to SIGBUS on platforms who care, like Solaris LP64
.IF "$(OS)$(CPU)"=="SOLARISU"
LINKFLAGS+=-xmemalign=8i
.ENDIF

# --- Files --------------------------------------------------------

OBJFILES= $(OBJ)$/create_sRGB_profile.obj

# --- Targets ------------------------------------------------------

# svdem
APP1LIBSALCPPRT:=
UWINAPILIB:=
APP1NOSAL=		TRUE
APP1TARGET= 	$(TARGET)
APP1LIBS=\
    $(SLB)$/proflib.lib \
    $(SLB)$/icutil.lib
APP1OBJS= $(OBJFILES)
APP1STDLIBS=

.INCLUDE :	target.mk

ALLTAR: $(TARGET)$(EXECPOST)

$(TARGET)$(EXECPOST): $(BIN)$/$(TARGET)$(EXECPOST) makefile.mk
    rm -f $@
    $(GNUCOPY) $(BIN)$/$(TARGET)$(EXECPOST) $@
