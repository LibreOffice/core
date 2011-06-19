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

PRJ=..

PRJNAME=registry
TARGET=regtest
TARGETTYPE=CUI
LIBTARGET=NO

ENABLE_EXCEPTIONS := TRUE

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------
CDEFS += -DDLL_VERSION=$(EMQ)"$(DLLPOSTFIX)$(EMQ)"

RGTLIB = rgt.lib
.IF "$(GUI)"=="UNX"
RGTLIB = -lrgt$(DLLPOSTFIX)
.ENDIF


CXXFILES= 	regtest.cxx   	\
            test.cxx		\
            regspeed.cxx


APP1TARGET= $(TARGET)
APP1OBJS=   $(OBJ)$/regtest.obj 

APP1STDLIBS=\
            $(SALLIB)	\
            $(REGLIB)

APP2TARGET= test
APP2OBJS=   $(OBJ)$/test.obj
APP2RPATH=NONE
.IF "$(GUI)"=="UNX" && "$(OS)"!="MACOSX"

.IF "$(OS)"=="LINUX"
APP2LINKFLAGS=-Wl,-rpath,\''$$ORIGIN:$$ORIGIN/../lib'\'
.ENDIF

.IF "$(OS)"=="SOLARIS"
APP2LINKFLAGS=-R\''$$ORIGIN/../lib:$$ORIGIN'\'
.ENDIF
    
.ENDIF # "$(OS)"=="UNX"
    
APP2STDLIBS=\
            $(RGTLIB)

APP3TARGET= regspeed
APP3OBJS=   $(OBJ)$/regspeed.obj

APP3STDLIBS=\
            $(SALLIB)	\
            $(REGLIB) 	 

.INCLUDE :  target.mk
