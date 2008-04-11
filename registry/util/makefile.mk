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
# $Revision: 1.10 $
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

PRJ=..

PRJNAME=registry
TARGET=reg

UNIXVERSIONNAMES=UDK

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

# ------------------------------------------------------------------

USE_LDUMP2=TRUE
#USE_DEFFILE=TRUE
LDUMP2=LDUMP3

DOCPPFILES= $(INC)$/registry$/registry.hxx \
            $(INC)$/registry$/registry.h \
            $(INC)$/registry$/regtype.h \
            $(INC)$/registry$/reflread.hxx \
            $(INC)$/registry$/reflwrit.hxx \
            $(INC)$/registry$/refltype.hxx \

LIB1TARGET= $(SLB)$/$(TARGET).lib

LIB1FILES= 	$(SLB)$/$(TARGET)cpp.lib

SHL1TARGET= $(TARGET)
SHL1IMPLIB= ireg
SHL1STDLIBS= \
    $(SALLIB) \
    $(SALHELPERLIB) \
    $(STORELIB)

SHL1VERSIONMAP=	$(TARGET).map

SHL1LIBS= $(LIB1TARGET)	
SHL1DEPN= $(LIB1TARGET)
SHL1DEF= $(MISC)$/$(SHL1TARGET).def
DEF1NAME= $(SHL1TARGET)
DEF1DES	= RegistryRuntime
SHL1RPATH=URELIB

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

makedocpp: $(DOCPPFILES)
     docpp -H -m -f  -u -d $(OUT)$/doc$/$(PRJNAME) $(DOCPPFILES)

