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


PRJ=../../..

PRJNAME=shell
TARGET=qa_zipimpl
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
.IF "$(OS)" == "WNT"
	NO_DEFAULT_STL=TRUE
	USE_STLP_DEBUG=
.ENDIF
USE_DEFFILE=TRUE
# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk
UWINAPILIB =

SHL1IMPLIB = i$(SHL1TARGET)
SHL1OBJS = $(SLOFILES)
SHL1RPATH = NONE
.IF "$(OS)" == "WNT"
	SHL1STDLIBS = msvcprt.lib 
.ENDIF
SHL1LIBS = $(SOLARLIBDIR)$/zlib.lib $(SLB)$/zipfile.lib   
SLOFILES=$(SLO)$/testzipimpl.obj
SHL1TARGET = $(TARGET)
DEF1NAME=$(SHL1TARGET)
# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.IF "$(OS)" == "WNT"
	INCLUDE!:=$(subst,/stl, $(INCLUDE))
	.EXPORT : INCLUDE 
.ENDIF

