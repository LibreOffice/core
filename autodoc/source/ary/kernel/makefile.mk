#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 17:13:05 $
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

PRJ=..$/..$/..

PRJNAME=autodoc
TARGET=ary_kernel



# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true
PRJINC=$(PRJ)$/source

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/source$/mkinc$/fullcpp.mk



# --- Files --------------------------------------------------------


OBJFILES= \
    $(OBJ)$/ary_disp.obj        \
    $(OBJ)$/ce_null.obj         \
    $(OBJ)$/cessentl.obj        \
    $(OBJ)$/id_gener.obj        \
    $(OBJ)$/inheritgraph.obj    \
    $(OBJ)$/inheritnode.obj     \
    $(OBJ)$/instlist.obj        \
    $(OBJ)$/namesort.obj        \
    $(OBJ)$/nametree.obj        \
    $(OBJ)$/opertype.obj        \
    $(OBJ)$/project.obj         \
    $(OBJ)$/qualiname.obj       \
    $(OBJ)$/reposy.obj          \
    $(OBJ)$/slots.obj           \
    $(OBJ)$/x_ary.obj



# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



