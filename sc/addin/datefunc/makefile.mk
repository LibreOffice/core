#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 17:10:17 $
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


