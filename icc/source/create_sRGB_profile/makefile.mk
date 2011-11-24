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

PRJ=..$/..$/

PRJNAME=icc
TARGET=create_sRGB_profile
LIBTARGET=NO
TARGETTYPE=CUI

ENABLE_EXCEPTIONS=TRUE
EXTERNAL_WARNINGS_NOT_ERRORS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_SAMPLEICC)"=="YES"
CFLAGS+=$(SAMPLEICC_CFLAGS)
.ELSE
CFLAGS+= \
    -I$(PRJ)$(INPATH)$/misc$/build$/SampleICC-1.3.2$/IccProfLib \
    -I$(PRJ)$(INPATH)$/misc$/build$/SampleICC-1.3.2$/Contrib$/ICC_utils
.ENDIF

# --- Files --------------------------------------------------------

OBJFILES= $(OBJ)$/create_sRGB_profile.obj

# --- Targets ------------------------------------------------------

APP1LIBSALCPPRT:=
UWINAPILIB:=
APP1NOSAL=	TRUE
APP1TARGET= 	$(TARGET)
.IF "$(SYSTEM_SAMPLEICC)"=="YES"
APP1LIBS=
APP1STDLIBS=\
    $(SAMPLEICC_LIBS) \
    -lICC_utils
# this library is not in pkgconfig but in the same dir
# so no need to cry for it and just hardcode it here
# bug also opened upstream about this
.ELSE
APP1LIBS=\
    $(SLB)$/proflib.lib \
    $(SLB)$/icutil.lib
APP1STDLIBS=
.ENDIF
APP1OBJS= $(OBJFILES)

.INCLUDE :	target.mk

ALLTAR: $(TARGET)$(EXECPOST)

$(TARGET)$(EXECPOST): $(BIN)$/$(TARGET)$(EXECPOST)
.IF "$(CROSS_COMPILING)" != "YES"
    cd $(BIN)$/ && $(AUGMENT_LIBRARY_PATH) $(BIN)$/$(TARGET)$(EXECPOST)
.ELSE
    cd $(BIN)$/ && $(SRC_ROOT)$/icc$/$(INPATH_FOR_BUILD)$/bin$/$(TARGET)
.ENDIF
