#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-01-11 14:29:41 $
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
#*************************************************************************

PRJ := ..$/..
PRJNAME := xmloff
TARGET := xof

AUTOSEG := true
ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

CDEFS += -DCONV_STAR_FONTS

SHL1TARGET = $(TARGET)$(UPD)$(DLLPOSTFIX)
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
    $(ONELIB) \
    $(RTLLIB) \
    $(SALHELPERLIB) \
    $(SALLIB) \
    $(TOOLSLIB)
.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
    SHL1STDLIBS += -lxo$(OFFICEUPD)$(DLLPOSTFIX)
.ELSE
    SHL1STDLIBS += $(LIBPRE) ixo.lib
.ENDIF
SHL1VERSIONMAP = xof.map
SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

SLOFILES = $(SHL1OBJS)

.INCLUDE: target.mk
