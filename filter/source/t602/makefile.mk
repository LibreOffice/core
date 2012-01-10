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


PRJ	= ..$/..
PRJNAME = filter
TARGET  =t602filter
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk

SRS1NAME = $(TARGET)
SRC1FILES = $(SRS1NAME).src

RESLIB1NAME=$(SRS1NAME)
RESLIB1SRSFILES= $(SRS)$/$(RESLIB1NAME).srs

SLOFILES=$(SLO)$/t602filter.obj \
         $(SLO)$/filterenv.obj

LIBNAME=$(TARGET)
SHL1TARGETDEPN=makefile.mk
SHL1OBJS=$(SLOFILES)
SHL1TARGET=$(LIBNAME)$(DLLPOSTFIX)
SHL1IMPLIB=i$(LIBNAME)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
#SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1STDLIBS= \
    $(XMLOFFLIB) \
    $(CPPULIB) \
    $(CPPUHELPERLIB) \
    $(SALLIB) \
    $(TOOLSLIB)

# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk

ALLTAR : $(MISC)/t602filter.component

$(MISC)/t602filter.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        t602filter.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt t602filter.component
