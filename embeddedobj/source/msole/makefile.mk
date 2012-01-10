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

PRJNAME=embeddedobj
TARGET=emboleobj


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# .IF "$(DISABLE_ATL)"==""

LIBTARGET=NO
USE_DEFFILE=YES

# --- Files --------------------------------------------------------

SHL1TARGET= $(TARGET)

SHL1IMPLIB= i$(TARGET)

SLOFILES =  \
        $(SLO)$/closepreventer.obj\
        $(SLO)$/oleregister.obj\
        $(SLO)$/xolefactory.obj\
        $(SLO)$/olepersist.obj\
        $(SLO)$/oleembed.obj\
        $(SLO)$/olevisual.obj\
        $(SLO)$/olemisc.obj\
        $(SLO)$/ownview.obj

EXCEPTIONSFILES= \
        $(SLO)$/closepreventer.obj\
        $(SLO)$/oleregister.obj\
        $(SLO)$/xolefactory.obj\
        $(SLO)$/olepersist.obj\
        $(SLO)$/oleembed.obj\
        $(SLO)$/olevisual.obj\
        $(SLO)$/olemisc.obj\
        $(SLO)$/ownview.obj

.IF "$(GUI)"=="WNT"

SLOFILES += \
        $(SLO)$/graphconvert.obj\
        $(SLO)$/olecomponent.obj\
        $(SLO)$/olewrapclient.obj\
        $(SLO)$/xdialogcreator.obj\
        $(SLO)$/advisesink.obj

EXCEPTIONSFILES += \
        $(SLO)$/graphconvert.obj\
        $(SLO)$/olecomponent.obj\
        $(SLO)$/olewrapclient.obj\
        $(SLO)$/xdialogcreator.obj\
        $(SLO)$/advisesink.obj

.ENDIF

SHL1OBJS= $(SLOFILES)

SHL1STDLIBS=\
    $(UNOTOOLSLIB)\
    $(SALLIB)\
    $(VOSLIB)\
    $(CPPULIB)\
    $(CPPUHELPERLIB)\
    $(COMPHELPERLIB)\
    $(TOOLSLIB)
    

.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
EMBOBJLIB=-lembobj
.ELSE
EMBOBJLIB=iembobj.lib
.ENDIF

SHL1STDLIBS+=\
    $(EMBOBJLIB)\
    $(OLE32LIB)\
    $(GDI32LIB)\
    $(UUIDLIB)\
    $(OLEAUT32LIB)

DEF1EXPORTFILE=	exports.dxp
.ELIF "$(GUI)"=="OS2"

SHL1STDLIBS+=\
    iembobj.lib

DEF1EXPORTFILE=	exports.dxp

.ENDIF


SHL1DEF= $(MISC)$/$(SHL1TARGET).def

DEF1NAME= $(SHL1TARGET)

# .ENDIF
# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : $(MISC)/emboleobj.component

.IF "$(OS)" == "WNT"
my_platform = .windows
.END

$(MISC)/emboleobj.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        emboleobj.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt emboleobj$(my_platform).component
