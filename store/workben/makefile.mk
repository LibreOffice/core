#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 08:49:17 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..

PRJNAME=store
TARGET=workben

LIBTARGET=NO
TARGETTYPE=CUI

# --- Settings ---

.INCLUDE : svpre.mk
.INCLUDE : settings.mk
.INCLUDE : sv.mk

.IF "$(STORELIB)" == ""
.IF "$(GUI)" == "UNX"
STORELIB=	-lstore
.ELSE  # unx
STORELIB=	$(LB)$/istore.lib
.ENDIF # unx
.ENDIF # storelib

.IF "$(GUI)" == "UNX"
STOREDBGLIB=	$(LB)$/libstoredbg.a
.ELSE  # unx
STOREDBGLIB=	$(LB)$/storedbg.lib
.ENDIF # unx

CFLAGS+= -I..$/source

# --- Files ---

CXXFILES=	\
    t_file.cxx	\
    t_base.cxx	\
    t_store.cxx

OBJFILES=	\
    $(OBJ)$/t_file.obj	\
    $(OBJ)$/t_base.obj	\
    $(OBJ)$/t_store.obj

APP1TARGET=		t_file
APP1OBJS=		$(OBJ)$/t_file.obj
APP1STDLIBS=	$(STOREDBGLIB)
APP1STDLIBS+=	$(SALLIB)
APP1DEPN=	\
    $(STOREDBGLIB)	\
    $(L)$/isal.lib

APP2TARGET=		t_base
APP2OBJS=		$(OBJ)$/t_base.obj
APP2STDLIBS=	$(STOREDBGLIB)
APP2STDLIBS+=	$(SALLIB)
APP2DEPN=	\
    $(STOREDBGLIB)	\
    $(L)$/isal.lib

APP3TARGET=		t_store
APP3OBJS=		$(OBJ)$/t_store.obj
APP3STDLIBS=	$(STORELIB)
APP3STDLIBS+=	$(SALLIB)
APP3DEPN=	\
    $(SLB)$/store.lib	\
    $(L)$/isal.lib

# --- Targets ---

.INCLUDE : target.mk

