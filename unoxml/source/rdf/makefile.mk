#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.2 $
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

PRJNAME=unoxml
TARGET=unordf
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(SYSTEM_REDLAND)" == "YES"
CFLAGS+=-DSYSTEM_REDLAND $(REDLAND_CFLAGS)
.ENDIF

.IF "$(SYSTEM_LIBXSLT)" == "YES"
CFLAGS+= $(LIBXSLT_CFLAGS)
.ELSE
LIBXSLTINCDIR=external$/libxslt
CFLAGS+= -I$(SOLARINCDIR)$/$(LIBXSLTINCDIR)
.ENDIF

# --- Files --------------------------------------------------------
.IF "$(L10N_framework)"==""

SLOFILES =	\
   $(SLO)$/CBlankNode.obj \
   $(SLO)$/CURI.obj \
   $(SLO)$/CLiteral.obj \
   $(SLO)$/librdf_repository.obj \
   $(SLO)$/librdf_services.obj


SHL1DEPN=   makefile.mk
SHL1OBJS=   $(SLOFILES)

SHL1TARGET=	$(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB= i$(TARGET)

SHL1VERSIONMAP=../service/exports.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1STDLIBS= \
    $(REDLANDLIB) \
    $(XSLTLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB)	\
    $(SALLIB) \

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

