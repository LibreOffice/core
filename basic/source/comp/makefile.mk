#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: vg $ $Date: 2007-02-06 14:13:16 $
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

EXCEPTIONSFILES=$(SLO)$/parser.obj

# --- Targets --------------------------------------------------------------

.INCLUDE :  target.mk
