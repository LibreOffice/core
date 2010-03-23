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

PRJNAME=oox
TARGET=oox
USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.IF "$(L10N_framework)"==""
# --- Allgemein ----------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=	\
    $(SLB)$/token.lib\
    $(SLB)$/helper.lib\
    $(SLB)$/core.lib\
    $(SLB)$/ole.lib\
    $(SLB)$/ppt.lib\
    $(SLB)$/xls.lib\
    $(SLB)$/vml.lib\
    $(SLB)$/drawingml.lib\
    $(SLB)$/diagram.lib\
    $(SLB)$/chart.lib\
    $(SLB)$/table.lib\
    $(SLB)$/shape.lib\
    $(SLB)$/dump.lib\
    $(SLB)$/docprop.lib

# --- Shared-Library -----------------------------------------------

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB= i$(TARGET)
SHL1USE_EXPORTS=name

SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)\
        $(COMPHELPERLIB)\
        $(RTLLIB)		\
        $(SALLIB)		\
        $(BASEGFXLIB)	\
        $(SAXLIB)

# link openssl, copied this bit from ucb/source/ucp/webdav/makefile.mk
.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= $(OPENSSLLIB)
.ELSE # WNT
.IF "$(OS)"=="SOLARIS"
SHL1STDLIBS+= -lnsl -lsocket -ldl
.ENDIF # SOLARIS
.IF "$(SYSTEM_OPENSSL)"=="YES"
SHL1STDLIBS+= $(OPENSSLLIB)
.ELSE
SHL1STDLIBS+= $(OPENSSLLIBST)
.ENDIF
.ENDIF # WNT

SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
SHL1LIBS=   $(LIB1TARGET)
DEF1NAME    =$(SHL1TARGET)
DEFLIB1NAME =$(TARGET)

# --- Targets ----------------------------------------------------------
.ENDIF # L10N_framework

.INCLUDE :  target.mk
