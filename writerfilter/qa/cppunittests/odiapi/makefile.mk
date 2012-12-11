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

PRJNAME=writerfilter
TARGET=test-odiapi
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGSCXX += $(CPPUNIT_CFLAGS)

SHL1OBJS=\
    $(SLO)$/testProperty.obj \
    $(SLO)$/FileLoggerImpl.obj\
    $(SLO)$/ExternalViewLogger.obj\
    $(SLO)$/testCore.obj

SHL1TARGET=$(TARGET)
SHL1STDLIBS=$(SALLIB) $(TOOLSLIB) $(CPPUNITLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+=   $(LB)$/iodiapi.lib
.ELIF "$(GUI)"=="UNX" && "$(OS)"!="MACOSX"
SHL1STDLIBS+=$(LB)$/libodiapi.so
.ELIF "$(OS)"=="MACOSX"
SHL1STDLIBS+=$(LB)$/libodiapi.dylib
.ENDIF

SHL1IMPLIB= i$(SHL1TARGET)

DEF1NAME    =$(SHL1TARGET)

SHL1VERSIONMAP = export.map

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk
