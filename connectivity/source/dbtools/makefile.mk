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
# $Revision: 1.15 $
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
PRJNAME=connectivity
TARGET=dbt
USE_LDUMP2=TRUE

TARGETTYPE=CUI
USE_DEFFILE=TRUE

# --- Settings ----------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/version.mk

LDUMP=ldump2.exe

# --- Library -----------------------------------

LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1FILES=\
        $(SLB)$/sdbcx.lib		\
        $(SLB)$/commontools.lib \
        $(SLB)$/sql.lib			\
        $(SLB)$/simpledbt.lib   \
        $(SLB)$/cnr.lib

# --- dynamic library ---------------------------

SHL1TARGET=	$(DBTOOLS_TARGET)$(DLLPOSTFIX)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(SALLIB)					\
    $(JVMACCESSLIB)				\
    $(UNOTOOLSLIB)              \
    $(COMPHELPERLIB)            \
    $(I18NISOLANGLIB)

# NETBSD: somewhere we have to instantiate the static data members.
# NETBSD-1.2.1 doesn't know about weak symbols so the default mechanism for GCC won't work.
# SCO and MACOSX: the linker does know about weak symbols, but we can't ignore multiple defined symbols
.IF "$(OS)"=="NETBSD" || "$(OS)"=="SCO" || "$(OS)$(COM)"=="OS2GCC" || "$(OS)"=="MACOSX"
SHL1STDLIBS+=$(UCBHELPERLIB)
.ENDIF

#SHL1DEPN=
SHL1IMPLIB=	idbtools

SHL1LIBS=	$(LIB1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt \
            $(LIB1TARGET)
            
DEFLIB1NAME=$(TARGET)


# --- Targets ----------------------------------

.INCLUDE : target.mk

# --- filter file ------------------------------

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo _TI				>$@
    @echo _real				>>$@

