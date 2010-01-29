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
# $Revision: 1.16 $
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
PRJNAME=filter
TARGET=flash

ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

.IF "$(SYSTEM_ZLIB)" == "YES"
CFLAGS+=-DSYSTEM_ZLIB
.ENDIF

SRS1NAME=$(TARGET)
SRC1FILES =	impswfdialog.src				

SLOFILES=	$(SLO)$/swffilter.obj								\
            $(SLO)$/swfwriter.obj								\
            $(SLO)$/swfwriter1.obj								\
            $(SLO)$/swfwriter2.obj								\
            $(SLO)$/swfuno.obj									\
            $(SLO)$/swfexporter.obj							\
            $(SLO)$/swfdialog.obj							\
            $(SLO)$/impswfdialog.obj

# --- Library -----------------------------------

RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES= $(SRS)$/$(TARGET).srs

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

# static libraries must come at the end for MACOSX
.IF "$(OS)" != "MACOSX"
SHL1STDLIBS+=\
    $(ZLIB3RDLIB)
.ENDIF


# dynamic libraries
SHL1STDLIBS+=\
    $(SVTOOLLIB)		\
    $(BASEGFXLIB)		\
    $(VCLLIB)			\
    $(UNOTOOLSLIB)		\
    $(TOOLSLIB)			\
    $(COMPHELPERLIB)	\
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(SALLIB) 

# static libraries must come at the end for MACOSX
.IF "$(OS)" == "MACOSX"
SHL1STDLIBS+=\
    $(ZLIB3RDLIB)
.ENDIF

SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)

# --- Targets ----------------------------------

.INCLUDE : target.mk

