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
PRJNAME=avmediagst
TARGET=avmediagst

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

.IF "$(verbose)"!="" || "$(VERBOSE)"!=""
CDEFS+= -DVERBOSE
.ENDIF

# --- Files ----------------------------------

.IF "$(GUI)" == "UNX"  && "$(GUIBASE)"!="aqua" && "$(ENABLE_GSTREAMER)" != ""

PKGCONFIG_MODULES=gtk+-2.0 gstreamer-0.10 gstreamer-interfaces-0.10
.INCLUDE : pkg_config.mk

SLOFILES= \
        $(SLO)$/gstuno.obj		\
        $(SLO)$/gstmanager.obj		\
        $(SLO)$/gstwindow.obj		\
        $(SLO)$/gstplayer.obj		\
        $(SLO)$/gstframegrabber.obj

EXCEPTIONSFILES= \
        $(SLO)$/gstuno.obj		\
        $(SLO)$/gstplayer.obj		\
        $(SLO)$/gstframegrabber.obj

SHL1TARGET=$(TARGET)
SHL1STDLIBS= $(CPPULIB) $(SALLIB) $(COMPHELPERLIB) $(CPPUHELPERLIB) $(TOOLSLIB) $(VCLLIB)
SHL1STDLIBS+=$(PKGCONFIG_LIBS)
SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
# on Solaris checkdll does not work: LD_LIBRARY_PATH breaks the 2 libxml2.so.2
SHL1NOCHECK=t

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

.ENDIF

.INCLUDE :  	target.mk

ALLTAR : $(MISC)/avmediagst.component

$(MISC)/avmediagst.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        avmediagst.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt avmediagst.component
