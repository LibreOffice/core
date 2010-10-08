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

PRJNAME=io
TARGET = acceptor.uno
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE
COMP1TYPELIST = acceptor
COMPRDB=$(SOLARBINDIR)$/udkapi.rdb

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk
.IF "$(L10N_framework)"==""
DLLPRE =
# ------------------------------------------------------------------

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb
UNOUCROUT=$(OUT)$/inc$/acceptor
INCPRE+= $(UNOUCROUT)


SLOFILES= \
        $(SLO)$/acceptor.obj \
        $(SLO)$/acc_pipe.obj \
        $(SLO)$/acc_socket.obj

SHL1TARGET= $(TARGET)
SHL1VERSIONMAP = $(SOLARENV)/src/unloadablecomponent.map
SHL1RPATH=URELIB

SHL1STDLIBS= \
        $(SALLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB) 

SHL1DEPN=
SHL1IMPLIB=		i$(TARGET)
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)

# --- Targets ------------------------------------------------------
.ENDIF 		# L10N_framework

.INCLUDE :	target.mk

ALLTAR : $(MISC)/acceptor.component

$(MISC)/acceptor.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        acceptor.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_URE_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt acceptor.component
