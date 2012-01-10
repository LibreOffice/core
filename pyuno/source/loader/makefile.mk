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

PRJNAME=pyuno
TARGET=pythonloader.uno
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.IF "$(L10N_framework)"==""
DLLPRE = 

#-------------------------------------------------------------------

.IF "$(OS)$(COMEX)" == "SOLARIS4"
# no -Bdirect for SunWS CC
DIRECT = $(LINKFLAGSDEFS)
.ENDIF

.IF "$(SYSTEM_PYTHON)" == "YES"
PYTHONLIB=$(PYTHON_LIBS)
CFLAGS+=$(PYTHON_CFLAGS)
.IF "$(EXTRA_CFLAGS)"!=""
PYTHONLIB+=-framework Python
.ENDIF # "$(EXTRA_CFLAGS)"!=""
.ELSE
.INCLUDE :  pyversion.mk

CFLAGS+=-I$(SOLARINCDIR)$/python
.ENDIF

SHL1TARGET=	$(TARGET)

SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)		\
        $(PYUNOLIB)		\
        $(PYTHONLIB)

SHL1VERSIONMAP=$(SOLARENV)$/src$/component.map
SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
SLOFILES=       $(SLO)$/pyuno_loader.obj

# --- Targets ------------------------------------------------------

ALL : ALLTAR \
    $(DLLDEST)$/pythonloader.py
.ENDIF # L10N_framework

.INCLUDE :  target.mk
.IF "$(L10N_framework)"==""
$(DLLDEST)$/%.py: %.py
    cp $? $@
.ENDIF # L10N_framework

ALLTAR : $(MISC)/pythonloader.component

$(MISC)/pythonloader.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt pythonloader.component
    $(XSLTPROC) --nonet --stringparam uri \
        'vnd.sun.star.expand:$$OOO_BASE_DIR/program/$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt pythonloader.component
