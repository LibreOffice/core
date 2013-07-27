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
PRJNAME=writerfilter
.IF "$(GUI)" == "OS2"
TARGET=wfltuno
.ELSE
TARGET=writerfilter.uno
.ENDIF
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CDEFS+=-DWRITERFILTER_DLLIMPLEMENTATION

# --- Files --------------------------------------------------------

SLOFILES=$(SLO)$/component.obj

SHL1TARGET=$(TARGET)

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC" || "$(GUI)"=="OS2"
DOCTOKLIB=-ldoctok
OOXMLLIB=-looxml
RESOURCEMODELLIB=-lresourcemodel
.ELIF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
DOCTOKLIB=-ldoctok
OOXMLLIB=-looxml
RESOURCEMODELLIB=-lresourcemodel
.ELSE
DOCTOKLIB=$(LB)$/idoctok.lib
OOXMLLIB=$(LB)$/iooxml.lib
RESOURCEMODELLIB=$(LB)$/iresourcemodel.lib
.ENDIF
.ENDIF

SHL1STDLIBS=$(SALLIB)\
    $(CPPULIB)\
    $(COMPHELPERLIB)\
    $(CPPUHELPERLIB)\
    $(UCBHELPERLIB)\
    $(DOCTOKLIB) \
    $(OOXMLLIB) \
    $(RESOURCEMODELLIB)

SHL1LIBS=\
    $(SLB)$/debugservices_doctok.lib \
    $(SLB)$/debugservices_ooxml.lib

SHL1IMPLIB=i$(SHL1TARGET)

SHL1OBJS = $(SLO)$/component.obj

SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

