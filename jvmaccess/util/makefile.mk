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
PRJNAME = jvmaccess
TARGET = $(PRJNAME)

ENABLE_EXCEPTIONS = TRUE

.IF "$(OS)" != "WNT"
UNIXVERSIONNAMES = UDK
.ENDIF # WNT

.INCLUDE: settings.mk

.IF "$(UNIXVERSIONNAMES)" == ""
SHL1TARGET = $(TARGET)$(UDK_MAJOR)$(COMID)
.ELSE # UNIXVERSIONNAMES
SHL1TARGET = $(TARGET)$(COMID)
.ENDIF # UNIXVERSIONNAMES

.IF "$(COM)" == "MSC"
SHL1IMPLIB = i$(TARGET)
.ELSE
SHL1IMPLIB = $(TARGET)$(COMID)
.ENDIF
SHL1LIBS = $(SLB)$/$(TARGET).lib
SHL1STDLIBS = $(CPPULIB) $(SALLIB) $(SALHELPERLIB)
.IF "$(OS)" == "WNT"
SHL1STDLIBS += $(ADVAPI32LIB)
.ENDIF # WNT
SHL1RPATH = URELIB

.IF "$(COMNAME)" == "msci"
SHL1VERSIONMAP = msvc_win32_intel.map
.ELIF "$(COMNAME)" == "mscx"
SHL1VERSIONMAP = msvc_win32_x86-64.map
.ELIF "$(COMNAME)" == "sunpro5"
SHL1VERSIONMAP = cc5_solaris_sparc.map
.ELIF "$(GUI)$(COM)" == "WNTGCC"
SHL1VERSIONMAP = mingw.map
.ELIF "$(COMNAME)" == "gcc3"
SHL1VERSIONMAP = gcc3.map
.ENDIF

DEF1NAME = $(SHL1TARGET)

.INCLUDE: target.mk
