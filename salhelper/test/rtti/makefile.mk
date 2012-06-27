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
PRJ=..$/..

PRJNAME=	salhelper
TARGET=		rtti
TARGET1=samplelibrtti
LIBTARGET=NO
TARGETTYPE=CUI


ENABLE_EXCEPTIONS=TRUE

USE_DEFFILE=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

#RTTI on
.IF "$(OS)" == "WNT"
CFLAGS+= -GR
.ENDIF

SLOFILES=	\
        $(SLO)$/samplelibrtti.obj

LIB1TARGET=$(SLB)$/$(TARGET1).lib
LIB1OBJFILES= \
        $(SLO)$/samplelibrtti.obj

SHL1TARGET=	$(TARGET1)

SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)


SHL1DEPN=
SHL1IMPLIB=	i$(TARGET1)
SHL1LIBS=	$(SLB)$/$(TARGET1).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1EXPORTFILE=	exports.dxp

DEF1NAME=	$(SHL1TARGET)

.IF "$(OS)$(CPUNAME)"=="SOLARISSPARC"
SHL1VERSIONMAP=	sols.map
.ENDIF


# ------------------------------------------------------------------

APP1NOSAL=TRUE

APP1TARGET=	$(TARGET)

APP1OBJS=	$(OBJ)$/rttitest.obj

APP1STDLIBS= \
    $(SALLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB)

.IF "$(OS)" == "WNT"
APP1STDLIBS+=	$(LB)$/isamplelibrtti.lib
.ELSE
APP1STDLIBS+=	-lsamplelibrtti
.ENDIF 

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

