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

PRJ=..$/..$/..$/

PRJNAME=dtrans
TARGET=dndTest
TARGETTYPE=CUI
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings ---

.INCLUDE : settings.mk

# --- Files ---

CFLAGS+= -D_WIN32_DCOM

INCPRE+=	-I$(ATL_INCLUDE)

OBJFILES=	$(OBJ)$/dndTest.obj	\
        $(OBJ)$/atlwindow.obj \
        $(OBJ)$/targetlistener.obj \
        $(OBJ)$/sourcelistener.obj \
        $(OBJ)$/dataobject.obj
        
APP1NOSAL=TRUE

APP1TARGET=	$(TARGET)
APP1OBJS=$(OBJFILES)

APP1STDLIBS= \
    $(SALLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB)		\
    $(UWINAPILIB) \
    $(USER32LIB)		\
    $(OLE32LIB)		\
    comsupp.lib		\
    $(OLEAUT32LIB)	\
    $(GDI32LIB)		\
    $(UUIDLIB)

APP1LIBS=	\
            $(SLB)$/dtobjfact.lib	\
            $(SLB)$/dtutils.lib

APP1DEF=	$(MISC)\$(APP1TARGET).def

# --- Targets ---

.INCLUDE : target.mk

