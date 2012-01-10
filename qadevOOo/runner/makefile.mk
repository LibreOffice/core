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


PRJ = ..
PRJNAME = OOoRunner
TARGET  = $(PRJNAME)

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

SUBDIRS_TESTS = mod ifc complex
SUBDIRS_RUNNER = util share stats lib complexlib helper basicrunner \
            base org$/openoffice convwatch graphical

JARCOMMANDS_TESTS = $(foreach,i,$(SUBDIRS_TESTS) -C $(CLASSDIR) $i)
JARCOMMANDS_RUNNER = $(foreach,i,$(SUBDIRS_RUNNER) -C $(CLASSDIR) $i)

# --- Targets ------------------------------------------------------
.IF "$(SOLAR_JAVA)"=="TRUE" && "$(L10N_framework)"==""
OWNJAR: ALLTAR


.INCLUDE :  target.mk

# LLA: parameter v is only verbose, need too long!
OWNJAR: LIGHT
    jar cfm $(CLASSDIR)$/$(TARGET).jar manifest -C $(PRJ) objdsc $(JARCOMMANDS_TESTS) $(JARCOMMANDS_RUNNER)

# LLA: parameter v is only verbose, need too long!
LIGHT:
    jar cfm $(CLASSDIR)$/$(TARGET)Light.jar manifest $(JARCOMMANDS_RUNNER)

.ELSE
all:
        @echo "no java"
.ENDIF
