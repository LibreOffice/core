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
PRJNAME=xmlhelp
TARGET=chelp

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE


# --- Settings ---------------------------------------------------------

.INCLUDE: settings.mk

# GCC versions 4.2.x introduced a warning "allocating zero-element array"
# Allocating zero-element arrays is an allowed if not somewhat dubious 
# technique though, so this warning is plain wrong and has been fixed 
# in gcc 4.3. Unfortunately there is no way at all to suppress this warning.
# Some files in this directory use zero allocated arrays, we need to
# diable the WaE mechanism for the GCC 4.2.x series.
.IF "$(COM)"=="GCC"
.IF "$(CCNUMVER)">="000400020000" && "$(CCNUMVER)"<="000400020003"
CFLAGSWERRCXX:=
.ENDIF # "$(CCNUMVER)">="000400020000" && "$(CCNUMVER)"<="000400020003"
.ENDIF # "$(COM)"=="GCC"

CFLAGS +=  -DHAVE_EXPAT_H

.IF "$(SYSTEM_DB)" == "YES"
CFLAGS+=-DSYSTEM_DB -I$(DB_INCLUDES)
.ENDIF

.IF "$(SYSTEM_LIBXML)" == "YES"
CFLAGS+= $(LIBXML_CFLAGS)
.ELSE
LIBXMLINCDIR=external$/libxml
CFLAGS+= -I$(SOLARINCDIR)$/$(LIBXMLINCDIR)
.ENDIF

.IF "$(SYSTEM_LIBXSLT)" == "YES"
CFLAGS+= $(LIBXSLT_CFLAGS)
.ELSE
LIBXSLTINCDIR=external$/libxslt
CFLAGS+= -I$(SOLARINCDIR)$/$(LIBXSLTINCDIR)
.ENDIF

.IF "$(GUI)"=="WNT"
CFLAGS+=-GR
.ENDIF

# --- General -----------------------------------------------------

SLOFILES=\
    $(SLO)$/services.obj    	   \
    $(SLO)$/provider.obj    	   \
    $(SLO)$/content.obj     	   \
    $(SLO)$/resultset.obj     	   \
    $(SLO)$/resultsetbase.obj      \
    $(SLO)$/resultsetforroot.obj   \
    $(SLO)$/resultsetforquery.obj  \
    $(SLO)$/contentcaps.obj        \
    $(SLO)$/urlparameter.obj       \
    $(SLO)$/inputstream.obj        \
    $(SLO)$/databases.obj          \
    $(SLO)$/db.obj		           \
    $(SLO)$/bufferedinputstream.obj

# --- Targets ----------------------------------------------------------

.INCLUDE: target.mk

