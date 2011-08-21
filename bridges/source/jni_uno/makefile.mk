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

PRJ=..$/..

PRJNAME=bridges
TARGET=java_uno
USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(SOLAR_JAVA)"==""
nojava:
    @echo "Not building jni-uno bridge because Java is disabled"
.ENDIF

# --- Files --------------------------------------------------------

.IF "$(GUI)$(COM)" == "WNTGCC"
.IF "$(EXCEPTIONS)" == "sjlj"
CFLAGS += -DBROKEN_ALLOCA
.ENDIF
.ENDIF

SLOFILES= \
    $(SLO)$/jni_info.obj		\
    $(SLO)$/jni_data.obj		\
    $(SLO)$/jni_uno2java.obj	\
    $(SLO)$/jni_java2uno.obj	\
    $(SLO)$/jni_bridge.obj \
    $(SLO)$/nativethreadpool.obj 

SHL1TARGET=$(TARGET)

SHL1STDLIBS= \
    $(JVMACCESSLIB)			\
    $(CPPULIB)			\
    $(SALLIB) \
    $(SALHELPERLIB)

SHL1VERSIONMAP=$(TARGET).map

SHL1CREATEJNILIB=TRUE
SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
SHL1RPATH=URELIB

.IF "$(debug)" != ""
.IF "$(COM)" == "MSC"
CFLAGS += -Ob0
.ENDIF
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

