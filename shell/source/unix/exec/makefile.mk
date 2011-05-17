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

PRJ=..$/..$/..
PRJNAME=shell
TARGET=exec

LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

COMP1TYPELIST=syssh

TESTAPP1=urltest

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

DLLPRE=

SLOFILES=$(SLO)$/shellexec.obj\
    $(SLO)$/shellexecentry.obj
    
SHL1OBJS=$(SLOFILES) 
    
SHL1TARGET=syssh.uno
.IF "$(GUI)" == "OS2"
SHL1IMPLIB=i$(TARGET)
.ELSE
SHL1IMPLIB=
.ENDIF

SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
                    
SHL1STDLIBS=$(CPPULIB)\
            $(CPPUHELPERLIB)\
            $(SALLIB)
SHL1LIBS=
SHL1DEPN=

.IF "$(test)" != "" 

APP1TARGET=$(TESTAPP1)
APP1STDLIBS= $(SHL1STDLIBS)
APP1OBJS= \
    $(SLO)$/shellexec.obj \
    $(SLO)$/$(APP1TARGET).obj

.ENDIF # "$(test)" != "" 


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

run_test : $(BIN)$/$(TESTAPP1).sh
    dmake test=t
    $(BIN)$/$(TESTAPP1) urltest.txt

$(BIN)$/$(TESTAPP1).sh : $$(@:f)
    $(COPY) $< $@
    -chmod +x $@ 

ALLTAR : $(MISC)/syssh.component

$(MISC)/syssh.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        syssh.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt syssh.component
