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

PRJ := ..$/..
PRJNAME := stoc

TARGET := test_registry_tdprovider

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

DLLPRE = # no leading "lib" on .so files

SLOFILES = $(SLO)$/testregistrytdprovider.obj

SHL1TARGET = $(ENFORCEDSHLPREFIX)testregistrytdprovider.uno
SHL1OBJS = $(SLOFILES)
SHL1VERSIONMAP = $(SOLARENV)/src/component.map
SHL1STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB)
SHL1IMPLIB = itestregistrytdprovider
DEF1NAME = $(SHL1TARGET)

.INCLUDE: target.mk

ALLTAR: test

$(MISC)$/$(TARGET)$/all.rdb: types.idl
    - rm $@
    - $(MKDIR) $(MISC)$/$(TARGET)
    idlc -O$(MISC)$/$(TARGET) -I$(SOLARIDLDIR) -C -cid -we $<
    regmerge $@ /UCR $(MISC)$/$(TARGET)$/types.urd
    regmerge $@ / $(SOLARBINDIR)$/types.rdb

$(SLOFILES): $(MISC)$/$(TARGET)$/all.rdb

test .PHONY: $(SHL1TARGETN) $(MISC)$/$(TARGET)$/all.rdb
    uno -c test.registrytdprovider.impl -l $(subst,$/,/ $(SHL1TARGETN)) \
        -ro $(subst,$/,/ $(MISC)$/$(TARGET)$/all.rdb)
