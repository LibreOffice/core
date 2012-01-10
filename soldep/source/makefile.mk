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

PRJNAME=soldep
TARGET=soldep


# --- Settings -----------------------------------------------------
#.INCLUDE :  $(PRJ)$/util$/perl.mk
.INCLUDE :  settings.mk

# fixme, code is not yet 64 bit clean
.IF "$(OS)$(CPU)"=="LINUXX" || ("$(COM)"=="C52" && "$(CPU)"=="U")
all:
    @echo nothing to do
.ENDIF


# --- Files --------------------------------------------------------

SLOFILES        = \
    $(SLO)$/soldep.obj		\
    $(SLO)$/soldlg.obj		\
    $(SLO)$/depper.obj		\
    $(SLO)$/hashobj.obj		\
    $(SLO)$/connctr.obj		\
    $(SLO)$/depwin.obj		\
    $(SLO)$/graphwin.obj	\
    $(SLO)$/tbox.obj		\
    $(SLO)$/sdtresid.obj		\
    $(SLO)$/objwin.obj

SHL1TARGET	=$(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB	=$(TARGET)
SHL1LIBS	=$(SLB)$/$(TARGET).lib \
            $(SLB)$/bootstrpdt.lib
SHL1DEF		=$(MISC)$/$(SHL1TARGET).def
SHL1DEPN	=$(SHL1LIBS)
SHL1STDLIBS     = $(SVTOOLLIB)		\
                $(CPPUHELPERLIB)	\
                $(COMPHELPERLIB)	\
                $(VCLLIB)			\
                $(UCBHELPERLIB)	\
                $(SOTLIB)			\
                $(TOOLSLIB) 		\
                $(VOSLIB)			\
                $(SALLIB)           \
                $(CPPULIB)  \
                   $(PERL_LIB)

#.IF "$(GUI)" == "UNX"
#SHL1STDLIBS+=\
#		$(SALLIB)
#.ENDIF


DEF1NAME    =$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME	=$(TARGET)

SRC1FILES = \
    soldlg.src
RES1TARGET = dep
SRS1NAME=$(TARGET)
SRS1FILES = \
    $(SRS)$/$(TARGET).srs

RESLIB1NAME = dep
RESLIB1SRSFILES = \
    $(SRS)$/$(TARGET).srs


#------------- Application ---------------
APP1TARGET=soldepl

APP1ICON=soldep.ico

APP1STDLIBS= \
            $(SVTOOLLIB)		\
            $(CPPUHELPERLIB)	\
            $(COMPHELPERLIB)	\
            $(UCBHELPERLIB)	\
            $(VCLLIB)			\
            $(SOTLIB)			\
            $(TOOLSLIB) 		\
            $(VOSLIB)			\
            $(SALLIB)           \
               $(CPPULIB)  \
               $(PERL_LIB)

APP1LIBS=\
    $(SLB)$/bootstrpdt.lib \
    $(SLB)$/soldep.lib

APP1OBJS= \
    $(SLO)$/depapp.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo WEP > $@
    @echo LIBMAIN >> $@
    @echo LibMain >> $@
    @echo __CT >> $@
