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

PRJNAME=idl
TARGET=idl

TARGETTYPE=CUI

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------------

LIB1TARGET= $(LB)$/$(TARGET).lib
LIB1FILES = $(LB)$/prj.lib		 \
            $(LB)$/objects.lib	 \
            $(LB)$/cmptools.lib

APP1TARGET= svidl
APP1RPATH=NONE

APP1STDLIBS=	\
    $(TOOLSLIB)	\
    $(SALLIB)

APP1LIBS= $(LB)$/$(TARGET).lib
.IF "$(GUI)" != "UNX"
.IF "$(COM)"!="GCC"
APP1OBJS=	$(OBJ)$/svidl.obj	\
            $(OBJ)$/command.obj
.ENDIF
.ENDIF

.IF "$(OS)"=="LINUX" || "$(OS)"=="FREEBSD"
APP1STDLIBS+=-lcrypt
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

