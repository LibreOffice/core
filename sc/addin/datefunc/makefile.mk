#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 16:44:46 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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

ALL: \
    $(MISC)$/$(TARGET).lst \
    ALLTAR

.INCLUDE: target.mk

$(MISC)$/x$(TARGET).c: $(TARGET).cl $(CL2CSRC) $(MISC)$/cl2c.pl
    $(CL2C) $(TARGET).cl $(MISC)$/x$(TARGET).c $(CL2CSRC) $(CL2CRID)

$(MISC)$/cl2c.pl: ..$/util/cl2c.pl
.IF "$(GUI)"=="UNX"
    +tr -d "\015" < ..$/util$/cl2c.pl > $@
    +chmod +rw $@
.ELSE
    @+$(COPY) ..$/util$/cl2c.pl $@
.ENDIF
    
$(INCCOM)$/xlang.h : $(SOLARINCDIR)$/tools$/lang.hxx
.IF "$(GUI)"=="OS2"
    @+$(COPY) $(SOLARINCDIR)$/tools$/lang.hxx $(tmp)$/lang.hxx 
    @+$(COPY) $(tmp)$/lang.hxx $@
.ELSE
    @+$(COPY) $(SOLARINCDIR)$/tools$/lang.hxx $@
.ENDIF

$(SLOFILES) : $(INCCOM)$/xlang.h 

$(MISC)$/$(TARGET).lst : \
    $(MISC)$/x$(TARGET).c \
    $(INCCOM)$/xlang.h \
    ..$/inc$/$(TARGET).hrc \
    ..$/inc$/addin.h
.IF "$(GUI)"=="WNT"
    +echo $(<:+"\n":s/ //) > $@
.ELSE
    +echo $< > $@
.ENDIF

# --- Def-File ---

# ------------------------------------------------------------------
# Windows + OS/2 DEF File
# ------------------------------------------------------------------

.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"

$(MISC)$/$(SHL1TARGET).def: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo LIBRARY     $(SHL1TARGET)>$@
    @echo DESCRIPTION 'DateF StarCalc Addin DLL'>>$@
.IF "$(GUI)" == "WNT"
    @echo DATA        READ WRITE NONSHARED>>$@
    @echo EXPORTS>>$@
.ENDIF
.IF "$(GUI)" == "OS2"
    @echo PROTMODE                                                  >>$@
    @echo CODE        LOADONCALL                                    >>$@
    @echo DATA        PRELOAD MULTIPLE NONSHARED                    >>$@
    @echo EXPORTS                                                   >>$@
.ENDIF
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


