#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2007-05-09 13:49:05 $
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

PRJ     := ..$/..
PRJNAME := cppu
TARGET  := mapping.tester

ENABLE_EXCEPTIONS  := TRUE
NO_BSYMBOLIC       := TRUE
NO_SHL_DESCRIPTION := TRUE


.INCLUDE :  settings.mk

VERSIONOBJ := # NO GetVersionInfo symbols :-)
ENVINCPRE  := -I$(OUT)$/inc$/$(TARGET)
#CDEFS += -DLOG_LIFECYCLE

.IF "$(COM)" == "GCC"
LINKFLAGS += -rdynamic
.ENDIF


.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
ObjectFactory_LIB := -lObjectFactory.$(COMID)

.ELSE
ObjectFactory_LIB := $(LIBPRE) iObjectFactory.$(COMID).lib

.ENDIF


APP1TARGET  := mapping.tester
APP1OBJS    := $(OBJ)$/mapping.tester.obj
APP1STDLIBS := $(ObjectFactory_LIB) $(SALLIB) $(CPPULIB) $(CPPUHELPERLIB) 


.INCLUDE :  target.mk


