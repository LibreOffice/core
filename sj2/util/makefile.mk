#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 15:30:02 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
