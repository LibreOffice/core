#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: kz $ $Date: 2006-10-05 11:00:03 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
