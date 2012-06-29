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
PRJNAME := comphelper
TARGET := qa_weakbag

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk
.INCLUDE : $(PRJ)$/version.mk

CFLAGSCXX += $(CPPUNIT_CFLAGS)

DLLPRE = # no leading "lib" on .so files

INCPRE += $(MISC)$/$(TARGET)$/inc

SHL1TARGET = $(TARGET)_weakbag
SHL1OBJS = $(SLO)$/test_weakbag.obj $(SLO)$/test_weakbag_noadditional.obj
SHL1STDLIBS = $(CPPUHELPERLIB) $(CPPULIB) $(CPPUNITLIB) $(SALLIB) $(COMPHELPERLIB)
SHL1VERSIONMAP = ..$/version.map
SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

SLOFILES = $(SHL1OBJS)

.INCLUDE: target.mk
.INCLUDE: _cppunit.mk
