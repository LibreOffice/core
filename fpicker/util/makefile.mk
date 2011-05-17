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

PRJNAME=fpicker
TARGET=fpicker
TARGET1=fps
TARGET2=fop
USE_LDUMP2=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- fps dynlib ----------------------------------------------

COMMON_LIBS=$(CPPULIB)\
            $(CPPUHELPERLIB)\
            $(COMPHELPERLIB)\
            $(SALLIB)\
            $(VCLLIB)\
            $(TOOLSLIB)

.IF "$(GUI)"=="WNT"

SHL1TARGET=$(TARGET1)
SHL1STDLIBS=		$(COMMON_LIBS) \
            $(UWINAPILIB) \
            $(ADVAPI32LIB) \
            $(SHELL32LIB)\
            $(OLE32LIB)\
            $(GDI32LIB)\
            $(OLEAUT32LIB)\
            $(COMDLG32LIB)\
            $(KERNEL32LIB)\
            $(UUIDLIB)\
            Delayimp.lib\
            /DELAYLOAD:shell32.dll

SHL1DEPN=
SHL1IMPLIB=i$(SHL1TARGET)
SHL1LIBS=$(SLB)$/fps.lib\
         $(SLB)$/utils.lib
SHL1RES=$(RES)$/$(TARGET1).res
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

.ENDIF # "$(GUI)"=="WNT"

# --- fop dynlib --------------------------------------------------

.IF "$(GUI)"=="WNT"
SHL2NOCHECK=TRUE
SHL2TARGET=$(TARGET2)

SHL2STDLIBS=		$(COMMON_LIBS) \
            $(UWINAPILIB) \
            $(ADVAPI32LIB) \
            $(OLE32LIB)\
            $(GDI32LIB)\
            $(SHELL32LIB)\
            $(OLEAUT32LIB)

SHL2DEPN=
SHL2IMPLIB=i$(SHL2TARGET)
SHL2LIBS=$(SLB)$/fop.lib\
         $(SLB)$/utils.lib
SHL2DEF=$(MISC)$/$(SHL2TARGET).def

DEF2NAME=$(SHL2TARGET)
DEF2EXPORTFILE=	exports.dxp

.ENDIF          # "$(GUI)"=="WNT"


.INCLUDE :  target.mk

ALLTAR : $(MISC)/fop.component $(MISC)/fps.component

$(MISC)/fop.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        fop.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL2TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt fop.component

$(MISC)/fps.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        fps.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt fps.component
