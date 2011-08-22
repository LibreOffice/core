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

PRJ=..$/..$/..
PRJNAME=shell
TARGET=smplmail
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST=$(TARGET)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES=$(SLO)$/smplmailentry.obj\
         $(SLO)$/smplmailmsg.obj\
         $(SLO)$/smplmailclient.obj\
         $(SLO)$/smplmailsuppl.obj\
         $(SLO)$/simplemapi.obj
                        
SHL1TARGET=$(TARGET).uno

SHL1STDLIBS=$(CPPULIB)\
            $(CPPUHELPERLIB)\
            $(SALLIB)\
            $(ADVAPI32LIB)

SHL1IMPLIB=i$(SHL1TARGET)

SHL1OBJS=$(SLOFILES) 
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- mapimailer --------------------------------------------------------

TARGETTYPE=CUI

OBJFILES=   $(OBJ)$/senddoc.obj\
            $(OBJ)$/simplemapi.obj

APP1TARGET=senddoc
APP1OBJS=$(OBJFILES)
APP1STDLIBS=$(KERNEL32LIB)\
            $(SALLIB)
            
APP1DEF=$(MISC)$/$(APP1TARGET).def

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk


ALLTAR : $(MISC)/smplmail.component

$(MISC)/smplmail.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        smplmail.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt smplmail.component
