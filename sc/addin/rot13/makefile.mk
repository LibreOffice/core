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

PRJNAME=sc
TARGET=rot
LIBTARGET=NO


# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

CL2C=+$(PERL) $(MISC)$/cl2c.pl
CL2CRID=RID_SC_ADDIN_ROT13
CL2CSRC=$(TARGET)13.src

# --- Files --------------------------------------------------------

CFILES= $(MISC)$/x$(TARGET).c

SLOFILES= \
            $(SLO)$/x$(TARGET).obj

# ==========================================================================

SHL1TARGET= rot$(DLLPOSTFIX)
SHL1IMPLIB= irot
SHL1OBJS=   $(SLO)$/x$(TARGET).obj

SHL1VERSIONMAP=exports.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

.IF "$(GUI)" == "WNT"
SHL1STDLIBS=     $(GDI32LIB) $(ADVAPI32LIB) $(COMDLG32LIB) \
                 $(UUIDLIB) $(OLE32LIB) $(SHELL32LIB) $(WINSPOOLLIB)
.IF "$(GVER)" == "W40"
SHL1STDLIBS=    $(SHL1STDLIBS) $(COMCTL32LIB)
.ENDIF # W40
.ENDIF # WNT

# --- Targets -------------------------------------------------------

ALLTAR:	$(MISC)$/rot.lst 

.INCLUDE: target.mk

$(MISC)$/x$(TARGET).c: $(TARGET)13.cl $(CL2CSRC) $(MISC)$/cl2c.pl
    $(CL2C) $(TARGET)13.cl $(MISC)$/x$(TARGET).c $(CL2CSRC) $(CL2CRID)

# copy file to include in package
$(INCCOM)$/xlang.h : $(SOLARINCDIR)$/i18npool$/lang.h
    @$(COPY) $(SOLARINCDIR)$/i18npool$/lang.h $@

$(SLOFILES) : $(INCCOM)$/xlang.h $(CFILES)

$(MISC)$/rot.lst : \
    $(MISC)$/x$(TARGET).c \
    $(INCCOM)$/xlang.h \
    ..$/inc$/rot13.hrc \
    ..$/inc$/addin.h
    @echo $< > $@
