#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

PRJ=..$/..$/

PRJNAME=salhelper
TARGET=dynloader
TARGET1=samplelib
TARGETTYPE=CUI
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings ---

.INCLUDE : settings.mk

# --- Files ---

#RTTI on
.IF "$(OS)" == "WNT"
CFLAGS+= -GR
.ENDIF


#---------------------------------------------------------------------------
# Build the test library which is loaded by the 
# RealDynamicLoader

SLOFILES=	$(SLO)$/samplelib.obj

LIB1TARGET=	$(SLB)$/$(TARGET1).lib
LIB1OBJFILES=	$(SLOFILES)


SHL1TARGET=	$(TARGET1)

SHL1STDLIBS= \
    $(SALLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(TARGET1)
SHL1LIBS=	$(SLB)$/$(TARGET1).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEFLIB1NAME=	$(TARGET1)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt

# ------------------------------------------------------------------------------

OBJFILES=	$(OBJ)$/loader.obj

APP1TARGET=	$(TARGET)
APP1OBJS=	$(OBJFILES)

APP1STDLIBS= \
    $(SALHELPERLIB)	\
    $(SALLIB)

APP1DEF=	$(MISC)\$(APP1TARGET).def

# --- Targets ---

.INCLUDE : target.mk

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo __CT>>$@
