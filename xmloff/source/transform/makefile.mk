#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:52:26 $
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
    $(SALLIB) \
    $(TOOLSLIB)
.IF ("$(GUI)"=="UNX" || "$(COM)"=="GCC") && "$(GUI)"!="OS2"
    SHL1STDLIBS += -lxo$(DLLPOSTFIX)
.ELSE
    SHL1STDLIBS += ixo.lib
.ENDIF
SHL1VERSIONMAP = xof.map
SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

SLOFILES = $(SHL1OBJS)

.INCLUDE: target.mk
