#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: vg $ $Date: 2006-09-25 13:02:42 $
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

PRJ=..$/..

PRJNAME=tools
TARGET=fsys
ENABLE_EXCEPTIONS=true

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :      $(PRJ)$/util$/makefile.pmk

.IF "$(COM)"=="GCC"
CFLAGSCXX+=-fexceptions
.ENDIF


# --- Files --------------------------------------------------------

SLOFILES= \
            $(SLO)$/tempfile.obj   \
            $(SLO)$/wldcrd.obj   \
            $(SLO)$/fstat.obj    \
            $(SLO)$/comdep.obj   \
            $(SLO)$/filecopy.obj \
            $(SLO)$/dirent.obj   \
            $(SLO)$/tdir.obj	\
            $(SLO)$/urlobj.obj

OBJFILES=   $(OBJ)$/wldcrd.obj   \
            $(OBJ)$/fstat.obj    \
            $(OBJ)$/comdep.obj   \
            $(OBJ)$/filecopy.obj \
            $(OBJ)$/dirent.obj   \
            $(OBJ)$/tdir.obj	\
            $(OBJ)$/urlobj.obj

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

