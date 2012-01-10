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
TARGET=pl
TARGETTYPE=GUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(WITH_MOZILLA)" != "NO"

LIB1TARGET = $(SLB)$/plall.lib
LIB1FILES  = \
    $(SLB)$/plbase.lib	\
    $(SHL1LINKLIB)

.IF "$(GUI)" == "UNX"
.IF "$(GUIBASE)"=="aqua"
.IF "$(WITH_MOZILLA)"=="YES"
SHL1LINKLIB = $(SLB)$/plaqua.lib
.ENDIF
.ELSE
SHL1LINKLIB = $(SLB)$/plunx.lib
.ENDIF # $(GUIBASE)==aqua
.IF "$(OS)" == "SOLARIS"
SHL1OWNLIBS = -lsocket
.ENDIF # SOLARIS
.ENDIF # UNX

.IF "$(GUI)" == "WNT"
SHL1LINKLIB = $(SLB)$/plwin.lib
SHL1OWNLIBS = \
    $(VERSIONLIB)	\
    $(OLE32LIB)	\
    $(ADVAPI32LIB)
.ENDIF # WNT

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB= i$(TARGET)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1LIBS=$(LIB1TARGET)

.IF "$(OS)"=="MACOSX"
SHL1STDLIBS= \
    $(LIBSTLPORT)		\
    $(TKLIB)
.ELSE
SHL1STDLIBS= \
    $(TKLIB)
.ENDIF

SHL1STDLIBS+= \
    $(VCLLIB)			\
    $(SVLLIB)			\
        $(UNOTOOLSLIB)                     \
    $(TOOLSLIB)			\
    $(VOSLIB)			\
    $(UCBHELPERLIB)		\
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(SALLIB)

.IF "$(GUIBASE)"=="unx"
SHL1STDLIBS+=$(X11LINK_DYNAMIC)
.ENDIF

SHL1STDLIBS+=$(SHL1OWNLIBS)

.ENDIF # $(WITH_MOZILLA) != "NO"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk




ALLTAR : $(MISC)/pl.component

$(MISC)/pl.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        pl.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt pl.component
