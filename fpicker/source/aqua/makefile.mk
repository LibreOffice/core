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
# $Revision: 1.4 $
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

PRJ=..$/..

PRJNAME=fpicker
TARGET=fps_aqua.uno
TARGETTYPE=GUI
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

DLLPRE=

# ------------------------------------------------------------------

.IF "$(GUIBASE)" != "aqua"

dummy:
    @echo "Nothing to build. GUIBASE == $(GUIBASE)"

.ELSE # we build for aqua


# --- Files --------------------------------------------------------

SLOFILES =\
        $(SLO)$/resourceprovider.obj       \
        $(SLO)$/FPentry.obj                \
        $(SLO)$/SalAquaPicker.obj          \
        $(SLO)$/SalAquaFilePicker.obj      \
        $(SLO)$/SalAquaFolderPicker.obj    \
        $(SLO)$/CFStringUtilities.obj      \
        $(SLO)$/FilterHelper.obj           \
        $(SLO)$/ControlHelper.obj          \
        $(SLO)$/NSString_OOoAdditions.obj  \
        $(SLO)$/NSURL_OOoAdditions.obj     \
        $(SLO)$/AquaFilePickerDelegate.obj

SHL1NOCHECK=TRUE
SHL1TARGET= $(TARGET)
SHL1OBJS=   $(SLOFILES)
SHL1STDLIBS=\
    $(VCLLIB) \
    $(TOOLSLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)

SHL1VERSIONMAP=exports.map
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

.ENDIF # "$(GUIBASE)" != "aqua"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
