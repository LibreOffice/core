#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: fs $ $Date: 2000-09-29 11:28:15 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..
PRJNAME=comphelper
TARGET=comphelper

TARGETTYPE=CUI
USE_LDUMP2=TRUE

ENABLE_EXCEPTIONS=TRUE

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/version.mk

# --- Library -----------------------------------

LIB1FILES=	$(SLB)$/container.lib		\
            $(SLB)$/evtattmgr.lib		\
            $(SLB)$/misc.lib			\
            $(SLB)$/processfactory.lib	\
            $(SLB)$/property.lib		\
            $(SLB)$/streaming.lib

LIB1TARGET=$(SLB)$/$(TARGET).lib

SHL1TARGET=$(COMPHLP_TARGET)$(COMPHLP_MAJOR)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(OSLLIB)					\
    $(SALLIB)					\
    $(CPPRTLLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)

SHL1LIBS=	$(LIB1TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt \
            $(LIB1TARGET)

DEFLIB1NAME=$(TARGET)

# --- Targets ----------------------------------

.INCLUDE : target.mk

.IF "$(depend)"==""

# --- Goodies-Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo CLEAR_THE_FILE	> $@
    @echo __CT				>>$@
.ENDIF

