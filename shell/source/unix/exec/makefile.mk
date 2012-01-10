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
PRJNAME=shell
TARGET=exec

LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

COMP1TYPELIST=syssh

TESTAPP1=urltest

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

DLLPRE=

SLOFILES=$(SLO)$/shellexec.obj\
    $(SLO)$/shellexecentry.obj
    
SHL1OBJS=$(SLOFILES) 
    
SHL1TARGET=syssh.uno
.IF "$(GUI)" == "OS2"
SHL1IMPLIB=i$(TARGET)
.ELSE
SHL1IMPLIB=
.ENDIF

SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
                    
SHL1STDLIBS=$(CPPULIB)\
            $(CPPUHELPERLIB)\
            $(SALLIB)
SHL1LIBS=
SHL1DEPN=

.IF "$(test)" != "" 

APP1TARGET=$(TESTAPP1)
APP1STDLIBS= $(SHL1STDLIBS)
APP1OBJS= \
    $(SLO)$/shellexec.obj \
    $(SLO)$/$(APP1TARGET).obj

.ENDIF # "$(test)" != "" 


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

run_test : $(BIN)$/$(TESTAPP1).sh
    dmake test=t
    $(BIN)$/$(TESTAPP1) urltest.txt

$(BIN)$/$(TESTAPP1).sh : $$(@:f)
    $(COPY) $< $@
    -chmod +x $@ 

ALLTAR : $(MISC)/syssh.component

$(MISC)/syssh.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        syssh.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt syssh.component
