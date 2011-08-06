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
TARGET= testjavavm
PRJNAME=	stoc
TARGET6=	testjavavm
TARGETTYPE=	CUI
LIBTARGET=	NO
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=	TRUE

UNOUCRDEP=	$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=	$(SOLARBINDIR)$/udkapi.rdb
UNOUCROUT=	$(OUT)$/inc$
INCPRE+=	$(OUT)$/inc$

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Application 6 - testjavavm ------------------------------------

UNOTYPES= com.sun.star.lang.XMultiComponentFactory \
    com.sun.star.uno.XWeak	\
    com.sun.star.java.XJavaVM	\
    com.sun.star.java.XJavaThreadRegister_11	\
    com.sun.star.registry.XSimpleRegistry \
    com.sun.star.lang.XComponent \
    com.sun.star.registry.XImplementationRegistration \
    com.sun.star.lang.XSingleServiceFactory \
    com.sun.star.uno.TypeClass \
    com.sun.star.lang.XMultiServiceFactory

APP6TARGET= 	$(TARGET6)
APP6OBJS  = 	$(OBJ)$/testjavavm.obj 
APP6STDLIBS= \
        $(CPPULIB) 		\
        $(CPPUHELPERLIB) 	\
        $(SALHELPERLIB) 	\
        $(SALLIB)	

# --- Target ------------------------------------------------

.INCLUDE :	target.mk

