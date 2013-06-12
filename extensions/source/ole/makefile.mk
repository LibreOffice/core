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

# -----------------------------------------------------------------

PRJNAME=extensions
TARGET=oleautobridge.uno
TARGET2=oleautobridge2.uno

ENABLE_EXCEPTIONS=TRUE

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

LIBTARGET=NO
USE_DEFFILE=YES

USE_DEFFILE=TRUE

INCPRE+= $(foreach,i,$(ATL_INCLUDE) -I$(i))

# --- Settings -----------------------------------------------------

.IF "$(GUI)" == "WNT" && "$(DISABLE_ATL)"==""

SLOFILES= \
            $(SLO)$/servreg.obj		\
            $(SLO)$/servprov.obj	\
            $(SLO)$/unoobjw.obj		\
            $(SLO)$/oleobjw.obj		\
            $(SLO)$/olethread.obj	\
            $(SLO)$/oledll.obj		\
        $(SLO)$/jscriptclasses.obj	\
        $(SLO)$/ole2uno.obj		\
        $(SLO)$/windata.obj		\
        $(SLO)$/unotypewrapper.obj

SECOND_BUILD=OWNGUID
OWNGUID_SLOFILES=$(SLOFILES)
OWNGUIDCDEFS+= -DOWNGUID

# the original library
SHL1TARGET=$(TARGET)
SHL1STDLIBS=\
        $(SALLIB) 	\
    $(VOSLIB)	\
    $(CPPULIB)	\
    $(CPPUHELPERLIB)	\
    $(OLE32LIB) 	\
    $(UUIDLIB) 	\
    $(ADVAPI32LIB)	\
    $(OLEAUT32LIB)

.IF "$(COM)"=="MSC"
.IF "$(WINDOWS_VISTA_PSDK)"!="" || "$(CCNUMVER)"<="001399999999"
    SHL1STDLIBS+= $(ATL_LIB)$/atls.lib
.ENDIF # "$(WINDOWS_VISTA_PSDK)"!="" || "$(CCNUMVER)"<="001399999999"
.ENDIF # "$(COM)"=="MSC"

SHL1LIBS=
SHL1OBJS=$(SLOFILES)

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=$(SHL1TARGET).dxp

# the second library
SHL2TARGET=$(TARGET2)
SHL2STDLIBS=\
        $(SALLIB) 	\
    $(VOSLIB)	\
    $(CPPULIB)	\
    $(CPPUHELPERLIB)	\
    $(OLE32LIB) 	\
    $(UUIDLIB) 	\
    $(ADVAPI32LIB)	\
    $(OLEAUT32LIB)

.IF "$(COM)"=="MSC"
.IF "$(WINDOWS_VISTA_PSDK)"!="" || "$(CCNUMVER)"<="001399999999"
    SHL2STDLIBS+= $(ATL_LIB)$/atls.lib
.ENDIF # "$(WINDOWS_VISTA_PSDK)"!="" || "$(CCNUMVER)"<="001399999999"
.ENDIF # "$(COM)"=="MSC"

SHL2LIBS=
SHL2OBJS=$(REAL_OWNGUID_SLOFILES)

DEF2NAME=$(SHL2TARGET)
DEF2EXPORTFILE=$(TARGET).dxp

.ENDIF

#----------------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : $(MISC)/oleautobridge.component

$(MISC)/oleautobridge.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt oleautobridge.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt oleautobridge.component
