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
# $Revision: 1.19 $
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
PRJNAME=svtools
TARGET=bmp
LIBTARGET=NO

# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

OBJFILES=   $(OBJ)$/bmp.obj			\
            $(OBJ)$/bmpcore.obj		\
            $(OBJ)$/g2g.obj			\
            $(OBJ)$/bmpsum.obj

# --- APP1TARGET ---------------------------------------------------

APP1TARGET= $(TARGET)

APP1STDLIBS =		\
    $(VCLLIB)		\
    $(TOOLSLIB)		\
    $(BASEGFXLIB) $(UCBHELPERLIB) $(CPPULIB) $(COMPHELPERLIB) $(CPPUHELPERLIB) $(SALHELPERLIB) $(I18NISOLANGLIB) \
    $(UNOTOOLSLIB) \
    $(ICUDATALIB) $(ICUUCLIB) $(ICULELIB) \
    $(SOTLIB) $(I18NUTILLIB) $(JVMACCESSLIB) \
    $(VOSLIB) 		\
    $(SALLIB)

APP1OBJS=   $(OBJ)$/bmp.obj			\
            $(OBJ)$/bmpcore.obj		
            
APP1BASE=0x10000000

# --- APP2TARGET --------------------------------------------------

APP2TARGET	=	bmpsum
APP2BASE	=	0x10000000
APP2OBJS	=   $(OBJ)$/bmpsum.obj

APP2STDLIBS	=	$(VCLLIB)		\
                $(TOOLSLIB)		\
                $(BASEGFXLIB) $(UCBHELPERLIB) $(CPPULIB) $(COMPHELPERLIB) $(CPPUHELPERLIB) $(SALHELPERLIB) $(I18NISOLANGLIB) \
                $(VOSLIB) 		\
                $(UNOTOOLSLIB) \
                $(ICUDATALIB) $(ICUUCLIB) $(ICULELIB) \
                $(SOTLIB) $(I18NUTILLIB) $(JVMACCESSLIB) \
                $(SALLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
