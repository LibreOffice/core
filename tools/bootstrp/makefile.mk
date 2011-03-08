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

PRJNAME=tools
TARGET=btstrp
TARGET1=bootstrp2
TARGETTYPE=CUI
LIBTARGET=NO
# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(HAVE_GETOPT)" == "YES"
CDEFS += -DHAVE_GETOPT
.ENDIF

# --- Files --------------------------------------------------------

OBJFILES= \
          $(OBJ)$/appdef.obj \
          $(OBJ)$/command.obj \
          $(OBJ)$/cppdep.obj\
          $(OBJ)$/inimgr.obj\
          $(OBJ)$/mkcreate.obj \
          $(OBJ)$/sstring.obj \
          $(OBJ)$/prj.obj

SLOFILES= \
          $(SLO)$/appdef.obj \
          $(SLO)$/command.obj \
          $(SLO)$/cppdep.obj \
          $(SLO)$/inimgr.obj \
          $(SLO)$/mkcreate.obj \
          $(SLO)$/sstring.obj \
          $(SLO)$/prj.obj


LIB1TARGET= $(LB)$/$(TARGET).lib
LIB1ARCHIV= $(LB)$/lib$(TARGET).a
LIB1OBJFILES=\
          $(OBJ)$/appdef.obj \
          $(OBJ)$/command.obj \
          $(OBJ)$/cppdep.obj \
          $(OBJ)$/inimgr.obj \
          $(OBJ)$/mkcreate.obj \
          $(OBJ)$/sstring.obj

LIB2TARGET= $(LB)$/$(TARGET1).lib
LIB2ARCHIV= $(LB)$/lib$(TARGET1).a
LIB2OBJFILES=\
        $(OBJ)$/prj.obj

APP1TARGET= sspretty
APP1OBJS=   $(OBJ)$/sspretty.obj
APP1LIBS=   $(LB)$/$(TARGET).lib $(LB)$/$(TARGET1).lib
APP1STDLIBS=$(SALLIB)  $(TOOLSLIB) $(BASEGFXLIB) $(UCBHELPERLIB) $(CPPULIB) $(COMPHELPERLIB) $(CPPUHELPERLIB) $(SALHELPERLIB) $(I18NISOLANGLIB)

APP2TARGET= rscdep
APP2OBJS=	$(OBJ)$/rscdep.obj
APP2LIBS=   $(LB)$/$(TARGET).lib $(LB)$/$(TARGET1).lib
APP2STDLIBS= $(SALLIB)  $(TOOLSLIB) $(BASEGFXLIB) $(UCBHELPERLIB) $(CPPULIB) $(COMPHELPERLIB) $(I18NISOLANGLIB) $(CPPUHELPERLIB) $(SALHELPERLIB)
.IF "$(HAVE_GETOPT)" != "YES"
.IF "$(OS)"=="WNT"
APP2STDLIBS+=gnu_getopt.lib
.ENDIF
.ENDIF
APP2RPATH=  NONE
APP2RPATH=  NONE
APP2RPATH=  NONE

APP3TARGET=  so_checksum
APP3OBJS=    $(OBJ)$/md5.obj \
             $(OBJ)$/so_checksum.obj
APP3STDLIBS= $(TOOLSLIB) $(SALLIB)  $(BASEGFXLIB) $(UCBHELPERLIB) $(CPPULIB) $(COMPHELPERLIB) $(I18NISOLANGLIB) $(CPPUHELPERLIB) $(SALHELPERLIB)

DEPOBJFILES	= $(APP1OBJS) $(APP2OBJS) $(APP3OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
