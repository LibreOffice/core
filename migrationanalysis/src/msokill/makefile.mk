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

PRJNAME		=	migrationanalysis
TARGET		=	msokill
LIBTARGET	=	NO
TARGETTYPE	=	CUI

ENABLE_EXCEPTIONS	=	TRUE

# --- Settings ------------------------------------------------------------

.INCLUDE : settings.mk

# --- Allgemein -----------------------------------------------------------

OBJFILES= 	$(OBJ)$/msokill.obj \
            $(OBJ)$/StdAfx.obj

# --- Targets ------------------------------------------------------

UWINAPILIB=
LIBSALCPPRT=

APP1NOSAL=		TRUE
APP1TARGET=		msokill

#APP1STDLIB
STDLIB1=msi.lib\
    shell32.lib\
    oleaut32.lib\
    gdi32.lib\
    comdlg32.lib\
    advapi32.lib\
    comctl32.lib\
    shlwapi.lib\
    oledlg.lib\
    ole32.lib\
    uuid.lib\
    oleacc.lib\
    winspool.lib\

APP1OBJS=		$(OBJFILES)

PAW_RES_DIR:=$(BIN)$/ProAnalysisWizard$/Resources
PAW_RES_EXE:=$(PAW_RES_DIR)$/$(APP1TARGET).exe

# --- setup --------------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : $(PAW_RES_EXE)

$(PAW_RES_EXE) : $(BIN)$/$$(@:f)
    -$(MKDIRHIER) $(@:d)
    $(COPY) $(BIN)$/$(@:f) $@

