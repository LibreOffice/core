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
SUBPRJ=$(PRJ)$/bf_so3

PRJPCH=

PRJNAME=binfilter
TARGET=bf_so

USE_LDUMP2=TRUE
#LDUMP2=LDUMP3

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(SUBPRJ)$/util$/makefile.pmk

RSCFLAGS+=-p

# --- Files --------------------------------------------------------


LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=\
        $(SLB)$/so3_solink.lib		\
        $(SLB)$/so3_dialog.lib      \
        $(SLB)$/so3_data.lib        \
        $(SLB)$/so3_inplace.lib     \
        $(SLB)$/so3_misc.lib        \
        $(SLB)$/so3_persist.lib \
        $(SLB)$/so3_copied.lib

.IF "$(GUI)"=="WNT"
LIB1FILES+=$(SLB)$/so3_ole.lib
.ENDIF

.IF "$(UPDATER)"==""
RESLIB1NAME=so
RESLIB1SRSFILES= $(SRS)$/so3_src.srs
.ENDIF

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB= $(TARGET)
SHL1STDLIBS=\
        $(VCLLIB) \
        $(SOTLIB) \
        $(UNOTOOLSLIB) \
        $(TOOLSLIB)	\
        $(BFSVTOOLLIB) \
        $(COMPHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB) \
        $(UCBHELPERLIB)

.IF "$(linkinc)" == ""
SHL1LIBS=	$(SLB)$/$(TARGET).lib
.ENDIF

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= \
            $(OLE32LIB) $(UUIDLIB) $(ADVAPI32LIB) $(GDI32LIB)
.ENDIF

.IF "$(GUI)"=="WIN"
SHL1STDLIBS= \
            ole2.lib compobj.lib storage.lib shell.lib \
            ole2disp.lib typelib.lib
.ENDIF

.IF "$(GUI)"=="UNX"
SHL1DEPN= $(LIB1TARGET)
.ELSE
SHL1DEPN= $(LIB1TARGET)
.ENDIF

.IF "$(linkinc)" == ""
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
SHL1OBJS+=	$(SHLOBJECTS)
.ELSE
SHL1OBJS+=	$(SLO)$/so2dll.obj					\
            $(SHLOBJECTS)
.ENDIF
.ENDIF
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

.ELSE			#"$(linkinc)" == ""
SHL11FILE=	$(MISC)$/so3_data.slo
SHL12FILE=  $(MISC)$/so3_dialog.slo
SHL13FILE=  $(MISC)$/so3_inplace.slo
SHL14FILE=	$(MISC)$/vmisc.slo
.IF "$(GUI)"=="WNT"
SHL15FILE=  $(MISC)$/so3_ole.slo
.ENDIF
SHL16FILE=  $(MISC)$/so3_persist.slo
SHL17FILE=  $(MISC)$/so3_solink.slo
.ENDIF

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME =$(TARGET)
DEF1DES 	=StarObjects

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  target.pmk


# --- SO2-Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo WEP > $@
    @echo LIBMAIN >> $@
    @echo LibMain >> $@
    @echo CreateDdeExtern@SvBaseLink >> $@
    @echo RemoveDdeExtern@SvBaseLink >> $@
    @echo DataChanged@SvBaseLink >> $@
    @echo allocator >> $@
    @echo Exception >> $@
    @echo =@8 >> $@
    @echo __CT >> $@

