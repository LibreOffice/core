#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
    echo $< > $@

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


