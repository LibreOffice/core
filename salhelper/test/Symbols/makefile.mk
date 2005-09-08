#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 17:02:12 $
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

PRJ=..$/..$/

PRJNAME=salhelper
TARGET=dynloader
TARGET1=samplelib
TARGETTYPE=CUI
LIBTARGET=NO

NO_BSYMBOLIC=	TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings ---

.INCLUDE : settings.mk

# --- Files ---

#RTTI on
.IF "$(OS)" == "WNT"
CFLAGS+= -GR
.ENDIF


#---------------------------------------------------------------------------
# Build the test library which is loaded by the 
# RealDynamicLoader

SLOFILES=	$(SLO)$/samplelib.obj

LIB1TARGET=	$(SLB)$/$(TARGET1).lib
LIB1OBJFILES=	$(SLOFILES)


SHL1TARGET=	$(TARGET1)

SHL1STDLIBS= \
    $(SALLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(TARGET1)
SHL1LIBS=	$(SLB)$/$(TARGET1).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEFLIB1NAME=	$(TARGET1)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt

#DEF1EXPORTFILE=	exports.dxp

# ------------------------------------------------------------------------------

OBJFILES=	$(OBJ)$/loader.obj

APP1TARGET=	$(TARGET)
APP1OBJS=	$(OBJFILES)

APP1STDLIBS= \
    $(SALHELPERLIB)	\
    $(SALLIB)

APP1DEF=	$(MISC)\$(APP1TARGET).def

# --- Targets ---

.INCLUDE : target.mk

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo __CT>>$@
