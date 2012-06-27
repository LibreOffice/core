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

PRJNAME=soltools
TARGET=cpp
TARGETTYPE=CUI
NO_DEFAULT_STL=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : $(PRJ)$/util$/makefile.pmk
.INCLUDE :  settings.mk

.IF "$(CROSS_COMPILING)"=="YES"

all:
# nothing

.ENDIF

UWINAPILIB=$(0)
LIBSALCPPRT=$(0)

# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/_cpp.obj		\
    $(OBJ)$/_eval.obj 	\
    $(OBJ)$/_include.obj \
    $(OBJ)$/_lex.obj 	\
    $(OBJ)$/_macro.obj 	\
    $(OBJ)$/_mcrvalid.obj \
    $(OBJ)$/_nlist.obj 	\
    $(OBJ)$/_tokens.obj 	\
    $(OBJ)$/_unix.obj

# nonstandard cpp options needed for Mac (-isysroot),
# needs the custom stgetopt defined here :/
.IF "$(OS)" == "MACOSX" || "$(OS)" == "AIX" || "$(HAVE_GETOPT)" != "YES"
OBJFILES += $(OBJ)$/_getopt.obj
.ENDIF
.IF "$(HAVE_GETOPT)" == "YES"
CDEFS += -DHAVE_GETOPT
.ENDIF

APP1TARGET	=	$(TARGET)
APP1STACK	=	1000000
APP1LIBS	=	$(LB)$/$(TARGET).lib
APP1DEPN	=   $(LB)$/$(TARGET).lib

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


