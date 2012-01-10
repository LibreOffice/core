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



PRJ                 = ..$/..$/..

PRJNAME             = filter
TARGET              = filterconfig
ENABLE_EXCEPTIONS   = TRUE
VERSION             = 1
USE_DEFFILE         = TRUE
LIBTARGET           = NO

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Library -----------------------------------

SHL1TARGET=     $(TARGET)$(VERSION)

SLOFILES=       \
                $(SLO)$/configflush.obj             \
                $(SLO)$/basecontainer.obj           \
                $(SLO)$/cacheitem.obj               \
                $(SLO)$/contenthandlerfactory.obj   \
                $(SLO)$/filtercache.obj             \
                $(SLO)$/filterfactory.obj           \
                $(SLO)$/frameloaderfactory.obj      \
                $(SLO)$/lateinitlistener.obj        \
                $(SLO)$/lateinitthread.obj          \
                $(SLO)$/querytokenizer.obj          \
                $(SLO)$/registration.obj            \
                $(SLO)$/typedetection.obj           \
                $(SLO)$/cacheupdatelistener.obj

SHL1OBJS=       $(SLOFILES)

SHL1STDLIBS=    \
                $(COMPHELPERLIB)    \
                $(CPPUHELPERLIB)    \
                $(CPPULIB)          \
                $(SALLIB)           \
                $(TOOLSLIB)         \
                $(UNOTOOLSLIB)      \
                $(FWELIB)

DEF1NAME=       $(SHL1TARGET)
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def
SHL1DEPN=
SHL1IMPLIB=     i$(SHL1TARGET)
SHL1VERSIONMAP= $(SOLARENV)/src/component.map

# --- Targets ----------------------------------

.INCLUDE : target.mk

ALLTAR : $(MISC)/filterconfig1.component

$(MISC)/filterconfig1.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt filterconfig1.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt filterconfig1.component
