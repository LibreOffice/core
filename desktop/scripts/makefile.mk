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

PRJNAME=desktop
TARGET=scripts

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Targets -------------------------------------------------------------

UNIXTEXT= \
    $(MISC)$/sbase.sh \
    $(MISC)$/scalc.sh \
    $(MISC)$/sdraw.sh \
    $(MISC)$/simpress.sh \
    $(MISC)$/smaster.sh \
    $(MISC)$/smath.sh \
    $(MISC)$/sweb.sh \
    $(MISC)$/swriter.sh \
    $(MISC)$/mozwrapper.sh \
    $(MISC)$/unoinfo.sh \
    $(MISC)$/unopkg.sh

.IF "$(OS)" != "MACOSX"

ALLTAR: $(MISC)$/soffice.sh-expanded

$(MISC)/soffice.sh-expanded: $(MISC)/soffice.sh
.IF "$(OS)" == "LINUX" && "$(CPUNAME)" == "POWERPC"
    $(SED) 's/^#@# //' $< > $@
.ELSE
    $(COPY) $< $@
.ENDIF

UNIXTEXT+= $(MISC)$/soffice.sh

.ENDIF

.INCLUDE :  target.mk
