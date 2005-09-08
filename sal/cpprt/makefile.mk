#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 14:25:01 $
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

PRJNAME=SAL
TARGET=salcpprt

ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES =	\
    $(SLO)$/operators_new_delete.obj


.IF "$(OS)" != "SOLARIS"

# build as archive
LIB1TARGET=$(LB)$/$(TARGET).lib
LIB1ARCHIV=$(LB)$/lib$(TARGET).a
LIB1OBJFILES=$(SLOFILES)

.ELSE  # SOLARIS

# build as shared library (interposer needed for -Bdirect)
LINKFLAGS+= -z interpose

SHL1TARGET=	$(TARGET)
SHL1IMPLIB=	i$(TARGET)

SHL1STDLIBS=$(SALLIB)
SHL1OBJS=	$(SLOFILES)

.ENDIF # SOLARIS

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

