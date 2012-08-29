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
TARGETTYPE=CUI
LIBTARGET=NO

ENABLE_EXCEPTIONS := TRUE

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------
CDEFS += -DDLL_VERSION=$(EMQ)"$(DLLPOSTFIX)$(EMQ)"

RGTLIB = rgt.lib
.IF "$(GUI)"=="UNX"
RGTLIB = -lrgt$(DLLPOSTFIX)
.ENDIF


CXXFILES= 	regtest.cxx   	\
            test.cxx		\
            regspeed.cxx


APP1TARGET= $(TARGET)
APP1OBJS=   $(OBJ)$/regtest.obj 

APP1STDLIBS=\
            $(SALLIB)	\
            $(REGLIB)

APP2TARGET= test
APP2OBJS=   $(OBJ)$/test.obj
APP2RPATH=NONE
.IF "$(GUI)"=="UNX" && "$(OS)"!="MACOSX"

.IF "$(OS)"=="LINUX" || "$(OS)"=="FREEBSD"
APP2LINKFLAGS=-Wl,-z,origin -Wl,-rpath,\''$$ORIGIN:$$ORIGIN/../lib'\'
.ENDIF

.IF "$(OS)"=="SOLARIS"
APP2LINKFLAGS=-R\''$$ORIGIN/../lib:$$ORIGIN'\'
.ENDIF
    
.ENDIF # "$(OS)"=="UNX"
    
APP2STDLIBS=\
            $(RGTLIB)

APP3TARGET= regspeed
APP3OBJS=   $(OBJ)$/regspeed.obj

APP3STDLIBS=\
            $(SALLIB)	\
            $(REGLIB) 	 

.INCLUDE :  target.mk
