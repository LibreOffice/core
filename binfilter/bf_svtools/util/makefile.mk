#*************************************************************************
#*
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
BFPRJ=..

PRJNAME=binfilter
RESTARGET=bf_svt
TARGET=bf_svtool

NO_HIDS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
INC+= -I$(PRJ)$/inc$/bf_svtools

# --- general section ----------------------------------------------------

LIB1TARGET= $(SLB)$/bf_svt.lib
LIB1FILES=	\
        $(SLB)$/misc.lib		\
        $(SLB)$/items.lib		\
        $(SLB)$/unoiface.lib	\
        $(SLB)$/filter.lib		\
        $(SLB)$/igif.lib		\
        $(SLB)$/jpeg.lib		\
        $(SLB)$/ixpm.lib		\
        $(SLB)$/ixbm.lib		\
        $(SLB)$/numbers.lib 	\
        $(SLB)$/numbers.uno.lib 	\
        $(SLB)$/wmf.lib 		\
        $(SLB)$/undo.lib		\
        $(SLB)$/syslocale.lib   \
        $(SLB)$/svdde.lib \
        $(SLB)$/config.lib	\
        $(SLB)$/filerec.lib \
        $(SLB)$/items1.lib	\
        $(SLB)$/misc1.lib	\
        $(SLB)$/notify.lib	\
        $(SLB)$/svarray.lib

RESLIB1NAME=	$(RESTARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES= \
        $(SRS)$/ehdl.srs        \

SHL1TARGET= bf_svt$(DLLPOSTFIX)
SHL1IMPLIB= bf_svt
SHL1DEF = $(MISC)$/$(SHL1TARGET).def
SHL1LIBS= $(SLB)$/bf_svt.lib

.IF "$(OS)"!="MACOSX"
# static libraries
SHL1STDLIBS+= $(JPEG3RDLIB)
.ENDIF

# dynamic libraries
SHL1STDLIBS+= \
        $(UNOTOOLSLIB)		\
        $(VCLLIB)			\
        $(SOTLIB)			\
        $(TOOLSLIB)			\
        $(I18NISOLANGLIB)   \
        $(UCBHELPERLIB)		\
        $(COMPHELPERLIB)	\
        $(CPPUHELPERLIB)	\
        $(CPPULIB)			\
        $(SALLIB)

.IF "$(OS)"=="MACOSX"
# static libraries go at end
SHL1STDLIBS+= $(JPEG3RDLIB)
.ENDIF

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= \
        $(UWINAPILIB) \
        $(ADVAPI32LIB)	\
        $(GDI32LIB) \
        $(OLE32LIB) 	\
        $(UUIDLIB) 	\
        $(ADVAPI32LIB)	\
        $(OLEAUT32LIB)
.ENDIF # WNT


DEF1NAME = $(SHL1TARGET)
DEF1DEPN = $(MISC)$/$(SHL1TARGET).flt \
           $(LIB1TARGET)

DEF1DES		=Bf_Svtools
DEFLIB1NAME	=bf_svt

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

$(MISC)$/$(SHL1TARGET).flt: svt.flt
    @echo ------------------------------
    @echo Making: $@
    $(TYPE) svt.flt >$@
