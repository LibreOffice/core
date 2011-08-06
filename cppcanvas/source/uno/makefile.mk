#*************************************************************************
#
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Copyright 2010 Novell, Inc.
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   This file is part of OpenOffice.org.
#
#   OpenOffice.org is free software: you can redistribute it and/or modify
#   it under the terms of the GNU Lesser General Public License version 3
#	only, as published by the Free Software Foundation.
#
#   OpenOffice.org is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Lesser General Public License version 3 for more details
#	(a copy is included in the LICENSE file that accompanied this code).
#
#   You should have received a copy of the GNU Lesser General Public License
#   version 3 along with OpenOffice.org.  If not, see
#	<http://www.openoffice.org/license.html>
#   for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..$/..

PRJNAME=cppcanvas
TARGET=mtfrenderer
ENABLE_EXCEPTIONS=TRUE


# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk

# --- Common ----------------------------------------------------------

.IF "$(verbose)"!="" || "$(VERBOSE)"!=""
CDEFS+= -DVERBOSE
.ENDIF

SLOFILES =	$(SLO)$/uno_mtfrenderer.obj

SHL1TARGET=$(TARGET).uno

SHL1STDLIBS= $(SALLIB) $(CPPULIB) $(CPPUHELPERLIB) $(COMPHELPERLIB) $(CPPCANVASLIB) $(BASEGFXLIB)

SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=exports.map

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# ==========================================================================

.INCLUDE :	target.mk

$(MISC)/mtfrenderer.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        mtfrenderer.component
	$(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
         $(SOLARENV)/bin/createcomponent.xslt mtfrenderer.component

ALLTAR : $(MISC)/mtfrenderer.component
