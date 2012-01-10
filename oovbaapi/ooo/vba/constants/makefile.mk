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

PRJNAME=oovbapi

TARGET=constants
PACKAGE=org$/vba$/constants

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(L10N_framework)"==""
# ------------------------------------------------------------------------
# I tried to use the IDLFILES target but it failed dismally
MY_GEN_IDL_PATH=$(MISC)$/idl
MY_GEN_UCR_PATH=$(OUT)$/ucr$/$(PACKAGE)
MYTMPIDLFILES=$(shell @ls $(MY_GEN_IDL_PATH))
#IDLFILES=$(foreach,i,$(MYTMPIDLFILES) $(MY_GEN_IDL_PATH)$/$(i))
MYIDLFILES=$(foreach,i,$(MYTMPIDLFILES) $(MY_GEN_IDL_PATH)$/$(i))
MYURDFILES=$(foreach,i,$(MYIDLFILES) $(MY_GEN_UCR_PATH)$/$(i:b).urd)
MYDBTARGET=$(OUT)$/ucr/constants.db
.ENDIF

.INCLUDE :  target.mk

.IF "$(L10N_framework)"==""
ALLTAR: $(MYDBTARGET)

$(MY_GEN_UCR_PATH)$/%.urd: $(MY_GEN_IDL_PATH)$/%.idl
    $(IDLC) -C -O $(MY_GEN_UCR_PATH) -I $(MY_GEN_IDL_PATH) -I $(SOLARIDLDIR) $?

$(MYDBTARGET) : $(MYURDFILES)  $(MYIDLFILES)
    $(REGMERGE) $(OUT)$/ucr/constants.db /UCR @$(mktmp $(MYURDFILES))

.ENDIF
