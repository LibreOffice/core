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

PRJ=..

PRJNAME=svl
TARGET=svl
RESTARGETSIMPLE=svs
GEN_HID=TRUE
# GEN_HID_OTHER=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- general section ----------------------------------------------------

.IF "$(GUI)"!="UNX"
LIB2TARGET= $(LB)$/isvl.lib
LIB2FILES=	$(LB)$/_isvl.lib
.ENDIF

LIB1TARGET= $(SLB)$/svl.lib
LIB1FILES=	\
        $(SLB)$/config.lib	\
        $(SLB)$/svdde.lib \
        $(SLB)$/undo.lib		\
        $(SLB)$/numbers.lib 	\
        $(SLB)$/numbers.uno.lib 	\
        $(SLB)$/filerec.lib \
        $(SLB)$/filepicker.lib \
        $(SLB)$/items.lib	\
        $(SLB)$/misc.lib	\
        $(SLB)$/notify.lib	\
                $(SLB)$/unoiface.lib      \
        $(SLB)$/svarray.lib \
        $(SLB)$/svsql.lib

# generation of resourcen-lib ----------------------------------------

RESLIB1NAME=	$(RESTARGETSIMPLE)
RESLIB1SRSFILES=\
        $(SRS)$/items.srs \
        $(SRS)$/misc.srs

# build the shared library  --------------------------------------------------

SHL1TARGET= svl$(DLLPOSTFIX)
SHL1IMPLIB= _isvl
SHL1USE_EXPORTS=name
#Do not link with VCL or any other library that links with VCL
SHL1STDLIBS= \
        $(UNOTOOLSLIB)		\
        $(TOOLSLIB) 		\
        $(I18NISOLANGLIB)   \
        $(UCBHELPERLIB)		\
        $(COMPHELPERLIB)	\
        $(CPPUHELPERLIB)	\
        $(CPPULIB)			\
        $(VOSLIB)			\
                $(SOTLIB)     \
        $(SALLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= \
        $(UWINAPILIB) \
        $(ADVAPI32LIB)	\
        $(GDI32LIB)
.ENDIF # WNT

SHL1LIBS=	$(SLB)$/svl.lib

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1DEPN=	$(SLB)$/svl.lib
DEFLIB1NAME=svl
DEF1DES =SvTools lite

# --- Targets ------------------------------------------------------

.IF "$(GUI)"=="UNX"
SVTTARGETS= $(LB)$/lib$(SHL1TARGET)$(DLLPOST)
.ELSE
SVTTARGETS= $(LB)$/isvl.lib
.ENDIF

# just a quick fix - has to be cleaned up some day...
.IF "$(L10N-framework)"==""
ALL: $(SLB)$/svl.lib \
    $(MISC)$/$(SHL1TARGET).def \
    $(SVTTARGETS) \
    ALLTAR
.ENDIF          # "$(L10N-framework)"==""

.INCLUDE :	target.mk


ALLTAR : $(MISC)/svl.component

$(MISC)/svl.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        svl.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt svl.component
