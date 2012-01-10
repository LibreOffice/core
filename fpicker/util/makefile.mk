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

PRJNAME=fpicker
TARGET=fpicker
TARGET1=fps
TARGET2=fop
USE_LDUMP2=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- fps dynlib ----------------------------------------------

COMMON_LIBS=$(CPPULIB)\
            $(CPPUHELPERLIB)\
            $(COMPHELPERLIB)\
            $(SALLIB)\
            $(VCLLIB)\
            $(TOOLSLIB)

.IF "$(GUI)"=="WNT"

SHL1TARGET=$(TARGET1)
SHL1STDLIBS=		$(COMMON_LIBS) \
            $(UWINAPILIB) \
            $(ADVAPI32LIB) \
            $(SHELL32LIB)\
            $(OLE32LIB)\
            $(GDI32LIB)\
            $(OLEAUT32LIB)\
            $(COMDLG32LIB)\
            $(KERNEL32LIB)\
            $(UUIDLIB)

SHL1DEPN=
SHL1IMPLIB=i$(SHL1TARGET)
SHL1LIBS=$(SLB)$/fps.lib\
         $(SLB)$/utils.lib
SHL1RES=$(RES)$/$(TARGET1).res
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

.ENDIF # "$(GUI)"=="WNT"

# --- fop dynlib --------------------------------------------------

.IF "$(GUI)"=="WNT"
SHL2NOCHECK=TRUE
SHL2TARGET=$(TARGET2)

SHL2STDLIBS=		$(COMMON_LIBS) \
            $(UWINAPILIB) \
            $(ADVAPI32LIB) \
            $(OLE32LIB)\
            $(GDI32LIB)\
            $(SHELL32LIB)\
            $(OLEAUT32LIB)

SHL2DEPN=
SHL2IMPLIB=i$(SHL2TARGET)
SHL2LIBS=$(SLB)$/fop.lib\
         $(SLB)$/utils.lib
SHL2DEF=$(MISC)$/$(SHL2TARGET).def

DEF2NAME=$(SHL2TARGET)
DEF2EXPORTFILE=	exports.dxp

.ENDIF          # "$(GUI)"=="WNT"


.INCLUDE :  target.mk

ALLTAR : $(MISC)/fop.component $(MISC)/fps.component

$(MISC)/fop.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        fop.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL2TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt fop.component

$(MISC)/fps.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        fps.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt fps.component
