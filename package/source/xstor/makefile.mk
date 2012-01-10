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

PRJNAME=package
TARGET=xstor

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(L10N_framework)"==""

SLOFILES =  \
        $(SLO)$/ohierarchyholder.obj\
        $(SLO)$/ocompinstream.obj\
        $(SLO)$/oseekinstream.obj\
        $(SLO)$/owriteablestream.obj\
        $(SLO)$/xstorage.obj\
        $(SLO)$/xfactory.obj\
        $(SLO)$/disposelistener.obj\
        $(SLO)$/selfterminatefilestream.obj\
        $(SLO)$/switchpersistencestream.obj\
        $(SLO)$/register.obj

SHL1TARGET=$(TARGET)
SHL1STDLIBS=\
    $(SALLIB) 	\
    $(CPPULIB)	\
    $(CPPUHELPERLIB)	\
    $(COMPHELPERLIB)

SHL1OBJS=$(SLOFILES)
SHL1DEF=$(MISC)$/$(TARGET).def

SHL1IMPLIB=i$(SHL1TARGET)

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=$(SHL1TARGET).dxp

.ENDIF # L10N_framework

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk


ALLTAR : $(MISC)/xstor.component

$(MISC)/xstor.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        xstor.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt xstor.component
