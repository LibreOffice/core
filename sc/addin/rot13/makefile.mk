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
