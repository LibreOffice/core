#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: vg $ $Date: 2007-02-06 14:23:15 $
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

PRJNAME=ucb
TARGET=ucbdemo
TARGETTYPE=GUI
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings ---

.INCLUDE : settings.mk

# --- Files ---

.IF "$(depend)" != ""

OBJFILES=\
    $(OBJ)$/srcharg.obj \
    $(OBJ)$/ucbdemo.obj

.ENDIF # depend

#SRSFILES= $(SRS)$/ucbdemo.srs

#
# UCBDEMO
#
APP1TARGET=	ucbdemo
APP1OBJS=\
    $(OBJ)$/srcharg.obj \
    $(OBJ)$/ucbdemo.obj
#APP1RES=	$(RES)$/ucbdemo.res

.IF "$(COMPHELPERLIB)"==""

.IF "$(GUI)" == "UNX"
COMPHELPERLIB=-licomphelp2
.ENDIF # unx

.IF "$(GUI)"=="WNT"
COMPHELPERLIB=icomphelp2.lib
.ENDIF # wnt

.ENDIF

APP1STDLIBS=\
    $(SALLIB) \
    $(VOSLIB) \
    $(CPPULIB) \
    $(CPPUHELPERLIB) \
    $(COMPHELPERLIB) \
    $(TOOLSLIB) \
    $(SVTOOLLIB) \
    $(SVLIB) \
    $(UCBHELPERLIB)

APP1DEF=	$(MISC)\$(APP1TARGET).def

# --- Targets ---

.INCLUDE : target.mk

