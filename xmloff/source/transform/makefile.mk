#*************************************************************************
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
#*************************************************************************

PRJ := ..$/..
PRJNAME := xmloff
TARGET := xof
LIBTARGET=NO
ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

SHL1TARGET = $(TARGET)$(DLLPOSTFIX)
SHL1OBJS = \
    $(SLO)$/ChartOASISTContext.obj \
    $(SLO)$/ChartOOoTContext.obj \
    $(SLO)$/ChartPlotAreaOASISTContext.obj \
    $(SLO)$/ChartPlotAreaOOoTContext.obj \
    $(SLO)$/ControlOASISTContext.obj \
    $(SLO)$/ControlOOoTContext.obj \
    $(SLO)$/CreateElemTContext.obj \
    $(SLO)$/DeepTContext.obj \
    $(SLO)$/DlgOASISTContext.obj \
    $(SLO)$/DocumentTContext.obj \
    $(SLO)$/EventMap.obj \
    $(SLO)$/EventOASISTContext.obj \
    $(SLO)$/EventOOoTContext.obj \
    $(SLO)$/FlatTContext.obj \
    $(SLO)$/FormPropOASISTContext.obj \
    $(SLO)$/FormPropOOoTContext.obj \
    $(SLO)$/FrameOASISTContext.obj \
    $(SLO)$/FrameOOoTContext.obj \
    $(SLO)$/IgnoreTContext.obj \
    $(SLO)$/MergeElemTContext.obj \
    $(SLO)$/MetaTContext.obj \
    $(SLO)$/MutableAttrList.obj \
    $(SLO)$/NotesTContext.obj \
    $(SLO)$/OOo2Oasis.obj \
    $(SLO)$/Oasis2OOo.obj \
    $(SLO)$/PersAttrListTContext.obj \
    $(SLO)$/PersMixedContentTContext.obj \
    $(SLO)$/ProcAddAttrTContext.obj \
    $(SLO)$/ProcAttrTContext.obj \
    $(SLO)$/PropertyActionsOASIS.obj \
    $(SLO)$/PropertyActionsOOo.obj \
    $(SLO)$/RenameElemTContext.obj \
    $(SLO)$/StyleOASISTContext.obj \
    $(SLO)$/StyleOOoTContext.obj \
    $(SLO)$/TransformerActions.obj \
    $(SLO)$/TransformerBase.obj \
    $(SLO)$/TransformerContext.obj \
    $(SLO)$/TransformerTokenMap.obj \
    $(SLO)$/XMLFilterRegistration.obj

SHL1STDLIBS = \
    $(COMPHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALHELPERLIB) \
    $(SALLIB)
.IF ("$(GUI)"=="UNX" || "$(COM)"=="GCC") && "$(GUI)"!="OS2"
    SHL1STDLIBS += -lxo$(DLLPOSTFIX)
.ELSE
    SHL1STDLIBS += ixo.lib
.ENDIF
SHL1VERSIONMAP = $(SOLARENV)/src/component.map
SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

SLOFILES = $(SHL1OBJS)

.INCLUDE: target.mk

ALLTAR : $(MISC)/xof.component

$(MISC)/xof.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        xof.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt xof.component
