#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 18:15:31 $
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

PRJNAME=idlc
TARGET=idlcpp
TARGETTYPE=CUI
TARGETTHREAD=ST

# --- Settings -----------------------------------------------------

NO_DEFAULT_STL=TRUE
LIBSALCPPRT=$(0)

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

CXXFILES=   \
            cpp.c	\
            eval.c	\
            getopt.c	\
            include.c	\
            lex.c	\
            macro.c	\
            nlist.c	\
            tokens.c	\
            unix.c

OBJFILES=   \
            $(OBJ)$/cpp.obj	\
            $(OBJ)$/eval.obj	\
            $(OBJ)$/getopt.obj	\
            $(OBJ)$/include.obj	\
            $(OBJ)$/lex.obj	\
            $(OBJ)$/macro.obj	\
            $(OBJ)$/nlist.obj	\
            $(OBJ)$/tokens.obj	\
            $(OBJ)$/unix.obj

# --- CPP -------------------------------------------------------

APP1TARGET= $(TARGET)
.IF "$(GUI)" != "UNX"
APP1OBJS=$(OBJ)$/cpp.obj
.ENDIF

APP1LIBS= $(LB)$/idlcpp.lib
APP1DEPN=  

.IF "$(GUI)" == "MAC"
APP1STDLIBS=$(MWPPCLibraries)PPCToolLibs.o  \
            $(STDSLOGUI)
MACRES= $(SV_RES)MPWToolCfrg.r -d SVTOOLNAME="¶"SV TOOL¶""
.ENDIF

# --- Targets ------------------------------------------------------
.IF "$(depend)" == ""

all: \
    ALLTAR

.ENDIF


.INCLUDE :  target.mk

