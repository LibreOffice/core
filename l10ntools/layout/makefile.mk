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

PRJ=..

INCPRE=$(MISC)

PRJNAME=l10ntools
TARGET=tralay
#TARGETTYPE=GUI
TARGETTYPE=CUI
LIBTARGET=no

# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=TRUE

.INCLUDE :  settings.mk

.IF "$(SYSTEM_EXPAT)" == "YES"
CFLAGS+=-DSYSTEM_EXPAT
.ENDIF

# --- Files --------------------------------------------------------

APP1TARGET=$(TARGET)

OBJFILES =\
    $(OBJ)/export2.obj\
    $(OBJ)/helpmerge.obj\
    $(OBJ)/layoutparse.obj\
    $(OBJ)/merge.obj\
    $(OBJ)/tralay.obj\
    $(OBJ)/xmlparse.obj

APP1OBJS = $(OBJFILES)

APP1STDLIBS =\
    $(TOOLSLIB)\
    $(EXPATASCII3RDLIB)\
    $(CPPULIB) \
    $(SALLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

test .PHONY:
    ../$(INPATH)/bin/tralay -l en-US -o out.sdf zoom.xml
    cat out.sdf > trans.sdf
    sed 's/en-US\t/de\tde:/' out.sdf >> trans.sdf 
    ../$(INPATH)/bin/tralay -m trans.sdf -l de -o zoom-DE.xml zoom.xml
