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
TARGET=testsyssh
TARGET1=testsmplmail
TARGET2=testprx
TARGET4=testfopen
LIBTARGET=NO
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

CFLAGS+=/GX

OBJFILES1=$(OBJ)$/TestSysShExec.obj
OBJFILES2=$(OBJ)$/TestSmplMail.obj
OBJFILES3=$(OBJ)$/TestProxySet.obj
OBJFILES4=$(OBJ)$/Testfopen.obj

OBJFILES=$(OBJFILES1)\
         $(OBJFILES2)

APP1TARGET=$(TARGET)
APP1OBJS=$(OBJFILES1)

APP1STDLIBS+=	$(CPPULIB)			\
                $(CPPUHELPERLIB)	\
                $(SALLIB) 	 		\
                $(USER32LIB)

APP1DEF=		$(MISC)$/$(APP1TARGET).def


# --- TestSmplMail ---

APP2TARGET=$(TARGET1)
APP2OBJS=$(OBJFILES2)

APP2STDLIBS+=	$(CPPULIB)			\
                $(CPPUHELPERLIB)	\
                $(SALLIB) 	 		\
                $(USER32LIB)

APP2DEF=		$(MISC)$/$(APP2TARGET).def

# --- TestProxy ---

APP3TARGET=$(TARGET2)
APP3OBJS=$(OBJFILES3)

APP3STDLIBS+=	$(CPPULIB)			\
                $(CPPUHELPERLIB)	\
                $(SALLIB)

APP3DEF=		$(MISC)$/$(APP3TARGET).def

# --- Testfopen ---

APP4TARGET=$(TARGET4)
APP4OBJS=$(OBJFILES4)

APP4STDLIBS+=	$(SALLIB)

APP4DEF=		$(MISC)$/$(APP4TARGET).def

# --- Targets ------------------------------------------------------

.INCLUDE :		target.mk


