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
TARGET=schart3

NO_HIDS=TRUE

LIBTARGET=NO

# --- Settings -----------------------------------------------------------
.INCLUDE :  settings.mk
INC+= -I$(PRJ)$/inc$/bf_sch
IENV!:=$(IENV);$(RES);..$/res

# --- Files --------------------------------------------------------

RESLIB1LIST=\
    $(SRS)$/sch_app.srs	\
    $(SRS)$/sch_core.srs	

RESLIB1NAME=bf_sch
RESLIB1SRSFILES=\
    $(RESLIB1LIST)

LIB2TARGET= $(SLB)$/sch_schmod.lib
LIB2OBJFILES=	$(SLO)$/sch_schmod.obj

LIB3TARGET= $(SLB)$/bf_sch.lib
LIB3FILES=	$(SLB)$/sch_core.lib \
        $(SLB)$/sch_app.lib \
        $(SLB)$/sch_docshell.lib \
        $(SLB)$/sch_chxchart.lib \
        $(SLB)$/sch_xml.lib 


SHL1TARGET= bf_sch$(DLLPOSTFIX)
SHL1IMPLIB= bf_schi

SHL1VERSIONMAP= bf_sch.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1STDLIBS=\
    $(BFSVXLIB)	\
    $(LEGACYSMGRLIB)	\
    $(BFSO3LIB)	\
    $(BFGOODIESLIB)	\
    $(BFSVTOOLLIB)	\
    $(VCLLIB)	\
    $(SOTLIB)	\
    $(UNOTOOLSLIB) \
    $(TOOLSLIB)	\
    $(COMPHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)
    

SHL1BASE=	0x1d100000
SHL1LIBS=   $(LIB3TARGET)

.IF "$(GUI)"!="UNX"
.IF "$(GUI)$(COM)" != "WNTGCC"
SHL1OBJS=   $(SLO)$/sch_schdll.obj
.ENDIF
.ENDIF # ! UNX

.IF "$(GUI)" == "WNT" || "$(GUI)" == "WIN"
SHL1RES=	$(RCTARGET)
.ENDIF # WNT || WIN

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk

