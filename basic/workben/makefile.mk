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

PRJNAME=basic
TARGET=miniapp
TARGETTYPE=GUI

# --- Settings ---------------------------------------------------

.INCLUDE :  settings.mk

# --- SBASIC IDE --------------------------------------------------------

APP1TARGET=$(PRJNAME)app
APP1STDLIBS= \
            $(SALLIB) \
            $(TOOLSLIB) \
            $(UNOTOOLSLIB) \
            $(SVTOOLLIB) \
            $(SVLLIB) \
            $(VCLLIB) \
            $(COMPHELPERLIB) \
            $(UCBHELPERLIB) \
            $(CPPUHELPERLIB) \
            $(CPPULIB) \
            $(SJLIB) \
            $(SOTLIB) \


#.IF "$(GUI)"=="WNT" || "$(COM)"=="GCC"
#APP1STDLIBS+=$(CPPULIB)
#.ENDIF
#.IF "$(GUI)"=="UNX"
#APP1STDLIBS+= \
#			 \
#			$(SALLIB)
#.ENDIF

.IF "$(GUI)"!="OS2"
APP1LIBS= \
            $(LB)$/basic.lib 
.ENDIF
APP1LIBS+= \
            $(LB)$/app.lib \
            $(LB)$/sample.lib
.IF "$(GUI)"=="UNX" || "$(GUI)"=="OS2"
APP1STDLIBS+=	\
            $(BASICLIB)
.ENDIF


APP1DEPN=	$(L)$/itools.lib $(SVLIBDEPEND) $(LB)$/basic.lib $(LB)$/app.lib $(LB)$/sample.lib

APP1OBJS = $(OBJ)$/ttbasic.obj 

.IF "$(GUI)" != "UNX"
APP1OBJS+=	\
            $(OBJ)$/app.obj \
            $(SLO)$/sbintern.obj
.ENDIF

.INCLUDE :  target.mk

