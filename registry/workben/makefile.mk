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
# $Revision: 1.8 $
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

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------
CDEFS += -DDLL_VERSION=$(EMQ)"$(DLLPOSTFIX)$(EMQ)"

RGTLIB = rgt.lib
.IF "$(GUI)"=="UNX" || "$(GUI)"=="OS2"
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

APP2STDLIBS=\
            $(RGTLIB)

APP3TARGET= regspeed
APP3OBJS=   $(OBJ)$/regspeed.obj

APP3STDLIBS=\
            $(SALLIB)	\
            $(REGLIB) 	 

.INCLUDE :  target.mk
