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
PRJNAME=shell
TARGET=syssh
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST=$(TARGET)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES=$(SLO)$/SysShentry.obj\
         $(SLO)$/SysShExec.obj

SHL1TARGET=$(TARGET).uno

SHL1STDLIBS=$(CPPULIB)\
            $(CPPUHELPERLIB)\
            $(SALLIB)\
            $(UWINAPILIB) \
            $(ADVAPI32LIB) \
            $(OLE32LIB) \
            $(SHELL32LIB)

SHL1DEPN=
SHL1IMPLIB=i$(SHL1TARGET)

SHL1OBJS=$(SLOFILES) 
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
INCLUDE!:=$(subst,/stl, $(INCLUDE))
.EXPORT : INCLUDE


ALLTAR : $(MISC)/syssh.component

$(MISC)/syssh.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        syssh.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt syssh.component
