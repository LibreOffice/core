#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: vg $ $Date: 2007-09-20 16:34:42 $
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

TARGETTYPE=CUI
NO_DEFAULT_STL=TRUE

PRJNAME=rsc
TARGET=rscpp

.IF "$(cpp)" != ""
PRJNAME=CPP
TARGET=cpp
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

CDEFS+=-DSOLAR

.IF "$(cpp)" != ""
CDEFS+=-DNOMAIN
.ENDIF

# --- Files --------------------------------------------------------

OBJFILES=   \
            $(OBJ)$/cpp1.obj \
            $(OBJ)$/cpp2.obj \
            $(OBJ)$/cpp3.obj \
            $(OBJ)$/cpp4.obj \
            $(OBJ)$/cpp5.obj \
            $(OBJ)$/cpp6.obj	\

.IF "$(cpp)" == ""
LIBSALCPPRT=$(0)
APP1TARGET= $(TARGET)
APP1LIBS=$(LB)$/$(TARGET).lib
.IF "$(GUI)" != "OS2"
APP1STACK=32768
.ENDIF
.ENDIF

.IF "$(GUI)"=="UNX"
CDEFS+=-Dunix
.ENDIF

.IF "$(OS)$(CPU)"=="SOLARISI"
# cc: Sun C 5.5 Patch 112761-10 2004/08/10
# Solaris x86 compiler ICE
# "cpp6.c", [get]:ube: internal error
# remove after compiler upgrade
NOOPTFILES=$(OBJ)$/cpp6.obj
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

cpp1.c: cppdef.h cpp.h
cpp2.c: cppdef.h cpp.h
cpp3.c: cppdef.h cpp.h
cpp4.c: cppdef.h cpp.h
cpp5.c: cppdef.h cpp.h
cpp6.c: cppdef.h cpp.h 


