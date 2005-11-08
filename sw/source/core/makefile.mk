#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: rt $ $Date: 2005-11-08 17:14:11 $
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
        $(SLB)$/view.lib \
        $(SLB)$/frmedt.lib \
        $(SLB)$/ole.lib \
        $(SLB)$/fields.lib \
        $(SLB)$/tox.lib \
        $(SLB)$/undo.lib \
        $(SLB)$/bastyp.lib


SUBLIBS2= \
        $(SLB)$/draw.lib \
                $(SLB)$/sw3io.lib \
        $(SLB)$/swg.lib \
        $(SLB)$/layout.lib \
        $(SLB)$/text.lib \
        $(SLB)$/txtnode.lib \
        $(SLB)$/doc.lib \
        $(SLB)$/docnode.lib \
                $(SLB)$/unocore.lib \
                $(SLB)$/objectpositioning.lib \
                $(SLB)$/SwNumberTree.lib

.IF "$(PRODUCT)" == ""
SUBLIBS2+= \
        $(SLB)$/except.lib
.ENDIF

#-------------------------------------------------------------------------

################################################################

LIB1TARGET=$(SLB)$/core1.lib
LIB1FILES= \
        $(SUBLIBS1)

LIB2TARGET=$(SLB)$/core2.lib
LIB2FILES= \
        $(SUBLIBS2)

.INCLUDE :  target.mk

################################################################

#-------------------------------------------------------------------------


