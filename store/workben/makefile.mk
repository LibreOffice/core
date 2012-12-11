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

PRJNAME=store
TARGET=workben

LIBTARGET=NO
TARGETTYPE=CUI
NO_DEFAULT_STL=TRUE

# --- Settings ---

.INCLUDE : settings.mk

.IF "$(STORELIB)" == ""
.IF "$(OS)" != "WNT"
STORELIB=	-lstore
.ELSE  # unx
.IF "$(OS)$(COM)"=="WNTGCC"
STORELIB=	-lstore$(UDK_MAJOR)
.ELSE
STORELIB=	$(LB)$/istore.lib
.ENDIF
.ENDIF # unx
.ENDIF # storelib

.IF "$(OS)" != "WNT"
STOREDBGLIB=	$(LB)$/libstoredbg.a
.ELSE  # unx
.IF "$(OS)$(COM)"=="WNTGCC"
STOREDBGLIB=	$(LB)$/libstoredbg.a
.ELSE
STOREDBGLIB=	$(LB)$/storedbg.lib
.ENDIF
.ENDIF # unx

CFLAGS+= -I..$/source

# --- Files ---

OBJFILES=	\
    $(OBJ)$/t_leak.obj	\
    $(OBJ)$/t_file.obj	\
    $(OBJ)$/t_page.obj	\
    $(OBJ)$/t_base.obj	\
    $(OBJ)$/t_store.obj

APP1TARGET=		t_file
APP1OBJS=		$(OBJ)$/t_file.obj
APP1STDLIBS=	$(STOREDBGLIB)
APP1STDLIBS+=	$(SALLIB)
APP1DEPN=	$(STOREDBGLIB)
APP1RPATH=	UREBIN

APP2TARGET=		t_page
APP2OBJS=		$(OBJ)$/t_page.obj
APP2STDLIBS=	$(STOREDBGLIB)
APP2STDLIBS+=	$(SALLIB)
APP2DEPN=	$(STOREDBGLIB)

APP3TARGET=		t_base
APP3OBJS=		$(OBJ)$/t_base.obj
APP3STDLIBS=	$(STOREDBGLIB)
APP3STDLIBS+=	$(SALLIB)
APP3DEPN=	$(STOREDBGLIB)
APP3RPATH=	UREBIN

APP4TARGET=		t_store
APP4OBJS=		$(OBJ)$/t_store.obj
APP4STDLIBS=	$(STORELIB)
APP4STDLIBS+=	$(SALLIB)
APP4DEPN=	$(SLB)$/store.lib
APP4RPATH=	UREBIN

 APP5TARGET=		t_leak
 APP5OBJS=		$(OBJ)$/t_leak.obj
 APP5STDLIBS+=	$(SALLIB)

# --- Targets ---

.INCLUDE : target.mk

