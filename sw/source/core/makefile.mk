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

PRJ=..$/..
PRJNAME=sw

TARGET=core

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

.IF "$(CALLTARGETS)"=="core"
RC_SUBDIRS=
.ENDIF

# --- Files --------------------------------------------------------


SUBLIBS1= \
        $(SLB)$/access.lib \
        $(SLB)$/graphic.lib \
        $(SLB)$/para.lib \
        $(SLB)$/attr.lib \
        $(SLB)$/edit.lib \
        $(SLB)$/crsr.lib \
        $(SLB)$/view.lib


SUBLIBS2= \
        $(SLB)$/draw.lib \
        $(SLB)$/sw3io.lib \
        $(SLB)$/swg.lib \
        $(SLB)$/layout.lib \
        $(SLB)$/text.lib \
        $(SLB)$/doc.lib \
        $(SLB)$/txtnode.lib

SUBLIBS3= \
        $(SLB)$/docnode.lib \
                $(SLB)$/unocore.lib \
                $(SLB)$/objectpositioning.lib \
                $(SLB)$/SwNumberTree.lib \
                $(SLB)$/tablecore.lib

SUBLIBS4= \
        $(SLB)$/frmedt.lib \
        $(SLB)$/ole.lib \
        $(SLB)$/fields.lib \
        $(SLB)$/tox.lib \
        $(SLB)$/undo.lib \
        $(SLB)$/bastyp.lib
#-------------------------------------------------------------------------

LIB1TARGET=$(SLB)$/core1.lib
LIB1FILES= \
        $(SUBLIBS1)

LIB2TARGET=$(SLB)$/core2.lib
LIB2FILES= \
        $(SUBLIBS2)

LIB3TARGET=$(SLB)$/core3.lib
LIB3FILES= \
        $(SUBLIBS3)

LIB4TARGET=$(SLB)$/core4.lib
LIB4FILES= \
        $(SUBLIBS4)

.INCLUDE :  target.mk

#-------------------------------------------------------------------------
