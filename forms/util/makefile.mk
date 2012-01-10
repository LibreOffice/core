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



PRJ=..
PRJNAME=forms
TARGET=frm
USE_DEFFILE=TRUE

# --- Settings ----------------------------------

.INCLUDE :	settings.mk
.INCLUDE: $(PRJ)$/makefile.pmk

# --- Library -----------------------------------
# --- frm ---------------------------------------
LIB1TARGET=$(SLB)$/forms.lib
LIB1FILES=\
        $(SLB)$/common.lib \
        $(SLB)$/resource.lib \
        $(SLB)$/component.lib \
        $(SLB)$/helper.lib \
        $(SLB)$/solarcomponent.lib  \
        $(SLB)$/solarcontrol.lib \
        $(SLB)$/richtext.lib \
        $(SLB)$/runtime.lib \
        $(SLB)$/xforms.lib \
        $(SLB)$/xformssubmit.lib \
        $(SLB)$/xformsxpath.lib

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(EDITENGLIB) \
        $(SALLIB) \
        $(CPPULIB) \
        $(CPPUHELPERLIB) \
        $(TOOLSLIB) \
        $(I18NISOLANGLIB) \
        $(VCLLIB) \
        $(SVTOOLLIB) \
        $(SVLLIB)	\
        $(TKLIB) \
        $(SFX2LIB) \
        $(VOSLIB) \
        $(UNOTOOLSLIB) \
        $(COMPHELPERLIB) \
        $(DBTOOLSLIB) \
        $(TKLIB) \
        $(SVXCORELIB) \
        $(UCBHELPERLIB) \
        $(LIBXML2LIB) \
        $(ICUUCLIB) \
        $(ICUINLIB)

SHL1LIBS=$(LIB1TARGET)
SHL1DEPN=$(LIB1TARGET)	\
        makefile.mk


SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

# === .res file ==========================================================

RES1FILELIST=\
    $(SRS)$/resource.srs \

RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES=$(RES1FILELIST)

.IF "$(GUI)"=="OS2"
SHL1STDLIBS += pthread.lib libz.lib
.ENDIF

# --- Targets ----------------------------------

.INCLUDE : target.mk

# --- Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo __CT				    >$@
    @echo createRegistryInfo    >>$@
    @echo queryInterface        >>$@
    @echo queryAggregation      >>$@
    @echo NavigationToolBar     >>$@
    @echo ONavigationBar        >>$@


ALLTAR : $(MISC)/frm.component

$(MISC)/frm.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        frm.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt frm.component
