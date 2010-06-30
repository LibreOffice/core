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

PRJNAME=soltools
TARGET=cpp
TARGETTYPE=CUI
NO_DEFAULT_STL=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : $(PRJ)$/util$/makefile.pmk
.INCLUDE :  settings.mk

UWINAPILIB=$(0)
LIBSALCPPRT=$(0)

# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/_cpp.obj		\
    $(OBJ)$/_eval.obj 	\
    $(OBJ)$/_include.obj \
    $(OBJ)$/_lex.obj 	\
    $(OBJ)$/_macro.obj 	\
    $(OBJ)$/_mcrvalid.obj \
    $(OBJ)$/_nlist.obj 	\
    $(OBJ)$/_tokens.obj 	\
    $(OBJ)$/_unix.obj

# nonstandard cpp options needed for Mac (-isysroot),
# needs the custom stgetopt defined here :/
.IF "$(OS)" == "MACOSX" || "$(HAVE_GETOPT)" != "YES"
OBJFILES += $(OBJ)$/_getopt.obj
.ENDIF
.IF "$(HAVE_GETOPT)" == "YES"
CDEFS += -DHAVE_GETOPT
.ENDIF

APP1TARGET	=	$(TARGET)
.IF "$(GUI)" != "OS2"
APP1STACK	=	1000000
.ENDIF
APP1LIBS	=	$(LB)$/$(TARGET).lib
APP1DEPN	=   $(LB)$/$(TARGET).lib

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


