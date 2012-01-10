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




PRJ=..$/..$/..$/..

PRJNAME=shell
TARGET=shlxthdl

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

INCPRE+=$(MFC_INCLUDE)

# --- Files --------------------------------------------------------


RCFILES=$(RES)$/$(TARGET).rc

ULFFILES=$(TARGET).ulf

.IF "$(WITH_LANG)"!=""
ULFDIR:=$(COMMONMISC)$/$(TARGET)
.ELSE			# "$(WITH_LANG)"!=""
ULFDIR:=.
.ENDIF			# "$(WITH_LANG)"!=""

# --- Targets ------------------------------------------------------

.IF "$(BUILD_X64)"!=""
$(RES)$/x64$/$(TARGET).res : $(RES)$/$(TARGET).res
    -$(MKDIR) $(RES)$/x64
    @@cp $< $@
.ENDIF # "$(BUILD_X64)"!=""

.INCLUDE :	set_wntx64.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_wntx64.mk



# Generate the native Windows resource file
# using lngconvex.exe 

$(RCFILES) : $(ULFDIR)$/shlxthdl.ulf makefile.mk rcfooter.txt rcheader.txt rctmpl.txt
    $(subst,$(SOLARBINDIR)$/lngconvex,$(BIN)$/lngconvex $(LNGCONVEX)) -ulf $(ULFDIR)$/shlxthdl.ulf -rc $(RES)$/shlxthdl.rc -rct rctmpl.txt -rch rcheader.txt -rcf rcfooter.txt
    
