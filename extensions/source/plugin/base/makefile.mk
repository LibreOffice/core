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

PRJNAME=extensions
TARGET=plbase
TARGETTYPE=GUI
ENABLE_EXCEPTIONS=TRUE

.INCLUDE :  ..$/util$/makefile.pmk

INCPRE+=-I$(SOLARINCDIR)$/mozilla$/plugin
.IF "$(SOLAR_JAVA)" != ""
INCPRE+=-I$(SOLARINCDIR)$/mozilla$/java
INCPRE+=-I$(SOLARINCDIR)$/mozilla$/nspr
CDEFS+=-DOJI
.ENDIF

.IF "$(DISABLE_XAW)" != ""
CDEFS+=-DDISABLE_XAW
.ENDIF

.IF "$(WITH_MOZILLA)" != "NO"

.IF "$(GUIBASE)"=="aqua"
CFLAGSCXX+=$(OBJCXXFLAGS)
.ENDIF  # "$(GUIBASE)"=="aqua"

SLOFILES=		\
                $(SLO)$/plctrl.obj		\
                $(SLO)$/service.obj		\
                $(SLO)$/xplugin.obj		\
                $(SLO)$/nfuncs.obj		\
                $(SLO)$/manager.obj		\
                $(SLO)$/context.obj		\
                $(SLO)$/evtlstnr.obj	\
                $(SLO)$/plcom.obj		\
                $(SLO)$/multiplx.obj    \
                $(SLO)$/plmodel.obj

.ENDIF # $(WITH_MOZILLA) != "NO"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

