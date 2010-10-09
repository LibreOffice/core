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
PRJNAME=formula
TARGET=for
TARGET2=forui
USE_DEFFILE=TRUE
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

# USE_LDUMP2=TRUE
# --- Settings ----------------------------------
.INCLUDE :  settings.mk


# --- formula core (for) -----------------------------------

LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1FILES=\
        $(SLB)$/core_resource.lib \
        $(SLB)$/core_api.lib

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(TOOLSLIB) 			\
        $(SVTOOLLIB)			\
                $(SVLLIB) \
        $(UNOTOOLSLIB)			\
        $(COMPHELPERLIB) 		\
        $(CPPUHELPERLIB) 		\
        $(CPPULIB)				\
        $(VCLLIB)				\
        $(SALLIB)

SHL1IMPLIB=i$(TARGET)
SHL1USE_EXPORTS=name
SHL1LIBS=$(LIB1TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEFLIB1NAME=$(TARGET)

# --- .res file ----------------------------------------------------------

RES1FILELIST=\
    $(SRS)$/core_strings.srs

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=$(RES1FILELIST)


# --- formula UI core (forui) -----------------------------------
LIB2TARGET=$(SLB)$/$(TARGET2).lib
LIB2FILES=\
        $(SLB)$/uidlg.lib \
        $(SLB)$/ui_resource.lib

SHL2TARGET=$(TARGET2)$(DLLPOSTFIX)

SHL2STDLIBS= \
        $(SFXLIB)				\
        $(SVTOOLLIB)			\
        $(SVLLIB)  \
        $(VCLLIB)				\
        $(UNOTOOLSLIB)			\
        $(TOOLSLIB)				\
        $(CPPULIB)				\
        $(SO2LIB)				\
        $(CPPUHELPERLIB)        \
        $(SALLIB)
        
.IF "$(GUI)"=="OS2"
SHL2STDLIBS+= \
        $(LB)$/ifor.lib
SHL2DEPN=$(LB)$/i$(TARGET).lib
.ELIF "$(GUI)"!="WNT" || "$(COM)"=="GCC"
SHL2STDLIBS+= \
        -lfor$(DLLPOSTFIX)
SHL2DEPN=$(SHL1TARGETN)
.ELSE
SHL2STDLIBS+= \
        $(LB)$/ifor.lib
SHL2DEPN=$(LB)$/i$(TARGET).lib
.ENDIF


SHL2IMPLIB=i$(TARGET2)
SHL2LIBS=$(LIB2TARGET)
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
DEF2NAME=$(SHL2TARGET)
.IF "$(GUI)"=="OS2"
DEFLIB2NAME=$(TARGET2)
.ENDIF

SHL2USE_EXPORTS=name

# --- .res file ----------------------------------------------------------

RES2FILELIST=\
    $(SRS)$/uidlg.srs


RESLIB2NAME=$(TARGET2)
RESLIB2IMAGES=$(PRJ)$/res
RESLIB2SRSFILES=$(RES2FILELIST)


# --- Targets ----------------------------------

.INCLUDE : target.mk


ALLTAR : $(MISC)/for.component

$(MISC)/for.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        for.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt for.component
