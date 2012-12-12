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



PRJ=..$/..$/..$/..$/..
PRJNAME=setup_native
TARGET=data_registration
PACKAGE=org.openoffice.Office

ABSXCSROOT=$(SOLARXMLDIR)
XCSROOT=..
DTDDIR=$(ABSXCSROOT)
XSLDIR=$(ABSXCSROOT)$/processing
PROCESSOUT=$(MISC)$/$(TARGET)
PROCESSORDIR=$(SOLARBINDIR)

.INCLUDE :  settings.mk
.IF "$(L10N_framework)"==""

# --- Files  -------------------------------------------------------

.IF "$(ENABLE_OOOIMPROVEMENT)"!=""

XCUFILES= \
    Jobs.xcu

MODULEFILES=

LOCALIZEDFILES=

PACKAGEDIR=$(subst,.,$/ $(PACKAGE))
SPOOLDIR=$(MISC)$/registry$/spool

MYXCUFILES= \
    $(SPOOLDIR)$/$(PACKAGEDIR)$/Jobs$/Jobs-registration.xcu

.ELSE # "$(ENABLE_OOOIMPROVEMENT)"!=""

dummy:
    @echo "Nothing to build"

.ENDIF # "$(ENABLE_OOOIMPROVEMENT)"!=""

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.ENDIF # L10N_framework
ALLTAR : $(MYXCUFILES)

$(SPOOLDIR)$/$(PACKAGEDIR)$/Jobs$/Jobs-registration.xcu : $(PROCESSOUT)$/registry$/data$/$/$(PACKAGEDIR)$/Jobs.xcu
    @-$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@

