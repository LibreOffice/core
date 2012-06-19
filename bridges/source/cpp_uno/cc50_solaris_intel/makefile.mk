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

PRJNAME=bridges
TARGET=sunpro5_uno
LIBTARGET=no
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

# disable check for PIC code as it would complain about 
# hand coded assembler
CHECKFORPIC=

.IF "$(COM)$(CPU)" == "C50I" || "$(COM)$(CPU)" == "C52I"

CFLAGS += -O5 -xO5

SLOFILES= \
    $(SLO)$/cpp2uno.obj		\
    $(SLO)$/uno2cpp.obj		\
    $(SLO)$/except.obj		\
    $(SLO)$/call.obj

SHL1TARGET= $(TARGET)

SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
SHL1IMPLIB= i$(TARGET)
SHL1VERSIONMAP=..$/..$/bridge_exports.map
SHL1RPATH=  URELIB

SHL1OBJS = $(SLOFILES)
SHL1LIBS = $(SLB)$/cpp_uno_shared.lib

SHL1STDLIBS= \
    $(CPPULIB)	\
    $(SALLIB)

.ENDIF

.INCLUDE :	target.mk

$(SLO)$/%.obj: %.s
    CC -c -o $(SLO)$/$(@:b).o $< && touch $@

