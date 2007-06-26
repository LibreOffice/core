#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: hr $ $Date: 2007-06-26 16:05:41 $
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

PRJNAME=embeddedobj
TARGET=embobj
#LIBTARGET=NO
#USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

LIB1OBJFILES= \
        $(SLO)$/miscobj.obj\
        $(SLO)$/specialobject.obj\
        $(SLO)$/persistence.obj\
        $(SLO)$/embedobj.obj\
        $(SLO)$/inplaceobj.obj\
        $(SLO)$/visobj.obj\
        $(SLO)$/dummyobject.obj\
        $(SLO)$/xcreator.obj\
        $(SLO)$/xfactory.obj\
        $(SLO)$/register.obj\
        $(SLO)$/convert.obj\
        $(SLO)$/docholder.obj\
        $(SLO)$/confighelper.obj\
        $(SLO)$/intercept.obj

LIB1TARGET=$(SLB)$/$(TARGET).lib

SHL1TARGET= $(TARGET)

SHL1STDLIBS=\
    $(SALLIB)\
    $(CPPULIB)\
    $(COMPHELPERLIB)\
    $(CPPUHELPERLIB)

SHL1DEPN=
SHL1IMPLIB= i$(TARGET)

SHL1LIBS=	$(LIB1TARGET)

SHL1DEF= $(MISC)$/$(SHL1TARGET).def

DEF1NAME= $(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt \
            $(LIB1TARGET)

DEFLIB1NAME=$(TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk


$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo CLEAR_THE_FILE	> $@
    @echo __CT				>>$@

