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


LINKFLAGSDEFS=""

PRJ=..$/..

PRJNAME=cppuhelper
TARGET=defbootstrap
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

PERLINST1:=$(shell @+$(PERL) -V:installarchlib)
PERLINST2:=$(subst,installarchlib=, $(PERLINST1))
PERLINST3:=$(PERLINST2:s/'//)
PERLINST :=$(PERLINST3:s/;//)

PERLLIBS:=$(PERLINST)$/CORE
PERLINCS:=$(PERLINST)$/CORE

CFLAGS += -I$(PERLINCS)

.IF "$(GUI)"=="WNT"
PERLLIB=perl58.lib

LIB!:=$(LIB);$(PERLLIBS)
.EXPORT : LIB

.ENDIF

# --- Files --------------------------------------------------------

#CFLAGS +=   -DVERSION=\"0.01\" -DXS_VERSION=\"0.01\" -fPIC -I/develop6/update/dev/solenv/unxlngi3/lib/perl5/5.6.0/i686-linux/CORE/
#CFLAGS += -I/usr/local/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DVERSION=\"0.01\" -DXS_VERSION=\"0.01\" -KPIC -I/develop6/update/dev/solenv/unxsols2/lib/perl5/5.6.0/sun4-solaris/CORE  

SLOFILES= $(SLO)$/defbootstrap_lib.obj

SHL1TARGET=$(TARGET)

SHL1STDLIBS= \
        $(PERLLIB)			\
        $(CPPUHELPERLIB)	\
        $(CPPULIB)			\
        $(SALLIB)

SHL1DEPN=
SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
SHL1VERSIONMAP=defbootstrap.map


.IF "$(GUI)"=="WNT"
UNODLL=$(DLLDEST)$/UNO.dll
.ELSE
UNODLL=$(DLLDEST)$/UNO.so	
.ENDIF

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL: \
    ALLTAR				\
    $(UNODLL)			\
    $(BIN)$/UNO.pm
.ENDIF

$(BIN)$/UNO.pm: UNO.pm
    cp UNO.pm $@

$(UNODLL): $(SHL1TARGETN)
    cp $(SHL1TARGETN) $@

.INCLUDE :	target.mk

