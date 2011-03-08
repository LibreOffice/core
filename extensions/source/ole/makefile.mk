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
PRJ=..$/..

# -----------------------------------------------------------------

PRJNAME=extensions
TARGET=oleautobridge.uno
TARGET2=oleautobridge2.uno

ENABLE_EXCEPTIONS=TRUE

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

LIBTARGET=NO
USE_DEFFILE=YES

USE_DEFFILE=TRUE

INCPRE+= $(foreach,i,$(ATL_INCLUDE) -I$(i))

# --- Settings -----------------------------------------------------

.IF "$(GUI)" == "WNT" && "$(DISABLE_ATL)"==""

.IF "$(USE_DEBUG_RUNTIME)"!=""
CDEFS+=-D_DEBUG
.ENDIF # "$(USE_DEBUG_RUNTIME)"!=""

SLOFILES= \
            $(SLO)$/servreg.obj		\
            $(SLO)$/servprov.obj	\
            $(SLO)$/unoobjw.obj		\
            $(SLO)$/oleobjw.obj		\
            $(SLO)$/olethread.obj	\
            $(SLO)$/oledll.obj		\
        $(SLO)$/jscriptclasses.obj	\
        $(SLO)$/ole2uno.obj		\
        $(SLO)$/windata.obj		\
        $(SLO)$/unotypewrapper.obj

SECOND_BUILD=OWNGUID
OWNGUID_SLOFILES=$(SLOFILES)
OWNGUIDCDEFS+= -DOWNGUID

# the original library
SHL1TARGET=$(TARGET)
SHL1STDLIBS=\
        $(SALLIB) 	\
    $(CPPULIB)	\
    $(CPPUHELPERLIB)	\
    $(OLE32LIB) 	\
    $(UUIDLIB) 	\
    $(ADVAPI32LIB)	\
    $(OLEAUT32LIB)

.IF "$(COM)"=="MSC"
.IF "$(WINDOWS_VISTA_PSDK)"!="" || "$(CCNUMVER)"<="001399999999"
.IF "$(USE_DEBUG_RUNTIME)" != ""
    SHL1STDLIBS+= $(ATL_LIB)$/atlsd.lib
.ELSE
    SHL1STDLIBS+= $(ATL_LIB)$/atls.lib
.ENDIF
.ENDIF # "$(WINDOWS_VISTA_PSDK)"!="" || "$(CCNUMVER)"<="001399999999"
.ENDIF # "$(COM)"=="MSC"

SHL1LIBS=
SHL1OBJS=$(SLOFILES)

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=$(SHL1TARGET).dxp

# the second library
SHL2TARGET=$(TARGET2)
SHL2STDLIBS=\
        $(SALLIB) 	\
    $(CPPULIB)	\
    $(CPPUHELPERLIB)	\
    $(OLE32LIB) 	\
    $(UUIDLIB) 	\
    $(ADVAPI32LIB)	\
    $(OLEAUT32LIB)

.IF "$(COM)"=="MSC"
.IF "$(WINDOWS_VISTA_PSDK)"!="" || "$(CCNUMVER)"<="001399999999"
.IF "$(USE_DEBUG_RUNTIME)" != ""
    SHL2STDLIBS+= $(ATL_LIB)$/atlsd.lib
.ELSE
    SHL2STDLIBS+= $(ATL_LIB)$/atls.lib
.ENDIF
.ENDIF # "$(WINDOWS_VISTA_PSDK)"!="" || "$(CCNUMVER)"<="001399999999"
.ENDIF # "$(COM)"=="MSC"

SHL2LIBS=
SHL2OBJS=$(REAL_OWNGUID_SLOFILES)

DEF2NAME=$(SHL2TARGET)
DEF2EXPORTFILE=$(TARGET).dxp

.ENDIF # "$(GUI)" == "WNT" && "$(DISABLE_ATL)"==""

#----------------------------------------------------------------

.INCLUDE :  target.mk

