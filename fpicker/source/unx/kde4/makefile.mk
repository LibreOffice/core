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



PRJ=..$/..$/..

PRJNAME=fpicker
TARGET=fps_kde4.uno
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE=

# ------------------------------------------------------------------

# Currently just KDE is supported...
.IF "$(GUIBASE)" != "unx" || "$(ENABLE_KDE4)" != "TRUE"

dummy:
    @echo "Nothing to build. GUIBASE == $(GUIBASE), ENABLE_KDE4 is not set"

.ELSE # we build for KDE

CFLAGS+= $(KDE4_CFLAGS)

# --- Files --------------------------------------------------------

SLOFILES =\
        $(SLO)$/KDE4FilePicker.obj		\
        $(SLO)$/KDE4FilePicker.moc.obj \
        $(SLO)$/KDE4FPEntry.obj

SHL1NOCHECK=TRUE
SHL1TARGET=$(TARGET)
SHL1STDLIBS=$(CPPULIB)\
        $(CPPUHELPERLIB)\
        $(SALLIB)\
        $(VCLLIB)\
        $(TOOLSLIB) \
        $(KDE4_LIBS) -lkio -lkfile


SHL1OBJS=$(SLOFILES)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1VERSIONMAP=exports.map

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

$(MISC)$/KDE4FilePicker.moc.cxx : KDE4FilePicker.hxx
    $(MOC4) $< -o $@

ALLTAR : $(MISC)/fps_kde4.component

$(MISC)/fps_kde4.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        fps_kde4.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt fps_kde4.component
