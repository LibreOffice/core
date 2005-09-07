#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 18:02:15 $
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
TARGET=display_toolkit


# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true
PRJINC=$(PRJ)$/source

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/source$/mkinc$/fullcpp.mk


# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/hf_docentry.obj     \
    $(OBJ)$/hf_funcdecl.obj     \
    $(OBJ)$/hf_linachain.obj    \
    $(OBJ)$/hf_navi_main.obj    \
    $(OBJ)$/hf_navi_sub.obj     \
    $(OBJ)$/hf_title.obj     	\
    $(OBJ)$/htmlfile.obj        \
    $(OBJ)$/out_node.obj        \
    $(OBJ)$/out_position.obj    \
    $(OBJ)$/out_tree.obj        \
    $(OBJ)$/outputstack.obj


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



