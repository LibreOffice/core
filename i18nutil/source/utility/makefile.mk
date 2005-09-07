#*************************************************************************
#*
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 17:40:56 $
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
#************************************************************************
PRJ=..$/..

PRJNAME=i18nutil
TARGET=i18nutil
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

EXCEPTIONSFILES= \
    $(SLO)$/casefolding.obj

SLOFILES= \
    $(EXCEPTIONSFILES) \
    $(SLO)$/unicode.obj \
    $(SLO)$/widthfolding.obj \
    $(SLO)$/oneToOneMapping.obj

# Unicode utilities 
SHL1TARGET=		$(TARGET)$(COMID)
SHL1IMPLIB=		i$(TARGET)

DEF1DEPN=		$(MISC)$/$(SHL1TARGET).flt
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
DEF1NAME=		$(SHL1TARGET)
DEFLIB1NAME=	$(SHL1TARGET)

SHL1OBJS=		$(SLOFILES)

LIB1TARGET=	$(SLB)$/$(SHL1TARGET).lib
LIB1OBJFILES=$(SHL1OBJS)

SHL1STDLIBS= \
        $(CPPULIB) \
        $(SALLIB)

# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo CLEAR_THE_FILE > $@
    @echo __CT >> $@

