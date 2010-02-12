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

PRJNAME=sc
TARGET=dfa
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

CL2C=+$(PERL) $(MISC)$/cl2c.pl
CL2CRID=RID_SC_ADDIN_DFA
CL2CSRC=$(TARGET).src

# --- Files --------------------------------------------------------

CFILES= $(MISC)$/x$(TARGET).c

# no version OBJ
#VERSIONOBJ=

SLOFILES= \
            $(SLO)$/x$(TARGET).obj

# ==========================================================================

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB= i$(TARGET)
SHL1OBJS=   $(SLO)$/x$(TARGET).obj
SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

.IF "$(GUI)" == "WNT"
SHL1STDLIBS=     $(GDI32LIB) $(ADVAPI32LIB) $(COMDLG32LIB) \
                 $(UUIDLIB) $(OLE32LIB) $(SHELL32LIB) $(WINSPOOLLIB)
.IF "$(GVER)" == "W40"
SHL1STDLIBS=    $(SHL1STDLIBS) $(COMCTL32LIB)
.ENDIF
.ENDIF

# --- Targets -------------------------------------------------------

ALLTAR:	$(MISC)$/$(TARGET).lst 
    

.INCLUDE: target.mk

$(MISC)$/x$(TARGET).c: $(TARGET).cl $(CL2CSRC)
    $(CL2C) $(TARGET).cl $(MISC)$/x$(TARGET).c $(CL2CSRC) $(CL2CRID)

# copy file to include in package
$(INCCOM)$/xlang.h : $(SOLARINCDIR)$/i18npool$/lang.h
    @$(COPY) $(SOLARINCDIR)$/i18npool$/lang.h $@


$(SLOFILES) : $(INCCOM)$/xlang.h $(CFILES)

$(MISC)$/$(TARGET).lst : \
    $(MISC)$/x$(TARGET).c \
    $(INCCOM)$/xlang.h \
    ..$/inc$/$(TARGET).hrc \
    ..$/inc$/addin.h
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
    echo $< > $@
.ELSE
    echo $(<:+"\n":s/ //) > $@
.ENDIF

# --- Def-File ---

# ------------------------------------------------------------------
# Windows DEF File
# ------------------------------------------------------------------

.IF "$(GUI)"=="WNT"

$(MISC)$/$(SHL1TARGET).def: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo LIBRARY     $(SHL1TARGET)>$@
.IF "$(COM)"!="GCC"
    @echo DESCRIPTION 'DateF StarCalc Addin DLL'>>$@
    @echo DATA        READ WRITE NONSHARED>>$@
.ENDIF
    @echo EXPORTS>>$@
    @echo     GetFunctionCount>>$@
    @echo     GetFunctionData>>$@
    @echo     GetParameterDescription>>$@
    @echo     SetLanguage >>$@
    @echo     ScDate_GetDiffWeeks>>$@
    @echo     ScDate_GetDiffMonths>>$@
    @echo     ScDate_GetDiffYears>>$@
    @echo     ScDate_IsLeapYear>>$@
    @echo     ScDate_DaysInMonth>>$@
    @echo     ScDate_DaysInYear>>$@
    @echo     ScDate_WeeksInYear>>$@

.ENDIF

.IF "$(GUI)"=="OS2"

$(MISC)$/$(SHL1TARGET).def: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo LIBRARY     $(SHL1TARGET) INITINSTANCE TERMINSTANCE >$@
    @echo DESCRIPTION 'DateF StarCalc Addin DLL'>>$@
    @echo DATA        MULTIPLE>>$@
    @echo EXPORTS>>$@
    @echo     _GetFunctionCount>>$@
    @echo     _GetFunctionData>>$@
    @echo     _GetParameterDescription>>$@
    @echo     _SetLanguage >>$@
    @echo     _ScDate_GetDiffWeeks>>$@
    @echo     _ScDate_GetDiffMonths>>$@
    @echo     _ScDate_GetDiffYears>>$@
    @echo     _ScDate_IsLeapYear>>$@
    @echo     _ScDate_DaysInMonth>>$@
    @echo     _ScDate_DaysInYear>>$@
    @echo     _ScDate_WeeksInYear>>$@

.ENDIF


