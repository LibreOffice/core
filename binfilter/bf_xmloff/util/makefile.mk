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

PRJNAME=binfilter
TARGET=bf_xo

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ----------------------------------------------------
NO_BSYMBOLIC=TRUE
ENABLE_EXCEPTIONS=TRUE

LIB1TARGET= $(SLB)$/bf_xo.lib
LIB1FILES=	\
    $(SLB)$/xmloff_core.lib \
    $(SLB)$/xmloff_meta.lib \
    $(SLB)$/xmloff_script.lib \
    $(SLB)$/xmloff_style.lib \
    $(SLB)$/xmloff_text.lib \
    $(SLB)$/xmloff_draw.lib \
    $(SLB)$/xmloff_chart.lib \
    $(SLB)$/xmloff_forms.lib

# --- Shared-Library -----------------------------------------------

.IF "$(GUI)"!="UNX" && "$(GUI)"!="OS2"
LIB4TARGET= $(LB)$/ibf_xo.lib
LIB4FILES=	$(LB)$/_ibf_xo.lib
.IF "$(GUI)"!="OS2"
LIB4OBJFILES=\
    $(OBJ)$/xmloff_xmlkywd.obj
.ENDIF
.ENDIF

SHL1TARGET= bf_xo$(DLLPOSTFIX)
.IF "$(GUI)" == "OS2"
SHL1IMPLIB= ibf_xo
.ELSE
SHL1IMPLIB= _ibf_xo
.ENDIF

SHL1STDLIBS= \
        $(BFSVTOOLLIB)	\
        $(TOOLSLIB)		\
        $(I18NISOLANGLIB) \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)\
        $(COMPHELPERLIB)\
        $(VCLLIB)		\
        $(TKLIB)		\
        $(SALLIB)		\
        $(SALHELPERLIB)		\
        $(UNOTOOLSLIB)	\
        $(BFGOODIESLIB) \
        $(BFSO3LIB)

# SCO: the linker does know about weak symbols, but we can't ignore multiple defined symbols
.IF "(OS)"=="SCO"
SHL1STDLIBS+=-licg617mxp
.ENDIF

SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
SHL1LIBS=   $(LIB1TARGET)

# --- Def-File ---------------------------------------------------------

DEF1NAME    =$(SHL1TARGET)
.IF "$(GUI)" != "OS2"
DEF1DEPN    =$(MISC)$/$(SHL1TARGET).flt
.ENDIF
DEFLIB1NAME =bf_xo
DEF1DES     =XML Office Lib
DEF1EXPORTFILE=	exports.dxp
# --- Targets ----------------------------------------------------------

.INCLUDE :  target.mk

# --- Filter -----------------------------------------------------------

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    $(TYPE) bf_xo.flt > $@

