#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: vg $ $Date: 2007-10-15 12:29:09 $
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

PRJNAME=registry
TARGET=reg

UNIXVERSIONNAMES=UDK

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

# ------------------------------------------------------------------

USE_LDUMP2=TRUE
#USE_DEFFILE=TRUE
LDUMP2=LDUMP3

DOCPPFILES= $(INC)$/registry$/registry.hxx \
            $(INC)$/registry$/registry.h \
            $(INC)$/registry$/regtype.h \
            $(INC)$/registry$/reflread.hxx \
            $(INC)$/registry$/reflwrit.hxx \
            $(INC)$/registry$/refltype.hxx \

LIB1TARGET= $(SLB)$/$(TARGET).lib

LIB1FILES= 	$(SLB)$/$(TARGET)cpp.lib

SHL1TARGET= $(TARGET)
SHL1IMPLIB= ireg
SHL1STDLIBS= \
    $(SALLIB) \
    $(SALHELPERLIB) \
    $(STORELIB)

SHL1VERSIONMAP=	$(TARGET).map

SHL1LIBS= $(LIB1TARGET)	
SHL1DEPN= $(LIB1TARGET)
SHL1DEF= $(MISC)$/$(SHL1TARGET).def
DEF1NAME= $(SHL1TARGET)
DEF1DES	= RegistryRuntime
SHL1RPATH=URELIB

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

makedocpp: $(DOCPPFILES)
     docpp -H -m -f  -u -d $(OUT)$/doc$/$(PRJNAME) $(DOCPPFILES)

