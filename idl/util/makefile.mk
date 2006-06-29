#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: ihi $ $Date: 2006-06-29 11:19:04 $
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

PRJNAME=idl
TARGET=idl

TARGETTYPE=CUI

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------------

LIB1TARGET= $(LB)$/$(TARGET).lib
LIB1FILES = $(LB)$/prj.lib		 \
            $(LB)$/objects.lib	 \
            $(LB)$/cmptools.lib

APP1TARGET= svidl

APP1STDLIBS=	\
    $(TOOLSLIB)	\
    $(SALLIB)

APP1LIBS= $(LB)$/$(TARGET).lib
.IF "$(GUI)" != "UNX"
APP1OBJS=	$(OBJ)$/svidl.obj	\
            $(OBJ)$/command.obj
.ENDIF

.IF "$(OS)"=="LINUX" || "$(OS)"=="FREEBSD"
APP1STDLIBS+=-lcrypt
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

