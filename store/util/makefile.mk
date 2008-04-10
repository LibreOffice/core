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
# $Revision: 1.24 $
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

PRJNAME=store
TARGET=store
TARGETTYPE=CUI

USE_LDUMP2=TRUE
USE_DEFFILE=TRUE

NO_BSYMBOLIC=TRUE
NO_DEFAULT_STL=TRUE

UNIXVERSIONNAMES=UDK

# --- Settings ---

.INCLUDE : settings.mk

# --- Debug-Library ---

.IF "$(debug)" != ""

LIB1TARGET=	$(LB)$/$(TARGET)dbg.lib
LIB1ARCHIV=	$(LB)$/lib$(TARGET)dbg.a
LIB1FILES=	$(LB)$/store.lib

.ENDIF # debug

# --- Shared-Library ---

SHL1TARGET= 	$(TARGET)
SHL1IMPLIB= 	istore

SHL1VERSIONMAP=	$(TARGET).map

SHL1STDLIBS=	$(SALLIB)

# system STLport5 needs it
.IF "$(USE_STLP_DEBUG)" != "" || "$(STLPORT_VER)" >= "500"
SHL1STDLIBS+=$(LIBSTLPORT)
.ENDIF

# On gcc3 __Unwind_SetIP is not in supc++ but in libgcc_s.so
.IF "$(COMID)"=="gcc3" && "$(GUI)"!="OS2"
.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= 	-lsupc++
.ELSE
.IF "$(OS)"=="NETBSD"
SHL1STDLIBS+= 	-lsupc++
.ELIF "$(OS)"=="MACOSX"
.IF "$(CCNUMVER)"<="000399999999"
SHL1STDLIBS+=	-lsupc++
.ENDIF # CCNUMVER
.ELIF "$(CCNUMVER)"<="000400000999"
SHL1STDLIBS+= 	-lsupc++ -lgcc_s
.ENDIF
.ENDIF
.ENDIF


SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=	$(SLB)$/store.lib
SHL1RPATH=  URELIB


# --- Def-File ---

DEF1NAME=	$(SHL1TARGET)
DEF1DES=Store

# --- Targets ---

.INCLUDE : target.mk



