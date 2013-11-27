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
PRJNAME=winaccessibility
TARGET=UAccCOM

# --- Settings -----------------------------------------------------
.IF "$(GUI)"!="WNT" || "$(DISABLE_ATL)"!=""
all:
    @echo "$TARGET will not be built because GUI='$(GUI)' and DISABLE_ATL='$(DISABLE_ATL)'"
.ELSE

PROF_EDITION=TRUE
.INCLUDE :	settings.mk

VERSIONOBJ=
LIBTARGET=NO
USE_DEFFILE=YES
UWINAPILIB=

INCPRE+=$(foreach,i,$(ATL_INCLUDE) -I$(i)) \
    -I$(MISC) \

CFLAGS+=-D_UNICODE -DUNICODE

# --- Files --------------------------------------------------------
.IF "$(PRODUCT)"!=""
RC+=-DPRODUCT
.ENDIF

RCFILES=\
        $(TARGET).rc
#RCDEPN=$(MISC)$/envsettings.h

SLOFILES=\
    $(SLO)$/AccAction.obj \
    $(SLO)$/AccActionBase.obj \
    $(SLO)$/AccComponent.obj \
    $(SLO)$/AccComponentBase.obj \
    $(SLO)$/AccEditableText.obj \
    $(SLO)$/AccHyperLink.obj \
    $(SLO)$/AccHypertext.obj \
    $(SLO)$/AccImage.obj \
    $(SLO)$/AccRelation.obj \
    $(SLO)$/AccText.obj \
    $(SLO)$/AccValue.obj \
    $(SLO)$/EnumVariant.obj \
    $(SLO)$/StdAfx.obj \
    $(SLO)$/UAccCOM.obj \
    $(SLO)$/UNOXWrapper.obj \
    $(SLO)$/AccTable.obj \
    $(SLO)$/AccTextBase.obj \
    $(SLO)$/MAccessible.obj \
    $(SLO)$/CheckEnableAccessible.obj \
    
EXCEPTIONSFILES= \
    $(SLO)$/AccAction.obj \
    $(SLO)$/AccActionBase.obj \
    $(SLO)$/AccComponent.obj \
    $(SLO)$/AccComponentBase.obj \
    $(SLO)$/AccEditableText.obj \
    $(SLO)$/AccHyperLink.obj \
    $(SLO)$/AccHypertext.obj \
    $(SLO)$/AccImage.obj \
    $(SLO)$/AccRelation.obj \
    $(SLO)$/AccText.obj \
    $(SLO)$/AccValue.obj \
    $(SLO)$/EnumVariant.obj \
    $(SLO)$/StdAfx.obj \
    $(SLO)$/UAccCOM.obj \
    $(SLO)$/UNOXWrapper.obj \
    $(SLO)$/AccTable.obj \
    $(SLO)$/AccTextBase.obj \
    $(SLO)$/MAccessible.obj \

SHL1TARGET=$(TARGET)
SHL1STDLIBS=\
    $(ICUUCLIB)\
    $(ICUINLIB)\
    $(CPPULIB)\
    $(VCLLIB) \
    $(SALLIB)\
    $(KERNEL32LIB) \
    $(USER32LIB) \
    $(ADVAPI32LIB) \
    $(OLE32LIB) \
    $(OLEAUT32LIB) \
    $(SHLWAPILIB) \
    $(UUIDLIB) \
    oleacc.lib
    
.IF "$(COM)"!="GCC"
.IF "$(CCNUMVER)" > "001300000000"
    SHL1STDLIBS+= $(ATL_LIB)$/atls.lib
.ENDIF
.ENDIF 
    
SHL1OBJS=$(SLOFILES)
SHL1DEF=$(TARGET).def 
SHL1RES=$(RES)/$(TARGET).res 

DEF1NAME= $(TARGET)
#DEF1EXPORTFILE=	exports.dxp

# --- Targets ----------------------------------
.INCLUDE : target.mk

ALLTAR : \
    $(MISC)/$(TARGET).manifest \

$(MISC)/$(TARGET).manifest: $(BIN)$/$(TARGET)$(DLLPOST)
    cat *.rgs > $(MISC)$/$(TARGET).rgs
    mt.exe -rgs:$(MISC)$/$(TARGET).rgs -tlb:$(MISC)$/$(TARGET).tlb -dll:$(TARGET).dll -out:$(MISC)$/$(TARGET).manifest
    mt.exe -manifest $(MISC)$/$(TARGET).manifest -outputresource:$(BIN)$/$(TARGET)$(DLLPOST)\;\#97

.ENDIF # "$(GUI)"!="WNT" || "$(DISABLE_ATL)"!=""

