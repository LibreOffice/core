#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.23 $
#
#   last change: $Author: hr $ $Date: 2007-11-01 17:12:06 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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

CDEFS+=-D_TOOLS_STRINGLIST

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
APP1STDLIBS=$(SALLIB) $(VOSLIB) $(TOOLSLIB)

APP2TARGET= rscdep
APP2OBJS=	$(OBJ)$/rscdep.obj
APP2LIBS=   $(LB)$/$(TARGET).lib $(LB)$/$(TARGET1).lib
APP2STDLIBS= $(SALLIB) $(VOSLIB) $(TOOLSLIB)

APP3TARGET=  so_checksum
APP3OBJS=    $(OBJ)$/md5.obj \
             $(OBJ)$/so_checksum.obj
APP3STDLIBS= $(TOOLSLIB) $(SALLIB)

DEPOBJFILES	= $(APP1OBJS) $(APP2OBJS) $(APP3OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
