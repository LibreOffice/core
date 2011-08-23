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

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=..$/..
BFPRJ=..

PRJNAME=binfilter
TARGET=bf_ofa

NO_HIDS=TRUE

# --- Settings ------------------------------------------------------------

.INCLUDE :	settings.mk
#INC+= -I$(PRJ)$/inc$/bf_offmgr

.IF "$(prjpch)" != ""
CDEFS+=-DPRECOMPILED
.ENDIF

RSCLOCINC!:=$(RSCLOCINC);$(PRJ)$/RES
SOLARLIB+=-L$(LB)

# --- Allgemein -----------------------------------------------------------

RESLIB1LIST=\
    $(SRS)$/so3_src.srs 	

RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES=$(RESLIB1LIST)

# -------------------------------------------------------------------------
LIB3TARGET= $(SLB)$/$(TARGET).lib
LIB3FILES=	$(SLB)$/offmgr_app.lib 

SHL2TARGET= $(TARGET)$(DLLPOSTFIX)
SHL2IMPLIB= $(TARGET)
SHL2LIBS=	$(SLB)$/$(TARGET).lib

# dynamic libraries
SHL2STDLIBS+= \
    $(BFSVTOOLLIB) \
    $(BFSVXLIB) \
    $(LEGACYSMGRLIB)	\
    $(BFBASICLIB) \
    $(TOOLSLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)

.IF "$(GUI)"=="WNT"
SHL2STDLIBS += $(ADVAPI32LIB)
.ENDIF # WNT

SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
SHL2BASE=	0x1de00000

DEF2NAME=	$(SHL2TARGET)
DEF2DEPN=	$(MISC)$/$(SHL2TARGET).flt
DEFLIB2NAME =$(TARGET)
DEF2DES 	=offmgr app-interface

# -------------------------------------------------------------------------

.IF "$(GUI)"=="WNT"
LIB4TARGET= $(LB)$/a$(TARGET).lib
LIB4FILES=	$(LB)$/$(TARGET).lib
.ENDIF

.IF "$(L10N_framework)"==""
.IF "$(BUILD_SOSL)"==""
.IF "$(depend)" == ""
ALL:\
    $(LIB3TARGET)			\
    $(INCCOM)$/class.lst	\
    ALLTAR

.ENDIF # "$(depend)" == ""
.ENDIF # "$(BUILD_SOSL)" == ""
.ENDIF

.INCLUDE :	target.mk

$(MISC)$/$(SHL2TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo WEP>$@
    @echo PlugInApplication>>$@
    @echo __dt__17OfficeApplication>>$@
    @echo LibMain>>$@
    @echo _Impl>>$@
    @echo _Imp>>$@
    @echo solver>>$@
    @echo bad_alloc>>$@
    @echo exception12>>$@
.IF "$(GUI)"=="WNT"
    @echo __CT>>$@
.ENDIF

# ------------------------------------------------------------------
# Windows NT
# ------------------------------------------------------------------

$(INCCOM)$/class.lst:
.IF "$(BUILD_SOSL)"==""
.IF "$(GUI)$(CPU)$(UPDATER)"=="WNTIYES"
    -$(COPY) class.lst $@
.ELSE
    @echo nix
.ENDIF
.ENDIF			# "$(BUILD_SOSL)"==""
