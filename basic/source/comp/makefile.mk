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

PRJNAME=basic
TARGET=comp

# --- Settings ------------------------------------------------------------

.INCLUDE :  settings.mk

CXXFILES= \
    sbcomp.cxx   \
    dim.cxx      \
    exprtree.cxx \
    exprnode.cxx \
    exprgen.cxx  \
    codegen.cxx	 \
    io.cxx       \
    loops.cxx    \
    parser.cxx   \
    scanner.cxx  \
    token.cxx    \
    symtbl.cxx	 \
    buffer.cxx

SLOFILES= \
    $(SLO)$/sbcomp.obj		\
    $(SLO)$/dim.obj			\
    $(SLO)$/exprtree.obj		\
    $(SLO)$/exprnode.obj		\
    $(SLO)$/exprgen.obj		\
    $(SLO)$/codegen.obj		\
    $(SLO)$/io.obj			\
    $(SLO)$/loops.obj		\
    $(SLO)$/parser.obj		\
    $(SLO)$/scanner.obj		\
    $(SLO)$/token.obj		\
    $(SLO)$/symtbl.obj		\
    $(SLO)$/buffer.obj

EXCEPTIONSFILES= \
    $(SLO)$/parser.obj \
    $(SLO)$/exprtree.obj

# --- Targets --------------------------------------------------------------

.INCLUDE :  target.mk
