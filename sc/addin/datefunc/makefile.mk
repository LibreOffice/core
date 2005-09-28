#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: hr $ $Date: 2005-09-28 15:29:48 $
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

SHL1TARGET= $(TARGET)$(UPD)$(DLLPOSTFIX)
SHL1IMPLIB= i$(TARGET)
SHL1OBJS=   $(SLO)$/x$(TARGET).obj
SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

.IF "$(GUI)" == "WNT"
SHL1STDLIBS=     gdi32.lib advapi32.lib comdlg32.lib \
                 uuid.lib ole32.lib shell32.lib winspool.lib
.IF "$(GVER)" == "W40"
SHL1STDLIBS=    $(SHL1STDLIBS) comctl32.lib
.ENDIF
.ENDIF

# --- Targets -------------------------------------------------------

ALLTAR:	$(MISC)$/$(TARGET).lst 
    

.INCLUDE: target.mk

$(MISC)$/x$(TARGET).c: $(TARGET).cl $(CL2CSRC) $(MISC)$/cl2c.pl
    $(CL2C) $(TARGET).cl $(MISC)$/x$(TARGET).c $(CL2CSRC) $(CL2CRID)

$(MISC)$/cl2c.pl: ..$/util/cl2c.pl
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
    +tr -d "\015" < ..$/util$/cl2c.pl > $@
    +chmod +rw $@
.ELSE
    @+$(COPY) ..$/util$/cl2c.pl $@
.ENDIF
    
# convert C++ //... comments to C /*...*/ comments without affecting http://...
$(INCCOM)$/xlang.h : $(SOLARINCDIR)$/tools$/lang.hxx
    @+$(SED) -e "s#\([ \t]\)//\(.*\)#\1/*\2 */#" -e "s#^//\(.*\)#/*\1 */#" $(SOLARINCDIR)$/tools$/lang.hxx >$@

$(SLOFILES) : $(INCCOM)$/xlang.h $(CFILES)

$(MISC)$/$(TARGET).lst : \
    $(MISC)$/x$(TARGET).c \
    $(INCCOM)$/xlang.h \
    ..$/inc$/$(TARGET).hrc \
    ..$/inc$/addin.h
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
    +echo $< > $@
.ELSE
    +echo $(<:+"\n":s/ //) > $@
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
    @echo DESCRIPTION 'DateF StarCalc Addin DLL'>>$@
    @echo DATA        READ WRITE NONSHARED>>$@
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


