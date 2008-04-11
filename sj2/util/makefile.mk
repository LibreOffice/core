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
# $Revision: 1.16 $
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

PRJNAME=sj2
TARGET=sj
TARGETTYPE=GUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=  \
    $(SLB)$/jscpp.lib

SHL1DEPN= $(LIB1TARGET)

SHL1TARGET= j$(DLLPOSTFIX)_g
SHL1IMPLIB= $(TARGET)

SHL1STDLIBS= \
        $(VCLLIB) \
        $(TOOLSLIB) \
        $(CPPULIB) \
        $(SALLIB)

.IF "$(GUI)$(COMID)"=="WNTMSC"
SHL1STDLIBS+= \
        $(SVTOOLLIB)
.ENDIF # "$(GUI)$(COMID)"=="WNTMSC"

.IF "$(SOLAR_JAVA)"!=""
    SHL1STDLIBS+=$(JVMACCESSLIB)
.ENDIF

SHL1LIBS=   $(SLB)$/$(TARGET).lib
SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME    =$(SHL1TARGET)
DEF1DEPN    =$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME =$(TARGET)
DEF1DES     =JavaCPP
DEF1CEXP    =Java

JARTARGET=classes.jar
JARCLASSDIRS=stardiv
JARCLASSPATH = sandbox.jar

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  target.pmk

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo WEP>$@
    @echo LIBMAIN>>$@
    @echo LibMain>>$@
    @echo bad_alloc::bad_alloc>>$@
    @echo exception::exception>>$@
.IF "$(COM)"=="MSC"
    @echo __CT>>$@
    @echo _C@>>$@
.ENDIF
