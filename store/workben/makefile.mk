#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..

PRJNAME=store
TARGET=workben

LIBTARGET=NO
TARGETTYPE=CUI
NO_DEFAULT_STL=TRUE

# --- Settings ---

.INCLUDE : settings.mk

.IF "$(STORELIB)" == ""
.IF "$(GUI)" == "UNX"
STORELIB=	-lstore
.ELSE  # unx
.IF "$(GUI)$(COM)"=="WNTGCC"
STORELIB=	-lstore$(UDK_MAJOR)
.ELSE
STORELIB=	$(LB)$/istore.lib
.ENDIF
.ENDIF # unx
.ENDIF # storelib

.IF "$(GUI)" == "UNX"
STOREDBGLIB=	$(LB)$/libstoredbg.a
.ELSE  # unx
.IF "$(GUI)$(COM)"=="WNTGCC"
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

