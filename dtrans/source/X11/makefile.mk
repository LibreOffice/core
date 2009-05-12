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
# $Revision: 1.14 $
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

PRJNAME=dtrans
TARGET=dtransX11
TARGETTYPE=GUI

ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST=$(TARGET)
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

.IF "$(GUIBASE)"=="aqua"

dummy:
    @echo "Nothing to build for Mac OS X"
 
.ELSE		# "$(GUIBASE)"=="aqua"

.IF "$(COM)$(CPU)" == "C50I" || "$(COM)$(CPU)" == "C52I"
NOOPTFILES=\
    $(SLO)$/X11_selection.obj
.ENDIF

SLOFILES=\
    $(SLO)$/X11_dndcontext.obj		\
    $(SLO)$/X11_transferable.obj	\
    $(SLO)$/X11_clipboard.obj		\
    $(SLO)$/X11_selection.obj		\
    $(SLO)$/X11_droptarget.obj		\
    $(SLO)$/X11_service.obj			\
    $(SLO)$/bmp.obj					\
    $(SLO)$/config.obj

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

.IF "$(OS)"=="MACOSX"
SHL1STDLIBS= $(LIBSTLPORT) $(CPPUHELPERLIB)
.ELSE
SHL1STDLIBS= $(CPPUHELPERLIB)
.ENDIF

SHL1STDLIBS+= \
        $(UNOTOOLSLIB)	\
        $(CPPULIB) 	\
        $(SALLIB)	\
        -lX11

SHL1DEPN=
SHL1IMPLIB=		i$(SHL1TARGET) 
SHL1OBJS=		$(SLOFILES)

SHL1VERSIONMAP=exports.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

.ENDIF		# "$(OS)"=="MACOSX"

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
