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

PRJNAME=io
TARGET = acceptor.uno
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE
COMP1TYPELIST = acceptor
COMPRDB=$(SOLARBINDIR)$/udkapi.rdb

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk
.IF "$(L10N_framework)"==""
DLLPRE =
# ------------------------------------------------------------------

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb
UNOUCROUT=$(OUT)$/inc$/acceptor
INCPRE+= $(UNOUCROUT)


SLOFILES= \
        $(SLO)$/acceptor.obj \
        $(SLO)$/acc_pipe.obj \
        $(SLO)$/acc_socket.obj

SHL1TARGET= $(TARGET)
SHL1VERSIONMAP = $(SOLARENV)/src/unloadablecomponent.map
SHL1RPATH=URELIB

SHL1STDLIBS= \
        $(SALLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB) 

SHL1DEPN=
SHL1IMPLIB=		i$(TARGET)
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)

# --- Targets ------------------------------------------------------
.ENDIF 		# L10N_framework

.INCLUDE :	target.mk

ALLTAR : $(MISC)/acceptor.component

$(MISC)/acceptor.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        acceptor.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_URE_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt acceptor.component
