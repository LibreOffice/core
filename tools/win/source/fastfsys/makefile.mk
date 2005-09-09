#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 14:49:50 $
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

PRJ=..\..\..

PRJNAME=TOOLS
TARGET=fastfsys

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- WNT ----------------------------------------------------------

.IF "$(GUI)" == "WNT"

# --- Files --------------------------------------------------------


OBJFILES=   $(OBJ)$/shutil.obj          \
            $(OBJ)$/shidl.obj               \
            $(OBJ)$/shmalloc.obj    \
            $(OBJ)$/fffolder.obj    \
            $(OBJ)$/ffmenu.obj              \
            $(OBJ)$/ffitem.obj              \
            $(OBJ)$/fflink.obj              \
            $(OBJ)$/ffparser.obj              \
            $(OBJ)$/wincidl.obj             \
            $(OBJ)$/wincshf.obj


SLOFILES=   $(SLO)$/shutil.obj          \
            $(SLO)$/shidl.obj               \
            $(SLO)$/shmalloc.obj    \
            $(SLO)$/fffolder.obj    \
            $(SLO)$/ffmenu.obj              \
            $(SLO)$/ffitem.obj              \
            $(SLO)$/fflink.obj              \
            $(SLO)$/ffparser.obj              \
            $(SLO)$/wincidl.obj             \
            $(SLO)$/wincshf.obj

# --- Targets ------------------------------------------------------

.ENDIF

.INCLUDE :  target.mk
