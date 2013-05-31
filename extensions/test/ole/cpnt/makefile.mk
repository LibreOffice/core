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
PRJ=..$/..$/..

PRJNAME=	extensions
TARGET=		oletest
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

INCPRE+= -I$(ATL_INCLUDE)

SLOFILES=	\
        $(SLO)$/cpnt.obj

SHL1TARGET= $(TARGET)

SHL1STDLIBS= \
        $(SALLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB)

SHL1DEPN=
SHL1IMPLIB=		i$(TARGET)
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp


ALLTAR : 	$(MISC)$/$(TARGET).cppumaker.done


.INCLUDE :  target.mk

ALLIDLFILES:=	..$/idl$/oletest.idl

$(BIN)$/oletest.rdb: $(ALLIDLFILES)
    idlc -I$(PRJ) -I$(SOLARIDLDIR)  -O$(BIN) $?
    regmerge $@ /UCR $(BIN)$/{$(?:f:s/.idl/.urd/)}
    touch $@

$(MISC)$/$(TARGET).cppumaker.done: $(BIN)$/oletest.rdb
    $(CPPUMAKER) -O$(INCCOM) -BUCR $< -X$(SOLARBINDIR)/types.rdb
    $(TOUCH) $@

