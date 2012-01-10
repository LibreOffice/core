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

PRJNAME=dtrans
TARGET=dtrans
TARGET1=mcnttype
TARGET2=ftransl
TARGET3=sysdtrans
TARGET4=dnd

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.IF "$(L10N_framework)"==""

# --- mcnttype dynlib ----------------------------------------------

SHL1TARGET=$(TARGET1)

SHL1LIBS=$(SLB)$/mcnttype.lib

SHL1STDLIBS= \
        $(SALLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB) 

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= \
        $(UWINAPILIB)
.ENDIF



SHL1IMPLIB=i$(SHL1TARGET) 

#--- comment -----------------

SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
DEF1NAME=		$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp
SHL1VERSIONMAP=$(SOLARENV)/src/component.map


.IF "$(GUI)"=="WNT"

# --- ftransl dll ---

SHL2TARGET=$(TARGET2)

SHL2LIBS=$(SLB)$/ftransl.lib\
         $(SLB)$/dtutils.lib

SHL2STDLIBS= \
        $(SALLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB)\
        $(UWINAPILIB)\
        $(ADVAPI32LIB)\
        $(OLE32LIB)\
        $(GDI32LIB)

SHL2IMPLIB=i$(SHL2TARGET) 

SHL2DEF=		$(MISC)$/$(SHL2TARGET).def
DEF2NAME=		$(SHL2TARGET)
DEF2EXPORTFILE=	exports.dxp


# --- sysdtrans dll ---

SHL3TARGET=$(TARGET3)

SHL3LIBS=$(SLB)$/sysdtrans.lib\
         $(SLB)$/dtutils.lib\
         $(SLB)$/dtobjfact.lib\
         $(SLB)$/mtaolecb.lib

.IF "$(COM)" == "GCC"
SHL3OBJS=$(SLO)$/XNotifyingDataObject.obj
.ENDIF

SHL3STDLIBS= \
        $(SALLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB)\
        $(UWINAPILIB)\
        $(ADVAPI32LIB)\
        $(OLE32LIB)\
        $(OLEAUT32LIB)\
        $(GDI32LIB)\
        $(SHELL32LIB)\
        $(UUIDLIB)
        
SHL3IMPLIB=i$(SHL3TARGET) 

SHL3DEF=		$(MISC)$/$(SHL3TARGET).def
DEF3NAME=		$(SHL3TARGET)
DEF3EXPORTFILE=	exports.dxp


# --- dnd dll ---

SHL4TARGET=$(TARGET4)

SHL4LIBS=	\
            $(SLB)$/dnd.lib\
            $(SLB)$/dtobjfact.lib\
            $(SLB)$/dtutils.lib

SHL4STDLIBS= \
        $(SALLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB) \
        $(UWINAPILIB)\
        $(ADVAPI32LIB)\
        $(OLE32LIB)\
        $(OLEAUT32LIB)\
        $(GDI32LIB)\
        $(SHELL32LIB)\
        $(UUIDLIB)

SHL4DEPN=
SHL4IMPLIB=i$(SHL4TARGET) 

SHL4DEF=		$(MISC)$/$(SHL4TARGET).def

DEF4NAME=		$(SHL4TARGET)
DEF4EXPORTFILE=	exports.dxp

.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"

# --- sysdtrans dll ---

SHL3TARGET=$(TARGET3)

SHL3LIBS=$(SLB)$/sysdtrans.lib

SHL3STDLIBS= \
        $(SALLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB) \
        UClip.lib
        
SHL3IMPLIB=i$(SHL3TARGET) 

SHL3DEF=		$(MISC)$/$(SHL3TARGET).def
DEF3NAME=		$(SHL3TARGET)
DEF3EXPORTFILE=	exports.dxp

.ENDIF			# "$(GUI)"=="OS2"
.ENDIF # L10N_framework

.INCLUDE :  target.mk

ALLTAR : \
    $(MISC)/dnd.component \
    $(MISC)/ftransl.component \
    $(MISC)/mcnttype.component \
    $(MISC)/sysdtrans.component

$(MISC)/dnd.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        dnd.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL4TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt dnd.component

$(MISC)/ftransl.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        ftransl.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL2TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ftransl.component

$(MISC)/mcnttype.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        mcnttype.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt mcnttype.component

$(MISC)/sysdtrans.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        sysdtrans.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL3TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt sysdtrans.component
