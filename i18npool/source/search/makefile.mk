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

PRJNAME=i18npool
TARGET=i18nsearch.uno
USE_DEFFILE=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
DLLPRE =

# --- Files --------------------------------------------------------

EXCEPTIONSFILES= \
            $(SLO)$/textsearch.obj

SLOFILES=	\
            $(EXCEPTIONSFILES) \
            $(SLO)$/levdis.obj

# on unxsoli, no optimization for textsearch because of i105945
.IF "$(OS)$(COM)$(CPUNAME)"=="SOLARISC52INTEL"
EXCEPTIONSNOOPTFILES= \
        $(SLO)$/textsearch.obj
.ENDIF

SHL1TARGET= $(TARGET)
SHL1OBJS=	$(SLOFILES)

.IF ("$(GUI)"=="UNX" || "$(COM)"=="GCC") && "$(GUI)"!="OS2"
I18NREGEXPLIB=-li18nregexp$(COMID)
.ELSE
I18NREGEXPLIB=ii18nregexp.lib
.ENDIF

SHL1STDLIBS= \
                $(CPPULIB) \
                $(CPPUHELPERLIB) \
                $(SALLIB) \
                $(I18NREGEXPLIB)

SHL1DEPN=		makefile.mk
SHL1VERSIONMAP= $(SOLARENV)/src/component.map
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
DEF1NAME=		$(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk


ALLTAR : $(MISC)/i18nsearch.component

$(MISC)/i18nsearch.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        i18nsearch.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt i18nsearch.component
