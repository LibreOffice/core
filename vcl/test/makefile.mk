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
# $Revision: 1.11 $
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

PRJNAME=vcl
TARGET=dndtest
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

.IF "$(GUI)" == "OS2"
TARGETTYPE=GUI
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

APP1OBJS= \
    $(OBJ)$/dndtest.obj


APP1NOSAL=		TRUE
APP1TARGET= 	$(TARGET)
APP1STDLIBS=	$(CPPULIB)			\
                $(CPPUHELPERLIB)	\
                $(TOOLSLIB) 		\
                $(SALLIB)			\
                $(VOSLIB)			\
                $(SOTLIB)			\
                $(COMPHELPERLIB)	\
                $(VCLLIB)

# --- Targets ------------------------------------------------------

APP2TARGET= canvasbitmaptest
APP2OBJS=	\
    $(OBJ)$/canvasbitmaptest.obj

APP2NOSAL=		TRUE
APP2STDLIBS=$(TOOLSLIB) 		\
            $(COMPHELPERLIB)	\
            $(CPPULIB)			\
            $(CPPUHELPERLIB)	\
            $(UCBHELPERLIB)		\
            $(SALLIB)			\
            $(VCLLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

