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

PRJNAME=FlatXml
TARGET=FlatXml
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE
COMP1TYPELIST=$(TARGET)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(GUI)"=="WNT"
DLLPOSTFIX=.dll
DLLPREFIX=
.ELSE
DLLPOSTFIX=.so
DLLPREFIX=lib
.ENDIF

# --- Files --------------------------------------------------------
ALLIDLFILES = test_bridge.idl
#CPPUMAKERFLAGS += -C

UNOUCRDEP=$(SOLARBINDIR)$/applicat.rdb
UNOUCRRDB=$(SOLARBINDIR)$/applicat.rdb 

# output directory (one dir for each project)
UNOUCROUT=$(OUT)$/inc$/examples

# adding to inludeoath
INCPRE+=$(UNOUCROUT)

SLOFILES = $(SLO)$/FlatXml.obj

SHL1TARGET= FlatXml

SHL1STDLIBS= \
        $(SALLIB)	\
        $(VOSLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB) 

SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)
SHL1LIBS=	$(SLB)$/$(SHL1TARGET).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR :	$(MISC)$/component_officeclient_registered.flag

.IF "$(GUI)"=="WNT"
PATHPREFIX = .
CURRENTDIR = $(BIN)
.ELSE
PATHPREFIX = ..$/bin
CURRENTDIR = $(LB)
.ENDIF

$(MISC)$/component_officeclient_registered.flag .SETDIR=$(CURRENTDIR): 
    -rm -f ..$/misc$/component_officeclient_registered.flag
    $(COPY) $(SOLARBINDIR)$/applicat.rdb $(PATHPREFIX)$/officeclient.rdb
    
