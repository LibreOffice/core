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
PRJNAME=ucb
# Version
UCPGIO_MAJOR=1
TARGET=ucpgio
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

.INCLUDE: settings.mk
.IF "$(L10N_framework)"==""

.IF "$(ENABLE_GIO)"!=""
COMPILER_WARN_ALL=TRUE
PKGCONFIG_MODULES=gio-2.0
.INCLUDE: pkg_config.mk

.IF "$(OS)" == "SOLARIS"
LINKFLAGS+=-z nodefs
.ENDIF          # "$(OS)" == "SOLARIS"

# no "lib" prefix
DLLPRE =

SLOFILES=$(SLO)$/gio_provider.obj\
         $(SLO)$/gio_content.obj\
         $(SLO)$/gio_resultset.obj\
         $(SLO)$/gio_datasupplier.obj\
         $(SLO)$/gio_seekable.obj\
         $(SLO)$/gio_inputstream.obj\
         $(SLO)$/gio_outputstream.obj\
         $(SLO)$/gio_mount.obj

SHL1TARGET=$(TARGET)$(UCPGIO_MAJOR).uno
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1IMPLIB=i$(TARGET)
SHL1STDLIBS=\
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(SALHELPERLIB)  \
    $(UCBHELPERLIB)

SHL1STDLIBS+=$(PKGCONFIG_LIBS)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

.ENDIF          # "$(ENABLE_GIO)"!=""
.ENDIF # L10N_framework

.INCLUDE: target.mk

ALLTAR : $(MISC)/ucpgio.component

$(MISC)/ucpgio.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        ucpgio.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ucpgio.component
