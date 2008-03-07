#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: kz $ $Date: 2008-03-07 16:58:30 $
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
PRJNAME=avmedia
TARGET=avmedia
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

# --- Resources ---------------------------------

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res	
RESLIB1SRSFILES=			\
    $(SRS)$/viewer.srs		\
    $(SRS)$/framework.srs

# --- Files -------------------------------------

LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1FILES=\
    $(SLB)$/viewer.lib		\
    $(SLB)$/framework.lib	
                
# ==========================================================================

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB=i$(TARGET)
SHL1STDLIBS=$(UNOTOOLSLIB) $(TOOLSLIB) $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB) $(VCLLIB) $(COMPHELPERLIB) $(SVTOOLLIB) $(SVLLIB) $(SFXLIB) $(SOTLIB) $(VOSLIB)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(SLB)$/$(TARGET).lib

DEF1NAME=$(SHL1TARGET)
DEF1DEPN=$(MISC)$/$(SHL1TARGET).flt $(LIB1TARGET)
DEF1DES=Avmedia
DEFLIB1NAME	=$(TARGET)

# --- Targets ----------------------------------

.INCLUDE : target.mk

.IF "$(depend)"==""
$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo _Impl>$@
    @echo WEP>>$@
    @echo LIBMAIN>>$@
    @echo LibMain>>$@
    @echo CT>>$@
.ENDIF
