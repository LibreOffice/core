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

PRJ=..$/..$/..

PRJNAME=javaldx
TARGET=javaldx
TARGETTYPE=CUI
.IF "$(OS)"!="IRIX"
NO_DEFAULT_STL=TRUE
.ENDIF
LIBTARGET=NO
ENABLE_EXCEPTIONS=true

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(OS)"=="MACOSX"
STDLIBCUIMT+=-lstdc++
.ENDIF

.IF "$(SOLAR_JAVA)"==""
nojava:
    @echo "Not building jvmfwk/plugins/javaenvsetup/ because Java is disabled"
.ENDIF

# --- Files --------------------------------------------------------

OBJFILES=$(OBJ)$/javaldx.obj

APP1TARGET=javaldx
APP1OBJS=$(OBJFILES)
APP1STDLIBS=$(SALLIB) $(CPPULIB) $(CPPUHELPERLIB) $(JVMFWKLIB)
APP1RPATH=UREBIN

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
