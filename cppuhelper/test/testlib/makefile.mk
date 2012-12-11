#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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

.IF "$(OS)"=="WNT"
PERLLIB=perl58.lib

LIB!:=$(LIB);$(PERLLIBS)
.EXPORT : LIB

.ENDIF

# --- Files --------------------------------------------------------

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


.IF "$(OS)"=="WNT"
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

