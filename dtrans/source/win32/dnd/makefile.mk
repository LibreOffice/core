#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: vg $ $Date: 2006-05-24 14:34:11 $
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
