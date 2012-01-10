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



PRJ=..

PRJNAME=oox
TARGET=oox
USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.IF "$(L10N_framework)"==""
# --- Allgemein ----------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=	\
    $(SLB)$/token.lib\
    $(SLB)$/helper.lib\
    $(SLB)$/core.lib\
    $(SLB)$/ole.lib\
    $(SLB)$/ppt.lib\
    $(SLB)$/xls.lib\
    $(SLB)$/vml.lib\
    $(SLB)$/drawingml.lib\
    $(SLB)$/diagram.lib\
    $(SLB)$/chart.lib\
    $(SLB)$/table.lib\
    $(SLB)$/shape.lib\
    $(SLB)$/dump.lib\
    $(SLB)$/docprop.lib

# --- Shared-Library -----------------------------------------------

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB= i$(TARGET)
SHL1USE_EXPORTS=name

SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)\
        $(COMPHELPERLIB)\
        $(RTLLIB)		\
        $(SALLIB)		\
        $(BASEGFXLIB)	\
        $(SAXLIB)       \
        $(XMLSCRIPTLIB)

# link openssl, copied this bit from ucb/source/ucp/webdav/makefile.mk
.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= $(OPENSSLLIB)
.ELSE # WNT
.IF "$(OS)"=="SOLARIS"
SHL1STDLIBS+= -lnsl -lsocket -ldl
.ENDIF # SOLARIS
.IF "$(SYSTEM_OPENSSL)"=="YES"
SHL1STDLIBS+= $(OPENSSLLIB)
.ELSE
SHL1STDLIBS+= $(OPENSSLLIBST)
.ENDIF
.ENDIF # WNT

SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
SHL1LIBS=   $(LIB1TARGET)
DEF1NAME    =$(SHL1TARGET)
DEFLIB1NAME =$(TARGET)

# --- Targets ----------------------------------------------------------
.ENDIF # L10N_framework

.INCLUDE :  target.mk

ALLTAR : $(MISC)/oox.component

$(MISC)/oox.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        oox.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt oox.component
