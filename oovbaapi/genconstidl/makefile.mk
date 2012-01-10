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


TARGET=genconstidl
PRJ=..
PRJNAME=oovbaapi

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

.IF "$(L10N_framework)"==""

# symbol files of Microsoft API
MYSYMFILES = access.api vba.api adodb.api msforms.api word.api dao.api powerpoint.api excel.api stdole.api

# ------------------------------------------------------------------

MY_GEN_IDL_PATH=$(MISC)$/idl

MYDONEFILES += $(foreach,i,$(MYSYMFILES) $(MISC)$/$(i:b).done)

# --- Targets ------------------------------------------------------

ALLTAR: GENIDL

GENIDL : $(MYDONEFILES)

GENRDB : GENIDL $(MYURDFILES)

$(MISC)$/%.done : %.api
    @@-$(MKDIR) $(MY_GEN_IDL_PATH)
    @echo Processing $?
    $(PERL) api-to-idl.pl $? $(MY_GEN_IDL_PATH)
    @$(TOUCH) $@

CLEAN :
    @$(RM) $(MY_GEN_IDL_PATH)$/*.idl
    @$(RM) $(foreach,i,$(MYSYMFILES) $(MISC)$/$(i:b).done)

.ELSE
ivo:
    $(ECHO)
.ENDIF # L10N_framework
