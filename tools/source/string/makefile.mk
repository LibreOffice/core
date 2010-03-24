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

PRJNAME=tools
TARGET=str

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

ALWAYSDBGFILES = $(SLO)$/debugprint.obj

.IF "$(ALWAYSDBGFILES)" != ""
ALWAYSDBGTARGET=do_it_alwaysdebug
.ENDIF

SLOFILES=   $(SLO)$/tstring.obj     \
            $(SLO)$/tustring.obj    \
            $(SLO)$/tenccvt.obj     \
            $(SLO)$/debugprint.obj

OBJFILES=   $(OBJ)$/tstring.obj     \
            $(OBJ)$/tustring.obj    \
            $(OBJ)$/tenccvt.obj     \
            $(OBJ)$/debugprint.obj

# --- Targets ------------------------------------------------------

.IF "$(ALWAYSDBG_FLAG)"==""
TARGETDEPS+=$(ALWAYSDBGTARGET)
.ENDIF

.INCLUDE : target.mk

.IF "$(ALWAYSDBGTARGET)" != ""
.IF "$(ALWAYSDBG_FLAG)" == ""
# --------------------------------------------------
# - ALWAYSDBG - files always compiled with debugging
# --------------------------------------------------
$(ALWAYSDBGTARGET):
       @echo --- ALWAYSDBGFILES ---
       @dmake $(MFLAGS) $(MAKEFILE) debug=true $(ALWAYSDBGFILES) ALWAYSDBG_FLAG=TRUE $(CALLMACROS)
       @echo --- ALWAYSDBGFILES OVER ---

$(ALWAYSDBGFILES):
       @echo --- ALWAYSDBG ---
       @dmake $(MFLAGS) $(MAKEFILE) debug=true ALWAYSDBG_FLAG=TRUE $(CALLMACROS) $@
       @echo --- ALWAYSDBG OVER ---
.ENDIF
.ENDIF
