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

PRJNAME=extensions
TARGET=oooimprovement
ENABLE_EXCEPTIONS=TRUE

PACKAGE=org.openoffice.Office
ABSXCSROOT=$(SOLARXMLDIR)
XCSROOT=..
DTDDIR=$(ABSXCSROOT)
XSLDIR=$(ABSXCSROOT)$/processing
PROCESSOUT=$(MISC)$/$(TARGET)
PROCESSORDIR=$(SOLARBINDIR)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES= \
    $(SLO)$/myconfigurationhelper.obj \
    $(SLO)$/config.obj \
    $(SLO)$/corecontroller.obj \
    $(SLO)$/errormail.obj \
    $(SLO)$/invite_job.obj \
    $(SLO)$/logpacker.obj \
    $(SLO)$/logstorage.obj \
    $(SLO)$/onlogrotate_job.obj \
    $(SLO)$/oooimprovement_exports.obj \
    $(SLO)$/soaprequest.obj \
    $(SLO)$/soapsender.obj \

SHL1STDLIBS= \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB) \
        $(UNOTOOLSLIB) \

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
.IF "$(GUI)"=="OS2"
SHL1TARGET= oooimp$(DLLPOSTFIX)
.ENDIF

SHL1LIBS= $(SLB)$/$(TARGET).lib
SHL1DEF= $(MISC)$/$(SHL1TARGET).def
DEF1NAME= $(SHL1TARGET)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

PACKAGEDIR=$(subst,.,$/ $(PACKAGE))
SPOOLDIR=$(MISC)$/registry$/spool

XCUFILES= \
    Jobs.xcu \
    Logging.xcu

MYXCUFILES= \
    $(SPOOLDIR)$/$(PACKAGEDIR)$/Jobs$/Jobs-oooimprovement.xcu \
    $(SPOOLDIR)$/$(PACKAGEDIR)$/Logging$/Logging-oooimprovement.xcu


# --- Targets ---

.INCLUDE : target.mk

ALLTAR : $(MYXCUFILES)

XCU_SOURCEDIR:=.

$(SPOOLDIR)$/$(PACKAGEDIR)$/Jobs$/Jobs-oooimprovement.xcu : $(XCU_SOURCEDIR)$/Jobs.xcu
    @-$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@

$(SPOOLDIR)$/$(PACKAGEDIR)$/Logging$/Logging-oooimprovement.xcu : $(XCU_SOURCEDIR)$/Logging.xcu
    @-$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@

ALLTAR : $(MISC)/oooimprovement.component

$(MISC)/oooimprovement.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt oooimprovement.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt oooimprovement.component
