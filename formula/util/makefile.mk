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
PRJNAME=formula
TARGET=for
TARGET2=forui
USE_DEFFILE=TRUE
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

# USE_LDUMP2=TRUE
# --- Settings ----------------------------------
.INCLUDE :  settings.mk


# --- formula core (for) -----------------------------------

LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1FILES=\
        $(SLB)$/core_resource.lib \
        $(SLB)$/core_api.lib

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(TOOLSLIB) 			\
        $(SVTOOLLIB)			\
                $(SVLLIB) \
        $(UNOTOOLSLIB)			\
        $(COMPHELPERLIB) 		\
        $(CPPUHELPERLIB) 		\
        $(CPPULIB)				\
        $(VCLLIB)				\
        $(SALLIB)

SHL1IMPLIB=i$(TARGET)
SHL1USE_EXPORTS=name
SHL1LIBS=$(LIB1TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEFLIB1NAME=$(TARGET)

# --- .res file ----------------------------------------------------------

RES1FILELIST=\
    $(SRS)$/core_strings.srs

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=$(RES1FILELIST)


# --- formula UI core (forui) -----------------------------------
LIB2TARGET=$(SLB)$/$(TARGET2).lib
LIB2FILES=\
        $(SLB)$/uidlg.lib \
        $(SLB)$/ui_resource.lib

SHL2TARGET=$(TARGET2)$(DLLPOSTFIX)

SHL2STDLIBS= \
        $(SFXLIB)				\
        $(SVTOOLLIB)			\
        $(SVLLIB)  \
        $(VCLLIB)				\
        $(UNOTOOLSLIB)			\
        $(TOOLSLIB)				\
        $(CPPULIB)				\
        $(SO2LIB)				\
        $(CPPUHELPERLIB)        \
        $(SALLIB)
        
.IF "$(GUI)"!="WNT" || "$(COM)"=="GCC"
SHL2STDLIBS+= \
        -lfor$(DLLPOSTFIX)
SHL2DEPN=$(SHL1TARGETN)
.ELSE
SHL2STDLIBS+= \
        $(LB)$/ifor.lib
SHL2DEPN=$(LB)$/i$(TARGET).lib
.ENDIF


SHL2IMPLIB=i$(TARGET2)
SHL2LIBS=$(LIB2TARGET)
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
DEF2NAME=$(SHL2TARGET)
.IF "$(GUI)"=="OS2"
DEFLIB2NAME=$(TARGET2)
.ENDIF

SHL2USE_EXPORTS=name

# --- .res file ----------------------------------------------------------

RES2FILELIST=\
    $(SRS)$/uidlg.srs


RESLIB2NAME=$(TARGET2)
RESLIB2IMAGES=$(PRJ)$/res
RESLIB2SRSFILES=$(RES2FILELIST)


# --- Targets ----------------------------------

.INCLUDE : target.mk


ALLTAR : $(MISC)/for.component

$(MISC)/for.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        for.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt for.component
