#************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
# ***********************************************************************/
PRJ=..
PRJNAME=writerfilter
TARGET=writerfilter.uno
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CDEFS+=-DWRITERFILTER_DLLIMPLEMENTATION

# --- Files --------------------------------------------------------

SLOFILES=$(SLO)$/component.obj

SHL1TARGET=$(TARGET)

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
DOCTOKLIB=-ldoctok
OOXMLLIB=-looxml
RESOURCEMODELLIB=-lresourcemodel
.ELIF "$(GUI)"=="OS2"
DOCTOKLIB=$(LB)$/idoctok.lib
OOXMLLIB=$(LB)$/iooxml.lib
RESOURCEMODELLIB=$(LB)$/iresourcemodel.lib
.ELIF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
DOCTOKLIB=-ldoctok
OOXMLLIB=-looxml
RESOURCEMODELLIB=-lresourcemodel
.ELSE
DOCTOKLIB=$(LB)$/idoctok.lib
OOXMLLIB=$(LB)$/iooxml.lib
RESOURCEMODELLIB=$(LB)$/iresourcemodel.lib
.ENDIF
.ENDIF

SHL1STDLIBS=$(SALLIB)\
    $(CPPULIB)\
    $(COMPHELPERLIB)\
    $(CPPUHELPERLIB)\
    $(UCBHELPERLIB)\
    $(DOCTOKLIB) \
    $(OOXMLLIB) \
    $(RESOURCEMODELLIB)

SHL1LIBS=\
    $(SLB)$/debugservices_doctok.lib \
    $(SLB)$/debugservices_ooxml.lib

SHL1IMPLIB=i$(SHL1TARGET)

SHL1OBJS = $(SLO)$/component.obj

SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

