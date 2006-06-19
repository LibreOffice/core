#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: hr $ $Date: 2006-06-20 00:32:26 $
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
TARGET=store
ENABLE_EXCEPTIONS=TRUE

# --- Settings ---

.INCLUDE : settings.mk

# --- Files ---

SLOFILES=	\
    $(SLO)$/object.obj		\
    $(SLO)$/memlckb.obj		\
    $(SLO)$/filelckb.obj	\
    $(SLO)$/storbase.obj	\
    $(SLO)$/storcach.obj	\
    $(SLO)$/stordata.obj	\
    $(SLO)$/storlckb.obj	\
    $(SLO)$/stortree.obj	\
    $(SLO)$/storpage.obj	\
    $(SLO)$/store.obj

.IF "$(debug)" != ""
OBJFILES=	\
    $(OBJ)$/object.obj		\
    $(OBJ)$/memlckb.obj		\
    $(OBJ)$/filelckb.obj	\
    $(OBJ)$/storbase.obj	\
    $(OBJ)$/storcach.obj	\
    $(OBJ)$/stordata.obj	\
    $(OBJ)$/storlckb.obj	\
    $(OBJ)$/stortree.obj	\
    $(OBJ)$/storpage.obj	\
    $(OBJ)$/store.obj
.ENDIF # debug

# --- Targets ---

.INCLUDE : target.mk

