#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.13 $
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

PRJ=..$/..$/..

PRJNAME=dtrans
TARGET=dnd
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST=$(TARGET)
USE_BOUNDCHK=

.IF "$(USE_BOUNDCHK)"=="TR"
bndchk=tr
stoponerror=tr
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

#CFLAGS+=-GR -DUNICODE -D_UNICODE

#DBG_CONSOLE_OUT: when IDropTarget or IDropSource are called we write to a console
#DBG_CLIPBOARD_DATA: To provide DoDragDrop with an data object we call OleGetClipboard
#CFLAGS+= -DDBG_CONSOLE_OUT -DDBG_CLIPBOARD_DATA


SLOFILES=	$(SLO)$/dndentry.obj	\
            $(SLO)$/target.obj		\
            $(SLO)$/idroptarget.obj	\
            $(SLO)$/source.obj		\
            $(SLO)$/globals.obj		\
            $(SLO)$/targetdropcontext.obj	\
            $(SLO)$/targetdragcontext.obj	\
            $(SLO)$/sourcecontext.obj


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
