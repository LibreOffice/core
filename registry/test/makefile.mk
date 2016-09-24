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

PRJ=..

PRJNAME=registry
TARGET=regtest

USE_LDUMP2=TRUE

ENABLE_EXCEPTIONS := TRUE

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk


# ------------------------------------------------------------------
CDEFS += -DDLL_VERSION=$(EMQ)"$(DLLPOSTFIX)$(EMQ)"

CXXFILES= \
               testregcpp.cxx	\
               testmerge.cxx		


LIB1TARGET= $(SLB)$/$(TARGET).lib

LIB1OBJFILES= \
                $(SLO)$/testregcpp.obj	\
                $(SLO)$/testmerge.obj


SHL1TARGET= rgt$(DLLPOSTFIX)
SHL1IMPLIB= rgt
SHL1STDLIBS=	\
                $(SALLIB) \
                $(SALHELPERLIB)	\
                $(REGLIB) \
                $(STDLIBCPP)

SHL1LIBS=	$(LIB1TARGET)
SHL1DEPN=	$(LIB1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)
             
DEF1DEPN	=$(MISC)$/rgt$(DLLPOSTFIX).flt $(SLOFILES)
DEFLIB1NAME =$(TARGET)
DEF1DES 	=Registry Runtime - TestDll

# --- Targets ------------------------------------------------------

#all: \
#	ALLTAR	

.INCLUDE :  target.mk

# --- SO2-Filter-Datei ---


$(MISC)$/rgt$(DLLPOSTFIX).flt:
    @echo ------------------------------
    @echo Making: $@
    @echo WEP>$@
    @echo LIBMAIN>>$@
    @echo LibMain>>$@


