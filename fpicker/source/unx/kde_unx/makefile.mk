#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
# Copyright 2010 Novell, Inc.
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

PRJNAME=fpicker
TARGET=fps_kde.uno
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE=

# ------------------------------------------------------------------

# Currently just KDE is supported...
.IF "$(GUIBASE)" != "unx" || "$(ENABLE_KDE)" != "TRUE"

dummy:
    @echo "Nothing to build. GUIBASE == $(GUIBASE), ENABLE_KDE is not set"

.ELSE # we build for KDE

# --- Files --------------------------------------------------------

SLOFILES =\
        $(SLO)$/UnxCommandThread.obj	\
        $(SLO)$/UnxFilePicker.obj		\
        $(SLO)$/UnxNotifyThread.obj		\
        $(SLO)$/UnxFPentry.obj

SHL1NOCHECK=TRUE
SHL1TARGET=fps_kde.uno
SHL1STDLIBS=$(CPPULIB)\
        $(CPPUHELPERLIB)\
        $(SALLIB)\
        $(VCLLIB)\
        $(TOOLSLIB)

SHL1OBJS=$(SLOFILES)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1VERSIONMAP=$(SOLARENV)/src/component.map

.ENDIF # "$(GUIBASE)" != "unx" || "$(ENABLE_KDE)" != "TRUE"

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
