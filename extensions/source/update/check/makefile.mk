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

PRJNAME=extensions
TARGET=updchk
PACKAGE=org.openoffice.Office

LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

ABSXCSROOT=$(SOLARXMLDIR)
XCSROOT=..
DTDDIR=$(ABSXCSROOT)
XSLDIR=$(ABSXCSROOT)$/processing
PROCESSOUT=$(MISC)$/$(TARGET)
PROCESSORDIR=$(SOLARBINDIR)

# no validation by inspector class
NO_INSPECTION=TRUE

# --- Settings ---

.INCLUDE : settings.mk

# no "lib" prefix
DLLPRE =

# --- Files ---

SRS2NAME=$(TARGET)
SRC2FILES=\
    updatehdl.src

RESLIB2NAME=upd
RESLIB2SRSFILES= $(SRS)$/updchk.srs
RESLIB2DEPN= updatehdl.src updatehdl.hrc

SLOFILES=\
    $(SLO)$/download.obj \
    $(SLO)$/updatecheck.obj \
    $(SLO)$/updatecheckjob.obj \
    $(SLO)$/updatecheckconfig.obj \
    $(SLO)$/updateprotocol.obj \
    $(SLO)$/updatehdl.obj
        
SHL1NOCHECK=TRUE
SHL1TARGET=$(TARGET).uno   
SHL1OBJS=$(SLOFILES)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1IMPLIB=i$(SHL1TARGET)
SHL1STDLIBS=    \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(CURLLIB) \
        $(SALLIB) \
        $(SHELL32LIB) \
        $(OLE32LIB)
        
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

PACKAGEDIR=$(subst,.,$/ $(PACKAGE))
SPOOLDIR=$(MISC)$/registry$/spool

XCUFILES= \
    Addons.xcu \
    Jobs.xcu

MYXCUFILES= \
    $(SPOOLDIR)$/$(PACKAGEDIR)$/Addons$/Addons-onlineupdate.xcu \
    $(SPOOLDIR)$/$(PACKAGEDIR)$/Jobs$/Jobs-onlineupdate.xcu

LOCALIZEDFILES= \
    Addons.xcu \
    Jobs.xcu

.IF "$(test)" != ""
APP1TARGET=updateprotocoltest
APP1STDLIBS= $(SHL1STDLIBS)
APP1OBJS= \
    $(SLO)$/updateprotocol.obj \
    $(SLO)$/updateprotocoltest.obj


.ENDIF # "$(test)" != ""

# --- Targets ---

.INCLUDE : target.mk

ALLTAR : $(MYXCUFILES)

.IF "$(WITH_LANG)"!=""
XCU_SOURCEDIR:=$(PROCESSOUT)$/merge$/$(PACKAGEDIR)
.ELSE			# "$(WITH_LANG)"!=""
XCU_SOURCEDIR:=.
.ENDIF			# "$(WITH_LANG)"!=""

$(SPOOLDIR)$/$(PACKAGEDIR)$/Addons$/Addons-onlineupdate.xcu : $(XCU_SOURCEDIR)$/Addons.xcu
    @-$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@

$(SPOOLDIR)$/$(PACKAGEDIR)$/Jobs$/Jobs-onlineupdate.xcu : $(XCU_SOURCEDIR)$/Jobs.xcu
    @-$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@
#	@$(PERL) transform.pl < $< > $@


ALLTAR : $(MISC)/updchk.uno.component

$(MISC)/updchk.uno.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        updchk.uno.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt updchk.uno.component
